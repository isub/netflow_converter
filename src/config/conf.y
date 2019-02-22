/*********************************************************************************************************
* Software License Agreement (BSD License)                                                               *
* Author: Sebastien Decugis <sdecugis@freediameter.net>							 *
*													 *
* Copyright (c) 2013, WIDE Project and NICT								 *
* All rights reserved.											 *
* 													 *
* Redistribution and use of this software in source and binary forms, with or without modification, are  *
* permitted provided that the following conditions are met:						 *
* 													 *
* * Redistributions of source code must retain the above 						 *
*   copyright notice, this list of conditions and the 							 *
*   following disclaimer.										 *
*    													 *
* * Redistributions in binary form must reproduce the above 						 *
*   copyright notice, this list of conditions and the 							 *
*   following disclaimer in the documentation and/or other						 *
*   materials provided with the distribution.								 *
* 													 *
* * Neither the name of the WIDE Project or NICT nor the 						 *
*   names of its contributors may be used to endorse or 						 *
*   promote products derived from this software without 						 *
*   specific prior written permission of WIDE Project and 						 *
*   NICT.												 *
* 													 *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED *
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A *
* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 	 *
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 	 *
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR *
* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF   *
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.								 *
*********************************************************************************************************/

/* Yacc extension's configuration parser.
 */

/* For development only : */
%debug 
%error-verbose

/* The parser receives the configuration file filename as parameter */
%parse-param {char * conffile}

/* Keep track of location */
%locations 
%pure-parser

%{
#include <string>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "../util/options.h"
#include "../util/logger.h"
#include "../converter/nfc_output_template.h"
#include "../mapper/netflow_converter_mapper.h"
#include "conf.tab.hpp"	/* bison is not smart enough to define the YYLTYPE before including this code, so... */

/* Forward declaration */
int yyparse (char * conffile);

/* Parse the configuration file */
int conf_handle (char * conffile)
{
	extern FILE * confin;
	int ret;

	logger_message( 9, "Parsing configuration file: %s\n", conffile);

	confin = fopen (conffile, "r");
	if (confin == NULL) {
		ret = errno;
		logger_message( 9, "Unable to open extension configuration file %s for reading: %s\n", conffile, strerror(ret));
		logger_message( 9, "Error occurred, message logged -- configuration file\n");
		return ret;
	}

	ret = yyparse (conffile);

	fclose (confin);

	if (ret != 0) {
		logger_message( 9, "Unable to parse the configuration file\n");
		return EINVAL;
	}

	return 0;
}

/* The Lex parser prototype */
int conflex (YYSTYPE *lvalp, YYLTYPE *llocp);

/* Function to report the errors */
void yyerror (YYLTYPE *ploc, char * conffile, char const *s)
{
	logger_message( 0, "Error in configuration parsing\n");

	if (ploc->first_line != ploc->last_line)
		logger_message( 0, "%s:%d.%d-%d.%d : %s\n", conffile, ploc->first_line, ploc->first_column, ploc->last_line, ploc->last_column, s);
	else if (ploc->first_column != ploc->last_column)
		logger_message( 0, "%s:%d.%d-%d : %s\n", conffile, ploc->first_line, ploc->first_column, ploc->last_column, s);
	else
		logger_message( 0, "%s:%d.%d : %s\n", conffile, ploc->first_line, ploc->first_column, s);
}

%}

/* Values returned by lex for token */
%union {
	char 		*string;	/* The string is allocated by strdup in lex.*/
	int		 integer;	/* Store integer values */
}

/* In case of error in the lexical analysis */
%token 		LEX_ERROR

/* Key words */
%token 	CONVERTER_VERBOSITY
%token 	DATA_DIR
%token 	DATA_DIR_RECURSIVE
%token 	FILTER_TIME_FORMAT
%token 	FILTER_TIME_START
%token 	FILTER_TIME_STOP
%token 	FILTER_TIME_TOLERANCE_BEFORE
%token 	FILTER_TIME_TOLERANCE_AFTER
%token 	FILTER_ADDR_SRC
%token 	FILTER_ADDR_DST
%token 	OUTPUT_FORMAT_DATE
%token 	OUTPUT_FORMAT_DATE_ADD
%token 	CONVERTER_THREAD_COUNT
%token 	CONVERTER_OUTPUT_TEMPLATE
%token	CONVERTER_MAPPER
%token	CONVERTER_RESUL_FILE

