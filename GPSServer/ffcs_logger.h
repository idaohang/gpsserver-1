#ifndef FFCS_LOGGER_H_
#define FFCS_LOGGER_H_

/*#define DEBUG 1*/

#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>
#include "ffcs_logger_queue.h"

#ifdef	__cplusplus
extern "C"{
#endif



/*日志级别 - 控制日志显示的级别*/
#define FFCS_LOGLEVEL_ERROR		0x00 /*错误*/
#define FFCS_LOGLEVEL_WARN		0x01 /*警告*/
#define FFCS_LOGLEVEL_MESSAGE	0x02 /*提示*/
#define FFCS_LOGLEVEL_DEBUG		0x03 /*调试*/

/*日志媒体 - 控制日志输出的媒体*/
#define FFCS_LOGMEDIA_SCREEN	0x00 /*屏幕*/
#define FFCS_LOGMEDIA_FILE		0x01 /*文件*/
#define FFCS_LOGMEDIA_ALL		0x02 /*全部*/

/*日志输出模式 - 控制日志输出采用同步模式还是异步模式*/
#define FFCS_LOGMULTI_SYN		0x00 /*同步，单线程模式*/
#define FFCS_LOGMULTI_ASYN		0x01 /*异步，多线程模式*/

/*日志队列状态*/
#define FFCS_LOGQSTATUS_OPEN	0x00 /*日志队列正常，允许写入日志*/
#define FFCS_LOGQSTATUS_CLOSED	0x01 /*日志队列满或异常，不允许写入日志*/

/*日志队列状态*/
#define FFCS_LOGCLEANER_CLOSED	0x00 /*日志清理关*/
#define FFCS_LOGCLEANER_OPEN	0x01 /*日志清理开*/

#define FFCS_LOGQ_MAXSIZE		1024*4 /*日志队列满负荷能力*/

/*日志文件*/
#define FFCS_LOG_MAX_NAMESIZE 255 /*最大文件名长度*/

#define FFCS_LOG_FILEDIR "log" /*默认文件根目录*/
#define FFCS_LOG_FILENAME "system_log" /*默认日志文件名*/
#define FFCS_LOG_FILETYPE ".log" /*默认扩展名*/
#define FFCS_LOG_MAX_FILESIZE 32*1024*1024 /*最大日志文件大小32M*/

/*
#define dbg_printf(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

#define fun_logger_error(lf, fmt, ...) fun_logger_error_extend(lf,__FILE__,__LINE__,__FUNCTION__,fmt,##__VA_ARGS__)
#define fun_logger_warn(lf, fmt, ...) fun_logger_warn_extend(lf,__FILE__,__LINE__,__FUNCTION__,fmt,##__VA_ARGS__)
#define fun_logger_message(lf, fmt, ...) fun_logger_message_extend(lf,__FILE__,__LINE__,__FUNCTION__,fmt,##__VA_ARGS__)
#define fun_logger_debug(lf, fmt, ...) fun_logger_debug_extend(lf,__FILE__,__LINE__,__FUNCTION__,fmt,##__VA_ARGS__)
*/

/*
#define WriteLogByLevel(tp, fmt, ...)	(tp==3)?fun_logger_error(&g_log_file,fmt,##__VA_ARGS__):	\
										(tp==2)?fun_logger_message(&g_log_file,fmt,##__VA_ARGS__):	\
												fun_logger_debug(&g_log_file,fmt,##__VA_ARGS__)
*/

#define fun_perror(msg){										\
	char err[128];												\
	memset(err, '\0', sizeof(err));								\
	sprintf(err, "[%s:%d]->%s", __FILE__, __LINE__, msg);		\
	perror(err);												\
}



#ifdef HP_UNIX
	#pragma pack 1
#elif defined(AIX)
	#pragma options align=packed
#elif defined(_WIN32)
	#pragma pack(push, 1)
#else
	#pragma pack(1)
#endif

typedef struct{
	unsigned int _media; /*日志媒体 默认=FFCS_LOGMEDIA_SCREEN*/
	unsigned int _level; /*日志级别 默认=FFCS_LOGLEVEL_MESSAGE*/
	unsigned int _multithread; /*日志输出模式 默认=FFCS_LOGMULTI_SYN*/
}FFCS_LOG_TYPE;

typedef struct{
	char _parentdir[FFCS_LOG_MAX_NAMESIZE]; /*日志文件根目录*/
	char _filedir[FFCS_LOG_MAX_NAMESIZE]; /*日志文件目录, 包括根目录、子目录*/
	char _filename[FFCS_LOG_MAX_NAMESIZE]; /*日志文件名*/
	char _fullpath[FFCS_LOG_MAX_NAMESIZE*2]; /*日志文件全路径名，包括目录和文件名*/

	FILE * _fp; /*日志文件指针*/

	pthread_t _pt_logger; /*日志线程标识*/

	pthread_t _pt_cleaner; /*日志清理线程标识*/

	LogLinkQueue _LogQ_SEND; /*日志输出队列*/
	unsigned int _LogQ_Status; /*日志队列状态，=FFCS_LOGQSTATUS_OPEN允许写入，=FFCS_LOGQSTATUS_CLOSED禁止写入*/

	FFCS_LOG_TYPE _log_type;

	int _cleaner_switch; /*日志清理开关，=1时有效*/
	int _saved_month; /*日志保存月数，=0表示不删除，要求小于120*/
}FFCS_LOG_FILE;

