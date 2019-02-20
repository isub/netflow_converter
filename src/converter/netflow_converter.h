#ifndef __NETFLOW_CONVERTER_H__
#define __NETFLOW_CONVERTER_H__

#ifdef __cplus_plus
extern "C" {
#endif

  /*
   *  инициализация конвертора
   */
  int netflow_converter_init();

  /*
   *  запуск конвертора
   */
  void * netflow_converter_start(void * );


#ifdef __cplus_plus
}
#endif

#endif /* __NETFLOW_CONVERTER_H__ */
