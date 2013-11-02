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



/*��־���� - ������־��ʾ�ļ���*/
#define FFCS_LOGLEVEL_ERROR		0x00 /*����*/
#define FFCS_LOGLEVEL_WARN		0x01 /*����*/
#define FFCS_LOGLEVEL_MESSAGE	0x02 /*��ʾ*/
#define FFCS_LOGLEVEL_DEBUG		0x03 /*����*/

/*��־ý�� - ������־�����ý��*/
#define FFCS_LOGMEDIA_SCREEN	0x00 /*��Ļ*/
#define FFCS_LOGMEDIA_FILE		0x01 /*�ļ�*/
#define FFCS_LOGMEDIA_ALL		0x02 /*ȫ��*/

/*��־���ģʽ - ������־�������ͬ��ģʽ�����첽ģʽ*/
#define FFCS_LOGMULTI_SYN		0x00 /*ͬ�������߳�ģʽ*/
#define FFCS_LOGMULTI_ASYN		0x01 /*�첽�����߳�ģʽ*/

/*��־����״̬*/
#define FFCS_LOGQSTATUS_OPEN	0x00 /*��־��������������д����־*/
#define FFCS_LOGQSTATUS_CLOSED	0x01 /*��־���������쳣��������д����־*/

/*��־����״̬*/
#define FFCS_LOGCLEANER_CLOSED	0x00 /*��־�����*/
#define FFCS_LOGCLEANER_OPEN	0x01 /*��־����*/

#define FFCS_LOGQ_MAXSIZE		1024*4 /*��־��������������*/

/*��־�ļ�*/
#define FFCS_LOG_MAX_NAMESIZE 255 /*����ļ�������*/

#define FFCS_LOG_FILEDIR "log" /*Ĭ���ļ���Ŀ¼*/
#define FFCS_LOG_FILENAME "system_log" /*Ĭ����־�ļ���*/
#define FFCS_LOG_FILETYPE ".log" /*Ĭ����չ��*/
#define FFCS_LOG_MAX_FILESIZE 32*1024*1024 /*�����־�ļ���С32M*/

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
	unsigned int _media; /*��־ý�� Ĭ��=FFCS_LOGMEDIA_SCREEN*/
	unsigned int _level; /*��־���� Ĭ��=FFCS_LOGLEVEL_MESSAGE*/
	unsigned int _multithread; /*��־���ģʽ Ĭ��=FFCS_LOGMULTI_SYN*/
}FFCS_LOG_TYPE;

