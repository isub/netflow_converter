#include <stdlib.h>
#include <map>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <endian.h>
#include <time.h>

#include "../../nfc_dict.h"
#include "../../../util/options.h"
#include "../../ipfix/data_types/ipfix_data_types.h"
#include "ipfix_dictionary.h"

struct SIPFIXDictKey {
  std::pair<uint16_t, uint16_t> m_pairInfoElementIdRange;
  uint32_t m_ui32EnterpriseNumber;
  SIPFIXDictKey( uint16_t p_ui16First, uint16_t p_ui16Second, uint32_t p_ui32EntNumb );
};

struct SIPFIXDictData {
  char                 *m_pszName;
  ipfix::EDataType     m_eDataType;
  ipfix::EDataSemantic m_eDataSemantic;
  ipfix::EStatus       m_eStatus;
  char                 *m_pszDescription;
  ipfix::EUnits        m_eUnits;
  std::pair<uint64_t, uint64_t> *m_ppairRange;
  SIPFIXDictData(
    const char *p_pszName,
    ipfix::EDataType p_eDataType,
    ipfix::EDataSemantic p_eDataSemantic,
    ipfix::EStatus p_eStatus,
    const char *p_pszDescription,
    ipfix::EUnits p_eUnits,
    std::pair<uint64_t, uint64_t> *p_pairRange );
  virtual ~SIPFIXDictData();
};

bool operator < ( const SIPFIXDictKey &p_soLeft, const SIPFIXDictKey &p_soRight );

static std::map< SIPFIXDictKey, SIPFIXDictData*> g_mapDict;

static int ipfix_dict_get_range( const char *p_pszRange, std::pair<uint64_t, uint64_t> *p_ppairRange );
static const char * ipfix_dict_get_unit_name( ipfix::EUnits p_eUnit );
static int ipfix_dict_get_value( const ipfix::EDataType p_eDataType, const uint8_t *p_puiData, const size_t p_stDataLength, std::string *p_pstrValue, uint64_t *p_pui64ForFilter );
static int ipfix_dict_get_time_value( const ipfix::EDataType p_eDataType, const uint8_t *p_puiData, const size_t p_stDataLength, std::string *p_pstrValue, uint64_t *p_pui64ForFilter );

int ipfix_dictionary_add(
  const char *p_pszIEIdRange,
  const uint32_t p_ui32EnterpriseNumber,
  const char *p_pszName,
  ipfix::EDataType p_eDataType,
  ipfix::EDataSemantic p_eDataSemantic,
  ipfix::EStatus p_eStatus,
  const char *p_pszDescription,
  ipfix::EUnits p_eUnits,
  const char *p_pszValueRange )
{
  int iRetVal = 0;
  std::pair<uint64_t, uint64_t> pairRange;
  SIPFIXDictData *psoDictData;
  int iFnRes;

  if ( 0 == ipfix_dict_get_range( p_pszIEIdRange, &pairRange ) ) {
  } else {
    return EINVAL;
  }

  SIPFIXDictKey soDictKey( static_cast<uint16_t>( pairRange.first ), static_cast<uint16_t>( pairRange.second ), p_ui32EnterpriseNumber );

  iFnRes = ipfix_dict_get_range( p_pszValueRange, &pairRange );
  psoDictData = new SIPFIXDictData(
    p_pszName,
    p_eDataType,
    p_eDataSemantic,
    p_eStatus,
    p_pszDescription,
    p_eUnits,
    ( ( iFnRes != 0 ) ? NULL : &pairRange ) );

  std::pair<std::map<SIPFIXDictKey, SIPFIXDictData*>::iterator, bool> pairInserResult;

  pairInserResult = g_mapDict.insert( std::pair<SIPFIXDictKey, SIPFIXDictData*>( soDictKey, psoDictData ) );
  logger_message( 2, "ipfix dictionary: id range: %s; Enterprise Number: %u; information element: %s\n", p_pszIEIdRange, p_ui32EnterpriseNumber, p_pszName );

  if ( pairInserResult.second ) {
  } else {
    delete psoDictData;
    iRetVal = EALREADY;
  }

  return iRetVal;
}

