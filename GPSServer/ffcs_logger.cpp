/*==================================================================================================
 * 项目名称: 日志库
 *    功能: 提供函数,实现格式日志打印,可选择打印屏幕,文件或两者均有.最新版本支持代码文件、行、函数的打印
 *    作者: huangjf
 *    联系: huangjf@ffcs.cn
 * 最近修改: 2008-12-1
 *    版本: v3.1.0
  ==================================================================================================*/

/*==================================================================================================
 * 修改记录：
 *
 *		20070808 huangjf 增加线程安全控制
 * 		20080630 huangjf 增加打印代码行数
 * 		20081006 huangjf 增加日志输出线程模式，提高日志输出的效率
 * 		20081201 huangjf 修正部分语法，以支持AIX系统
 * 		20090121 huangjf 增加日志定期清理功能
 *
  ==================================================================================================*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include "ffcs_logger.h"
#include "ffcs_logger_queue.h"


pthread_attr_t gThreadAttr;

/*
pthread_mutex_t mutex_write_screen = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_write_file = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_logger_send_queue = PTHREAD_MUTEX_INITIALIZER;
*/
pthread_mutex_t mutex_write_screen;
pthread_mutex_t mutex_write_file;
pthread_mutex_t mutex_logger_send_queue;

static int fun_logger_check_subdir(FFCS_LOG_FILE * _lf);
static int fun_logger_make_dir(const char * _fdir);
static int fun_logger_rmdir(const char * _fdir);
static void fun_logger_currenttime(char * _stime);
static void fun_logger_create_subdirname(char * _subdir, int _level);
static void fun_logger_create_filename(char * _filename);
static void fun_logger_getloglevel(int _var_log_level, char * _level);



/*-----------------------------------------------------------------------
 * name:	屏幕打印
 * input:	fmt  -- 带格式字符串
 * 			ap   -- 不定参数列表
 * output:
 * return:	 -1 -- 失败
 * 			>=0 -- 输出字符数
 *-----------------------------------------------------------------------*/
int fun_logger_screen(const char * fmt, va_list ap) {
	int r;
	pthread_mutex_lock(&mutex_write_screen); /*线程安全*/
	r = vfprintf(stderr, fmt, ap);
	pthread_mutex_unlock(&mutex_write_screen); /*线程安全*/
	fflush(stderr);
	return r;
}

/*-----------------------------------------------------------------------
 * name:	文件打印
 * input:	_lf  -- FFCS_LOG_FILE结构
 * 			fmt  -- 带格式字符串
 * 			ap   -- 不定参数列表
 * output:
 * return:	 -1 -- 失败
 * 			>=0 -- 输出字符数
 *-----------------------------------------------------------------------*/
int fun_logger_file(FFCS_LOG_FILE * _lf, const char * fmt, va_list ap) {
	if (_lf==NULL) return -1;

	if (_lf->_fp==NULL) {
		if (strlen(_lf->_parentdir)<=0)
			sprintf(_lf->_parentdir, "%s", FFCS_LOG_FILEDIR);
		if (fun_logger_make_dir(_lf->_parentdir)<0){
			printf("创建根目录：'%s' 失败\n", _lf->_parentdir);
			return -1;
		}
		if (_lf->_parentdir[strlen(_lf->_parentdir)-1] != '/') {
			sprintf(_lf->_parentdir, "%s/", _lf->_parentdir);
		}

		fun_logger_check_subdir(_lf);

		fun_logger_create_filename(_lf->_filename);

		sprintf(_lf->_fullpath, "%s%s", _lf->_filedir, _lf->_filename);

		_lf->_fp = fopen(_lf->_fullpath, "a+"); /*日志文件*/
		if (_lf->_fp==NULL) {
			fun_perror("");
			printf("打开文件：'%s' 失败\n", _lf->_fullpath);
			return -1;
		}
	}

	if (_lf->_fp!=NULL && strlen(_lf->_fullpath)>0) {
		if (_lf->_log_type._multithread == FFCS_LOGMULTI_ASYN) {
			/*异步，多线程*/
			LogQueueData qdata;
			memset(&qdata, 0x00, sizeof(LogQueueData));

			if (_lf->_LogQ_Status == FFCS_LOGQSTATUS_OPEN) {
				qdata._size = vsprintf(qdata._buff, fmt, ap);
				if (qdata._size > 0) {
					pthread_mutex_lock(&mutex_logger_send_queue); /*线程安全*/
					fun_log_queue_in(&_lf->_LogQ_SEND, &qdata);
					pthread_mutex_unlock(&mutex_logger_send_queue); /*线程安全*/
				}
				return qdata._size;
			}else{
				return vprintf(fmt, ap);
			}
		} else {
			/*同步*/
			int r;
			struct stat _stat;
			pthread_mutex_lock(&mutex_write_file); /*线程安全*/
			fstat(fileno(_lf->_fp), &_stat);
			if(fun_logger_check_subdir(_lf)>0 || _stat.st_size>FFCS_LOG_MAX_FILESIZE){
				fun_logger_create_filename(_lf->_filename);
				memset(_lf->_fullpath, 0, sizeof(_lf->_fullpath));
				sprintf(_lf->_fullpath, "%s%s", _lf->_filedir, _lf->_filename);
				_lf->_fp = freopen(_lf->_fullpath, "a+", _lf->_fp); /*日志文件*/
			}
			r = vfprintf(_lf->_fp, fmt, ap);
			pthread_mutex_unlock(&mutex_write_file); /*线程安全*/
			fflush(_lf->_fp);
			return r;
		}
	}

	return -1;
}

