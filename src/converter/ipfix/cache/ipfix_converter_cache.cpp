#include <vector>
#include <map>
#include <errno.h>
#include <pthread.h>

#include "../../../util/logger.h"
#include "../data_types/ipfix_data_types.h"
#include "ipfix_converter_cache.h"

/* значение этой структуры используется в качестве ключа в ассоциативном списке */
struct SIPFIXTemplateCache {
	uint32_t m_ui32ObservationDomain;
	uint16_t m_ui16TemplateId;
	bool operator < ( const SIPFIXTemplateCache &p_soRight ) const;
	SIPFIXTemplateCache( const uint32_t p_ui32ObservDomainId, const uint16_t p_ui16TemplateId )
		: m_ui32ObservationDomain( p_ui32ObservDomainId ), m_ui16TemplateId( p_ui16TemplateId ) { }
};

/* значения этих структур используются в качестве значений в ассоциативном списке */
struct SIPFIXTemplateField {
  SIPFIXField m_soField;
  off_t m_stOffset;
  bool operator == ( const SIPFIXTemplateField &p_soRight ) const;
  SIPFIXTemplateField( SIPFIXField &p_soField, off_t p_stOffset ) : m_soField( p_soField ), m_stOffset( p_stOffset ) { }
};

struct SIPFIXTemplateFieldList {
	size_t   m_stDataSetLength;
	std::vector< SIPFIXTemplateField > m_vectFieldList;
	void AddField( SIPFIXField &p_soIPFIXField );
	bool operator == ( const SIPFIXTemplateFieldList &p_soRight ) const;
	SIPFIXTemplateFieldList() : m_stDataSetLength( 0 ) { }
	~SIPFIXTemplateFieldList() { m_vectFieldList.clear(); }
};

static pthread_rwlock_t g_tIPFIXCacheMutex;
static std::map< SIPFIXTemplateCache, SIPFIXTemplateFieldList* > g_mapTemplateCache;

static int ipfix_converter_template_cache_rd_lock();
static int ipfix_converter_template_cache_wr_lock();
static int ipfix_converter_template_cache_unlock();

int ipfix_converter_cache_init()
{
  int iRetVal = 0;

  iRetVal = pthread_rwlock_init( &g_tIPFIXCacheMutex, NULL );

  return iRetVal;
}

void ipfix_converter_cache_fin()
{
  pthread_rwlock_destroy( &g_tIPFIXCacheMutex );
}

SIPFIXTemplateCache * ipfix_converter_create_template( const uint32_t p_ui32ObservDomainId, const uint16_t p_ui16TemplateId )
{
	return ( new SIPFIXTemplateCache( p_ui32ObservDomainId, p_ui16TemplateId ) );
}

SIPFIXTemplateFieldList * ipfix_converter_create_template_fieldList()
{
	return ( new SIPFIXTemplateFieldList );
}

void ipfix_converter_add_template_field( SIPFIXTemplateFieldList *p_psoIPFIXTemplateFieldList, SIPFIXField &p_soIPFIXField )
{
	p_psoIPFIXTemplateFieldList->AddField( p_soIPFIXField );
}

int ipfix_converter_add_template( SIPFIXTemplateCache * p_psoIPFIXTemplateCache, SIPFIXTemplateFieldList *p_psoFieldList )
{
	int iRetVal = 0;
	int iFnRes;
	std::map< SIPFIXTemplateCache, SIPFIXTemplateFieldList * >::iterator iter;

	if( 0 == ( iFnRes = ipfix_converter_template_cache_wr_lock() ) ) {
	} else {
		/* ошибка блокировки мьютекса */
		return iFnRes;
	}

	iter = g_mapTemplateCache.find( *p_psoIPFIXTemplateCache );
	if( iter == g_mapTemplateCache.end() ) {
		/* there is no template in the cache */
		logger_message( 5,
			"this is a new template %u [field count: %u] for observation domain %#010x\n",
			p_psoIPFIXTemplateCache->m_ui16TemplateId, p_psoFieldList->m_vectFieldList.size(), p_psoIPFIXTemplateCache->m_ui32ObservationDomain );
		g_mapTemplateCache.insert( std::pair<SIPFIXTemplateCache, SIPFIXTemplateFieldList *>( *p_psoIPFIXTemplateCache, p_psoFieldList ) );
	} else {
		if( ( *p_psoFieldList ) == ( *( iter->second ) ) ) {
			delete p_psoFieldList;
			iRetVal = EALREADY;
		} else {
			logger_message( 5,
				"old template: %u[%u]:%#010x; new template: %u[%u]:%#010x\n",
				iter->first.m_ui16TemplateId, iter->second->m_vectFieldList.size(), iter->first.m_ui32ObservationDomain,
				p_psoIPFIXTemplateCache->m_ui16TemplateId, p_psoFieldList->m_vectFieldList.size(), p_psoIPFIXTemplateCache->m_ui32ObservationDomain );
			delete &( *iter->second );
			iter->second = p_psoFieldList;
		}
	}

	if( 0 == ( iFnRes = ipfix_converter_template_cache_unlock() ) ) {
	} else {
		/* ошибка разблокировки мьютекса */
	}

	delete p_psoIPFIXTemplateCache;

	return iRetVal;
}

