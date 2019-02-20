#include <map>
#include <string>

#include "../util/logger.h"
#include "../converter/ipfix/mapper/ipfix_mapper.h"
#include "../converter/nfv9/mapper/nfv9_mapper.h"
#include "netflow_converter_mapper.h"

struct SMapper {
	std::string m_strMappedName;
	uint32_t m_ui32Div;
	SMapper( const char *p_pszMappedName, uint32_t p_ui32Div ) : m_strMappedName( p_pszMappedName ), m_ui32Div( p_ui32Div )
	{
	}
};

static std::map<uint16_t, std::map<std::string, SMapper> > g_mapMapper;

void nfc_mapper_add( const uint16_t p_ui16Version, const char *p_pszFieldName, const char *p_pszFieldNameMapped, uint32_t p_ui32Div )
{
	SMapper soMapper( p_pszFieldNameMapped, p_ui32Div );
	std::map<uint16_t, std::map<std::string, SMapper> >::iterator iterMapper;

	iterMapper = g_mapMapper.find( p_ui16Version );
	if( iterMapper != g_mapMapper.end() ) {
		iterMapper->second.insert( std::pair < std::string, SMapper>( p_pszFieldName, soMapper ) );
	} else {
		std::map<std::string, SMapper> soMap;

		soMap.insert( std::pair<std::string, SMapper>( std::string( p_pszFieldName ), soMapper ) );
		g_mapMapper.insert( std::pair < uint16_t, std::map<std::string, SMapper> >( p_ui16Version, soMap ) );
	}

	logger_message( 9, "%s: version: '%u'; attribute name: '%s'; mapped attribute name: '%s'; divider: '%u'\n", __FUNCTION__, p_ui16Version, p_pszFieldName, p_pszFieldNameMapped, p_ui32Div );
}

void nf_converter_field_name_mapper( const uint16_t p_ui16Version, const std::string &p_strFieldName, std::string &p_strMappedFieldName, uint32_t *p_pui32Divider )
{
	std::map<uint16_t, std::map<std::string, SMapper> >::iterator iterMapper;
	std::map<std::string, SMapper>::iterator iterField;

	iterMapper = g_mapMapper.find( p_ui16Version );
	if( iterMapper != g_mapMapper.end() ) {
	} else {
		goto __give_out_result__;
	}

	iterField = iterMapper->second.find( p_strFieldName );
	if( iterField == iterMapper->second.end() ) {
		goto __give_out_result__;
	} else {
		p_strMappedFieldName = iterField->second.m_strMappedName;
		*p_pui32Divider = iterField->second.m_ui32Div;
		return;
	}

__give_out_result__:
	p_strMappedFieldName = p_strFieldName;
	p_pui32Divider = 0;
}
