/****************************************************************
 * 文件名称: API-Socket.c                        
 * 功    能: Socket函数                         
 * 备    注:
 ***************************************************************/

/****************************************************************
 * 头文件包含区
****************************************************************/
#include "API-Socket.h"
#include "FFCS_TypeDef.h"

/****************************************************************
 * 变量定义区
****************************************************************/
static NmsInfo	_s_szNmsInfo;

/****************************************************************
 * 名    称: Listen
 * 功    能: 监听
 * 输    入: 
			 ff_int iPort		监听端口
 * 输    出: 
			 无
 * 返    回: 
			 >0: 服务端句柄，<0: 失败
 * 备    注：服务端使用
****************************************************************/
/*
ff_int Listen(ff_int iPort)
{
	ff_int				iRet = 0;
	ff_int 				iSockFd = 0;
	ff_int    	 		iSockFdLen = 0;

	ff_int				iReuseAddr;

	ff_sockaddr_in		szSockAddrIn;

	iSockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (iSockFd < 0) 
	{
		#ifdef DEBUG
        	printf("netLister: iSockFd [%d] error ! [%d]\iNum", iPort, iSockFd);
		#endif
		
        return -1;
	}

	//设置socket可重用
	iReuseAddr = SO_REUSEADDR;
	iRet = setsockopt(iSockFd, SOL_SOCKET, SO_REUSEADDR, (void *)&iReuseAddr, sizeof(iReuseAddr));
	if (iRet < 0)
	{
		if (iSockFd > 0)
		{
			Close(iSockFd);
		}
  		return -2;
	}

	szSockAddrIn.sin_family = AF_INET;
	szSockAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
	//inet_aton("127.0.0.1", &(szSockAddrIn.sin_addr));
	szSockAddrIn.sin_port = htons(iPort);

	iSockFdLen = sizeof(szSockAddrIn);
	iRet = bind(iSockFd, (ff_sockaddr *)&szSockAddrIn, iSockFdLen);
	if (iRet < 0) 
	{
		#ifdef DEBUG
        	printf("netLister: bind [%d] error ! [%d]\iNum", iPort, iRet);
		#endif
		
        Close(iSockFd);

        return iRet;
	}

	//设置成非阻塞
	iRet = fcntl(iSockFd, F_SETFL, O_NONBLOCK);
	if (iRet == -1)
	{
		if (iSockFd > 0)
		{
			Close(iSockFd);
		}
		return -6;
	}

	iRet = listen(iSockFd, SOMAXCONN);
	if (iRet < 0) 
	{
		#ifdef DEBUG
        	printf("Listen: lisent [%d] error ! [%d]\iNum", iPort, iRet);
		#endif
		
        Close(iSockFd);

        return iRet;
	}

	return iSockFd;
}
*/
ff_int Listen(ff_int port)
{
	int sockfd;
	int nFlag = 1;
	struct sockaddr_in server_addr;  

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		return -2;
	}

	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&nFlag,sizeof(nFlag));   
	 //配置server socket信息
	memset(&(server_addr.sin_zero),0,8); //clear string
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons((u_short)port);
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY); 
		
	if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))<0)
	{
		Close(sockfd);
		return -3;
	}
	
	if(listen(sockfd, SOMAXCONN) <0){
		Close(sockfd);
		return -4;
	}
	return sockfd;
}
/****************************************************************
 * 名    称: Accept
 * 功    能: 接收连接
 * 输    入: 
			 ff_int iSrvFd		服务端句柄
 * 输    出: 
			 无
 * 返    回: 
			 >0: 客户端句柄，<0: 失败
 * 备    注：服务端使用，默认返回句柄为非阻塞。
****************************************************************/
ff_int Accept(ff_int iSrvFd)
{
	ff_int				iRet = 0;

    	ff_int				iSockFd = 0;
    	ff_int				iSockFdLen = 0;
	ff_sockaddr_in		szSockAddrIn;

	ff_int				iReuseAddr = 0;
	ff_int				iRecvVal = 0;
	ff_int				iRecvLen = 0;
	ff_int				iSendVal = 0;
	ff_int				iSendLen = 0;

    iSockFdLen = sizeof(ff_sockaddr_in);
    iSockFd = accept(iSrvFd, (ff_sockaddr *)&szSockAddrIn, (socklen_t*)&iSockFdLen);
	if(iSockFd == -1)
	{
		return -1;
	}

	//设置成非阻塞
	iRet = fcntl(iSockFd, F_SETFL, O_NONBLOCK);
	if (iRet == -1)
	{
		if (iSockFd > 0)
		{
			close(iSockFd);
		}
		return -2;
	}

	/*设置socket可重用*/
	iReuseAddr = SO_REUSEADDR;
	iRet = setsockopt(iSockFd, SOL_SOCKET, SO_REUSEADDR, (void *)&iReuseAddr, sizeof(iReuseAddr));
	if (iRet < 0)
	{
		if (iSockFd > 0)
		{
			close(iSockFd);
		}
  		return -3;
	}

	/*设置接收缓存*/
	iRecvVal = 128 * 1024;
	iRecvLen = 4;
	iRet = setsockopt(iSockFd, SOL_SOCKET, SO_RCVBUF, (void *)&iRecvVal, iRecvLen);
	if (iRet < 0)
	{
		if (iSockFd > 0)
		{
			close(iSockFd);
		}
  		return -4;
	}

	/*设置发送缓存*/
	iSendVal = 128 * 1024;
	iSendLen = 4;
	iRet = setsockopt(iSockFd, SOL_SOCKET, SO_SNDBUF, (void *)&iSendVal, iSendLen);
	if (iRet < 0)
	{
		if (iSockFd > 0)
		{
			close(iSockFd);
		}
  		return -5;
	}

    return iSockFd;
}

