#include <errno.h>
#include <arpa/inet.h>
#include <string.h>

#include "../../../util/logger.h"
#include "../../../data/data_loader.h"
#include "../../../mapper/netflow_converter_mapper.h"
#include "../../../filter/nf_filter_data.h"
#include "../cache/nfv9_converter_cache.h"
#include "../dict/nfv9_dict.h"
#include "../../nfc_output_template.h"
#include "nfv9_data_types.h"

#pragma pack(push,1)

struct SNFV9FlowSetHdr {
	uint16_t m_ui16FlowSetId;
	uint16_t m_ui16Length;
};

struct SNFV9TemplateFlowSetHdr {
	uint16_t m_ui16TemplateId;
	uint16_t m_ui16FieldCount;
};

#pragma pack(pop)

static int nfv9_read_set_hdr( nf_data_loader::SFileInfo *p_psoDataLoader, SNFV9FlowSetHdr *p_psoSetHdr );
static int nfv9_parse_flowset( nf_data_loader::SFileInfo *p_psoDataLoader, const SNFV9HeaderSpecific *p_psoHdr, const SNFV9FlowSetHdr *p_psoSetHdr );
static size_t nfv9_extract_template_set_hdr( const uint8_t *p_puiData, SNFV9TemplateFlowSetHdr *p_psoTemplateHdr );
static int nfv9_parse_template_flowset( nf_data_loader::SFileInfo *p_psoDataLoader, const SNFV9HeaderSpecific *p_psoHdr, const SNFV9FlowSetHdr *p_psoSetHdr );
static int nfv9_parse_data_flowset( nf_data_loader::SFileInfo *p_psoDataLoader, const SNFV9HeaderSpecific *p_psoHdr, const SNFV9FlowSetHdr *p_psoSetHdr );

int nfv9_parse_packet( const SNFV9HeaderSpecific *p_psoHdr, nf_data_loader::SFileInfo *p_psoDataLoader )
{
	int iRetVal = 0;
	int iFnRes;
	uint16_t ui16Count = 0;
	SNFV9FlowSetHdr soSetHdr;

	while( ui16Count < p_psoHdr->m_ui16Count ) {
		if( 0 == nfv9_read_set_hdr( p_psoDataLoader, &soSetHdr ) ) {
		} else {
			iRetVal = -1;
			break;
		}
		iFnRes = nfv9_parse_flowset( p_psoDataLoader, p_psoHdr, &soSetHdr );
		if( 0 < iFnRes ) {
			/* operated successfully */
			ui16Count += static_cast< uint16_t >( iFnRes );
			iRetVal = ui16Count;
		} else if( 0 == iFnRes ) {
			/* nothing operated */
			break;
		} else {
			/* error occurred */
			iRetVal = -1;
			break;
		}
	}

	return iRetVal;
}

static int nfv9_read_set_hdr( nf_data_loader::SFileInfo *p_psoDataLoader, SNFV9FlowSetHdr *p_psoSetHdr )
{
	int iRetVal = 0;
	uint8_t *pui8Data;
	size_t stRead;

	stRead = data_loader_get_data( p_psoDataLoader, sizeof( *p_psoSetHdr ), &pui8Data );
	if( stRead == sizeof( *p_psoSetHdr ) ) {
		p_psoSetHdr->m_ui16FlowSetId = ntohs( reinterpret_cast< SNFV9FlowSetHdr* >( pui8Data )->m_ui16FlowSetId );
		p_psoSetHdr->m_ui16Length = ntohs( reinterpret_cast< SNFV9FlowSetHdr* >( pui8Data )->m_ui16Length );
	} else {
		iRetVal = EINVAL;
	}

	return iRetVal;
}

