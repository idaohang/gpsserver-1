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
//nLen: 插入szBuf的字节数
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
//nLen: szBuf的字节数
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
	if (nOriSockFd != -1)	//原来已经登入
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
		//已经登入过
		sprintf(szBuf, "update T_LOGIN_RECORD set logout_time = now() where sim_phone = '%s' and logout_time is null;", sSimPhone.c_str());
		string sSQL = szBuf;
		g_rDatabaseList[nThreadId].ExecSQL(sSQL);
	}

	sprintf(szBuf, "insert into T_LOGIN_RECORD(sim_phone, term_id, ip, reception, driver_id, led_version, login_time) values ('%s', '%s', '%s', %u, '%s', %u, now());", \
			sSimPhone.c_str(), sTermId.c_str(), sIP.c_str(), unReceptionQuality, sDriverId.c_str(), unLedVersion);
	sSQL = szBuf;

	
	//设置RFID地址为0x00
	T485Data rSendReq;
	rSendReq.sCammandID = "0";
	rSendReq.sUserID = "0";
	rSendReq.sData = RFIDSetReaderAdr(0x00);
	SendToGPS485Data(rSendReq, rPacket.nSockFd);
	sleep(1);
	//设置RFID为应答模式
	rSendReq.sCammandID = "1";
	rSendReq.sUserID = "0";
	rSendReq.sData = RFIDSetAnswerMode();
	SendToGPS485Data(rSendReq, rPacket.nSockFd);
	sleep(1);
	//设置RFID波特率
	rSendReq.sCammandID = "2";
	rSendReq.sUserID = "0";
	rSendReq.sData = RFIDSetReaderBaudRate(0);	//9600
	SendToGPS485Data(rSendReq, rPacket.nSockFd);
	

	return g_rDatabaseList[nThreadId].ExecSQL(sSQL);
}

