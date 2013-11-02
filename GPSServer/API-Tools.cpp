/****************************************************************
 * 文件名称: API-Tools.c                        
 * 功    能: 工具函数                         	
 * 备    注:									
 ***************************************************************/

/****************************************************************
 * 头文件包含区
****************************************************************/
#include "API-Tools.h"
#include "FFCS_TypeDef.h"

/****************************************************************
 * 名    称: FmtTime
 * 功    能: 时间格式化为字符串
 * 输    入: 
			 ff_time 	tTime			时间戳
 			 ff_char 	*strFmt			格式，参考strftime函数
 			 ff_char 	*strContent		格式化后的字符串
 			 ff_int 	iLen			长度
 * 输    出: 
			 *strContent			格式化字符串指针
 * 返    回: 
			 *strContent			格式化字符串指针
 * 备    注：
****************************************************************/
ff_char *FmtTime(ff_time tTime, ff_char *strFmt, ff_char *strContent, ff_int iLen)
{
	ff_tm		tmTmp;

	if ((tTime<0) || (strFmt==NULL) || (strContent==NULL) || (iLen<1))
	{
		return strContent;
	}

	localtime_r(&tTime, &tmTmp);
	
	strftime(strContent, iLen, strFmt, &tmTmp);

	return strContent;
}

/****************************************************************
 * 名    称: FmtNowTime
 * 功    能: 将当前时间格式化为字符串
 * 输    入: 
			 ff_char 	*strFmt			格式，参考strftime函数
 			 ff_char 	*strContent		格式化后的字符串
 			 ff_int 	iLen			长度
 * 输    出: 
			 *strContent			格式化字符串指针
 * 返    回: 
			 *strContent			格式化字符串指针
 * 备    注：
****************************************************************/
ff_char *FmtNowTime(ff_char *strFmt, ff_char *strContent, ff_int iLen)
{
	ff_time			tNow;
	ff_tm			tmTmp;

	if ((strFmt==NULL) || (strContent==NULL) || (iLen<1))
	{
		return strContent;
	}

	time(&tNow);

	localtime_r(&tNow, &tmTmp);

	strftime(strContent, iLen, strFmt, &tmTmp);

	return strContent;
}

/****************************************************************
 * 名    称: FmtNowAdjustTime
 * 功    能: 当前时间调整若干秒后格式化为字符串
 * 输    入: 
			 ff_char 	*strFmt			格式，参考strftime函数
 			 ff_char 	*strContent		格式化后的字符串
 			 ff_int 	iLen			长度
 			 ff_int 	iAdtSec			调整秒数，可正可负
 * 输    出: 
			 无
 * 返    回: 
			 
 * 备    注：
****************************************************************/
ff_char *FmtNowAdjustTime(ff_char *strFmt, ff_char *strContent, ff_int iLen, ff_int iAdtSec)
{
	ff_time			tAdjustTime;
	struct tm		tmTmp;
	struct tm		*pTm;

	if ((strFmt==NULL) || (strContent==NULL) || (iLen<1))
	{
		return strContent;
	}

	time(&tAdjustTime);

	tAdjustTime += iAdtSec;

	pTm = localtime_r(&tAdjustTime, &tmTmp);

	strftime(strContent, iLen, strFmt, pTm);

	return strContent;
}

/****************************************************************
 * 名    称: RTrimRN
 * 功    能: 去右回车换行
 * 输    入: 
			 ff_char *strContent	字符串
 * 输    出: 
			 无
 * 返    回: 
			 
 * 备    注：
****************************************************************/
ff_char *RTrimRN(ff_char *strContent)
{
	ff_int 	iLen = 0;
	ff_int 	iFlag = 1;

	if (strContent == NULL)
	{
		return strContent;
	}

	iLen = strlen(strContent);
	if (iLen == 0)
	{
		return strContent;
	}

	iLen--;

	while ((iLen>=0) && (iFlag==1)) 
	{
		if ((strContent[iLen]=='\n') || (strContent[iLen]=='\r'))
		{
			strContent[iLen] = 0;
		}
		else
		{
			iFlag = 0;
		}

		iLen--;
	}

	return strContent;
}

