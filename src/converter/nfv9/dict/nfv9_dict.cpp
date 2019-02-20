#include <map>
#include <errno.h>
#include <arpa/inet.h>

#include "../../nfc_dict.h"
#include "nfv9_dict.h"

enum ENFV9DataType {
	m_eUint,
	m_eIPAddr,
	m_eIPV6Addr,
	m_eMACAddr,
	m_eSysUpTime
};

struct SDictValue {
	ENFV9DataType m_eDataType;
	std::string m_strFieldName;
	uint16_t m_ui16FieldLen;
	std::string m_strFieldDescr;
	SDictValue( ENFV9DataType p_eDataType, const char *p_pszName, uint16_t p_ui16Len, const char *p_pszDescr ) :
		m_eDataType( p_eDataType ), m_strFieldName( p_pszName ), m_ui16FieldLen( p_ui16Len ), m_strFieldDescr( p_pszDescr )
	{
	}
};

static std::map<uint16_t, SDictValue> g_mapDict;

static int nfv9_dict_add(
	ENFV9DataType p_eDataType,
	const char *p_pszName,
	const uint16_t m_ui16FiledId,
	const uint16_t m_ui16FiledLen,
	const char *p_pszDescription );

int nfv9_dict_init()
{
	CHECK_DICT( nfv9_dict_add( m_eUint, "IN_BYTES", 1, 4, "Incoming counter with length N x 8 bits for the number of bytes associated with an IP Flow. By default N is 4" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "IN_PKTS",	2,	4,	"Incoming counter with length N x 8 bits for the number of packets associated with an IP Flow. By default N is 4" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "FLOWS",	3,	4,	"Number of Flows that were aggregated; by default N is 4" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "PROTOCOL",	4,	1,	"IP protocol byte" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "TOS",	5,	1,	"Type of service byte setting when entering the incoming interface" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "TCP_FLAGS",	6,	1,	"TCP flags; cumulative of all the TCP flags seen in this Flow" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "L4_SRC_PORT",	7,	2,	"TCP/UDP source port number (for example, FTP, Telnet, or equivalent)" ));
	CHECK_DICT( nfv9_dict_add( m_eIPAddr, "IPV4_SRC_ADDR",	8,	4,	"IPv4 source address" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "SRC_MASK",	9,	1,	"The number of contiguous bits in the source subnet mask (i.e., the mask in slash notation)" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "INPUT_SNMP",	10,	2,	"Input interface index. By default N is 2, but higher values can be used" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "L4_DST_PORT",	11,	2,	"TCP/UDP destination port number (for example, FTP, Telnet, or equivalent)" ));
	CHECK_DICT( nfv9_dict_add( m_eIPAddr, "IPV4_DST_ADDR",	12,	4,	"IPv4 destination address" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "DST_MASK",	13,	1,	"The number of contiguous bits in the destination subnet mask (i.e., the mask in slash notation)" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "OUTPUT_SNMP",	14,	2,	"Output interface index. By default N is 2, but higher values can be used" ));
	CHECK_DICT( nfv9_dict_add( m_eIPAddr, "IPV4_NEXT_HOP",	15,	4,	"IPv4 address of the next-hop router" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "SRC_AS",	16,	2,	"Source BGP autonomous system number where N could be 2 or 4. By default N is 2" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "DST_AS",	17,	2,	"Destination BGP autonomous system number where N could be 2 or 4. By default N is 2" ));
	CHECK_DICT( nfv9_dict_add( m_eIPAddr, "BGP_IPV4_NEXT_HOP",	18,	4,	"Next-hop router's IP address in the BGP domain" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "MUL_DST_PKTS",	19,	8,	"IP multicast outgoing packet counter with length N x 8 bits for packets associated with the IP Flow. By default N is 4" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "MUL_DST_BYTES",	20,	8,	"IP multicast outgoing Octet (byte) counter with length N x 8 bits for the number of bytes associated with the IP Flow. Bydefault N is 4" ));
	CHECK_DICT( nfv9_dict_add( m_eSysUpTime, "LAST_SWITCHED",	21,	4,	"sysUptime in msec at which the last packet of this Flow was switched" ));
	CHECK_DICT( nfv9_dict_add( m_eSysUpTime, "FIRST_SWITCHED",	22,	4,	"sysUptime in msec at which the first packet of this Flow was switched" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "OUT_BYTES",	23,	4,	"Outgoing counter with length N x 8 bits for the number of bytes associated  with an IP Flow. By default N is 4" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "OUT_PKTS",	24,	4,	"Outgoing counter with length N x 8 bits for the number of packets associated with an IP Flow. By default N is 4" ));
	CHECK_DICT( nfv9_dict_add( m_eIPV6Addr, "IPV6_SRC_ADDR",	27,	16,	"IPv6 source address" ));
	CHECK_DICT( nfv9_dict_add( m_eIPV6Addr, "IPV6_DST_ADDR",	28,	16,	"IPv6 destination address" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "IPV6_SRC_MASK",	29,	1,	"Length of the IPv6 source mask in contiguous bits" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "IPV6_DST_MASK",	30,	1,	"Length of the IPv6 destination mask in contiguous bits" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "IPV6_FLOW_LABEL",	31,	3,	"IPv6 flow label as per RFC 2460 definition" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "ICMP_TYPE",	32,	2,	"Internet Control Message Protocol (ICMP) packet type; reported as ICMP Type * 256 + ICMP code" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "MUL_IGMP_TYPE",	33,	1,	"Internet Group Management Protocol (IGMP) packet type" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "SAMPLING_INTERVAL",	34,	4,	"When using sampled NetFlow, the rate at which packets are sampled; for example, a value of 100 indicates that one of every hundred packets is sampled" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "SAMPLING_ALGORITHM",	35,	1,	"For sampled NetFlow platform-wide: 0x01 deterministic sampling 0x02 random sampling Use in connection with SAMPLING_INTERVAL" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "FLOW_ACTIVE_TIMEOUT",	36,	2,	"Timeout value (in seconds) for active flow entries in the NetFlow cache" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "FLOW_INACTIVE_TIMEOUT",	37,	2,	"Timeout value (in seconds) for inactive Flow entries in the NetFlow cache" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "ENGINE_TYPE",	38,	1,	"Type of Flow switching engine (route processor, linecard, etc...)" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "ENGINE_ID",	39,	1,	"ID number of the Flow switching engine" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "TOTAL_BYTES_EXP",	40,	4,	"Counter with length N x 8 bits for the number of bytes exported by the Observation Domain. By default N is 4" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "TOTAL_PKTS_EXP",	41,	4,	"Counter with length N x 8 bits for the number of packets exported by the Observation Domain. By default N is 4" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "TOTAL_FLOWS_EXP",	42,	4,	"Counter with length N x 8 bits for the number of Flows exported by the Observation Domain. By default N is 4" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "MPLS_TOP_LABEL_TYPE",	46,	1,	"MPLS Top Label Type: 0x00 UNKNOWN 0x01 TE-MIDPT 0x02 ATOM 0x03 VPN 0x04 BGP 0x05 LDP" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "MPLS_TOP_LABEL_IP_ADDR",	47,	4,	"Forwarding Equivalent Class corresponding to the MPLS Top Label" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "FLOW_SAMPLER_ID",	48,	1,	"Identifier shown in \"show flow-sampler\"" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "FLOW_SAMPLER_MODE",	49,	1,	"The type of algorithm used for sampling data: 0x02 random sampling Use in connection with FLOW_SAMPLER_MODE Packet interval at which to sample. Use in connection with FLOW_SAMPLER_MODE" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "DST_TOS",	55,	1,	"Type of Service byte setting when exiting outgoing interface" ));
	CHECK_DICT( nfv9_dict_add( m_eMACAddr, "SRC_MAC",	56,	6,	"Source MAC Address" ));
	CHECK_DICT( nfv9_dict_add( m_eMACAddr, "DST_MAC",	57,	6,	"Destination MAC Address" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "SRC_VLAN",	58,	2,	"Virtual LAN identifier associated with ingress interface" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "DST_VLAN",	59,	2,	"Virtual LAN identifier associated with egress interface" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "IP_PROTOCOL_VERSION",	60,	1,	"Internet Protocol Version Set to 4 for IPv4, set to 6 for IPv6. If not present in the template, then version 4 is assumed" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "DIRECTION",	61,	1,	"Flow direction: 0 - ingress flow 1 - egress flow" ));
	CHECK_DICT( nfv9_dict_add( m_eIPV6Addr, "IPV6_NEXT_HOP",	62,	16,	"IPv6 address of the next-hop router" ));
	CHECK_DICT( nfv9_dict_add( m_eIPV6Addr, "BGP_IPV6_NEXT_HOP",	63,	16,	"Next-hop router in the BGP domain" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "IPV6_OPTION_HEADERS",	64,	4,	"Bit-encoded field identifying IPv6 option headers found in the flow" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "MPLS_LABEL_1",	70,	3,	"MPLS label at position 1 in the stack" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "MPLS_LABEL_2",	71,	3,	"MPLS label at position 2 in the stack" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "MPLS_LABEL_3",	72,	3,	"MPLS label at position 3 in the stack" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "MPLS_LABEL_4",	73,	3,	"MPLS label at position 4 in the stack" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "MPLS_LABEL_5",	74,	3,	"MPLS label at position 5 in the stack" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "MPLS_LABEL_6",	75,	3,	"MPLS label at position 6 in the stack" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "MPLS_LABEL_7",	76,	3,	"MPLS label at position 7 in the stack" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "MPLS_LABEL_8",	77,	3,	"MPLS label at position 8 in the stack" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "MPLS_LABEL_9",	78,	3,	"MPLS label at position 9 in the stack" ));
	CHECK_DICT( nfv9_dict_add( m_eUint, "MPLS_LABEL_10", 79, 3, "MPLS label at position 10 in the stack" ));

	return 0;
}

