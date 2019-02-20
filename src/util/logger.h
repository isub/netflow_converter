#ifndef __LOGGER_H__
#define __LOGGER_H__

#ifdef __cplus_plus
extern "C" {
#endif

  /*
   *  инициализация конвертора
   */
	void logger_set_verbosity( int p_iVerbosity );
	int logger_open_log( const char *p_pszFileName );

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