/*-----------------------------------------------------------------------
 * name:	系统初始化
 * 			每次系统调用此日志类时 需要调用此函数初始化日志系统
 * input:	fdir  -- 日志文件目录
 * 			...  -- 支持5个参数：日志媒体、日志级别、日志输出模式、日志清理开关、日志保存月数
 * output:	_lf  -- FFCS_LOG_FILE结构
 * return:	-1 -- 失败
 * 			=1 -- 成功
 *-----------------------------------------------------------------------*/
int fun_logger_init(FFCS_LOG_FILE * _lf, const char * _fdir, ...) {
	va_list ap;

	if (_lf==NULL) return -1;

	va_start(ap, _fdir);
	_lf->_log_type._media = va_arg(ap, int);
	_lf->_log_type._level = va_arg(ap, int);
	_lf->_log_type._multithread = va_arg(ap, int);
	_lf->_cleaner_switch = va_arg(ap, int);
	_lf->_saved_month = va_arg(ap, int);
	va_end(ap);

	if (_lf->_log_type._media<FFCS_LOGMEDIA_SCREEN || _lf->_log_type._media>FFCS_LOGMEDIA_ALL) {
		_lf->_log_type._media = FFCS_LOGMEDIA_SCREEN; /*日志媒体*/
	}
	if (_lf->_log_type._level<FFCS_LOGLEVEL_ERROR || _lf->_log_type._level>FFCS_LOGLEVEL_DEBUG) {
		_lf->_log_type._level = FFCS_LOGLEVEL_MESSAGE; /*日志级别*/
	}
	if (_lf->_log_type._multithread<FFCS_LOGMULTI_SYN || _lf->_log_type._multithread>FFCS_LOGMULTI_ASYN) {
		_lf->_log_type._multithread = FFCS_LOGMULTI_SYN; /*日志输出模式*/
	}
	if (_lf->_cleaner_switch<FFCS_LOGCLEANER_CLOSED || _lf->_cleaner_switch>FFCS_LOGCLEANER_OPEN) {
		_lf->_cleaner_switch = FFCS_LOGCLEANER_CLOSED; /*日志清理开关*/
	}
	if (_lf->_saved_month<0 || _lf->_saved_month>120) {
		_lf->_saved_month = 0; /*日志保存月数*/
	}

	memset(_lf->_parentdir, 0, sizeof(_lf->_parentdir));
	strncpy(_lf->_parentdir, _fdir, sizeof(_lf->_parentdir));
	if (strlen(_lf->_parentdir)<=0)
		sprintf(_lf->_parentdir, "%s", FFCS_LOG_FILEDIR);
	if (fun_logger_make_dir(_lf->_parentdir)<0){
		printf("创建根目录：'%s' 失败\n", _lf->_parentdir);
		return -1;
	}
	if (_lf->_parentdir[strlen(_lf->_parentdir)-1] != '/') {
		sprintf(_lf->_parentdir, "%s/", _lf->_parentdir);
	}

	fun_logger_check_subdir(_lf);

	fun_logger_create_filename(_lf->_filename);

	sprintf(_lf->_fullpath, "%s%s", _lf->_filedir, _lf->_filename);

	_lf->_fp = fopen(_lf->_fullpath, "a+"); /*日志文件*/
	if (_lf->_fp==NULL) {
		fun_perror("");
		return -1;
	}

	pthread_mutex_init(&mutex_write_screen, NULL);
	pthread_mutex_init(&mutex_write_file, NULL);
	pthread_mutex_init(&mutex_logger_send_queue, NULL);

	pthread_attr_init(&gThreadAttr);
	pthread_attr_setdetachstate(&gThreadAttr, PTHREAD_CREATE_DETACHED);

	/*异步，多线程模式*/
	if (_lf->_log_type._multithread == FFCS_LOGMULTI_ASYN) {

		/*日志数据队列*/
		if (fun_log_queue_init(&_lf->_LogQ_SEND) < 0) {
			printf("创建日志队列失败\n");
			return -1;
		}
		_lf->_LogQ_Status = FFCS_LOGQSTATUS_OPEN;

		/*日志输出线程*/
		if (pthread_create(&_lf->_pt_logger, &gThreadAttr, pthr_logger_process, _lf)) {
			printf("创建日志处理线程失败\n");
			return -1;
		}
		if (!_lf->_pt_logger) {
			printf("创建日志处理线程失败\n");
			return -1;
		}
	}

	/*监控目录，删除过期日志*/
	if (pthread_create(&_lf->_pt_cleaner, &gThreadAttr, pthr_cleaner_process, _lf)) {
		printf("创建日志清理线程失败\n");
		return -1;
	}
	if (!_lf->_pt_cleaner) {
		printf("创建日志清理线程失败\n");
		return -1;
	}


	return 1;
}

