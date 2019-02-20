#ifndef __NFC_OUTPUT_TEMPLATE__
#define __NFC_OUTPUT_TEMPLATE__

#include <string>

struct SNFCOutputTemplate;

void nfc_outputtemplate_field_add( const char *p_pszFieldName );

SNFCOutputTemplate * nfc_outputtemplate_object_create();

void nfc_outputtemplate_object_erase( SNFCOutputTemplate * p_psoOutputTemplate );
void nfc_outputtemplate_object_add( SNFCOutputTemplate * p_psoOutputTemplate, std::string &p_strAttrName, std::string &p_strAttrValue );
void nfc_outputtemplate_object_output( SNFCOutputTemplate * p_psoOutputTemplate );
void nfc_outputtemplate_object_delete( SNFCOutputTemplate * p_psoOutputTemplate );

void nfc_outputtemplate_output_header();

#endif /* __NFC_OUTPUT_TEMPLATE__ */
