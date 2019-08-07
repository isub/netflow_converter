#include <vector>
#include <map>
#include <errno.h>
#include <pthread.h>

#include "../../../util/logger.h"
#include "../data_types/nfv9_data_types.h"
#include "nfv9_converter_cache.h"

/* значение этой структуры используется в качестве ключа в ассоциативном списке */
struct SNFV9TemplateCache {
	uint32_t m_ui32SourceId;
	uint16_t m_ui16TemplateId;
	SNFV9TemplateFieldList *m_psoFieldList;
	bool operator < ( const SNFV9TemplateCache &p_soRight ) const;
	SNFV9TemplateCache( uint32_t p_ui32SourceId, uint16_t p_ui16TemplateId );
	SNFV9TemplateCache( uint32_t p_ui32SourceId, uint16_t p_ui16TemplateId, SNFV9TemplateFieldList *p_pvectFieldList );
};

/* значения этих структур используются в качестве значений в ассоциативном списке */
struct SNFV9TemplateField {
	SNFV9Field m_soField;
	off_t m_stOffset;
	SNFV9TemplateField( SNFV9Field &p_soField, off_t p_stOffset );
};

struct SNFV9TemplateFieldList {
	uint16_t m_ui16FieldCount;
	size_t   m_stDataSetLength;
	std::vector< SNFV9TemplateField> m_vectFieldList;
	void AddField( SNFV9Field &p_soNFV9Field );
	SNFV9TemplateFieldList() : m_ui16FieldCount( 0 ), m_stDataSetLength( 0 ) {}
};

static pthread_rwlock_t g_tNFV9CacheMutex;
static std::map< SNFV9TemplateCache, SNFV9TemplateFieldList* > g_umapTemplateCache;

static int nfv9_converter_template_cache_rd_lock();
static int nfv9_converter_template_cache_wr_lock();
static int nfv9_converter_template_cache_unlock();

int nfv9_converter_cache_init()
{
	int iRetVal = 0;

	iRetVal = pthread_rwlock_init( &g_tNFV9CacheMutex, NULL );

	return iRetVal;
}

void nfv9_converter_cache_fin()
{
	pthread_rwlock_destroy( &g_tNFV9CacheMutex );
}

SNFV9TemplateCache * nfv9_converter_create_template( uint32_t p_ui32SourceId, uint16_t p_ui16TemplateId )
{
	SNFV9TemplateCache *psoRetVal;
	int iFnRes;
	SNFV9TemplateCache soTemplate( p_ui32SourceId, p_ui16TemplateId, NULL );
	std::map< SNFV9TemplateCache, SNFV9TemplateFieldList* >::iterator iter;

	if( 0 == ( iFnRes = nfv9_converter_template_cache_rd_lock() ) ) {
	} else {
	  /* ошибка блокировки мьютекса */
		return NULL;
	}

	iter = g_umapTemplateCache.find( soTemplate );
	if( iter != g_umapTemplateCache.end() ) {
		psoRetVal = NULL;
	} else {
		psoRetVal = new SNFV9TemplateCache( p_ui32SourceId, p_ui16TemplateId );
	}

	if( 0 == ( iFnRes = nfv9_converter_template_cache_unlock() ) ) {
	} else {
	  /* ошибка разблокировки мьютекса */
	}

	return psoRetVal;
}

void nfv9_converter_add_template_field( SNFV9TemplateCache *p_psoNFV9TemplateCache, SNFV9Field &p_soNFV9Field )
{
	p_psoNFV9TemplateCache->m_psoFieldList->AddField( p_soNFV9Field );
}

int nfv9_converter_add_template( SNFV9TemplateCache * p_psoNFV9TemplateCache )
{
	int iRetVal = 0;
	int iFnRes;
	SNFV9TemplateCache soTemplate( p_psoNFV9TemplateCache->m_ui32SourceId, p_psoNFV9TemplateCache->m_ui16TemplateId, NULL );
	std::pair < std::map< SNFV9TemplateCache, SNFV9TemplateFieldList* >::iterator, bool> insertResult;

	if( 0 == ( iFnRes = nfv9_converter_template_cache_wr_lock() ) ) {
	} else {
	  /* ошибка блокировки мьютекса */
		return iFnRes;
	}

	insertResult = g_umapTemplateCache.insert( std::pair<SNFV9TemplateCache, SNFV9TemplateFieldList* >( soTemplate, p_psoNFV9TemplateCache->m_psoFieldList ) );
	if( insertResult.second ) {
	} else {
		delete p_psoNFV9TemplateCache->m_psoFieldList;
		iRetVal = EALREADY;
	}

	if( 0 == ( iFnRes = nfv9_converter_template_cache_unlock() ) ) {
	} else {
	  /* ошибка разблокировки мьютекса */
	}

	delete p_psoNFV9TemplateCache;

	return iRetVal;
}