int nfv9_dict_convert_value(
	const SNFV9HeaderSpecific *p_psoHdr,
	const uint16_t p_ui16IEId,
	const uint8_t *p_puiData,
	const size_t p_stLength,
	std::string &p_strAttrName,
	std::string *p_pstrAttrValue,
	std::string *p_pstrValueUnits,
	uint64_t *p_pui64ForFilter )
{
	int iRetVal = 0;
	std::map<uint16_t, SDictValue>::iterator iterDict;

	iterDict = g_mapDict.find( p_ui16IEId );
	if( iterDict != g_mapDict.end() ) {
	} else {
		return ENODATA;
	}

	p_strAttrName = iterDict->second.m_strFieldName;
	if( NULL != p_pstrAttrValue ) {
		p_pstrAttrValue->clear();
	}
	if( NULL != p_pstrValueUnits ) {
		p_pstrValueUnits->clear();
	}
	if( NULL != p_pui64ForFilter ) {
		*p_pui64ForFilter = 0;
	}

	if( NULL != p_pstrAttrValue ) {
		switch( iterDict->second.m_eDataType ) {
			case m_eUint:
			{
				uint64_t ui64Value;
				iRetVal = nfc_dict_get_integer_value( p_puiData, p_stLength, &ui64Value );
				if( 0 == iRetVal ) {
					*p_pstrAttrValue = std::to_string( ui64Value );
					*p_pui64ForFilter = ui64Value;
				}
				break;
			}
			case m_eIPAddr:
				iRetVal = nfc_dict_get_inetaddr_value( AF_INET, p_puiData, p_stLength, p_pstrAttrValue, p_pui64ForFilter );
				break;
			case m_eIPV6Addr:
				iRetVal = nfc_dict_get_inetaddr_value( AF_INET6, p_puiData, p_stLength, p_pstrAttrValue, p_pui64ForFilter );
				break;
			case m_eMACAddr:
				/* unsupported in current release */
				iRetVal = EINVAL;
				break;
			case m_eSysUpTime:
			{
				uint64_t ui64Time;

				iRetVal = nfc_dict_get_integer_value( p_puiData, p_stLength, &ui64Time );
				if( 0 == iRetVal ) {
					/* correction by System Uptime */
					ui64Time += ( static_cast< uint64_t >( p_psoHdr->m_ui32UNIXSec ) * 1000 );
					ui64Time -= p_psoHdr->m_ui32SysUpTime;
					iRetVal = nfc_dict_get_time_value( 1000, ui64Time, p_pstrAttrValue );
					if( NULL != p_pstrValueUnits ) {
						p_pstrValueUnits->assign( "ms" );
					}
					if( NULL != p_pui64ForFilter ) {
						*p_pui64ForFilter = ui64Time;
					}
				} else {
					iRetVal = EINVAL;
				}
			}
				break;
			default:
				iRetVal = EINVAL;
				break;
		}
	}

	return iRetVal;
}

