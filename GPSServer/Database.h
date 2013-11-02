#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <iostream>
#include <mysql.h>
#include <string>
#include <stdio.h>
#include "LogAdmin.h"
using namespace std;

class CDatabase
{
public:
	CDatabase()
	{
		mysql_init(&mysql);
		m_bConStat = false;
	}
	
	~CDatabase()
	{
		ReleaseQueryRet();
		Close();
	}

	void SetDbInfo(const char *pHost, const char *pUser, const char *pPassword, const char *pDB, unsigned int unPort = 0)
	{
		m_sDbHost = pHost;
		m_unDbPort = unPort;
		m_sDbName = pDB;
		m_sUserName = pUser;
		m_sPassword = pPassword;
	}

	int Connect(const char *pHost, const char *pUser, const char *pPassword, const char *pDB, unsigned int unPort = 0)
	{
		SetDbInfo(pHost, pUser, pPassword, pDB, unPort);
		return Connect();
	}

	int Connect()
	{
		if (!mysql_real_connect(&mysql, m_sDbHost.c_str(), m_sUserName.c_str(), m_sPassword.c_str(), m_sDbName.c_str(), m_unDbPort, NULL, 0))
		{
			WriteLogByLevel(3, "Failed to connect to database, Error: %s\n", mysql_error(&mysql));
			return -1;
		}
		WriteLogByLevel(1, "Connect to database:%s success.\n", m_sDbName.c_str());
		m_bConStat = true;
		mysql_query(&mysql, "SET NAMES  'utf8' ");//防止查询中文乱码
		return 0;
	}

	void Close()
	{
		WriteLogByLevel(1, "Close database connection.\n");
		if (m_bConStat == false)
		{
			return;
		}
		ReleaseQueryRet();
		mysql_close(&mysql);
		m_bConStat = false;
	}

	int ExecSQL(string &sSQL)
	{
		if (!m_bConStat)
		{	
			Connect();
		}
		if (!m_bConStat)
		{
			WriteLogByLevel(2, "ExecSQL:[%s], but the database is not connected.\n", sSQL.c_str());
			return -1;
		}
		int nRet = mysql_real_query(&mysql, sSQL.data(), sSQL.length());
		if (nRet == 0)
		{
			WriteLogByLevel(1, "ExecSQL:[%s] success.\n", sSQL.c_str());
			return 0;
		}
		WriteLogByLevel(2, "ExecSQL:[%s] fail, nRet = %d, mysql_errno() = %d, error:%s\n", sSQL.c_str(), nRet, mysql_errno(&mysql), mysql_error(&mysql));
		if (mysql_errno(&mysql) == 1062)
		{
			//违反主键约束，不关闭连接
			return 0;
		}
		m_bConStat = false;
		Close();
		return -1;
	}

	int QuerySQL(string &sSQL)
	{
		if (ExecSQL(sSQL) != 0)
		{
			return -1;
		}
		pQueryRet = mysql_store_result(&mysql);
		if (pQueryRet == NULL)
			return -1;
		int nRetCount = mysql_field_count(&mysql);
		if (pQueryRet == NULL)
		{
			if (nRetCount == 0)
				return 0;
			else
			{
				//取结果失败
				WriteLogByLevel(2, "mysql_store_result error: %s\n", mysql_error(&mysql));
				return -1;
			}
		}
		return nRetCount;
	}

	char** FetchRow()
	{
		return mysql_fetch_row(pQueryRet);
	}
	
	void ReleaseQueryRet()
	{
		if (pQueryRet != NULL)
		{
			mysql_free_result(pQueryRet);
			pQueryRet = NULL;
		}
	}
private:
	MYSQL_RES *pQueryRet;
	bool m_bConStat;
	MYSQL mysql;

	string m_sDbHost;
	unsigned int m_unDbPort;
	string m_sDbName;
	string m_sUserName;
	string m_sPassword;
};


#endif

