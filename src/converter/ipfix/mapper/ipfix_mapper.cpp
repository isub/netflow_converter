#include <string.h>

#include "../../../mapper/netflow_converter_mapper.h"
#include "ipfix_mapper.h"

int ipfix_field_name_mapper( const char *p_pszFieldName, std::string *p_pstrMappedName, uint32_t *p_pui32Divider )
{
  int iRetVal = 0;

  *p_pui32Divider = 1;
  if ( 0 == strcmp( p_pszFieldName, "flowStartSeconds" ) ) {
    *p_pstrMappedName = NF_MAPPER_FLOW_START;
  } else if ( 0 == strcmp( p_pszFieldName, "flowStartMilliseconds" ) ) {
    *p_pstrMappedName = NF_MAPPER_FLOW_START;
    *p_pui32Divider = 1000;
  } else if ( 0 == strcmp( p_pszFieldName, "flowStartMicroseconds" ) ) {
    *p_pstrMappedName = NF_MAPPER_FLOW_START;
    *p_pui32Divider = 1000000;
  } else if ( 0 == strcmp( p_pszFieldName, "flowStartNanoseconds" ) ) {
    *p_pstrMappedName = NF_MAPPER_FLOW_START;
    *p_pui32Divider = 1000000000;
  } else if ( 0 == strcmp( p_pszFieldName, "flowEndSeconds" ) ) {
    *p_pstrMappedName = NF_MAPPER_FLOW_END;
  } else if ( 0 == strcmp( p_pszFieldName, "flowEndMilliseconds" ) ) {
    *p_pstrMappedName = NF_MAPPER_FLOW_END;
    *p_pui32Divider = 1000;
  } else if ( 0 == strcmp( p_pszFieldName, "flowEndMicroseconds" ) ) {
    *p_pstrMappedName = NF_MAPPER_FLOW_END;
    *p_pui32Divider = 1000000;
  } else if ( 0 == strcmp( p_pszFieldName, "flowEndNanoseconds" ) ) {
    *p_pstrMappedName = NF_MAPPER_FLOW_END;
    *p_pui32Divider = 1000000000;
  } else if ( 0 == strcmp( p_pszFieldName, "sourceIPv4Address" ) ) {
    *p_pstrMappedName = NF_MAPPER_SRC_IPV4;
  } else if ( 0 == strcmp( p_pszFieldName, "destinationIPv4Address" ) ) {
    *p_pstrMappedName = NF_MAPPER_DST_IPV4;
  } else if ( 0 == strcmp( p_pszFieldName, "sourceTransportPort" ) ) {
    *p_pstrMappedName = NF_MAPPER_SRC_PORT;
  } else if ( 0 == strcmp( p_pszFieldName, "destinationTransportPort" ) ) {
    *p_pstrMappedName = NF_MAPPER_DST_PORT;
  } else {
    iRetVal = -1;
  }

  return iRetVal;
}
