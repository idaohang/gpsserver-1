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
#include <string>
#include <map>
#include <stdlib.h>
#include "GlobalStruct.h"
#include "API-Tools.h"
#include "API-Socket.h"
#include "LogAdmin.h"
#include "Database.h"
#include "SendToGPS.h"
#include "ProcGPSMsg.h"
#include "Session.h"
#include "RFID.h"
#include "3des.h"

using namespace std;

extern int g2u(char *inbuf, int inlen,char *outbuf, int outlen);

CShipSerialMap g_rShipSerialMap;
CTerminalList	g_rTermList;
CDatabase g_rDatabaseList[MAX_PROC_GPS_THREAD];
CImageBuf g_rImageBuf;
extern TConfig g_rConfig;
extern CSessionMap g_rSessionMap[MAX_PROC_GPS_THREAD];

extern int CallSendMsg(char *szMsgContent);

//szNum: "12 34 56 78"
//szBuf: 0x0C 0x22 0x38 0x4E
//nLen: ����szBuf���ֽ���
void SetMemNumString(char *szBuf, int nLen, const char *szNum)
{
	string sTmp = "";
	for (int i = strlen(szNum); i < nLen * 2; ++i)
	{
		sTmp += "0";
	}
	sTmp = sTmp + szNum;
	unsigned int unInt;
	unsigned char c;
	for (int i = 0; i < nLen; ++i)
	{
		unInt = atol(sTmp.substr(i * 2, 2).c_str());
		*(szBuf + i) = *((char *)&unInt);
	}
	return ;
}
//0x0C 0x22 0x38 0x4E --> "12 34 56 78"
string GetMemNumString(const char *szBuf, int nLen)
{
	unsigned int unTemp = 0;
	string sResult = "";
	char szStrValue[16] = {0};
	for (int i = 0; i < nLen; ++i)
	{
		sprintf(szStrValue, "%u", (unsigned char)(szBuf[i]));
		sResult += szStrValue;
	}
	return sResult;
}

void SetMemInt(char *szBuf, unsigned int nLen, const int nInt)
{
	printf("SetMemInt : ");
	for (int i = 0; i < 4; ++i)
	{
		printf("%02x ", *((char*)&nInt + i));
	}
	printf("\n");
	memcpy(szBuf, (char*)&nInt, nLen);
	return ;
}

unsigned int GetMemInt(char *szBuf, unsigned int nLen)
{
	unsigned int unResult = 0;
	memcpy(&unResult, szBuf, nLen > 4 ? 4 : nLen);
	return unResult;
}

//szNum: "12345678"
//szBuf: 0x12 0x23 0x56 0x78
//nLen: szBuf���ֽ���
void SetMemHexString(char *szBuf, int nLen, const char *szNum)
{
	string sTmp = "";
	for (int i = strlen(szNum); i < nLen * 2; ++i)
	{
		sTmp += "0";
	}
	sTmp = sTmp + szNum;
	unsigned int unHex;
	unsigned char c;
	for (int i = 0; i < nLen; ++i)
	{
		unHex = atol(sTmp.substr(i * 2, 1).c_str()) * 16 + atol(sTmp.substr(i * 2 + 1, 1).c_str());
		*(szBuf + i) = *((char *)&unHex);
	}
	return ;
}

//0x12 0x11 0x15 0x13-->"12111513"
string GetMemHexString(const char *szBuf, int nLen)
{
	unsigned int unTemp = 0;
	string sResult = "";
	char szStrValue[16] = {0};
	for (int i = 0; i < nLen; ++i)
	{
		sprintf(szStrValue, "%02x", (unsigned char)(szBuf[i]));
		sResult += szStrValue;
	}
	return sResult;
}

int ProcGPSLogout(TPacket &rPacket, int nThreadId)
{
	WriteLogByLevel(1, "[>%d<]<<<<<<<<<<<GPS, ProcGPSLogout.\n", rPacket.nSockFd);
	string sSimPhone = g_rTermList.GetSimPhoneBySockFd(rPacket.nSockFd);

	char szBuf[1024] = {0};
	int nRet;

	sprintf(szBuf, "update T_LOGIN_RECORD set logout_time = now() where sim_phone = '%s' and logout_time is null;", sSimPhone.c_str());
	string sSQL = szBuf;
	return g_rDatabaseList[nThreadId].ExecSQL(sSQL);
}

