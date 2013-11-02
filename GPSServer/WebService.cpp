#include "stdio.h"
#include "soapH.h"
#include "GPSServiceSoapBinding.nsmap"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <iconv.h>
#include <errno.h>

#include "LogAdmin.h"
#include "GlobalStruct.h"
#include "SendToGPS.h"
#include "Session.h"

#define SOAP_THREADS 50

static int 	g_nCurThread = 0;
pthread_mutex_t queue_cs;
extern TConfig g_rConfig;
extern CTerminalList	g_rTermList;
extern CSessionMap g_rSessionMap[MAX_PROC_GPS_THREAD];


/*代码转换:从一种编码转为另一种编码*/
int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
	iconv_t cd;
	int rc;
	char **pin = &inbuf;
	char **pout = &outbuf;
	
	cd = iconv_open(to_charset,from_charset);
	if (cd==0) 
	return -1;
	memset(outbuf,0,outlen);
	if (iconv(cd,pin,(size_t*)&inlen,pout,(size_t*)&outlen)==-1) 
	{
		switch(errno)
		{
			case  E2BIG:
				printf("E2BiG");
				break;
			case  EILSEQ:
				printf("EILSEQ");
				break;
			case  EINVAL:
				printf("EINVAL");
				break;
		} 
		return -1;
	}
	iconv_close(cd);
	return 0;
}
/*UNICODE码转为GB2312码*/
int u2g(char *inbuf,int inlen,char *outbuf,int outlen)
{
	return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen);
}
/*GB2312码转为UNICODE码*/
int g2u(char *inbuf, int inlen,char *outbuf, int outlen)
{
	return code_convert("gb2312","utf-8",inbuf,inlen,outbuf,outlen);
}


class CIdGenerator
{
public:
	CIdGenerator(int nLen) : m_nLen(nLen)
	{
		pthread_mutex_init(&m_lock, NULL);
	}
	~CIdGenerator()
	{
		pthread_mutex_destroy(&m_lock);
	}

	unsigned int Get()
	{
		unsigned int unId;
		pthread_mutex_lock(&m_lock);
		unId = m_unId;
		m_unId = (m_unId + 1) % (10 * m_nLen);
		pthread_mutex_unlock(&m_lock);
		return unId;
	}

	string GetIdString()
	{
		char szBuf[16] = {0};
		sprintf(szBuf, "%u", this->Get());
		return string(szBuf);
	}
private:
	unsigned int m_unId;
	int m_nLen;
	pthread_mutex_t m_lock;
};

CIdGenerator g_rIdGenerator(4);


void *process_request(void*); 