static int nfv9_parse_flowset( nf_data_loader::SFileInfo *p_psoDataLoader, const SNFV9HeaderSpecific *p_psoHdr, const SNFV9FlowSetHdr *p_psoSetHdr )
{
	int iRetVal = 0;

	logger_message( 9, "%s: template id %u source id %#010x encountered\n", __FUNCTION__, p_psoSetHdr->m_ui16FlowSetId, p_psoHdr->m_ui32SourceId );

	if( 0 == p_psoSetHdr->m_ui16FlowSetId ) {
		/* template FlowSet */
		iRetVal = nfv9_parse_template_flowset( p_psoDataLoader, p_psoHdr, p_psoSetHdr );
	} else if( 255 >= p_psoSetHdr->m_ui16FlowSetId ) {
		/* reserved */
		logger_message( 0, "%s: reserved template id %u encountered\n", __FUNCTION__, p_psoSetHdr->m_ui16FlowSetId );
		iRetVal = -1;
	} else /* if( 65536 >= p_psoSetHdr->m_ui16FlowSetId ) */ {
		/* data FlowSet */
		iRetVal = nfv9_parse_data_flowset( p_psoDataLoader, p_psoHdr, p_psoSetHdr );
	}

	return iRetVal;
}

static size_t nfv9_extract_template_set_hdr( const uint8_t *p_puiData, SNFV9TemplateFlowSetHdr *p_psoTemplateHdr )
{
	memcpy( p_psoTemplateHdr, p_puiData, sizeof( *p_psoTemplateHdr ) );

	p_psoTemplateHdr->m_ui16TemplateId = ntohs( p_psoTemplateHdr->m_ui16TemplateId );
	p_psoTemplateHdr->m_ui16FieldCount = ntohs( p_psoTemplateHdr->m_ui16FieldCount );

	return sizeof( *p_psoTemplateHdr );
}

size_t nfv9_extract_template_field(const uint8_t *p_pui8Data, SNFV9Field *p_psoNFV9Field )
{
	size_t stRetVal;

	memcpy( p_psoNFV9Field, p_pui8Data, sizeof( *p_psoNFV9Field ) );
	p_psoNFV9Field->m_ui16Length = ntohs( p_psoNFV9Field->m_ui16Length );
	p_psoNFV9Field->m_ui16Type = ntohs( p_psoNFV9Field->m_ui16Type );
	stRetVal = sizeof( *p_psoNFV9Field );

	logger_message( 4, "information element id: %s; field length: %u\n", nfv9_dict_get_iename( p_psoNFV9Field->m_ui16Type ), p_psoNFV9Field->m_ui16Length );

	return stRetVal;
}

static int nfv9_parse_template_flowset( nf_data_loader::SFileInfo *p_psoDataLoader, const SNFV9HeaderSpecific *p_psoHdr, const SNFV9FlowSetHdr *p_psoSetHdr )
{
	int iRetVal = 0;
	uint8_t *pui8Data;
	size_t stDataSize = p_psoSetHdr->m_ui16Length - sizeof( *p_psoSetHdr );
	size_t stOperated = 0;
	SNFV9TemplateFlowSetHdr soTmpltHdr;
	SNFV9Field soTmpltFld;
	uint16_t ui16FieldCount;
	SNFV9TemplateCache *psoTmpltCache;

	if( stDataSize == data_loader_get_data( p_psoDataLoader, stDataSize, &pui8Data ) ) {
	} else {
		return -1;
	}

	while( stOperated < stDataSize ) {
		ui16FieldCount = 0;
		stOperated += nfv9_extract_template_set_hdr( pui8Data, &soTmpltHdr );
		if( NULL == ( psoTmpltCache = nfv9_converter_create_template( p_psoHdr->m_ui32SourceId, soTmpltHdr.m_ui16TemplateId ) ) ) {
			/* this template is already in cache */
			logger_message( 9, "%s: template %u for source id %#010x already in cache\n", __FUNCTION__, soTmpltHdr.m_ui16TemplateId, p_psoHdr->m_ui32SourceId );
			stOperated += sizeof( soTmpltFld ) * soTmpltHdr.m_ui16FieldCount;
			++ iRetVal;
			continue;
		} else {
			/* it is new template */
			logger_message( 4, "this is a new template %u for source id %#010x\n", soTmpltHdr.m_ui16TemplateId, p_psoHdr->m_ui32SourceId );
		}

		while( stOperated < stDataSize && ui16FieldCount < soTmpltHdr.m_ui16FieldCount ) {
			stOperated += nfv9_extract_template_field( pui8Data + stOperated, &soTmpltFld );
			nfv9_converter_add_template_field( psoTmpltCache, soTmpltFld );
			++ ui16FieldCount;
		}

		if( ui16FieldCount == soTmpltHdr.m_ui16FieldCount ) {
			nfv9_converter_add_template( psoTmpltCache );
			++ iRetVal;
		} else {
			logger_message( 0, "%s: template field count %u but operated %u\n", __FUNCTION__, soTmpltHdr.m_ui16FieldCount, ui16FieldCount );
		}
	}

	if( stDataSize == stOperated ) {
		/* all data was operated */
		logger_message( 5, "%s: template set operated completely\n", __FUNCTION__ );
	} else {
		/* may by padding */
		logger_message( 5, "%s: template set padding size %u\n", __FUNCTION__, stDataSize - stOperated );
	}

	return iRetVal;
}