#ifdef HP_UNIX
	#pragma pack 4
#elif defined(AIX)
	#pragma options align=reset
#elif defined(_WIN32)
	#pragma pack(pop)
#else
	#pragma pack()
#endif



/*全局变量*/
/*
#ifndef G_LOG_FILE_
	#define G_LOG_FILE_
	FFCS_LOG_FILE g_log_file;
#else
	extern FFCS_LOG_FILE g_log_file;
#endif
*/


/*-----------------------------------------------------------------------
 * name:	屏幕打印
 * input:	fmt  -- 带格式字符串
 * 			ap   -- 不定参数列表
 * output:
 * return:	 -1 -- 失败
 * 			>=0 -- 输出字符数
 *-----------------------------------------------------------------------*/
int fun_logger_screen(const char * fmt, va_list ap);

/*-----------------------------------------------------------------------
 * name:	文件打印
 * input:	_lf  -- FFCS_LOG_FILE结构
 * 			fmt  -- 带格式字符串
 * 			ap   -- 不定参数列表
 * output:
 * return:	 -1 -- 失败
 * 			>=0 -- 输出字符数
 *-----------------------------------------------------------------------*/
int fun_logger_file(FFCS_LOG_FILE * _lf, const char * fmt, va_list ap);

/*-----------------------------------------------------------------------
 * name:	系统初始化
 * 			每次系统调用此日志类时 需要调用此函数初始化日志系统
 * input:	fdir  -- 日志文件目录
 * 			...  -- 支持5个参数：日志媒体、日志级别、日志输出模式、日志清理开关、日志保存月数
 * output:	_lf  -- FFCS_LOG_FILE结构
 * return:	-1 -- 失败
 * 			=1 -- 成功
 *-----------------------------------------------------------------------*/
int fun_logger_init(FFCS_LOG_FILE * _lf, const char * _fdir, ...);

/*-----------------------------------------------------------------------
 * name:	error日志
 * input:	_lf  -- FFCS_LOG_FILE结构
 * 			fmt  -- 带格式字符串
 * 			...  -- 不定参数列表
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_error(FFCS_LOG_FILE * _lf, const char * fmt, ...);

/*-----------------------------------------------------------------------
 * name:	warn日志
 * input:	_lf  -- FFCS_LOG_FILE结构
 * 			fmt  -- 带格式字符串
 * 			...  -- 不定参数列表
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_warn(FFCS_LOG_FILE * _lf, const char * fmt, ...);

/*-----------------------------------------------------------------------
 * name:	message日志
 * input:	_lf  -- FFCS_LOG_FILE结构
 * 			fmt  -- 带格式字符串
 * 			...  -- 不定参数列表
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_message(FFCS_LOG_FILE * _lf, const char * fmt, ...);

/*-----------------------------------------------------------------------
 * name:	message日志
 * input:	_lf  -- FFCS_LOG_FILE结构
 * 			fmt  -- 带格式字符串
 * 			...  -- 不定参数列表
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_debug(FFCS_LOG_FILE * _lf, const char * fmt, ...);

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
void fun_logger_error_extend(FFCS_LOG_FILE * _lf, const char * _file_, int _line_, const char * _fun_, const char * fmt, ...);

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
void fun_logger_warn_extend(FFCS_LOG_FILE * _lf, const char * _file_, int _line_, const char * _fun_, const char * fmt, ...);

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
void fun_logger_message_extend(FFCS_LOG_FILE * _lf, const char * _file_, int _line_, const char * _fun_, const char * fmt, ...);

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
void fun_logger_debug_extend(FFCS_LOG_FILE * _lf, const char * _file_, int _line_, const char * _fun_, const char * fmt, ...);

/*-----------------------------------------------------------------------
 * name:	以指定格式打印，不增加其他信息
 * input:	_lf  -- FFCS_LOG_FILE结构
 * 			fmt  -- 带格式字符串
 * 			...  -- 不定参数列表
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_printf(FFCS_LOG_FILE * _lf, const char * fmt, ...);

/*-----------------------------------------------------------------------
 * name:	关闭日志
 * input:	_lf  -- FFCS_LOG_FILE结构
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_close(FFCS_LOG_FILE * _lf);

/*-----------------------------------------------------------------------
 * name:	日志处理线程
 * input:
 * output:	arg  -- FFCS_LOG_FILE结构参数
 * return:
 *-----------------------------------------------------------------------*/
void * pthr_logger_process(void * arg);

/*-----------------------------------------------------------------------
 * name:	日志清理线程
 * input:
 * output:	arg  -- FFCS_LOG_FILE结构参数
 * return:
 *-----------------------------------------------------------------------*/
void * pthr_cleaner_process(void * arg);



#ifdef	__cplusplus
}
#endif

#endif /*FFCS_LOGGER_H_*/