/*-----------------------------------------------------------------------
 * name:	error日志
 * input:	_lf  -- FFCS_LOG_FILE结构
 * 			fmt  -- 带格式字符串
 * 			...  -- 不定参数列表
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_error(FFCS_LOG_FILE * _lf, const char * fmt, ...) {
	va_list ap;
	char _level[10];
	char _stime[20];
	char _buff[30];

	if (_lf->_log_type._level < FFCS_LOGLEVEL_ERROR) return;

	va_start(ap, fmt);

	memset(_level, 0, sizeof(_level));
	memset(_stime, 0, sizeof(_stime));
	memset(_buff, 0, sizeof(_buff));
	fun_logger_getloglevel(FFCS_LOGLEVEL_ERROR, _level);
	fun_logger_currenttime(_stime);
	sprintf(_buff, "[%s] [%s]: ", _stime, _level);
	switch (_lf->_log_type._media) {
	case FFCS_LOGMEDIA_ALL:
		fun_logger_screen(_buff, NULL);
		fun_logger_screen(fmt, ap);
		fun_logger_file(_lf, _buff, NULL);
		fun_logger_file(_lf, fmt, ap);
		break;
	case FFCS_LOGMEDIA_FILE:
		fun_logger_file(_lf, _buff, NULL);
		fun_logger_file(_lf, fmt, ap);
		break;
	case FFCS_LOGMEDIA_SCREEN:
	default:
		fun_logger_screen(_buff, NULL);
		fun_logger_screen(fmt, ap);
		break;
	}

	va_end(ap);
}

/*-----------------------------------------------------------------------
 * name:	warn日志
 * input:	_lf  -- FFCS_LOG_FILE结构
 * 			fmt  -- 带格式字符串
 * 			...  -- 不定参数列表
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_warn(FFCS_LOG_FILE * _lf, const char * fmt, ...) {
	va_list ap;
	char _level[10];
	char _stime[20];
	char _buff[30];

	if (_lf->_log_type._level < FFCS_LOGLEVEL_WARN) return;

	va_start(ap, fmt);

	memset(_level, 0, sizeof(_level));
	memset(_stime, 0, sizeof(_stime));
	memset(_buff, 0, sizeof(_buff));
	fun_logger_getloglevel(FFCS_LOGLEVEL_WARN, _level);
	fun_logger_currenttime(_stime);
	sprintf(_buff, "[%s] [%s]: ", _stime, _level);
	switch (_lf->_log_type._media) {
	case FFCS_LOGMEDIA_ALL:
		fun_logger_screen(_buff, NULL);
		fun_logger_screen(fmt, ap);
		fun_logger_file(_lf, _buff, NULL);
		fun_logger_file(_lf, fmt, ap);
		break;
	case FFCS_LOGMEDIA_FILE:
		fun_logger_file(_lf, _buff, NULL);
		fun_logger_file(_lf, fmt, ap);
		break;
	case FFCS_LOGMEDIA_SCREEN:
	default:
		fun_logger_screen(_buff, NULL);
		fun_logger_screen(fmt, ap);
		break;
	}

	va_end(ap);
}

/*-----------------------------------------------------------------------
 * name:	message日志
 * input:	_lf  -- FFCS_LOG_FILE结构
 * 			fmt  -- 带格式字符串
 * 			...  -- 不定参数列表
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_message(FFCS_LOG_FILE * _lf, const char * fmt, ...) {
	va_list ap;
	char _level[10];
	char _stime[20];
	char _buff[30];

	if (_lf->_log_type._level < FFCS_LOGLEVEL_MESSAGE) return;

	va_start(ap, fmt);

	memset(_level, 0, sizeof(_level));
	memset(_stime, 0, sizeof(_stime));
	memset(_buff, 0, sizeof(_buff));
	fun_logger_getloglevel(FFCS_LOGLEVEL_MESSAGE, _level);
	fun_logger_currenttime(_stime);
	sprintf(_buff, "[%s] [%s]: ", _stime, _level);
	switch (_lf->_log_type._media) {
	case FFCS_LOGMEDIA_ALL:
		fun_logger_screen(_buff, NULL);
		fun_logger_screen(fmt, ap);
		fun_logger_file(_lf, _buff, NULL);
		fun_logger_file(_lf, fmt, ap);
		break;
	case FFCS_LOGMEDIA_FILE:
		fun_logger_file(_lf, _buff, NULL);
		fun_logger_file(_lf, fmt, ap);
		break;
	case FFCS_LOGMEDIA_SCREEN:
	default:
		fun_logger_screen(_buff, NULL);
		fun_logger_screen(fmt, ap);
		break;
	}

	va_end(ap);
}

/*-----------------------------------------------------------------------
 * name:	message日志
 * input:	_lf  -- FFCS_LOG_FILE结构
 * 			fmt  -- 带格式字符串
 * 			...  -- 不定参数列表
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_debug(FFCS_LOG_FILE * _lf, const char * fmt, ...) {
	va_list ap;
	char _level[10];
	char _stime[20];
	char _buff[30];

	if (_lf->_log_type._level < FFCS_LOGLEVEL_DEBUG) return;

	va_start(ap, fmt);

	memset(_level, 0, sizeof(_level));
	memset(_stime, 0, sizeof(_stime));
	memset(_buff, 0, sizeof(_buff));
	fun_logger_getloglevel(FFCS_LOGLEVEL_DEBUG, _level);
	fun_logger_currenttime(_stime);
	sprintf(_buff, "[%s] [%s]: ", _stime, _level);
	switch (_lf->_log_type._media) {
	case FFCS_LOGMEDIA_ALL:
		fun_logger_screen(_buff, NULL);
		fun_logger_screen(fmt, ap);
		fun_logger_file(_lf, _buff, NULL);
		fun_logger_file(_lf, fmt, ap);
		break;
	case FFCS_LOGMEDIA_FILE:
		fun_logger_file(_lf, _buff, NULL);
		fun_logger_file(_lf, fmt, ap);
		break;
	case FFCS_LOGMEDIA_SCREEN:
	default:
		fun_logger_screen(_buff, NULL);
		fun_logger_screen(fmt, ap);
		break;
	}

	va_end(ap);
}

/*-----------------------------------------------------------------------
 * name:	error日志，支持打印代码行
 * input:	_lf  -- FFCS_LOG_FILE结构
 * 			_file_  -- 日志文件（宏：__FILE__）
 * 			_line_  -- 日志行（宏：__LINE__）
 * 			_fun_  -- 日志函数名（宏：__FUNCTION__）
 * 			fmt  -- 带格式字符串
 * 			...  -- 不定参数列表
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_error_extend(FFCS_LOG_FILE * _lf, const char * _file_, int _line_, const char * _fun_, const char * fmt, ...) {
	va_list ap;
	char _level[10];
	char _stime[20];
	char _buff[255];

	if (_lf->_log_type._level < FFCS_LOGLEVEL_ERROR) return;

	va_start(ap, fmt);

	memset(_level, 0, sizeof(_level));
	memset(_stime, 0, sizeof(_stime));
	memset(_buff, 0, sizeof(_buff));
	fun_logger_getloglevel(FFCS_LOGLEVEL_ERROR, _level);
	fun_logger_currenttime(_stime);
	sprintf(_buff, "[%s] [%s] [%s(%d)-%s]: ", _stime, _level, _file_, _line_, _fun_);
	switch (_lf->_log_type._media) {
	case FFCS_LOGMEDIA_ALL:
		fun_logger_screen(_buff, NULL);
		fun_logger_screen(fmt, ap);
		fun_logger_file(_lf, _buff, NULL);
		fun_logger_file(_lf, fmt, ap);
		break;
	case FFCS_LOGMEDIA_FILE:
		fun_logger_file(_lf, _buff, NULL);
		fun_logger_file(_lf, fmt, ap);
		break;
	case FFCS_LOGMEDIA_SCREEN:
	default:
		fun_logger_screen(_buff, NULL);
		fun_logger_screen(fmt, ap);
		break;
	}

	va_end(ap);
}

/*-----------------------------------------------------------------------
 * name:	warn日志，支持打印代码行
 * input:	_lf  -- FFCS_LOG_FILE结构
 * 			_file_  -- 日志文件（宏：__FILE__）
 * 			_line_  -- 日志行（宏：__LINE__）
 * 			_fun_  -- 日志函数名（宏：__FUNCTION__）
 * 			fmt  -- 带格式字符串
 * 			...  -- 不定参数列表
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_warn_extend(FFCS_LOG_FILE * _lf, const char * _file_, int _line_, const char * _fun_, const char * fmt, ...) {
	va_list ap;
	char _level[10];
	char _stime[20];
	char _buff[255];

	if (_lf->_log_type._level < FFCS_LOGLEVEL_WARN) return;

	va_start(ap, fmt);

	memset(_level, 0, sizeof(_level));
	memset(_stime, 0, sizeof(_stime));
	memset(_buff, 0, sizeof(_buff));
	fun_logger_getloglevel(FFCS_LOGLEVEL_WARN, _level);
	fun_logger_currenttime(_stime);
	sprintf(_buff, "[%s] [%s] [%s(%d)-%s]: ", _stime, _level, _file_, _line_, _fun_);
	switch (_lf->_log_type._media) {
	case FFCS_LOGMEDIA_ALL:
		fun_logger_screen(_buff, NULL);
		fun_logger_screen(fmt, ap);
		fun_logger_file(_lf, _buff, NULL);
		fun_logger_file(_lf, fmt, ap);
		break;
	case FFCS_LOGMEDIA_FILE:
		fun_logger_file(_lf, _buff, NULL);
		fun_logger_file(_lf, fmt, ap);
		break;
	case FFCS_LOGMEDIA_SCREEN:
	default:
		fun_logger_screen(_buff, NULL);
		fun_logger_screen(fmt, ap);
		break;
	}

	va_end(ap);
}

/*-----------------------------------------------------------------------
 * name:	messag日志，支持打印代码行
 * input:	_lf  -- FFCS_LOG_FILE结构
 * 			_file_  -- 日志文件（宏：__FILE__）
 * 			_line_  -- 日志行（宏：__LINE__）
 * 			_fun_  -- 日志函数名（宏：__FUNCTION__）
 * 			fmt  -- 带格式字符串
 * 			...  -- 不定参数列表
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_message_extend(FFCS_LOG_FILE * _lf, const char * _file_, int _line_, const char * _fun_, const char * fmt, ...) {
	va_list ap;
	char _level[10];
	char _stime[20];
	char _buff[255];

	if (_lf->_log_type._level < FFCS_LOGLEVEL_MESSAGE) return;

	va_start(ap, fmt);

	memset(_level, 0, sizeof(_level));
	memset(_stime, 0, sizeof(_stime));
	memset(_buff, 0, sizeof(_buff));
	fun_logger_getloglevel(FFCS_LOGLEVEL_MESSAGE, _level);
	fun_logger_currenttime(_stime);
	sprintf(_buff, "[%s] [%s] [%s(%d)-%s]: ", _stime, _level, _file_, _line_, _fun_);
	switch (_lf->_log_type._media) {
	case FFCS_LOGMEDIA_ALL:
		fun_logger_screen(_buff, NULL);
		fun_logger_screen(fmt, ap);
		fun_logger_file(_lf, _buff, NULL);
		fun_logger_file(_lf, fmt, ap);
		break;
	case FFCS_LOGMEDIA_FILE:
		fun_logger_file(_lf, _buff, NULL);
		fun_logger_file(_lf, fmt, ap);
		break;
	case FFCS_LOGMEDIA_SCREEN:
	default:
		fun_logger_screen(_buff, NULL);
		fun_logger_screen(fmt, ap);
		break;
	}

	va_end(ap);
}

/*-----------------------------------------------------------------------
 * name:	debug日志，支持打印代码行
 * input:	_lf  -- FFCS_LOG_FILE结构
 * 			_file_  -- 日志文件（宏：__FILE__）
 * 			_line_  -- 日志行（宏：__LINE__）
 * 			_fun_  -- 日志函数名（宏：__FUNCTION__）
 * 			fmt  -- 带格式字符串
 * 			...  -- 不定参数列表
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_debug_extend(FFCS_LOG_FILE * _lf, const char * _file_, int _line_, const char * _fun_, const char * fmt, ...) {
	va_list ap;
	char _level[10];
	char _stime[20];
	char _buff[255];

	if (_lf->_log_type._level < FFCS_LOGLEVEL_DEBUG) return;

	va_start(ap, fmt);

	memset(_level, 0, sizeof(_level));
	memset(_stime, 0, sizeof(_stime));
	memset(_buff, 0, sizeof(_buff));
	fun_logger_getloglevel(FFCS_LOGLEVEL_DEBUG, _level);
	fun_logger_currenttime(_stime);
	sprintf(_buff, "[%s] [%s] [%s(%d)-%s]: ", _stime, _level, _file_, _line_, _fun_);
	switch (_lf->_log_type._media) {
	case FFCS_LOGMEDIA_ALL:
		fun_logger_screen(_buff, NULL);
		fun_logger_screen(fmt, ap);
		fun_logger_file(_lf, _buff, NULL);
		fun_logger_file(_lf, fmt, ap);
		break;
	case FFCS_LOGMEDIA_FILE:
		fun_logger_file(_lf, _buff, NULL);
		fun_logger_file(_lf, fmt, ap);
		break;
	case FFCS_LOGMEDIA_SCREEN:
	default:
		fun_logger_screen(_buff, NULL);
		fun_logger_screen(fmt, ap);
		break;
	}

	va_end(ap);
}

/*-----------------------------------------------------------------------
 * name:	以指定格式打印，不增加其他信息
 * input:	_lf  -- FFCS_LOG_FILE结构
 * 			fmt  -- 带格式字符串
 * 			...  -- 不定参数列表
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_printf(FFCS_LOG_FILE * _lf, const char * fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	switch (_lf->_log_type._media) {
	case FFCS_LOGMEDIA_ALL:
		fun_logger_screen(fmt, ap);
		fun_logger_file(_lf, fmt, ap);
		break;
	case FFCS_LOGMEDIA_FILE:
		fun_logger_file(_lf, fmt, ap);
		break;
	case FFCS_LOGMEDIA_SCREEN:
	default:
		fun_logger_screen(fmt, ap);
		break;
	}

	va_end(ap);
}


/*-----------------------------------------------------------------------
 * name:	关闭日志
 * input:	_lf  -- FFCS_LOG_FILE结构
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_close(FFCS_LOG_FILE * _lf) {
	if (_lf->_pt_logger) {
		pthread_cancel(_lf->_pt_logger);
		pthread_detach(_lf->_pt_logger);
	}
	_lf->_pt_logger = 0;

	/*异步，多线程模式*/
	if (_lf->_log_type._multithread == FFCS_LOGMULTI_ASYN) {
		fun_log_queue_destroy(&_lf->_LogQ_SEND);
	}

	if (_lf->_fp) {
		fclose(_lf->_fp);
		_lf->_fp = NULL;
	}

	pthread_mutex_unlock(&mutex_write_screen); /*线程安全*/
	pthread_mutex_unlock(&mutex_write_file); /*线程安全*/
	pthread_mutex_unlock(&mutex_logger_send_queue); /*线程安全*/
	pthread_mutex_destroy(&mutex_write_screen);
	pthread_mutex_destroy(&mutex_write_file);
	pthread_mutex_destroy(&mutex_logger_send_queue);
}


