/****************************************************************
 * ��   �� : SocketAPI.h	
 * ��   �� : ȫ������
 * ע   �� : FFSC  Queym   New Create     2006/01/10       
 *           Copyright by FFSC
****************************************************************/
#ifndef __API_SOCKET_H__
#define __API_SOCKET_H__
#include "FFCS_TypeDef.h"
#include "API-Tools.h"
/***************************************************************/
#define NMS_TRACE_INFO		0
#define NMS_CRITI_ALART		1
#define NMS_MAJOR_ALART		2
#define NMS_MINOR_ALART		3
#define NMS_WARNG_ALART		4
#define NMS_CLEAR_ALART		5
#define NMS_INFRM_MSG		10
#define NMS_FLUX_MSG		11

/*
��Ϣ����   :
0��������Ϣ����������׷�١�Ⱥ��׷��
1�����ظ澯��CRITICAL���������ܶ���������ҵ�����Ӱ�죩
2���ش�澯��MAJOR�������Բ���ҵ�����Ӱ�죩
3����С�澯��MINOR����ģ�鴦������У���ҵ�����̿������Ӱ�죩
4��һ��澯��WARNING������ҵ��һ�㲻���Ӱ��
5���澯�ָ���CLEAR�����������ڿɻָ��澯������·��������ϵȣ�
10��ģ���Ը�����ʾ����Ϣ
11: ���ܼ�ر�ʶ
*/

/*�澯��Ϣ�ṹ*/
typedef struct
{
	ff_uint			uiLen;				/*���ݰ���С*/
	ff_uint			uiMsgType;			/*��Ϣ����*/
	ff_uint			uiModName;			/*ģ������5λ���룩*/
	ff_uint			uiErrNo;			/*����ţ�8λ������*/
	ff_char			strContent[256];	/*��������*/
} NmsMsg;

/*������Ϣ*/
typedef struct 
{	
	ff_char			strIp[20];
	ff_uint			uiPort;
	ff_uint			uiSockFd;
	ff_uint			uiSockFlag;
	ff_sockaddr_in	szSockAddIn;
} NmsInfo;		

/***************************************************************/
/*����ԭ��*/
extern ff_int SendN(ff_int iFd, ff_char *strBuf, ff_int iLen, ff_int iTimeOut);
extern ff_int RecvN(ff_int iFd, ff_char *strBuf, ff_int iLen, ff_int iTimeOut);
extern ff_int RecvSmPack(ff_int iSockFd, ff_char *strBuf, ff_int iTimeOut);
extern ff_int Close(ff_int iSockFd);
ff_int Connect(ff_char *strIp, ff_int iPort);
ff_int Recv(ff_int iSockFd, ff_char *strBuf, ff_int iLen, ff_int iTimeOut);
ff_int Send(ff_int iSockFd, ff_char *strBuf, ff_int iLen, ff_int iTimeOut);
ff_int Accept(ff_int iSrvFd);
ff_int Listen(ff_int iPort);
ff_int EpollRecv(ff_int iSockFd, ff_char *strBuf, ff_int iLen, ff_int iTimeOut);

#endif