/* Tokens and types for routing table definition */
/* A (de)quoted string (malloc'd in lex parser; it must be freed after use) */
%token <string>	QSTRING

/* An integer value */
%token <integer> INTEGER


/* -------------------------------------- */
%%

	/* The grammar definition */
conffile:		/* empty grammar is OK */
			| conffile converterVerbosity
			| conffile dataDir
			| conffile dataDirRecursive
			| conffile filterTimeFormat
			| conffile filterTimeStart
			| conffile filterTimeStop
			| conffile filterTimeToleranceBefore
			| conffile filterTimeToleranceAfter
			| conffile filterAddrSrc
			| conffile filterAddrDst
			| conffile outputFormatDate
			| conffile outputFormatDateAdd
			| conffile converterThreadCount
			| conffile converterOutputTemplate
			| conffile converterMapper
			| conffile converterResultFile
			;

converterVerbosity:		CONVERTER_VERBOSITY '=' INTEGER ';'
			{
				if( 0 == g_psoOpt->m_iVerbosityLevel ) {
					g_psoOpt->m_iVerbosityLevel = $3;
					logger_set_verbosity( $3 );
					logger_message( 3, "%s = %d;\n", "converterVerbosity", $3 );
				}
				logger_message( 3, "actual value of '%s' is '%d'\n", "converterVerbosity", $3 );
			}
			;

dataDir:		DATA_DIR '=' QSTRING ';'
			{
				if( 0 == g_psoOpt->m_soDataDir.m_iDefinedInCLI ) {
					g_psoOpt->m_soDataDir.m_setDirList.insert( std::string( $3 ) );
					logger_message( 3, "%s = \"%s\";\n", "dataDir", $3 );
				}
			}
			;

dataDirRecursive:		DATA_DIR_RECURSIVE ';'
			{
				if( -1 == g_psoOpt->m_soDataDir.m_iRecursive ) {
					g_psoOpt->m_soDataDir.m_iRecursive = 1;
					logger_message( 3, "%s\n", "dataDirRecursive" );
				}
			}
			;

filterTimeFormat:		FILTER_TIME_FORMAT '=' QSTRING ';'
			{
				if( 0 == g_psoOpt->m_soFilterTime.m_strFormat.size() ) {
					g_psoOpt->m_soFilterTime.m_strFormat.assign( $3 );
					logger_message( 3, "%s = \"%s\";\n", "filterTimeFormat", $3 );
				}
			}
			;

filterTimeStart:		FILTER_TIME_START '=' QSTRING ';'
			{
				if( 0 == g_psoOpt->m_soFilterTime.m_tmFlowStart ) {
					options_str_to_time( $3, &g_psoOpt->m_soFilterTime.m_tmFlowStart );
					logger_message( 3, "%s = \"%s\";\n", "filterTimeStart", $3 );
				}
			}
			;

filterTimeStop:		FILTER_TIME_STOP '=' QSTRING ';'
			{
				if( 0 == g_psoOpt->m_soFilterTime.m_tmFlowStop ) {
					options_str_to_time( $3, &g_psoOpt->m_soFilterTime.m_tmFlowStop );
					logger_message( 3, "%s = \"%s\";\n", "filterTimeStop", $3 );
				}
			}
			;

filterTimeToleranceBefore:		FILTER_TIME_TOLERANCE_BEFORE '=' INTEGER ';'
			{
				if( 0 == g_psoOpt->m_soFilterTime.m_tmToleranceBefore ) {
					g_psoOpt->m_soFilterTime.m_tmToleranceBefore = $3;
					logger_message( 3, "%s = %d;\n", "filterTimeToleranceBefore", $3 );
				}
			}
			;