/*-----------------------------------------------------------------------
 * name:	检测目录
 * input:	_lf  -- FFCS_LOG_FILE结构
 * output:
 * return:	>0  -- 新建目录
 * 			=0  -- 目录无变化
 * 			<0  -- 失败
 *-----------------------------------------------------------------------*/
static int fun_logger_check_subdir(FFCS_LOG_FILE * _lf) {
	int ret;
	char _subdir[FFCS_LOG_MAX_NAMESIZE];
	memset(_subdir, 0, sizeof(_subdir));

	if (_lf==NULL) return -1;

	memset(_lf->_filedir, 0, sizeof(_lf->_filedir));
	sprintf(_lf->_filedir,  "%s", _lf->_parentdir);

	fun_logger_create_subdirname(_subdir, 1);
	strncat(_lf->_filedir, _subdir, sizeof(_lf->_filedir));
	ret = fun_logger_make_dir(_lf->_filedir);
	if(ret<0){
		memset(_lf->_filedir, 0, sizeof(_lf->_filedir));
		sprintf(_lf->_filedir,  "%s", _lf->_parentdir);
		return -1;
	}
	sprintf(_lf->_filedir, "%s/", _lf->_filedir);

	fun_logger_create_subdirname(_subdir, 2);
	strncat(_lf->_filedir, _subdir, sizeof(_lf->_filedir));
	ret = fun_logger_make_dir(_lf->_filedir);
	if(ret<0){
		memset(_lf->_filedir, 0, sizeof(_lf->_filedir));
		sprintf(_lf->_filedir,  "%s", _lf->_parentdir);
		return -1;
	}
	sprintf(_lf->_filedir, "%s/", _lf->_filedir);

	fun_logger_create_subdirname(_subdir, 3);
	strncat(_lf->_filedir, _subdir, sizeof(_lf->_filedir));
	ret = fun_logger_make_dir(_lf->_filedir);
	if(ret<0){
		memset(_lf->_filedir, 0, sizeof(_lf->_filedir));
		sprintf(_lf->_filedir,  "%s", _lf->_parentdir);
		return -1;
	}
	sprintf(_lf->_filedir, "%s/", _lf->_filedir);

	return ret;
}