int ProcGPSLogin(TPacket &rPacket, int nThreadId)
{
	WriteLogByLevel(1, "[>%d<]<<<<<<<<<<<GPS, ProcGPSLogin.\n", rPacket.nSockFd);
	int nTermId;
	string sTermId = GetMemNumString(rPacket.pBuffer + 1, 4);
	string sSimPhone = GetMemNumString(rPacket.pBuffer + 5, 6);
	string sIP = GetMemNumString(rPacket.pBuffer + 11, 1) + ".";
	sIP = sIP + GetMemNumString(rPacket.pBuffer + 12, 1);
	sIP = sIP + ".";
	sIP = sIP + GetMemNumString(rPacket.pBuffer + 13, 1);
	sIP = sIP + ".";
	sIP = sIP + GetMemNumString(rPacket.pBuffer + 14, 1);
	unsigned int unReceptionQuality = GetMemInt(rPacket.pBuffer + 15, 1);
	string sDriverId = GetMemNumString(rPacket.pBuffer + 16, 4);
	unsigned int unLedVersion = GetMemInt(rPacket.pBuffer + 20, 4);

	WriteLogByLevel(1, "[>%d<]ProcGPSLogin, sTermId:%s, sSimPhone:%s, sIP:%s, unReceptionQuality:%u, sDriverId:%s, unLedVersion:%u\n", \
					rPacket.nSockFd, sTermId.c_str(), sSimPhone.c_str(), sIP.c_str(), unReceptionQuality, sDriverId.c_str(), unLedVersion);

	int nOriSockFd = g_rTermList.GetSockFdBySimPhone(sSimPhone);
	if (nOriSockFd != -1)	//ԭ���Ѿ�����
	{
		g_rTermList.Erase(nOriSockFd);
	}
	g_rTermList.Insert(rPacket.nSockFd, sSimPhone);

	char szBuf[1024] = {0};
	int nRet;
	sprintf(szBuf, "select count(*) from T_LOGIN_RECORD where sim_phone = '%s' and logout_time is null or login_time < now();", sSimPhone.c_str());
	string sSQL = szBuf;
	int nFieldCount = g_rDatabaseList[nThreadId].QuerySQL(sSQL);
	if (nFieldCount <= 0)
	{
		WriteLogByLevel(1, "QuerySQL:%s error! return:%d\n", szBuf, nFieldCount);
		printf("QuerySQL:%s error! return:%d\n", szBuf, nFieldCount);
		return -1;
	}
	if (g_rDatabaseList[nThreadId].FetchRow()!= NULL)
	{
		g_rDatabaseList[nThreadId].ReleaseQueryRet();
		//�Ѿ������
		sprintf(szBuf, "update T_LOGIN_RECORD set logout_time = now() where sim_phone = '%s' and logout_time is null;", sSimPhone.c_str());
		string sSQL = szBuf;
		g_rDatabaseList[nThreadId].ExecSQL(sSQL);
	}

	sprintf(szBuf, "insert into T_LOGIN_RECORD(sim_phone, term_id, ip, reception, driver_id, led_version, login_time) values ('%s', '%s', '%s', %u, '%s', %u, now());", \
			sSimPhone.c_str(), sTermId.c_str(), sIP.c_str(), unReceptionQuality, sDriverId.c_str(), unLedVersion);
	sSQL = szBuf;

	
	//����RFID��ַΪ0x00
	T485Data rSendReq;
	rSendReq.sCammandID = "0";
	rSendReq.sUserID = "0";
	rSendReq.sData = RFIDSetReaderAdr(0x00);
	SendToGPS485Data(rSendReq, rPacket.nSockFd);
	sleep(1);
	//����RFIDΪӦ��ģʽ
	rSendReq.sCammandID = "1";
	rSendReq.sUserID = "0";
	rSendReq.sData = RFIDSetAnswerMode();
	SendToGPS485Data(rSendReq, rPacket.nSockFd);
	sleep(1);
	//����RFID������
	rSendReq.sCammandID = "2";
	rSendReq.sUserID = "0";
	rSendReq.sData = RFIDSetReaderBaudRate(0);	//9600
	SendToGPS485Data(rSendReq, rPacket.nSockFd);
	

	return g_rDatabaseList[nThreadId].ExecSQL(sSQL);
}

int ProcGPSHeart(TPacket &rPacket, int nThreadId)	//����������
{
	WriteLogByLevel(1, "[>%d<]<<<<<<<<<<<GPS, ProcGPSHeart.\n", rPacket.nSockFd);
	g_rTermList.SetKeepAliveBySockFd(rPacket.nSockFd);
	char szBuffer[16] = {0};
	szBuffer[0] = 0x02;
	szBuffer[1] = 0x01;
	szBuffer[2] = 0x00;
	szBuffer[3] = 0x78;
	szBuffer[4] = 0x03;
	SendToGPSHeart(rPacket.nSockFd);
	return 0;
}

int SolvedAlert(int nThreadId, unsigned int unNewCondition, string sSimPhone, string sDateTime)
{
	string sSQL;
	char szBuf[1024] = {0};
	//�жϸ澯�Ƿ�ָ�
	sprintf(szBuf, "select car_condition from T_GPS_TRACE where sim_phone = '%s' and car_condition <> 0 order by gps_time desc;", sSimPhone.c_str());
	sSQL = szBuf;
	int nFieldCount;
	nFieldCount = g_rDatabaseList[nThreadId].QuerySQL(sSQL);
	if (nFieldCount <= 0)
	{
		return 0;
	}
	printf("QuerySQL: %s, nFieldCount = %d\n", szBuf, nFieldCount);
	char **ppRow = g_rDatabaseList[nThreadId].FetchRow();
	if (ppRow == NULL)
	{
		printf("ppRow == NULL \n");
		return 0;
	}

	unsigned int unLastCondition = atol(ppRow[0] ? ppRow[0] : "0");
	g_rDatabaseList[nThreadId].ReleaseQueryRet();
	
	unsigned int unSolvedFlag = GetSolvedAlert(unLastCondition, unNewCondition);
	if (unSolvedFlag == 0)
	{
		return 0;
	}
	for (int i = 0; i < 32; ++i)
	{
		if (g_rConfig.rAlertIdMaskSet.find(i + 1) != g_rConfig.rAlertIdMaskSet.end())
		{
			//���˲��澯��alert_id
			continue;
		}
		if ((unSolvedFlag & (0x01 << i)) ? 1 : 0)
		{
			if (i + 1 == 5)	//�����澯����ҵ��ģ��ָ�
			{
				continue;
			}
			//���ֶ��и澯
			sprintf(szBuf, "update T_ALERT set release_time = str_to_date('%s', '%%Y%%m%%d%%H%%i%%s') where sim_phone = '%s' and alert_id = %d and release_time is null;", \
					sDateTime.c_str(), sSimPhone.c_str(), i + 1);
			sSQL = szBuf;
			int nRet = g_rDatabaseList[nThreadId].ExecSQL(sSQL);
			WriteLogByLevel(1, "[%s]�������������alert_id: %d, alert_time: %s, �����:%d\n", \
						sSimPhone.c_str(), i + 1, sDateTime.c_str(), nRet);
		}
	}
	return 0;
}


int ProcGPSTemperatureInfo(TPacket &rPacket, int nThreadId)
{
	WriteLogByLevel(1, "[>%d<]<<<<<<<<<<<GPS, ProcGPSTemperatureInfo.\n", rPacket.nSockFd);
	char szBuf[1024] = {0};
	string sSimPhone = g_rTermList.GetSimPhoneBySockFd(rPacket.nSockFd);

	//��ʪ���Ǹ�λ��ǰ����λ�ں�Ҫת�ɵ�λ��ǰ��ʹ��GetMemIntȡֵ
	char szIntBuf[8] = {0};
	szIntBuf[0] = rPacket.pBuffer[2];
	szIntBuf[1] = rPacket.pBuffer[1];
	unsigned int unTemperature = GetMemInt(szIntBuf, 2);
	szIntBuf[0] = rPacket.pBuffer[4];
	szIntBuf[1] = rPacket.pBuffer[3];
	unsigned int unHumidity = GetMemInt(szIntBuf, 2);
	//unsigned int unHumidity = GetMemInt(rPacket.pBuffer + 3, 2);

	double dTemperature = unTemperature / 10.0;
	double dHumidity = unHumidity / 10.0;
	printf("ProcGPSTemperatureInfo: t:%u = %lf, h:%u = %lf\n", unTemperature, dTemperature, unHumidity, dHumidity);

	sprintf(szBuf, "INSERT INTO T_TEMPERATURE (sim_phone, gps_time, temperature, humidity) VALUES ('%s', now(), '%lf', '%lf');", \
			sSimPhone.c_str(), dTemperature, dHumidity);
	string sSQL = szBuf;
	return g_rDatabaseList[nThreadId].ExecSQL(sSQL);
}