void* thread_SOAP_server_main(void* nouseparam)
{
	int iretc;
	int i = 0;
	pthread_attr_t		ThreadAttr;	
	pthread_attr_init(&ThreadAttr);
	pthread_attr_setdetachstate(&ThreadAttr,PTHREAD_CREATE_DETACHED);		
	struct soap gSoap;
	SOAP_SOCKET m, s; 		// master and slave sockets 
	char szLogBuf[2048];	
	memset((void*)szLogBuf,0,sizeof(szLogBuf));
#ifdef NODOSOAP
	return NULL;
#endif
	pthread_mutex_init(&queue_cs, NULL); 

	pthread_mutex_lock(&queue_cs);	
	soap_init(&gSoap); 
	soap_set_mode(&gSoap, SOAP_C_UTFSTRING); 
	gSoap.mode |= SOAP_C_UTFSTRING; //关键 
	gSoap.connect_timeout =5;
	gSoap.send_timeout = g_rConfig.unWebserviceTimeout;
	gSoap.recv_timeout   = g_rConfig.unWebserviceTimeout;	//接收数据超时 
	gSoap.bind_flags = SO_REUSEADDR;		//地址重用	
	gSoap.accept_timeout = 5;
	gSoap.max_keep_alive=100;
	pthread_mutex_unlock(&queue_cs);
	
	m = soap_bind(&gSoap, g_rConfig.sServAddr.c_str(), g_rConfig.unServPort, 1000); 
	if (m < 0) 
	{
		sprintf(szLogBuf,"[Err]SOAP FAULT: [%s][%s]\n", *soap_faultcode(&gSoap), *soap_faultstring(&gSoap));
		printf(szLogBuf);
		soap_done(&gSoap);
		return NULL;
	}

	if(SOAP_THREADS<=1)	//启动单线程
	{
		sprintf(szLogBuf,"[Info]Single Thread Soap Start Succ....\n");
		//AAA_LOG(LM_DEBUG,"(%P|%t) %s", szLogBuf);
		WriteLogByLevel(1,szLogBuf);

		for (; ;) 
		{
			s = soap_accept(&gSoap); 
			if (s < 0) 
			{ 
				sprintf(szLogBuf,"[Err]Soap_Accept SOAP FAULT: [%s][%s]\n", *soap_faultcode(&gSoap), *soap_faultstring(&gSoap));
				//AAA_LOG(LM_ERROR,"(%P|%t) %s", szLogBuf);
				WriteLogByLevel(3,szLogBuf);
				continue;
			} 
			sprintf(szLogBuf,"[Info]Accepted connection from IP=%d.%d.%d.%d socket=%d\n", (gSoap.ip>>24)&0xFF, (gSoap.ip>>16)&0xFF, (gSoap.ip>>8)&0xFF, (gSoap.ip&0xFF), s);
			//AAA_LOG(LM_DEBUG,"(%P|%t)%s", szLogBuf);
			WriteLogByLevel(1,szLogBuf);

			if (soap_serve(&gSoap) != SOAP_OK) // process RPC request
			{
				sprintf(szLogBuf,"[Err]Soap_Server SOAP FAULT: [%s][%s]\n", *soap_faultcode(&gSoap), *soap_faultstring(&gSoap));
				//AAA_LOG(LM_ERROR,"(%P|%t) %s", szLogBuf);
				WriteLogByLevel(3,szLogBuf);
			}
			soap_destroy(&gSoap); // clean up class instances 
			soap_end(&gSoap); // clean up everything and close socket 
		 }
	}
	else
	{
		sprintf(szLogBuf,"[Info]Multi Thread Soap Start Succ....\n");
		//AAA_LOG(LM_DEBUG,"(%P|%t) %s", szLogBuf);
		WriteLogByLevel(1,szLogBuf);

		for (; ;) 
		{ 
			while(g_nCurThread <= g_rConfig.unMaxSession)
			{
				s = soap_accept(&gSoap); 
				if (!soap_valid_socket(s)) 
				{ 
					if(gSoap.errnum) 
					{ 
						sprintf(szLogBuf,"[Err]][Accept]SOAP FAULT: [%s][%s]\n", *soap_faultcode(&gSoap), *soap_faultstring(&gSoap));
						//AAA_LOG(LM_ERROR,"(%P|%t) %s", szLogBuf);
						WriteLogByLevel(3,szLogBuf);
					} 
					else
					{ 
					//	sprintf(szLogBuf, "Server timed out\n"); 
					} 
					continue; 		
				} 

				sprintf(szLogBuf, "[Info]Thread %d accepts socket %d connection from IP %d.%d.%d.%d\n", i, s, (gSoap.ip >> 24)&0xFF, (gSoap.ip >> 16)&0xFF, (gSoap.ip >> 8)&0xFF, gSoap.ip&0xFF); 
				//AAA_LOG(LM_DEBUG,"(%P|%t) %s", szLogBuf);
				WriteLogByLevel(1,szLogBuf);
				struct soap *soapArr = soap_copy(&gSoap);
				if(!soapArr)	{
					/*closesocket((SOAP_SOCKET) s );*/
					gSoap.fclosesocket(&gSoap, (SOAP_SOCKET) s );
					continue; 	
				}
				pthread_t tid;
				soapArr->socket = s; 	// new socket fd 
				iretc=pthread_create(&tid, &ThreadAttr, (void*(*)(void*))process_request, (void*)soapArr); 

				if(0==iretc) {
					sprintf(szLogBuf, "[Info]process_request start sumthreads=%d,iretc=%d\n", g_nCurThread,iretc); 
					//AAA_LOG(LM_DEBUG,"(%P|%t) %s", szLogBuf);
					WriteLogByLevel(1,szLogBuf);
					pthread_mutex_lock(&queue_cs);
					g_nCurThread++;
					pthread_mutex_unlock(&queue_cs);
				}else {
					sprintf(szLogBuf, "process_request create err=%d\n",iretc); 
					//AAA_LOG(LM_DEBUG,"(%P|%t) %s", szLogBuf);
					WriteLogByLevel(1,szLogBuf);
					soap_destroy(soapArr); // dealloc C++ data 
					soap_end(soapArr); // dealloc data and clean up 
					soap_done(soapArr); // detach soap struct 
					free(soapArr); 
				}
			}

			usleep(10000); 
		} 
	} 
	soap_done(&gSoap);
	pthread_mutex_destroy(&queue_cs); 
}