bool SNFV9TemplateCache::operator < ( const SNFV9TemplateCache &p_soRight ) const
{
	if( m_ui32SourceId < p_soRight.m_ui32SourceId ) {
		return true;
	}

	if( m_ui32SourceId > p_soRight.m_ui32SourceId ) {
		return false;
	}

	if( m_ui16TemplateId < p_soRight.m_ui16TemplateId ) {
		return true;
	}

	if( m_ui16TemplateId > p_soRight.m_ui16TemplateId ) {
		return false;
	}

	if( m_psoFieldList->m_ui16FieldCount < p_soRight.m_psoFieldList->m_ui16FieldCount ) {
		return true;
	} else {
		return false;
	}
}

SNFV9TemplateFieldList * nfv9_converter_get_field_list( uint32_t p_ui32SourceId, uint16_t p_ui16TemplateId )
{
	SNFV9TemplateFieldList * psoRetVal;
	int iFnRes;
	SNFV9TemplateCache soTemplate( p_ui32SourceId, p_ui16TemplateId, NULL );
	std::map< SNFV9TemplateCache, SNFV9TemplateFieldList* >::iterator iter;

	if( 0 == ( iFnRes = nfv9_converter_template_cache_rd_lock() ) ) {
	} else {
	  /* ошибка блокировки мьютекса */
		return NULL;
	}

	iter = g_umapTemplateCache.find( soTemplate );
	if( iter != g_umapTemplateCache.end() ) {
		psoRetVal = iter->second;
	} else {
		psoRetVal = NULL;
	}

	if( 0 == ( iFnRes = nfv9_converter_template_cache_unlock() ) ) {
	} else {
	  /* ошибка разблокировки мьютекса */
	}

	return psoRetVal;
}

uint16_t nfv9_converter_get_field_count( SNFV9TemplateFieldList * p_psoFieldList )
{
	return p_psoFieldList->m_ui16FieldCount;
}

size_t nfv9_converter_get_data_set_length( SNFV9TemplateFieldList * p_psoFieldList )
{
	return p_psoFieldList->m_stDataSetLength;
}

int nfv9_converter_template_cache_get_field_info( SNFV9TemplateFieldList * p_psoFieldList, uint16_t p_ui16FieldIndex, SNFV9Field *p_psoField, off_t *p_pstOffset )
{
	if( p_ui16FieldIndex < p_psoFieldList->m_vectFieldList.size() ) {
	} else {
		return EINVAL;
	}

	*p_psoField = p_psoFieldList->m_vectFieldList[p_ui16FieldIndex].m_soField;
	*p_pstOffset = p_psoFieldList->m_vectFieldList[p_ui16FieldIndex].m_stOffset;

	return 0;
}

static int nfv9_converter_template_cache_rd_lock()
{
	return pthread_rwlock_rdlock( &g_tNFV9CacheMutex );
}

static int nfv9_converter_template_cache_wr_lock()
{
	return pthread_rwlock_unlock( &g_tNFV9CacheMutex );
}

static int nfv9_converter_template_cache_unlock()
{
	return pthread_rwlock_unlock( &g_tNFV9CacheMutex );
}

SNFV9TemplateCache::SNFV9TemplateCache( uint32_t p_ui32SourceId, uint16_t p_ui16TemplateId )
	: m_ui32SourceId( p_ui32SourceId ), m_ui16TemplateId( p_ui16TemplateId )
{
	m_psoFieldList = new SNFV9TemplateFieldList;
}

SNFV9TemplateCache::SNFV9TemplateCache( uint32_t p_ui32SourceId, uint16_t p_ui16TemplateId, SNFV9TemplateFieldList *p_psoFieldList )
	: m_ui32SourceId( p_ui32SourceId ), m_ui16TemplateId( p_ui16TemplateId ), m_psoFieldList( p_psoFieldList )
{
}

void SNFV9TemplateFieldList::AddField( SNFV9Field &p_soNFV9Field )
{
	++ m_ui16FieldCount;
	SNFV9TemplateField soTmplsFld( p_soNFV9Field, m_stDataSetLength );
	m_stDataSetLength += p_soNFV9Field.m_ui16Length;

	m_vectFieldList.push_back( soTmplsFld );
}

SNFV9TemplateField::SNFV9TemplateField( SNFV9Field &p_soField, off_t p_stOffset )
	: m_soField( p_soField ), m_stOffset( p_stOffset )
{
}
