#ifndef __NETFLOW_CONVERTER_H__
#define __NETFLOW_CONVERTER_H__

#ifdef __cplus_plus
extern "C" {
#endif

  /*
   *  инициализация конвертора
   */
  int netflow_converter_init( const char *p_pszResultFileName );

  /*
   *  запуск конвертора
   */
  int netflow_converter_start();


#ifdef __cplus_plus
}
#endif

#endif /* __NETFLOW_CONVERTER_H__ */
