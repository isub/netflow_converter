#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#define OPTIONS_DEFAULT_TIME_FORMAT "%d.%m.%Y %H:%M:%S"
#define OPTIONS_DEFAULT_THREAD_COUNT 1

#include <set>
#include <string>
#include <time.h>
#include <stdint.h>

struct SOptions {
  /* уровень логгировани сообщений */
  int m_iVerbosityLevel;
  /* директории с исходными данными */
  struct {
    std::set<std::string> m_setDirList;
    int m_iRecursive;
  } m_soDataDir;
  /* параметры фильтра по времени */
  struct {
    std::string m_strFormat;
    time_t m_tmFlowStart;
    time_t m_tmFlowStop;
    time_t m_tmToleranceBefore;
    time_t m_tmToleranceAfter;
  } m_soFilterTime;
  /* параметры фильтра по ip-адресам */
  struct {
    uint32_t m_ui32AddrSrc;
    uint32_t m_ui32AddrDst;
  } m_soFilterAddress;
  /* параметры вывода результатов */
  struct {
    std::string m_strOutputFormatDate;
    std::string m_strOutputFormatDateAdd;
  } m_soOutputFormat;
  struct {
    uint32_t m_ui32ThreadCount;
	std::string m_strConfFile;
	std::string m_strResultFile;
  } m_soConverter;
};

extern SOptions *g_psoOpt;

int options_read_options( int argc, char *argv[ ] );
int conf_handle( char * conffile );

int options_str_to_time( const char *p_pszStr, time_t *p_ptmTime );
int options_str_to_addr( const char *p_pszStr, uint32_t *p_pui32Addr );

#endif /* __OPTIONS_H__ */