/****************************************************************
 * 名    称: AcceptAddr
 * 功    能: 接收连接
 * 输    入: 
			 ff_int 		iSrvFd			服务端句柄
 			 ff_sockaddr_in *szSockAddrIn	地址信息
 * 输    出: 
			 无
 * 返    回: 
			 >0: 客户端句柄，<0: 失败
 * 备    注：服务端使用，默认返回句柄为非阻塞。
****************************************************************/
ff_int AcceptAddr(ff_int iSrvFd, ff_sockaddr_in *szSockAddrIn)
{
	ff_int				iRet = 0;

    ff_int				iSockFd = 0;
    ff_int				iSockFdLen = 0;

	ff_int				iReuseAddr = 0;
	ff_int				iRecvVal = 0;
	ff_int				iRecvLen = 0;
	ff_int				iSendVal = 0;
	ff_int				iSendLen = 0;

    iSockFdLen = sizeof(ff_sockaddr_in);
    iSockFd= accept(iSrvFd, (ff_sockaddr *)szSockAddrIn, (socklen_t*)&iSockFdLen);

	//设置成非阻塞
	iRet = fcntl(iSockFd, F_SETFL, O_NONBLOCK);
	if (iRet == -1)
	{
		if (iSockFd > 0)
		{
			close(iSockFd);
		}
		return -1;
	}

	/*设置socket可重用*/
	iReuseAddr = SO_REUSEADDR;
	iRet = setsockopt(iSockFd, SOL_SOCKET, SO_REUSEADDR, (void *)&iReuseAddr, sizeof(iReuseAddr));
	if (iRet < 0)
	{
		if (iSockFd > 0)
		{
			close(iSockFd);
		}
  		return -2;
	}

	/*设置接收缓存*/
	iRecvVal = 128 * 1024;
	iRecvLen = 4;
	iRet = setsockopt(iSockFd, SOL_SOCKET, SO_RCVBUF, (void *)&iRecvVal, iRecvLen);
	if (iRet < 0)
	{
		if (iSockFd > 0)
		{
			close(iSockFd);
		}
  		return -3;
	}

	/*设置发送缓存*/
	iSendVal = 128 * 1024;
	iSendLen = 4;
	iRet = setsockopt(iSockFd, SOL_SOCKET, SO_SNDBUF, (void *)&iSendVal, iSendLen);
	if (iRet < 0)
	{
		if (iSockFd > 0)
		{
			close(iSockFd);
		}
  		return -4;
	}

    return iSockFd;
}

