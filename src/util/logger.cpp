/* для write */
#include <unistd.h>
/* для vasprintf */
#include <stdio.h>
/* для free */
#include <stdlib.h>

#include <stdarg.h>

#include "logger.h"

static int g_iLogLevel;

int logger_init( const char *p_pszFileName, int p_iLogLevel )
{
  int iRetVal = 0;

  g_iLogLevel = p_iLogLevel;

  return iRetVal;
}

void logger_deinit()
{
}

void logger_message( int p_iLogLevel, const char *p_pszMessage, ... )
{
  if ( g_iLogLevel >= p_iLogLevel ) {
  } else {
    return;
  }

  va_list ap;
  int iFnRes;
  char *pszMessage = NULL;

  va_start( ap, p_pszMessage );
  iFnRes = vasprintf( &pszMessage, p_pszMessage, ap );
  va_end( ap );

  if ( 0 < iFnRes ) {
    write( STDOUT_FILENO, pszMessage, iFnRes );
    write( STDOUT_FILENO, "\r\n", 2 );
    free( pszMessage );
  }
}
