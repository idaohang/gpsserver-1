#include <string>
#include "SendToGPS.h"
#include "LogAdmin.h"
#include "GlobalStruct.h"
#include "RFID.h"

extern void SetMemNumString(char *szBuf, int nLen, const char *szNum);
extern void SetMemInt(char *szBuf, unsigned int nLen, const int nInt);
extern void SetMemHexString(char *szBuf, int nLen, const char *szNum);
extern CPacketQueue g_rSendGPSPkgQueue[MAX_PROC_GPS_THREAD];

string MakePacket(const char *szPacketBody, int nBodyLen)
{
	printf("MakePacket, IN: ");
	for (int i = 0; i < nBodyLen; ++i)
	{
		printf("%02x ", *(unsigned char *)(szPacketBody + i));
	}
	printf("\n");
	
	string sPacket;
	char szBuf[16] = {0};
	szBuf[0] = 0x02;
	SetMemInt(szBuf + 1, 2, nBodyLen);

	sPacket.append(szBuf, 3);
	sPacket.append(szPacketBody, nBodyLen);
	sPacket.append(1, 0x03);

	printf("MakePacket, OUT: ");
	for (int i = 0; i < sPacket.length(); ++i)
	{
		printf("%02x ", *(unsigned char *)(sPacket.data() + i));
	}
	printf("\n");
	
	return sPacket;
}

void SendPacket(string &sPacket, int nSockFd)
{
	TPacket rPkg;
	rPkg.nSockFd = nSockFd;
	rPkg.pBuffer = (char *)malloc(sPacket.length());
	memset(rPkg.pBuffer, 0x00, sPacket.length());
	memcpy(rPkg.pBuffer, sPacket.data(), sPacket.length());
	rPkg.unLen = sPacket.length();
	int nThreadId = nSockFd % MAX_PROC_GPS_THREAD;
	g_rSendGPSPkgQueue[nThreadId].WriteDataToQueue(&rPkg);
}

void SendToGPSTakePhoto(TTakePhotoReq &req, int nSockFd)
{
	WriteLogByLevel(1, "[>%d<]>>>>>>>>>>>GPS, SendToGPSTakePhoto.\n", nSockFd);
	char szBuf[16] = {0};
	szBuf[0] = 'P';
	SetMemNumString(szBuf + 1, 2, req.sUserID.c_str());
	SetMemNumString(szBuf + 3, 2, req.sCammandID.c_str());
	SetMemInt(szBuf + 5, 1, req.nCamID);
	SetMemInt(szBuf + 6, 1, req.nAction);
	SetMemInt(szBuf + 7, 1, req.nPhotoSize);
	string sPacket = MakePacket(szBuf, 8);
	SendPacket(sPacket, nSockFd);
}

void SendToGPSConfig(TConfigReq &req, int nSockFd)
{
	WriteLogByLevel(1, "[>%d<]>>>>>>>>>>>GPS, SendToGPSConfig.\n", nSockFd);
	char szBuf[8] = {0};
	szBuf[0] = 'Q';
	SetMemNumString(szBuf + 1, 2, req.sUserID.c_str());
	SetMemNumString(szBuf + 3, 2, req.sCammandID.c_str());
	string sBody(szBuf, 5);
	sBody = sBody + req.sCommand;
	string sPacket = MakePacket(sBody.data(), sBody.length());
	SendPacket(sPacket, nSockFd);
}

void SendToGPSVersion(TVersionReq &req, int nSockFd)
{
	WriteLogByLevel(1, "[>%d<]>>>>>>>>>>>GPS, SendToGPSVersion.\n", nSockFd);
	char szBuf[8] = {0};
	szBuf[0] = 'V';
	SetMemNumString(szBuf + 1, 2, req.sUserID.c_str());
	SetMemNumString(szBuf + 3, 2, req.sCammandID.c_str());
	string sPacket = MakePacket(szBuf, 5);
	SendPacket(sPacket, nSockFd);
}

//收到终端报警后，需要发送处理告警后，终端以后才能继续发送告警
void SendToGPSReleaseAlert(TReleaseAlertReq &req, int nSockFd)
{
	WriteLogByLevel(1, "[>%d<]>>>>>>>>>>>GPS, SendToGPSReleaseAlert.\n", nSockFd);
	char szBuf[8] = {0};
	szBuf[0] = 'C';
	SetMemNumString(szBuf + 1, 2, req.sUserID.c_str());
	SetMemNumString(szBuf + 3, 2, req.sCammandID.c_str());
	string sPacket = MakePacket(szBuf, 5);
	SendPacket(sPacket, nSockFd);
}


