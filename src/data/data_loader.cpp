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

static const std::string *g_pstrFileName;
static void *g_pvMMap;
static size_t g_stFileSize;
static size_t g_stFileOffs;
static int g_iFD;

/* общее количество элементов, которое было открыто */
static uint32_t g_ui32OpenedFilesCount;

void data_loader_close_mapped();
void data_loader_close_malloced();

int data_loader_open( SFileInfo *p_psoFileInfo )
{
  int iRetVal = 0;

  g_pstrFileName = file_list_get_file_name( p_psoFileInfo );
  g_iFD = open( g_pstrFileName->c_str(), O_RDONLY );
  if ( -1 != g_iFD ) {
    g_pvMMap = mmap( NULL, file_list_get_file_size( p_psoFileInfo ), PROT_READ, MAP_PRIVATE, g_iFD, 0 );
    if ( MAP_FAILED != g_pvMMap ) {
      ++ g_ui32OpenedFilesCount;
      g_stFileSize = file_list_get_file_size( p_psoFileInfo );
      g_stFileOffs = 0;

      if ( 0 == g_pstrFileName->compare( g_pstrFileName->length() - 4, 4, ".bz2" ) ) {
        void *pvData;
        size_t stSize;

        iRetVal = data_loader_bz_get_data( g_pvMMap, g_stFileSize, &pvData, &stSize );
        if ( 0 == iRetVal ) {
          logger_message( 5, "%s : decompressed sucessfully", g_pstrFileName->c_str() );
          data_loader_close_mapped();
          g_pvMMap = pvData;
          g_stFileSize = stSize;
        } else {
          logger_message( 0, "%s : error occurred while decompression", g_pstrFileName->c_str() );
        }
      }
    } else {
      iRetVal = errno;
      close( g_iFD );
    }
  } else {
    iRetVal = errno;
  }

  return iRetVal;
}

size_t data_loader_get_data( const size_t p_stAmount, uint8_t **p_ppData )
{
  int stRetVal = 0;

  /* проверяем есть ли еще непрочитанные данные в файле */
  if ( g_stFileSize != g_stFileOffs ) {
  }

  stRetVal = ( p_stAmount < g_stFileSize - g_stFileOffs ) ? p_stAmount : ( g_stFileSize - g_stFileOffs );
  *p_ppData = reinterpret_cast<uint8_t*>( g_pvMMap ) + g_stFileOffs;
  g_stFileOffs += stRetVal;

  return stRetVal;
}

void data_loader_close_mapped()
{
  if ( MAP_FAILED != g_pvMMap ) {
    munmap( g_pvMMap, g_stFileSize );
    g_pvMMap = MAP_FAILED;
    close( g_iFD );
  }
}

void data_loader_close_malloced()
{
  if ( MAP_FAILED != g_pvMMap ) {
    free( g_pvMMap );
    g_pvMMap = MAP_FAILED;
    close( g_iFD );
  }
}

void data_loader_close()
{
  if ( 0 == g_pstrFileName->compare( g_pstrFileName->length() - 4, 4, ".bz2" ) ) {
    data_loader_close_malloced();
  } else {
    data_loader_close_mapped();
  }
  if ( g_stFileOffs == g_stFileSize ) {
    logger_message( 5, "%s : operated completely", g_pstrFileName != NULL ? g_pstrFileName->c_str() : "<NULL>" );
  } else {
    logger_message( 5, "%s : %f", g_pstrFileName != NULL ? g_pstrFileName->c_str() : "<NULL>", ( static_cast<double>( g_stFileOffs ) / g_stFileSize ) * 100 );
  }

  g_stFileSize = 0;
  g_stFileOffs = 0;
  g_pstrFileName = NULL;
}

int data_loader_get_total_count_of_opened_files()
{
  return g_ui32OpenedFilesCount;
}