bool SIPFIXTemplateCache::operator < ( const SIPFIXTemplateCache &p_soRight ) const
{
	if( m_ui32ObservationDomain < p_soRight.m_ui32ObservationDomain ) {
		return true;
	}

	if( m_ui32ObservationDomain > p_soRight.m_ui32ObservationDomain ) {
		return false;
	}

	if( m_ui16TemplateId < p_soRight.m_ui16TemplateId ) {
		return true;
	} else {
		return false;
	}
}

bool SIPFIXTemplateFieldList::operator == ( const SIPFIXTemplateFieldList &p_soRight ) const
{
	if( m_vectFieldList.size() == p_soRight.m_vectFieldList.size() ) {
	} else {
		return false;
	}

	for( size_t stInd = 0; stInd < m_vectFieldList.size(); ++stInd ) {
		if( m_vectFieldList[ stInd ] == p_soRight.m_vectFieldList[ stInd ] ) {
		} else {
			return false;
		}
	}

	return true;
}

SIPFIXTemplateFieldList * ipfix_converter_get_field_list( uint32_t p_ui32ObservDomainId, uint16_t p_ui16TemplateId )
{
	SIPFIXTemplateFieldList * psoRetVal;
	int iFnRes;
	SIPFIXTemplateCache soTemplate( p_ui32ObservDomainId, p_ui16TemplateId );
	std::map< SIPFIXTemplateCache, SIPFIXTemplateFieldList * >::iterator iter;

	if( 0 == ( iFnRes = ipfix_converter_template_cache_rd_lock() ) ) {
	} else {
		/* ошибка блокировки мьютекса */
		return NULL;
	}

	iter = g_mapTemplateCache.find( soTemplate );
	if( iter != g_mapTemplateCache.end() ) {
		psoRetVal = iter->second;
		logger_message( 5,
			"%s: selected template: %u[%u]:%#010x\n",
			__FUNCTION__, iter->first.m_ui16TemplateId, iter->second->m_vectFieldList.size(), iter->first.m_ui32ObservationDomain );
	} else {
		psoRetVal = NULL;
	}

	if( 0 == ( iFnRes = ipfix_converter_template_cache_unlock() ) ) {
	} else {
		/* ошибка разблокировки мьютекса */
	}

	return psoRetVal;
}

uint16_t ipfix_converter_get_field_count( SIPFIXTemplateFieldList * p_psoFieldList )
{
	uint16_t ui16RetVal;

	ipfix_converter_template_cache_rd_lock();

	ui16RetVal = p_psoFieldList->m_vectFieldList.size();

	ipfix_converter_template_cache_unlock();

	return ui16RetVal;
}

size_t ipfix_converter_get_data_set_length( SIPFIXTemplateFieldList * p_psoFieldList )
{
  return p_psoFieldList->m_stDataSetLength;
}

int ipfix_converter_template_cache_get_field_info( SIPFIXTemplateFieldList * p_psoFieldList, uint16_t p_ui16FieldIndex, SIPFIXField *p_psoField, off_t *p_pstOffset )
{
  if ( p_ui16FieldIndex < p_psoFieldList->m_vectFieldList.size() ) {
  } else {
    return EINVAL;
  }

  *p_psoField = p_psoFieldList->m_vectFieldList[ p_ui16FieldIndex ].m_soField;
  *p_pstOffset = p_psoFieldList->m_vectFieldList[ p_ui16FieldIndex ].m_stOffset;

  return 0;
}

static int ipfix_converter_template_cache_rd_lock()
{
  return pthread_rwlock_rdlock( &g_tIPFIXCacheMutex );
}

static int ipfix_converter_template_cache_wr_lock()
{
  return pthread_rwlock_unlock( &g_tIPFIXCacheMutex );
}

static int ipfix_converter_template_cache_unlock()
{
  return pthread_rwlock_unlock( &g_tIPFIXCacheMutex );
}

void SIPFIXTemplateFieldList::AddField( SIPFIXField &p_soIPFIXField )
{
	SIPFIXTemplateField soTmplsFld( p_soIPFIXField, m_stDataSetLength );
	m_stDataSetLength += p_soIPFIXField.m_soFieldCommon.m_ui16FieldLength;

	m_vectFieldList.push_back( soTmplsFld );
}

bool SIPFIXTemplateField::operator == ( const SIPFIXTemplateField &p_soRight ) const
{
	if( m_soField == p_soRight.m_soField ) {
	} else {
		return false;
	}

	if( m_stOffset == p_soRight.m_stOffset ) {
	} else {
		return false;
	}

	return true;
}
