#include "../converter/ipfix/mapper/ipfix_mapper.h"
#include "netflow_converter_mapper.h"

void nf_converter_field_name_mapper( const uint16_t p_ui16Version, const char *p_pszFieldName, std::string *p_pstrMappedFieldName, uint32_t *p_pui32Divider )
{
  int iFnRes = -1;

  switch ( p_ui16Version ) {
    case 10:
      iFnRes = ipfix_field_name_mapper( p_pszFieldName, p_pstrMappedFieldName, p_pui32Divider );
      break;
  }

  if ( 0 == iFnRes ) {
  } else {
    *p_pstrMappedFieldName = p_pszFieldName;
    p_pui32Divider = 0;
  }
}
