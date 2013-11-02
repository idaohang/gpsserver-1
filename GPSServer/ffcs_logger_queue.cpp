/*==================================================================================================
 * 项目名称: 队列
 *    功能: 实现日志队列接口
 *    作者: huangjf
 *    联系: huangjf@ffcs.cn
 * 最近修改: 2008-10-6
 *    版本: v1.0.0
  ==================================================================================================*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ffcs_logger_queue.h"


/*-----------------------------------------------------------------------
 * name:	初始化队列
 * input:	Q  -- 队列类型
 * output:
 * return:	>0 -- 成功
 * 			-1 -- 失败
 *-----------------------------------------------------------------------*/
int fun_log_queue_init(LogLinkQueue * Q){
	Q->root = Q->rear = (LogQueueNode *)malloc(sizeof(LogQueueNode));
	if (!Q->root) return -1;
	memset(&Q->root->data, 0, sizeof(LogQueueData));
	Q->root->next = NULL;
	return 1;
}


/*-----------------------------------------------------------------------
 * name:	释放队列资源
 * input:	Q  -- 队列类型
 * output:
 * return:	>0 -- 成功
 * 			-1 -- 失败
 *-----------------------------------------------------------------------*/
int fun_log_queue_destroy(LogLinkQueue * Q){
	fun_log_queue_clear(Q);
	if (Q->root) free(Q->root);
	return 1;
}


/*-----------------------------------------------------------------------
 * name:	清空队列节点
 * input:	Q  -- 队列类型
 * output:
 * return:	>=0 -- 清除的个数
 * 			 -1 -- 失败
 *-----------------------------------------------------------------------*/
int fun_log_queue_clear(LogLinkQueue * Q){
	int num = 0;
	if (Q->root==Q->rear) return 0;
	while(Q->root->next){
		Q->rear = Q->root->next;
		Q->root->next = Q->rear->next;
		if (Q->rear) free(Q->rear);
		num++;
	}
	Q->rear = Q->root;
	return num;
}


/*-----------------------------------------------------------------------
 * name:	返回队列长度
 * input:	Q  -- 队列类型
 * output:
 * return:	=0 -- 空
 * 			>0 -- 节点个数
 *-----------------------------------------------------------------------*/
int fun_log_queue_length(LogLinkQueue * Q){
	int size = 0;
	LogQueueNode * node;
	if (Q->root==Q->rear) return 0;
	node = Q->root->next;
	while(node){
		size++;
		node = node->next;
	}
	return size;
}


/*-----------------------------------------------------------------------
 * name:	返回队列头个节点的数据
 * input:	Q  -- 队列类型
 * output:	data  -- QueueData数据结构
 * return:	>0 -- 成功
 * 			=0 -- 表示队列已为空
 * 			-1 -- 失败
 *-----------------------------------------------------------------------*/
int fun_log_queue_out(LogLinkQueue * Q, LogQueueData * pdata){
	LogQueueNode * node;
	if (Q->root==Q->rear) return 0;
	if (pdata==NULL) return -1;
	node = Q->root->next;
	if (!node) return -1;
	Q->root->next = node->next;
	memcpy(pdata, &node->data, sizeof(LogQueueData));
	if (node) free(node);
	if (!Q->root->next) {
		Q->rear=Q->root;
		return 0;
	}
	return 1;
}


/*-----------------------------------------------------------------------
 * name:	添加数据到队列尾节点
 * input:	Q  -- 队列类型
 * 			data  -- QueueData数据结构
 * output:
 * return:	>0 -- 成功
 * 			-1 -- 失败
 *-----------------------------------------------------------------------*/
int fun_log_queue_in(LogLinkQueue * Q, LogQueueData * pdata){
	LogQueueNode * node = (LogQueueNode *)malloc(sizeof(LogQueueNode));
	if (!node) return -1;
	memcpy(&node->data, pdata, sizeof(LogQueueData));
	node->next = NULL;
	Q->rear->next = node;
	Q->rear = node;
	return 1;
}


/*-----------------------------------------------------------------------
 * name:	删除指定节点
 * input:	Q  -- 队列类型
 * 			idx  -- 索引，从0开始
 * output:
 * return:	>0 -- 成功
 * 			=0 -- 表示队列已为空
 * 			-1 -- 失败
 *-----------------------------------------------------------------------*/
int fun_log_queue_del(LogLinkQueue * Q, int idx){
	int i;
	LogQueueNode * node;
	LogQueueNode * temp;
	if (Q->root==Q->rear) return 0;
	node = Q->root;
	for(i=0; (node!=NULL) && (i<idx); i++){
		node = node->next;
	}
	if (!node) return -1;
	temp = node->next;
	if (!temp) return -1;
	node->next = temp->next;
	if (temp) free(temp);
	if (!Q->root->next) {
		Q->rear=Q->root;
		return 0;
	}
	return i;
}


/*-----------------------------------------------------------------------
 * name:	查找指定节点
 * input:	Q  -- 队列类型
 * 			pdata  -- 查找对象
 * output:
 * return:	>=0 -- 对象索引
 * 			 -1 -- 失败
 *-----------------------------------------------------------------------*/
