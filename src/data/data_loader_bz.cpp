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

struct nf_data_loader_bz::SBz2Data {
  bz_stream m_soBZStream;
};

struct SDataBulk {
  void *m_pvData;
  size_t m_stSize;
  SDataBulk( void *p_pvData, size_t p_stSize ) : m_pvData( p_pvData ), m_stSize( p_stSize ) { }
  ~SDataBulk() { logger_message( 9, "Data Bulk released" ); }
};

nf_data_loader_bz::SBz2Data * data_loader_bz_init( const void *p_pvData, const size_t p_stSize )
{
  int iFnRes;
  nf_data_loader_bz::SBz2Data * psoRetVal = new nf_data_loader_bz::SBz2Data;

  memset( &psoRetVal->m_soBZStream, 0, sizeof( psoRetVal->m_soBZStream ) );

  iFnRes = BZ2_bzDecompressInit( &psoRetVal->m_soBZStream, 0, 0 );
  if ( BZ_OK == iFnRes ) {
  } else {
    delete psoRetVal;
    return NULL;
  }

  psoRetVal->m_soBZStream.next_in = reinterpret_cast<char*>( const_cast<void*>( p_pvData ) );
  psoRetVal->m_soBZStream.avail_in = p_stSize;

  return psoRetVal;
}

void data_loader_bz_fini( nf_data_loader_bz::SBz2Data * p_psoBz2Data )
{
  BZ2_bzDecompressEnd( &p_psoBz2Data->m_soBZStream );
  delete p_psoBz2Data;
}

size_t data_loader_bz_get_data( nf_data_loader_bz::SBz2Data * p_psoBz2Data, const size_t p_szRequested, void **p_ppDataDecompresed )
{
  size_t stRetVal;
  void *pvBulk;
  int iFnRes;

  pvBulk = malloc( p_szRequested );

  p_psoBz2Data->m_soBZStream.next_out = reinterpret_cast<char*>( pvBulk );
  p_psoBz2Data->m_soBZStream.avail_out = p_szRequested;

  do {
    iFnRes = BZ2_bzDecompress( &p_psoBz2Data->m_soBZStream );
  } while ( 0 != p_psoBz2Data->m_soBZStream.avail_out && BZ_OK == iFnRes && BZ_STREAM_END != iFnRes && 0 != p_psoBz2Data->m_soBZStream.avail_in );

  if ( BZ_OK == iFnRes || BZ_STREAM_END == iFnRes ) {
    stRetVal = p_szRequested - p_psoBz2Data->m_soBZStream.avail_out;
    *p_ppDataDecompresed = pvBulk;
  } else {
    stRetVal = 0;
    free( pvBulk );
  }

  return stRetVal;
}
