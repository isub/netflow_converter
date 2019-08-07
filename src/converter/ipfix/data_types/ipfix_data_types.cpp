#include <errno.h>
/* memcpy */
#include <string.h>
/* ntohs */
#include <arpa/inet.h>
#include <string>

#include "../../../filter/nf_filter_data.h"
#include "../../../mapper/netflow_converter_mapper.h"
#include "../dict/ipfix_dictionary.h"
#include "../cache/ipfix_converter_cache.h"
#include "../../../util/logger.h"
#include "../../nfc_output_template.h"
#include "ipfix_data_types.h"

#pragma pack(push,1)

struct SIPFIXSet {
  uint16_t m_ui16SetId;
  uint16_t m_ui16Length;
};

struct SIPFIXTemplateRecordHeader {
  uint16_t m_ui16TemplateId;
  uint16_t m_ui16FieldCount;
};

#pragma pack(pop)

static size_t ipfix_extract_set_hdr( uint8_t *p_puiData, SIPFIXSet *p_psoSet ); 
static size_t ipfix_parse_set( SIPFIXHeaderSpecific *p_psoHdr, uint8_t *p_puiData, size_t p_stSize );

static size_t ipfix_extract_template_set_hdr( const uint8_t *p_pui8Data, SIPFIXTemplateRecordHeader *p_psoTemplateHdr );
static size_t ipfix_extract_template_field( uint8_t *p_puiData, SIPFIXField *p_psoTemplateField );
static size_t ipfix_parse_template_set( SIPFIXHeaderSpecific *p_psoHdr, uint8_t *p_puiData, size_t p_stSize );

static size_t ipfix_parse_data_set( SIPFIXHeaderSpecific *p_psoHdr, uint16_t p_ui16TemplateId, uint8_t *p_puiData, size_t p_stSize );

int ifpix_parse_packet( SIPFIXHeaderSpecific *p_psoHdr, uint8_t *p_puiData, size_t p_stSize )
{
  int iRetVal = 0;
  size_t stOperated = 0;
  size_t stFnRes;

  if ( p_psoHdr->m_ui16Length - sizeof( SIPFIXHeader ) <= p_stSize ) {
    do {
      stFnRes = ipfix_parse_set( p_psoHdr, p_puiData + stOperated, p_stSize - stOperated );
      if ( static_cast<size_t>( -1 ) != stFnRes ) {
        stOperated += stFnRes;
      } else {
        break;
      }
    } while ( p_stSize > stOperated );
  } else {
    return EINVAL;
  }

  if ( p_stSize == stOperated ) {
    logger_message( 8, "%s: data were processed completely\n", __FUNCTION__ );
  } else {
    iRetVal = EINVAL;
    logger_message( 0, "%s: data were processed partial: data block size: %u; processed: %u\n", __FUNCTION__, p_stSize, stOperated );
  }

  return iRetVal;
}

bool operator == ( const SIPFIXField &p_soLeft, const SIPFIXField &p_soRight )
{
	if( p_soLeft.m_soFieldCommon.m_ui16FieldLength == p_soRight.m_soFieldCommon.m_ui16FieldLength ) {
	} else {
		return false;
	}

	if( p_soLeft.m_soFieldCommon.m_unionInfoElement.m_ui16FullInfoElementId == p_soRight.m_soFieldCommon.m_unionInfoElement.m_ui16FullInfoElementId ) {
	} else {
		return false;
	}

	if( p_soLeft.m_ui32EnterpriseNumber == p_soRight.m_ui32EnterpriseNumber ) {
	} else {
		return false;
	}

	return true;
}

static size_t ipfix_parse_set( SIPFIXHeaderSpecific *p_psoHdr, uint8_t *p_puiData, size_t p_stSize )
{
  size_t stRetVal = 0;
  SIPFIXSet soSet;

  while ( p_stSize > stRetVal + sizeof( soSet ) /* because later we will read soSet */ ) {
    stRetVal += ipfix_extract_set_hdr( p_puiData + stRetVal, &soSet );
    if ( soSet.m_ui16SetId >= 256 /* && soSet.m_ui16SetId <= 65535 */ ) {
      /* Data set */
      logger_message( 9, "%s: data set\n", __FUNCTION__ );
      stRetVal += ipfix_parse_data_set( p_psoHdr, soSet.m_ui16SetId, p_puiData + stRetVal, soSet.m_ui16Length );
    } else if ( soSet.m_ui16SetId == 2 ) {
      /* Template set */
      stRetVal += ipfix_parse_template_set( p_psoHdr, p_puiData + stRetVal, soSet.m_ui16Length );
      logger_message( 9, "%s: template set\n", __FUNCTION__ );
    } else if ( soSet.m_ui16SetId == 3 ) {
      /* Option Template set */
      logger_message( 9, "%s: option template set\n", __FUNCTION__ );
      /* todo */ stRetVal += soSet.m_ui16Length - sizeof( soSet );
    } else if ( soSet.m_ui16SetId >= 4 && soSet.m_ui16SetId <= 255 ) {
      /* Reserved */
      logger_message( 0, "%s: reserved: set id: %u\n", __FUNCTION__, soSet.m_ui16SetId );
      /* todo */ stRetVal = static_cast<size_t>( -1 );
      break;
    } else {
      /* 0 and 1 are not used */
      logger_message( 0, "%s: unused: set id: %u\n", __FUNCTION__, soSet.m_ui16SetId );
      /* todo */ stRetVal = static_cast<size_t>( -1 );
      break;
    }
  }

  return stRetVal;
}