void *process_request(void *soap) 
{
	//int iretc;
	struct soap *pSp=(struct soap *)soap;
	pSp->send_timeout = 30;
	pSp->recv_timeout = 30;
	pSp->connect_timeout = 5;
	pSp->accept_timeout = 5;
	pSp->max_keep_alive=100;
	
	//int	tmpi=pSp->socket;
	soap_serve((struct soap*)soap); 
	soap_destroy((struct soap*)soap); // dealloc C++ data 
	soap_end((struct soap*)soap); // dealloc data and clean up 
	soap_done((struct soap*)soap); // detach soap struct 
	free((struct soap*)soap); 

	//线程数递减
	pthread_mutex_lock(&queue_cs);
	g_nCurThread--;
	//if(nCurThread<0)	nCurThread=0;
	pthread_mutex_unlock(&queue_cs);

	return NULL; 
}

TSession* CreateSession(string &sUserId, int nSockFd)
{
	if (nSockFd < 0)
	{
		return NULL;
	}
	TSession *pSession = new TSession;
	pSession->m_nProcFlag = 0;
	pSession->m_nUserId = atol(sUserId.c_str());
	pSession->m_tTime = time(NULL);
	g_rSessionMap[nSockFd % MAX_PROC_GPS_THREAD].Insert(pSession->m_nUserId, pSession);
	return pSession;
}

void DestroySession(string &sUserId, int nSockFd)
{
	if (nSockFd < 0)
	{
		return ;
	}
	g_rSessionMap[nSockFd % MAX_PROC_GPS_THREAD].Delete(atol(sUserId.c_str()));
	return ;
}