/****************************************************************
 * 名    称: Close
 * 功    能: 关闭连接
 * 输    入: 
			 ff_int iFd			句柄
 * 输    出: 
			 无
 * 返    回: 
			 1
 * 备    注：
****************************************************************/
ff_int Close(ff_int iSockFd)
{
	struct linger  szLig;
	
	szLig.l_linger = 0;
	setsockopt(iSockFd, SOL_SOCKET, SO_LINGER, (ff_char *)&szLig, sizeof(szLig));
	close(iSockFd);
	
	return 1;
}

/****************************************************************
 * 名    称: Connect
 * 功    能: 连接服务端
 * 输    入: 
			 ff_char 	*strIp		服务端IP
 			 ff_int 	iPort		服务端端口
 * 输    出: 
			 无
 * 返    回: 
			 >0: 客户端句柄 <0:失败
 * 备    注：客户端使用，默认返回句柄为非阻塞。
****************************************************************/
/*ff_int Connect(ff_char *strIp, ff_int iPort)
{
	ff_int					iSockFd = -1;
	ff_int					iReuseAddr = 0;
	ff_int					iRecvVal = 0;
	ff_int					iRecvLen = 0;
	ff_int					iSendVal = 0;
	ff_int					iSendLen = 0;
	ff_int					iRet = 0;

	ff_char					cErr = 0;
	ff_int					iLen = 0;

	ff_fdset				szFds;
	ff_timeval 				szSelectTv;
                        	
	ff_sockaddr_in			szSrvAddr;

	bzero(&szSrvAddr, sizeof(szSrvAddr));
	szSrvAddr.sin_family = AF_INET;
	szSrvAddr.sin_port = htons(iPort);
	szSrvAddr.sin_addr.s_addr = inet_addr((ff_char *)strIp);
	
	iSockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (iSockFd < 0)
	{
  		return -1;
	}

	//设置socket可重用
	iReuseAddr = SO_REUSEADDR;
	iRet = setsockopt(iSockFd, SOL_SOCKET, SO_REUSEADDR, (void *)&iReuseAddr, sizeof(iReuseAddr));
	if (iRet < 0)
	{
		if (iSockFd > 0)
		{
			close(iSockFd);
		}
  		return -2;
	}
 
	//设置成非阻塞
	iRet = fcntl(iSockFd, F_SETFL, O_NONBLOCK);
	if (iRet == -1)
	{
		if (iSockFd > 0)
		{
			close(iSockFd);
		}
		return -3;
	}

	connect(iSockFd, (struct sockaddr *)&szSrvAddr, sizeof(szSrvAddr));

	//等上1秒钟看是否有连接返馈信息，即连接超时处理
	FD_ZERO(&szFds);
	FD_SET(iSockFd, &szFds);
	szSelectTv.tv_sec = 3;
    szSelectTv.tv_usec = 0;
	iRet = select(iSockFd+1, NULL, &szFds, NULL, &szSelectTv);	
	if (iRet <= 0) 
	{
		if (iSockFd > 0)
		{
			close(iSockFd);
		}
     	return -4;
	}

	if (FD_ISSET(iSockFd, &szFds))
    {
        iLen = sizeof(cErr);

        if (getsockopt(iSockFd, SOL_SOCKET, SO_ERROR, &cErr, (socklen_t *)&iLen) < 0)
        {
            return -8;
        }
        else
        {
            if (cErr != 0)
            {
				return -9;
            }
        }
    }
    else
    {
        return -7;
    }

	//设置接收缓存
	iRecvVal = 64 * 1024;
	iRecvLen = 4;
	iRet = setsockopt(iSockFd, SOL_SOCKET, SO_RCVBUF, (void *)&iRecvVal, iRecvLen);
	if (iRet < 0)
	{
		if (iSockFd > 0)
		{
			close(iSockFd);
		}
  		return -5;
	}

	//设置发送缓存
	iSendVal = 64 * 1024;
	iSendLen = 4;
	iRet = setsockopt(iSockFd, SOL_SOCKET, SO_SNDBUF, (void *)&iSendVal, iSendLen);
	if (iRet < 0)
	{
		if (iSockFd > 0)
		{
			close(iSockFd);
		}
  		return -6;
	}

	return iSockFd;
}
*/
ff_int Connect(ff_char *ip, ff_int port)
{
	int sockfd;
	int nFlag = 1;
    struct sockaddr_in server_addr;	 
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return -2;
    }
	
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&nFlag,sizeof(nFlag));   

	 //配置server socket信息
    memset(&(server_addr.sin_zero),0,8); //clear string
	server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((u_short)port);
	server_addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))<0)
    {
		Close(sockfd);
		int en = errno;
		return (-1);
    }
	int   optval = 64 * 1024;;//64k, default is 8k
	setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(char*)&optval, sizeof(optval)); 

    return sockfd;

}