static int nfv9_dict_add(
	ENFV9DataType p_eDataType,
	const char *p_pszName,
	const uint16_t p_ui16FiledId,
	const uint16_t p_ui16FiledLen,
	const char *p_pszDescription )
{
	int iRetVal = 0;

	std::pair< std::map< uint16_t, SDictValue >::iterator, bool > pairInsertResult;

	SDictValue soDictVal( p_eDataType, p_pszName, p_ui16FiledLen, p_pszDescription );

	pairInsertResult = g_mapDict.insert( std::pair<uint16_t, SDictValue>( p_ui16FiledId, soDictVal ) );
	logger_message( 2, "nfv9 dictionary: attribute id: %d; attribute name: %s\n", p_ui16FiledId, p_pszName );

	if( pairInsertResult.second ) {
	} else {
		return EALREADY;
	}

	return iRetVal;
}

const char * nfv9_dict_get_iename( const uint16_t p_ui16IEId )
{
	const char *pszIEName;

	std::map<uint16_t, SDictValue>::iterator iterDict = g_mapDict.find( p_ui16IEId );

	if( g_mapDict.end() != iterDict ) {
		pszIEName = iterDict->second.m_strFieldName.c_str();
	} else {
		pszIEName = NULL;
	}

	return pszIEName;
}
