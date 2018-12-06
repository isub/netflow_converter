#ifndef __IPFIX_CONVERTER_H__
#define __IPFIX_CONVERTER_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplus_plus
extern "C" {
#endif

  /*
   *  запуск конвертора
   */
  int ipfix_converter_convert_packet();


#ifdef __cplus_plus
}
#endif

#endif /* __IPFIX_CONVERTER_H__ */
