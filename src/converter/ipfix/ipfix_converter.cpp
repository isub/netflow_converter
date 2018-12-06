/* для memcpy */
#include <string.h>
/* для ntohl */
#include <netinet/in.h>

#include <errno.h>

#include "../../data/data_loader.h"
#include "../netflow_data_types.h"
#include "ipfix_data_types.h"
#include "../../util/logger.h"
#include "ipfix_converter.h"

int ipfix_converter_convert_packet()
{
  int iRetVal = 0;
  uint8_t *pui8Data;
  SIPFIXHeaderSpecific soIPFIXHdr;
  size_t stDataSize;
  size_t stRead;

  stRead = data_loader_get_data( sizeof( soIPFIXHdr ), &pui8Data );
  if ( stRead == sizeof( soIPFIXHdr ) ) {
    memcpy( &soIPFIXHdr, reinterpret_cast<SIPFIXHeaderSpecific*>( pui8Data ), sizeof( soIPFIXHdr ) );
    soIPFIXHdr.m_ui16Length = ntohs( soIPFIXHdr.m_ui16Length );
    soIPFIXHdr.m_ui32ExportTime = ntohl( soIPFIXHdr.m_ui32ExportTime );
    soIPFIXHdr.m_ui32SequenceNumber = ntohl( soIPFIXHdr.m_ui32SequenceNumber );
    soIPFIXHdr.m_ui32ObservDomainId = ntohl( soIPFIXHdr.m_ui32ObservDomainId );
    logger_message( 9, "%s: length: %u; export time: %u; sequence number: %u; observation domain: %u", __FUNCTION__, soIPFIXHdr.m_ui16Length, soIPFIXHdr.m_ui32ExportTime, soIPFIXHdr.m_ui32SequenceNumber, soIPFIXHdr.m_ui32ObservDomainId );
  } else {
    return EINVAL;
  }

  stDataSize = soIPFIXHdr.m_ui16Length - sizeof( SIPFIXHeader );

  stRead = data_loader_get_data( stDataSize, &pui8Data );
  if ( stRead == stDataSize ) {
    iRetVal = ifpix_parse_packet( &soIPFIXHdr, pui8Data, stRead );
  } else {
    iRetVal = EINVAL;
  }

  return 0;
}
