#ifndef _LOGADMIN_H_
#define _LOGADMIN_H_
#include "ffcs_logger.h"
#include <sys/stat.h>


int InitLogger(const char *sLogPath);
void CloseLog();

extern FFCS_LOG_FILE g_log_file;
#define WriteLogByLevel(tp, fmt, ...)	(tp==3)?fun_logger_error(&g_log_file,fmt,##__VA_ARGS__):	\
										(tp==2)?fun_logger_message(&g_log_file,fmt,##__VA_ARGS__):	\
												fun_logger_debug(&g_log_file,fmt,##__VA_ARGS__)
								
#endif