typedef struct{
	char _parentdir[FFCS_LOG_MAX_NAMESIZE]; /*��־�ļ���Ŀ¼*/
	char _filedir[FFCS_LOG_MAX_NAMESIZE]; /*��־�ļ�Ŀ¼, ������Ŀ¼����Ŀ¼*/
	char _filename[FFCS_LOG_MAX_NAMESIZE]; /*��־�ļ���*/
	char _fullpath[FFCS_LOG_MAX_NAMESIZE*2]; /*��־�ļ�ȫ·����������Ŀ¼���ļ���*/

	FILE * _fp; /*��־�ļ�ָ��*/

	pthread_t _pt_logger; /*��־�̱߳�ʶ*/

	pthread_t _pt_cleaner; /*��־�����̱߳�ʶ*/

	LogLinkQueue _LogQ_SEND; /*��־�������*/
	unsigned int _LogQ_Status; /*��־����״̬��=FFCS_LOGQSTATUS_OPEN����д�룬=FFCS_LOGQSTATUS_CLOSED��ֹд��*/

	FFCS_LOG_TYPE _log_type;

	int _cleaner_switch; /*��־�����أ�=1ʱ��Ч*/
	int _saved_month; /*��־����������=0��ʾ��ɾ����Ҫ��С��120*/
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



/*ȫ�ֱ���*/
/*
#ifndef G_LOG_FILE_
	#define G_LOG_FILE_
	FFCS_LOG_FILE g_log_file;
#else
	extern FFCS_LOG_FILE g_log_file;
#endif
*/


/*-----------------------------------------------------------------------
 * name:	��Ļ��ӡ
 * input:	fmt  -- ����ʽ�ַ���
 * 			ap   -- ���������б�
 * output:
 * return:	 -1 -- ʧ��
 * 			>=0 -- ����ַ���
 *-----------------------------------------------------------------------*/
int fun_logger_screen(const char * fmt, va_list ap);

/*-----------------------------------------------------------------------
 * name:	�ļ���ӡ
 * input:	_lf  -- FFCS_LOG_FILE�ṹ
 * 			fmt  -- ����ʽ�ַ���
 * 			ap   -- ���������б�
 * output:
 * return:	 -1 -- ʧ��
 * 			>=0 -- ����ַ���
 *-----------------------------------------------------------------------*/
int fun_logger_file(FFCS_LOG_FILE * _lf, const char * fmt, va_list ap);

/*-----------------------------------------------------------------------
 * name:	ϵͳ��ʼ��
 * 			ÿ��ϵͳ���ô���־��ʱ ��Ҫ���ô˺�����ʼ����־ϵͳ
 * input:	fdir  -- ��־�ļ�Ŀ¼
 * 			...  -- ֧��5����������־ý�塢��־������־���ģʽ����־�����ء���־��������
 * output:	_lf  -- FFCS_LOG_FILE�ṹ
 * return:	-1 -- ʧ��
 * 			=1 -- �ɹ�
 *-----------------------------------------------------------------------*/
int fun_logger_init(FFCS_LOG_FILE * _lf, const char * _fdir, ...);

/*-----------------------------------------------------------------------
 * name:	error��־
 * input:	_lf  -- FFCS_LOG_FILE�ṹ
 * 			fmt  -- ����ʽ�ַ���
 * 			...  -- ���������б�
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_error(FFCS_LOG_FILE * _lf, const char * fmt, ...);

/*-----------------------------------------------------------------------
 * name:	warn��־
 * input:	_lf  -- FFCS_LOG_FILE�ṹ
 * 			fmt  -- ����ʽ�ַ���
 * 			...  -- ���������б�
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_warn(FFCS_LOG_FILE * _lf, const char * fmt, ...);

/*-----------------------------------------------------------------------
 * name:	message��־
 * input:	_lf  -- FFCS_LOG_FILE�ṹ
 * 			fmt  -- ����ʽ�ַ���
 * 			...  -- ���������б�
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_message(FFCS_LOG_FILE * _lf, const char * fmt, ...);

/*-----------------------------------------------------------------------
 * name:	message��־
 * input:	_lf  -- FFCS_LOG_FILE�ṹ
 * 			fmt  -- ����ʽ�ַ���
 * 			...  -- ���������б�
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_debug(FFCS_LOG_FILE * _lf, const char * fmt, ...);

/*-----------------------------------------------------------------------
 * name:	error��־��֧�ִ�ӡ������
 * input:	_lf  -- FFCS_LOG_FILE�ṹ
 * 			_file_  -- ��־�ļ����꣺__FILE__��
 * 			_line_  -- ��־�У��꣺__LINE__��
 * 			_fun_  -- ��־���������꣺__FUNCTION__��
 * 			fmt  -- ����ʽ�ַ���
 * 			...  -- ���������б�
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_error_extend(FFCS_LOG_FILE * _lf, const char * _file_, int _line_, const char * _fun_, const char * fmt, ...);

/*-----------------------------------------------------------------------
 * name:	warn��־��֧�ִ�ӡ������
 * input:	_lf  -- FFCS_LOG_FILE�ṹ
 * 			_file_  -- ��־�ļ����꣺__FILE__��
 * 			_line_  -- ��־�У��꣺__LINE__��
 * 			_fun_  -- ��־���������꣺__FUNCTION__��
 * 			fmt  -- ����ʽ�ַ���
 * 			...  -- ���������б�
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_warn_extend(FFCS_LOG_FILE * _lf, const char * _file_, int _line_, const char * _fun_, const char * fmt, ...);

/*-----------------------------------------------------------------------
 * name:	messag��־��֧�ִ�ӡ������
 * input:	_lf  -- FFCS_LOG_FILE�ṹ
 * 			_file_  -- ��־�ļ����꣺__FILE__��
 * 			_line_  -- ��־�У��꣺__LINE__��
 * 			_fun_  -- ��־���������꣺__FUNCTION__��
 * 			fmt  -- ����ʽ�ַ���
 * 			...  -- ���������б�
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_message_extend(FFCS_LOG_FILE * _lf, const char * _file_, int _line_, const char * _fun_, const char * fmt, ...);

/*-----------------------------------------------------------------------
 * name:	debug��־��֧�ִ�ӡ������
 * input:	_lf  -- FFCS_LOG_FILE�ṹ
 * 			_file_  -- ��־�ļ����꣺__FILE__��
 * 			_line_  -- ��־�У��꣺__LINE__��
 * 			_fun_  -- ��־���������꣺__FUNCTION__��
 * 			fmt  -- ����ʽ�ַ���
 * 			...  -- ���������б�
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_debug_extend(FFCS_LOG_FILE * _lf, const char * _file_, int _line_, const char * _fun_, const char * fmt, ...);

/*-----------------------------------------------------------------------
 * name:	��ָ����ʽ��ӡ��������������Ϣ
 * input:	_lf  -- FFCS_LOG_FILE�ṹ
 * 			fmt  -- ����ʽ�ַ���
 * 			...  -- ���������б�
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_printf(FFCS_LOG_FILE * _lf, const char * fmt, ...);

/*-----------------------------------------------------------------------
 * name:	�ر���־
 * input:	_lf  -- FFCS_LOG_FILE�ṹ
 * output:
 * return:
 *-----------------------------------------------------------------------*/
void fun_logger_close(FFCS_LOG_FILE * _lf);

/*-----------------------------------------------------------------------
 * name:	��־�����߳�
 * input:
 * output:	arg  -- FFCS_LOG_FILE�ṹ����
 * return:
 *-----------------------------------------------------------------------*/
void * pthr_logger_process(void * arg);

/*-----------------------------------------------------------------------
 * name:	��־�����߳�
 * input:
 * output:	arg  -- FFCS_LOG_FILE�ṹ����
 * return:
 *-----------------------------------------------------------------------*/
void * pthr_cleaner_process(void * arg);



#ifdef	__cplusplus
}
#endif

#endif /*FFCS_LOGGER_H_*/