int ProcGPS485Data(TPacket &rPacket, int nThreadId)
{
	WriteLogByLevel(1, "[>%d<]<<<<<<<<<<<GPS, ProcGPS485Data.\n", rPacket.nSockFd);

	
	printf("===========================RFID Data==============================\n");
	for (int i = 0; i < rPacket.unLen; ++i)
	{
		printf("%02x ", *(unsigned char *)(rPacket.pBuffer + i));
		if (i % 10 == 9)
			printf("\n");
	}
	printf("\n");
	printf("===========================RFID End ==============================\n");

	if (rPacket.unLen < 6)
	{
		WriteLogByLevel(1, "%s\n", "In ProcGPS485Data, but data len is too short.");
		return -1;
	}
	string sSimPhone = g_rTermList.GetSimPhoneBySockFd(rPacket.nSockFd);
	string sRespPkg(rPacket.pBuffer + 1, rPacket.unLen - 1);
	return DecodeRespPacket(sRespPkg, nThreadId, sSimPhone);
}


int ProcGPSRealInfo(TPacket &rPacket, int nThreadId)
{
	WriteLogByLevel(1, "[>%d<]<<<<<<<<<<<GPS, ProcGPSRealInfo.\n", rPacket.nSockFd);
	string sSimPhone = g_rTermList.GetSimPhoneBySockFd(rPacket.nSockFd);

	char szBuf[1024] = {0};
	int nGPSAvailable = rPacket.pBuffer[1] == 0x41 ? 1 : 0;
	int nLatFlag = rPacket.pBuffer[2] == 0x4E ? 1 : 2;	// 1:��γ 2: ��γ
	unsigned int unLat1 = GetMemInt(rPacket.pBuffer + 3, 1);	//γ��_ ��
	unsigned int unLat2 = GetMemInt(rPacket.pBuffer + 4, 1);	//γ��_ ��
	unsigned int unLat3 = GetMemInt(rPacket.pBuffer + 5, 1);	//γ��_ �ֵ�С������
	unsigned int unLat4 = GetMemInt(rPacket.pBuffer + 6, 1);	//γ��_ �ֵ�С������
	sprintf(szBuf, "%u.%02u%02u", unLat2, unLat3, unLat4);
	printf("γ��: %u��%s��\n", unLat1, szBuf);
	double dLat = atof(szBuf) / 60 + unLat1;
	int nLongFlag = rPacket.pBuffer[7] == 0x45 ? 1 : 2;	// 1:����2: ����
	unsigned int unLong1 = GetMemInt(rPacket.pBuffer + 8, 1);	//����_ ��
	unsigned int unLong2 = GetMemInt(rPacket.pBuffer + 9, 1);	//����_ ��
	unsigned int unLong3 = GetMemInt(rPacket.pBuffer + 10, 1);	//����_ �ֵ�С������
	unsigned int unLong4 = GetMemInt(rPacket.pBuffer + 11, 1);	//����_ �ֵ�С������
	sprintf(szBuf, "%u.%02u%02u", unLong2, unLong3, unLong4);
	double dLong = atof(szBuf) / 60 + unLong1;
	if (unLat1 == 0 || unLong1 == 0)
	{
		return 0;
	}
	unsigned int unSpeed = GetMemInt(rPacket.pBuffer + 12, 1);	//�ٶ�
	unsigned int unAspect1 = GetMemInt(rPacket.pBuffer + 13, 1);	//����
	unsigned int unAspect2 = GetMemInt(rPacket.pBuffer + 14, 1);	//����

	/********��˹Э����£�ɾ��"����2"������"�ۼ����0"**********
	unsigned int unAspect3 = GetMemInt(rPacket.pBuffer + 15, 1);	//����С��
	sprintf(szBuf, "%u%02u.%02u", unAspect1, unAspect2, unAspect3);
	double dAspect = atof(szBuf);
	
	unsigned int unMileage1 = GetMemInt(rPacket.pBuffer + 16, 2);	//�ۼ����
	unsigned int unMileage2 = GetMemInt(rPacket.pBuffer + 18, 1);	//�ۼ����С��
	sprintf(szBuf, "%u.%02u", unMileage1, unMileage2);
	double dMileage = atof(szBuf);
	*******************************************************************************/
	sprintf(szBuf, "%u%02u.0", unAspect1, unAspect2);
	double dAspect = atof(szBuf);
	//�ۼ����=A*25600 + B*100 + C + D/100 
	unsigned int unMileageA = GetMemInt(rPacket.pBuffer + 15, 1);	//�ۼ����
	unsigned int unMileageB = GetMemInt(rPacket.pBuffer + 16, 1);	//�ۼ����
	unsigned int unMileageC = GetMemInt(rPacket.pBuffer + 17, 1);	//�ۼ����
	unsigned int unMileageD = GetMemInt(rPacket.pBuffer + 18, 1);	//�ۼ����
	unsigned int unMileageInt = unMileageA * 25600 + unMileageB * 100 + unMileageC;
	double dMileage = unMileageD / 100.0 + unMileageInt;
	
	string sTime = "20";
	sTime = sTime + GetMemHexString(rPacket.pBuffer + 19, 6);		//ʱ��YYYYMMDDHH24MISS  '%Y%m%d%H%i%s'
	unsigned int unCondition = GetMemInt(rPacket.pBuffer + 25, 4);	//������־

	//�����ѻָ��ĸ澯
	SolvedAlert(nThreadId, unCondition, sSimPhone, sTime);
	char szId[32] = {0};
	char szSeed [16] = {0};
	sprintf(szSeed, "%u%u", unLat3, unLong3);
	srand(atol(szSeed));
	sprintf(szId, "%s%u%02d", sSimPhone.c_str(), time(NULL), rand() % 100);
	sprintf(szBuf, "insert into T_GPS_TRACE (id, sim_phone, gps_time, lat_flag, lat, long_flag, long_value, speed, aspect, mileage, car_condition) values('%s', '%s', str_to_date('%s', '%%Y%%m%%d%%H%%i%%s'), %u, %.5lf, %u, %.5lf, %u, %.2lf, %.2lf, %u)", \
			szId, sSimPhone.c_str(), sTime.c_str(), nLatFlag, dLat, nLongFlag, dLong, unSpeed, dAspect, dMileage, unCondition);
	string sSQL = szBuf;
	return g_rDatabaseList[nThreadId].ExecSQL(sSQL);
}