int ProcGPSHeart(TPacket &rPacket, int nThreadId)	//处理心跳包
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
	//判断告警是否恢复
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
			//过滤不告警的alert_id
			continue;
		}
		if ((unSolvedFlag & (0x01 << i)) ? 1 : 0)
		{
			if (i + 1 == 5)	//按键告警，由业务模块恢复
			{
				continue;
			}
			//该字段有告警
			sprintf(szBuf, "update T_ALERT set release_time = str_to_date('%s', '%%Y%%m%%d%%H%%i%%s') where sim_phone = '%s' and alert_id = %d and release_time is null;", \
					sDateTime.c_str(), sSimPhone.c_str(), i + 1);
			sSQL = szBuf;
			int nRet = g_rDatabaseList[nThreadId].ExecSQL(sSQL);
			WriteLogByLevel(1, "[%s]车况报警解除，alert_id: %d, alert_time: %s, 入库结果:%d\n", \
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

	//温湿度是高位在前，低位在后，要转成低位在前后使用GetMemInt取值
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
	int nLatFlag = rPacket.pBuffer[2] == 0x4E ? 1 : 2;	// 1:北纬 2: 南纬
	unsigned int unLat1 = GetMemInt(rPacket.pBuffer + 3, 1);	//纬度_ 度
	unsigned int unLat2 = GetMemInt(rPacket.pBuffer + 4, 1);	//纬度_ 分
	unsigned int unLat3 = GetMemInt(rPacket.pBuffer + 5, 1);	//纬度_ 分的小数部分
	unsigned int unLat4 = GetMemInt(rPacket.pBuffer + 6, 1);	//纬度_ 分的小数部分
	sprintf(szBuf, "%u.%02u%02u", unLat2, unLat3, unLat4);
	printf("纬度: %u度%s分\n", unLat1, szBuf);
	double dLat = atof(szBuf) / 60 + unLat1;
	int nLongFlag = rPacket.pBuffer[7] == 0x45 ? 1 : 2;	// 1:东经2: 西经
	unsigned int unLong1 = GetMemInt(rPacket.pBuffer + 8, 1);	//经度_ 度
	unsigned int unLong2 = GetMemInt(rPacket.pBuffer + 9, 1);	//经度_ 分
	unsigned int unLong3 = GetMemInt(rPacket.pBuffer + 10, 1);	//经度_ 分的小数部分
	unsigned int unLong4 = GetMemInt(rPacket.pBuffer + 11, 1);	//经度_ 分的小数部分
	sprintf(szBuf, "%u.%02u%02u", unLong2, unLong3, unLong4);
	double dLong = atof(szBuf) / 60 + unLong1;
	if (unLat1 == 0 || unLong1 == 0)
	{
		return 0;
	}
	unsigned int unSpeed = GetMemInt(rPacket.pBuffer + 12, 1);	//速度
	unsigned int unAspect1 = GetMemInt(rPacket.pBuffer + 13, 1);	//方向
	unsigned int unAspect2 = GetMemInt(rPacket.pBuffer + 14, 1);	//方向

	/********蓝斯协议更新，删除"方向2"、增加"累计里程0"**********
	unsigned int unAspect3 = GetMemInt(rPacket.pBuffer + 15, 1);	//方向小数
	sprintf(szBuf, "%u%02u.%02u", unAspect1, unAspect2, unAspect3);
	double dAspect = atof(szBuf);
	
	unsigned int unMileage1 = GetMemInt(rPacket.pBuffer + 16, 2);	//累计里程
	unsigned int unMileage2 = GetMemInt(rPacket.pBuffer + 18, 1);	//累计里程小数
	sprintf(szBuf, "%u.%02u", unMileage1, unMileage2);
	double dMileage = atof(szBuf);
	*******************************************************************************/
	sprintf(szBuf, "%u%02u.0", unAspect1, unAspect2);
	double dAspect = atof(szBuf);
	//累计里程=A*25600 + B*100 + C + D/100 
	unsigned int unMileageA = GetMemInt(rPacket.pBuffer + 15, 1);	//累计里程
	unsigned int unMileageB = GetMemInt(rPacket.pBuffer + 16, 1);	//累计里程
	unsigned int unMileageC = GetMemInt(rPacket.pBuffer + 17, 1);	//累计里程
	unsigned int unMileageD = GetMemInt(rPacket.pBuffer + 18, 1);	//累计里程
	unsigned int unMileageInt = unMileageA * 25600 + unMileageB * 100 + unMileageC;
	double dMileage = unMileageD / 100.0 + unMileageInt;
	
	string sTime = "20";
	sTime = sTime + GetMemHexString(rPacket.pBuffer + 19, 6);		//时间YYYYMMDDHH24MISS  '%Y%m%d%H%i%s'
	unsigned int unCondition = GetMemInt(rPacket.pBuffer + 25, 4);	//车况标志

	//更新已恢复的告警
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

//判断哪些告警已经恢复
unsigned int GetSolvedAlert(unsigned int unAlertCondition, unsigned int unNewCondition)
{
	unsigned int unRet = unAlertCondition ^ unNewCondition;	//异或，状态有改变的位被设置成1
	return (unRet & unAlertCondition);	//过滤掉unNewCondition里的新告警
}

//判断哪些是新增告警
unsigned int GetNewAlert(unsigned int unAlertCondition, unsigned int unNewCondition)
{
	unsigned int unRet = unAlertCondition ^ unNewCondition;	//异或，状态有改变的位被设置成1
	return (unRet & unNewCondition);	//过滤掉unAlertCondition里的旧告警
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
	FmtNowTime("请注意: %Y年%m月%d日%H点%M分, ", szNowTime, 64);
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
	//sSMSContent.append("车辆");
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
	if (!CovertKey((char *)g_rConfig.s3desKey.c_str(), sz24BitKey))	//48字节密钥转成24位密钥
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
	RunPad(nPadMode, szPacket, strlen(szPacket), szSrc, &nSrcLen);	//明文补位
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
			//过滤不告警的alert_id
			continue;
		}
		if ((unCondition & (0x00000001 << i)) ? 1 : 0)
		{
			//该字段有告警
			sprintf(szBuf, "insert into T_ALERT (sim_phone, alert_id, alert_time) values('%s', %d, str_to_date('%s', '%%Y%%m%%d%%H%%i%%s'));", \
					sSimPhone.c_str(), i + 1, sDateTime.c_str());
			sSQL = szBuf;
			nRet = g_rDatabaseList[nThreadId].ExecSQL(sSQL);
			WriteLogByLevel(1, "[%s]车况报警，alert_id: %d, alert_time: %s, 入库结果:%d\n", \
						sSimPhone.c_str(), i + 1, sDateTime.c_str(), nRet);

			SendAlertSMS(nThreadId, sSimPhone, i+1);
			
			//如果是按键报警，回复已经处理
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
	int nLatFlag = rPacket.pBuffer[2] == 0x4E ? 1 : 2;	// 1:北纬 2: 南纬
	unsigned int unLat1 = GetMemInt(rPacket.pBuffer + 3, 1);	//纬度_ 度
	unsigned int unLat2 = GetMemInt(rPacket.pBuffer + 4, 1);	//纬度_ 分
	unsigned int unLat3 = GetMemInt(rPacket.pBuffer + 5, 1);	//纬度_ 分的小数部分
	unsigned int unLat4 = GetMemInt(rPacket.pBuffer + 6, 1);	//纬度_ 分的小数部分
	sprintf(szBuf, "%u.%02u%02u", unLat2, unLat3, unLat4);
	double dLat = atof(szBuf) / 60 + unLat1;
	int nLongFlag = rPacket.pBuffer[7] == 0x45 ? 1 : 2;	// 1:东经2: 西经
	unsigned int unLong1 = GetMemInt(rPacket.pBuffer + 8, 1);	//经度_ 度
	unsigned int unLong2 = GetMemInt(rPacket.pBuffer + 9, 1);	//经度_ 分
	unsigned int unLong3 = GetMemInt(rPacket.pBuffer + 10, 1);	//经度_ 分的小数部分
	unsigned int unLong4 = GetMemInt(rPacket.pBuffer + 11, 1);	//经度_ 分的小数部分
	sprintf(szBuf, "%u.%02u%02u", unLong2, unLong3, unLong4);
	double dLong = atof(szBuf) / 60 + unLong1;
	unsigned int unSpeed = GetMemInt(rPacket.pBuffer + 12, 1);	//速度
	unsigned int unAspect1 = GetMemInt(rPacket.pBuffer + 13, 1);	//方向
	unsigned int unAspect2 = GetMemInt(rPacket.pBuffer + 14, 1);	//方向
	unsigned int unAspect3 = GetMemInt(rPacket.pBuffer + 15, 1);	//方向小数
	sprintf(szBuf, "%u%02u.%02u", unAspect1, unAspect2, unAspect3);
	double dAspect = atof(szBuf);
	unsigned int unMileage1 = GetMemInt(rPacket.pBuffer + 16, 2);	//累计里程
	unsigned int unMileage2 = GetMemInt(rPacket.pBuffer + 18, 1);	//累计里程小数
	sprintf(szBuf, "%u.%02u", unMileage1, unMileage2);
	double dMileage = atof(szBuf);
	string sTime = "20";
	sTime = sTime + GetMemHexString(rPacket.pBuffer + 19, 6);		//时间YYYYMMDDHH24MISS  '%Y%m%d%H%i%s'
	unsigned int unCondition = GetMemInt(rPacket.pBuffer + 25, 4);	//车况标志

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

//在接收到ESC(0x1b)字符后，首先去掉该字节，并对随后的字节用ESC进行异或。
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
			break;	//字符串尾
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
		WriteLogByLevel(2, "[>%d<]ProcGPSImage, 未找到帧头标志0x7E.\n", rPacket.nSockFd);
		return -1;
	}

	string sImageData(rPacket.pBuffer + 6, rPacket.unLen - 6);
	int nEndPos = sImageData.find((char)0x7e);
	if (nEndPos == string::npos)
	{
		WriteLogByLevel(2, "[>%d<]ProcGPSImage, 未找到帧尾标志0x7E.\n", rPacket.nSockFd);
		return -1;
	}
	
	char cCamNo = sImageData[0] - 0x10;	//摄像机序列号0--3
	sImageData = sImageData.substr(3, nEndPos - 3 - 2); //除去帧头尾的0x7e标志后的帧体以及校验位
	
	DecodeESC(sImageData);		//处理转义符
	
	string sEndFlag = "";
	sEndFlag.append(1, (char)0xFF);
	sEndFlag.append(1, (char)0xD9);
	nEndPos = sImageData.find(sEndFlag);
	if (nEndPos != string::npos)	//有JPEG结束符，删除结束符后的数据, 保留结束符
	{
		sImageData = sImageData.substr(0, nEndPos + 2);
	}
	
	unsigned char c0 = sImageData.at(0);
	unsigned char c1 = sImageData.at(1);
	if ((c0 == 0xFF) && (c1 == 0xD8))	//JPEG头标识
	{
		g_rImageBuf.SetImageBufBySockFd(rPacket.nSockFd, sImageData);
	}
	else
	{
		string sBuf = g_rImageBuf.GetImageBufBySockFd(rPacket.nSockFd);
		sBuf.append(sImageData);
		g_rImageBuf.SetImageBufBySockFd(rPacket.nSockFd, sBuf);
	}
	
	if (nEndPos != string::npos)	//有JPEG结束符
	{
		string sImageBuf = g_rImageBuf.GetImageBufBySockFd(rPacket.nSockFd);
		g_rImageBuf.Erase(rPacket.nSockFd);

		//图片接收完整，入库
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

		WriteLogByLevel(1, "[>%d<]保存照片文件成功: %s\n", rPacket.nSockFd, szFileName);
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

//终端设置应答
int ProcGPSConfigResp(TPacket &rPacket, int nThreadId)
{
	WriteLogByLevel(1, "[>%d<]<<<<<<<<<<<GPS, ProcGPSConfigResp.\n", rPacket.nSockFd);
	char szBuf[1024] = {0};
	string sSimPhone = g_rTermList.GetSimPhoneBySockFd(rPacket.nSockFd);

	string sUserId = GetMemNumString(rPacket.pBuffer + 1, 2);
	string sCommandId = GetMemNumString(rPacket.pBuffer + 3, 2);
	string sResp(rPacket.pBuffer + 5, rPacket.unLen - 5);

	//匹配请求，返回WEBSERVICE
	//匹配请求，返回WEBSERVICE
	TSession *pSession = g_rSessionMap[nThreadId].Find(atol(sUserId.c_str()));
	if (pSession)
	{
		pSession->m_rResult.nResult = 0;
		pSession->m_nProcFlag = 1;
	}
	return 0;
}

//终端命令执行成功应答
int ProcGPSCmdSuccessResp(TPacket &rPacket, int nThreadId)
{
	WriteLogByLevel(1, "[>%d<]<<<<<<<<<<<GPS, ProcGPSCommandSuccessResp.\n", rPacket.nSockFd);
	char szBuf[1024] = {0};
	string sSimPhone = g_rTermList.GetSimPhoneBySockFd(rPacket.nSockFd);

	string sUserId = GetMemNumString(rPacket.pBuffer + 1, 2);
	string sCommandId = GetMemNumString(rPacket.pBuffer + 3, 2);

	//匹配请求，返回WEBSERVICE
	TSession *pSession = g_rSessionMap[nThreadId].Find(atol(sUserId.c_str()));
	if (pSession)
	{
		pSession->m_rResult.nResult = 0;
		pSession->m_nProcFlag = 1;
	}
	return 0;
}

//终端命令执行失败应答
int ProcGPSCmdFailResp(TPacket &rPacket, int nThreadId)
{
	WriteLogByLevel(1, "[>%d<]<<<<<<<<<<<GPS, ProcGPSCmdFailResp.\n", rPacket.nSockFd);
	char szBuf[1024] = {0};
	string sSimPhone = g_rTermList.GetSimPhoneBySockFd(rPacket.nSockFd);

	string sUserId = GetMemNumString(rPacket.pBuffer + 1, 2);
	string sCommandId = GetMemNumString(rPacket.pBuffer + 3, 2);

	//匹配请求，返回WEBSERVICE
	TSession *pSession = g_rSessionMap[nThreadId].Find(atol(sUserId.c_str()));
	if (pSession)
	{
		pSession->m_rResult.nResult = -1;
		pSession->m_nProcFlag = 2;
	}
	return 0;
}

//终端版本查询应答
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
	//匹配请求，返回WEBSERVICE
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

//终端普通数据
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
	if (sData.find("开始装车") != string::npos)
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
	else if (sData.find("结束装车") != string::npos)
	{
		g_rShipSerialMap.EraseSerial(sSimPhone);
		sprintf(szBuf, "update T_SHIP_INFO set end_time = now() where sim_phone = '%s' and end_time is null;", sSimPhone.c_str());
		string sSQL = szBuf;
		nRet = g_rDatabaseList[nThreadId].ExecSQL(sSQL);
		g_rTermList.SetRFIDWorkingFlagBySimPhone(sSimPhone, 0);
		return nRet;
	}
	else if (sData.find("开始卸货") != string::npos)
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
	else if (sData.find("结束卸货") != string::npos)
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

//录像模块当前状态应答
int ProcGPSVideoStatResp(TPacket &rPacket, int nThreadId)
{
	WriteLogByLevel(1, "[>%d<]<<<<<<<<<<<GPS, ProcGPSVideoStatResp.\n", rPacket.nSockFd);
	char szBuf[1024] = {0};
	string sSimPhone = g_rTermList.GetSimPhoneBySockFd(rPacket.nSockFd);

	string sUserId = GetMemNumString(rPacket.pBuffer + 1, 2);
	string sCommandId = GetMemNumString(rPacket.pBuffer + 3, 2);

	string sStatBuf = rPacket.pBuffer + 5;
	
	//项目前面都用":"把项目和值区分开。不同的项目都用;隔开。后续增加都从尾部开始。
	//VIDEO_1~ VIDEO_4----摄像头1~4的状态
	//SD1_Total----SD1卡的容量
	//SD1_Free-----SD1卡的剩余空间
	//WORK---------录像工作状态
	//例如VIDEO_1:OK;VIDEO_2:ERROR;VIDEO_3:ERROR;VIDEO_4:OK;SD1_Total:16000M;SD1_Free :4096M;SD2_Total:0M;SD2_Free :0M;WORK:YES

	//匹配请求，返回WEBSERVICE

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

