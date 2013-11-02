#ifndef	_GLOBAL_STRUCT_H_
#define _GLOBAL_STRUCT_H_
#include <stdio.h>
#include <string.h>
#include <pthread.h>
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
#include <list>
#include <net/if.h>
#include <net/if_arp.h>

#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <map>
#include <set>
#include "LogAdmin.h"

using namespace std;

#define RECV_TIMEOUT		5
#define SEND_TIMEOUT		5
#define MAX_BUFFER_LEN	1024
#define	CLIENT_KEY_LEN	4				
#define MAX_CLIENT_NUM	6000

//#define DEBUG_BIN

#pragma pack(1)
typedef struct 
{
	string sLogPath;
	string sGPSServIP;
	int nGPSServListenPort;
	string sPhotoPath;
	string sDbHost;
	unsigned int unDbPort;
	string sDbName;
	string sDbUser;
	string sDbPassword;
	set<int> rAlertIdMaskSet;
	unsigned int unKeepAlive;
	string sServAddr;
	unsigned int unServPort;
	unsigned int unMaxSession;
	unsigned int unWebserviceTimeout;
	unsigned int unSessionTimeout;
	string sSMSCenterWebAddr;
	string s3desKey;
}TConfig;

typedef struct TBuffer
{
	char		sBuffer[MAX_BUFFER_LEN];
	unsigned int	unPacketLen;
}TBuffer;

typedef struct TGPSJob
{
	int		nSockFd;
}TGPSJob;

typedef struct 
{
	int	nSockFd;
	char * pBuffer;
	unsigned int unLen;
}TPacket;
#pragma pack()

extern TConfig g_rConfig;

template <typename T>
class CDataQueue 
{
public:	
	CDataQueue(const int MaxQueueSize = 5120)
	{
		MaxSize = MaxQueueSize +1;
		queue = new T[MaxSize];
		MakeNull();
	}
	~CDataQueue()
	{
		delete queue;
	}
public:
	T *queue;
    int  front;
    int  rear;
	int MaxSize;

public:
	void MakeNull(void)
	{
		front = 0;
		rear  = 0;
	}

	int IsEmpty(void)
	{
		if( front == rear)
			return 1;
		else 
			return 0;
	}

	int IsFull()
	{
		if( (rear+1)%MaxSize == front )
			return 1;
		else
			return 0;
	}

	int WriteDataToQueue(T *pData)
	{
		struct timespec slptm;
		slptm.tv_sec = 0;
    slptm.tv_nsec = 100000;      
		for(int i = 0; i < 10; i++)
		{
			if(!IsFull())
			{					
				memcpy(&(queue[rear]), pData, sizeof(T));
				rear = (rear +1) % MaxSize;
				return 0;
			}
			else
			{
			}
			//nanosleep(&slptm, NULL);
		}
		return -1;
	}


	int ReadDataFromQueue(T * pData)
	{
		if(!IsEmpty())
		{			
			memcpy(pData, &(queue[front]), sizeof(T));
			front = (front+1) % MaxSize;
			return 0;
		}
		return -1;
	}

	int GetQueueSize()
	{
		int nSize = (rear-front+MaxSize) % MaxSize;
		return nSize;
	}

};

typedef CDataQueue<TGPSJob> CGPSJobQueue;
typedef CDataQueue<TPacket> CPacketQueue;
#define MAX_PROC_GPS_THREAD 10

class CTerminal
{
public:
	void SetSimPhone(string &sSimPhone)
	{
		this->sSimPhone = sSimPhone;
	}
	string GetSimPhone()
	{
		return sSimPhone;
	}

	void SetRFIDWorkingFlag(int nFlag)
	{
		nRFIDWorkingFlag = nFlag;
	}

	int GetRFIDWorkingFlag()
	{
		return nRFIDWorkingFlag;
	}
	string sSimPhone;
	time_t tKeepAlive;
	int nRFIDWorkingFlag;
};

class CTerminalList
{
public:
	CTerminalList()
	{
		pthread_rwlock_init(&m_lock, NULL);
	}
	~CTerminalList()
	{
		pthread_rwlock_destroy(&m_lock);
	}
	void Insert(int nSockFd, string sSimPhone)
	{
		CTerminal rTerm;
		rTerm.SetSimPhone(sSimPhone);
		rTerm.tKeepAlive = time(NULL);
		rTerm.nRFIDWorkingFlag = 0;
		pthread_rwlock_wrlock(&m_lock);
		m_rSockFdTermMap.insert(make_pair(nSockFd, rTerm));
		pthread_rwlock_unlock(&m_lock);
	}
	void Erase(int nSockFd)
	{
		pthread_rwlock_wrlock(&m_lock);
		m_rSockFdTermMap.erase(nSockFd);
		pthread_rwlock_unlock(&m_lock);
	}
	string GetSimPhoneBySockFd(int nSockFd)
	{
		string sSimPhone;
		std::map<int, CTerminal>::iterator it;
		pthread_rwlock_rdlock(&m_lock);
		it = m_rSockFdTermMap.find(nSockFd);
		if (it == m_rSockFdTermMap.end())
		{
			sSimPhone = "";
		}
		else
		{
			sSimPhone = it->second.GetSimPhone();
			//WriteLogByLevel(1, "[>%d<]GetSimPhoneBySockFd, simPhone:%s, tKeepAlive:%u\n", nSockFd, sSimPhone.c_str(), it->second.tKeepAlive);
			it->second.tKeepAlive = time(NULL);	//有收到消息包
		}
		pthread_rwlock_unlock(&m_lock);
		return sSimPhone;
	}
	int GetSockFdBySimPhone(string sSimPhone)
	{
		pthread_rwlock_rdlock(&m_lock);
		std::map<int, CTerminal>::iterator it = m_rSockFdTermMap.begin();
		for (; it != m_rSockFdTermMap.end(); ++it)
		{
			if (it->second.GetSimPhone() == sSimPhone)
			{
				pthread_rwlock_unlock(&m_lock);
				return it->first;
			}
		}
		pthread_rwlock_unlock(&m_lock);
		return -1;
	}