//�ж���Щ�澯�Ѿ��ָ�
unsigned int GetSolvedAlert(unsigned int unAlertCondition, unsigned int unNewCondition)
{
	unsigned int unRet = unAlertCondition ^ unNewCondition;	//���״̬�иı��λ�����ó�1
	return (unRet & unAlertCondition);	//���˵�unNewCondition����¸澯
}

//�ж���Щ�������澯
unsigned int GetNewAlert(unsigned int unAlertCondition, unsigned int unNewCondition)
{
	unsigned int unRet = unAlertCondition ^ unNewCondition;	//���״̬�иı��λ�����ó�1
	return (unRet & unNewCondition);	//���˵�unAlertCondition��ľɸ澯
}

int SendAlertSMS(int nThreadId, string &sSimPhone, int nAlertId)
{
	char szBuf[1024] = {0};
	string sSQL;
	sprintf(szBuf, "select t1.alert_info from T_ALERT_INFO t1 where t1.alert_id = %d and isSendMsg = 1;", nAlertId);

	sSQL = szBuf;
	int nFieldCount;
	nFieldCount = g_rDatabaseList[nThreadId].QuerySQL(sSQL);
	if (nFieldCount <= 0)
	{
		return 0;
	}
	printf("QuerySQL: %s, nFieldCount = %d\n", szBuf, nFieldCount);
	char **ppRow = g_rDatabaseList[nThreadId].FetchRow();
	if (ppRow == NULL)
	{
		printf("ppRow == NULL \n");
		return 0;
	}
	string sAlertInfo = ppRow[0];
	//unsigned int unLastCondition = atol(ppRow[0] ? ppRow[0] : "0");
	g_rDatabaseList[nThreadId].ReleaseQueryRet();

	sprintf(szBuf, "select t.car_no from CAR_INFO t where t.sim_phone = '%s';", sSimPhone.c_str());
	sSQL = szBuf;
	nFieldCount = g_rDatabaseList[nThreadId].QuerySQL(sSQL);
	if (nFieldCount <= 0)
	{
		return 0;
	}
	printf("QuerySQL: %s, nFieldCount = %d\n", szBuf, nFieldCount);
	ppRow = g_rDatabaseList[nThreadId].FetchRow();
	if (ppRow == NULL)
	{
		printf("ppRow == NULL \n");
		return 0;
	}
	string sCarNo = ppRow[0];
	g_rDatabaseList[nThreadId].ReleaseQueryRet();
	
	char szNowTime[64] = {0};
	FmtNowTime("��ע��: %Y��%m��%d��%H��%M��, ", szNowTime, 64);
	char szUtf[256] = {0};
	g2u(szNowTime, strlen(szNowTime), szUtf, sizeof(szUtf));
	string sSMSContent = szUtf;
	sSMSContent.append(sCarNo);
	printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
	for (int i = 0; i < sCarNo.length(); ++i)
	{
		printf("%02x ", *(unsigned char *)(sCarNo.data() + i));
	}
	printf("\n");
	//sSMSContent.append("����");
	sSMSContent.append(sAlertInfo);

	char szPacket[1024] = {0};
	sprintf(szPacket, "<RequestMessage>"
		"<PostType>ENTERPRISE_SMS_SEND</PostType>"
		"<PostData>"
		"<SendBody>"
			"<Message>"
				"<Basic>"
					"<ID></ID>"
					"<Title>Alert</Title>"
					"<Content>%s</Content>"
					"<Send>059187606091</Send>"
					"<TelnoDisplay></TelnoDisplay>"
		"<MsgSrctype></MsgSrctype>"
		"<Incept>", sSMSContent.c_str());

	sSQL = "select t.alert_man_phone from T_ALERT_MAN t where t.isCanceled = 0;";
	nFieldCount = g_rDatabaseList[nThreadId].QuerySQL(sSQL);
	if (nFieldCount <= 0)
	{
		return 0;
	}
	printf("QuerySQL: %s, nFieldCount = %d\n", sSQL.c_str(), nFieldCount);

	while (1)
	{
		ppRow = g_rDatabaseList[nThreadId].FetchRow();
		if (ppRow == NULL)
			break;
		
		if (ppRow[0] == NULL)
		{
			break;
		}
		strcat(szPacket, "<MobileNum>");
		strcat(szPacket, ppRow[0]);
		strcat(szPacket, "</MobileNum>");
	}
	strcat(szPacket, "</Incept>"
		"</Basic>"
		"<Expand>"
			"<Reserve></Reserve>"
			"<Schedule></Schedule>"
		"</Expand>"
		"</Message>"
		"</SendBody>"
		"</PostData>"
		"</RequestMessage>");
	g_rDatabaseList[nThreadId].ReleaseQueryRet();

	WriteLogByLevel(1, "SMS Packet: %s\n", szPacket);

	int nCryptMode = CBC;
	int nPadMode = PAD_PKCS_7;
	char cvec[10];
	memset(cvec, 0x00, sizeof(cvec));
	cvec[0]=0x01;
	cvec[1]=0x02;
	cvec[2]=0x03;
	cvec[3]=0x04;
	cvec[4]=0x05;
	cvec[5]=0x06;
	cvec[6]=0x07;
	cvec[7]=0x08;

	char sz24BitKey[256] = {0};
	memset(sz24BitKey, 0x00, sizeof(sz24BitKey));
	if (!CovertKey((char *)g_rConfig.s3desKey.c_str(), sz24BitKey))	//48�ֽ���Կת��24λ��Կ
	{
		WriteLogByLevel(1, "%s\n", "CovertKey error!");
		return -1;
	}
	WriteLogByLevel(1, "CovertKey, OUT:%s, %d\n", sz24BitKey, strlen(sz24BitKey));
	printf("\n\n\n\n key: %s\n", sz24BitKey);
		for (int i = 0; i < strlen(sz24BitKey); ++i)
		{
			printf("%02x ", *(unsigned char *)(sz24BitKey + i));
		}
		printf("\n\n\n\n");
		
	char szSrc[1024] = {0};
	memset(szSrc, 0x00, sizeof(szSrc));
	int nSrcLen = 0;
	RunPad(nPadMode, szPacket, strlen(szPacket), szSrc, &nSrcLen);	//���Ĳ�λ
	printf("\n\n\n\n src: %s, len=%d\n", szSrc, nSrcLen);
	for (int i = 0; i < nSrcLen; ++i)
	{
		printf("%02x ", *(unsigned char *)(szSrc + i));
	}
	printf("\n\n\n\n");

	char szOut[1024] = {0};
	memset(szOut, 0x00, sizeof(szOut));
	if (!Run3Des(ENCRYPT, nCryptMode, szSrc, nSrcLen, sz24BitKey, strlen(sz24BitKey), szOut, sizeof(szOut), cvec))
	{
		WriteLogByLevel(1, "%s\n", "Run3Des error!");
		return -3;
	}
	printf("\n\n\n\n len = %d out: %s\n", nSrcLen, szOut);
	for (int i = 0; i < nSrcLen; ++i)
	{
		printf("%02x ", *(unsigned char *)(szOut + i));
	}
	printf("\n\n\n\n");

	char* pStr = NULL;
	pStr = Base64Encode(szOut, nSrcLen);
	if (pStr == NULL)
	{
		WriteLogByLevel(1, "%s\n", "Base64Encode error!");
		return -4;
	}
	WriteLogByLevel(1, "Base64Encode:%s \n", pStr);
	CallSendMsg(pStr);
	free(pStr);
	return 0;

}

