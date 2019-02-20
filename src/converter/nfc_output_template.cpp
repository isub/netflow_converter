#include <vector>
#include <map>

#include "../util/logger.h"
#include "nfc_output_template.h"

struct SNFCOutputTemplate {
	std::map<std::string, size_t> m_mapValueIndex;
	std::vector<std::string > m_vectValue;
	SNFCOutputTemplate( size_t p_stSize ) : m_vectValue( p_stSize )
	{}
};

static std::vector<std::string> g_vectOutputTemplate;

void nfc_outputtemplate_field_add( const char *p_pszFieldName )
{
	g_vectOutputTemplate.push_back( p_pszFieldName );
	logger_message( 5, "'%s': '%s' was appended to output template\n", __FUNCTION__, p_pszFieldName );
}

SNFCOutputTemplate * nfc_outputtemplate_object_create()
{
	SNFCOutputTemplate * psoRetVal = new SNFCOutputTemplate( g_vectOutputTemplate.size() );

	for( size_t i = 0; i < g_vectOutputTemplate.size(); ++i ) {
		psoRetVal->m_mapValueIndex.insert( std::pair<std::string, size_t>( g_vectOutputTemplate[i], i ) );
		logger_message( 9, "'%s': field '%s' has index %u\n", __FUNCTION__, g_vectOutputTemplate[i].c_str(), i );
	}

	return psoRetVal;
}

void nfc_outputtemplate_object_erase( SNFCOutputTemplate * p_psoOutputTemplate )
{
	for(std::string &iterElem : p_psoOutputTemplate->m_vectValue ) {
		logger_message( 9, "'%s': field '%s' was erased\n", __FUNCTION__, iterElem.c_str() );
		iterElem.clear();
	}
	/* if output template is not defined */
	if( 0 == g_vectOutputTemplate.size() ) {
		p_psoOutputTemplate->m_vectValue.clear();
	}
}

void nfc_outputtemplate_object_add( SNFCOutputTemplate * p_psoOutputTemplate, std::string &p_strAttrName, std::string &p_strAttrValue )
{
	if( 0 != g_vectOutputTemplate.size() ) {
		std::map<std::string, size_t>::iterator iterIndex;

		iterIndex = p_psoOutputTemplate->m_mapValueIndex.find( p_strAttrName );
		if( p_psoOutputTemplate->m_mapValueIndex.end() != iterIndex ) {
			p_psoOutputTemplate->m_vectValue[iterIndex->second].assign( p_strAttrValue );
			logger_message( 9, "'%s': field '%s' has value '%s' and placed in %u field\n", __FUNCTION__, p_strAttrName.c_str(), p_strAttrValue.c_str(), iterIndex->second );
		}
	} else {
		/* if output template not defined */
		std::string strValue;

		strValue.assign( p_strAttrName );
		strValue += ':';
		strValue += p_strAttrValue;

		p_psoOutputTemplate->m_vectValue.push_back( strValue );
	}
}

void nfc_outputtemplate_object_output( SNFCOutputTemplate * p_psoOutputTemplate )
{
	bool bFirstLine = true;
	std::string strOutput;

	if( 0 != p_psoOutputTemplate->m_vectValue.size() ) {
		for( size_t i = 0; i < p_psoOutputTemplate->m_vectValue.size(); ++i ) {
			if( !bFirstLine ) {
				strOutput.append( "\t" );
			} else {
				bFirstLine = false;
			}
			logger_message( 9, "'%s': index %u; value '%s'\n", __FUNCTION__, i, p_psoOutputTemplate->m_vectValue[i].c_str() );
			strOutput.append( p_psoOutputTemplate->m_vectValue[i] );
		}
	} else {
		/* if output template not defined */
		for( size_t i = 0; i < p_psoOutputTemplate->m_vectValue.size(); ++i ) {
			if( !bFirstLine ) {
				strOutput.append( "\t" );
			} else {
				bFirstLine = false;
			}
			strOutput.append( p_psoOutputTemplate->m_vectValue[i] );
		}
	}

	strOutput.append( "\n" );

	logger_message( 0, strOutput.c_str() );
}

void nfc_outputtemplate_object_delete( SNFCOutputTemplate * p_psoOutputTemplate )
{
	delete p_psoOutputTemplate;
	logger_message( 9, "'%s': output template object was deleted\n", __FUNCTION__ );
}

void nfc_outputtemplate_output_header()
{
	if( 0 != g_vectOutputTemplate.size() ) {
		std::string strOut;

		for( size_t i = 0; i < g_vectOutputTemplate.size(); ++i ) {
			if( i != 0 ) {
				strOut.append( "\t" );
			} else {
			}
			strOut.append( g_vectOutputTemplate[i] );
		}
		strOut.append( "\n" );

		logger_message( 0, strOut.c_str() );
	}
}
