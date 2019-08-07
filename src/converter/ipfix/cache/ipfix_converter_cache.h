#ifndef __IPFIX_CONVERTER_CACHE_H__
#define __IPFIX_CONVERTER_CACHE_H__

#include <stdint.h>
#include <vector>
#include <unistd.h>

#include "../data_types/ipfix_data_types.h"

struct SIPFIXTemplateCache;
struct SIPFIXTemplateFieldList;

/* инициализация/деинициализация */
int ipfix_converter_cache_init();
void ipfix_converter_cache_fin();

/* работа с объектом памяти*/
SIPFIXTemplateCache * ipfix_converter_create_template( const uint32_t p_ui32ObservDomainId, const uint16_t p_ui16TemplateId );
SIPFIXTemplateFieldList * ipfix_converter_create_template_fieldList();
void ipfix_converter_add_template_field( SIPFIXTemplateFieldList *p_psoIPFIXTemplateFieldList, SIPFIXField &p_soIPFIXField );

/* работа со списком шаблонов */
int ipfix_converter_add_template( SIPFIXTemplateCache *p_psoIPFIXTemplateCache, SIPFIXTemplateFieldList *p_psoFieldList );

SIPFIXTemplateFieldList * ipfix_converter_get_field_list( uint32_t p_ui32ObservDomainId, uint16_t p_ui16TemplateId );
uint16_t ipfix_converter_get_field_count( SIPFIXTemplateFieldList * p_psoFieldList );
size_t ipfix_converter_get_data_set_length( SIPFIXTemplateFieldList * p_psoFieldList );
int ipfix_converter_template_cache_get_field_info( SIPFIXTemplateFieldList * p_psoFieldList, uint16_t p_ui16FieldIndex, SIPFIXField *p_psoField, off_t *p_pstOffset );

#endif /* __IPFIX_CONVERTER_CACHE_H__ */
