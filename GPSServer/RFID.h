#ifndef _RFID_H_
#define _RFID_H_
#include <string>
#include <list>
using namespace std;

string RFIDScanEPC();
string RFIDSetReaderAdr(char cAddr)	;
string RFIDSetScanTime(char cTime);
string RFIDSetPower(char cPower);
string RFIDSetAnswerMode();
string RFIDSetReaderBaudRate(char cBaudRate);
int DecodeRespPacket(string &sRespPkg, int nThreadId, string &sSimPhone);
#endif

