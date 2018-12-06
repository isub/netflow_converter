#ifndef __IPFIX_MAPPER_H__
#define __IPFIX_MAPPER_H__

#include <string>

int ipfix_field_name_mapper( const char *p_pszFieldName, std::string *p_pstrMappedName, uint32_t *p_pui32Divider );

#endif /* __IPFIX_MAPPER_H__ */