/*-----------------------------------------------------------------------
 * name:	建立目录
 * input:	_fdir  -- 目录
 * output:
 * return:	>0  -- 新建目录
 * 			=0  -- 目录已存在
 * 			<0  -- 创建失败
 *-----------------------------------------------------------------------*/
static int fun_logger_make_dir(const char * _fdir){
	if (strlen(_fdir) <= 0) return -1; /*文件名错误*/
	if (access(_fdir, F_OK) >= 0) return 0; /*目录已存在*/
	if (mkdir(_fdir, S_IRWXU) < 0) { /*建立目录失败*/
		fun_perror("");
		return -1;
	}
	/*建立目录成功*/
	return 1;
}

/*-----------------------------------------------------------------------
 * name:	删除目录或文件
 * input:	_fdir  -- 目录或文件路劲
 * output:
 * return:	>0  -- 删除成功
 * 			<0  -- 删除失败
 *-----------------------------------------------------------------------*/
static int fun_logger_rmdir(const char * _fdir) {
	char command[512]; /*命令*/
	if (strlen(_fdir) <= 0) return -1; /*文件名错误*/
	if (access(_fdir, F_OK) < 0) return -1; /*目录不存在*/

	/*使用命令行删除*/
	memset(command, 0x00, sizeof(command));
	sprintf(command, "rm -rf %s", _fdir);
	system(command);
/*
	if (remove(_fdir) < 0) { 删除目录失败
		fun_perror("");
		return -1;
	}
*/
	/*删除目录成功*/
	return 1;
}