/****************************************************************
 * 名    称: RTrim
 * 功    能: 去右空格
 * 输    入: 
			 ff_char *strContent	字符串
 * 输    出: 
			 无
 * 返    回: 
			 
 * 备    注：
****************************************************************/
ff_char *RTrim(ff_char *strContent)
{
	ff_int 	iLen = 0;
	ff_int 	iFlag = 1;

	if (strContent == NULL)
	{
		return strContent;
	}

	iLen = strlen(strContent);
	if (iLen == 0)
	{
		return strContent;
	}

	iLen--;

	while ((iLen>=0) && (iFlag==1))
	{
		if (strContent[iLen] == ' ')
		{
			strContent[iLen] = 0;
		}
		else
		{
			iFlag = 0;
		}

		iLen--;
	}

	return strContent;
}

/****************************************************************
 * 名    称: LTrim
 * 功    能: 去左空格
 * 输    入: 
			 ff_char *strContent	字符串
 * 输    出: 
			 无
 * 返    回: 
			 
 * 备    注：
****************************************************************/
ff_char *LTrim(char *strContent)
{
	ff_int 		iLen = 0;
	ff_int		iCount = 0;
	ff_int 		i = 0;
	ff_int		iFlag = 1;

	if (strContent == NULL)
	{
		return strContent;
	}

	iLen = strlen(strContent);
	if (iLen == 0)
	{
		return strContent;
	}

	while ((iCount<iLen) && (iFlag==1))
	{
		if (strContent[iCount] == ' ')
		{
			iCount++;
		}
		else
		{
			iFlag = 0;
		}
	}
	if (iCount == 0)
	{
		return strContent;
	}

	for (i=0; i<(iLen-iCount); i++)
	{
	    strContent[i] = strContent[(iCount + i)];
	}
	strContent[(iLen-iCount)] = 0;	

	return strContent;
}

/****************************************************************
 * 名    称: IntToBcd
 * 功    能: 整形转BCD码
 * 输    入: 
			 ff_int iNum			整数值
 * 输    出: 
			 无
 * 返    回: 
			 
 * 备    注：
****************************************************************/
ff_char IntToBcd(ff_int iNum)
{
	return (ff_char)((iNum / 10) << 4) | (iNum % 10);
}

/****************************************************************
 * 名    称: BcdToInt
 * 功    能: BCD码转整型
 * 输    入: 
			 ff_char cChar			
 * 输    出: 
			 无
 * 返    回: 
			 
 * 备    注：
****************************************************************/
ff_int BcdToInt (ff_char cChar)
{
	return (ff_int)(cChar >> 4) * 10 + (cChar & 0x0f);
}

/****************************************************************
 * 名    称: AsciiToBcd
 * 功    能: ASCII码转BCD码
 * 输    入: 
			 ff_uchar 	*pucBcd
 			 ff_uchar 	*pucAscii
 			 ff_int 	iLen
 			 ff_uchar 	ucType
 * 输    出: 
			 无
 * 返    回: 
			
 * 备    注：
****************************************************************/
void AsciiToBcd (ff_uchar *pucAscii, ff_uchar *pucBcd, ff_int iLen, ff_uchar ucType)  
{
	ff_int		iCnt;  
	ff_char		cTmp1;
	ff_char		cTmp2;  
 
	if ((pucAscii==NULL) || (pucBcd==NULL))
	{
		return;
	}

    if ((iLen&0x01) && (ucType))/*判别是否为奇数以及往那边对齐*/
	{
		cTmp2 = 0;
	}
    else
	{
		cTmp2 = 0x55;
	}
	
	for (iCnt=0; iCnt<iLen; pucAscii++, iCnt++) 
	{
		if (*pucAscii >= 'a')
		{
			cTmp1 = *pucAscii - 'a' + 10;
		}
		else if (*pucAscii >= 'A')
		{
			cTmp1 = *pucAscii- 'A' + 10;
		}
		else if (*pucAscii >= '0')
		{
			cTmp1 = *pucAscii - '0';
		}
		else if (*pucAscii == 0)
		{
			cTmp1 = 0x0f ;
		}
		else
		{
			cTmp1 = 0;
		}

		if (cTmp1 == 0x55)
		{
			cTmp2 = cTmp1;
		}
		else 
		{
			*pucBcd++ = cTmp2 << 4 | cTmp1;
			cTmp2 = 0x55;
		}
	}

	if (cTmp2 != 0x55)
	{
        *pucBcd = cTmp2 << 4;
	}
	
   	return ;
}