SOAP_FMAC5 int SOAP_FMAC6 ns3__takePhoto(struct soap* soap, struct ns2__TakePhotoReq *_in0, struct ns3__takePhotoResponse *_param_1)
{
	_param_1->_takePhotoReturn = (ns2__TakePhotoResp *)soap_malloc(soap, sizeof(ns2__TakePhotoResp));
	
	if (!_in0)
	{
		WriteLogByLevel(2, "In ns1__takePhoto, _in0 is NULL.\n");
		_param_1->_takePhotoReturn->result = -1;
		_param_1->_takePhotoReturn->photoName = (char *)soap_malloc(soap, 1);
		memset(_param_1->_takePhotoReturn->photoName, 0x00, 1);
		return SOAP_OK;
	}
	string sSimPhone = _in0->simPhone ? _in0->simPhone : "";
	int nSockFd = g_rTermList.GetSockFdBySimPhone(sSimPhone);
	if (nSockFd < 0)
	{
		WriteLogByLevel(2, "In ns1__takePhoto, could not find the terminal: %s.\n", sSimPhone.c_str());
		_param_1->_takePhotoReturn->result = -1;
		_param_1->_takePhotoReturn->photoName = (char *)soap_malloc(soap, 1);
		memset(_param_1->_takePhotoReturn->photoName, 0x00, 1);
		return SOAP_OK;
	}
	TTakePhotoReq req;
	req.nAction = _in0->action;
	req.nPhotoSize = _in0->photoSize;
	req.nCamID = _in0->camID;
	req.sUserID = g_rIdGenerator.GetIdString();
	TSession *pSession = CreateSession(req.sUserID, nSockFd);
	if (!pSession)
	{
		WriteLogByLevel(2, "In ns1__takePhoto, create session error.\n");
		_param_1->_takePhotoReturn->result = -1;
		_param_1->_takePhotoReturn->photoName = (char *)soap_malloc(soap, 1);
		memset(_param_1->_takePhotoReturn->photoName, 0x00, 1);
		return SOAP_OK;
	}
	time_t tSendTime = time(NULL);
	WriteLogByLevel(1, "[>%d<]WEBSERVICE>>>>>>>>>>TakePhotoReq, SimPhone:%s, UserId:%s, CamId:%d, Action:%d, PhotoSize:%d\n", \
			nSockFd, _in0->simPhone, req.sUserID.c_str(), req.nCamID, req.nAction, req.nPhotoSize);
	SendToGPSTakePhoto(req, nSockFd);
	while (pSession->m_nProcFlag != 2 && time(NULL) < tSendTime + g_rConfig.unWebserviceTimeout)
	{
		sleep (1);
	}
	if (pSession->m_nProcFlag == 2)
	{
		_param_1->_takePhotoReturn->result = pSession->m_rResult.nResult;
		_param_1->_takePhotoReturn->photoName = (char *)soap_malloc(soap, pSession->m_rTakePhotoResp.sPhotoName.length() + 1);
		strcpy(_param_1->_takePhotoReturn->photoName, pSession->m_rTakePhotoResp.sPhotoName.c_str());
		WriteLogByLevel(1, "[>%d<]WEBSERVICE<<<<<<<<<<TakePhotoResp, SimPhone:%s, Result:%d, PhotoName:%s\n", nSockFd, _in0->simPhone, _param_1->_takePhotoReturn->result, _param_1->_takePhotoReturn->photoName);
	}
	else 
	{
		_param_1->_takePhotoReturn->result = -1;
		_param_1->_takePhotoReturn->photoName = (char *)soap_malloc(soap, 1);
		memset(_param_1->_takePhotoReturn->photoName, 0x00, 1);
		WriteLogByLevel(1, "[>%d<]WEBSERVICE<<<<<<<<<<TakePhotoResp, SimPhone:%s, Result:%d\n", nSockFd, _in0->simPhone, _param_1->_takePhotoReturn->result);
	}
	
	DestroySession(req.sUserID, nSockFd);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 ns3__queryVersion(struct soap* soap, struct ns2__QueryVerReq *_in0, struct ns3__queryVersionResponse *_param_2)
{
	_param_2->_queryVersionReturn = (ns2__QueryVerResp *)soap_malloc(soap, sizeof(ns2__QueryVerResp));
	if (!_in0)
	{
		WriteLogByLevel(2, "In ns1__queryVersion, _in0 is NULL.\n");
		_param_2->_queryVersionReturn->result = -1;
		_param_2->_queryVersionReturn->version = (char *)soap_malloc(soap, strlen("not found") + 1);
		strcpy(_param_2->_queryVersionReturn->version, "not found");
		return SOAP_OK;
	}
	string sSimPhone = _in0->simPhone ? _in0->simPhone : "";
	int nSockFd = g_rTermList.GetSockFdBySimPhone(sSimPhone);
	if (nSockFd < 0)
	{
		WriteLogByLevel(2, "In ns1__queryVersion, could not find the terminal: %s.\n", sSimPhone.c_str());
		_param_2->_queryVersionReturn->result = -1;
		_param_2->_queryVersionReturn->version = (char *)soap_malloc(soap, strlen("not found") + 1);
		strcpy(_param_2->_queryVersionReturn->version, "not found");
		return SOAP_OK;
	}
	TVersionReq req;
	req.sUserID = g_rIdGenerator.GetIdString();
	TSession *pSession = CreateSession(req.sUserID, nSockFd);
	if (!pSession)
	{
		WriteLogByLevel(2, "In ns1__queryVersion, create session error.\n");
		_param_2->_queryVersionReturn->result = -1;
		_param_2->_queryVersionReturn->version = (char *)soap_malloc(soap, strlen("not found") + 1);
		strcpy(_param_2->_queryVersionReturn->version, "not found");
		return SOAP_OK;
	}
	time_t tSendTime = time(NULL);
	WriteLogByLevel(1, "[>%d<]WEBSERVICE>>>>>>>>>>QueryVersionReq, SimPhone:%s, UserId:%s\n", nSockFd, _in0->simPhone, req.sUserID.c_str());
	SendToGPSVersion(req, nSockFd);
	while (pSession->m_nProcFlag != 2 && time(NULL) < tSendTime + g_rConfig.unWebserviceTimeout)
	{
		sleep (1);
	}
	if (pSession->m_nProcFlag == 2)
	{
		_param_2->_queryVersionReturn->result = pSession->m_rResult.nResult;
		_param_2->_queryVersionReturn->version = (char *)soap_malloc(soap, pSession->m_rQueryVerResp.sVersion.length() + 1);
		strcpy(_param_2->_queryVersionReturn->version, pSession->m_rQueryVerResp.sVersion.c_str());
		WriteLogByLevel(1, "[>%d<]WEBSERVICE<<<<<<<<<<QueryVersionResp, SimPhone:%s, Result:%d, Version:%s\n", nSockFd, _in0->simPhone, _param_2->_queryVersionReturn->result, _param_2->_queryVersionReturn->version);
	}
	else 
	{
		_param_2->_queryVersionReturn->result = -1;
		_param_2->_queryVersionReturn->version = (char *)soap_malloc(soap, strlen("not found") + 1);
		strcpy(_param_2->_queryVersionReturn->version, "not found");
		WriteLogByLevel(1, "[>%d<]WEBSERVICE<<<<<<<<<<QueryVersionResp, SimPhone:%s, Result:%d\n", nSockFd, _in0->simPhone, _param_2->_queryVersionReturn->result);
	}

	DestroySession(req.sUserID, nSockFd);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 ns3__sendMessage(struct soap* soap, struct ns2__SendMsgReq *_in0, struct ns3__sendMessageResponse *out)
{
	out->_sendMessageReturn = (ns2__SendMsgResp *)soap_malloc(soap, sizeof(ns2__SendMsgResp));

	if (!_in0)
	{
		WriteLogByLevel(2, "In ns1__sendMessage, _in0 is NULL.\n");
		out->_sendMessageReturn->result = -1;
		return SOAP_OK;
	}
	
	string sSimPhone = _in0->simPhone ? _in0->simPhone : "";
	int nSockFd = g_rTermList.GetSockFdBySimPhone(sSimPhone);
	if (nSockFd < 0)
	{
		WriteLogByLevel(2, "In ns1__sendMessage, could not find the terminal: %s.\n", sSimPhone.c_str());
		out->_sendMessageReturn->result = -1;
		return SOAP_OK;
	}
	printf("=========================================================\n");
	int len = strlen(_in0->message);
	for (int i = 0; i < len; ++i)
	{
		printf("%02x ", *(unsigned char *)(_in0->message + i));
	}
	printf("\n");
	char szTmp[256] = {0};
	u2g(_in0->message, len, szTmp, 256);
	for (int i = 0; i < len; ++i)
	{
		printf("%02x ", *(unsigned char *)(szTmp + i));
	}
	printf("\n");
	TControlInfoReq req;
	req.sUserID = g_rIdGenerator.GetIdString();
	req.sInfo = szTmp;//_in0->message;
	TSession *pSession = CreateSession(req.sUserID, nSockFd);
	if (!pSession)
	{
		WriteLogByLevel(2, "In ns1__sendMessage, create session error.\n");
		out->_sendMessageReturn->result = -1;
		return SOAP_OK;
	}
	time_t tSendTime = time(NULL);
	WriteLogByLevel(1, "[>%d<]WEBSERVICE>>>>>>>>>>SendMessageReq, SimPhone:%s, UserId:%s, Message:%s\n", nSockFd, _in0->simPhone, req.sUserID.c_str(), szTmp);
	SendToGPSControlInfo(req, nSockFd);
	while (pSession->m_nProcFlag == 0 && time(NULL) < tSendTime + g_rConfig.unWebserviceTimeout)
	{
		sleep (1);
	}
	if (pSession->m_nProcFlag != 0)
	{
		out->_sendMessageReturn->result = pSession->m_rResult.nResult;
	}
	else 
	{
		out->_sendMessageReturn->result = -1;
	}
	WriteLogByLevel(1, "[>%d<]WEBSERVICE<<<<<<<<<<SendMessageResp, SimPhone:%s, Result:%d\n", nSockFd, _in0->simPhone, out->_sendMessageReturn->result);

	DestroySession(req.sUserID, nSockFd);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 ns3__setGPSFrequency(struct soap* soap, struct ns2__SetGPSFrequencyReq *_in0, struct ns3__setGPSFrequencyResponse * out)
{
	out->_setGPSFrequencyReturn = (ns2__SetGPSFrequencyResp *)soap_malloc(soap, sizeof(ns2__SetGPSFrequencyResp));

	if (!_in0)
	{
		WriteLogByLevel(2, "In ns1__setGPSFrequency, _in0 is NULL.\n");
		out->_setGPSFrequencyReturn->result = -1;
		return SOAP_OK;
	}
	
	string sSimPhone = _in0->simPhone ? _in0->simPhone : "";
	int nSockFd = g_rTermList.GetSockFdBySimPhone(sSimPhone);
	if (nSockFd < 0)
	{
		WriteLogByLevel(2, "In ns1__setGPSFrequency, could not find the terminal: %s.\n", sSimPhone.c_str());
		out->_setGPSFrequencyReturn->result = -1;
		return SOAP_OK;
	}
	TPositionReq req;
	req.sUserID = g_rIdGenerator.GetIdString();
	req.nInterval = _in0->interval;
	req.nTime = _in0->times;
	TSession *pSession = CreateSession(req.sUserID, nSockFd);
	if (!pSession)
	{
		WriteLogByLevel(2, "In ns1__setGPSFrequency, create session error.\n");
		out->_setGPSFrequencyReturn->result = -1;
		return SOAP_OK;
	}
	time_t tSendTime = time(NULL);
	WriteLogByLevel(1, "[>%d<]WEBSERVICE>>>>>>>>>>SetGPSFrequencyReq, SimPhone:%s, UserId:%s, Interval:%d, Times:%d\n", nSockFd, _in0->simPhone, req.sUserID.c_str(), _in0->interval, _in0->times);
	SendToGPSPosition(req, nSockFd);
	while (pSession->m_nProcFlag == 0 && time(NULL) < tSendTime + g_rConfig.unWebserviceTimeout)
	{
		sleep (1);
	}
	if (pSession->m_nProcFlag != 0)
	{
		out->_setGPSFrequencyReturn->result = pSession->m_rResult.nResult;
	}
	else 
	{
		out->_setGPSFrequencyReturn->result = -1;
	}
	WriteLogByLevel(1, "[>%d<]WEBSERVICE<<<<<<<<<<SetGPSFrequencyResp, SimPhone:%s, Result:%d\n", nSockFd, _in0->simPhone, out->_setGPSFrequencyReturn->result);

	DestroySession(req.sUserID, nSockFd);
	return SOAP_OK;
}





SOAP_FMAC5 int SOAP_FMAC6 ns3__setMileage(struct soap* soap, struct ns2__SetMileageReq *_in0, struct ns3__setMileageResponse * out)
{
	out->_setMileageReturn = (ns2__SetMileageResp *)soap_malloc(soap, sizeof(ns2__SetMileageResp));

	if (!_in0)
	{
		WriteLogByLevel(2, "In ns1__setMileage, _in0 is NULL.\n");
		out->_setMileageReturn->result = -1;
		return SOAP_OK;
	}
	
	string sSimPhone = _in0->simPhone ? _in0->simPhone : "";
	int nSockFd = g_rTermList.GetSockFdBySimPhone(sSimPhone);
	if (nSockFd < 0)
	{
		WriteLogByLevel(2, "In ns1__setMileage, could not find the terminal: %s.\n", sSimPhone.c_str());
		out->_setMileageReturn->result = -1;
		return SOAP_OK;
	}
	TSetMileageReq req;
	req.sUserID = g_rIdGenerator.GetIdString();
	req.nMileageInt = _in0->mileageInt;
	req.nMileageFloat = _in0->mileageFloat;
	TSession *pSession = CreateSession(req.sUserID, nSockFd);
	if (!pSession)
	{
		WriteLogByLevel(2, "In ns1__setMileage, create session error.\n");
		out->_setMileageReturn->result = -1;
		return SOAP_OK;
	}
	time_t tSendTime = time(NULL);
	WriteLogByLevel(1, "[>%d<]WEBSERVICE>>>>>>>>>>SetMileageReq, SimPhone:%s, UserId:%s, MileageInt:%d, MileageFloat:%d\n", nSockFd, _in0->simPhone, req.sUserID.c_str(), _in0->mileageInt, _in0->mileageFloat);
	SendToGPSSetMileage(req, nSockFd);
	while (pSession->m_nProcFlag == 0 && time(NULL) < tSendTime + g_rConfig.unWebserviceTimeout)
	{
		sleep (1);
	}
	if (pSession->m_nProcFlag != 0)
	{
		out->_setMileageReturn->result = pSession->m_rResult.nResult;
	}
	else 
	{
		out->_setMileageReturn->result = -1;
	}
	WriteLogByLevel(1, "[>%d<]WEBSERVICE<<<<<<<<<<SetMileageResp, SimPhone:%s, Result:%d\n", nSockFd, _in0->simPhone, out->_setMileageReturn->result);

	DestroySession(req.sUserID, nSockFd);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 ns3__updateSysConf(struct soap* soap, struct ns2__UpdateSysConfReq *_in0, struct ns3__updateSysConfResponse *out)
{
	out->_updateSysConfReturn = (ns2__UpdateSysConfResp *)soap_malloc(soap, sizeof(ns2__UpdateSysConfResp));

	if (!_in0)
	{
		WriteLogByLevel(2, "In ns1__updateSysConf, _in0 is NULL.\n");
		out->_updateSysConfReturn->result = -1;
		return SOAP_OK;
	}
	
	string sSimPhone = _in0->simPhone ? _in0->simPhone : "";
	int nSockFd = g_rTermList.GetSockFdBySimPhone(sSimPhone);
	if (nSockFd < 0)
	{
		WriteLogByLevel(2, "In ns1__updateSysConf, could not find the terminal: %s.\n", sSimPhone.c_str());
		out->_updateSysConfReturn->result = -1;
		return SOAP_OK;
	}
	TUpdateConfigReq req;
	req.sUserID = g_rIdGenerator.GetIdString();
	req.sCommand = _in0->command;
	TSession *pSession = CreateSession(req.sUserID, nSockFd);
	if (!pSession)
	{
		WriteLogByLevel(2, "In ns1__updateSysConf, create session error.\n");
		out->_updateSysConfReturn->result = -1;
		return SOAP_OK;
	}
	time_t tSendTime = time(NULL);
	WriteLogByLevel(1, "[>%d<]WEBSERVICE>>>>>>>>>>UpdateSysConfReq, SimPhone:%s, UserId:%s, Command:%s\n", nSockFd, _in0->simPhone, req.sUserID.c_str(), _in0->command);
	SendToGPSUpdateConfig(req, nSockFd);
	while (pSession->m_nProcFlag == 0 && time(NULL) < tSendTime + g_rConfig.unWebserviceTimeout)
	{
		sleep (1);
	}
	if (pSession->m_nProcFlag != 0)
	{
		out->_updateSysConfReturn->result = pSession->m_rResult.nResult;
	}
	else 
	{
		out->_updateSysConfReturn->result = -1;
	}
	WriteLogByLevel(1, "[>%d<]WEBSERVICE<<<<<<<<<<UpdateSysConfResp, SimPhone:%s, Result:%d\n", nSockFd, _in0->simPhone, out->_updateSysConfReturn->result);

	DestroySession(req.sUserID, nSockFd);
	return SOAP_OK;
}


SOAP_FMAC5 int SOAP_FMAC6 ns3__updateServerInfo(struct soap* soap, struct ns2__UpdateServerInfoReq *_in0, struct ns3__updateServerInfoResponse *out)
{
	out->_updateServerInfoReturn = (ns2__UpdateServerInfoResp *)soap_malloc(soap, sizeof(ns2__UpdateServerInfoResp));

	if (!_in0)
	{
		WriteLogByLevel(2, "In ns1__updateServerInfo, _in0 is NULL.\n");
		out->_updateServerInfoReturn->result = -1;
		return SOAP_OK;
	}
	
	string sSimPhone = _in0->simPhone ? _in0->simPhone : "";
	int nSockFd = g_rTermList.GetSockFdBySimPhone(sSimPhone);
	if (nSockFd < 0)
	{
		WriteLogByLevel(2, "In ns1__updateServerInfo, could not find the terminal: %s.\n", sSimPhone.c_str());
		out->_updateServerInfoReturn->result = -1;
		return SOAP_OK;
	}
	TServInfoReq req;
	req.sUserID = g_rIdGenerator.GetIdString();
	req.sCammandID = "0";
	req.sLogServerIp = _in0->logServerIp;
	req.sPassword = _in0->password;
	req.sUserName = _in0->userName;
	req.sUpdateServerIp = _in0->updateServerIp;
	char szPort[8] = {0};
	sprintf(szPort, "%u", _in0->updateServerPort);
	req.sUpdateServerPort = szPort;
	sprintf(szPort, "%u", _in0->logServerPort);
	req.sLogServerPort = szPort;
	TSession *pSession = CreateSession(req.sUserID, nSockFd);
	if (!pSession)
	{
		WriteLogByLevel(2, "In ns1__updateServerInfo, create session error.\n");
		out->_updateServerInfoReturn->result = -1;
		return SOAP_OK;
	}
	time_t tSendTime = time(NULL);
	WriteLogByLevel(1, "[>%d<]WEBSERVICE>>>>>>>>>>UpdateServerInfoReq, SimPhone:%s, UserId:%s\n", nSockFd, _in0->simPhone, req.sUserID.c_str());
	SendToGPSServInfo(req, nSockFd);
	while (pSession->m_nProcFlag == 0 && time(NULL) < tSendTime + g_rConfig.unWebserviceTimeout)
	{
		sleep (1);
	}
	if (pSession->m_nProcFlag != 0)
	{
		out->_updateServerInfoReturn->result = pSession->m_rResult.nResult;
	}
	else 
	{
		out->_updateServerInfoReturn->result = -1;
	}
	WriteLogByLevel(1, "[>%d<]WEBSERVICE<<<<<<<<<<UpdateServerInfoReq, SimPhone:%s, Result:%d\n", nSockFd, _in0->simPhone, out->_updateServerInfoReturn->result);

	DestroySession(req.sUserID, nSockFd);
	return SOAP_OK;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns1__OperationSms(struct soap* soap, struct _ns1__OperationSms *ns1__OperationSms, struct _ns1__OperationSmsResponse *ns1__OperationSmsResponse)
{
	return SOAP_ERR;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns1__OperationCenter(struct soap* soap, struct _ns1__OperationCenter *ns1__OperationCenter, struct _ns1__OperationCenterResponse *ns1__OperationCenterResponse)
{
	return SOAP_ERR;
}

SOAP_FMAC5 int SOAP_FMAC6 __ns1__sayHello(struct soap* soap, struct _ns1__sayHello *ns1__sayHello, struct _ns1__sayHelloResponse *ns1__sayHelloResponse)
{
	return SOAP_ERR;
}
/*
int main()
{
	thread_SOAP_server_main(NULL);
	while (1)
	{
		sleep (1);
	}
	return 0;
}
*/
int CallSendMsg(char *szMsgContent)
{
	struct soap soap;
	soap_init(&soap);
	soap.connect_timeout = 2;
	soap.send_timeout = 5;
	soap.recv_timeout = 5;
	struct _ns1__OperationCenter req;
	struct _ns1__OperationCenterResponse resp;

	req.in0 = (char *)soap_malloc(&soap, strlen(szMsgContent) + 1);
	memset(req.in0, 0x00, strlen(szMsgContent) + 1);
	strcpy(req.in0, szMsgContent);
	WriteLogByLevel(1, "CallSendMsg addr:%s, content:%s\n", g_rConfig.sSMSCenterWebAddr.c_str(), req.in0 ? req.in0 : "NULL");
	if ( soap_call___ns1__OperationCenter(&soap, g_rConfig.sSMSCenterWebAddr.c_str(), NULL, &req, &resp) != SOAP_OK)
	{
		WriteLogByLevel(1, "CallSendMsg error! [%s:%s]\n",  *soap_faultcode(&soap), *soap_faultstring(&soap));
		return -1;
	}
	WriteLogByLevel(1,  "CallSendMsg success! Resp:%s\n", resp.out == NULL ? "NULL" : resp.out);
	return 0;
}


