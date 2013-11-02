/****************************************************************
 * 名   称 : FFType.h	
 * 主   题 : 类型定义
 * 注   明 : FFSC New Create     2009/02/10        
 *           Copyright by FFSC
****************************************************************/
#ifndef _FF_TYPE_H_
#define _FF_TYPE_H_

/**/
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/timeb.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/select.h>

#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <ctype.h>
#include <semaphore.h>
#include <netdb.h>
#include <fcntl.h>
#include <math.h>

#include <net/if.h>
#include <net/if_arp.h>

#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>


/***************************************************************
类型定义
****************************************************************/
typedef		char					ff_char;

typedef		unsigned char			ff_uchar;

typedef		short					ff_short;

typedef		unsigned short			ff_ushort;

typedef		int						ff_int;

typedef		unsigned int			ff_uint;

typedef		long					ff_long;

typedef		unsigned long			ff_ulong;

typedef		long long				ff_llong;

typedef		unsigned long long		ff_ullong;

typedef		time_t					ff_time;

typedef		fd_set					ff_fdset;

typedef		struct  tm				ff_tm;

typedef		struct	timeval			ff_timeval;

typedef		struct 	timespec		ff_timespec;

typedef		struct sockaddr_in 		ff_sockaddr_in;

typedef		struct sockaddr			ff_sockaddr;

typedef		struct stat     		ff_stat;

typedef		sem_t 					ff_sem;



/**************************************************************/

#endif