static int ipfix_dict_get_range( const char *p_pszRange, std::pair<uint64_t, uint64_t> *p_ppairRange )
{
  int iRetVal = 0;
  const char *pszStart;
  char *pszEnd;
  unsigned long long ullValue;

  p_ppairRange->first = 0;
  p_ppairRange->second = 0;

  pszStart = p_pszRange;
  ullValue = strtoull( pszStart, &pszEnd, 0 );
  if ( p_pszRange != pszEnd ) {
    p_ppairRange->first = static_cast<uint64_t>( ullValue );
  } else {
    return EINVAL;
  }

  if ( *pszEnd == '-' ) {
    pszStart = pszEnd;
    ++ pszStart;
    ullValue = strtoull( pszStart, &pszEnd, 0 );
    if ( pszStart != pszEnd ) {
      p_ppairRange->second = static_cast<uint64_t>( ullValue );
    }
  }

  return iRetVal;
}

bool operator < ( const SIPFIXDictKey &p_soLeft, const SIPFIXDictKey &p_soRight )
{
  /* enterprise numbers are not equal */
  if ( p_soLeft.m_ui32EnterpriseNumber < p_soRight.m_ui32EnterpriseNumber ) {
    return true;
  } else if ( p_soLeft.m_ui32EnterpriseNumber != p_soRight.m_ui32EnterpriseNumber ) {
    return false;
  }

  /* enterprise numbers are equal */
  /* both range are singe value */
  if ( 0 == p_soLeft.m_pairInfoElementIdRange.second && 0 == p_soRight.m_pairInfoElementIdRange.second ) {
    if ( p_soLeft.m_pairInfoElementIdRange.first < p_soRight.m_pairInfoElementIdRange.first ) {
      return true;
    } else {
      return false;
    }
  }

  /* left is range but right is single value */
  if ( 0 != p_soLeft.m_pairInfoElementIdRange.second && 0 == p_soRight.m_pairInfoElementIdRange.second ) {
    if ( p_soLeft.m_pairInfoElementIdRange.second < p_soRight.m_pairInfoElementIdRange.first ) {
      /* range is less than single value */
      return true;
    } else {
      /* range is more than single value or they are equal */
      return false;
    }
  }

  /* left is single value but right is range */
  if ( 0 == p_soLeft.m_pairInfoElementIdRange.second && 0 != p_soRight.m_pairInfoElementIdRange.second ) {
    if ( p_soLeft.m_pairInfoElementIdRange.first < p_soRight.m_pairInfoElementIdRange.first ) {
      /* single value is less than range */
      return true;
    } else {
      /* single value is less than range or they are equal */
      return false;
    }
  }

  /* both are range */
  if ( 0 != p_soLeft.m_pairInfoElementIdRange.second && 0 != p_soRight.m_pairInfoElementIdRange.second ) {
    /* length of left range is less than length of right range */
    if ( p_soLeft.m_pairInfoElementIdRange.second - p_soLeft.m_pairInfoElementIdRange.first < p_soRight.m_pairInfoElementIdRange.second - p_soRight.m_pairInfoElementIdRange.first ) {
      return true;
    }
    if ( p_soLeft.m_pairInfoElementIdRange.second - p_soLeft.m_pairInfoElementIdRange.first == p_soRight.m_pairInfoElementIdRange.second - p_soRight.m_pairInfoElementIdRange.first ) {
      if ( p_soLeft.m_pairInfoElementIdRange.first < p_soRight.m_pairInfoElementIdRange.first ) {
        return true;
      } else {
        return false;
      }
    } else {
      return true;
    }
  }

  return true;
}

SIPFIXDictKey::SIPFIXDictKey( uint16_t p_ui16First, uint16_t p_ui16Second, uint32_t p_ui32EntNumb )
  : m_pairInfoElementIdRange( p_ui16First, p_ui16Second ), m_ui32EnterpriseNumber( p_ui32EntNumb )
{ }

