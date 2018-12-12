#ifndef __DATA_LOADER_BZ_H__
#define __DATA_LOADER_BZ_H__

#include <inttypes.h>
#include <stddef.h>

namespace nf_data_loader_bz {
  struct SBz2Data;
}

nf_data_loader_bz::SBz2Data * data_loader_bz_init( const void *p_pvData, const size_t p_stSize );
void data_loader_bz_fini( nf_data_loader_bz::SBz2Data * p_psoBz2Data );

size_t data_loader_bz_get_data( nf_data_loader_bz::SBz2Data * p_psoBz2Data, const size_t p_szRequested, void **p_ppDataDecompresed );

#endif /* __DATA_LOADER_BZ_H__ */