//下发文字调度信息，中文用GB编码
void SendToGPSControlInfo(TControlInfoReq &req, int nSockFd)
{
	WriteLogByLevel(1, "[>%d<]>>>>>>>>>>>GPS, SendToGPSControlInfo.\n", nSockFd);
	char szBuf[8] = {0};
	szBuf[0] = 'M';
	SetMemNumString(szBuf + 1, 2, req.sUserID.c_str());
	SetMemNumString(szBuf + 3, 2, req.sCammandID.c_str());
	string sBody(szBuf, 5);
	sBody = sBody + req.sInfo;
	string sPacket = MakePacket(sBody.data(), sBody.length());
	SendPacket(sPacket, nSockFd);
}

void SendToGPSPosition(TPositionReq &req, int nSockFd)
{
	WriteLogByLevel(1, "[>%d<]>>>>>>>>>>>GPS, SendToGPSPosition.\n", nSockFd);
	char szBuf[16] = {0};
	szBuf[0] = 0x60;
	SetMemNumString(szBuf + 1, 2, req.sUserID.c_str());
	SetMemNumString(szBuf + 3, 2, req.sCammandID.c_str());
	SetMemInt(szBuf + 5,  4, req.nTime);
	SetMemInt(szBuf + 9, 4, req.nInterval);
	string sPacket = MakePacket(szBuf, 13);
	SendPacket(sPacket, nSockFd);
}

void SendToGPSCancelMoni(TCancelMoniReq &req, int nSockFd)
{
	WriteLogByLevel(1, "[>%d<]>>>>>>>>>>>GPS, SendToGPSCancelMoni.\n", nSockFd);
	char szBuf[8] = {0};
	szBuf[0] = 0x61;
	SetMemNumString(szBuf + 1, 2, req.sUserID.c_str());
	SetMemNumString(szBuf + 3, 2, req.sCammandID.c_str());
	string sPacket = MakePacket(szBuf, 5);
	SendPacket(sPacket, nSockFd);
}

void SendToGPSServInfo(TServInfoReq &req, int nSockFd)
{
	WriteLogByLevel(1, "[>%d<]>>>>>>>>>>>GPS, SendToGPSServInfo.\n", nSockFd);
	char szBuf[128] = {0};
	szBuf[0] = 0x7B;
	SetMemNumString(szBuf + 1, 2, req.sUserID.c_str());
	SetMemNumString(szBuf + 3, 2, req.sCammandID.c_str());
	strcpy(szBuf + 5, req.sUpdateServerIp.c_str());
	strcpy(szBuf + 25, req.sUpdateServerPort.c_str());
	strcpy(szBuf + 30, req.sLogServerIp.c_str());
	strcpy(szBuf + 50, req.sLogServerPort.c_str());
	strcpy(szBuf + 55, req.sUserName.c_str());
	strcpy(szBuf + 75, req.sPassword.c_str());
	string sPacket = MakePacket(szBuf, 95);
	SendPacket(sPacket, nSockFd);
}

void SendToGPSCamStat(int nSockFd)
{
	WriteLogByLevel(1, "[>%d<]>>>>>>>>>>>GPS, SendToGPSCamStat.\n", nSockFd);
	char szBuf[8] = {0};
	szBuf[2] = 0x81;
	string sPacket = MakePacket(szBuf, 3);
	SendPacket(sPacket, nSockFd);
}

void SendToGPSHeart(int nSockFd)
{
	WriteLogByLevel(1, "[>%d<]>>>>>>>>>>>GPS, SendToGPSHeart.\n", nSockFd);
	char szBuf[8] = {0};
	szBuf[0] = 0x78;
	string sPacket = MakePacket(szBuf, 1);
	SendPacket(sPacket, nSockFd);
}

void SendToGPSSetMileage(TSetMileageReq &req, int nSockFd)
{
	WriteLogByLevel(1, "[>%d<]>>>>>>>>>>>GPS, SendToGPSSetMileage.\n", nSockFd);
	char szBuf[16] = {0};
	szBuf[0] = 0x41;
	SetMemNumString(szBuf + 1, 2, req.sUserID.c_str());
	SetMemNumString(szBuf + 3, 2, req.sCammandID.c_str());
	char szMileageInt[8] = {0};
	sprintf(szMileageInt, "%u", req.nMileageInt);
	SetMemHexString(szBuf + 5, 2, szMileageInt);
	SetMemInt(szBuf + 7, 1, req.nMileageFloat);
	string sPacket = MakePacket(szBuf, 8);
	SendPacket(sPacket, nSockFd);
}