SIPFIXDictData::SIPFIXDictData(
  const char *p_pszName,
  ipfix::EDataType p_eDataType,
  ipfix::EDataSemantic p_eDataSemantic,
  ipfix::EStatus p_eStatus,
  const char *p_pszDescription,
  ipfix::EUnits p_eUnits,
  std::pair<uint64_t, uint64_t> *p_ppairRange )
{
  m_pszName = ( ( NULL == p_pszName ) ? NULL : strdup( p_pszName ) );
  m_eDataType = p_eDataType;
  m_eDataSemantic = p_eDataSemantic;
  m_eStatus = p_eStatus;
  p_pszDescription = ( ( NULL == p_pszDescription ) ? NULL : p_pszDescription );
  m_eUnits = p_eUnits;
  m_ppairRange = ( ( NULL == p_ppairRange ) ? NULL : new std::pair<uint64_t, uint64_t>(*p_ppairRange) );
}

SIPFIXDictData::~SIPFIXDictData()
{
  if ( NULL != m_pszName ) {
    free( m_pszName );
    m_pszName = NULL;
  }
  if ( NULL != m_pszDescription ) {
    free( m_pszDescription );
    m_pszDescription = NULL;
  }
  if ( NULL != m_ppairRange ) {
    delete m_ppairRange;
    m_ppairRange = NULL;
  }
}

int ipfix_dictionary_convert_value(
  const uint16_t p_ui16IEId,
  const uint32_t p_ui32EntNumb,
  const uint8_t *p_puiData,
  const size_t p_stLength,
  std::string &p_strAttrName,
  std::string *p_pstrAttrValue,
  std::string *p_pstrValueUnits,
  uint64_t *p_pui64ForFilter )
{
  int iRetVal = 0;
  SIPFIXDictKey soDictKey( p_ui16IEId, 0, p_ui32EntNumb );
  std::map<SIPFIXDictKey, SIPFIXDictData*>::iterator iter;

  iter = g_mapDict.find( soDictKey );
  if ( iter != g_mapDict.end() ) {
    p_strAttrName = iter->second->m_pszName;
    if ( NULL != p_pstrValueUnits ) {
      *p_pstrValueUnits = ipfix_dict_get_unit_name( iter->second->m_eUnits ) ? : "";
    }
    if ( NULL != p_pstrAttrValue ) {
      p_pstrAttrValue->clear();
      ipfix_dict_get_value( iter->second->m_eDataType, p_puiData, p_stLength, p_pstrAttrValue, p_pui64ForFilter );
    }
  } else {
    return ENODATA;
  }

  return iRetVal;
}

const char * ipfix_dict_get_iename( const uint16_t p_ui16IEId, const uint32_t p_ui32EntNumb )
{
	SIPFIXDictKey soIEKey = { p_ui16IEId, 0, p_ui32EntNumb };

	std::map<SIPFIXDictKey, SIPFIXDictData*>::iterator iterDict;

	iterDict = g_mapDict.find( soIEKey );
	if( iterDict != g_mapDict.end() ) {
		return iterDict->second->m_pszName;
	} else {
		return NULL;
	}
}

static const char * ipfix_dict_get_unit_name( ipfix::EUnits p_eUnit )
{
  switch ( p_eUnit ) {
    default:
    case ipfix::undefinedUnit:
      return NULL;
    case ipfix::fouroctetwords:
      return "4-octet-words";
    case ipfix::bits:
      return "bits";
    case ipfix::entries:
      return "entries";
    case ipfix::flows:
      return "flows";
    case ipfix::frames:
      return "frames";
    case ipfix::hops:
      return "hops";
    case ipfix::inferred:
      return "inferred";
    case ipfix::messages:
      return "messages";
    case ipfix::seconds:
      return "s";
    case ipfix::milliseconds:
      return "ms";
    case ipfix::microseconds:
      return "us";
    case ipfix::nanoseconds:
      return "ns";
    case ipfix::octets:
      return "octets";
    case ipfix::packets:
      return "packets";
    case ipfix::ports:
      return "ports";
  }
}

