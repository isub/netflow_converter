#include <string.h>

#include "../util/options.h"
#include "../mapper/netflow_converter_mapper.h"
#include "filter_time.h"
#include "nf_filter_data.h"

bool nf_filter_data( const char *p_pszFieldName, uint64_t p_ui64Data, uint32_t p_ui32Divider )
{
  bool bRetVal = true;

  if ( 0 == strcmp( p_pszFieldName, NF_MAPPER_FLOW_START ) ) {
    /* проверяем время начала потока */
    uint32_t ui32Value;

    if ( 1 != p_ui32Divider ) {
      ui32Value = static_cast<uint32_t>( p_ui64Data / p_ui32Divider );
    } else {
      ui32Value = static_cast<uint32_t>( p_ui64Data );
    }

    /* проверяем начало временного интервала */
    if ( 0 != g_psoOpt->m_soFilterTime.m_tmFlowStart ) {
      if ( g_psoOpt->m_soFilterTime.m_tmFlowStart <= ui32Value ) {
      } else {
        return false;
      }
    }
    /* проверяем конец временного интервала */
    if ( 0 != g_psoOpt->m_soFilterTime.m_tmFlowStop ) {
      if ( g_psoOpt->m_soFilterTime.m_tmFlowStop > ui32Value ) {
      } else {
        return false;
      }
    }

    return true;
  } else if ( 0 == strcmp( p_pszFieldName, NF_MAPPER_SRC_IPV4 ) ) {
    /* проверяем source ipv4 address */
    uint32_t ui32Value;

    ui32Value = static_cast<uint32_t>( p_ui64Data );

    if ( 0 != g_psoOpt->m_soFilterAddress.m_ui32AddrSrc ) {
      if ( g_psoOpt->m_soFilterAddress.m_ui32AddrSrc != ui32Value ) {
        return false;
      } else {
        return true;
      }
    }

    return true;
  } else if ( 0 == strcmp( p_pszFieldName, NF_MAPPER_DST_IPV4 ) ) {
    /* проверяем destination ipv4 address */
    uint32_t ui32Value;

    ui32Value = static_cast<uint32_t>( p_ui64Data );

    if ( 0 != g_psoOpt->m_soFilterAddress.m_ui32AddrDst ) {
      if ( g_psoOpt->m_soFilterAddress.m_ui32AddrDst != ui32Value ) {
        return false;
      } else {
        return true;
      }
    }

    return true;
  }

  return bRetVal;
}