int ProcAlertCondition(int nThreadId, unsigned int unCondition, string sSimPhone, string sDateTime)
{
	char szBuf[1024] = {0};
	string sSQL;
	int nRet = 0;
	for (int i = 0; i < 32; ++i)
	{
		if (g_rConfig.rAlertIdMaskSet.find(i + 1) != g_rConfig.rAlertIdMaskSet.end())
		{
			//���˲��澯��alert_id
			continue;
		}
		if ((unCondition & (0x00000001 << i)) ? 1 : 0)
		{
			//���ֶ��и澯
			sprintf(szBuf, "insert into T_ALERT (sim_phone, alert_id, alert_time) values('%s', %d, str_to_date('%s', '%%Y%%m%%d%%H%%i%%s'));", \
					sSimPhone.c_str(), i + 1, sDateTime.c_str());
			sSQL = szBuf;
			nRet = g_rDatabaseList[nThreadId].ExecSQL(sSQL);
			WriteLogByLevel(1, "[%s]����������alert_id: %d, alert_time: %s, �����:%d\n", \
						sSimPhone.c_str(), i + 1, sDateTime.c_str(), nRet);

			SendAlertSMS(nThreadId, sSimPhone, i+1);
			
			//����ǰ����������ظ��Ѿ�����
			if (i + 1 == 5)
			{
				TReleaseAlertReq req;
				SendToGPSReleaseAlert(req, g_rTermList.GetSockFdBySimPhone(sSimPhone));
			}
		}
	}
	return 0;
}

int ProcGPSAlert(TPacket &rPacket, int nThreadId)
{
	WriteLogByLevel(1, "[>%d<]========ProcGPSAlert.\n", rPacket.nSockFd);
	char szBuf[1024] = {0};
	int nGPSAvailable = rPacket.pBuffer[1] == 0x41 ? 1 : 0;
	int nLatFlag = rPacket.pBuffer[2] == 0x4E ? 1 : 2;	// 1:��γ 2: ��γ
	unsigned int unLat1 = GetMemInt(rPacket.pBuffer + 3, 1);	//γ��_ ��
	unsigned int unLat2 = GetMemInt(rPacket.pBuffer + 4, 1);	//γ��_ ��
	unsigned int unLat3 = GetMemInt(rPacket.pBuffer + 5, 1);	//γ��_ �ֵ�С������
	unsigned int unLat4 = GetMemInt(rPacket.pBuffer + 6, 1);	//γ��_ �ֵ�С������
	sprintf(szBuf, "%u.%02u%02u", unLat2, unLat3, unLat4);
	double dLat = atof(szBuf) / 60 + unLat1;
	int nLongFlag = rPacket.pBuffer[7] == 0x45 ? 1 : 2;	// 1:����2: ����
	unsigned int unLong1 = GetMemInt(rPacket.pBuffer + 8, 1);	//����_ ��
	unsigned int unLong2 = GetMemInt(rPacket.pBuffer + 9, 1);	//����_ ��
	unsigned int unLong3 = GetMemInt(rPacket.pBuffer + 10, 1);	//����_ �ֵ�С������
	unsigned int unLong4 = GetMemInt(rPacket.pBuffer + 11, 1);	//����_ �ֵ�С������
	sprintf(szBuf, "%u.%02u%02u", unLong2, unLong3, unLong4);
	double dLong = atof(szBuf) / 60 + unLong1;
	unsigned int unSpeed = GetMemInt(rPacket.pBuffer + 12, 1);	//�ٶ�
	unsigned int unAspect1 = GetMemInt(rPacket.pBuffer + 13, 1);	//����
	unsigned int unAspect2 = GetMemInt(rPacket.pBuffer + 14, 1);	//����
	unsigned int unAspect3 = GetMemInt(rPacket.pBuffer + 15, 1);	//����С��
	sprintf(szBuf, "%u%02u.%02u", unAspect1, unAspect2, unAspect3);
	double dAspect = atof(szBuf);
	unsigned int unMileage1 = GetMemInt(rPacket.pBuffer + 16, 2);	//�ۼ����
	unsigned int unMileage2 = GetMemInt(rPacket.pBuffer + 18, 1);	//�ۼ����С��
	sprintf(szBuf, "%u.%02u", unMileage1, unMileage2);
	double dMileage = atof(szBuf);
	string sTime = "20";
	sTime = sTime + GetMemHexString(rPacket.pBuffer + 19, 6);		//ʱ��YYYYMMDDHH24MISS  '%Y%m%d%H%i%s'
	unsigned int unCondition = GetMemInt(rPacket.pBuffer + 25, 4);	//������־

	string sSimPhone = g_rTermList.GetSimPhoneBySockFd(rPacket.nSockFd);
	ProcAlertCondition(nThreadId, unCondition, sSimPhone, sTime);

	char szId[32] = {0};
	srand(time(NULL));
	sprintf(szId, "%s%u%02d", sSimPhone.c_str(), time(NULL), rand() % 100);
	sprintf(szBuf, "insert into T_GPS_TRACE (id, sim_phone, gps_time, lat_flag, lat, long_flag, long_value, speed, aspect, mileage, car_condition) values('%s', '%s', str_to_date('%s', '%%Y%%m%%d%%H%%i%%s'), %u, %.5lf, %u, %.5lf, %u, %.2lf, %.2lf, %u)", \
			szId, sSimPhone.c_str(), sTime.c_str(), nLatFlag, dLat, nLongFlag, dLong, unSpeed, dAspect, dMileage, unCondition);
	string sSQL = szBuf;
	return g_rDatabaseList[nThreadId].ExecSQL(sSQL);
}

