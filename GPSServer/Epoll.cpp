#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "GlobalStruct.h"
#include "API-Tools.h"
#include "API-Socket.h"
#include "LogAdmin.h"
#include "ProcGPSMsg.h"
#include "SendToGPS.h"
#include "IniFile.h"
#include <sys/stat.h>
#include "Database.h"
#include "Session.h"

#define CONFIG_FILE_NAME "Config.ini"

CGPSJobQueue g_rGPSJobQueue[MAX_PROC_GPS_THREAD];
CPacketQueue g_rRecvGPSPkgQueue[MAX_PROC_GPS_THREAD];
CPacketQueue g_rSendGPSPkgQueue[MAX_PROC_GPS_THREAD];
extern CTerminalList	g_rTermList;
extern CDatabase g_rDatabaseList[MAX_PROC_GPS_THREAD];
int epfd;
TConfig	g_rConfig;
CSessionMap g_rSessionMap[MAX_PROC_GPS_THREAD];

extern void* thread_SOAP_server_main(void* nouseparam);

void setnonblocking(int sock)
{

    int opts;

    opts=fcntl(sock,F_GETFL);

    if(opts<0)

    {

    perror("fcntl(sock,GETFL)");

    _exit(1);

    }

    opts = opts|O_NONBLOCK;

    if(fcntl(sock,F_SETFL,opts)<0)

    {

    perror("fcntl(sock,SETFL,opts)");

    _exit(1);

    }

}

int CloseSockFd(int nSockFd)
{
	struct epoll_event ev;
	ev.data.fd = nSockFd;
	//��������ע��Ķ������¼�
	ev.events = EPOLLIN|EPOLLET;
	//ע��ev
	if (epoll_ctl(epfd, EPOLL_CTL_DEL, nSockFd, &ev) < 0)
	{
		printf("remove fd:%d from epoll set, error\n", nSockFd);
	}
	Close(nSockFd);
	g_rTermList.Erase(nSockFd);
	return 0;
}

int SendGPSMsg(TPacket &rPacket)
{
	return SendN(rPacket.nSockFd, rPacket.pBuffer, rPacket.unLen, SEND_TIMEOUT);
}

void *SendGPSMsgThread(void *pParam)
{
	int nThreadNo = *(int *)pParam;
	int	nSockFd;
	TPacket rPacket;
	while (1)
	{
		if (g_rSendGPSPkgQueue[nThreadNo].ReadDataFromQueue(&rPacket) < 0)
		{
			MSleep(50);
			continue;
		}
		if (SendGPSMsg(rPacket) < 0)
		{
			WriteLogByLevel(2, "[>%d<]SendGPSMsgThread, SendGPSMsg error!\n", rPacket.nSockFd);
			CloseSockFd(rPacket.nSockFd);
		}
		WriteLogByLevel(1, "[>%d<]SendGPSMsg.\n", rPacket.nSockFd);
		if (rPacket.pBuffer != NULL)
		{
			free(rPacket.pBuffer);
			rPacket.pBuffer = NULL;
		}
	}
	return NULL;
}

