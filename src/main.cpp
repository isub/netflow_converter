#include <pthread.h>

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
  int iFnRes;
  pthread_t * pmtConverterThread = NULL;

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

  pmtConverterThread = new pthread_t[ g_psoOpt->m_soConverter.m_ui32ThreadCount ];

  for ( int i = 0; i < g_psoOpt->m_soConverter.m_ui32ThreadCount; ++i ) {
    pthread_create( &pmtConverterThread[ i ], NULL, netflow_converter_start, NULL );
  }

  for ( int i = 0; i < g_psoOpt->m_soConverter.m_ui32ThreadCount; ++i ) {
    pthread_join( pmtConverterThread[ i ], NULL );
  }

  delete[ ] pmtConverterThread;
  pmtConverterThread = NULL;

  logger_message( 0, "operated files: %d", data_loader_get_total_count_of_opened_files() );

  file_list_fini();

  ipfix_converter_cache_fin();

  logger_deinit();
  
  return iRetVal;
}