//更新系统配置.ini的配置文件
void SendToGPSUpdateConfig(TUpdateConfigReq &req, int nSockFd)
{
	WriteLogByLevel(1, "[>%d<]>>>>>>>>>>>GPS, SendToGPSUpdateConfig.\n", nSockFd);
	char szBuf[8] = {0};
	szBuf[0] = 0x41;
	SetMemNumString(szBuf + 1, 2, req.sUserID.c_str());
	SetMemNumString(szBuf + 3, 2, req.sCammandID.c_str());
	string sBody(szBuf, 5);
	sBody = sBody + req.sCommand;
	string sPacket = MakePacket(sBody.data(), sBody.length());
	SendPacket(sPacket, nSockFd);
}

//设定报警区域
void SendToGPSSetArea(TSetAreaReq &req, int nSockFd)
{
	WriteLogByLevel(1, "[>%d<]>>>>>>>>>>>GPS, SendToGPSSetArea.\n", nSockFd);
	char szBuf[128] = {0};
	szBuf[0] = 0x62;
	SetMemNumString(szBuf + 1, 2, req.sUserID.c_str());
	SetMemNumString(szBuf + 3, 2, req.sCammandID.c_str());
	unsigned int nPointCount = req.pointList.size() > 8 ? 8 : req.pointList.size();
	SetMemInt(szBuf + 5, 1, nPointCount);
	
	std::list<TGPSPoint>::iterator it = req.pointList.begin();
	for (int i = 0; i < nPointCount && it != req.pointList.end(); ++it, ++i)
	{
		int nLatInt = (int)(it->dLat);			//纬度整数
		double dLatFloat = it->dLat - nLatInt;		//纬度小数
		double dLatMin = dLatFloat * 60;				//小数转成分
		int nLatMinInt = (int)dLatMin;			//分的整数
		double dLatMinFloat = dLatMin - nLatMinInt;	//分的小数部分
		int nLatMinFloat1 = (int)(dLatMinFloat * 100);	//小数点后前两位
		int nLatMinFloat2 = ((int)(dLatMinFloat * 10000)) % 100;	//小数点后前三四位
		SetMemInt(szBuf + 6 + i * 8, 1, nLatInt);
		SetMemInt(szBuf + 6 + i * 8 + 1, 1, nLatMinInt);
		SetMemInt(szBuf + 6 + i * 8 + 2, 1, nLatMinFloat1);
		SetMemInt(szBuf + 6 + i * 8 + 3, 1, nLatMinFloat2);

		int nLongInt = (int)(it->dLong);			//纬度整数
		double dLongFloat = it->dLong - nLongInt;		//纬度小数
		double dLongMin = dLongFloat * 60;				//小数转成分
		int nLongMinInt = (int)dLongMin;			//分的整数
		double dLongMinFloat = dLongMin - nLongMinInt;	//分的小数部分
		int nLongMinFloat1 = (int)(dLongMinFloat * 100);	//小数点后前两位
		int nLongMinFloat2 = ((int)(dLongMinFloat * 10000)) % 100;	//小数点后前三四位
		SetMemInt(szBuf + 6 + i * 8 + 4, 1, nLongInt);
		SetMemInt(szBuf + 6 + i * 8 + 5, 1, nLongMinInt);
		SetMemInt(szBuf + 6 + i * 8 + 6, 1, nLongMinFloat1);
		SetMemInt(szBuf + 6 + i * 8 + 7, 1, nLongMinFloat2);
	}
	
	string sPacket = MakePacket(szBuf, 6 + nPointCount * 8);
	SendPacket(sPacket, nSockFd);
}

void SendToGPS485Data(T485Data &req, int nSockFd)
{
	WriteLogByLevel(1, "[>%d<]>>>>>>>>>>>GPS, SendToGPS485Data.\n", nSockFd);
	char szBuf[256] = {0};
	szBuf[0] = 0x7c;
	SetMemNumString(szBuf + 1, 2, req.sUserID.c_str());
	SetMemNumString(szBuf + 3, 2, req.sCammandID.c_str());

	memcpy(szBuf + 5, req.sData.data(), req.sData.length());
	
	string sPacket = MakePacket(szBuf, req.sData.length() + 5);
	SendPacket(sPacket, nSockFd);
}

void SendToGPSScanEPC(int nSockFd)
{
	T485Data req;
	req.sCammandID = "0";
	req.sUserID = "0";
	req.sData = RFIDScanEPC();
	SendToGPS485Data(req, nSockFd);
}