int ProcGPSMsg(TPacket &rPacket, int nThreadNo)
{
	if (rPacket.pBuffer == NULL)
	{
		WriteLogByLevel(1, "In ProcGPSMsg, rPacket.pBuffer == NULL\n");
		return -1;
	}
	char cType = rPacket.pBuffer[0];
	switch(cType)
	{
	case 0x4C:	//����
		ProcGPSLogin(rPacket, nThreadNo);
		/*{
		//DEBUG
		//SendToGPSCamStat(rPacket.nSockFd); ûЧ��
		TSetAreaReq req;
		TGPSPoint rPoint1;
		rPoint1.dLat = 20.1142;
		rPoint1.dLong = 109.2641;
		TGPSPoint rPoint2;
		rPoint2.dLat = 20.1142;
		rPoint2.dLong = 111.2641;
		TGPSPoint rPoint3;
		rPoint3.dLat = 24.1142;
		rPoint3.dLong = 111.2641;
		TGPSPoint rPoint4;
		rPoint4.dLat = 24.1142;
		rPoint4.dLong = 109.2641;
		req.pointList.push_back(rPoint1);
		req.pointList.push_back(rPoint2);
		req.pointList.push_back(rPoint3);
		req.pointList.push_back(rPoint4);
		SendToGPSSetArea(req, rPacket.nSockFd);
		}*/
		break;
	case 0x80:
		ProcGPSTemperatureInfo(rPacket, nThreadNo);
		break;
	case 0x76:
		ProcGPS485Data(rPacket, nThreadNo);
		break;
	case 0x43:	//ʵʱ��Ϣ
		ProcGPSRealInfo(rPacket, nThreadNo);
		break;
	case 0x41:	//�쳣��Ϣ
		ProcGPSAlert(rPacket, nThreadNo);
		break;
	case 0x6C:	//������Ϣ
		ProcGPSLogout(rPacket, nThreadNo);
		CloseSockFd(rPacket.nSockFd);
		break;
	case 0x53:	//ִ������ɹ�
		ProcGPSCmdSuccessResp(rPacket, nThreadNo);
		break;
	case 0x45:	//ִ������ɹ�
		ProcGPSCmdFailResp(rPacket, nThreadNo);
		break;	
	case 0x65:	//�ն�����Ӧ��
		ProcGPSConfigResp(rPacket, nThreadNo);
		break;	
	case 0x56:	//�汾��Ϣ
		ProcGPSQueryVerResp(rPacket, nThreadNo);
		break;	
	case 0x4A:	//ͼƬ����
		ProcGPSImage(rPacket, nThreadNo);
		break;
	case 0x44:	//��ͨ���ݴ���
		ProcGPSCommonData(rPacket, nThreadNo);
		break;
	case 0x7B:	//¼��״̬
		ProcGPSVideoStatResp(rPacket, nThreadNo);
		break;
	case 0x72:
		ProcGPSHeart(rPacket, nThreadNo);
		break;
	case 0x55:
		ProcGPSICCardData(rPacket, nThreadNo);	//IC������
	default:
		break;
	}
	free(rPacket.pBuffer);
	rPacket.pBuffer = NULL;
	return 0;
}

void * ProcGPSMsgThread(void *pParam)
{
	int nThreadNo = *(int *)pParam;
	TPacket rPacket;
	while (1)
	{
		if (g_rRecvGPSPkgQueue[nThreadNo].ReadDataFromQueue(&rPacket) < 0)
		{
			MSleep(50);
			continue;
		}
		ProcGPSMsg(rPacket, nThreadNo);
		free(rPacket.pBuffer);
		memset(&rPacket, 0x00, sizeof(rPacket));
	}
	return NULL;
}


int RecvGPSMsg(int nSockFd, int nThreadNo)
{
	if (nSockFd < 1)
	{
		WriteLogByLevel(1, "In RecvGPSMsg, socket fd is out of range.\n");
		return -1;
	}
	
	int		nRet;
	char	szHead[4] = {0};
	nRet = EpollRecv(nSockFd, szHead, 3, RECV_TIMEOUT);
	if (nRet == 0)	//EAGAIN, ˵��ȫ��������
	{
		return 1;
	}
	if (nRet != 1)	//���ͣ�����
	{
		CloseSockFd(nSockFd);
		WriteLogByLevel(1, "Recv packet head from GPS error! nSockFd = %d\n",nSockFd);
		return -1;
	}
	if (szHead[0] != 0x02)		//֡ͷ��0x02
	{
		CloseSockFd(nSockFd);
		WriteLogByLevel(1, "GPS packet head is error! nSockFd = %d\n",nSockFd);
		return -1;
	}
	unsigned int unLen = 0;
	memcpy(&unLen, szHead + 1, 2);
	char *pBody = (char *)malloc(unLen + 1);
	nRet = EpollRecv(nSockFd, pBody, unLen + 1, RECV_TIMEOUT);
	if (nRet == 0)	//EAGAIN, ˵��ȫ��������
	{
		return 1;
	}
	if (nRet != 1)	//���ͣ�����
	{
		CloseSockFd(nSockFd);
		WriteLogByLevel(1, "Recv body from GPS error! nSockFd = %d\n",nSockFd);
		return -1;
	}
	if (*(pBody + unLen) != 0x03)	//�жϽ�����
	{
		CloseSockFd(nSockFd);
		WriteLogByLevel(1, "Recv body from GPS, but end char error! nSockFd = %d\n",nSockFd);
		return -1;
	}
	TPacket rPacket;
	rPacket.pBuffer = pBody;
	rPacket.nSockFd = nSockFd;
	rPacket.unLen = unLen;
	g_rRecvGPSPkgQueue[nThreadNo].WriteDataToQueue(&rPacket);
	return 0;
}

