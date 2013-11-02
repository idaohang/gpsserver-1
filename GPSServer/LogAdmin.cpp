#include "LogAdmin.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

FFCS_LOG_FILE g_log_file;

int InitLogger(const char *sLogPath)
{
	if (sLogPath == NULL)
	{
		printf("ERROR: LOG PATH IS NULL\n");
		return -1;
	}
	
	return fun_logger_init(&g_log_file, sLogPath, FFCS_LOGMEDIA_FILE, FFCS_LOGLEVEL_DEBUG, FFCS_LOGMULTI_ASYN, FFCS_LOGCLEANER_OPEN, 3);	
}

void CloseLog()
{
	return fun_logger_close(&g_log_file);
}
