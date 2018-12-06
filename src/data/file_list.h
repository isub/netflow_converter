#ifndef __FILE_LIST_H__
#define __FILE_LIST_H__

#include <stddef.h>
#include <string>
#include <set>

#ifdef __cplus_plus
extern "C" {
#endif

  struct SFileInfo;

  /*
   *  инициализация
   */
  int file_list_init( const std::set<std::string> *p_psetDirList, const int p_iRecursive );

	/*
	 *	добавляет новый файл для обработки
	 *	p_pszFileName - имя файла
	 */
	int file_list_add_data_file( const char *p_pszFileName );

	/*
	 *	перейти к следующему файлу
	 */
	int file_list_go_to_next_file();

  /*
   *  возвращает количество файлов количество файлов
   */
  int file_list_get_file_count();

  SFileInfo * file_list_get_current_file_info();

  const std::string * file_list_get_file_name( const SFileInfo *p_psoFileInfo );

  size_t file_list_get_file_size( const SFileInfo *p_psoFileInfo );

#ifdef __cplus_plus
}
#endif

#endif /* __FILE_LIST_H__ */
