#include <getopt.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "options.h"
#include "logger.h"

static SOptions g_soOptions;
SOptions *g_psoOpt;

static void options_init();

static void options_issue();

int options_read_options( int argc, char *argv [] )
{
	int iRetVal = 0;
	int iOptInd;
	int iFnRes;

	options_init();

	option msoOption [] = {
	  { "converterVerbosity",			required_argument, NULL, 0 },
	  { "dataDir",						required_argument, NULL, 0 },
	  { "dataDirRecursive",				no_argument,       NULL, 0 },
	  { "filterTimeFormat",				required_argument, NULL, 0 },
	  { "filterTimeStart",				required_argument, NULL, 0 },
	  { "filterTimeStop",				required_argument, NULL, 0 },
	  { "filterTimeToleranceBefore",	required_argument, NULL, 0 },
	  { "filterTimeToleranceAfter",		required_argument, NULL, 0 },
	  { "filterAddrSrc",				required_argument, NULL, 0 },
	  { "filterAddrDst",				required_argument, NULL, 0 },
	  { "outputFormatDate",				required_argument, NULL, 0 },
	  { "outputFormatDateAdd",			required_argument, NULL, 0 },
	  { "converterThreadCount",			required_argument, NULL, 0 },
	  { "converterConf",				required_argument, NULL, 0 },
	  { "converterResultFile",			required_argument, NULL, 0 },
	  { "help",							no_argument, NULL, 0 },
	  { NULL, 0, NULL, 0}
	};

	while( -1 != ( iFnRes = getopt_long( argc, argv, "", msoOption, &iOptInd ) ) ) {
		switch( iFnRes ) {
			case 0:
				logger_message( 1, "CLI parameter: '%s'; value: '%s'\n", msoOption[iOptInd].name, optarg );
				switch( iOptInd ) {
					case 0:
						logger_set_verbosity( atol( optarg ) );
						g_soOptions.m_iVerbosityLevel = atol( optarg );
						break;
					case 1:
						g_soOptions.m_soDataDir.m_setDirList.insert( std::string( optarg ) );
						break;
					case 2:
						g_soOptions.m_soDataDir.m_iRecursive = 1;
						break;
					case 3:
						g_soOptions.m_soFilterTime.m_strFormat.assign( optarg );
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
						g_soOptions.m_soOutputFormat.m_strOutputFormatDate.assign( optarg );
						break;
					case 11:
						g_soOptions.m_soOutputFormat.m_strOutputFormatDateAdd.assign( optarg );
						break;
					case 12:
						g_soOptions.m_soConverter.m_ui32ThreadCount = atol( optarg );
						break;
					case 13:
						g_soOptions.m_soConverter.m_strConfFile.assign( optarg );
						break;
					case 14:
						g_soOptions.m_soConverter.m_strResultFile.assign( optarg );
						break;
					case 15:
						options_issue();
						return EINVAL;
				}
				break;
			case '?':
				options_issue();
				return EINVAL;
		}
		if( 0 != iRetVal ) {
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
	g_soOptions.m_soFilterTime.m_tmFlowStop = static_cast< time_t >( -1 );
	g_soOptions.m_soFilterTime.m_tmToleranceBefore = 0;
	g_soOptions.m_soFilterTime.m_tmToleranceAfter = 0;

	g_soOptions.m_soFilterAddress.m_ui32AddrSrc = 0;
	g_soOptions.m_soFilterAddress.m_ui32AddrDst = 0;

	g_soOptions.m_soOutputFormat.m_strOutputFormatDate = OPTIONS_DEFAULT_TIME_FORMAT;
	g_soOptions.m_soOutputFormat.m_strOutputFormatDateAdd.clear();

	g_soOptions.m_soConverter.m_ui32ThreadCount = 0;

	g_psoOpt = &g_soOptions;
}

int options_str_to_time( const char *p_pszStr, time_t *p_ptmTime )
{
	int iRetVal = 0;
	tm soTm;
	const char *pszTimeFormat;
	const char *pszFnRes;

	if( 0 == g_soOptions.m_soFilterTime.m_strFormat.length() ) {
		pszTimeFormat = OPTIONS_DEFAULT_TIME_FORMAT;
	} else {
		pszTimeFormat = g_soOptions.m_soFilterTime.m_strFormat.c_str();
	}

	memset( &soTm, 0, sizeof( soTm ) );
	pszFnRes = strptime( p_pszStr, pszTimeFormat, &soTm );
	if( NULL != pszFnRes && *pszFnRes == '\0' ) {
		*p_ptmTime = mktime( &soTm );
	} else {
		logger_message( 0, "can't to recognize date value '%s' by using format '%s' at '%s'\n", p_pszStr, pszTimeFormat, pszFnRes );

		iRetVal = -1;
	}

	return iRetVal;
}

int options_str_to_addr( const char *p_pszStr, uint32_t *p_pui32Addr )
{
	int iRetVal = 0;
	in_addr soAddr;

	if( 0 != inet_aton( p_pszStr, &soAddr ) ) {
		*p_pui32Addr = soAddr.s_addr;
	} else {
		logger_message( 0, "can't to recognize ip-addres value '%s'\n", p_pszStr );
		iRetVal = -1;
	}

	return iRetVal;
}

static void options_issue()
{
	logger_message( 0, "netflow Converter поддерживает 9ю и 10ю версии NetFlow\n" );
	logger_message( 0, "параметры командной строки:\n" );
	logger_message( 0, "\tconverterVerbosity - уровень логгирования. Рекомендуемое значение '0'\n" );
	logger_message( 0, "\tdataDir=<имя директории> - имя директории с исходными данными\n" );
	logger_message( 0, "\tdataDirRecursive - если задан этот параметр конвертор будет осуществлять поиск файлов с исходными данными во вложенных директориях\n" );
	logger_message( 0, "\tfilterTimeFormat=<формат времени> - формат представления времени для фильтра. Используется для преобразования значений, заданных параметрами filterTimeStart и filterTimeStop\n" );
	logger_message( 0, "\tfilterTimeStart=<время начала потока> - время начала потока. Используется для выборки данных из файлов с исходными данными, а так же для отбора файлов с исходными данными\n" );
	logger_message( 0, "\tfilterTimeStop=<время окончания потока> - время окончания потока (см. описание параметра filterTimeStart)\n" );
	logger_message( 0, "\tfilterTimeToleranceBefore=<корректировочное значение> - временной интервал (в сек). Корректировочное значение вычитается из значения, полученного в параметре filterTimeStart. Используется только при отборе файлов с исходными данными\n" );
	logger_message( 0, "\tfilterTimeToleranceAfter=<корректировочное значение> - временной интервал (в сек). Корректировочное значение прибавляется к значению, полученному в параметре filterTimeStop. Используется только при отборе файлов с исходными данными\n" );
	logger_message( 0, "\tfilterAddrSrc=<ip-адрес> - ip-адрес источника. Используется фильтром для выборки записей\n" );
	logger_message( 0, "\tfilterAddrDst=<ip-адрес> - ip-адрес получателя. Используется фильтром для выборки записей\n" );
	logger_message( 0, "\toutputFormatDate=<формат времени> - формат времени, используемый для вывода данных\n" );
	logger_message( 0, "\toutputFormatDateAdd=<sec|msec|usec|nsec> - расширение формата вывода времени: sec - не изменяет формат, msec - дополняет миллисекундами, usec - микросекундами, nsec - наносекундами\n" );
	logger_message( 0, "\tconverterThreadCount=<количество потоков> - количество потоков обработки исходных данных. Не имеет смысла задавать значение равное или превышающее число процессоров в системе\n" );
	logger_message( 0, "\tconverterConf=<имя файла> - имя файла конфигурации конвертора\n" );
	logger_message( 0, "\tconverterResultFile=<имя файла> для вывода результатов\n" );
	logger_message( 0, "\thelp - вывод помощи и выход из программы\n" );
	logger_message( 0, "Все параметры командной строки поддерживаются в файле конфигурациии\n" );
	logger_message( 0, "кроме того в конфигурационном файле должны быть описаны следующие параметры:\n" );
	logger_message( 0, "converterOutputTemplate = { \"<первое поле>\"; \"<второе поле>\"; \"<N-ное поле>\"; }; - список имен атрибутов, которые следует выводить в качестве результата выборки\n" );
	logger_message( 0, "converterMapper = { <9|10>; \"<исходное значение>\"; \"<новое значение>\"; <делитель>; }; - параметр маппинга:\n" );
	logger_message( 0, "\tпервый параметр из списка, заключенного в фигурные скобки, - номер протокола netFlow;\n" );
	logger_message( 0, "\tвторой - имя атрибута согласно спецификации;\n" );
	logger_message( 0, "\tтретий - измененное значение имени атрибута для приведения в соответсвии с именами атрибутов в нотации шаблона вывода и фильтров\n" );
	logger_message( 0, "\txtчетвертый - делитель (для секунд - 1, для миллисекунд - 1000 и т.д.)\n" );
	logger_message( 0, "Что надо знать для правильной настройки конвертора:\n" );
	logger_message( 0, "\tшаблон вывода и фильтр работает с именами атрибутов, обработанных предварительно процедурой маппинга\n" );
	logger_message( 0, "\tшаблон вывода поддерживает произвольные имена атрибутов:\n" );
	logger_message( 0, "\tфильтр, напротив, работает с ограниченным перечнем унифицированных имен атрибутов:\n" );
	logger_message( 0, "\t\tflowStart - время начала потока в секундах\n" );
	logger_message( 0, "\t\tflowEnd - время окончания потока в секундах\n" );
	logger_message( 0, "\t\tsourceIPv4Address - ipv4-адрес источника\n" );
	logger_message( 0, "\t\tdestinationIPv4Address - ipv4-адрес получателя\n" );
	logger_message( 0, "\tнапример, в конфигурационном файле заданы следующие параметры:\n" );
	logger_message( 0, "\t\toutputFormatDate = \"\%Y.\%m.\%d_\%H:\%M:\%S\";\n" );
	logger_message( 0, "\t\toutputFormatDateAdd = \"msec\";\n" );
	logger_message( 0, "\t\tconverterOutputTemplate = { \"Дядя Вася\"; };\n" );
	logger_message( 0, "\t\tconverterMapper = { 10; \"flowStartMilliseconds\"; \"Дядя Вася\"; 1000; };\n" );
	logger_message( 0, "\tпроцедура маппинга получив на входе значение flowStartMilliseconds=1550672116123\n" );
	logger_message( 0, "\tпреобразует его согласно шаблону outputFormatDate=\"\%Y.\%m.\%d_\%H:\%M:\%S\" в \"20.02.2019_14:15:16\"\n" );
	logger_message( 0, "\tзатем, в соответсвии с параметром outputFormatDateAdd=\"msec\" допишет к полученной строке \":123\"\n" );
	logger_message( 0, "\tв итоге получим \"20.02.2019_14:15:16:123\"\n" );
	logger_message( 0, "\tимя атрибута, согласно параметру converterMapper = { 10; \"flowStartMilliseconds\"; \"Дядя Вася\"; 1000; };\n" );
	logger_message( 0, "\tпреобразуется из \"flowStartMilliseconds\" в \"Дядя Вася\", исходное значение 1550672116123 делится на 1000 и передается фильтру\n" );
	logger_message( 0, "\tфильтр не поддерживает атрибут \"Дядя Вася\", поэтому фильтрация по времени не сработает и обработка записи будет продолжена\n" );
	logger_message( 0, "\tи если запись будет соответствовать другим параметрам фильтра, то в результатах выборки мы увидим:\n" );
	logger_message( 0, "\t\tДядя Вася\n" );
	logger_message( 0, "\t\t20.02.2019_14:15:16:123\n" );
	logger_message( 0, "\tесли же мы исправим параметр converterMapper следующим образом:\n" );
	logger_message( 0, "\t\tconverterMapper = { 10; \"flowStartMilliseconds\"; \"flowStart\"; 1000; };\n" );
	logger_message( 0, "\tто фильтру передается значение flowStart=1550672116. Фильтр в этом случае корректно обработает запись по заданному времени.\n" );
	logger_message( 0, "\tно теперь в результатах выборки это значение мы не обнаружим\n" );
	logger_message( 0, "\tдля этого надо исправить параметр converterOutputTemplate следующим образом:\n" );
	logger_message( 0, "\t\tconverterOutputTemplate = { \"flowStart\"; };\n" );
	logger_message( 0, "\tв этом случае мы увидим в результатах выборки такие записи:\n" );
	logger_message( 0, "\t\tflowStart\n" );
	logger_message( 0, "\t\t20.02.2019_14:15:16:123\n" );
}
