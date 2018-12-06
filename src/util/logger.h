#ifndef __LOGGER_H__
#define __LOGGER_H__

#ifdef __cplus_plus
extern "C" {
#endif

  /*
   *  инициализация конвертора
   */
  int logger_init( const char *p_pszFileName, int p_iLogLevel );

  /*
   *  деинициализация логгера
   */
  void logger_deinit();

  /*
   *  запись сообщения в лог
   */
  void logger_message( int p_iLogLevel, const char *p_pszMessage, ... );

#ifdef __cplus_plus
}
#endif


#endif /* __LOGGER_H__ */
