#ifndef __NFC_DICT_H__
#define __NFC_DICT_H__

#include <stdint.h>
#include <string>

#include "../util/logger.h"

#define CHECK_DICT(__call__) { \
						int __iRetVal__ = (__call__); \
						if( 0 == __iRetVal__ ) {} else { return __iRetVal__; } \
					}

int nfc_dict_get_integer_value( const uint8_t *p_puiData, const size_t p_stDataLength, uint64_t *p_pui64Value );
int nfc_dict_get_inetaddr_value( int p_iAddressFamily, const uint8_t *p_puiData, const size_t p_stDataLength, std::string *p_pstrValue, uint64_t *p_pui64ForFilter );
int nfc_dict_get_time_value( uint32_t p_ui32Div, uint64_t p_ui64Time, std::string *p_pstrValue );

#endif /* __NFC_DICT_H__ */
