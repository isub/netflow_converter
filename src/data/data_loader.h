#ifndef __DATA_LOADER_H__
#define __DATA_LOADER_H__

#include <inttypes.h>
#include <stddef.h>

#include "file_list.h"

namespace nf_data_loader {
  struct SFileInfo;
}

nf_data_loader::SFileInfo * data_loader_open( nf_file_list::SFileInfo *p_psoFileInfo );

/*
  *	загрузка данных в буфер
  *	p_stAmount - объем запрошенных данных
  *	p_ppData - указатель на адрес следующего блока данных
  *	возвращаемое значение - сколько осталось данных в текущем файле, но не более значения p_ui64Amount
  */
size_t data_loader_get_data( nf_data_loader::SFileInfo * p_psoFileInfo, const size_t p_stAmount, uint8_t **p_ppData );

void data_loader_close( nf_data_loader::SFileInfo * p_psoFileInfo );

/*
  *  возвращает общее количество открытых файлов
  */
int data_loader_get_total_count_of_opened_files();

#endif /* __DATA_LOADER_H__ */
