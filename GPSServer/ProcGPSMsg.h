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

//�ж���Щ�澯�Ѿ��ָ�
unsigned int GetSolvedAlert(unsigned int unAlertCondition, unsigned int unNewCondition);
//�ж���Щ�������澯
unsigned int GetNewAlert(unsigned int unAlertCondition, unsigned int unNewCondition);

int ProcAlertCondition(int nThreadId, unsigned int unCondition, string sSimPhone, string sDateTime);

int ProcGPSAlert(TPacket &rPacket, int nThreadId);

//�ڽ��յ�ESC(0x1b)�ַ�������ȥ�����ֽڣ����������ֽ���ESC�������
int DecodeESC(string &sIn);

int ProcGPSImage(TPacket &rPacket, int nThreadId);

//�ն�����Ӧ��
int ProcGPSConfigResp(TPacket &rPacket, int nThreadId);

//�ն�����ִ�гɹ�Ӧ��
int ProcGPSCmdSuccessResp(TPacket &rPacket, int nThreadId);

//�ն�����ִ��ʧ��Ӧ��
int ProcGPSCmdFailResp(TPacket &rPacket, int nThreadId);

//�ն˰汾��ѯӦ��
int ProcGPSQueryVerResp(TPacket &rPacket, int nThreadId);

//�ն���ͨ����
int ProcGPSCommonData(TPacket &rPacket, int nThreadId);
//¼��ģ�鵱ǰ״̬Ӧ��
int ProcGPSVideoStatResp(TPacket &rPacket, int nThreadId);

int SendGPSTakePhoto(int nUserID, int nCommandID, int nCamID, int nAction, int nPhotoSize);

int ProcGPSTemperatureInfo(TPacket &rPacket, int nThreadId);

int ProcGPS485Data(TPacket &rPacket, int nThreadId);

int ProcGPSICCardData(TPacket &rPacket, int nThreadId);
#endif

