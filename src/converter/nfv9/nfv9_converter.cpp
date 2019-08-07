#include <netinet/in.h>
#include <string.h>
#include <errno.h>

#include "../nfc_dict.h"
#include "../../util/logger.h"
#include "data_types/nfv9_data_types.h"
#include "nfv9_converter.h"

int nfv9_converter_convert_packet( nf_data_loader::SFileInfo *p_psoDataLoader )
{
	int iRetVal = 0;
	int iFnRes;
	uint8_t *pui8Data;
	SNFV9HeaderSpecific soNFV9Hdr;
	size_t stRead;

	stRead = data_loader_get_data( p_psoDataLoader, sizeof( soNFV9Hdr ), &pui8Data );
	if( stRead == sizeof( soNFV9Hdr ) ) {
		memcpy( &soNFV9Hdr, reinterpret_cast< SNFV9HeaderSpecific* >( pui8Data ), sizeof( soNFV9Hdr ) );
		soNFV9Hdr.m_ui16Count = ntohs( soNFV9Hdr.m_ui16Count );
		soNFV9Hdr.m_ui32SysUpTime = ntohl( soNFV9Hdr.m_ui32SysUpTime );
		soNFV9Hdr.m_ui32UNIXSec = ntohl( soNFV9Hdr.m_ui32UNIXSec );
		soNFV9Hdr.m_ui32SeqNumber = ntohl( soNFV9Hdr.m_ui32SeqNumber );
		soNFV9Hdr.m_ui32SourceId = ntohl( soNFV9Hdr.m_ui32SourceId );
	#ifdef DEBUG
		std::string strTimeValue;
		nfc_dict_get_time_value( 1, static_cast< uint64_t >( soNFV9Hdr.m_ui32UNIXSec ), &strTimeValue );
		logger_message( 5, "%s: count: %u; uptime: %u; UNIX seconds: %s; sequence number: %u; source id: %#010x\n", __FUNCTION__, soNFV9Hdr.m_ui16Count, soNFV9Hdr.m_ui32SysUpTime, strTimeValue.c_str(), soNFV9Hdr.m_ui32SeqNumber, soNFV9Hdr.m_ui32SourceId );
	#endif
	} else {
		return EINVAL;
	}

	iFnRes = nfv9_parse_packet( &soNFV9Hdr, p_psoDataLoader );
	if( 0 <= iFnRes ) {
		logger_message(
			9,
			"%s: operated: %u; must have: %u\n",
			__FUNCTION__, iFnRes, soNFV9Hdr.m_ui16Count );
	} else {
		iRetVal = -1;
	}

	return iRetVal;
}