//����MAX_RECV_GPS_THREAD���߳̽�����Ϣ
void *RecvGPSMsgThread(void *pParam)
{
	int nThreadNo = *(int *)pParam;
	int	nSockFd;
	TGPSJob rGPSJob;
	while (1)
	{
		if (g_rGPSJobQueue[nThreadNo].ReadDataFromQueue(&rGPSJob) < 0)
		{
			MSleep(50);
			continue;
		}
		int nRet = 0;
		while (1)		//EPOLL ETģʽ��
		{
			nRet = RecvGPSMsg(rGPSJob.nSockFd, nThreadNo);
			if (nRet == 1)	//������
			{
				break;
			}
			else if (nRet < 0)	//������
			{
				break;
			}
		}
	}
	return NULL;
}



/*****************************************
EPOLLOUT����ʾ��Ӧ���ļ�����������д��
EPOLLPRI����ʾ��Ӧ���ļ��������н��������ݿɶ����Ҳ�̫������ʲô��˼������������client�ر� socket�����������¼�����
EPOLLERR����ʾ��Ӧ���ļ���������������
EPOLLHUP����ʾ��Ӧ���ļ����������Ҷϣ�
EPOLLET����ʾ��Ӧ���ļ����������¼�������

Ҫ���еĲ�������ע���¼������ܵ�ȡֵEPOLL_CTL_ADD ע�ᡢEPOLL_CTL_MOD ��
�ġ�EPOLL_CTL_DEL ɾ��
******************************************/
int EpollProc(int nEpollListenPort)
{
	int nfds, i, connfd, sockfd, listenfd, nRet = 0;
	
	//����epoll_event�ṹ��ı���,ev����ע���¼�,�������ڻش�Ҫ������¼�
	struct epoll_event ev, events[MAX_CLIENT_NUM];
	
	//�������ڴ���accept��epollר�õ��ļ�������
	epfd = epoll_create(MAX_CLIENT_NUM + 1);

	struct sockaddr_in clientaddr;
	struct sockaddr_in serveraddr;
	socklen_t clilen = sizeof clientaddr;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	//��socket����Ϊ��������ʽ
	setnonblocking(listenfd);
	
	//������Ҫ������¼���ص��ļ�������
	ev.data.fd = listenfd;
	//����Ҫ������¼�����
	ev.events = EPOLLIN|EPOLLET;
	//ע��epoll�¼�
	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
	
	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	//inet_aton(g_sAgentAddr, &(serveraddr.sin_addr));
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	serveraddr.sin_port=htons(nEpollListenPort);
	while (bind(listenfd,(struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
	{
		WriteLogByLevel(3, "Server bind pord:%d error!\n", nEpollListenPort);
		printf("Server bind pord:%d error!\n", nEpollListenPort);
		sleep(5);
	}
	listen(listenfd, MAX_CLIENT_NUM);

	WriteLogByLevel(2, "Server listen at port:%d\n", nEpollListenPort);

	TGPSJob rJob;
	char	sTmpBuf[128];
	while (1)
	{
		//�ȴ�epoll�¼��ķ���
		nfds=epoll_wait(epfd, events, MAX_CLIENT_NUM, -1);
		for (i = 0; i<nfds; ++i)
		{
			//���ܿͻ�������
			if(events[i].data.fd == listenfd)
			{
				connfd = accept(listenfd,(struct sockaddr *)&clientaddr, &clilen);
				if (connfd < 0)
				{
					printf("accept a client, but connfd < 0\n");
					continue;
				}
				setnonblocking(connfd);
				char *str = inet_ntoa(clientaddr.sin_addr);
				printf("connect from %s\n", inet_ntoa(clientaddr.sin_addr));
				//�������ڶ��������ļ�������
				ev.data.fd = connfd;
				//��������ע��Ķ������¼�
				ev.events = EPOLLIN|EPOLLET;
				//ע��ev
				if (epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev) < 0)
				{
					printf("epoll set insertion error, fd = %d\n", connfd);
				}
			}
			//������Ϣ
			else if (events[i].events & EPOLLIN)
			{
				if ( (sockfd = events[i].data.fd) < 0) 
					continue;
				printf("reading!, sockfd = %d\n", sockfd); 
				rJob.nSockFd = sockfd;
				g_rGPSJobQueue[sockfd % MAX_PROC_GPS_THREAD].WriteDataToQueue(&rJob);
			}
			else if (events[i].events & EPOLLHUP)
			{
				printf("===EPOLLHUP===, sockfd = %d\n", sockfd);
			}
			else if (events[i].events & EPOLLERR)
			{
				printf("===EPOLLERR===, sockfd = %d\n", sockfd);
			}
			else if (events[i].events & EPOLLPRI)
			{
				printf("===EPOLLPRI===, sockfd = %d\n", sockfd);
			}
			else if (events[i].events & EPOLLOUT)
			{
				printf("===EPOLLOUT===, sockfd = %d\n", sockfd);
			}
		}
	}
}

void SetAlertMask(string &sAlertMask)
{
	WriteLogByLevel(1, "SetAlertMask: %s\n", sAlertMask.c_str());
	g_rConfig.rAlertIdMaskSet.clear();
	if (sAlertMask.length() == 0)
		return ;
	
	int nPos1 = 0, nPos2 = 0;
	while (1)
	{
		nPos2 = sAlertMask.find(',', nPos1);
		if (nPos2 == string::npos)
		{
			g_rConfig.rAlertIdMaskSet.insert(atol(sAlertMask.substr(nPos1).c_str()));
			return ;
		}
		g_rConfig.rAlertIdMaskSet.insert(atol(sAlertMask.substr(nPos1, nPos2).c_str()));
		nPos1 = nPos2 + 1;
	}
}

int ReadConfig(char *szConfigFile)
{
	CIniFile rConfigFile;
	if (rConfigFile.OpenIniFile(szConfigFile) == FALSE)
	{
		printf("Can't open config file\n");
		exit(0);
	}
	string sAlertMaskSet = rConfigFile.ReadString("ALERT_MASK", "ALERT_ID_SET", "");
	SetAlertMask(sAlertMaskSet);
	
	g_rConfig.sGPSServIP = rConfigFile.ReadString("SERVER", "IP", "127.0.0.1");
	g_rConfig.nGPSServListenPort = rConfigFile.ReadInt("SERVER", "PORT", 12345);
	g_rConfig.unKeepAlive = rConfigFile.ReadInt("SERVER", "KEEP_ALIVE", 60);
	printf("g_rConfig.nGPSServListenPort = %d\n", g_rConfig.nGPSServListenPort);
	g_rConfig.sLogPath = rConfigFile.ReadString("LOG", "PATH", "./defaultlog");
	g_rConfig.sPhotoPath = rConfigFile.ReadString("PHOTO", "PATH", "./photo");
	g_rConfig.sDbHost = rConfigFile.ReadString("DB", "HOST", "127.0.0.1");
	g_rConfig.sDbName = rConfigFile.ReadString("DB", "NAME", "wlps");
	g_rConfig.sDbUser = rConfigFile.ReadString("DB", "USERNAME", "wlpsuser");
	g_rConfig.sDbPassword = rConfigFile.ReadString("DB", "PASSWORD", "wlpsuser");
	g_rConfig.unDbPort = rConfigFile.ReadInt("DB", "PORT", 3306);
	for (int i = 0; i < MAX_PROC_GPS_THREAD; ++i)
	{
		g_rDatabaseList[i].SetDbInfo(g_rConfig.sDbHost.c_str(), g_rConfig.sDbUser.c_str(),\
				g_rConfig.sDbPassword.c_str(), g_rConfig.sDbName.c_str(), g_rConfig.unDbPort);
	}
	g_rConfig.unWebserviceTimeout = rConfigFile.ReadInt("WEBSERVICE", "TIMEOUT", 30);
	g_rConfig.sServAddr = rConfigFile.ReadString("WEBSERVICE", "ADDR", "127.0.0.1");
	g_rConfig.unServPort = rConfigFile.ReadInt("WEBSERVICE", "PORT", 9090);
	g_rConfig.unMaxSession = rConfigFile.ReadInt("WEBSERVICE", "MAX_SESSION", 100);
	g_rConfig.unSessionTimeout = rConfigFile.ReadInt("SESSION", "TIMEOUT", 40);
	g_rConfig.sSMSCenterWebAddr = rConfigFile.ReadString("SMS_CENTER", "ADDR", "http://120.36.0.57/services/imRoot");
	g_rConfig.s3desKey = rConfigFile.ReadString("SMS_CENTER", "3DES_KEY", "464336343333324634313245414131424138453938303131");
	WriteLogByLevel(1, "Read config success.\n");
	return 0;
}

void * Maintain(void *pParam)
{
	struct stat st;	
	time_t tLastModifyTime = 0;
	while (1)
	{
		stat(CONFIG_FILE_NAME, &st);	
		if (st.st_mtime > tLastModifyTime)
		{
			tLastModifyTime = st.st_mtime;
			ReadConfig(CONFIG_FILE_NAME);
		}
		sleep(10);
	}	
	return NULL;
}

void * ProcDeadTerm(void *pParam)
{
	std::list<int> rDeadTermSockFdList;
	std::list<int>::iterator it;
	while (1)
	{
		rDeadTermSockFdList.clear();
		if (g_rTermList.GetDeadTerms(rDeadTermSockFdList) <= 0)
		{
			sleep (2);
			continue;
		}
		
		char szBuf[1024] = {0};
		string sSQL;
		int nRet;	
		CDatabase db;
		db.Connect(g_rConfig.sDbHost.c_str(), g_rConfig.sDbUser.c_str(),\
				g_rConfig.sDbPassword.c_str(), g_rConfig.sDbName.c_str(), g_rConfig.unDbPort);
		
		it = rDeadTermSockFdList.begin();
		for (; it != rDeadTermSockFdList.end(); ++it)
		{
			WriteLogByLevel(1, "[>%d<]Terminal is dead, close it.\n", *it);
			string sSimPhone = g_rTermList.GetSimPhoneBySockFd(*it);
			CloseSockFd(*it);
			sprintf(szBuf, "update T_LOGIN_RECORD set logout_time = now() where sim_phone = '%s' and logout_time is null;", sSimPhone.c_str());
			sSQL = szBuf;
			db.ExecSQL(sSQL);
		}
		db.Close();
	}
	return NULL;
}

void *ProcSessionTimeout(void *pParam)
{
	while (1)
	{
		for (int i = 0; i < MAX_PROC_GPS_THREAD; ++i)
		{
			g_rSessionMap[i].DeleteTimeout();
		}
		sleep(2);
	}
	return NULL;
}

int main()
{
	pthread_attr_t threadAttr;
	pthread_attr_init(&threadAttr);
	pthread_attr_setscope(&threadAttr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
	pthread_t pid;
	pthread_create(&pid, &threadAttr, Maintain, NULL);

	sleep (3);
	printf("Process start up...\n");
	
	InitLogger(g_rConfig.sLogPath.c_str());
	WriteLogByLevel(1, "GPS Server startup!\n");

	int nThreadNo[MAX_PROC_GPS_THREAD];
	for (int i = 0; i < MAX_PROC_GPS_THREAD; ++i)
	{ 
		nThreadNo[i] = i;
		pthread_create(&pid, &threadAttr, ProcGPSMsgThread, &(nThreadNo[i]));
		pthread_create(&pid, &threadAttr, RecvGPSMsgThread, &(nThreadNo[i]));
		pthread_create(&pid, &threadAttr, SendGPSMsgThread, &(nThreadNo[i]));
	}

	pthread_create(&pid, &threadAttr, thread_SOAP_server_main, NULL);
	
	pthread_create(&pid, &threadAttr, ProcDeadTerm, NULL);

	//pthread_create(&pid, &threadAttr, ProcSessionTimeout, NULL);
	
	EpollProc(g_rConfig.nGPSServListenPort);
	
	pthread_attr_destroy (&threadAttr);
	WriteLogByLevel(1, "GPS Server exit!\n");
	return 0;
}

