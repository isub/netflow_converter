#include <bzlib.h>
#include <list>
/* для memset */
#include <string.h>
/* для malloc */
#include <stdlib.h>

#include "data_loader_bz.h"
#include "../util/logger.h"

#define BZ_BULK_SIZE 0xA000000
#define GZ_MAKE_UIN64_T(hi,lo) ( static_cast<size_t>( lo ) | ( static_cast<size_t>( hi ) << 32 ) )

struct SDataBulk {
  void *m_pvData;
  size_t m_stSize;
  SDataBulk( void *p_pvData, size_t p_stSize ) : m_pvData( p_pvData ), m_stSize( p_stSize ) { }
  ~SDataBulk() { logger_message( 9, "Data Bulk released" ); }
};

int data_loader_bz_get_data( void *p_pvData, size_t p_stSize, void **p_ppDataDecompresed, size_t *p_pszSizeDecompresed )
{
  int iRetVal = 0;
  bz_stream soBZStream;
  std::list<SDataBulk*> listBulk;
  void *pvBulk;
  size_t stBulkSize;
  size_t stTotalSize = 0;
  int iFnRes;

  memset( &soBZStream, 0, sizeof( soBZStream ) );
  iFnRes = BZ2_bzDecompressInit( &soBZStream, 0, 0 );
  if ( BZ_OK == iFnRes ) {
  } else {
    return iFnRes;
  }

  soBZStream.next_in = reinterpret_cast<char*>( p_pvData );
  soBZStream.avail_in = p_stSize;

  do {
    pvBulk = malloc( BZ_BULK_SIZE );

    soBZStream.next_out = reinterpret_cast<char*>( pvBulk );
    soBZStream.avail_out = BZ_BULK_SIZE;

    iFnRes = BZ2_bzDecompress( &soBZStream );
    if ( BZ_OK == iFnRes || BZ_STREAM_END ==iFnRes ) {
      stBulkSize = BZ_BULK_SIZE - soBZStream.avail_out;
      listBulk.push_back( new SDataBulk( pvBulk, stBulkSize ) );
    } else {
      break;
    }
  } while ( BZ_OK == iFnRes );

  if ( BZ_STREAM_END == iFnRes ) {
    size_t stOffset = 0;

    stTotalSize = GZ_MAKE_UIN64_T( soBZStream.total_out_hi32, soBZStream.total_out_lo32 );
    *p_ppDataDecompresed = malloc( stTotalSize );
    *p_pszSizeDecompresed = stTotalSize;

    for ( std::list<SDataBulk*>::iterator iter = listBulk.begin(); iter != listBulk.end(); ++iter ) {
      memcpy( reinterpret_cast<uint8_t*>( *p_ppDataDecompresed ) + stOffset, ( *iter )->m_pvData, ( *iter )->m_stSize );
      stOffset += ( *iter )->m_stSize;
    }
    if ( stOffset == stTotalSize ) {
    } else {
      logger_message( 0, "invalid size of decompressed file" );
      iRetVal = -1;
    }
  } else {
    iRetVal = -1;
  }

  __clean_and_exit__:
  for ( std::list<SDataBulk*>::iterator iter = listBulk.begin(); iter != listBulk.end(); ++iter ) {
    free( ( *iter )->m_pvData );
    delete *iter;
  }

  BZ2_bzDecompressEnd( &soBZStream );

  return iRetVal;
}
