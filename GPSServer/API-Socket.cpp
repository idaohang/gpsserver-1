/****************************************************************
 * �ļ�����: API-Socket.c                        
 * ��    ��: Socket����                         
 * ��    ע:
 ***************************************************************/

/****************************************************************
 * ͷ�ļ�������
****************************************************************/
#include "API-Socket.h"
#include "FFCS_TypeDef.h"

/****************************************************************
 * ����������
****************************************************************/
static NmsInfo	_s_szNmsInfo;

/****************************************************************
 * ��    ��: Listen
 * ��    ��: ����
 * ��    ��: 
			 ff_int iPort		�����˿�
 * ��    ��: 
			 ��
 * ��    ��: 
			 >0: ����˾����<0: ʧ��
 * ��    ע�������ʹ��
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

	//����socket������
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

	//���óɷ�����
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
	 //����server socket��Ϣ
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
 * ��    ��: Accept
 * ��    ��: ��������
 * ��    ��: 
			 ff_int iSrvFd		����˾��
 * ��    ��: 
			 ��
 * ��    ��: 
			 >0: �ͻ��˾����<0: ʧ��
 * ��    ע�������ʹ�ã�Ĭ�Ϸ��ؾ��Ϊ��������
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

	//���óɷ�����
	iRet = fcntl(iSockFd, F_SETFL, O_NONBLOCK);
	if (iRet == -1)
	{
		if (iSockFd > 0)
		{
			close(iSockFd);
		}
		return -2;
	}

	/*����socket������*/
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

	/*���ý��ջ���*/
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

	/*���÷��ͻ���*/
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
 * ��    ��: AcceptAddr
 * ��    ��: ��������
 * ��    ��: 
			 ff_int 		iSrvFd			����˾��
 			 ff_sockaddr_in *szSockAddrIn	��ַ��Ϣ
 * ��    ��: 
			 ��
 * ��    ��: 
			 >0: �ͻ��˾����<0: ʧ��
 * ��    ע�������ʹ�ã�Ĭ�Ϸ��ؾ��Ϊ��������
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

	//���óɷ�����
	iRet = fcntl(iSockFd, F_SETFL, O_NONBLOCK);
	if (iRet == -1)
	{
		if (iSockFd > 0)
		{
			close(iSockFd);
		}
		return -1;
	}

	/*����socket������*/
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

	/*���ý��ջ���*/
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

	/*���÷��ͻ���*/
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
 * ��    ��: Close
 * ��    ��: �ر�����
 * ��    ��: 
			 ff_int iFd			���
 * ��    ��: 
			 ��
 * ��    ��: 
			 1
 * ��    ע��
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
 * ��    ��: Connect
 * ��    ��: ���ӷ����
 * ��    ��: 
			 ff_char 	*strIp		�����IP
 			 ff_int 	iPort		����˶˿�
 * ��    ��: 
			 ��
 * ��    ��: 
			 >0: �ͻ��˾�� <0:ʧ��
 * ��    ע���ͻ���ʹ�ã�Ĭ�Ϸ��ؾ��Ϊ��������
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

	//����socket������
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
 
	//���óɷ�����
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

	//����1���ӿ��Ƿ������ӷ�����Ϣ�������ӳ�ʱ����
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

	//���ý��ջ���
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

	//���÷��ͻ���
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

	 //����server socket��Ϣ
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
 * ��    ��: SendN
 * ��    ��: ������Ϣ��
 * ��    ��: 
			 ff_int 	iFd			���;��
 			 ff_char 	*strBuf		������Ϣ��
 			 ff_int 	iLen		���ͳ���
 			 ff_int 	iTimeOut	��ʱʱ��
 * ��    ��: 
			 ��
 * ��    ��: 
			 1: �ɹ�, <0: ʧ��
 * ��    ע��ֻ��Է��������ʹ�á��ú���δʹ��select����ʱʱ��Ϊ0ʱ��������������
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
 * ��    ��: RecvN
 * ��    ��: ������Ϣ��
 * ��    ��: 
			 ff_int 	iFd			���վ��
 			 ff_char 	*strBuf		������Ϣ��
 			 ff_int 	iLen		���ճ���
 			 ff_int 	iTimeOut	��ʱʱ��
 * ��    ��: 
			 ��
 * ��    ��: 
			 1: �ɹ�, <0: ʧ��
 * ��    ע��ֻ��Է��������ʹ�á��ú���δʹ��select����ʱʱ��Ϊ0ʱ��������������
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

		if ((iNum==-1) && (errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)) /*�ض�����*/
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
 * ��    ��: Recv
 * ��    ��: ������Ϣ��
 * ��    ��: 
			 ff_int 	iFd			���վ��
 			 ff_char 	*strBuf		������Ϣ��
 			 ff_int 	iLen		���ճ���
 			 ff_int 	iTimeOut	��ʱʱ��
 * ��    ��: 
			 ��
 * ��    ��: 
			 1: �ɹ�, <0: ʧ��, 0 �����ݿɽ���
 * ��    ע��ֻ��Է��������ʹ�á��ú�������ʹ��selectԤ�ж��Ƿ������ݡ��޷�ʵ��������
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
 * ��    ��: Send
 * ��    ��: ������Ϣ��
 * ��    ��: 
			 ff_int 	iFd			���;��
 			 ff_char 	*strBuf		������Ϣ��
 			 ff_int 	iLen		���ͳ���
 			 ff_int 	iTimeOut	��ʱʱ��
 * ��    ��: 
			 ��
 * ��    ��: 
			 1: �ɹ�, <0: ʧ��, 0: �޷�����
 * ��    ע��ֻ��Է��������ʹ�á��ú�������ʹ��selecԤ�ж��Ƿ�ɷ��͡��޷�ʵ��������
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
 * ��    ��: InitAlarm
 * ��    ��: ��ʼ���澯
 * ��    ��: 
			 ff_char 	*strIp		�澯�����IP
 			 ff_uint 	uiPort		�澯����˶˿�
 * ��    ��: 
			 ��
 * ��    ��: 
			 1: �ɹ�, <0: ʧ��
 * ��    ע��
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
 * ��    ��: SendToAlarm
 * ��    ��: ���͸澯
 * ��    ��: 
			 ff_char 	*strBuf
 			 ff_int 	iLen
 			 ff_int 	iType
 			 ff_int 	iModeId
 			 ff_int 	iErrNo
 			 ff_int 	iOutIn
 			 ff_int 	iCmdId
 * ��    ��: 
			 ��
 * ��    ��: 
			 1: �ɹ�, <0: ʧ��
 * ��    ע��
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
 * ��    ��: RecvSmPack
 * ��    ��: ���ձ�׼����Ϣ��
 * ��    ��: 
			 ff_int 	iSockFd		���;��
 			 ff_char 	*strBuf		������Ϣ��
 			 ff_int 	iTimeOut	��ʱʱ��
 * ��    ��: 
			 strBuf
 * ��    ��: 
			 1: �ɹ�, <0: ʧ��, 0: �޿ɽ�����
 * ��    ע��
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
 * ��    ��: GetLocalIpAddr
 * ��    ��: ȡ����IP
 * ��    ��: 
			 ff_char *strIpAddr		IP��ַ���
 * ��    ��: 
			 *strIpAddr
 * ��    ��: 
			 ��
 * ��    ע��
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

		if ((iNum==-1) && (errno == EWOULDBLOCK || errno == EINTR)) /*�ض�����*/
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