static int ipfix_dict_get_value( const ipfix::EDataType p_eDataType, const uint8_t *p_puiData, const size_t p_stDataLength, std::string *p_pstrValue, uint64_t *p_pui64ForFilter )
{
  int iRetVal = 0;

  switch ( p_eDataType ) {
    case ipfix::unsigned8:
    case ipfix::unsigned16:
    case ipfix::unsigned32:
    case ipfix::unsigned64:
	{
      uint64_t ui64Value;
      iRetVal = nfc_dict_get_integer_value( p_puiData, p_stDataLength, &ui64Value );
      if ( 0 == iRetVal ) {
        *p_pstrValue = std::to_string( ui64Value );
        *p_pui64ForFilter = ui64Value;
      }
      break;
    }
    case ipfix::signed8:
    case ipfix::signed16:
    case ipfix::signed32:
    case ipfix::signed64: {
      int64_t i64Value;
      iRetVal = nfc_dict_get_integer_value( p_puiData, p_stDataLength, reinterpret_cast<uint64_t*>( &i64Value ) );
      if ( 0 == iRetVal ) {
        *p_pstrValue = std::to_string( i64Value );
        *p_pui64ForFilter = static_cast<uint64_t>( i64Value );
      }
      break;
    }
    case ipfix::float32:
    case ipfix::float64:
      /* unsupported in current release */
      return EINVAL;
      break;
    case ipfix::boolean:
      /* unsupported in current release */
      return EINVAL;
      break;
    case ipfix::macAddress:
      /* unsupported in current release */
      return EINVAL;
      break;
    case ipfix::octetArray:
    case ipfix::string:
      /* unsupported in current release */
      return EINVAL;
    case ipfix::dateTimeSeconds:
    case ipfix::dateTimeMilliseconds:
    case ipfix::dateTimeMicroseconds:
    case ipfix::dateTimeNanoseconds:
      iRetVal = ipfix_dict_get_time_value( p_eDataType, p_puiData, p_stDataLength, p_pstrValue, p_pui64ForFilter );
      break;
    case ipfix::ipv4Address:
      iRetVal = nfc_dict_get_inetaddr_value( AF_INET, p_puiData, p_stDataLength, p_pstrValue, p_pui64ForFilter );
      break;
    case ipfix::ipv6Address:
      iRetVal = nfc_dict_get_inetaddr_value( AF_INET6, p_puiData, p_stDataLength, p_pstrValue, p_pui64ForFilter );
      break;
    case ipfix::basicList:
    case ipfix::subTemplateList:
    case ipfix::subTemplateMultiList:
    default:
      /* unsupported in current release */
      return EINVAL;
  }

  return iRetVal;
}

static int ipfix_dict_get_time_value( const ipfix::EDataType p_eDataType, const uint8_t *p_puiData, const size_t p_stDataLength, std::string *p_pstrValue, uint64_t *p_pui64ForFilter )
{
	int iRetVal = 0;
	uint32_t ui32Div;
	uint64_t ui64Time;

	switch( p_eDataType ) {
		case ipfix::dateTimeSeconds:
			ui32Div = 1;
			break;
		case ipfix::dateTimeMilliseconds:
			ui32Div = 1000;
			break;
		case ipfix::dateTimeMicroseconds:
			ui32Div = 1000000;
			break;
		case ipfix::dateTimeNanoseconds:
			ui32Div = 1000000000;
			break;
		default:
			return EINVAL;
	}

	iRetVal = nfc_dict_get_integer_value( p_puiData, p_stDataLength, &ui64Time );
	if( 0 == iRetVal ) {
	} else {
		return iRetVal;
	}

	*p_pui64ForFilter = ui64Time;

	iRetVal = nfc_dict_get_time_value( ui32Div, ui64Time, p_pstrValue );

	return iRetVal;
}
