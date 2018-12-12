#include <getopt.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "options.h"
#include "logger.h"

#define OPTIONS_DEFAULT_TIME_FORMAT "%d.%m.%Y %H:%M:%S"
#define OPTIONS_DEFAULT_THREAD_COUNT 1

static SOptions g_soOptions;
SOptions *g_psoOpt;

static void options_init();
static int options_str_to_time( const char *p_pszStr, time_t *p_ptmTime );
static int options_str_to_addr( const char *p_pszStr, uint32_t *p_pui32Addr );

int options_read_options( int argc, char *argv[ ] )
{
  int iRetVal = 0;
  int iOptInd;
  int iFnRes;

  options_init();

  option msoOption[ ] = {
    { "verbosity",             required_argument, NULL, 0 },
    { "dir-data",              required_argument, NULL, 0 },
    { "dir-recursive",         no_argument,       NULL, 0 },
    { "filter-time-format",    required_argument, NULL, 0 },
    { "filter-time-start",     required_argument, NULL, 0 },
    { "filter-time-stop",      required_argument, NULL, 0 },
    { "filter-time-tolerance-before", required_argument, NULL, 0 },
    { "filter-time-tolerance-after", required_argument, NULL, 0 },
    { "filter-addr-src",       required_argument, NULL, 0 },
    { "filter-addr-dst",       required_argument, NULL, 0 },
    { "output-format-date",    required_argument, NULL, 0 },
    { "output-format-date-add",required_argument, NULL, 0 },
    { "converter-thread-count",required_argument, NULL, 0 },
    { NULL, 0, NULL, 0}
  };

  while ( -1 != ( iFnRes = getopt_long( argc, argv, "", msoOption, &iOptInd ) ) ) {
    switch ( iFnRes ) {
      case 0:
        switch ( iOptInd ) {
          case 0:
            g_soOptions.m_iVerbosityLevel = atol( optarg );
            break;
          case 1:
            g_soOptions.m_soDataDir.m_setDirList.insert( std::string( optarg ) );
            break;
          case 2:
            g_soOptions.m_soDataDir.m_iRecursive = 1;
            break;
          case 3:
            g_soOptions.m_soFilterTime.m_strFormat = optarg;
            break;
          case 4:
            iRetVal = options_str_to_time( optarg, &g_soOptions.m_soFilterTime.m_tmFlowStart );
            break;
          case 5:
            iRetVal = options_str_to_time( optarg, &g_soOptions.m_soFilterTime.m_tmFlowStop );
            break;
          case 6:
            g_soOptions.m_soFilterTime.m_tmToleranceBefore = atol( optarg );
            break;
          case 7:
            g_soOptions.m_soFilterTime.m_tmToleranceAfter = atol( optarg );
            break;
          case 8:
            iRetVal = options_str_to_addr( optarg, &g_soOptions.m_soFilterAddress.m_ui32AddrSrc );
            break;
          case 9:
            iRetVal = options_str_to_addr( optarg, &g_soOptions.m_soFilterAddress.m_ui32AddrDst );
            break;
          case 10:
            g_soOptions.m_soOutputFormat.m_strOutputFormatDate = optarg;
            break;
          case 11:
            g_soOptions.m_soOutputFormat.m_strOutputFormatDateAdd = optarg;
            break;
          case 12:
            g_soOptions.m_soConverter.m_ui32ThreadCount = atol( optarg );
            if ( 0 != g_soOptions.m_soConverter.m_ui32ThreadCount ) {
            } else {
              g_soOptions.m_soConverter.m_ui32ThreadCount = OPTIONS_DEFAULT_THREAD_COUNT;
            }
            break;
        }
        break;
      case '?':
        /* unknown attribute */
        break;
    }
    if ( 0 != iRetVal ) {
      break;
    }
  }

  return iRetVal;
}

void options_init()
{
  g_soOptions.m_iVerbosityLevel = 0;

  g_soOptions.m_soDataDir.m_iRecursive = 0;

  g_soOptions.m_soFilterTime.m_tmFlowStart = 0;
  g_soOptions.m_soFilterTime.m_tmFlowStop = static_cast<time_t>( -1 );
  g_soOptions.m_soFilterTime.m_tmToleranceBefore = 0;
  g_soOptions.m_soFilterTime.m_tmToleranceAfter = 0;

  g_soOptions.m_soFilterAddress.m_ui32AddrSrc = 0;
  g_soOptions.m_soFilterAddress.m_ui32AddrDst = 0;

  g_soOptions.m_soOutputFormat.m_strOutputFormatDate = OPTIONS_DEFAULT_TIME_FORMAT;
  g_soOptions.m_soOutputFormat.m_strOutputFormatDateAdd = "";

  g_soOptions.m_soConverter.m_ui32ThreadCount = OPTIONS_DEFAULT_THREAD_COUNT;

  g_psoOpt = &g_soOptions;
}

int options_str_to_time( const char *p_pszStr, time_t *p_ptmTime )
{
  int iRetVal = 0;
  tm soTm;
  const char *pszTimeFormat;
  const char *pszFnRes;

  if ( 0 == g_soOptions.m_soFilterTime.m_strFormat.length() ) {
    pszTimeFormat = OPTIONS_DEFAULT_TIME_FORMAT;
  } else {
    pszTimeFormat = g_soOptions.m_soFilterTime.m_strFormat.c_str();
  }

  memset( &soTm, 0, sizeof( soTm ) );
  pszFnRes = strptime( p_pszStr, pszTimeFormat, &soTm );
  if ( NULL != pszFnRes && *pszFnRes == '\0' ) {
    *p_ptmTime = mktime( &soTm );
  } else {
    logger_message( 0, "can't to recognize date value '%s' by using format '%s' at '%s'", p_pszStr, pszTimeFormat, pszFnRes );

    iRetVal = -1;
  }

  return iRetVal;
}

int options_str_to_addr( const char *p_pszStr, uint32_t *p_pui32Addr )
{
  int iRetVal = 0;
  in_addr soAddr;

  if ( 0 != inet_aton( p_pszStr, &soAddr ) ) {
    *p_pui32Addr = soAddr.s_addr;
  } else {
    logger_message( 0, "can't to recognize ip-addres value '%s'", p_pszStr );
    iRetVal = -1;
  }

  return iRetVal;
}