static int nfv9_parse_data_flowset( nf_data_loader::SFileInfo *p_psoDataLoader, const SNFV9HeaderSpecific *p_psoHdr, const SNFV9FlowSetHdr *p_psoSetHdr )
{
	int iRetVal = 0;
	uint16_t ui16DataSetLength;
	SNFV9TemplateFieldList * psoTemplate;
	uint16_t ui16FieldCount;
	size_t stRead;
	size_t stRecordLen;
	size_t stOperated = 0;
	uint8_t *pui8Data;
	SNFV9Field soField;
	off_t stOffset;

	/* determine data size */
	ui16DataSetLength = p_psoSetHdr->m_ui16Length - sizeof( *p_psoSetHdr );

	/* load data block */
	stRead = data_loader_get_data( p_psoDataLoader, ui16DataSetLength, &pui8Data );
	if( stRead == static_cast< size_t >( ui16DataSetLength ) ) {
	} else {
		return -1;
	}

	/* load template description */
	psoTemplate = nfv9_converter_get_field_list( p_psoHdr->m_ui32SourceId, p_psoSetHdr->m_ui16FlowSetId );
	if( NULL != psoTemplate ) {
	} else {
		logger_message( 9, "%s: template id %u source id %#010x was not found in cache\n", __FUNCTION__, p_psoSetHdr->m_ui16FlowSetId, p_psoHdr->m_ui32SourceId );
		return 0;
	}

	ui16FieldCount = nfv9_converter_get_field_count( psoTemplate );
	stRecordLen = nfv9_converter_get_data_set_length( psoTemplate );

	std::string strAttrName;
	std::string strAttrValue;
	std::string strAttrUnits;
	std::string strAttrMappedName;
	uint64_t ui64ForFilter;
	uint32_t ui32Div;
	SNFCOutputTemplate *psoOutput;

	psoOutput = nfc_outputtemplate_object_create();

	while( stOperated + stRecordLen <= static_cast< size_t >( ui16DataSetLength ) ) {
		nfc_outputtemplate_object_erase( psoOutput );
		/* start operate next record */
		for( uint16_t ui16Ind = 0; ui16Ind < ui16FieldCount; ++ui16Ind ) {
			if( 0 == nfv9_converter_template_cache_get_field_info( psoTemplate, ui16Ind, &soField, &stOffset ) ) {
			} else {
				break;
			}
			if( static_cast< size_t >( stOffset ) < stRecordLen ) {
			} else {
				goto __cleanup_and_exit__;
			}
			if( 0 == nfv9_dict_convert_value( p_psoHdr, soField.m_ui16Type, &pui8Data[stOperated + stOffset], soField.m_ui16Length, strAttrName, &strAttrValue, &strAttrUnits, &ui64ForFilter ) ) {
			} else {
				continue;
			}
			nf_converter_field_name_mapper( 9, strAttrName, strAttrMappedName, &ui32Div );
			if( ! nf_filter_data( strAttrMappedName, ui64ForFilter, ui32Div ) ) {
				goto __next_record__;
			}
			nfc_outputtemplate_object_add( psoOutput, strAttrMappedName, strAttrValue );
		}
		nfc_outputtemplate_object_output( psoOutput );
	__next_record__:
		stOperated += stRecordLen;
		++ iRetVal;
	}

__cleanup_and_exit__:
	nfc_outputtemplate_object_delete( psoOutput );

	return iRetVal;
}
