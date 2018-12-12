#ifndef __FILE_LIST_H__
#define __FILE_LIST_H__

#include <stdint.h>
#include <string>
#include <set>

namespace nf_file_list {
  struct SFileInfo;
}

/*
  *  инициализация
  */
int file_list_init( const std::set<std::string> *p_psetDirList, const int p_iRecursive );

void file_list_fini();

/*
	*	добавляет новый файл для обработки
	*	p_pszFileName - имя файла
	*/
int file_list_add_data_file( const char *p_pszFileName );

/*
	*	перейти к следующему файлу
	*/
nf_file_list::SFileInfo * file_list_get_next_file_info();
const std::string       * file_list_get_file_name( const nf_file_list::SFileInfo *p_psoFileInfo );
size_t                    file_list_get_file_size( const nf_file_list::SFileInfo *p_psoFileInfo );

/*
  *  возвращает количество файлов количество файлов
  */
int file_list_get_file_count();


#endif /* __FILE_LIST_H__ */
