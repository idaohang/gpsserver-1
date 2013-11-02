#ifndef _PROC_GPS_MSG_H_
#define _PROC_GPS_MSG_H_

void SetMemNumString(char *szBuf, int nLen, const char *szNum);
//0x0C 0x22 0x38 0x4E --> "12 34 56 78"
string GetMemNumString(const char *szBuf, int nLen);

void SetMemInt(char *szBuf, unsigned int nLen, const int nInt);
unsigned int GetMemInt(char *szBuf, unsigned int nLen);

void SetMemHexString(char *szBuf, int nLen, const char *szNum);

int ProcGPSLogout(TPacket &rPacket, int nThreadId);

int ProcGPSLogin(TPacket &rPacket, int nThreadId);

int ProcGPSHeart(TPacket &rPacket, int nThreadId)	;

int SolvedAlert(int nThreadId, unsigned int unNewCondition, string sSimPhone, string sDateTime);

int ProcGPSRealInfo(TPacket &rPacket, int nThreadId);

//判断哪些告警已经恢复
unsigned int GetSolvedAlert(unsigned int unAlertCondition, unsigned int unNewCondition);
//判断哪些是新增告警
unsigned int GetNewAlert(unsigned int unAlertCondition, unsigned int unNewCondition);

int ProcAlertCondition(int nThreadId, unsigned int unCondition, string sSimPhone, string sDateTime);

int ProcGPSAlert(TPacket &rPacket, int nThreadId);

//在接收到ESC(0x1b)字符后，首先去掉该字节，并对随后的字节用ESC进行异或。
int DecodeESC(string &sIn);

int ProcGPSImage(TPacket &rPacket, int nThreadId);

//终端设置应答
int ProcGPSConfigResp(TPacket &rPacket, int nThreadId);

//终端命令执行成功应答
int ProcGPSCmdSuccessResp(TPacket &rPacket, int nThreadId);

//终端命令执行失败应答
int ProcGPSCmdFailResp(TPacket &rPacket, int nThreadId);

//终端版本查询应答
int ProcGPSQueryVerResp(TPacket &rPacket, int nThreadId);

//终端普通数据
int ProcGPSCommonData(TPacket &rPacket, int nThreadId);
//录像模块当前状态应答
int ProcGPSVideoStatResp(TPacket &rPacket, int nThreadId);

int SendGPSTakePhoto(int nUserID, int nCommandID, int nCamID, int nAction, int nPhotoSize);

int ProcGPSTemperatureInfo(TPacket &rPacket, int nThreadId);

int ProcGPS485Data(TPacket &rPacket, int nThreadId);

int ProcGPSICCardData(TPacket &rPacket, int nThreadId);
#endif

