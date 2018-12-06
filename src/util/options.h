#ifndef __OPTIONS_H__
#define __OPTIONS_H__

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
};

extern SOptions *g_psoOpt;

int options_read_options( int argc, char *argv[ ] );

#endif /* __OPTIONS_H__ */
