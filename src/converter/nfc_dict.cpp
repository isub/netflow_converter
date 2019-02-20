#include <arpa/inet.h>
#include <endian.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "../util/options.h"
#include "nfc_dict.h"

int nfc_dict_get_integer_value( const uint8_t *p_puiData, const size_t p_stDataLength, uint64_t *p_pui64Value )
{
	if( p_stDataLength <= sizeof( uint64_t ) ) {
	} else {
		return EINVAL;
	}

	*p_pui64Value = 0;
	memcpy( reinterpret_cast< uint8_t* >( p_pui64Value ) + sizeof( uint64_t ) - p_stDataLength, p_puiData, p_stDataLength );
	*p_pui64Value = be64toh( *p_pui64Value );

	return 0;
}

int nfc_dict_get_inetaddr_value( int p_iAddressFamily, const uint8_t *p_puiData, const size_t p_stDataLength, std::string *p_pstrValue, uint64_t *p_pui64ForFilter )
{
	in_addr soAddr;
	in6_addr soAddr6;
	char mcBuf[32];
	void *pvSrc;

	switch( p_iAddressFamily ) {
		case AF_INET:
		{

			if( p_stDataLength <= sizeof( soAddr ) ) {
			} else {
				return EINVAL;
			}

			memset( &soAddr.s_addr, 0, sizeof( soAddr.s_addr ) );
			memcpy( reinterpret_cast< uint8_t* >( &soAddr.s_addr ) + sizeof( soAddr.s_addr ) - p_stDataLength, p_puiData, p_stDataLength );
			pvSrc = &soAddr;
			*p_pui64ForFilter = soAddr.s_addr;

			break;
		}
		case AF_INET6:
		{
			if( p_stDataLength <= sizeof( soAddr6 ) ) {
			} else {
				return EINVAL;
			}

			memset( &soAddr6.s6_addr, 0, sizeof( soAddr6.s6_addr ) );
			memcpy( reinterpret_cast< uint8_t* >( &soAddr6.s6_addr ) + sizeof( soAddr6.s6_addr ) - p_stDataLength, p_puiData, p_stDataLength );
			*p_pui64ForFilter = 0;
			pvSrc = &soAddr6;

			break;
		}
		default:
			return EINVAL;
	}

	if( NULL != inet_ntop( p_iAddressFamily, pvSrc, mcBuf, sizeof( mcBuf ) ) ) {
		*p_pstrValue = mcBuf;
	} else {
		return errno;
	}

	return 0;
}

int nfc_dict_get_time_value( uint32_t p_ui32Div, uint64_t p_ui64Time, std::string *p_pstrValue )
{
	time_t tmTime;

	tmTime = p_ui64Time / ( p_ui32Div != 0 ? p_ui32Div : 1 );

	tm soTm;

	if( NULL != gmtime_r( &tmTime, &soTm ) ) {
	} else {
		return EINVAL;
	}

	char mcBuf[64];
	size_t stLen;

	stLen = strftime( mcBuf, sizeof( mcBuf ), g_psoOpt->m_soOutputFormat.m_strOutputFormatDate.c_str(), &soTm );
	if( stLen > 0 && stLen < sizeof( mcBuf ) ) {
	} else {
		return EINVAL;
	}

	*p_pstrValue = mcBuf;

	if( 0 != g_psoOpt->m_soOutputFormat.m_strOutputFormatDateAdd.length() ) {
	} else {
		return 0;
	}

	uint32_t ui32Mul;
	time_t tmNSec;

	tmNSec = p_ui64Time % p_ui32Div;
	const char * pszSplitSecond;

	if( 0 == g_psoOpt->m_soOutputFormat.m_strOutputFormatDateAdd.compare( "sec" ) ) {
		return 0;
	} else if( 0 == g_psoOpt->m_soOutputFormat.m_strOutputFormatDateAdd.compare( "msec" ) ) {
		ui32Mul = 1000;
		pszSplitSecond = ":%03u";
	} else if( 0 == g_psoOpt->m_soOutputFormat.m_strOutputFormatDateAdd.compare( "usec" ) ) {
		ui32Mul = 1000000;
		pszSplitSecond = ":%06u";
	} else if( 0 == g_psoOpt->m_soOutputFormat.m_strOutputFormatDateAdd.compare( "nsec" ) ) {
		ui32Mul = 1000000000;
		pszSplitSecond = ":%09u";
	} else {
		return EINVAL;
	}

	snprintf( mcBuf, sizeof( mcBuf ), pszSplitSecond, tmNSec * ( ui32Mul / p_ui32Div ) );

	*p_pstrValue += mcBuf;

	return 0;
}