/*-----------------------------------------------------------------------
 * name:	取当前时间
 * input:
 * output:	_stime  -- 时间字符串
 * return:
 *-----------------------------------------------------------------------*/
static void fun_logger_currenttime(char * _stime) {
	time_t timep;
	struct tm * p;
	time(&timep);
	p = localtime(&timep);
	sprintf(_stime, "%4d-%02d-%02d %02d:%02d:%02d", (1900+p->tm_year), (1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
}

/*-----------------------------------------------------------------------
 * name:	创建一个日志文件子目录名
 * input:	_level  -- 目录级别
 * output:	_subdir  -- 子目录名字符串
 * return:
 *-----------------------------------------------------------------------*/
static void fun_logger_create_subdirname(char * _subdir, int _level) {
	time_t timep;
	struct tm * p;
	char _str[20];

	time(&timep);
	p = localtime(&timep);
	switch(_level){
	case 1:
		sprintf(_str, "%4d", (1900+p->tm_year));
		break;
	case 2:
		sprintf(_str, "%02d", (1+p->tm_mon));
		break;
	case 3:
		sprintf(_str, "%02d", p->tm_mday);
		break;
	default:
		sprintf(_str, "%02d", 0);
		break;
	}

	sprintf(_subdir, "%s", _str);
}

/*-----------------------------------------------------------------------
 * name:	创建日志文件名后续时间字符串
 * input:
 * output:	_filename  -- 志文件名后续时间字符串
 * return:
 *-----------------------------------------------------------------------*/
static void fun_logger_create_filename(char * _filename) {
	time_t timep;
	struct tm * p;
	char _str[20];

	time(&timep);
	p = localtime(&timep);
	sprintf(_str, "%02d_%02d_%02d", p->tm_hour, p->tm_min, p->tm_sec);
	sprintf(_filename, "%s_%s%s", FFCS_LOG_FILENAME, _str, FFCS_LOG_FILETYPE);
}

/*-----------------------------------------------------------------------
 * name:	取日志级别
 * input:
 * output:	_level  -- 日志级别
 * return:
 *-----------------------------------------------------------------------*/
static void fun_logger_getloglevel(int _var_log_level, char * _level) {
	switch (_var_log_level) {
	case FFCS_LOGLEVEL_ERROR:
		sprintf(_level, "ERROR");
		break;
	case FFCS_LOGLEVEL_WARN:
		sprintf(_level, "WARN ");
		break;
	case FFCS_LOGLEVEL_DEBUG:
		sprintf(_level, "DEBUG");
		break;
	case FFCS_LOGLEVEL_MESSAGE:
	default:
		sprintf(_level, "MSG  ");
		break;
	}
}

/*-----------------------------------------------------------------------
 * name:	日志处理线程
 * input:
 * output:	arg  -- FFCS_LOG_FILE结构参数
 * return:
 *-----------------------------------------------------------------------*/
void * pthr_logger_process(void * arg) {
	struct timespec rqt;

	FFCS_LOG_FILE * ffcs_log_file_0 = (FFCS_LOG_FILE *)arg;
	if (ffcs_log_file_0 == NULL || ffcs_log_file_0->_log_type._multithread != FFCS_LOGMULTI_ASYN) {
		printf("日志处理线程有异常，输入参数不正确，线程(%ld)退出\n", pthread_self()&0xFFFF);
		pthread_exit(NULL);
	}
	printf("*** 日志处理线程(%ld)启动，日志文件根目录[%s]，日志级别[%d] ***\n", pthread_self()&0xFFFF, ffcs_log_file_0->_parentdir, ffcs_log_file_0->_log_type._level);

	pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void *)&mutex_write_screen);
	pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void *)&mutex_write_file);
	pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void *)&mutex_logger_send_queue);

	while (1) {
		if (ffcs_log_file_0->_fp==NULL) {
			if (strlen(ffcs_log_file_0->_parentdir)<=0)
				sprintf(ffcs_log_file_0->_parentdir, "%s", FFCS_LOG_FILEDIR);
			if (fun_logger_make_dir(ffcs_log_file_0->_parentdir)<0){
				printf("创建根目录：'%s' 失败\n", ffcs_log_file_0->_parentdir);

				rqt.tv_sec = 10;
				rqt.tv_nsec = 0;
				nanosleep(&rqt,NULL); /* 10秒*/
				continue;
			}
			if (ffcs_log_file_0->_parentdir[strlen(ffcs_log_file_0->_parentdir-1)-1] != '/') {
				sprintf(ffcs_log_file_0->_parentdir, "%s/", ffcs_log_file_0->_parentdir);
			}

			fun_logger_check_subdir(ffcs_log_file_0);

			fun_logger_create_filename(ffcs_log_file_0->_filename);

			sprintf(ffcs_log_file_0->_fullpath, "%s%s", ffcs_log_file_0->_filedir, ffcs_log_file_0->_filename);

			ffcs_log_file_0->_fp = fopen(ffcs_log_file_0->_fullpath, "a+"); /*日志文件*/
			if (ffcs_log_file_0->_fp==NULL) {
				fun_perror("");

				printf("打开文件：'%s' 失败\n", ffcs_log_file_0->_fullpath);

				rqt.tv_sec = 10;
				rqt.tv_nsec = 0;
				nanosleep(&rqt,NULL); /* 10秒*/
				continue;
			}
		}

		if (ffcs_log_file_0->_fp!=NULL && strlen(ffcs_log_file_0->_fullpath)>0) {
			int r = 0;
			int len = 0;
			struct stat _stat;
			LogQueueData pdata;

			pthread_mutex_lock(&mutex_write_file); /*线程安全*/
			fstat(fileno(ffcs_log_file_0->_fp), &_stat);
			if(fun_logger_check_subdir(ffcs_log_file_0)>0 || _stat.st_size>FFCS_LOG_MAX_FILESIZE){
				fun_logger_create_filename(ffcs_log_file_0->_filename);
				memset(ffcs_log_file_0->_fullpath, 0, sizeof(ffcs_log_file_0->_fullpath));
				sprintf(ffcs_log_file_0->_fullpath, "%s%s", ffcs_log_file_0->_filedir, ffcs_log_file_0->_filename);
				ffcs_log_file_0->_fp = freopen(ffcs_log_file_0->_fullpath, "a+", ffcs_log_file_0->_fp); /*日志文件*/
			}
			pthread_mutex_unlock(&mutex_write_file); /*线程安全*/

			/*判断日志队列状态*/
			pthread_mutex_lock(&mutex_logger_send_queue); /*线程安全*/
			len = fun_log_queue_length(&ffcs_log_file_0->_LogQ_SEND);
			pthread_mutex_unlock(&mutex_logger_send_queue); /*线程安全*/
			if (len>FFCS_LOGQ_MAXSIZE) {
				ffcs_log_file_0->_LogQ_Status = FFCS_LOGQSTATUS_CLOSED;
			}

			/*打印队列日志数据*/
			if (len > 0) {
				do {
					memset(&pdata, 0x00, sizeof(pdata));

					pthread_mutex_lock(&mutex_logger_send_queue); /*线程安全*/
					r = fun_log_queue_out(&ffcs_log_file_0->_LogQ_SEND, &pdata);
					pthread_mutex_unlock(&mutex_logger_send_queue); /*线程安全*/

					pthread_mutex_lock(&mutex_write_file); /*线程安全*/
					fprintf(ffcs_log_file_0->_fp, "%s", pdata._buff);
					pthread_mutex_unlock(&mutex_write_file); /*线程安全*/
				}while(r > 0);

				fflush(ffcs_log_file_0->_fp);
			}

			ffcs_log_file_0->_LogQ_Status = FFCS_LOGQSTATUS_OPEN;
		}

		rqt.tv_sec = 0;
		rqt.tv_nsec = 50000000; /* 50毫秒*/
		nanosleep(&rqt,NULL);
	}

	printf("*** 日志处理线程(%ld)退出 ***\n", pthread_self()&0xFFFF);

	pthread_cleanup_pop(0);
	pthread_cleanup_pop(0);
	pthread_cleanup_pop(0);

	pthread_exit(NULL);
}

