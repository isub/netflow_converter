#ifndef __IPFIX_DICTIONARY_H__
#define __IPFIX_DICTIONARY_H__

#include <stdint.h>
#include <string>

#define CHECK_DICT(__call__) (__call__)

namespace ipfix {
  enum EDataType {
    undefinedDataType = 0,
    unsigned8, unsigned16, unsigned32, unsigned64,
    signed8, signed16, signed32, signed64,
    float32, float64,
    boolean,
    macAddress,
    octetArray,
    string,
    dateTimeSeconds, dateTimeMilliseconds, dateTimeMicroseconds, dateTimeNanoseconds,
    ipv4Address, ipv6Address,
    basicList,
    subTemplateList, subTemplateMultiList
  };

  enum EDataSemantic {
    undefinedDataSemantic = 0,
    defaultDataSemantic,
    quantity,
    totalCounter,
    deltaCounter,
    identifier,
    flags,
    list,
    snmpCounter,
    snmpGauge
  };

  enum EStatus {
    undefinedStatus = 0,
    current,
    deprecated
  };

  enum EUnits {
    undefinedUnit = 0,
    fouroctetwords,
    bits,
    entries,
    flows,
    frames,
    hops,
    inferred,
    messages,
    seconds, milliseconds, microseconds, nanoseconds,
    octets, packets,
    ports,
  };
}

int ipfix_dictionary_add(
  const char *p_pszIEIdRange,
  const uint32_t p_ui32EnterpriseNumber,
  const char *p_pszName,
  ipfix::EDataType p_eDataType,
  ipfix::EDataSemantic p_eDataSemantic,
  ipfix::EStatus p_eStatus,
  const char *p_pszDescription,
  ipfix::EUnits p_eUnits,
  const char *p_pszValueRange );

int ipfix_dictionary_convert_value(
  const uint16_t p_ui16IEId,
  const uint32_t p_ui32EntNumb,
  const uint8_t *p_puiData,
  const size_t p_stLength,
  std::string &p_strAttrName,
  std::string *p_pstrAttrValue,
  std::string *p_pstrValueUnits,
  uint64_t *p_pui64ForFilter );

#endif /* __IPFIX_DICTIONARY_H__ */
