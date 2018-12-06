#include <map>
#include <set>
#include <string>
#include <errno.h>
/* для stat */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
/* для diropen */
#include <sys/types.h>
#include <dirent.h>
/* для strcmp */
#include <string.h>

#include "data_loader.h"
#include "../util/logger.h"
#include "../filter/filter_time.h"

struct SFileInfo {
	std::string m_strFileName;
	size_t m_stFileSize;
  int Init();
  SFileInfo( const char *p_pszFileName ) : m_strFileName( p_pszFileName ) { }
};

/* список файлов */
static std::multimap<time_t, SFileInfo * > g_mmapFileList;
/* текущий элемент списка */
static std::multimap<time_t, SFileInfo * >::iterator g_iterFileList;

/* обработка списка уже начата */
static volatile int g_iStarted;

/* обход всех директорий */
static int file_list_read_directory( const char *p_pszDir, const int p_iRecursive );

int file_list_init( const std::set<std::string> *p_psetDirList, const int p_iRecursive )
{
  int iRetVal = 0;

  g_iterFileList = g_mmapFileList.end();
  for ( std::set<std::string>::const_iterator iter = p_psetDirList->begin(); iter != p_psetDirList->end(); ++iter ) {
    file_list_read_directory( iter->c_str(), p_iRecursive );
  }

  logger_message( 3, "file list size: %d", file_list_get_file_count() );

  return iRetVal;
}

int file_list_add_data_file( const char *p_pszFileName )
{
	if( 0 == g_iStarted ) {
	} else {
		return EPERM;
	}

	SFileInfo *psoData = new SFileInfo( p_pszFileName );

	if( 0 == psoData->Init() ) {
	} else {
		delete psoData;
		return EINVAL;
	}

	int iRetVal = 0;
}

int file_list_go_to_next_file()
{
	int iRetVal = 0;

	if( g_iterFileList != g_mmapFileList.end() ) {
		++ g_iterFileList;
	} else {
		g_iterFileList = g_mmapFileList.begin();
	}

	if( g_iterFileList != g_mmapFileList.end() ) {
	} else {
		iRetVal = ENODATA;
	}

	return iRetVal;
}

SFileInfo * file_list_get_current_file_info()
{
  if ( g_iterFileList != g_mmapFileList.end() ) {
    return g_iterFileList->second;
  } else {
    return NULL;
  }
}

int file_list_get_file_count()
{
  return g_mmapFileList.size();
}

const std::string * file_list_get_file_name( const SFileInfo *p_psoFileInfo )
{
  if ( NULL != p_psoFileInfo ) {
    return &p_psoFileInfo->m_strFileName;
  } else {
    return NULL;
  }
}

size_t file_list_get_file_size( const SFileInfo *p_psoFileInfo )
{
  if ( NULL != p_psoFileInfo ) {
    return p_psoFileInfo->m_stFileSize;
  } else {
    return static_cast<size_t>( -1 );
  }
}

int SFileInfo::Init()
{
  if ( 0 != m_strFileName.length() ) {
  } else {
    return EINVAL;
  }

  int iRetVal = 0;
  struct stat soStat;

  if ( 0 == stat( m_strFileName.c_str(), &soStat ) && 0 != soStat.st_size && 0 != filter_time_file( soStat.st_mtime ) ) {
    m_stFileSize = soStat.st_size;
    g_mmapFileList.insert( std::pair<time_t, SFileInfo * >( soStat.st_mtime, this ) );
  } else {
    return errno;
  }

  return iRetVal;
}

int file_list_read_directory( const char *p_pszDir, const int p_iRecursive )
{
  int iRetVal = 0;
  DIR *psoDir;
  struct dirent *psoDirEnt;

  psoDir = opendir( p_pszDir );
  if ( NULL != psoDir ) {
    while ( psoDirEnt = readdir( psoDir ) ) {
      if ( psoDirEnt->d_type == DT_DIR ) {
        if ( 0 != strcmp( psoDirEnt->d_name, "." ) && 0 != strcmp( psoDirEnt->d_name, ".." ) && 0 != p_iRecursive ) {
          std::string strPath;

          strPath = p_pszDir;
          strPath += "/";
          strPath += psoDirEnt->d_name;

          file_list_read_directory( strPath.c_str(), p_iRecursive );
        } else {
          continue;
        }
      } else {
        std::string strFile;

        strFile = p_pszDir;
        strFile += '/';
        strFile += psoDirEnt->d_name;

        file_list_add_data_file( strFile.c_str() );
      }
    }
    closedir( psoDir );
    logger_message( 3, "directory %s was read successfully", p_pszDir );
  } else {
    return errno;
  }

  return iRetVal;
}