/****************************************************************
 * 名    称: SendN
 * 功    能: 发送消息包
 * 输    入: 
			 ff_int 	iFd			发送句柄
 			 ff_char 	*strBuf		发送消息包
 			 ff_int 	iLen		发送长度
 			 ff_int 	iTimeOut	超时时间
 * 输    出: 
			 无
 * 返    回: 
			 1: 成功, <0: 失败
 * 备    注：只针对非阻塞句柄使用。该函数未使用select。超时时间为0时，类似于阻塞。
****************************************************************/
ff_int SendN(ff_int iSockFd, ff_char *strBuf, ff_int iLen, ff_int iTimeOut)
{
	ff_time		tStart;
	ff_int 		iLeft;
	ff_int		iNum;
	ff_char 	*p;

	p = strBuf;
	iLeft = iLen;
	tStart = time(0);

	while (iLeft > 0)
	{
		if ((iTimeOut>0) && ((time(0)-tStart)>iTimeOut))
		{
			return -3;
		}

	    iNum = send(iSockFd, p, iLeft, 0);

	    if (iNum > 0)
	    {
			iLeft -= iNum;
			p += iNum;
			continue;
	    }

		if ((iNum==-1) && (errno==EWOULDBLOCK || errno==EINTR || errno==EAGAIN))
		{
			MSleep(5000);
			continue;
		}

	    if (iNum == 0)
		{
			return -1; /*connection closed*/
		}
		else
		{
			return -2;
		}
	}

	return 1;
}

/****************************************************************
 * 名    称: RecvN
 * 功    能: 接收消息包
 * 输    入: 
			 ff_int 	iFd			接收句柄
 			 ff_char 	*strBuf		接收消息包
 			 ff_int 	iLen		接收长度
 			 ff_int 	iTimeOut	超时时间
 * 输    出: 
			 无
 * 返    回: 
			 1: 成功, <0: 失败
 * 备    注：只针对非阻塞句柄使用。该函数未使用select。超时时间为0时，类似于阻塞。
****************************************************************/
ff_int RecvN(ff_int iSockFd, ff_char *strBuf, ff_int iLen, ff_int iTimeOut)
{
	ff_time 		tStart;
	ff_int 			iLeft;
	ff_int			iNum;
	ff_char 		*p;

	p = strBuf;
	iLeft = iLen;
	tStart = time(0);

	while (iLeft > 0)
	{
		if ((iTimeOut>0) && ((time(0)-tStart)>iTimeOut))
		{
			return -3;
		}

		iNum = recv(iSockFd, p, iLeft, MSG_NOSIGNAL);
		printf("recv, sockfd = %d, ret = %d\n", iSockFd, iNum);
		if (iNum > 0)
		{
			iLeft -= iNum;
			p += iNum;

			continue;
		}

		if ((iNum==-1) && (errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)) /*重读数据*/
		{
			MSleep(5000);
			continue;
		}

		if (iNum == 0)
		{
			printf("socket recv error, errno = %d, strerror = %s\n", errno, strerror(errno));
			return -1; /*connection closed*/
		}
		else
		{
			printf("socket recv error, errno = %d, strerror = %s\n", errno, strerror(errno));
			return -2;
		}

	}

	return 1;
}