/****************************************************************
 * 名    称: BcdToAscii
 * 功    能: BCD码转ASCII码
 * 输    入: 
			 ff_uchar 	*pucAscii
 			 ff_uchar 	*pucBcd
 			 ff_int 	iLen
 			 ff_uchar 	ucType
 * 输    出: 
			 无
 * 返    回: 
			 
 * 备    注：
****************************************************************/
void BcdToAscii(ff_uchar *pucBcd, ff_uchar *pucAscii, ff_int iLen, ff_uchar ucType)
{
	ff_int  iCnt = 0;
	
	if ((pucAscii==NULL) || (pucBcd==NULL))
	{
		return;
	}

	if ((iLen&0x01) && (ucType)) 	/*判别是否为奇数以及往那边对齐*/
	{                           		/*0左，1右*/
 	   	iCnt = 1;
	   	iLen++;
	}
	else
	{
		iCnt = 0;
	}
	
   	for (; iCnt<iLen; iCnt ++, pucAscii++) 
   	{
		*pucAscii = (( iCnt & 0x01) ? (*pucBcd++ & 0x0f) : (*pucBcd >> 4));
		*pucAscii += ((*pucAscii > 9) ? ('A' - 10) : '0');

		if (*pucAscii == 'F') 
		{
			*pucAscii = ' ';
		}
   	}

	return ;
}

/****************************************************************
 * 名    称: IffS
 * 功    能: PROC工具函数
 * 输    入: 
			 const ff_int 	iFlag
 			 ff_char 		*strDest
 			 const ff_char 	*strSrc
 			 const ff_char 	*strDef
 * 输    出: 
			 无
 * 返    回: 
			 
 * 备    注：
****************************************************************/
void IffS(const ff_int iFlag, ff_char *strDest, const ff_char *strSrc, const ff_char *strDef)
{
	if ((strDest==NULL) || (strSrc==NULL) || (strDef==NULL))
	{
		return;
	}

	if (iFlag == -1)
	{
		strcpy(strDest, strDef);
	}
	else
	{
		strcpy(strDest, strSrc);
	}
}

/****************************************************************
 * 名    称: IfS
 * 功    能: PROC工具函数
 * 输    入: 
			 const ff_int 	iFlag
 			 const ff_int 	iValue
 			 ff_char 		*strDest
 			 const ff_char 	*strSrc
 * 输    出: 
			 无
 * 返    回: 
			 
 * 备    注：
****************************************************************/
void IfS(const ff_int iFlag, const ff_int iValue, ff_char *strDest, const ff_char *strSrc)
{
	if ((strDest==NULL) || (strSrc==NULL))
	{
		return;
	}

	if (iFlag == iValue)
	{
		strcpy(strDest, strSrc);
	}
}

/****************************************************************
 * 名    称: MSleep
 * 功    能: 休眠，按微秒休眠
 * 输    入: 
			 ff_int iMicroSecs	休眠的微秒数
 * 输    出: 
			 无
 * 返    回: 
			
 * 备    注：
****************************************************************/
void MSleep(ff_int iMicroSecs)
{
    ff_timeval	tvTmp;

    tvTmp.tv_sec = iMicroSecs / 1000000;
    tvTmp.tv_usec = iMicroSecs % 1000000;

    select(0, 0, 0, 0, &tvTmp);

/*
	ff_timespec		tsTmp;

	tsTmp.tv_sec = iMicroSecs / 1000000;
	tsTmp.tv_nsec = iMicroSecs % 1000000;

	nanosleep(&tsTmp, NULL);
*/
}

