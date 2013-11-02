#ifndef _SEND_TO_GSP_H_
#define _SEND_TO_GSP_H_
#include <string>
#include "GlobalStruct.h"
#include "LogAdmin.h"

typedef struct 
{
	string sUserID;
	string sCammandID;
	int nCamID;
	int nAction;
	int nPhotoSize;
}TTakePhotoReq;

typedef struct 
{
	string sCommand;
	string sUserID;
	string sCammandID;
}TConfigReq;

typedef struct 
{
	string sUserID;
	string sCammandID;
}TVersionReq;

typedef struct 
{
	string sUserID;
	string sCammandID;
}TReleaseAlertReq;

typedef struct 
{
	string sUserID;
	string sCammandID;
	string sInfo;
}TControlInfoReq;

typedef struct 
{
	string sUserID;
	string sCammandID;
	int nTime;
	int nInterval;
}TPositionReq;

typedef struct
{
	string sUserID;
	string sCammandID;
}TCancelMoniReq;

typedef struct 
{
	string sUserID;
	string sCammandID;
	string sUpdateServerIp;
	string sUpdateServerPort;
	string sLogServerIp;
	string sLogServerPort;
	string sUserName;
	string sPassword;
}TServInfoReq;

typedef struct
{
	string sUserID;
	string sCammandID;
	int nMileageInt;
	int nMileageFloat;
}TSetMileageReq;

typedef struct 
{
	string sUserID;
	string sCammandID;
	string sCommand;
}TUpdateConfigReq;

typedef struct
{
	double dLong;
	double dLat;
}TGPSPoint;
typedef struct
{
	string sUserID;
	string sCammandID;
	std::list<TGPSPoint> pointList;
}TSetAreaReq;

typedef struct
{
	string sUserID;
	string sCammandID;
	string sData;
}T485Data;

string MakePacket(const char *szPacketBody, int nBodyLen);

void SendPacket(string &sPacket, int nSockFd);

void SendToGPSTakePhoto(TTakePhotoReq &req, int nSockFd);

void SendToGPSConfig(TConfigReq &req, int nSockFd);

void SendToGPSVersion(TVersionReq &req, int nSockFd);

void SendToGPSReleaseAlert(TReleaseAlertReq &req, int nSockFd);

void SendToGPSControlInfo(TControlInfoReq &req, int nSockFd);

void SendToGPSPosition(TPositionReq &req, int nSockFd);

void SendToGPSCancelMoni(TCancelMoniReq &req, int nSockFd);

void SendToGPSServInfo(TServInfoReq &req, int nSockFd);

void SendToGPSCamStat(int nSockFd);

void SendToGPSHeart(int nSockFd);

void SendToGPSSetMileage(TSetMileageReq &req, int nSockFd);

void SendToGPSUpdateConfig(TUpdateConfigReq &req, int nSockFd);

void SendToGPSSetArea(TSetAreaReq &req, int nSockFd);

void SendToGPS485Data(T485Data &req, int nSockFd);

void SendToGPSScanEPC(int nSockFd);
#endif

