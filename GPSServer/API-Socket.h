/****************************************************************
 * 名   称 : SocketAPI.h	
 * 主   题 : 全局声明
 * 注   明 : FFSC  Queym   New Create     2006/01/10       
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
信息类型   :
0：网管信息，用于流程追踪、群发追踪
1：严重告警（CRITICAL）：（可能对网关整体业务造成影响）
2：重大告警（MAJOR）：（对部分业务造成影响）
3：较小告警（MINOR）：模块处理过程中，对业务流程可能造成影响）
4：一般告警（WARNING）：对业务一般不造成影响
5：告警恢复（CLEAR），（仅用于可恢复告警，如链路、网络故障等）
10：模块自给（提示）信息
11: 性能监控标识
*/

/*告警信息结构*/
typedef struct
{
	ff_uint			uiLen;				/*数据包大小*/
	ff_uint			uiMsgType;			/*信息类型*/
	ff_uint			uiModName;			/*模块名（5位编码）*/
	ff_uint			uiErrNo;			/*错误号（8位整数）*/
	ff_char			strContent[256];	/*错误内容*/
} NmsMsg;

/*网管信息*/
typedef struct 
{	
	ff_char			strIp[20];
	ff_uint			uiPort;
	ff_uint			uiSockFd;
	ff_uint			uiSockFlag;
	ff_sockaddr_in	szSockAddIn;
} NmsInfo;		

/***************************************************************/
/*函数原型*/
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

