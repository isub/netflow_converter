#include <errno.h>
/* memcpy */
#include <string.h>
/* ntohs */
#include <arpa/inet.h>
#include <string>

#include "../../filter/nf_filter_data.h"
#include "../../mapper/netflow_converter_mapper.h"
#include "dict/ipfix_dictionary.h"
#include "ipfix_converter_cache.h"
#include "../../util/logger.h"
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

static size_t ipfix_extract_template_set_hdr( uint8_t *p_puiData, SIPFIXTemplateRecordHeader *p_psoTemplateHdr );
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
    logger_message( 8, "%s: data were processed completely", __FUNCTION__ );
  } else {
    iRetVal = EINVAL;
    logger_message( 0, "%s: data were processed partial: data block size: %u; processed: %u", __FUNCTION__, p_stSize, stOperated );
  }

  return iRetVal;
}

static size_t ipfix_parse_set( SIPFIXHeaderSpecific *p_psoHdr, uint8_t *p_puiData, size_t p_stSize )
{
  size_t stRetVal = 0;
  SIPFIXSet soSet;

  while ( p_stSize > stRetVal + sizeof( soSet ) /* because later we will read soSet */ ) {
    stRetVal += ipfix_extract_set_hdr( p_puiData + stRetVal, &soSet );
    if ( soSet.m_ui16SetId >= 256 && soSet.m_ui16SetId <= 65535 ) {
      /* Data set */
      logger_message( 9, "%s: data set", __FUNCTION__ );
      stRetVal += ipfix_parse_data_set( p_psoHdr, soSet.m_ui16SetId, p_puiData + stRetVal, soSet.m_ui16Length );
    } else if ( soSet.m_ui16SetId == 2 ) {
      /* Template set */
      stRetVal += ipfix_parse_template_set( p_psoHdr, p_puiData + stRetVal, soSet.m_ui16Length );
      logger_message( 9, "%s: template set", __FUNCTION__ );
    } else if ( soSet.m_ui16SetId == 3 ) {
      /* Option Template set */
      logger_message( 9, "%s: option template set", __FUNCTION__ );
      /* todo */ stRetVal += soSet.m_ui16Length - sizeof( soSet );
    } else if ( soSet.m_ui16SetId >= 4 && soSet.m_ui16SetId <= 255 ) {
      /* Reserved */
      logger_message( 0, "%s: reserved: set id: %u", __FUNCTION__, soSet.m_ui16SetId );
      /* todo */ stRetVal = static_cast<size_t>( -1 );
      break;
    } else {
      /* 0 and 1 are not used */
      logger_message( 0, "%s: unused: set id: %u", __FUNCTION__, soSet.m_ui16SetId );
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
  uint16_t ui16FieldCount = 0;
  SIPFIXTemplateCache *psoTmpltCache;

  stOperated = ipfix_extract_template_set_hdr( p_puiData, &soTmpltHdr );
  if ( NULL == ( psoTmpltCache = ipfix_converter_create_template( p_psoHdr->m_ui32ObservDomainId, soTmpltHdr.m_ui16TemplateId ) ) ) {
    /* this template is already in cache */
    logger_message( 9, "%s: template %u for observation domain %u already in cache", __FUNCTION__, soTmpltHdr.m_ui16TemplateId, p_psoHdr->m_ui32ObservDomainId );
    return stDataSize;
  } else {
    /* it is new template */
    logger_message( 9, "%s: this is a new template %u for observation domain %u", __FUNCTION__, soTmpltHdr.m_ui16TemplateId, p_psoHdr->m_ui32ObservDomainId );
  }

  while ( stOperated < stDataSize && ui16FieldCount < soTmpltHdr.m_ui16FieldCount ) {
    stOperated += ipfix_extract_template_field( p_puiData + stOperated, &soTmpltFld );
    ipfix_converter_add_template_field( psoTmpltCache, soTmpltFld );
    ++ ui16FieldCount;
  }

  if ( ui16FieldCount == soTmpltHdr.m_ui16FieldCount ) {
    ipfix_converter_add_template( psoTmpltCache );
  } else {
    logger_message( 0, "%s: template field count %u but operated %u", __FUNCTION__, soTmpltHdr.m_ui16FieldCount, ui16FieldCount );
  }

  if ( stDataSize == stOperated ) {
    /* all data was operated */
    logger_message( 5, "%s: template set operated completely", __FUNCTION__ );
  } else {
    /* may by padding */
    logger_message( 5, "%s: template set padding size %u", __FUNCTION__, stDataSize - stOperated );
  }

  return stDataSize;
}

static size_t ipfix_extract_template_set_hdr( uint8_t *p_puiData, SIPFIXTemplateRecordHeader *p_psoTemplateHdr )
{
  memcpy( p_psoTemplateHdr, p_puiData, sizeof( *p_psoTemplateHdr ) );

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

  if ( 0 == p_psoTemplateField->m_soFieldCommon.m_unionInfoElement.m_ui16EnterpriseBit ) {
    p_psoTemplateField->m_ui32EnterpriseNumber = 0;
  } else {
    memcpy( &p_psoTemplateField->m_ui32EnterpriseNumber, p_puiData, sizeof( p_psoTemplateField->m_ui32EnterpriseNumber ) );
    p_psoTemplateField->m_ui32EnterpriseNumber = ntohs( p_psoTemplateField->m_ui32EnterpriseNumber );
    stRetVal += sizeof( p_psoTemplateField->m_ui32EnterpriseNumber );
  }

  logger_message( 9, "%s: enterprise bit: %u;  information element id: %u; field length: %u; enterprise number: %u", __FUNCTION__, p_psoTemplateField->m_soFieldCommon.m_unionInfoElement.m_ui16EnterpriseBit, p_psoTemplateField->m_soFieldCommon.m_unionInfoElement.m_ui16InformationElementId, p_psoTemplateField->m_soFieldCommon.m_ui16FieldLength, p_psoTemplateField->m_ui32EnterpriseNumber );

  return stRetVal;
}

static size_t ipfix_parse_data_set( SIPFIXHeaderSpecific *p_psoHdr, uint16_t p_ui16TemplateId, uint8_t *p_puiData, size_t p_stSize )
{
  size_t stDataSize = p_stSize - sizeof( SIPFIXSet );
  size_t stOperated = 0;
  size_t stDataSetLen;
  uint16_t ui16FieldCount;
  std::vector<SIPFIXField> *pvectFieldList;
  SIPFIXTemplateFieldList *psoFieldList;
  std::string strAttrName;
  std::string strAttrMappedName;
  std::string strValue;
  std::string strUnits;
  std::string strString;
  uint32_t ui32Div;

  psoFieldList = ipfix_converter_get_field_list( p_psoHdr->m_ui32ObservDomainId, p_ui16TemplateId );

  if ( NULL != psoFieldList ) {
  } else {
    return stDataSize;
  }

  stDataSetLen = ipfix_converter_get_data_set_length( psoFieldList );
  ui16FieldCount = ipfix_converter_get_field_count( psoFieldList );

  SIPFIXField soField;
  off_t stOffset;
  uint64_t ui64Filter;

  while ( stOperated < stDataSize ) {
    for ( uint16_t uiInd = 0; uiInd < ui16FieldCount; ++uiInd ) {
      if ( 0 != strString.length() ) {
        strString += '\t';
      }
      if ( 0 == ipfix_converter_template_cache_get_field_info( psoFieldList, uiInd, &soField, &stOffset ) ) {
      } else {
        break;
      }
      if ( 0 == ipfix_dictionary_convert_value( soField.m_soFieldCommon.m_unionInfoElement.m_ui16FullInfoElementId, soField.m_ui32EnterpriseNumber, p_puiData + stOperated + stOffset, soField.m_soFieldCommon.m_ui16FieldLength, strAttrName, &strValue, &strUnits, &ui64Filter ) ) {
        nf_converter_field_name_mapper( 10, strAttrName.c_str(), &strAttrMappedName, &ui32Div );
        if ( ! nf_filter_data( strAttrMappedName.c_str(), ui64Filter, ui32Div ) ) {
          goto __next_record;
        }
        strAttrMappedName += ':';
        strAttrMappedName += strValue;
        strAttrMappedName += strUnits;
        strString += strAttrMappedName;
      }
    }
    logger_message( 0, strString.c_str() );
    __next_record:
    strString.clear();
    stOperated += stDataSetLen;
  }

  if ( stOperated == stDataSize ) {
    logger_message( 6, "%s: data set operated completely", __FUNCTION__ );
  } else {
    logger_message( 6, "%s: data set padding size %u", __FUNCTION__, stDataSize - stOperated );
  }

  return stDataSize;
}