//�ڽ��յ�ESC(0x1b)�ַ�������ȥ�����ֽڣ����������ֽ���ESC�������
int DecodeESC(string &sIn)
{
	int nPos = 0;
	int nFrom = 0;
	char c;
	while ((nPos = sIn.find((char)0x1b, nFrom)) != string::npos)
	{
		sIn.erase(nPos, 1);
		if (nPos + 1 == sIn.length())
		{
			break;	//�ַ���β
		}
		c = sIn.at(nPos) ^ 0x1b;
		sIn.replace(nPos, 1, 1, c);
		nFrom = nPos + 1;
	}
	return 0;
}

int ProcGPSImage(TPacket &rPacket, int nThreadId)
{	
	WriteLogByLevel(1, "[>%d<]<<<<<<<<<<<GPS, ProcGPSImage.\n", rPacket.nSockFd);
	char szBuf[1024] = {0};
	string sSimPhone = g_rTermList.GetSimPhoneBySockFd(rPacket.nSockFd);
	string sUserId = GetMemNumString(rPacket.pBuffer + 1, 2);
	string sCommandId = GetMemNumString(rPacket.pBuffer + 3, 2);

	if (rPacket.pBuffer[5] != 0x7e)
	{
		WriteLogByLevel(2, "[>%d<]ProcGPSImage, δ�ҵ�֡ͷ��־0x7E.\n", rPacket.nSockFd);
		return -1;
	}

	string sImageData(rPacket.pBuffer + 6, rPacket.unLen - 6);
	int nEndPos = sImageData.find((char)0x7e);
	if (nEndPos == string::npos)
	{
		WriteLogByLevel(2, "[>%d<]ProcGPSImage, δ�ҵ�֡β��־0x7E.\n", rPacket.nSockFd);
		return -1;
	}
	
	char cCamNo = sImageData[0] - 0x10;	//��������к�0--3
	sImageData = sImageData.substr(3, nEndPos - 3 - 2); //��ȥ֡ͷβ��0x7e��־���֡���Լ�У��λ
	
	DecodeESC(sImageData);		//����ת���
	
	string sEndFlag = "";
	sEndFlag.append(1, (char)0xFF);
	sEndFlag.append(1, (char)0xD9);
	nEndPos = sImageData.find(sEndFlag);
	if (nEndPos != string::npos)	//��JPEG��������ɾ���������������, ����������
	{
		sImageData = sImageData.substr(0, nEndPos + 2);
	}
	
	unsigned char c0 = sImageData.at(0);
	unsigned char c1 = sImageData.at(1);
	if ((c0 == 0xFF) && (c1 == 0xD8))	//JPEGͷ��ʶ
	{
		g_rImageBuf.SetImageBufBySockFd(rPacket.nSockFd, sImageData);
	}
	else
	{
		string sBuf = g_rImageBuf.GetImageBufBySockFd(rPacket.nSockFd);
		sBuf.append(sImageData);
		g_rImageBuf.SetImageBufBySockFd(rPacket.nSockFd, sBuf);
	}
	
	if (nEndPos != string::npos)	//��JPEG������
	{
		string sImageBuf = g_rImageBuf.GetImageBufBySockFd(rPacket.nSockFd);
		g_rImageBuf.Erase(rPacket.nSockFd);

		//ͼƬ�������������
		char szNowTime[16] = {0};
		char szCommand[512] = {0};
		FmtNowTime("%Y%m%d%H%M%S", szNowTime, 15);
		char szTmpFileName[256] = {0};
		sprintf(szTmpFileName, "%s/%s/%d/%s.tmp", g_rConfig.sPhotoPath.c_str(), sSimPhone.c_str(), cCamNo, szNowTime);
		char szFileName[256] = {0};
		sprintf(szFileName, "%s/%s/%d/%s.jpg", g_rConfig.sPhotoPath.c_str(), sSimPhone.c_str(), cCamNo, szNowTime);

		TSession *pSession = g_rSessionMap[nThreadId].Find(atol(sUserId.c_str()));

		if (CheckDirExist(szTmpFileName) != 1)
		{
			if (pSession)
			{
				pSession->m_rResult.nResult = -1;
				pSession->m_nProcFlag = 2;
			}
			WriteLogByLevel(2, "[>%d<]Create directory %s error!\n", rPacket.nSockFd, szTmpFileName);
			return -1;
		}
		
		FILE *fp = fopen(szTmpFileName, "wb");
		if (fp == NULL)
		{
			if (pSession)
			{
				pSession->m_rResult.nResult = -1;
				pSession->m_nProcFlag = 2;
			}
			WriteLogByLevel(2, "[>%d<]In ProcGPSImage, create jpeg file error! file:%s\n", rPacket.nSockFd, szTmpFileName);
			return -1;
		}
		if (fwrite(sImageBuf.data(), sImageBuf.length(), 1, fp) <= 0)
		{
			WriteLogByLevel(2, "[>%d<]In ProcGPSImage, write jpeg data error! file:%s\n", rPacket.nSockFd, szTmpFileName);
			fclose(fp);
			sprintf(szCommand, "rm %s", szTmpFileName);
			system(szCommand);
			if (pSession)
			{
				pSession->m_rResult.nResult = -1;
				pSession->m_nProcFlag = 2;
			}
			return -1;
		}
		fclose(fp);
		sprintf(szCommand, "mv %s %s", szTmpFileName, szFileName);
		system(szCommand);

		char szWebName[256] = {0};
		sprintf(szWebName, "%s/%d/%s.jpg", sSimPhone.c_str(), cCamNo, szNowTime);

		WriteLogByLevel(1, "[>%d<]������Ƭ�ļ��ɹ�: %s\n", rPacket.nSockFd, szFileName);
		char szSQL[1024] = {0};
		sprintf(szSQL, "insert into PHOTO(path, sim_phone, create_time, name) values('%s', '%s', str_to_date('%s', '%%Y%%m%%d%%H%%i%%s'), '%s.jpg');", \
				szWebName, sSimPhone.c_str(), szNowTime, szNowTime);
		string sSQL = szSQL;
		g_rDatabaseList[nThreadId].ExecSQL(sSQL);

		if (pSession)
		{
			pSession->m_rTakePhotoResp.sPhotoName = szWebName;
			pSession->m_rResult.nResult = 0;
			pSession->m_nProcFlag = 2;
		}
		return 0;
	}
}

