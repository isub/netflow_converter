#ifndef __NETFLOW_DATA_TYPES_H__
#define __NETFLOW_DATA_TYPES_H__

#include <stdint.h>

#pragma pack(push,1)

struct SNetFlowVersion {
  uint16_t m_uiVersionNumber;
};

#pragma pack(pop)

#ifdef __cplus_plus
extern "C" {
#endif

  uint16_t netflow_data_get_version_number( const uint8_t *p_pmuiData );

#ifdef __cplus_plus
}
#endif


#endif /* __NETFLOW_DATA_TYPES_H__ */
