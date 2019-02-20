#ifndef __IPFIX_DATA_TYPES_H__
#define __IPFIX_DATA_TYPES_H__

#include <stdint.h>
#include <stddef.h>

#include "../../netflow_data_types.h"

#pragma pack(push,1)

struct SIPFIXFieldCommon {
  union {
    struct {
      uint16_t m_ui16InformationElementId : 15;
      uint16_t m_ui16EnterpriseBit : 1;
    };
    uint16_t m_ui16FullInfoElementId;
  } m_unionInfoElement;
  uint16_t m_ui16FieldLength;
};

struct SIPFIXField {
  SIPFIXFieldCommon m_soFieldCommon;
  uint32_t m_ui32EnterpriseNumber;
};

struct SIPFIXHeaderSpecific {
  uint16_t m_ui16Length;
  uint32_t m_ui32ExportTime;
  uint32_t m_ui32SequenceNumber;
  uint32_t m_ui32ObservDomainId;
};

struct SIPFIXHeader {
  SNetFlowVersion m_soVersion;
  SIPFIXHeaderSpecific m_soSpecific;
};

#pragma pack(pop)

int ifpix_parse_packet( SIPFIXHeaderSpecific *p_psoHdr, uint8_t *p_puiData, size_t p_stSize );

#endif /* __IPFIX_DATA_TYPES_H__ */