filterTimeToleranceAfter:		FILTER_TIME_TOLERANCE_AFTER '=' INTEGER ';'
			{
				if( 0 == g_psoOpt->m_soFilterTime.m_tmToleranceAfter ) {
					g_psoOpt->m_soFilterTime.m_tmToleranceAfter = $3;
					logger_message( 3, "%s = %d;\n", "filterTimeToleranceAfter", $3 );
				}
			}
			;

filterAddrSrc:		FILTER_ADDR_SRC '=' QSTRING ';'
			{
				if( 0 == g_psoOpt->m_soFilterAddress.m_ui32AddrSrc ) {
					options_str_to_addr( $3, &g_psoOpt->m_soFilterAddress.m_ui32AddrSrc );
					logger_message( 3, "%s = \"%s\";\n", "filterAddrSrc", $3 );
				}
			}
			;

filterAddrDst:		FILTER_ADDR_DST '=' QSTRING ';'
			{
				if( 0 == g_psoOpt->m_soFilterAddress.m_ui32AddrDst ) {
					options_str_to_addr( $3, &g_psoOpt->m_soFilterAddress.m_ui32AddrDst );
					logger_message( 3, "%s = \"%s\";\n", "filterAddrDst", $3 );
				}
			}
			;

outputFormatDate:		OUTPUT_FORMAT_DATE '=' QSTRING ';'
			{
				if( 0 == g_psoOpt->m_soOutputFormat.m_strOutputFormatDate.size() ) {
					g_psoOpt->m_soOutputFormat.m_strOutputFormatDate.assign( $3 );
					logger_message( 3, "%s = \"%s\";\n", "outputFormatDate", $3 );
				}
			}
			;

outputFormatDateAdd:		OUTPUT_FORMAT_DATE_ADD '=' QSTRING ';'
			{
				if( 0 == g_psoOpt->m_soOutputFormat.m_strOutputFormatDateAdd.size() ) {
					g_psoOpt->m_soOutputFormat.m_strOutputFormatDateAdd.assign( $3 );
					logger_message( 3, "%s = \"%s\";\n", "outputFormatDateAdd", $3 );
				}
			}
			;

converterThreadCount:		CONVERTER_THREAD_COUNT '=' INTEGER ';'
			{
				if( 0 == g_psoOpt->m_soConverter.m_ui32ThreadCount ){
					g_psoOpt->m_soConverter.m_ui32ThreadCount = $3;
					logger_message( 3, "%s = %d;\n", "converterThreadCount", $3 );
				}
			}
			;

converterOutputTemplate:
			CONVERTER_OUTPUT_TEMPLATE '=' converterOutputTemplateInfo ';'
			;

converterOutputTemplateInfo:
			'{' converterOutputTemplateInfoElement '}'
			;

converterOutputTemplateInfoElement:
			| converterOutputTemplateInfoElement QSTRING ';'
			{
				nfc_outputtemplate_field_add( $2 );
				logger_message( 3, "%s = { \"%s\"; };\n", "converterOutputTemplate", $2 );
			}
			;

converterMapper:
			CONVERTER_MAPPER '=' converterMapperInfo ';'
			;

converterMapperInfo:
			'{' converterMapperInfoElement '}'
			;

converterMapperInfoElement:
			INTEGER ';' QSTRING ';' QSTRING ';' INTEGER ';'
			{
				nfc_mapper_add( $1, $3, $5, $7 );
				logger_message( 3, "%s = { %d; \"%s\"; \"%s\"; %d; };\n", "converterMapper", $1, $3, $5, $7 );
			}
			;

converterResultFile:		CONVERTER_RESUL_FILE '=' QSTRING ';'
			{
				if( 0 == g_psoOpt->m_soConverter.m_strResultFile.size() ) {
					g_psoOpt->m_soConverter.m_strResultFile.assign( $3 );
					logger_message( 3, "%s = \"%s\";\n", "converterResultFile", $3 );
				}
			}
			;