/*-----------------------------------------------------------------------
 * name:	日志清理线程
 * input:
 * output:	arg  -- FFCS_LOG_FILE结构参数
 * return:
 *-----------------------------------------------------------------------*/
void * pthr_cleaner_process(void * arg) {
	struct timespec rqt;
	struct stat _stat;

	time_t timep;
	struct tm * p;
	char rm_year[4+1], rm_month[2+1]; /*待删除年月*/
	char rm_dir[FFCS_LOG_MAX_NAMESIZE]; /*待删除目录路劲*/
	int c_year, c_month; /*当前年月*/

	FFCS_LOG_FILE * ffcs_log_file_0 = (FFCS_LOG_FILE *)arg;
	if (ffcs_log_file_0 == NULL) {
		printf("日志清理线程有异常，输入参数不正确，线程(%ld)退出\n", pthread_self()&0xFFFF);
		pthread_exit(NULL);
	}
	printf("*** 日志清理线程(%ld)启动，日志文件根目录[%s] ***\n", pthread_self()&0xFFFF, ffcs_log_file_0->_parentdir);


	while (1) {
		if (ffcs_log_file_0->_cleaner_switch != FFCS_LOGCLEANER_OPEN || ffcs_log_file_0->_saved_month <= 0 || ffcs_log_file_0->_saved_month >= 120) {
			ffcs_log_file_0->_saved_month = 0;

			rqt.tv_sec = 300; /*300秒 = 5分钟*/
			rqt.tv_nsec = 0;
			nanosleep(&rqt,NULL);
			continue;
		}

		if (strlen(ffcs_log_file_0->_parentdir) <= 0) {
			printf("日志清理线程有异常，日志文件根目录不合法，线程(%ld)退出\n", pthread_self()&0xFFFF);
			pthread_exit(NULL);
		}
		if (ffcs_log_file_0->_parentdir[strlen(ffcs_log_file_0->_parentdir)-1] != '/') {
			sprintf(ffcs_log_file_0->_parentdir, "%s/", ffcs_log_file_0->_parentdir);
		}

		memset(rm_year, 0x00, sizeof(rm_year));
		memset(rm_month, 0x00, sizeof(rm_month));
		memset(rm_dir, 0x00, sizeof(rm_dir));

		/*取当前年月*/
		time(&timep);
		p = localtime(&timep);
		c_year = (1900+p->tm_year);
		c_month = (1+p->tm_mon);

		/*取待删除年月*/
		if (c_month <= (ffcs_log_file_0->_saved_month+1)) {
			sprintf(rm_year, "%04d", c_year-1);
			sprintf(rm_month, "%02d", c_month+12-(ffcs_log_file_0->_saved_month+1));
		} else {
			sprintf(rm_year, "%04d", c_year);
			sprintf(rm_month, "%02d", c_month-(ffcs_log_file_0->_saved_month+1));
		}

		/*删除*/
		if (strcmp(rm_month, "12") == 0) {
			sprintf(rm_dir, "%s%s", ffcs_log_file_0->_parentdir, rm_year);
		} else {
			sprintf(rm_dir, "%s%s/%s", ffcs_log_file_0->_parentdir, rm_year, rm_month);
		}

		printf("*** 系统自动清理过期日志[%s] ***\n", rm_dir);

		fun_logger_rmdir(rm_dir);

		rqt.tv_sec = 300; /*300秒 = 5分钟*/
		rqt.tv_nsec = 0;
		nanosleep(&rqt,NULL);
	}

	printf("*** 日志清理线程(%ld)退出 ***\n", pthread_self()&0xFFFF);

	pthread_exit(NULL);
}



