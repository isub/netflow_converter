#include <arpa/inet.h>

#include "netflow_data_types.h"

uint16_t netflow_data_get_version_number( const uint8_t *p_pmuiData )
{
  uint16_t ui16RetVal = 0;

  ui16RetVal = ntohs( *reinterpret_cast<const uint16_t*>( p_pmuiData ) );

  return ui16RetVal;
}
