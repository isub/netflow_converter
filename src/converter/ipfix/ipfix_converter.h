#ifndef __IPFIX_CONVERTER_H__
#define __IPFIX_CONVERTER_H__

#include "../../data/data_loader.h"

/*
*  запуск конвертора
*/
int ipfix_converter_convert_packet( nf_data_loader::SFileInfo *psoDataLoader );

#endif /* __IPFIX_CONVERTER_H__ */
