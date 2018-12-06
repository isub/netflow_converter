#ifndef __DATA_LOADER_BZ_H__
#define __DATA_LOADER_BZ_H__

#include <inttypes.h>
#include <stddef.h>

#ifdef __cplus_plus
extern "C" {
#endif

  int data_loader_bz_get_data( void *p_pvData, size_t p_stSize, void **p_ppDataDecompresed, size_t *p_pszSizeDecompresed );

#ifdef __cplus_plus
}
#endif

#endif /* __DATA_LOADER_BZ_H__ */