/****************************************************************
 * 名    称: Recv
 * 功    能: 接收消息包
 * 输    入: 
			 ff_int 	iFd			接收句柄
 			 ff_char 	*strBuf		接收消息包
 			 ff_int 	iLen		接收长度
 			 ff_int 	iTimeOut	超时时间
 * 输    出: 
			 无
 * 返    回: 
			 1: 成功, <0: 失败, 0 无数据可接收
 * 备    注：只针对非阻塞句柄使用。该函数内有使用select预判断是否有数据。无法实现阻塞。
****************************************************************/
ff_int Recv(ff_int iSockFd, ff_char *strBuf, ff_int iLen, ff_int iTimeOut)
{
	ff_int			iRet = 0;
	ff_timeval		tvSelect;
	ff_fdset  		fsSets;

	tvSelect.tv_sec = iTimeOut;
	tvSelect.tv_usec = 0;

	FD_ZERO(&fsSets);
	FD_SET(iSockFd, &fsSets);
	iRet = select(iSockFd+1, &fsSets, NULL, NULL, &tvSelect);
	switch (iRet)
	{
		case -1: /**/
	        return -1;

	        break;
		case 0:  /**/
	        return 0;

	        break;
		default : /**/
	        iRet = RecvN(iSockFd, strBuf, iLen, iTimeOut);
	        if (iRet < 0)
	        {
	        	printf("Recv error, iRet = %d, errno = %d, strerror()=%s\n", iRet, errno, strerror(errno));
                return -2;
	        }

			break;
	}

	return 1;
}

/****************************************************************
 * 名    称: Send
 * 功    能: 发送消息包
 * 输    入: 
			 ff_int 	iFd			发送句柄
 			 ff_char 	*strBuf		发送消息包
 			 ff_int 	iLen		发送长度
 			 ff_int 	iTimeOut	超时时间
 * 输    出: 
			 无
 * 返    回: 
			 1: 成功, <0: 失败, 0: 无法发送
 * 备    注：只针对非阻塞句柄使用。该函数内有使用selec预判断是否可发送。无法实现阻塞。
****************************************************************/
ff_int Send(ff_int iSockFd, ff_char *strBuf, ff_int iLen, ff_int iTimeOut)
{
	ff_int			iRet = 0;
	ff_timeval		tvSelect;
	ff_fdset  		fsSets;

	tvSelect.tv_sec = iTimeOut;
	tvSelect.tv_usec = 0;

	FD_ZERO(&fsSets);
	FD_SET(iSockFd, &fsSets);
	iRet = select(iSockFd+1, NULL, &fsSets, NULL, &tvSelect);
	switch (iRet)
	{
		case -1: /**/
	        return -1;

	        break;
		case 0:  /**/
	        return 0;

	        break;
		default : /**/
	        iRet = SendN(iSockFd, strBuf, iLen, iTimeOut);
	        if (iRet < 0)
	        {
                return -1;
	        }

			break;
	}

	return 1;
}