//�ն�����Ӧ��
int ProcGPSConfigResp(TPacket &rPacket, int nThreadId)
{
	WriteLogByLevel(1, "[>%d<]<<<<<<<<<<<GPS, ProcGPSConfigResp.\n", rPacket.nSockFd);
	char szBuf[1024] = {0};
	string sSimPhone = g_rTermList.GetSimPhoneBySockFd(rPacket.nSockFd);

	string sUserId = GetMemNumString(rPacket.pBuffer + 1, 2);
	string sCommandId = GetMemNumString(rPacket.pBuffer + 3, 2);
	string sResp(rPacket.pBuffer + 5, rPacket.unLen - 5);

	//ƥ�����󣬷���WEBSERVICE
	//ƥ�����󣬷���WEBSERVICE
	TSession *pSession = g_rSessionMap[nThreadId].Find(atol(sUserId.c_str()));
	if (pSession)
	{
		pSession->m_rResult.nResult = 0;
		pSession->m_nProcFlag = 1;
	}
	return 0;
}

//�ն�����ִ�гɹ�Ӧ��
int ProcGPSCmdSuccessResp(TPacket &rPacket, int nThreadId)
{
	WriteLogByLevel(1, "[>%d<]<<<<<<<<<<<GPS, ProcGPSCommandSuccessResp.\n", rPacket.nSockFd);
	char szBuf[1024] = {0};
	string sSimPhone = g_rTermList.GetSimPhoneBySockFd(rPacket.nSockFd);

	string sUserId = GetMemNumString(rPacket.pBuffer + 1, 2);
	string sCommandId = GetMemNumString(rPacket.pBuffer + 3, 2);

	//ƥ�����󣬷���WEBSERVICE
	TSession *pSession = g_rSessionMap[nThreadId].Find(atol(sUserId.c_str()));
	if (pSession)
	{
		pSession->m_rResult.nResult = 0;
		pSession->m_nProcFlag = 1;
	}
	return 0;
}

//�ն�����ִ��ʧ��Ӧ��
int ProcGPSCmdFailResp(TPacket &rPacket, int nThreadId)
{
	WriteLogByLevel(1, "[>%d<]<<<<<<<<<<<GPS, ProcGPSCmdFailResp.\n", rPacket.nSockFd);
	char szBuf[1024] = {0};
	string sSimPhone = g_rTermList.GetSimPhoneBySockFd(rPacket.nSockFd);

	string sUserId = GetMemNumString(rPacket.pBuffer + 1, 2);
	string sCommandId = GetMemNumString(rPacket.pBuffer + 3, 2);

	//ƥ�����󣬷���WEBSERVICE
	TSession *pSession = g_rSessionMap[nThreadId].Find(atol(sUserId.c_str()));
	if (pSession)
	{
		pSession->m_rResult.nResult = -1;
		pSession->m_nProcFlag = 2;
	}
	return 0;
}

//�ն˰汾��ѯӦ��
int ProcGPSQueryVerResp(TPacket &rPacket, int nThreadId)
{
	WriteLogByLevel(1, "[>%d<]<<<<<<<<<<<GPS, ProcGPSQueryVerResp.\n", rPacket.nSockFd);
	char szBuf[1024] = {0};
	string sSimPhone = g_rTermList.GetSimPhoneBySockFd(rPacket.nSockFd);

	string sUserId = GetMemNumString(rPacket.pBuffer + 1, 2);
	string sCommandId = GetMemNumString(rPacket.pBuffer + 3, 2);

	int nLen = 5;
	char *pEnd = strchr(rPacket.pBuffer + 5, 0x03);
	if (pEnd == NULL)
	{
		WriteLogByLevel(1, "[>%d<]ProcGPSQueryVerResp, end char is not found\n", rPacket.nSockFd);
		return -1;
	}
	nLen = pEnd - rPacket.pBuffer;
	char szVersion[256];
	memset(szVersion, 0x00, 256);
	memcpy(szVersion, rPacket.pBuffer + 5, nLen - 5);
	WriteLogByLevel(1, "[>%d<]ProcGPSQueryVerResp, version:%s\n", rPacket.nSockFd, szVersion);
	//ƥ�����󣬷���WEBSERVICE
	TSession *pSession = g_rSessionMap[nThreadId].Find(atol(sUserId.c_str()));
	if (pSession)
	{
		pSession->m_rQueryVerResp.sVersion = szVersion;
		pSession->m_nProcFlag = 2;
	}
	return 0;
}


string GenerateId()
{
	time_t nCurTime;
	struct timeval rTimeval;
	struct timezone rTimeZone;
	time(&nCurTime);
	gettimeofday(&rTimeval,&rTimeZone);
	char szBuf[32];
	memset(szBuf, 0x00, sizeof(szBuf));
	sprintf(szBuf, "%u%06d", nCurTime, rTimeval.tv_usec);
	return string(szBuf);
}

