#ifndef __NETFLOW_CONVERTER_MAPPER_H__
#define __NETFLOW_CONVERTER_MAPPER_H__

#include <stdint.h>
#include <string>

void nfc_mapper_add( const uint16_t p_ui16Version, const char *p_pszFieldName, const char *p_pszFieldNameMapped, uint32_t p_ui32Div );
void nf_converter_field_name_mapper( const uint16_t p_ui16Version, const std::string &p_strFieldName, std::string &p_strMappedFieldName, uint32_t *p_pui32Divider );

#endif /* __NETFLOW_CONVERTER_MAPPER_H__ */