static size_t ipfix_extract_set_hdr( uint8_t *p_puiData, SIPFIXSet *p_psoSet )
{
  memcpy( p_psoSet, p_puiData, sizeof( *p_psoSet ) );

  p_psoSet->m_ui16SetId = ntohs( p_psoSet->m_ui16SetId );
  p_psoSet->m_ui16Length = ntohs( p_psoSet->m_ui16Length );

  return sizeof( *p_psoSet );
}

static size_t ipfix_parse_template_set( SIPFIXHeaderSpecific *p_psoHdr, uint8_t *p_puiData, size_t p_stSize )
{
	size_t stDataSize = p_stSize - sizeof( SIPFIXSet );
	size_t stOperated = 0;
	SIPFIXTemplateRecordHeader soTmpltHdr;
	SIPFIXField soTmpltFld;
	SIPFIXTemplateCache *psoTmpltCache;
	SIPFIXTemplateFieldList *psoFieldList;
	uint16_t ui16FieldCount = 0;

	stOperated = ipfix_extract_template_set_hdr( p_puiData, &soTmpltHdr );
	logger_message( 5,
		"%s: template %u [field count: %u] for observation domain %#010x encountered\n",
		__FUNCTION__, soTmpltHdr.m_ui16TemplateId, soTmpltHdr.m_ui16FieldCount, p_psoHdr->m_ui32ObservDomainId );

	psoTmpltCache = ipfix_converter_create_template( p_psoHdr->m_ui32ObservDomainId, soTmpltHdr.m_ui16TemplateId );
	if( NULL == psoTmpltCache ) {
		logger_message( 0,
			"%s: fatal error: can not to create template object: template %u [field count : %u] for observation domain %#010x: ",
			__FUNCTION__, soTmpltHdr.m_ui16TemplateId, soTmpltHdr.m_ui16FieldCount, p_psoHdr->m_ui32ObservDomainId );
		return stDataSize;
	}

	psoFieldList = ipfix_converter_create_template_fieldList();

	while( stOperated < stDataSize && ui16FieldCount < soTmpltHdr.m_ui16FieldCount ) {
		stOperated += ipfix_extract_template_field( p_puiData + stOperated, &soTmpltFld );
		ipfix_converter_add_template_field( psoFieldList, soTmpltFld );
		++ui16FieldCount;
	}

	if( soTmpltHdr.m_ui16FieldCount == ui16FieldCount ) {
		ipfix_converter_add_template( psoTmpltCache, psoFieldList );
	} else {
		logger_message( 0, "%s: template field count %u but operated %u\n", __FUNCTION__, soTmpltHdr.m_ui16FieldCount, ui16FieldCount );
		return stDataSize;
	}

	if( stDataSize == stOperated ) {
		/* all data was operated */
		logger_message( 5, "%s: template set operated completely\n", __FUNCTION__ );
	} else {
		/* may by padding */
		logger_message( 5, "%s: template set padding size %u\n", __FUNCTION__, stDataSize - stOperated );
	}

	return stDataSize;
}

static size_t ipfix_extract_template_set_hdr( const uint8_t *p_pui8Data, SIPFIXTemplateRecordHeader *p_psoTemplateHdr )
{
  memcpy( p_psoTemplateHdr, p_pui8Data, sizeof( *p_psoTemplateHdr ) );

  p_psoTemplateHdr->m_ui16TemplateId = ntohs( p_psoTemplateHdr->m_ui16TemplateId );
  p_psoTemplateHdr->m_ui16FieldCount = ntohs( p_psoTemplateHdr->m_ui16FieldCount );

  return sizeof( *p_psoTemplateHdr );
}