/****************************************************************
 * 名    称: TimeFormatInt2Str
 * 功    能: 时间转换成字符串
 * 输    入: 
			 ff_int 	iTime		时间戳
 			 ff_uchar 	ucFormat	转换格式
 			 ff_char 	*strTime	转换后存放的字符串
 			 
 			 格式值说明：
								1			MMDDHHMM
								2			MMDDHHMMSS
								3			YYMMDDHHMM
								4			YYMMDDHHMMSS
								5			YYYYMMDDHHMMSS
								6			YYYYMMDDHHMMSS000
								7			YYMMDDHHMMSS000R
 * 输    出: 
			 无
 * 返    回: 
			 
 * 备    注：
****************************************************************/
void TimeFormatInt2Str(ff_int iTime, ff_uchar ucFormat, ff_char *strTime)
{
	ff_time			tCurrent;
	ff_tm			tmFmtDatetime;
	ff_tm			*ptmFmtDatetime = NULL;
	
	if(iTime == 0)
	{
		return;
	}

	tCurrent = iTime;
	ptmFmtDatetime = localtime_r(&tCurrent, &tmFmtDatetime);
	if (ptmFmtDatetime == NULL)
	{
		return;
	}
	
	if (ucFormat == 1)
	{
		sprintf(strTime, "%02d%02d%02d%02d", ptmFmtDatetime->tm_mon+1, ptmFmtDatetime->tm_mday, ptmFmtDatetime->tm_hour, ptmFmtDatetime->tm_min);
	}
	else if (ucFormat == 2)
	{
		sprintf(strTime, "%02d%02d%02d%02d%02d", ptmFmtDatetime->tm_mon+1, ptmFmtDatetime->tm_mday, ptmFmtDatetime->tm_hour, ptmFmtDatetime->tm_min, ptmFmtDatetime->tm_sec);
	}
	else if (ucFormat == 3)
	{
		sprintf(strTime, "%02d%02d%02d%02d%02d", ptmFmtDatetime->tm_year-100, ptmFmtDatetime->tm_mon+1, ptmFmtDatetime->tm_mday, ptmFmtDatetime->tm_hour, ptmFmtDatetime->tm_min);
	}
	else if (ucFormat == 4)
	{
		sprintf(strTime, "%02d%02d%02d%02d%02d%02d", ptmFmtDatetime->tm_year-100, ptmFmtDatetime->tm_mon+1, ptmFmtDatetime->tm_mday, ptmFmtDatetime->tm_hour, ptmFmtDatetime->tm_min, ptmFmtDatetime->tm_sec);
	}
	else if (ucFormat == 5)
	{
		sprintf(strTime, "20%02d%02d%02d%02d%02d%02d", ptmFmtDatetime->tm_year-100, ptmFmtDatetime->tm_mon+1, ptmFmtDatetime->tm_mday, ptmFmtDatetime->tm_hour, ptmFmtDatetime->tm_min, ptmFmtDatetime->tm_sec);
	}
	else if (ucFormat == 7)
	{
		sprintf(strTime, "%02d%02d%02d%02d%02d%02d000R", ptmFmtDatetime->tm_year-100, ptmFmtDatetime->tm_mon+1, ptmFmtDatetime->tm_mday, ptmFmtDatetime->tm_hour, ptmFmtDatetime->tm_min, ptmFmtDatetime->tm_sec);
	}
	else
	{
		sprintf(strTime, "20%02d%02d%02d%02d%02d%02d000", ptmFmtDatetime->tm_year-100, ptmFmtDatetime->tm_mon+1, ptmFmtDatetime->tm_mday, ptmFmtDatetime->tm_hour, ptmFmtDatetime->tm_min, ptmFmtDatetime->tm_sec);
	}
}

/****************************************************************
 * 名    称: TimeFormatStr2Int
 * 功    能: 时间转换成字符串
 * 输    入: 
 			 ff_char 	*strTime	转换后存放的字符串
 			 ff_uchar 	ucFormat	转换格式
			 ff_uint 	iTime		时间戳
 			 
 			 格式值说明：
								1			MMDDHHMM
								2			MMDDHHMMSS
								3			YYMMDDHHMM
								4			YYMMDDHHMMSS
								5			YYYYMMDDHHMMSS
								6			YYYYMMDDHHMMSS000
								7			YYMMDDHHMMSS000R
 * 输    出: 
			 无
 * 返    回: 
			 
 * 备    注：
****************************************************************/
void TimeFormatStr2Int(ff_char *strInTime, ff_uchar ucFormat, ff_uint *uiOutTime)
{
	ff_time		tTmp;
	ff_tm 		tmTmp;
	ff_tm		*ptmTmp = NULL;
	ff_char  	strTmp[5];
	ff_char		strTime[20];

	memset(strTime, 0, sizeof(strTime));

	if (ucFormat == 1) /*MMDDHHMM*/
	{
		time(&tTmp);
		ptmTmp = localtime_r(&tTmp, &tmTmp);
		sprintf(strTime, "20%02d", ptmTmp->tm_year-100);
		memcpy(strTime+4, strInTime, 8);		
	}
	else if (ucFormat == 2) /*MMDDHHMMSS*/
	{
		time(&tTmp);
		ptmTmp = localtime_r(&tTmp, &tmTmp);
		sprintf(strTime, "20%02d", ptmTmp->tm_year-100);
		memcpy(strTime+4, strInTime, 10);
	}
	else if (ucFormat == 3)	/*YYMMDDHHMM*/
	{
		strcpy(strTime, "20");
		memcpy(strTime+2, strInTime, 10);
	}
	else if (ucFormat == 4) /*YYMMDDHHMMSS*/
	{
		strcpy(strTime, "20");
		memcpy(strTime+2, strInTime, 12);		
	}
	else if (ucFormat == 5) /*YYYYMMDDHHMMSS*/
	{
		memcpy(strTime, strInTime, 14);
	}
	else if (ucFormat == 6) /*YYYYMMDDHHMMSS000*/
	{
		memcpy(strTime, strInTime, 17);
	}
	else
	{
		strcpy(strTime, "20");
		memcpy(strTime+2, strInTime, 16);		
	}

	memset(&tmTmp, 0, sizeof(ff_tm));
	memset(strTmp, 0, sizeof(strTmp));/*yyyy*/
	strncpy(strTmp, strTime, 4);
	tmTmp.tm_year = atoi(strTmp)-1900;
	memset(strTmp, 0, sizeof(strTmp));/*mm*/
	strncpy(strTmp, strTime+4, 2);
	tmTmp.tm_mon  = atoi(strTmp)-1;
	memset(strTmp, 0, sizeof(strTmp));
	strncpy(strTmp, strTime+6, 2);/*dd*/
	tmTmp.tm_mday = atoi(strTmp);
	memset(strTmp, 0, sizeof(strTmp));
	strncpy(strTmp, strTime+8, 2);/*hh*/
	tmTmp.tm_hour = atoi(strTmp);
	memset(strTmp, 0, sizeof(strTmp));
	strncpy(strTmp, strTime+10, 2);/*mm*/
	tmTmp.tm_min  = atoi(strTmp);
	memset(strTmp, 0, sizeof(strTmp));
	strncpy(strTmp, strTime+12, 2);/*ss*/	
	tmTmp.tm_sec  = atoi(strTmp);
	tmTmp.tm_isdst = 0;
	tTmp = mktime(&tmTmp);
	if (tTmp <= 0)
	{
		*uiOutTime = 0;
	}
	else
	{
		*uiOutTime = tTmp;
	}
}

