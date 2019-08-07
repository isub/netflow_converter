#include <stddef.h>
#include <pthread.h>

#include "../data/file_list.h"
#include "../data/data_loader.h"
#include "../util/logger.h"
#include "ipfix/ipfix_converter.h"
#include "netflow_data_types.h"
#include "netflow_converter.h"

#include "nfv9/nfv9_converter.h"

int netflow_converter_init()
{
	int iRetVal = 0;

	logger_message( 1, "converter is initialized\n" );

	return iRetVal;
}

void * netflow_converter_start( void * )
{
	nf_file_list::SFileInfo *psoFileListInfo;
	nf_data_loader::SFileInfo *psoDataLoader;
	size_t stRead;
	uint8_t *pmuiData;
	uint16_t ui16VersionNumber;
	int iFnRes = 0;

	while( NULL != ( psoFileListInfo = file_list_get_next_file_info() ) ) {
		logger_message( 5, "file '%s' is operating\n", file_list_get_file_name( psoFileListInfo )->c_str() );
		psoDataLoader = data_loader_open( psoFileListInfo );
		if( NULL != psoDataLoader ) {
		} else {
			continue;
		}
		while( 0 != ( stRead = data_loader_get_data( psoDataLoader, sizeof( SNetFlowVersion ), &pmuiData ) ) ) {
			if( stRead == sizeof( SNetFlowVersion ) ) {
				ui16VersionNumber = netflow_data_get_version_number( pmuiData );
			} else {
				continue;
			}
			switch( ui16VersionNumber ) {
			case 9:
				iFnRes = nfv9_converter_convert_packet( psoDataLoader );
				break;
			case 10:
				iFnRes = ipfix_converter_convert_packet( psoDataLoader );
				break;
			default:
				logger_message( 0, "file '%s': unsupported NetFlow version: %d\n", file_list_get_file_name( psoFileListInfo )->c_str(), ui16VersionNumber );
				iFnRes = -1;
				break;
			}
			if( 0 == iFnRes ) {
			} else {
				logger_message( 5, "file '%s' is operated with errors\n", file_list_get_file_name( psoFileListInfo )->c_str() );
				break;
			}
		}
		if( 0 == iFnRes ) {
			logger_message( 5, "file '%s' is operated successfully\n", file_list_get_file_name( psoFileListInfo )->c_str() );
		}
		data_loader_close( psoDataLoader );
	}

	pthread_exit( NULL );
}
