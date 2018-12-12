#include <errno.h>
/* для mmap */
#include <sys/mman.h>
/* для open */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
/* для close*/
#include <unistd.h>
/* для free */
#include <stdlib.h>

#include <string>

#include "file_list.h"
#include "data_loader.h"
#include "data_loader_bz.h"
#include "../util/logger.h"

namespace nf_data_loader {
  enum ECompressionType {
    m_eNone,
    m_eBz2
  };

  struct SFileInfo {
    const std::string *m_pstrFileName;
    void              *m_pvMMap;
    size_t             m_stFileSize;
    size_t             m_stFileOffs;
    size_t             m_stDataWritten;
    int                m_iFD;
    void              *m_pvDecompressedData;
    nf_data_loader::ECompressionType m_eCompressionType;
    nf_data_loader_bz::SBz2Data * m_psoBz2Data;
    SFileInfo( const std::string *p_pstrFileName )
      : m_pstrFileName( p_pstrFileName ), m_pvMMap( MAP_FAILED ), m_stFileSize( 0 ), m_stFileOffs( 0 ), m_stDataWritten( 0 ), m_iFD( -1 ), m_pvDecompressedData( NULL ), m_eCompressionType( m_eNone ), m_psoBz2Data( NULL )
    { }
    virtual ~SFileInfo();
  };
}

/* общее количество элементов, которое было открыто */
static uint32_t g_ui32OpenedFilesCount;

void data_loader_close();

nf_data_loader::SFileInfo * data_loader_open( nf_file_list::SFileInfo *p_psoFileInfo )
{
  int iRetVal = 0;
  nf_data_loader::SFileInfo *psoRetVal = new nf_data_loader::SFileInfo( file_list_get_file_name( p_psoFileInfo ) );

  psoRetVal->m_iFD = open( psoRetVal->m_pstrFileName->c_str(), O_RDONLY );
  if ( -1 != psoRetVal->m_iFD ) {
    psoRetVal->m_pvMMap = mmap( NULL, file_list_get_file_size( p_psoFileInfo ), PROT_READ, MAP_PRIVATE, psoRetVal->m_iFD, 0 );
    if ( MAP_FAILED != psoRetVal->m_pvMMap ) {
      ++ g_ui32OpenedFilesCount;
      psoRetVal->m_stFileSize = file_list_get_file_size( p_psoFileInfo );
      psoRetVal->m_stFileOffs = 0;

      if ( 0 == psoRetVal->m_pstrFileName->compare( psoRetVal->m_pstrFileName->length() - 4, 4, ".bz2" ) ) {
        psoRetVal->m_eCompressionType = nf_data_loader::m_eBz2;
        psoRetVal->m_psoBz2Data = data_loader_bz_init( psoRetVal->m_pvMMap, psoRetVal->m_stFileSize );
        if ( NULL != psoRetVal->m_psoBz2Data ) {
        } else {
          delete psoRetVal;
          return NULL;
        }
      }
    } else {
      iRetVal = errno;
      delete psoRetVal;
      return NULL;
    }
  } else {
    iRetVal = errno;
    delete psoRetVal;
    return NULL;
  }

  return psoRetVal;
}

size_t data_loader_get_data( nf_data_loader::SFileInfo * p_psoFileInfo, const size_t p_stAmount, uint8_t **p_ppData )
{
  int stRetVal = 0;

  /* проверяем есть ли еще непрочитанные данные в файле */
  if ( p_psoFileInfo->m_stFileSize != p_psoFileInfo->m_stFileOffs ) {
  } else {
    return 0;
  }

  if ( nf_data_loader::m_eNone == p_psoFileInfo->m_eCompressionType ) {
    stRetVal = ( p_stAmount < p_psoFileInfo->m_stFileSize - p_psoFileInfo->m_stFileOffs ) ? p_stAmount : ( p_psoFileInfo->m_stFileSize - p_psoFileInfo->m_stFileOffs );
    *p_ppData = reinterpret_cast<uint8_t*>( p_psoFileInfo->m_pvMMap ) + p_psoFileInfo->m_stFileOffs;
    p_psoFileInfo->m_stFileOffs += stRetVal;
  } else {
    if ( NULL != p_psoFileInfo->m_pvDecompressedData ) {
      free( p_psoFileInfo->m_pvDecompressedData );
      p_psoFileInfo->m_pvDecompressedData = NULL;
    }
    stRetVal = data_loader_bz_get_data( p_psoFileInfo->m_psoBz2Data, p_stAmount, reinterpret_cast<void**>( p_ppData ) );
    if ( 0 != stRetVal ) {
      p_psoFileInfo->m_pvDecompressedData = *p_ppData;
    }
  }

  p_psoFileInfo->m_stDataWritten += stRetVal;

  return stRetVal;
}

void data_loader_close( nf_data_loader::SFileInfo * p_psoFileInfo )
{
  logger_message( 3, "%s: %s operated", __FUNCTION__, p_psoFileInfo->m_pstrFileName->c_str() );
  logger_message( 5, "%s: %s: have read from disk: %u; have written in buffer: %u", __FUNCTION__, p_psoFileInfo->m_pstrFileName->c_str(), p_psoFileInfo->m_stFileOffs, p_psoFileInfo->m_stDataWritten );
  delete p_psoFileInfo;
}

int data_loader_get_total_count_of_opened_files()
{
  return g_ui32OpenedFilesCount;
}

nf_data_loader::SFileInfo::~SFileInfo()
{
  if ( -1 != m_iFD ) {
    close( m_iFD );
    m_iFD = -1;
  }
  if ( MAP_FAILED != m_pvMMap ) {
    munmap( m_pvMMap, m_stFileSize );
    m_pvMMap = MAP_FAILED;
  }
  if ( NULL != m_pvDecompressedData ) {
    free( m_pvDecompressedData );
    m_pvDecompressedData = NULL;
  }
  if ( NULL != m_psoBz2Data ) {
    data_loader_bz_fini( m_psoBz2Data );
    m_psoBz2Data = NULL;
  }
}
