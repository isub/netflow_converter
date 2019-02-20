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
/* strptime */
#include <time.h>
#include <pthread.h>
#include <stdio.h>

#include "../util/logger.h"
#include "../filter/filter_time.h"
#include "file_list.h"

namespace nf_file_list {
  struct SFileInfo {
    std::string m_strFileName;
    size_t m_stFileSize;
    int Init( time_t *p_ptFileTime );
    SFileInfo( const char *p_pszFileName ) : m_strFileName( p_pszFileName ) { }
    virtual ~SFileInfo() { }
  };
}

/* список файлов */
static std::multimap<time_t, nf_file_list::SFileInfo * > g_mmapFileList;
/* текущий элемент списка */
static std::multimap<time_t, nf_file_list::SFileInfo * >::iterator g_iterFileList;

/* обработка списка уже начата */
static volatile int g_iStarted;

static pthread_mutex_t g_mutexFileList;

/* обход всех директорий */
static int file_list_read_directory( const char *p_pszDir, const int p_iRecursive );

int file_list_init( const std::set<std::string> *p_psetDirList, const int p_iRecursive )
{
  int iRetVal = 0;

  iRetVal = pthread_mutex_init( &g_mutexFileList, NULL );
  if ( 0 == iRetVal ) {
  } else {
    return EINVAL;
  }

  g_iterFileList = g_mmapFileList.end();
  for ( std::set<std::string>::const_iterator iter = p_psetDirList->begin(); iter != p_psetDirList->end(); ++iter ) {
    file_list_read_directory( iter->c_str(), p_iRecursive );
  }

  logger_message( 5, "file list size: %d\n", file_list_get_file_count() );

  return iRetVal;
}

void file_list_fini()
{
  pthread_mutex_destroy( &g_mutexFileList );
}

int file_list_add_data_file( const char *p_pszFileName )
{
	time_t tFileTime;

	if( 0 == g_iStarted ) {
	} else {
		return EPERM;
	}

	nf_file_list::SFileInfo *psoData = new nf_file_list::SFileInfo( p_pszFileName );

	if( 0 == psoData->Init( &tFileTime ) ) {
		if( 0 == pthread_mutex_lock( &g_mutexFileList ) ) {
			g_mmapFileList.insert( std::pair<time_t, nf_file_list::SFileInfo * >( tFileTime, psoData ) );
			pthread_mutex_unlock( &g_mutexFileList );
		}
	} else {
		delete psoData;
		return EINVAL;
	}

	return 0;
}

nf_file_list::SFileInfo * file_list_get_next_file_info()
{
  int iFnRes;
  nf_file_list::SFileInfo * psoRetVal = NULL;

  iFnRes = pthread_mutex_lock( &g_mutexFileList );
  if ( 0 == iFnRes ) {
  } else {
    return NULL;
  }

  if ( g_iterFileList != g_mmapFileList.end() ) {
    ++ g_iterFileList;
  } else {
    if ( 0 == g_iStarted ) {
      g_iterFileList = g_mmapFileList.begin();
      g_iStarted = 1;
    } else {
      /* список файлов обработан полностью */
      goto __unlock_and_exit__;
    }
  }

  if ( g_iterFileList != g_mmapFileList.end() ) {
    psoRetVal = g_iterFileList->second;
  }

  __unlock_and_exit__:
  pthread_mutex_unlock( &g_mutexFileList );

  return psoRetVal;
}

int file_list_get_file_count()
{
  return g_mmapFileList.size();
}

const std::string * file_list_get_file_name( const nf_file_list::SFileInfo *p_psoFileInfo )
{
  if ( NULL != p_psoFileInfo ) {
    return &p_psoFileInfo->m_strFileName;
  } else {
    return NULL;
  }
}

size_t file_list_get_file_size( const nf_file_list::SFileInfo *p_psoFileInfo )
{
  if ( NULL != p_psoFileInfo ) {
    return p_psoFileInfo->m_stFileSize;
  } else {
    return static_cast<size_t>( -1 );
  }
}

int nf_file_list::SFileInfo::Init( time_t *p_ptFileTime )
{
	if( 0 != m_strFileName.length() ) {
	} else {
		return EINVAL;
	}

	int iRetVal = 0;
	struct stat soStat;

	if( 0 == stat( m_strFileName.c_str(), &soStat ) ) {
	} else {
		return errno;
	}

	const char *pszBaseName;
	const char *pszFnRes;
	tm soTm;
	time_t tTime = static_cast< time_t >( -1 );

	memset( &soTm, 0, sizeof( soTm ) );
	pszBaseName = basename( m_strFileName.c_str() );

	if( NULL != ( pszFnRes = strptime( pszBaseName, "nf%Y%m%d%H%M%S.dat.bz2", &soTm ) ) && *pszFnRes == '\0' ) {
		tTime = mktime( &soTm );
	} else if( NULL != ( pszFnRes = strptime( pszBaseName, "C0A8FE12_%Y%m%d%H%M%S.old.bz2", &soTm ) ) && *pszFnRes == '\0' ) {
		tTime = mktime( &soTm );
	} else {
		logger_message( 5, "can't to recognize date value '%s'\n", pszBaseName, pszFnRes );
	}

	if( tTime != static_cast< time_t >( -1 ) ) {
	} else {
		tTime = soStat.st_mtime;
	}

	if( 0 != soStat.st_size && 0 != filter_time_file( tTime ) ) {
		m_stFileSize = soStat.st_size;
		*p_ptFileTime = tTime;
	} else {
		iRetVal = EINVAL;
	}

	return iRetVal;
}

int file_list_read_directory( const char *p_pszDir, const int p_iRecursive )
{
	int iRetVal = 0;
	DIR *psoDir;
	struct dirent *psoDirEnt;

	psoDir = opendir( p_pszDir );
	if( NULL != psoDir ) {
		while( NULL != ( psoDirEnt = readdir( psoDir ) ) ) {
			if( psoDirEnt->d_type == DT_DIR ) {
				if( 0 != strcmp( psoDirEnt->d_name, "." ) && 0 != strcmp( psoDirEnt->d_name, ".." ) && 0 != p_iRecursive ) {
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
		logger_message( 5, "directory %s was read successfully\n", p_pszDir );
	} else {
		return errno;
	}

	return iRetVal;
}
