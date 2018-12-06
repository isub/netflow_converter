#ifndef __DATA_LOADER_H__
#define __DATA_LOADER_H__

#include <inttypes.h>
#include <stddef.h>

#include "file_list.h"

#ifdef __cplus_plus
extern "C" {
#endif

  int data_loader_open( SFileInfo *p_psoFileInfo );

  /*
   *	загрузка данных в буфер
   *	p_stAmount - объем запрошенных данных
   *	p_ppData - указатель на адрес следующего блока данных
   *	возвращаемое значение - сколько осталось данных в текущем файле, но не более значения p_ui64Amount
   */
  size_t data_loader_get_data( const size_t p_stAmount, uint8_t **p_ppData );

  void data_loader_close();

  /*
   *  возвращает общее количество открытых файлов
   */
  int data_loader_get_total_count_of_opened_files();

#ifdef __cplus_plus
}
#endif

#endif /* __DATA_LOADER_H__ */