static size_t ipfix_extract_template_field( uint8_t *p_puiData, SIPFIXField *p_psoTemplateField )
{
	size_t stRetVal;

	memcpy( &p_psoTemplateField->m_soFieldCommon, p_puiData, sizeof( p_psoTemplateField->m_soFieldCommon ) );
	p_psoTemplateField->m_soFieldCommon.m_unionInfoElement.m_ui16FullInfoElementId = ntohs( p_psoTemplateField->m_soFieldCommon.m_unionInfoElement.m_ui16FullInfoElementId );
	p_psoTemplateField->m_soFieldCommon.m_ui16FieldLength = ntohs( p_psoTemplateField->m_soFieldCommon.m_ui16FieldLength );
	stRetVal = sizeof( p_psoTemplateField->m_soFieldCommon );

	if( 0 == p_psoTemplateField->m_soFieldCommon.m_unionInfoElement.m_ui16EnterpriseBit ) {
		p_psoTemplateField->m_ui32EnterpriseNumber = 0;
	} else {
		memcpy( &p_psoTemplateField->m_ui32EnterpriseNumber, p_puiData, sizeof( p_psoTemplateField->m_ui32EnterpriseNumber ) );
		p_psoTemplateField->m_ui32EnterpriseNumber = ntohs( p_psoTemplateField->m_ui32EnterpriseNumber );
		stRetVal += sizeof( p_psoTemplateField->m_ui32EnterpriseNumber );
	}

	logger_message( 4, "information element id: %s; field length: %u\n", ipfix_dict_get_iename( p_psoTemplateField->m_soFieldCommon.m_unionInfoElement.m_ui16InformationElementId, p_psoTemplateField->m_ui32EnterpriseNumber ), p_psoTemplateField->m_soFieldCommon.m_ui16FieldLength );

	return stRetVal;
}

static size_t ipfix_parse_data_set( SIPFIXHeaderSpecific *p_psoHdr, uint16_t p_ui16TemplateId, uint8_t *p_puiData, size_t p_stSize )
{
	size_t stDataSize = p_stSize - sizeof( SIPFIXSet );
	size_t stOperated = 0;
	size_t stDataSetLen;
	uint16_t ui16FieldCount;
	SIPFIXTemplateFieldList *psoFieldList;
	std::string strAttrName;
	std::string strAttrMappedName;
	std::string strAttrValue;
	std::string strUnits;
	uint32_t ui32Div;

	psoFieldList = ipfix_converter_get_field_list( p_psoHdr->m_ui32ObservDomainId, p_ui16TemplateId );

	if( NULL != psoFieldList ) {
	} else {
		return stDataSize;
	}

	stDataSetLen = ipfix_converter_get_data_set_length( psoFieldList );
	ui16FieldCount = ipfix_converter_get_field_count( psoFieldList );

	SIPFIXField soField;
	off_t stOffset;
	uint64_t ui64Filter;

	SNFCOutputTemplate *psoOutput;

	psoOutput = nfc_outputtemplate_object_create();

	while( stOperated + stDataSetLen <= stDataSize ) {
		nfc_outputtemplate_object_erase( psoOutput );
		for( uint16_t uiInd = 0; uiInd < ui16FieldCount; ++uiInd ) {
			if( 0 == ipfix_converter_template_cache_get_field_info( psoFieldList, uiInd, &soField, &stOffset ) ) {
			} else {
				continue;
			}
			if( 0 == ipfix_dictionary_convert_value( soField.m_soFieldCommon.m_unionInfoElement.m_ui16FullInfoElementId, soField.m_ui32EnterpriseNumber, p_puiData + stOperated + stOffset, soField.m_soFieldCommon.m_ui16FieldLength, strAttrName, &strAttrValue, &strUnits, &ui64Filter ) ) {
				nf_converter_field_name_mapper( 10, strAttrName, strAttrMappedName, &ui32Div );
			} else {
				continue;
			}
			if( ! nf_filter_data( strAttrMappedName, ui64Filter, ui32Div ) ) {
				goto __next_record;
			}
			nfc_outputtemplate_object_add( psoOutput, strAttrMappedName, strAttrValue );
		}
		nfc_outputtemplate_object_output( psoOutput );
	__next_record:
		stOperated += stDataSetLen;
	}

	if( stOperated == stDataSize ) {
		logger_message( 6, "%s: data set operated completely\n", __FUNCTION__ );
	} else {
		logger_message( 6, "%s: data set padding size %u\n", __FUNCTION__, stDataSize - stOperated );
	}

	nfc_outputtemplate_object_delete( psoOutput );

	return stDataSize;
}
