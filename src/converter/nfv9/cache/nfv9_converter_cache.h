#ifndef __NFV9_CONVERTER_CACHE_H__
#define __NFV9_CONVERTER_CACHE_H__

#include <stdint.h>
#include <vector>
#include <unistd.h>

#include "../data_types/nfv9_data_types.h"

struct SNFV9TemplateCache;
struct SNFV9TemplateFieldList;

/* инициализация/деинициализация */
int nfv9_converter_cache_init();
void nfv9_converter_cache_fin();

/* работа с объектом памяти*/
SNFV9TemplateCache * nfv9_converter_create_template( uint32_t p_ui32SourceId, uint16_t p_ui16TemplateId );
void nfv9_converter_add_template_field( SNFV9TemplateCache *p_psoNFV9TemplateCache, SNFV9Field &p_soNFV9Field );

/* работа со списком шаблонов */
int nfv9_converter_add_template( SNFV9TemplateCache *p_psoNFV9TemplateCache );

SNFV9TemplateFieldList * nfv9_converter_get_field_list( uint32_t p_ui32SourceId, uint16_t p_ui16TemplateId );
uint16_t nfv9_converter_get_field_count( SNFV9TemplateFieldList * p_psoFieldList );
size_t nfv9_converter_get_data_set_length( SNFV9TemplateFieldList * p_psoFieldList );
int nfv9_converter_template_cache_get_field_info( SNFV9TemplateFieldList * p_psoFieldList, uint16_t p_ui16FieldIndex, SNFV9Field *p_psoField, off_t *p_pstOffset );

#endif /* __NFV9_CONVERTER_CACHE_H__ */