int fun_log_queue_find(LogLinkQueue * Q, LogQueueData * pdata){
	int size = 0;
	LogQueueNode * node;
	if (Q->root==Q->rear) return -1;
	node = Q->root->next;
	while(node){
		if (!memcmp(&node->data, pdata, sizeof(LogQueueData))) return size;
		size++;
		node = node->next;
	}
	return -1;
}


/*-----------------------------------------------------------------------
 * name:	查找指定节点，并输出到缓存，节点不移出队列
 * input:	Q  -- 队列类型
 * 			idx  -- 节点索引
 *
 * output:	pdata  -- 查找对象
 * return:	>=0 -- 对象索引
 * 			 -1 -- 失败
 *-----------------------------------------------------------------------*/
int fun_log_queue_findbyidx(LogLinkQueue * Q, int idx, LogQueueData ** pdata){
	int i;
	LogQueueNode * node;
	if (Q->root==Q->rear) return -1;
	node = Q->root;
	for(i=0; (node!=NULL) && (i<=idx); i++){
		node = node->next;
	}
	if (!node) return -1;
	if (node) *pdata = &node->data;
	return i;
}


/*-----------------------------------------------------------------------
 * name:	取指定节点，同时将节点移出队列
 * input:	Q  -- 队列类型
 * 			idx  -- 节点索引
 *
 * output:	pdata  -- 查找对象
 * return:	>=0 -- 对象索引
 * 			 -1 -- 失败
 *-----------------------------------------------------------------------*/
int fun_log_queue_get(LogLinkQueue * Q, int idx, LogQueueData * pdata){
	int i;
	LogQueueNode * node;
	LogQueueNode * temp;
	if (Q->root==Q->rear) return -1;
	node = Q->root;
	for(i=0; (node!=NULL) && (i<idx); i++){
		node = node->next;
	}
	if (!node) return -1;
	temp = node->next;
	if (!temp) return -1;
	node->next = temp->next;
	if (temp) memcpy(pdata, &temp->data, sizeof(LogQueueData));
	if (temp) free(temp);
	if (!Q->root->next) Q->rear=Q->root;
	return i;
}


/*
int main(int argc, char * argv[]){
	char str1[] = "hello";
	char str2[] = "world";

	LogQueueData * data1;
	LogQueueData * data2;

	data1 = (LogQueueData *)malloc(sizeof(LogQueueData));
	memset(data1->str, '\0', sizeof(data1->str));
	strncpy(data1->str, str1, strlen(str1));
	printf("data1->str=%s ", data1->str);

	data2 = (LogQueueData *)malloc(sizeof(LogQueueData));
	memset(data2->str, '\0', sizeof(data2->str));
	strncpy(data2->str, str2, strlen(str2));
	printf("data2->str=%s ", data2->str);

	LogLinkQueue link_queue;
	LogLinkQueue * Q = &link_queue;
	fun_log_queue_init(Q);

	fun_log_queue_in(Q, data1);
	fun_log_queue_in(Q, data2);
	fun_log_queue_clear(Q);
	printf("queue_length=%d \n", fun_log_queue_length(Q));

	fun_log_queue_in(Q, data1);
	fun_log_queue_in(Q, data2);
	printf("queue_length=%d \n", fun_log_queue_length(Q));



	LogQueueData * pdata = (LogQueueData *)malloc(sizeof(LogQueueData));

	if (fun_log_queue_out(Q, pdata)>0)
		printf("=%s \n", pdata->str);

	if (fun_log_queue_out(Q, pdata)>0)
		printf("=%s \n", pdata->str);
	printf("queue_length=%d \n", fun_log_queue_length(Q));



	fun_log_queue_in(Q, data1);
	fun_log_queue_in(Q, data2);
	printf("queue_length=%d \n", fun_log_queue_length(Q));
	printf("del=%d ", fun_log_queue_del(Q, 1));
	printf("del=%d ", fun_log_queue_del(Q, 1));
	printf("del=%d ", fun_log_queue_del(Q, 0));
	printf("del=%d \n", fun_log_queue_del(Q, 0));
	printf("queue_length=%d \n", fun_log_queue_length(Q));



	fun_log_queue_clear(Q);
	fun_log_queue_in(Q, data1);
	fun_log_queue_in(Q, data2);
	fun_log_queue_in(Q, data1);
	fun_log_queue_in(Q, data2);
	printf("queue_length=%d \n", fun_log_queue_length(Q));

	printf("find=%d ", fun_log_queue_find(Q, data1));
	printf("find=%d \n", fun_log_queue_find(Q, data2));
	printf("del=%d ", fun_log_queue_del(Q, 0));
	printf("find=%d ", fun_log_queue_find(Q, data1));
	printf("find=%d \n", fun_log_queue_find(Q, data2));
	printf("del=%d ", fun_log_queue_del(Q, 1));
	printf("find=%d ", fun_log_queue_find(Q, data1));
	printf("find=%d \n", fun_log_queue_find(Q, data2));

	free(data1);
	free(data2);
	free(pdata);
	fun_log_queue_destroy(Q);
	return 1;
}
*/
