#include "data/data_loader.h"
#include "converter/netflow_converter.h"
#include "util/logger.h"
#include "util/options.h"
#include "filter/filter_time.h"
#include "converter/ipfix/ipfix_converter_cache.h"
#include "converter/ipfix/dict/iana/ipfix_dict_iana.h"

int main( int argc, char *argv [] )
{
	int iRetVal = 0;

  if ( 0 == ( iRetVal = options_read_options( argc, argv ) ) ) {
  } else {
    return iRetVal;
  }

  filter_time_init( g_psoOpt->m_soFilterTime.m_tmFlowStart, g_psoOpt->m_soFilterTime.m_tmFlowStop, g_psoOpt->m_soFilterTime.m_tmToleranceBefore, g_psoOpt->m_soFilterTime.m_tmToleranceAfter );

  iRetVal = logger_init( "log.log", g_psoOpt->m_iVerbosityLevel );

  iRetVal = ipfix_converter_cache_init();

  iRetVal = ipfix_dict_iana_init();

  iRetVal = file_list_init( &g_psoOpt->m_soDataDir.m_setDirList, g_psoOpt->m_soDataDir.m_iRecursive );

  iRetVal = netflow_converter_init( "result.txt" );

  iRetVal = netflow_converter_start();

  ipfix_converter_cache_fin();

  logger_deinit();
  
  return iRetVal;
}
