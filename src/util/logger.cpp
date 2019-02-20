#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>

#include "logger.h"

static int g_iLogLevel;
static FILE *g_psoFile;

void logger_set_verbosity( int p_iVerbosity )
{
	g_iLogLevel = p_iVerbosity;
}

int logger_open_log( const char *p_pszFileName )
{
	int iRetVal = 0;

	g_psoFile = fopen( p_pszFileName, "w" );
	if( NULL != g_psoFile ) {
	} else {
		iRetVal = errno;
	}

	return iRetVal;
}

void logger_deinit()
{
	if( NULL != g_psoFile ) {
		fflush( g_psoFile );
		fclose( g_psoFile );
	}
}

void logger_message( int p_iLogLevel, const char *p_pszMessage, ... )
{
	if( g_iLogLevel >= p_iLogLevel ) {
	} else {
		return;
	}

	va_list ap;
	int iFnRes;
	char *pszMessage = NULL;

	va_start( ap, p_pszMessage );
	iFnRes = vasprintf( &pszMessage, p_pszMessage, ap );
	va_end( ap );

	if( 0 < iFnRes ) {
		if( NULL != g_psoFile ) {
			fwrite( pszMessage, iFnRes, 1, g_psoFile );
		} else {
			fwrite( pszMessage, iFnRes, 1, stdout );
		}
		free( pszMessage );
	}
}