//�ն���ͨ����
int ProcGPSCommonData(TPacket &rPacket, int nThreadId)
{
	WriteLogByLevel(1, "[>%d<]<<<<<<<<<<<GPS, ProcGPSCommonData.\n", rPacket.nSockFd);

	char szBuf[1024] = {0};
	string sSimPhone = g_rTermList.GetSimPhoneBySockFd(rPacket.nSockFd);

	string sData(rPacket.pBuffer + 1, rPacket.unLen - 1);
	printf("==========ProcGPSCommonData Begin==========\n");
	printf("Data:\n");
	for (int i = 0; i < sData.length(); ++i)
	{
		printf("%02x ", *(unsigned char *)(sData.data() + i));
	}
	printf("\n");
	printf("CommonData: %s\n", sData.data());
	printf("==========ProcGPSCommonData End============\n");
	int nRet;
	if (sData.find("��ʼװ��") != string::npos)
	{
		sprintf(szBuf, "update T_SHIP_INFO set end_time = now() where sim_phone = '%s' and end_time is null;", sSimPhone.c_str());
		string sSQL = szBuf;
		g_rDatabaseList[nThreadId].ExecSQL(sSQL);

		string sSerialId = GenerateId();
		g_rShipSerialMap.SetSerial(sSimPhone, sSerialId);
		sprintf(szBuf, "insert into T_SHIP_INFO (serial_id, sim_phone, load_unload_flag, begin_time)" 
				"values ('%s', '%s', 0, now());", \
				sSerialId.c_str(), sSimPhone.c_str());
		sSQL = szBuf;
		nRet = g_rDatabaseList[nThreadId].ExecSQL(sSQL);
		g_rTermList.SetRFIDWorkingFlagBySimPhone(sSimPhone, 1);
		
		SendToGPSScanEPC(rPacket.nSockFd);
		return nRet;
	}
	else if (sData.find("����װ��") != string::npos)
	{
		g_rShipSerialMap.EraseSerial(sSimPhone);
		sprintf(szBuf, "update T_SHIP_INFO set end_time = now() where sim_phone = '%s' and end_time is null;", sSimPhone.c_str());
		string sSQL = szBuf;
		nRet = g_rDatabaseList[nThreadId].ExecSQL(sSQL);
		g_rTermList.SetRFIDWorkingFlagBySimPhone(sSimPhone, 0);
		return nRet;
	}
	else if (sData.find("��ʼж��") != string::npos)
	{
		sprintf(szBuf, "update T_SHIP_INFO set end_time = now() where sim_phone = '%s' and end_time is null;", sSimPhone.c_str());
		string sSQL = szBuf;
		g_rDatabaseList[nThreadId].ExecSQL(sSQL);
		
		string sSerialId = GenerateId();
		g_rShipSerialMap.SetSerial(sSimPhone, sSerialId);
		sprintf(szBuf, "insert into T_SHIP_INFO (serial_id, sim_phone, load_unload_flag, begin_time)" 
				"values ('%s', '%s', 1, now());", \
				sSerialId.c_str(), sSimPhone.c_str());
		sSQL = szBuf;
		nRet = g_rDatabaseList[nThreadId].ExecSQL(sSQL);
		g_rTermList.SetRFIDWorkingFlagBySimPhone(sSimPhone, 1);

		SendToGPSScanEPC(rPacket.nSockFd);
		return nRet;
	}
	else if (sData.find("����ж��") != string::npos)
	{
		g_rShipSerialMap.EraseSerial(sSimPhone);
		sprintf(szBuf, "update T_SHIP_INFO set end_time = now() where sim_phone = '%s' and end_time is null;", sSimPhone.c_str());
		string sSQL = szBuf;
		nRet = g_rDatabaseList[nThreadId].ExecSQL(sSQL);
		g_rTermList.SetRFIDWorkingFlagBySimPhone(sSimPhone, 0);
		return nRet;
	}
	return 0;
}

//¼��ģ�鵱ǰ״̬Ӧ��
int ProcGPSVideoStatResp(TPacket &rPacket, int nThreadId)
{
	WriteLogByLevel(1, "[>%d<]<<<<<<<<<<<GPS, ProcGPSVideoStatResp.\n", rPacket.nSockFd);
	char szBuf[1024] = {0};
	string sSimPhone = g_rTermList.GetSimPhoneBySockFd(rPacket.nSockFd);

	string sUserId = GetMemNumString(rPacket.pBuffer + 1, 2);
	string sCommandId = GetMemNumString(rPacket.pBuffer + 3, 2);

	string sStatBuf = rPacket.pBuffer + 5;
	
	//��Ŀǰ�涼��":"����Ŀ��ֵ���ֿ�����ͬ����Ŀ����;�������������Ӷ���β����ʼ��
	//VIDEO_1~ VIDEO_4----����ͷ1~4��״̬
	//SD1_Total----SD1��������
	//SD1_Free-----SD1����ʣ��ռ�
	//WORK---------¼����״̬
	//����VIDEO_1:OK;VIDEO_2:ERROR;VIDEO_3:ERROR;VIDEO_4:OK;SD1_Total:16000M;SD1_Free :4096M;SD2_Total:0M;SD2_Free :0M;WORK:YES

	//ƥ�����󣬷���WEBSERVICE

	return 0;
}


int ProcGPSICCardData(TPacket &rPacket, int nThreadId)
{
	WriteLogByLevel(1, "[>%d<]<<<<<<<<<<<GPS, ProcGPSICCardData.\n", rPacket.nSockFd);

	char szBuf[1024] = {0};
	string sSimPhone = g_rTermList.GetSimPhoneBySockFd(rPacket.nSockFd);

	string sData(rPacket.pBuffer + 1, rPacket.unLen - 1);
	//string sSerialId = GenerateId();
	sprintf(szBuf, "insert into T_DRIVER_PUNCH (sim_phone, ic_num, punch_time)" 
			"values ('%s', '%s', now());", \
			sSimPhone.c_str(), sData.c_str());
	string sSQL = szBuf;
	int nRet = g_rDatabaseList[nThreadId].ExecSQL(sSQL);
	return nRet;
}