/*
int main(int argc,char *argv[]) {
	int i;

	FFCS_LOG_FILE _log_file1;
	fun_logger_init(&_log_file1, "log", FFCS_LOGMEDIA_ALL, FFCS_LOGLEVEL_DEBUG, FFCS_LOGMULTI_ASYN, FFCS_LOGCLEANER_OPEN, 3);

//	FFCS_LOG_FILE _log_file2;
//	fun_logger_init(&_log_file2, "log2", FFCS_LOGMEDIA_ALL, FFCS_LOGLEVEL_DEBUG);
	do {
		//dbg_printf("%02X, %02X", 1, 2);

		fun_logger_error(&_log_file1, "This is 测试 = %d\n", i);
		fun_logger_warn(&_log_file1, "This is 测试 = %d\n", i);
		fun_logger_message(&_log_file1, "This is 测试 = %d\n", i);
		fun_logger_debug(&_log_file1, "This is 测试 = %d\n", i);
		fun_logger_debug(&_log_file1, "This is 测试 = %d\n", i);

//		fun_logger_error(&_log_file2, "This is test = %d\n", i);
//		fun_logger_warn(&_log_file2, "This is test = %d\n", i);
//		fun_logger_message(&_log_file2, "This is test = %d\n", i);
//		fun_logger_debug(&_log_file2, "This is test = %d\n", i);
	} while (0);

	while(1){
		sleep(3);
	}

	fun_logger_close(&_log_file1);
//	fun_logger_close(&_log_file2);

	return 1;
}
*/


