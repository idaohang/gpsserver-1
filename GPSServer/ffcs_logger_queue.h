#ifndef FFCS_MENU_QUEUE_H_
#define FFCS_MENU_QUEUE_H_

/*定义操作系统*/
#define LINUX
/*
#define AIX
#define HP_UNIX
#define _WIN32
*/

#ifdef	__cplusplus
extern "C"{
#endif



#ifdef HP_UNIX
	#pragma pack 1
#elif defined(AIX)
	#pragma options align=packed
#elif defined(_WIN32)
	#pragma pack(push, 1)
#else
	#pragma pack(1)
#endif

/*队列节点数据结构 菜单群组结构*/
typedef struct{
	char _buff[1024]; /*待写入日志数据缓存*/
	int _size; /*待写入日志数据长度*/
}LogQueueData;

/*队列节点类型*/
typedef struct LogQueueNode{
	LogQueueData data;
	struct LogQueueNode * next;
}LogQueueNode;

/*队列类型*/
typedef struct{
	LogQueueNode * root;
	LogQueueNode * rear;
}LogLinkQueue;

#ifdef HP_UNIX
	#pragma pack 4
#elif defined(AIX)
	#pragma options align=reset
#elif defined(_WIN32)
	#pragma pack(pop)
#else
	#pragma pack()
#endif



/*-----------------------------------------------------------------------
 * name:	初始化队列
 * input:	Q  -- 队列类型
 * output:
 * return:	>0 -- 成功
 * 			-1 -- 失败
 *-----------------------------------------------------------------------*/
int fun_log_queue_init(LogLinkQueue * Q);

/*-----------------------------------------------------------------------
 * name:	释放队列资源
 * input:	Q  -- 队列类型
 * output:
 * return:	>0 -- 成功
 * 			-1 -- 失败
 *-----------------------------------------------------------------------*/
int fun_log_queue_destroy(LogLinkQueue * Q);

/*-----------------------------------------------------------------------
 * name:	清空队列节点
 * input:	Q  -- 队列类型
 * output:
 * return:	>=0 -- 清除的个数
 * 			 -1 -- 失败
 *-----------------------------------------------------------------------*/
int fun_log_queue_clear(LogLinkQueue * Q);

/*-----------------------------------------------------------------------
 * name:	返回队列长度
 * input:	Q  -- 队列类型
 * output:
 * return:	=0 -- 空
 * 			>0 -- 节点个数
 *-----------------------------------------------------------------------*/
int fun_log_queue_length(LogLinkQueue * Q);

/*-----------------------------------------------------------------------
 * name:	返回队列头个节点的数据
 * input:	Q  -- 队列类型
 * output:	data  -- QueueData数据结构
 * return:	>0 -- 成功
 * 			-1 -- 失败
 *-----------------------------------------------------------------------*/
int fun_log_queue_out(LogLinkQueue * Q, LogQueueData * data);

/*-----------------------------------------------------------------------
 * name:	添加数据到队列尾节点
 * input:	Q  -- 队列类型
 * 			data  -- QueueData数据结构
 * output:
 * return:	>0 -- 成功
 * 			-1 -- 失败
 *-----------------------------------------------------------------------*/
int fun_log_queue_in(LogLinkQueue * Q, LogQueueData * data);

/*-----------------------------------------------------------------------
 * name:	删除指定节点
 * input:	Q  -- 队列类型
 * 			idx  -- 索引，从0开始
 * output:
 * return:	>0 -- 成功
 * 			-1 -- 失败
 *-----------------------------------------------------------------------*/
int fun_log_queue_del(LogLinkQueue * Q, int idx);

/*-----------------------------------------------------------------------
 * name:	查找指定节点
 * input:	Q  -- 队列类型
 * 			pdata  -- 查找对象
 * output:
 * return:	>=0 -- 对象索引
 * 			 -1 -- 失败
 *-----------------------------------------------------------------------*/
int fun_log_queue_find(LogLinkQueue * Q, LogQueueData * pdata);

/*-----------------------------------------------------------------------
 * name:	查找指定节点，并输出到缓存，节点不移出队列
 * input:	Q  -- 队列类型
 * 			idx  -- 节点索引
 *
 * output:	pdata  -- 查找对象
 * return:	>=0 -- 对象索引
 * 			 -1 -- 失败
 *-----------------------------------------------------------------------*/
int fun_log_queue_findbyidx(LogLinkQueue * Q, int idx, LogQueueData ** pdata);

/*-----------------------------------------------------------------------
 * name:	取指定节点
 * input:	Q  -- 队列类型
 * 			idx  -- 节点索引
 *
 * output:	pdata  -- 查找对象
 * return:	>=0 -- 对象索引
 * 			 -1 -- 失败
 *-----------------------------------------------------------------------*/
int fun_log_queue_get(LogLinkQueue * Q, int idx, LogQueueData * pdata);


#ifdef	__cplusplus
}
#endif



#endif /*FFCS_MENU_QUEUE_H_*/
