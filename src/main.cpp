#include <pthread.h>

#include "data/data_loader.h"
#include "converter/netflow_converter.h"
#include "util/logger.h"
#include "util/options.h"
#include "filter/filter_time.h"
#include "converter/ipfix/ipfix_converter_cache.h"
#include "converter/ipfix/dict/iana/ipfix_dict_iana.h"
#include "converter/nfv9/dict/nfv9_dict.h"
#include "converter/nfc_output_template.h"

int main( int argc, char *argv [] )
{
	int iRetVal = 0;
	pthread_t * pmtConverterThread = NULL;

	if( 0 == ( iRetVal = options_read_options( argc, argv ) ) ) {
	} else {
		return iRetVal;
	}

	if( 0 != g_psoOpt->m_soConverter.m_strConfFile.length() ) {
		if( 0 == conf_handle( const_cast< char* >( g_psoOpt->m_soConverter.m_strConfFile.c_str() ) ) ) {
		} else {
			return -1;
		}
	}

	filter_time_init( g_psoOpt->m_soFilterTime.m_tmFlowStart, g_psoOpt->m_soFilterTime.m_tmFlowStop, g_psoOpt->m_soFilterTime.m_tmToleranceBefore, g_psoOpt->m_soFilterTime.m_tmToleranceAfter );

	if( 0 != g_psoOpt->m_soConverter.m_strResultFile.size() ) {
		iRetVal = logger_open_log( g_psoOpt->m_soConverter.m_strResultFile.c_str() );
		if( 0 == iRetVal ) {
		} else {
			logger_message( 0, "can not to open file '%s'\n", g_psoOpt->m_soConverter.m_strResultFile.c_str() );
			return iRetVal;
		}
	}

	iRetVal = ipfix_converter_cache_init();

	iRetVal = ipfix_dict_iana_init();
	iRetVal = nfv9_dict_init();

	iRetVal = file_list_init( &g_psoOpt->m_soDataDir.m_setDirList, g_psoOpt->m_soDataDir.m_iRecursive == -1 ? 0 : g_psoOpt->m_soDataDir.m_iRecursive );

	iRetVal = netflow_converter_init();

	if( 0 != g_psoOpt->m_soConverter.m_ui32ThreadCount ) {
	} else {
		g_psoOpt->m_soConverter.m_ui32ThreadCount = OPTIONS_DEFAULT_THREAD_COUNT;
	}

	nfc_outputtemplate_output_header();

	pmtConverterThread = new pthread_t[g_psoOpt->m_soConverter.m_ui32ThreadCount];

	for( uint32_t i = 0; i < g_psoOpt->m_soConverter.m_ui32ThreadCount; ++i ) {
		pthread_create( &pmtConverterThread[i], NULL, netflow_converter_start, NULL );
	}

	for( uint32_t i = 0; i < g_psoOpt->m_soConverter.m_ui32ThreadCount; ++i ) {
		pthread_join( pmtConverterThread[i], NULL );
	}

	delete [] pmtConverterThread;
	pmtConverterThread = NULL;

	logger_message( 0, "operated files: %d\n", data_loader_get_total_count_of_opened_files() );

	file_list_fini();

	ipfix_converter_cache_fin();

	logger_deinit();

	return iRetVal;
}
