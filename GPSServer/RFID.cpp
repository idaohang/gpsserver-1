#include "RFID.h"
#include <string>
#include <list>
#include "LogAdmin.h"
#include "GlobalStruct.h"
#include "Database.h"
#include "SendToGPS.h"
using namespace std;

#define PRESET_VALUE 0xFFFF 
#define POLYNOMIAL  0x8408 

extern CShipSerialMap g_rShipSerialMap;
extern CTerminalList	g_rTermList;
extern CDatabase g_rDatabaseList[MAX_PROC_GPS_THREAD];
extern string GetMemHexString(const char *szBuf, int nLen);

unsigned int uiCrc16Cal(unsigned char  const    * pucY, unsigned char ucX) 
{ 
	unsigned char ucI,ucJ; 
	unsigned short int	uiCrcValue = PRESET_VALUE; 

	for(ucI = 0; ucI < ucX; ucI++) 
	{ 
		uiCrcValue = uiCrcValue ^ *(pucY + ucI); 
		for(ucJ = 0; ucJ < 8; ucJ++) 
		{ 
			if(uiCrcValue & 0x0001) 
			{ 
				uiCrcValue = (uiCrcValue >> 1) ^ POLYNOMIAL; 
			} 
			else 
			{ 
				uiCrcValue = (uiCrcValue >> 1); 
			} 
		} 
	}
	return uiCrcValue; 
}

string EncodeRequestPkg(char cCmd, char *szData, int nDataLen)
{
	char szBuf [128] = {0};
	int nLen = ( nDataLen > 92 ? 92 : nDataLen ) + 4;
	szBuf[0] = *((char *)&nLen);
	szBuf[1] = 0x00;
	szBuf[2] = cCmd;
	if (szData != NULL)
		memcpy(szBuf + 3, szData, nLen - 4);
	unsigned int unCRC = uiCrc16Cal((unsigned char *)szBuf, szBuf[0] - 1);
	szBuf[nLen - 1] = *(char *)(&unCRC);
	szBuf[nLen] = *((char *)&unCRC + 1);
	string sReqPkg(szBuf, nLen + 1);
	WriteLogByLevel(1, "In EncodeRequestPkg, cCmd = %02x\n", (unsigned char)cCmd);
	printf("**************SEND TO RFID****************\n");
	for (int i = 0;  i < sReqPkg.length(); ++i)
	{
		printf("%02x ", *(unsigned char *)(sReqPkg.data() + i));
	}
	printf("\n*****************END**********************\n");
	return sReqPkg;
}

int ProcEPCData(string &sEPCData, int nThreadId, string &sSimPhone)
{
	WriteLogByLevel(1, "ProcEPCData, sSimPhone: %s EPC: %s\n", sSimPhone.c_str(), sEPCData.c_str());
	string sSerial = g_rShipSerialMap.GetSerial(sSimPhone);
	if (sSerial.length() == 0)
	{
		WriteLogByLevel(1, "[%s] ProcEPCData, but the serial id is not found. epc:%s\n", sSimPhone.c_str(), sSerial.c_str());
		return -1;
	}
	char  szBuf[1024] = {0};
	sprintf(szBuf, "insert into T_SHIP_BOX_LIST values('%s', '%s', now(), '%s');", \
			sEPCData.c_str(), sSerial.c_str(), sSimPhone.c_str());
	string sql (szBuf);
	return g_rDatabaseList[nThreadId].ExecSQL(sql);
}

int ProcScanEPCResp(char cStat, const char *szData, char cDataLen, int nThreadId, string &sSimPhone)
{
	WriteLogByLevel(1, "In ProcScanEPCResp, sSimPhone = %s, cStat = %02x\n", sSimPhone.c_str(), (unsigned char)cStat);
	char cCount = szData[0];	//EPC个数
	switch (cStat)
	{
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
		{
			if (cCount < 0)
			{
				break;
			}
			int nPos = 1;
			for (int i = 0; i < cCount; ++i)
			{
				char cEPCLen = szData[nPos];	//EPC长度
				string sEPCData(szData + nPos + 1, cEPCLen);
				nPos = cEPCLen + 1;
				string sNum = GetMemHexString(sEPCData.data(), sEPCData.length());
				ProcEPCData(sNum, nThreadId, sSimPhone);
			}
		}
		break;
	default:
		break;
	}
	
	if (cStat != 0x03 && g_rTermList.GetRFIDWorkingFlagBySimPhone(sSimPhone) == 1)	//0x03,说明这条数据结束后，后面还有数据
	{
		SendToGPSScanEPC(g_rTermList.GetSockFdBySimPhone(sSimPhone));
	}
	return 0;
}

int DecodeRespPacket(string &sRespPkg, int nThreadId, string &sSimPhone)
{
	char cLen = sRespPkg.data()[0];
	if (uiCrc16Cal((unsigned char *)(sRespPkg.data()), cLen + 1) != 0)
	{
		WriteLogByLevel(1, "In DecodeRespPacket, but CRC error! simPhone:%s, cmd:%c\n", sSimPhone.c_str(), sRespPkg.data()[2]);
		if (g_rTermList.GetRFIDWorkingFlagBySimPhone(sSimPhone) == 1)
		{
			usleep(500);
			SendToGPSScanEPC(g_rTermList.GetSockFdBySimPhone(sSimPhone));
		}
		return -1;
	}
	char cAdr = sRespPkg.data()[1];
	char cCmd = sRespPkg.data()[2];
	char cStatus = sRespPkg.data()[3];
	const char *szData = sRespPkg.data() + 4;
	char cDataLen = cLen - 5;
	WriteLogByLevel(1, "In DecodeRespPacket, sSimPhone = %s, cmd:%02x\n" , sSimPhone.c_str(), (unsigned char)cCmd);
	switch(cCmd)
	{
	case 0x01:	//EPC 查询标签响应
		ProcScanEPCResp(cStatus, szData, cDataLen, nThreadId, sSimPhone);
		break;
	case 0x02:	//EPC 读数据响应
		break;
	case 0x50:	//6B 查询标签响应
		break;
	case 0x52:	//6B 读数据响应
		break;
	case 0x24:	//设置读写器地址响应
		break;
	case 0x25:	//设置查询时间间隔
		break;
	case 0x35:	//设置工作模式响应
		break;
	default:
		break;
	}
	
	return 0;
}

string RFIDScanEPC()
{
	return EncodeRequestPkg(0x01, NULL, 0);
}


//设置读卡器地址
string RFIDSetReaderAdr(char cAddr)	
{
	return EncodeRequestPkg(0x24, &cAddr, 1);
}

//设置查询时间，实际设置时间为cTime*100ms，默认为10*100ms，
//cTime为0x00--0x02时，回复默认值
string RFIDSetScanTime(char cTime)
{
	return EncodeRequestPkg(0x25, &cTime, 1);
}

//设置功率，范围0--30
string RFIDSetPower(char cPower)
{
	return EncodeRequestPkg(0x2F, &cPower, 1);
}

//设置为应答模式
string RFIDSetAnswerMode()
{
	char szParam[6] = {0};
	memset(szParam, 0x00, sizeof(szParam));
	return EncodeRequestPkg(0x35, szParam, 6);
}

//设置读卡器波特率
//0:9600, 1:19200, 2:38400, 3:43000, 4:56000, 5:57600, 6:115200
string RFIDSetReaderBaudRate(char cBaudRate)	
{
	return EncodeRequestPkg(0x28, &cBaudRate, 1);
}