	int GetTermBySockFd(int nSockFd, CTerminal &rTerm)
	{
		int nRet;
		string sSimPhone;
		std::map<int, CTerminal>::iterator it;
		pthread_rwlock_rdlock(&m_lock);
		it = m_rSockFdTermMap.find(nSockFd);
		if (it == m_rSockFdTermMap.end())
		{
			nRet = -1;
		}
		else
		{
			rTerm = it->second;
			nRet = 0;
		}
		pthread_rwlock_unlock(&m_lock);
		return nRet;
	}

	int SetRFIDWorkingFlagBySimPhone(string &sSimPhone, int nWorkingFlag)
	{
		pthread_rwlock_rdlock(&m_lock);
		std::map<int, CTerminal>::iterator it = m_rSockFdTermMap.begin();
		for (; it != m_rSockFdTermMap.end(); ++it)
		{
			if (it->second.GetSimPhone() == sSimPhone)
			{
				it->second.SetRFIDWorkingFlag(nWorkingFlag);
				pthread_rwlock_unlock(&m_lock);
				return 0;
			}
		}
		pthread_rwlock_unlock(&m_lock);
		return -1;
	}

	int GetRFIDWorkingFlagBySimPhone(string &sSimPhone)
	{
		pthread_rwlock_rdlock(&m_lock);
		std::map<int, CTerminal>::iterator it = m_rSockFdTermMap.begin();
		int nFlag = 0;
		for (; it != m_rSockFdTermMap.end(); ++it)
		{
			if (it->second.GetSimPhone() == sSimPhone)
			{
				nFlag = it->second.GetRFIDWorkingFlag();
				pthread_rwlock_unlock(&m_lock);
				return nFlag;
			}
		}
		pthread_rwlock_unlock(&m_lock);
		return -1;
	}
	
	int SetKeepAliveBySockFd(int nSockFd)
	{
		int nRet;
		std::map<int, CTerminal>::iterator it;
		pthread_rwlock_rdlock(&m_lock);
		it = m_rSockFdTermMap.find(nSockFd);
		if (it == m_rSockFdTermMap.end())
		{
			nRet = -1;
		}
		else
		{
			it->second.tKeepAlive = time(NULL);
			nRet = 0;
		}
		pthread_rwlock_unlock(&m_lock);
		return nRet;
	}

	int GetDeadTerms(std::list<int> &rDeadTermSockFdList)
	{
		int nCount = 0;
		time_t tNow = time(NULL);
		pthread_rwlock_rdlock(&m_lock);
		std::map<int, CTerminal>::iterator it = m_rSockFdTermMap.begin();
		for (; it != m_rSockFdTermMap.end(); ++it)
		{
			if (it->second.tKeepAlive + g_rConfig.unKeepAlive < tNow)
			{
				rDeadTermSockFdList.push_back(it->first);
				++nCount;
			}
		}
		pthread_rwlock_unlock(&m_lock);
		return nCount;
	}
private:
	pthread_rwlock_t	m_lock;
	std::map<int, CTerminal> m_rSockFdTermMap;
};


class CImageBuf
{
public:
	CImageBuf()
	{
		pthread_mutex_init(&m_lock, NULL);
	}
	~CImageBuf()
	{
		pthread_mutex_destroy(&m_lock);
	}

	string GetImageBufBySockFd(int nSockFd)
	{
		pthread_mutex_lock(&m_lock);
		string sBuf = m_rSockFdImageBufMap[nSockFd];
		pthread_mutex_unlock(&m_lock);
		return sBuf;
	}

	void SetImageBufBySockFd(int nSockFd, string sImageBuf)
	{
		pthread_mutex_lock(&m_lock);
		if (m_rSockFdImageBufMap.find(nSockFd) != m_rSockFdImageBufMap.end())
		{
			m_rSockFdImageBufMap.erase(nSockFd);
		}
		m_rSockFdImageBufMap[nSockFd] =  sImageBuf;
		pthread_mutex_unlock(&m_lock);
	}

	void Erase(int nSockFd)
	{
		pthread_mutex_lock(&m_lock);
		m_rSockFdImageBufMap.erase(nSockFd);
		pthread_mutex_unlock(&m_lock);
	}
private:
	std::map<int, string> m_rSockFdImageBufMap;
	pthread_mutex_t	m_lock;
};


class CShipSerialMap
{
public:
	CShipSerialMap()
	{
		pthread_mutex_init(&m_lock, NULL);
	}
	~CShipSerialMap()
	{
		pthread_mutex_destroy(&m_lock);
	}

	string GetSerial(string &sSimPhone)
	{
		pthread_mutex_lock(&m_lock);
		string sSerial;
		try
		{
			sSerial = m_map.at(sSimPhone);
		}
		catch (...)
		{
			sSerial = "";
		}
		pthread_mutex_unlock(&m_lock);
		return sSerial;
	}
	void SetSerial(string &sSimPhone, string &sSerial)
	{
		pthread_mutex_lock(&m_lock);
		m_map[sSimPhone] = sSerial;
		pthread_mutex_unlock(&m_lock);
	}
	void EraseSerial(string &sSimPhone)
	{
		pthread_mutex_lock(&m_lock);
		m_map.erase(sSimPhone);
		pthread_mutex_unlock(&m_lock);
	}
private:
	std::map<string, string> m_map;
	pthread_mutex_t m_lock;
};

#endif
