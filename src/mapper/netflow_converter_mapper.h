#ifndef __NETFLOW_CONVERTER_MAPPER_H__
#define __NETFLOW_CONVERTER_MAPPER_H__

#include <stdint.h>
#include <string>

#define NF_MAPPER_FLOW_START "flowStart"
#define NF_MAPPER_FLOW_END   "flowEnd"

#define NF_MAPPER_SRC_IPV4   "srcIPv4Address"
#define NF_MAPPER_DST_IPV4   "dstIPv4Address"

#define NF_MAPPER_SRC_PORT   "srcPort"
#define NF_MAPPER_DST_PORT   "dtsPort"

void nf_converter_field_name_mapper( const uint16_t p_ui16Version, const char *p_pszFieldName, std::string *p_pstrMappedFieldName, uint32_t *p_pui32Divider );

#endif /* __NETFLOW_CONVERTER_MAPPER_H__ */