/****************************************************************
 * 名    称: FmtMsgIdAscIgw
 * 功    能: 将ASCII码格式化成互通网关格式或将返转
 * 输    入: 
 			 ff_char 	*strInMsgId
			 ff_int 	iInMsgIdLen
			 ff_int 	iType
			 ff_char 	*strOutMsgId
			 ff_int 	*iOutMsgIdLen
 			 
 			 格式值说明：
						0: ASCII码转成互通网关格式
						1: 互通网关格式转成ASCII码
 * 输    出: 
			 无
 * 返    回: 
			 
 * 备    注：
****************************************************************/
void FmtMsgIdAscIgw(ff_char *strInMsgId, ff_int iInMsgIdLen, ff_int iType, ff_char *strOutMsgId, ff_int *iOutMsgIdLen)
{
	ff_int				iTemp1 = 0;
	ff_int				iTemp2 = 0;
	ff_uint				iTemp3 = 0;
	ff_int				iSeq = 0;
	ff_int				iGwid = 0;
	
	ff_uchar			ucTmp1 = 0;
	ff_uchar			ucTmp2 = 0;
	ff_uchar			ucTmp3 = 0;
	
	ff_char 			strBuf[32];
	ff_char				strTmp[32];
	
	ff_tm				tmTmp;
	ff_tm 				*tmFormat;
	ff_time				tCurTime;
	

	if (iType == 0)
	{
		memset(strBuf, 0, sizeof(strBuf));
		memcpy(strBuf, strInMsgId, iInMsgIdLen);
		memset(strTmp, 0, sizeof(strTmp));
		memcpy(strTmp, strBuf+16, 5);
		iSeq = atoi(strTmp) % 65535;
		
		memset(strTmp, 0, sizeof(strTmp));
		memcpy(strTmp, strBuf+10, 6);
		iGwid = atoi(strTmp);
		
		memset(strTmp, 0, sizeof(strTmp));
		memcpy(strTmp, strBuf, 10);
		
		TimeFormatStr2Int(strTmp, 2, &iTemp3);
		tCurTime = iTemp3;
		
		tmFormat = localtime_r(&tCurTime, &tmTmp);
		sprintf(strTmp, "20%02d%02d%02d%02d%02d%02d", tmFormat->tm_year-100, tmFormat->tm_mon+1, tmFormat->tm_mday, tmFormat->tm_hour, tmFormat->tm_min, tmFormat->tm_sec);
		
		iTemp1 = 0;
		iTemp2 = 0;
		iTemp1 = tmFormat->tm_mon + 1;
		iTemp1 = ((iTemp1 << 5) & 0xffffffe0);
		iTemp1 = (iTemp1 |tmFormat->tm_mday);
		iTemp1 = ((iTemp1 << 5) & 0xffffffe0);
		iTemp1 = (iTemp1 |tmFormat->tm_hour);
		iTemp1 = ((iTemp1 << 6) & 0xffffffc0);
		iTemp1 = (iTemp1 |tmFormat->tm_min);
		iTemp1 = ((iTemp1 << 6) & 0xffffffc0);
		iTemp1 = (iTemp1 |tmFormat->tm_sec);
		iTemp1 = ((iTemp1 << 6) & 0xffffffc0);
		iTemp1 = (iTemp1 | ((iGwid>>16) & 0x0000003f));
		iTemp2 = iGwid;
		iTemp2 = ((iTemp2 << 16) & 0xffff0000);
		iTemp2 = (iTemp2 | iSeq);
		*iOutMsgIdLen = 8;

		/*linux加下面这两行,solaris则不要*/
		iTemp1 = htonl(iTemp1);
		iTemp2 = htonl(iTemp2);
		memset(strBuf, 0, sizeof(strBuf));
		memcpy(strBuf, &iTemp1, 4);
		memcpy(strBuf+4, &iTemp2, 4);
		memcpy(strOutMsgId, strBuf, 8);
	}
	else
	{
		memset(strTmp, 0, sizeof(strTmp));

		/*月*/
		ucTmp3 = strInMsgId[0];
		ucTmp3 = ucTmp3 >> 4;
		iTemp3 = (ff_uint)ucTmp3;
		sprintf(strTmp, "%02d", iTemp3);
		/*日*/
		ucTmp1 = strInMsgId[0];
		ucTmp2 = strInMsgId[1];
		ucTmp1 = (ucTmp1 & 0x0f) << 1;
		ucTmp2 = ucTmp2 >> 7;
		ucTmp3 = ucTmp1 | ucTmp2;
		iTemp3 = (ff_uint)ucTmp3;
		sprintf(strBuf, "%02d", iTemp3);
		strcat(strTmp, strBuf);
		/*时*/
		ucTmp1 = strInMsgId[1];
		ucTmp3 = (ucTmp1 & 0x7c) >> 2;
		iTemp3 = (ff_uint)ucTmp3;
		sprintf(strBuf, "%02d", iTemp3);
		strcat(strTmp, strBuf);
		/*分*/
		ucTmp1 = strInMsgId[1];
		ucTmp2 = strInMsgId[2];
		ucTmp1 = (ucTmp1 & 0x03) << 4;
		ucTmp2 = (ucTmp2 & 0xf0) >> 4;
		ucTmp3 = ucTmp1 | ucTmp2;
		iTemp3 = (ff_uint)ucTmp3;
		sprintf(strBuf, "%02d", iTemp3);
		strcat(strTmp, strBuf);
		/*秒*/
		ucTmp1 = strInMsgId[2];
		ucTmp2 = strInMsgId[3];
		ucTmp1 = (ucTmp1 & 0x0f) << 2;
		ucTmp2 = (ucTmp2 & 0xc0) >> 6;
		ucTmp3 = ucTmp1 | ucTmp2;
		iTemp3 = (ff_uint)ucTmp3;
		sprintf(strBuf, "%02d", iTemp3);
		strcat(strTmp, strBuf);
		/*网关ID*/
		ucTmp3 = strInMsgId[3];		
		ucTmp3 = ucTmp3 & 0x3f;		
		strBuf[0] = 0x00;
		strBuf[1] = ucTmp3;
		strBuf[2] = strInMsgId[4];
		strBuf[3] = strInMsgId[5];
		memcpy(&iTemp3, strBuf, 4);
		/*linux加下面这行,solaris则不要*/
		iTemp3 = ntohl(iTemp3);
		sprintf(strBuf, "%06d", iTemp3);
		strcat(strTmp, strBuf);
		/*流水号*/
		memcpy(strBuf, strInMsgId+6, 2);
		ucTmp1 = strBuf[0];
		ucTmp2 = strBuf[1];
		iTemp1 = (ff_uint)ucTmp1;
		iTemp2 = (ff_uint)ucTmp2;
		iTemp3 = iTemp1 * 256 + iTemp2;
		sprintf(strBuf, "%05d", iTemp3);
		strcat(strTmp, strBuf);
		
		memcpy(strOutMsgId, strTmp, 21);		
		*iOutMsgIdLen = 21;
	}

	return;
}
/****************************************************************
 * 名    称: ConvertCharToHex
 * 功    能: 将消息包转换成16进制码
 * 输    入: 
			 char 			*strDest	目的字符串
 			 const ff_char 	*strSrc		来源字符串
 			 ff_int 		iLen		来源字符串需进行转换的长度
 * 输    出: 
			 无
 * 返    回: 
			 
 * 备    注：
****************************************************************/
ff_char *ConvertCharToHex(char *strDest, const ff_char *strSrc, ff_int iLen)
{
	ff_uchar 		ucTmp;
	ff_char 		strTmp[3072];
	ff_char			strHex[4];
	ff_int			i = 0;

	if((NULL==strDest) || (NULL==strSrc))
	{
		return NULL;
	}
	
	memset(strTmp, 0, sizeof(strTmp));
	for (i=0; i<iLen; i++)
	{
		ucTmp = strSrc[i];
		if(((i + 1)%4) == 0)
		{
			sprintf(strHex, "%02x ", ucTmp);
	    }
		else
		{
			sprintf(strHex, "%02x", ucTmp);
		}
		strncat(strTmp, strHex, 3);
		
		if(((i+1)%24) == 0)
		{
			strcat(strTmp, "\n");
		}
	}

	sprintf(strDest, "%s", strTmp);
		
	return strDest;	
}

