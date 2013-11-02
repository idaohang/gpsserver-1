#ifndef _SESSION_H_
#define _SESSION_H_
#include "LogAdmin.h"
#include <string>
#include <map>

extern TConfig g_rConfig;

typedef struct 
{
	string sPhotoName;
}TTakePhotoResp;

typedef struct
{
	string sVersion;
}TQueryVerResp;

typedef struct
{
	int nResult;
}TResult;

typedef struct 
{
	int m_nProcFlag;
	time_t m_tTime;
	int m_nUserId;
	TTakePhotoResp m_rTakePhotoResp;
	TQueryVerResp m_rQueryVerResp;
	TResult m_rResult;
}TSession;

class CSessionMap
{
public:
	CSessionMap()
	{
		pthread_mutex_init(&m_lock, NULL);
	}
	~CSessionMap()
	{
		pthread_mutex_lock(&m_lock);
		std::map<int, TSession*>::iterator it = m_map.begin();
		for (; it != m_map.end(); ++it)
		{
			if (it->second != NULL)
			{
				free(it->second);
			}
		}
		pthread_mutex_unlock(&m_lock);
		pthread_mutex_destroy(&m_lock);
	}

	int Insert(int nUserId, TSession* pSession)
	{
		std::map<int, TSession*>::iterator it;
		printf("Insert..............1\n");
		pthread_mutex_lock(&m_lock);
		printf("Insert..............11\n");
		it = m_map.find(nUserId);
		if (it != m_map.end())
		{
			
			printf("Insert..............2\n");
			pthread_mutex_unlock(&m_lock);
			return -1;
		}
		
		printf("Insert..............3\n");
		m_map.insert(make_pair(nUserId, pSession));
		
		printf("Insert..............4\n");
		pthread_mutex_unlock(&m_lock);
		return 0;
	}

	void Delete(int nUserId)
	{
		std::map<int, TSession*>::iterator it;
		pthread_mutex_lock(&m_lock);
		it = m_map.find(nUserId);
		if (it == m_map.end())
		{
			pthread_mutex_unlock(&m_lock);
			return ;
		}
		if (!it->second)
		{
			free(it->second);
		}
		m_map.erase(nUserId);
		pthread_mutex_unlock(&m_lock);
		return;
	}

	TSession* Find(int nUserId)
	{
		TSession *p = NULL;
		std::map<int, TSession*>::iterator it ;
		pthread_mutex_lock(&m_lock);
		it = m_map.find(nUserId);
		if (it != m_map.end())
		{
			p = it->second;
		}
		pthread_mutex_unlock(&m_lock);
		return p;
	}

	void DeleteTimeout()
	{
		TSession *p = NULL;
		std::map<int, TSession*>::iterator it;
		pthread_mutex_lock(&m_lock);
		for (it = m_map.begin(); it != m_map.end(); ++it)
		{
			p = it->second;
			if (p == NULL)
			{
				m_map.erase(it);
				it = m_map.begin();
			}
			else if (p->m_tTime + g_rConfig.unSessionTimeout < time(NULL))
			{
				WriteLogByLevel(1, "Delete timeout session, userid:%d\n", p->m_nUserId);
				m_map.erase(it);
				free(p);
				it = m_map.begin();
			}
		}
		pthread_mutex_unlock(&m_lock);
	}
private:
	std::map<int, TSession*> m_map;
	pthread_mutex_t	m_lock;
};
#endif