/****************************************************************
 * 名    称: InitAlarm
 * 功    能: 初始化告警
 * 输    入: 
			 ff_char 	*strIp		告警服务端IP
 			 ff_uint 	uiPort		告警服务端端口
 * 输    出: 
			 无
 * 返    回: 
			 1: 成功, <0: 失败
 * 备    注：
****************************************************************/
ff_int InitAlarm(ff_char *strIp, ff_uint uiPort)
{
	ff_int			iSockFd = 0;
	ff_int			iSockOptVal;
	ff_sockaddr_in	szUdpSrv;

	if (strIp == NULL)
	{
		return -1;
	}

	memset(&_s_szNmsInfo, 0, sizeof(_s_szNmsInfo));
	strcpy(_s_szNmsInfo.strIp, strIp);
	_s_szNmsInfo.uiPort = uiPort;

	memset(&szUdpSrv, 0, sizeof(szUdpSrv));

	szUdpSrv.sin_family = AF_INET;
	szUdpSrv.sin_addr.s_addr = inet_addr(_s_szNmsInfo.strIp);
	szUdpSrv.sin_port = htons(_s_szNmsInfo.uiPort);	

	iSockFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (iSockFd < 0)
	{
		return -1;
	}

	if (setsockopt(iSockFd, SOL_SOCKET, SO_BROADCAST, (void *)&iSockOptVal, sizeof(iSockOptVal)) < 0)
	{
		return -1;
   	}

	memcpy(&(_s_szNmsInfo.szSockAddIn), &szUdpSrv, sizeof(ff_sockaddr_in));
	_s_szNmsInfo.uiSockFd = iSockFd;
	_s_szNmsInfo.uiSockFlag = 1;

	return 1;
}

/****************************************************************
 * 名    称: SendToAlarm
 * 功    能: 发送告警
 * 输    入: 
			 ff_char 	*strBuf
 			 ff_int 	iLen
 			 ff_int 	iType
 			 ff_int 	iModeId
 			 ff_int 	iErrNo
 			 ff_int 	iOutIn
 			 ff_int 	iCmdId
 * 输    出: 
			 无
 * 返    回: 
			 1: 成功, <0: 失败
 * 备    注：
****************************************************************/
ff_int SendToAlarm(ff_char *strBuf, ff_int iLen, ff_int iType, ff_int iModeId, ff_int iErrNo, ff_int iOutIn, ff_int iCmdId)
{
	ff_int		iTmp = 0;
	ff_int		iRet = 0;
	ff_int		iOutIn2 = 0;
	ff_int		iCmdId2 = 0;
	ff_char 	strBuf2[1024];

	NmsMsg		szNmsMsg;

	memset(&szNmsMsg, 0, sizeof(NmsMsg));
	if (_s_szNmsInfo.uiSockFlag == 0)
	{
		return -1;	
	}

	szNmsMsg.uiMsgType = htonl(iType);
	szNmsMsg.uiModName = htonl(iModeId);
	szNmsMsg.uiErrNo = htonl(iErrNo);

	if (iType == NMS_TRACE_INFO)
	{
		iTmp = 16 + iLen + 8;
		szNmsMsg.uiLen = htonl(iTmp);
		iOutIn2 = htonl(iOutIn);
		iCmdId2 = htonl(iCmdId);
		memcpy(szNmsMsg.strContent, &iOutIn2,4);
		memcpy(szNmsMsg.strContent+4, &iCmdId2,4);
		memcpy(szNmsMsg.strContent+8, strBuf, iLen);
	}
	else
	{
		iTmp = 16 + iLen;
		szNmsMsg.uiLen = htonl(iTmp);
		memcpy(szNmsMsg.strContent, strBuf, iLen);
	}

	memset(strBuf2, 0, sizeof(strBuf2));
	memcpy(strBuf2, &szNmsMsg, iTmp);
	iRet = sendto(_s_szNmsInfo.uiSockFd, strBuf2, iTmp, 0, (ff_sockaddr *)&_s_szNmsInfo.szSockAddIn, sizeof(ff_sockaddr));
	if (iRet != iTmp)
	{
		return -1;
	}

	return 1;
}