/****************************************************************
 * 名    称: ConvertBinary
 * 功    能: 将消息包转换成16进制码，带ASCII码显示功能
 * 输    入: 
			 ff_char 	*strDest		目的字符串
 			 ff_char 	*strSrc			来源字符串
 			 ff_int 	iLen			来源字符串需进行转换的长度
 * 输    出: 
			 无
 * 返    回: 
			 
 * 备    注：
****************************************************************/
ff_char *ConvertBinary(ff_char *strDest, ff_char *strSrc, ff_int iLen)
{
    ff_int				i = 0;
    ff_int				j = 0;
    ff_int				k = 0;
    ff_int				m = 0;
    ff_int				iTmp = 0;
    ff_char				*strRet = NULL;

	strRet = strDest;
	if (strDest == NULL)
	{
		return NULL;
	}

	if (iLen%16 != 0)
	{
		m = iLen / 16 + 1;
	}
	else
	{
		m = iLen / 16;
	}

	iTmp = sprintf(strDest, "\n==============================Message(Length=%-5d)==============================\n", iLen);
	strDest += iTmp;
	for (i=0; i<m; i++)
	{
		iTmp = sprintf(strDest, "%05d: ", i+1);
		strDest += iTmp;

		for (j=0; (j<16)&&(i*16+j<iLen); j++)
		{
			iTmp = sprintf(strDest, "%02X ", (unsigned char)strSrc[i*16+j]);
			strDest += iTmp;
			if ((j+1)%4 == 0)
			{
				iTmp = sprintf(strDest, "%s", " ");
				strDest += iTmp;
			}
		}

		if (j < 16)
		{
			for (k=0; k<16-j; k++)
			{
				iTmp = sprintf(strDest, "%s", "   ");
				strDest += iTmp;
				if ((j+k+1)%4 == 0)
				{
					iTmp = sprintf(strDest, "%s", " ");
					strDest += iTmp;
				}
			}
		}

		iTmp = sprintf(strDest, "%s", "| ");
		strDest += iTmp;

		for (j=0; (j<16)&&(i*16+j<iLen); j++)
		{
			if (isprint(strSrc[i*16+j]) == 0)
			{
				iTmp = sprintf(strDest, "%s", ".");
			}
			else
			{
				iTmp = sprintf(strDest, "%c", strSrc[i*16+j]);
			}
			strDest += iTmp;

			if ((j+1)%4 == 0)
			{
				iTmp = sprintf(strDest, "%s", " ");
				strDest += iTmp;
			}
		}

		iTmp = sprintf(strDest, "%s", "\n");
		strDest += iTmp;
	}

	iTmp = sprintf(strDest, "%s", "=================================================================================\n\n");
	strDest += iTmp;

	return strRet;
}

