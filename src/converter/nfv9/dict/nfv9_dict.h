#ifndef __NFV9_DICT_H__
#define __NFV9_DICT_H__

#include <stdint.h>
#include <string>

#include "../data_types/nfv9_data_types.h"

int nfv9_dict_init();

int nfv9_dict_convert_value(
	const SNFV9HeaderSpecific *p_psoHdr,
	const uint16_t p_ui16IEId,
	const uint8_t *p_puiData,
	const size_t p_stLength,
	std::string &p_strAttrName,
	std::string *p_pstrAttrValue,
	std::string *p_pstrValueUnits,
	uint64_t *p_pui64ForFilter );

const char * nfv9_dict_get_iename( const uint16_t p_ui16IEId );

#endif /* __NFV9_DICT_H__ */
