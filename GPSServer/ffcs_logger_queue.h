#ifndef FFCS_MENU_QUEUE_H_
#define FFCS_MENU_QUEUE_H_

/*�������ϵͳ*/
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

/*���нڵ����ݽṹ �˵�Ⱥ��ṹ*/
typedef struct{
	char _buff[1024]; /*��д����־���ݻ���*/
	int _size; /*��д����־���ݳ���*/
}LogQueueData;

/*���нڵ�����*/
typedef struct LogQueueNode{
	LogQueueData data;
	struct LogQueueNode * next;
}LogQueueNode;

/*��������*/
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
 * name:	��ʼ������
 * input:	Q  -- ��������
 * output:
 * return:	>0 -- �ɹ�
 * 			-1 -- ʧ��
 *-----------------------------------------------------------------------*/
int fun_log_queue_init(LogLinkQueue * Q);

/*-----------------------------------------------------------------------
 * name:	�ͷŶ�����Դ
 * input:	Q  -- ��������
 * output:
 * return:	>0 -- �ɹ�
 * 			-1 -- ʧ��
 *-----------------------------------------------------------------------*/
int fun_log_queue_destroy(LogLinkQueue * Q);

/*-----------------------------------------------------------------------
 * name:	��ն��нڵ�
 * input:	Q  -- ��������
 * output:
 * return:	>=0 -- ����ĸ���
 * 			 -1 -- ʧ��
 *-----------------------------------------------------------------------*/
int fun_log_queue_clear(LogLinkQueue * Q);

/*-----------------------------------------------------------------------
 * name:	���ض��г���
 * input:	Q  -- ��������
 * output:
 * return:	=0 -- ��
 * 			>0 -- �ڵ����
 *-----------------------------------------------------------------------*/
int fun_log_queue_length(LogLinkQueue * Q);

/*-----------------------------------------------------------------------
 * name:	���ض���ͷ���ڵ������
 * input:	Q  -- ��������
 * output:	data  -- QueueData���ݽṹ
 * return:	>0 -- �ɹ�
 * 			-1 -- ʧ��
 *-----------------------------------------------------------------------*/
int fun_log_queue_out(LogLinkQueue * Q, LogQueueData * data);

/*-----------------------------------------------------------------------
 * name:	������ݵ�����β�ڵ�
 * input:	Q  -- ��������
 * 			data  -- QueueData���ݽṹ
 * output:
 * return:	>0 -- �ɹ�
 * 			-1 -- ʧ��
 *-----------------------------------------------------------------------*/
int fun_log_queue_in(LogLinkQueue * Q, LogQueueData * data);

/*-----------------------------------------------------------------------
 * name:	ɾ��ָ���ڵ�
 * input:	Q  -- ��������
 * 			idx  -- ��������0��ʼ
 * output:
 * return:	>0 -- �ɹ�
 * 			-1 -- ʧ��
 *-----------------------------------------------------------------------*/
int fun_log_queue_del(LogLinkQueue * Q, int idx);

/*-----------------------------------------------------------------------
 * name:	����ָ���ڵ�
 * input:	Q  -- ��������
 * 			pdata  -- ���Ҷ���
 * output:
 * return:	>=0 -- ��������
 * 			 -1 -- ʧ��
 *-----------------------------------------------------------------------*/
int fun_log_queue_find(LogLinkQueue * Q, LogQueueData * pdata);

/*-----------------------------------------------------------------------
 * name:	����ָ���ڵ㣬����������棬�ڵ㲻�Ƴ�����
 * input:	Q  -- ��������
 * 			idx  -- �ڵ�����
 *
 * output:	pdata  -- ���Ҷ���
 * return:	>=0 -- ��������
 * 			 -1 -- ʧ��
 *-----------------------------------------------------------------------*/
int fun_log_queue_findbyidx(LogLinkQueue * Q, int idx, LogQueueData ** pdata);

/*-----------------------------------------------------------------------
 * name:	ȡָ���ڵ�
 * input:	Q  -- ��������
 * 			idx  -- �ڵ�����
 *
 * output:	pdata  -- ���Ҷ���
 * return:	>=0 -- ��������
 * 			 -1 -- ʧ��
 *-----------------------------------------------------------------------*/
int fun_log_queue_get(LogLinkQueue * Q, int idx, LogQueueData * pdata);


#ifdef	__cplusplus
}
#endif



#endif /*FFCS_MENU_QUEUE_H_*/