/****************************************************************
 * 名    称: CheckNumber
 * 功    能: 字符串中数字检测
 * 输    入: 
			 ff_char *strContent	被检测字符串
 * 输    出: 
			 无
 * 返    回: 
			 
 * 备    注：
****************************************************************/
ff_int CheckNumber(ff_char *strContent)
{
	ff_int		iLen = 0;
	ff_int		i;

	iLen = strlen(strContent);
	i = 0;
	for (i=0; i<iLen; i++)
	{
		if ((strContent[i]<'0') || (strContent[i]>'9'))
		{
			return -1;
		}
	}

	return 1;
}

/****************************************************************
 * 名    称: CharToHex
 * 功    能: 转换成16进制码
 * 输    入: 
			 char 			*strDest	目的字符串
 			 const ff_char 	*strSrc		来源字符串
 			 ff_int 		iLen		来源字符串需进行转换的长度
 * 输    出: 
			 无
 * 返    回: 
			 
 * 备    注：
****************************************************************/
ff_char *CharToHex(char *strDest, const ff_char *strSrc, ff_int iLen)
{
	ff_uchar 		ucTmp;
	ff_char 		strTmp[3072];
	ff_char			strHex[4];
	ff_int			i = 0;

	if((NULL==strDest) || (NULL==strSrc))
	{
		return NULL;
	}
	
	memset(strTmp, 0, sizeof(strTmp));
	for (i=0; i<iLen; i++)
	{
		ucTmp = strSrc[i];
		sprintf(strHex, "%02x", ucTmp);
		strncat(strTmp, strHex, 3);
	}

	sprintf(strDest, "%s", strTmp);
		
	return strDest;	
}

