#include <stdint.h>
/* для memcpy */
#include <string.h>
/* для ntohl */
#include <netinet/in.h>

#include <errno.h>

#include "../nfc_dict.h"
#include "../netflow_data_types.h"
#include "data_types/ipfix_data_types.h"
#include "../../util/logger.h"
#include "ipfix_converter.h"

int ipfix_converter_convert_packet( nf_data_loader::SFileInfo *psoDataLoader )
{
  int iRetVal = 0;
  uint8_t *pui8Data;
  SIPFIXHeaderSpecific soIPFIXHdr;
  size_t stDataSize;
  size_t stRead;

  stRead = data_loader_get_data( psoDataLoader, sizeof( soIPFIXHdr ), &pui8Data );
  if ( stRead == sizeof( soIPFIXHdr ) ) {
    memcpy( &soIPFIXHdr, reinterpret_cast<SIPFIXHeaderSpecific*>( pui8Data ), sizeof( soIPFIXHdr ) );
    soIPFIXHdr.m_ui16Length = ntohs( soIPFIXHdr.m_ui16Length );
    soIPFIXHdr.m_ui32ExportTime = ntohl( soIPFIXHdr.m_ui32ExportTime );
    soIPFIXHdr.m_ui32SequenceNumber = ntohl( soIPFIXHdr.m_ui32SequenceNumber );
    soIPFIXHdr.m_ui32ObservDomainId = ntohl( soIPFIXHdr.m_ui32ObservDomainId );
#ifdef DEBUG
	std::string strTimeValue;
	nfc_dict_get_time_value( 1, static_cast< uint64_t >( soIPFIXHdr.m_ui32ExportTime ), &strTimeValue );
	logger_message( 9, "%s: length: %u; export time: %s; sequence number: %u; observation domain: %#010x\n", __FUNCTION__, soIPFIXHdr.m_ui16Length, strTimeValue.c_str(), soIPFIXHdr.m_ui32SequenceNumber, soIPFIXHdr.m_ui32ObservDomainId );
#endif
  } else {
    return EINVAL;
  }

  stDataSize = soIPFIXHdr.m_ui16Length - sizeof( SIPFIXHeader );

  stRead = data_loader_get_data( psoDataLoader, stDataSize, &pui8Data );
  if ( stRead == stDataSize ) {
    iRetVal = ifpix_parse_packet( &soIPFIXHdr, pui8Data, stRead );
  } else {
    iRetVal = EINVAL;
  }

  return iRetVal;
}
