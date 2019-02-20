#include <vector>
#include <map>
#include <errno.h>
#include <pthread.h>

#include "../../util/logger.h"
#include "ipfix_data_types.h"
#include "ipfix_converter_cache.h"

/* значение этой структуры используется в качестве ключа в ассоциативном списке */
struct SIPFIXTemplateCache {
  uint32_t m_ui32ObservationDomain;
  uint16_t m_ui16TemplateId;
  SIPFIXTemplateFieldList *m_psoFieldList;
  bool operator < ( const SIPFIXTemplateCache &p_soRight ) const;
  SIPFIXTemplateCache( uint32_t p_ui32ObservDomainId, uint16_t p_ui16TemplateId );
  SIPFIXTemplateCache( uint32_t p_ui32ObservDomainId, uint16_t p_ui16TemplateId, SIPFIXTemplateFieldList *p_pvectFieldList );
};

/* значения этих структур используются в качестве значений в ассоциативном списке */
struct SIPFIXTemplateField {
  SIPFIXField m_soField;
  off_t m_stOffset;
  SIPFIXTemplateField( SIPFIXField &p_soField, off_t p_stOffset );
};

struct SIPFIXTemplateFieldList {
  uint16_t m_ui16FieldCount;
  size_t   m_stDataSetLength;
  std::vector< SIPFIXTemplateField> m_vectFieldList;
  void AddField( SIPFIXField &p_soIPFIXField );
  SIPFIXTemplateFieldList() : m_ui16FieldCount( 0 ), m_stDataSetLength( 0 ) { }
};

static pthread_rwlock_t g_tIPFIXCacheMutex;
static std::map< SIPFIXTemplateCache, SIPFIXTemplateFieldList* > g_umapTemplateCache;

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

SIPFIXTemplateCache * ipfix_converter_create_template( uint32_t p_ui32ObservDomainId, uint16_t p_ui16TemplateId )
{
  SIPFIXTemplateCache *psoRetVal;
  int iFnRes;
  SIPFIXTemplateCache soTemplate( p_ui32ObservDomainId, p_ui16TemplateId, NULL );
  std::map< SIPFIXTemplateCache, SIPFIXTemplateFieldList* >::iterator iter;

  if ( 0 == ( iFnRes = ipfix_converter_template_cache_rd_lock() ) ) {
  } else {
    /* ошибка блокировки мьютекса */
    return NULL;
  }

  iter = g_umapTemplateCache.find( soTemplate );
  if ( iter != g_umapTemplateCache.end() ) {
    psoRetVal = NULL;
  } else {
    psoRetVal = new SIPFIXTemplateCache( p_ui32ObservDomainId, p_ui16TemplateId );
  }

  if ( 0 == ( iFnRes = ipfix_converter_template_cache_unlock() ) ) {
  } else {
    /* ошибка разблокировки мьютекса */
  }

  return psoRetVal;
}

void ipfix_converter_add_template_field( SIPFIXTemplateCache *p_psoIPFIXTemplateCache, SIPFIXField &p_soIPFIXField )
{
  p_psoIPFIXTemplateCache->m_psoFieldList->AddField( p_soIPFIXField );
}

int ipfix_converter_add_template( SIPFIXTemplateCache * p_psoIPFIXTemplateCache )
{
  int iRetVal = 0;
  int iFnRes;
  SIPFIXTemplateCache soTemplate( p_psoIPFIXTemplateCache->m_ui32ObservationDomain, p_psoIPFIXTemplateCache->m_ui16TemplateId, NULL );
  std::pair < std::map< SIPFIXTemplateCache, SIPFIXTemplateFieldList* >::iterator, bool> insertResult;

  if ( 0 == ( iFnRes = ipfix_converter_template_cache_wr_lock() ) ) {
  } else {
    /* ошибка блокировки мьютекса */
    return iFnRes;
  }

  insertResult = g_umapTemplateCache.insert( std::pair<SIPFIXTemplateCache, SIPFIXTemplateFieldList* >( soTemplate, p_psoIPFIXTemplateCache->m_psoFieldList ) );
  if ( insertResult.second ) {
  } else {
    delete p_psoIPFIXTemplateCache->m_psoFieldList;
    iRetVal = EALREADY;
  }

  if ( 0 == ( iFnRes = ipfix_converter_template_cache_unlock() ) ) {
  } else {
    /* ошибка разблокировки мьютекса */
  }

  delete p_psoIPFIXTemplateCache;

  return iRetVal;
}

bool SIPFIXTemplateCache::operator < ( const SIPFIXTemplateCache &p_soRight ) const
{
  if ( m_ui32ObservationDomain < p_soRight.m_ui32ObservationDomain ) {
    return true;
  } else {
    return false;
  }

  if ( m_ui32ObservationDomain == p_soRight.m_ui32ObservationDomain ) {
  } else {
    return false;
  }

  if ( m_ui16TemplateId < p_soRight.m_ui16TemplateId ) {
    return true;
  } else {
    return true;
  }
}

SIPFIXTemplateFieldList * ipfix_converter_get_field_list( uint32_t p_ui32ObservDomainId, uint16_t p_ui16TemplateId )
{
  SIPFIXTemplateFieldList * psoRetVal;
  int iFnRes;
  SIPFIXTemplateCache soTemplate ( p_ui32ObservDomainId, p_ui16TemplateId, NULL );
  std::map< SIPFIXTemplateCache, SIPFIXTemplateFieldList* >::iterator iter;

  if ( 0 == ( iFnRes = ipfix_converter_template_cache_rd_lock() ) ) {
  } else {
    /* ошибка блокировки мьютекса */
    return NULL;
  }

  iter = g_umapTemplateCache.find( soTemplate );
  if ( iter != g_umapTemplateCache.end() ) {
    psoRetVal = iter->second;
  } else {
    psoRetVal = NULL;
  }

  if ( 0 == ( iFnRes = ipfix_converter_template_cache_unlock() ) ) {
  } else {
    /* ошибка разблокировки мьютекса */
  }

  return psoRetVal;
}

uint16_t ipfix_converter_get_field_count( SIPFIXTemplateFieldList * p_psoFieldList )
{
  return p_psoFieldList->m_ui16FieldCount;
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

SIPFIXTemplateCache::SIPFIXTemplateCache( uint32_t p_ui32ObservDomainId, uint16_t p_ui16TemplateId )
  : m_ui32ObservationDomain( p_ui32ObservDomainId ), m_ui16TemplateId( p_ui16TemplateId )
{
  m_psoFieldList = new SIPFIXTemplateFieldList;
}

SIPFIXTemplateCache::SIPFIXTemplateCache( uint32_t p_ui32ObservDomainId, uint16_t p_ui16TemplateId, SIPFIXTemplateFieldList *p_psoFieldList )
  : m_ui32ObservationDomain( p_ui32ObservDomainId ), m_ui16TemplateId( p_ui16TemplateId ), m_psoFieldList( p_psoFieldList )
{ }

void SIPFIXTemplateFieldList::AddField( SIPFIXField &p_soIPFIXField )
{
  ++ m_ui16FieldCount;
  SIPFIXTemplateField soTmplsFld( p_soIPFIXField, m_stDataSetLength );
  m_stDataSetLength += p_soIPFIXField.m_soFieldCommon.m_ui16FieldLength;

  m_vectFieldList.push_back( soTmplsFld );
}

SIPFIXTemplateField::SIPFIXTemplateField( SIPFIXField &p_soField, off_t p_stOffset )
  : m_soField( p_soField ), m_stOffset( p_stOffset )
{ }