/****************************************************************
 * 名    称: HexToChar
 * 功    能: 转换成char
 * 输    入: 
			 char 			*strDest	目的字符串
 			 const ff_char 	*strSrc		来源字符串
 			 ff_int 		iLen		来源字符串需进行转换的长度
 * 输    出: 
			 无
 * 返    回: 
			 
 * 备    注：
****************************************************************/
ff_char *HexToChar(char *strDest, const ff_char *strSrc, ff_int iLen)
{
	ff_char 		strTmp[5];
	ff_char			strHex[4];
	ff_int			i = 0;

	if((NULL==strDest) || (NULL==strSrc))
	{
		return NULL;
	}
	
	for (i=0; i<iLen; i++)
	{
		memset(strTmp, 0, sizeof(strTmp));
		memset(strHex, 0, sizeof(strHex));
		strcpy(strHex, "0x");
		memcpy(strTmp, strSrc+i*2, 2);
		strcat(strHex, strTmp);

		strDest[i] = atoi(strHex);
	}
		
	return strDest;	
}

/***************************************************************
 * 名   称: CheckDirExist
 * 功   能：检查是否文件的路径存在，如果不存在，则创建它.            
 * 输   入: 
 			const char *strFileName		文件名
 			mode_t		iMode			权限设定           
 * 输   出: 
			无                               
 * 返   回: =1: 成功, -1: 失败
 * 备   注: 输入的文件名，如/data/wuxi/wuxi_20050205.txt；
            也可以是路径，但结尾必须要有/，如/data/wuxi/。
            输入的目录可以是相对目录,如../data/wuxi/wuxi_20050205.txt,
            或./data/wuxi/wuxi_20050205.txt;
            可以创建多级目录。
 **************************************************************/
ff_int CheckDirExist(const ff_char *strFileName, mode_t iMode)
{
	ff_char		strPath[1024+1];
	ff_char		strTmp1[1024+1];
	ff_char		strTmp2[1024+1];
	ff_char		strTmp3[1024+1];

	if (strFileName == NULL)
	{
		return -1;
	}

	if (strlen(strFileName) == 0)
	{
		return -1;
	}
	
	memset(strPath, 0, sizeof(strPath));
	memset(strTmp3, 0, sizeof(strTmp3));
	
	strncpy(strPath, strFileName, strchr(strFileName,'/')-strFileName);
	strcpy(strTmp3, strchr(strFileName,'/'));
	
	while (1)
	{
		memset(strTmp1, 0, sizeof(strTmp1));
		strcpy(strTmp1, strTmp3+1);  /*去掉第一个反斜杠*/
		if (strchr(strTmp1, '/') == NULL)  /*传入参数为带文件名的情况*/
		{
			break;
		}

		strcat(strPath, "/");
		memset(strTmp2, '\0', sizeof(strTmp2));
		strncpy(strTmp2, strTmp1, strlen(strTmp1)-strlen(strchr(strTmp1,'/')));
		strcat(strPath, strTmp2);

		if (mkdir(strPath, iMode) == -1)
		{
			if (errno != EEXIST)   /*除了目录已经存在的其他错误*/
			{
				return -2;
			}
		}
		memset(strTmp3, 0, sizeof(strTmp3));
		strcpy(strTmp3, strTmp1+strlen(strTmp2));  /*去掉前一级目录*/
		if (strlen(strTmp3) == 1)  /*只剩下最后一个反斜杠,传入参数为目录的情况*/
		{
			break;
		}
	}

	return 1;
}


