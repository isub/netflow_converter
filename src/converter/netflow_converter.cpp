#include <stddef.h>

#include "../data/file_list.h"
#include "../data/data_loader.h"
#include "netflow_data_types.h"
#include "netflow_converter.h"
#include "ipfix/ipfix_converter.h"
#include "../util/logger.h"

int netflow_converter_init( const char *p_pszResultFileName )
{
  int iRetVal = 0;

  logger_message( 1, "converter is initialized" );

  return iRetVal;
}

int netflow_converter_start()
{
  int iRetVal = 0;
  SFileInfo *psoFileInfo;
  size_t stRead;
  uint8_t *pmuiData;
  uint16_t ui16VersionNumber;
  int iFnRes;

  while ( 0 == file_list_go_to_next_file() ) {
    psoFileInfo = file_list_get_current_file_info();
    if ( NULL != psoFileInfo ) {
    } else {
      continue;
    }
    if ( 0 == data_loader_open( psoFileInfo ) ) {
    } else {
      continue;
    }
    while ( 0 != ( stRead = data_loader_get_data( sizeof( SNetFlowVersion ), &pmuiData ) ) ) {
      if ( stRead == sizeof( SNetFlowVersion ) ) {
        ui16VersionNumber = netflow_data_get_version_number( pmuiData );
      } else {
        continue;
      }
      switch ( ui16VersionNumber ) {
        case 9:
          break;
        case 10:
          iFnRes = ipfix_converter_convert_packet();
          break;
        default:
          iFnRes = -1;
          break;
      }
      if ( 0 == iFnRes ) {
      } else {
        break;
      }
    }
    data_loader_close();
  }

  logger_message( 0, "operated files: %d", data_loader_get_total_count_of_opened_files() );

  return iRetVal;
}