/****************************************************************
 * 名    称: RecvSmPack
 * 功    能: 接收标准短消息包
 * 输    入: 
			 ff_int 	iSockFd		发送句柄
 			 ff_char 	*strBuf		发送消息包
 			 ff_int 	iTimeOut	超时时间
 * 输    出: 
			 strBuf
 * 返    回: 
			 1: 成功, <0: 失败, 0: 无可接数据
 * 备    注：
****************************************************************/
ff_int RecvSmPack(ff_int iSockFd, ff_char *strBuf, ff_int iTimeOut)
{
	ff_int			iRet = 0;
	ff_int			iLen = 0;
	ff_timeval		tvSelect;
	ff_fdset  		fsSets;

	tvSelect.tv_sec = iTimeOut;
	tvSelect.tv_usec = 0;

	FD_ZERO(&fsSets);
	FD_SET(iSockFd, &fsSets);
	iRet = select(iSockFd+1, &fsSets, NULL, NULL, &tvSelect);
	switch (iRet)
	{
		case -1: /**/
			return -1;

			break;
		case 0:  /**/
			return 0;

			break;
		default : /**/
			iRet = RecvN(iSockFd, strBuf, 4, 1);
			if (iRet < 0)
			{
				return -2;
			}

			memcpy(&iLen, strBuf, 4);
			iLen = ntohl(iLen);
			if (iLen<=0 || iLen>2048)
			{
				return -3;
			}

			iRet = RecvN(iSockFd, strBuf+4, iLen-4, 1);
			if (iRet < 0)
			{
				return -4;
			}

			break;
	}

	return iLen;
}

/****************************************************************
 * 名    称: GetLocalIpAddr
 * 功    能: 取本机IP
 * 输    入: 
			 ff_char *strIpAddr		IP地址输出
 * 输    出: 
			 *strIpAddr
 * 返    回: 
			 无
 * 备    注：
****************************************************************/
ff_char *GetLocalIpAddr(ff_char *strIpAddr)
{
    ff_char				strTemp[32];
    ff_int				iSockFd;

    struct sockaddr_in	sin;
    struct ifreq		ifr;

    iSockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (iSockFd == -1)
    {
		return NULL;
    }

    strncpy(ifr.ifr_name,"eth0", sizeof(ifr.ifr_name));
    if (ioctl(iSockFd, SIOCGIFADDR, &ifr) < 0)
    {
		return NULL;
    }

    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));

    close(iSockFd);

    memset(strTemp, 0, sizeof(strTemp));
    memcpy(strTemp, inet_ntoa(sin.sin_addr), strlen(inet_ntoa(sin.sin_addr)));

	strcpy(strIpAddr, strTemp);

    return strIpAddr;
}


ff_int EpollRecv(ff_int iSockFd, ff_char *strBuf, ff_int iLen, ff_int iTimeOut)
{
	ff_time 		tStart;
	ff_int 			iLeft;
	ff_int			iNum;
	ff_char 		*p;

	p = strBuf;
	iLeft = iLen;
	tStart = time(0);

	while (iLeft > 0)
	{
		if ((iTimeOut>0) && ((time(0)-tStart)>iTimeOut))
		{
			return -3;
		}

		iNum = recv(iSockFd, p, iLeft, MSG_NOSIGNAL);
		printf("recv, sockfd = %d, ret = %d\n", iSockFd, iNum);
		if (iNum > 0)
		{
			iLeft -= iNum;
			p += iNum;

			continue;
		}

		if ((iNum==-1) && (errno == EAGAIN))
		{
			return 0;
		}

		if ((iNum==-1) && (errno == EWOULDBLOCK || errno == EINTR)) /*重读数据*/
		{
			MSleep(5000);
			continue;
		}

		if (iNum == 0)
		{
			printf("socket recv error, errno = %d, strerror = %s\n", errno, strerror(errno));
			return -1; /*connection closed*/
		}
		else
		{
			printf("socket recv error, errno = %d, strerror = %s\n", errno, strerror(errno));
			return -2;
		}

	}

	return 1;
}

