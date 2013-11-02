/****************************************************************
 * 名   称 : ToolsAPI.h	
 * 主   题 : 全局声明
 * 注   明 : FFSC  Queym   New Create     2006/01/10        
 *           Copyright by FFSC
****************************************************************/
#ifndef __API_TOOLS_H__
#define __API_TOOLS_H__
#include "FFCS_TypeDef.h"
/***************************************************************/
/*函数原型*/
extern ff_char *FmtTime(ff_time tTime, ff_char *strFmt, ff_char *strContent, ff_int iLen);
extern ff_char *FmtNowTime(ff_char *strFmt, ff_char *strContent, ff_int iLen);
extern ff_char *FmtNowAdjustTime(ff_char *strFmt, ff_char *strContent, ff_int iLen, ff_int iAdtSec);
extern ff_char *RTrimRN(ff_char *strContent);
extern ff_char *RTrim(ff_char *strContent);
extern ff_char *LTrim(char *strContent);
extern ff_char IntToBcd(ff_int iNum);
extern ff_int BcdToInt (ff_char cChar);
extern void AsciiToBcd (ff_uchar *pucBcd, ff_uchar *pucAscii, ff_int iLen, ff_uchar ucType);
extern void BcdToAscii(ff_uchar *pucAscii, ff_uchar *pucBcd, ff_int iLen, ff_uchar ucType);
extern void IffS(const ff_int iFlag, ff_char *strDest, const ff_char *strSrc, const ff_char *strDef);
extern void IfS(const ff_int iFlag, const ff_int iValue, ff_char *strDest, const ff_char *strSrc);
extern void MSleep(ff_int iMicroSecs);
extern void TimeFormatInt2Str(ff_int iTime, ff_uchar ucFormat, ff_char *strTime);
extern void TimeFormatStr2Int(ff_char *strTime, ff_uchar ucFormat, ff_uint *uiTime);
extern ff_char *ConvertCharToHex(char *strDest, const ff_char *strSrc, ff_int iLen);
extern ff_char *ConvertBinary(ff_char *strDest, ff_char *strSrc, ff_int iLen);
extern ff_int CheckNumber(ff_char *strContent);
ff_int CheckDirExist(const ff_char *strFileName, mode_t iMode = S_IRWXU);

#endif

