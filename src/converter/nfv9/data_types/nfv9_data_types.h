#ifndef __NFV9_DATA_TYPES_H__
#define __NFV9_DATA_TYPES_H__

#include "../../netflow_data_types.h"
#include "../../../data/data_loader.h"

#pragma pack(push,1)

struct SNFV9Field {
	uint16_t m_ui16Type;
	uint16_t m_ui16Length;
};

struct SNFV9HeaderSpecific {
	uint16_t m_ui16Count;
	uint32_t m_ui32SysUpTime;
	uint32_t m_ui32UNIXSec;
	uint32_t m_ui32SeqNumber;
	uint32_t m_ui32SourceId;
};
	
struct SNFV9Header {
	SNetFlowVersion m_soVersion;
	SNFV9HeaderSpecific m_soSpecific;
};

#pragma pack(pop)

int nfv9_parse_packet( const SNFV9HeaderSpecific *p_psoHdr, nf_data_loader::SFileInfo *p_psoDataLoader );

#endif /* __NFV9_DATA_TYPES_H__ */
