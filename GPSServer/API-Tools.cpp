/****************************************************************
 * �ļ�����: API-Tools.c                        
 * ��    ��: ���ߺ���                         	
 * ��    ע:									
 ***************************************************************/

/****************************************************************
 * ͷ�ļ�������
****************************************************************/
#include "API-Tools.h"
#include "FFCS_TypeDef.h"

/****************************************************************
 * ��    ��: FmtTime
 * ��    ��: ʱ���ʽ��Ϊ�ַ���
 * ��    ��: 
			 ff_time 	tTime			ʱ���
 			 ff_char 	*strFmt			��ʽ���ο�strftime����
 			 ff_char 	*strContent		��ʽ������ַ���
 			 ff_int 	iLen			����
 * ��    ��: 
			 *strContent			��ʽ���ַ���ָ��
 * ��    ��: 
			 *strContent			��ʽ���ַ���ָ��
 * ��    ע��
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
 * ��    ��: FmtNowTime
 * ��    ��: ����ǰʱ���ʽ��Ϊ�ַ���
 * ��    ��: 
			 ff_char 	*strFmt			��ʽ���ο�strftime����
 			 ff_char 	*strContent		��ʽ������ַ���
 			 ff_int 	iLen			����
 * ��    ��: 
			 *strContent			��ʽ���ַ���ָ��
 * ��    ��: 
			 *strContent			��ʽ���ַ���ָ��
 * ��    ע��
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
 * ��    ��: FmtNowAdjustTime
 * ��    ��: ��ǰʱ�������������ʽ��Ϊ�ַ���
 * ��    ��: 
			 ff_char 	*strFmt			��ʽ���ο�strftime����
 			 ff_char 	*strContent		��ʽ������ַ���
 			 ff_int 	iLen			����
 			 ff_int 	iAdtSec			���������������ɸ�
 * ��    ��: 
			 ��
 * ��    ��: 
			 
 * ��    ע��
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
 * ��    ��: RTrimRN
 * ��    ��: ȥ�һس�����
 * ��    ��: 
			 ff_char *strContent	�ַ���
 * ��    ��: 
			 ��
 * ��    ��: 
			 
 * ��    ע��
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
 * ��    ��: RTrim
 * ��    ��: ȥ�ҿո�
 * ��    ��: 
			 ff_char *strContent	�ַ���
 * ��    ��: 
			 ��
 * ��    ��: 
			 
 * ��    ע��
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
 * ��    ��: LTrim
 * ��    ��: ȥ��ո�
 * ��    ��: 
			 ff_char *strContent	�ַ���
 * ��    ��: 
			 ��
 * ��    ��: 
			 
 * ��    ע��
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
 * ��    ��: IntToBcd
 * ��    ��: ����תBCD��
 * ��    ��: 
			 ff_int iNum			����ֵ
 * ��    ��: 
			 ��
 * ��    ��: 
			 
 * ��    ע��
****************************************************************/
ff_char IntToBcd(ff_int iNum)
{
	return (ff_char)((iNum / 10) << 4) | (iNum % 10);
}

/****************************************************************
 * ��    ��: BcdToInt
 * ��    ��: BCD��ת����
 * ��    ��: 
			 ff_char cChar			
 * ��    ��: 
			 ��
 * ��    ��: 
			 
 * ��    ע��
****************************************************************/
ff_int BcdToInt (ff_char cChar)
{
	return (ff_int)(cChar >> 4) * 10 + (cChar & 0x0f);
}

/****************************************************************
 * ��    ��: AsciiToBcd
 * ��    ��: ASCII��תBCD��
 * ��    ��: 
			 ff_uchar 	*pucBcd
 			 ff_uchar 	*pucAscii
 			 ff_int 	iLen
 			 ff_uchar 	ucType
 * ��    ��: 
			 ��
 * ��    ��: 
			
 * ��    ע��
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

    if ((iLen&0x01) && (ucType))/*�б��Ƿ�Ϊ�����Լ����Ǳ߶���*/
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
 * ��    ��: BcdToAscii
 * ��    ��: BCD��תASCII��
 * ��    ��: 
			 ff_uchar 	*pucAscii
 			 ff_uchar 	*pucBcd
 			 ff_int 	iLen
 			 ff_uchar 	ucType
 * ��    ��: 
			 ��
 * ��    ��: 
			 
 * ��    ע��
****************************************************************/
void BcdToAscii(ff_uchar *pucBcd, ff_uchar *pucAscii, ff_int iLen, ff_uchar ucType)
{
	ff_int  iCnt = 0;
	
	if ((pucAscii==NULL) || (pucBcd==NULL))
	{
		return;
	}

	if ((iLen&0x01) && (ucType)) 	/*�б��Ƿ�Ϊ�����Լ����Ǳ߶���*/
	{                           		/*0��1��*/
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
 * ��    ��: IffS
 * ��    ��: PROC���ߺ���
 * ��    ��: 
			 const ff_int 	iFlag
 			 ff_char 		*strDest
 			 const ff_char 	*strSrc
 			 const ff_char 	*strDef
 * ��    ��: 
			 ��
 * ��    ��: 
			 
 * ��    ע��
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
 * ��    ��: IfS
 * ��    ��: PROC���ߺ���
 * ��    ��: 
			 const ff_int 	iFlag
 			 const ff_int 	iValue
 			 ff_char 		*strDest
 			 const ff_char 	*strSrc
 * ��    ��: 
			 ��
 * ��    ��: 
			 
 * ��    ע��
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
 * ��    ��: MSleep
 * ��    ��: ���ߣ���΢������
 * ��    ��: 
			 ff_int iMicroSecs	���ߵ�΢����
 * ��    ��: 
			 ��
 * ��    ��: 
			
 * ��    ע��
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
 * ��    ��: TimeFormatInt2Str
 * ��    ��: ʱ��ת�����ַ���
 * ��    ��: 
			 ff_int 	iTime		ʱ���
 			 ff_uchar 	ucFormat	ת����ʽ
 			 ff_char 	*strTime	ת�����ŵ��ַ���
 			 
 			 ��ʽֵ˵����
								1			MMDDHHMM
								2			MMDDHHMMSS
								3			YYMMDDHHMM
								4			YYMMDDHHMMSS
								5			YYYYMMDDHHMMSS
								6			YYYYMMDDHHMMSS000
								7			YYMMDDHHMMSS000R
 * ��    ��: 
			 ��
 * ��    ��: 
			 
 * ��    ע��
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
 * ��    ��: TimeFormatStr2Int
 * ��    ��: ʱ��ת�����ַ���
 * ��    ��: 
 			 ff_char 	*strTime	ת�����ŵ��ַ���
 			 ff_uchar 	ucFormat	ת����ʽ
			 ff_uint 	iTime		ʱ���
 			 
 			 ��ʽֵ˵����
								1			MMDDHHMM
								2			MMDDHHMMSS
								3			YYMMDDHHMM
								4			YYMMDDHHMMSS
								5			YYYYMMDDHHMMSS
								6			YYYYMMDDHHMMSS000
								7			YYMMDDHHMMSS000R
 * ��    ��: 
			 ��
 * ��    ��: 
			 
 * ��    ע��
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
 * ��    ��: FmtMsgIdAscIgw
 * ��    ��: ��ASCII���ʽ���ɻ�ͨ���ظ�ʽ�򽫷�ת
 * ��    ��: 
 			 ff_char 	*strInMsgId
			 ff_int 	iInMsgIdLen
			 ff_int 	iType
			 ff_char 	*strOutMsgId
			 ff_int 	*iOutMsgIdLen
 			 
 			 ��ʽֵ˵����
						0: ASCII��ת�ɻ�ͨ���ظ�ʽ
						1: ��ͨ���ظ�ʽת��ASCII��
 * ��    ��: 
			 ��
 * ��    ��: 
			 
 * ��    ע��
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

		/*linux������������,solaris��Ҫ*/
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

		/*��*/
		ucTmp3 = strInMsgId[0];
		ucTmp3 = ucTmp3 >> 4;
		iTemp3 = (ff_uint)ucTmp3;
		sprintf(strTmp, "%02d", iTemp3);
		/*��*/
		ucTmp1 = strInMsgId[0];
		ucTmp2 = strInMsgId[1];
		ucTmp1 = (ucTmp1 & 0x0f) << 1;
		ucTmp2 = ucTmp2 >> 7;
		ucTmp3 = ucTmp1 | ucTmp2;
		iTemp3 = (ff_uint)ucTmp3;
		sprintf(strBuf, "%02d", iTemp3);
		strcat(strTmp, strBuf);
		/*ʱ*/
		ucTmp1 = strInMsgId[1];
		ucTmp3 = (ucTmp1 & 0x7c) >> 2;
		iTemp3 = (ff_uint)ucTmp3;
		sprintf(strBuf, "%02d", iTemp3);
		strcat(strTmp, strBuf);
		/*��*/
		ucTmp1 = strInMsgId[1];
		ucTmp2 = strInMsgId[2];
		ucTmp1 = (ucTmp1 & 0x03) << 4;
		ucTmp2 = (ucTmp2 & 0xf0) >> 4;
		ucTmp3 = ucTmp1 | ucTmp2;
		iTemp3 = (ff_uint)ucTmp3;
		sprintf(strBuf, "%02d", iTemp3);
		strcat(strTmp, strBuf);
		/*��*/
		ucTmp1 = strInMsgId[2];
		ucTmp2 = strInMsgId[3];
		ucTmp1 = (ucTmp1 & 0x0f) << 2;
		ucTmp2 = (ucTmp2 & 0xc0) >> 6;
		ucTmp3 = ucTmp1 | ucTmp2;
		iTemp3 = (ff_uint)ucTmp3;
		sprintf(strBuf, "%02d", iTemp3);
		strcat(strTmp, strBuf);
		/*����ID*/
		ucTmp3 = strInMsgId[3];		
		ucTmp3 = ucTmp3 & 0x3f;		
		strBuf[0] = 0x00;
		strBuf[1] = ucTmp3;
		strBuf[2] = strInMsgId[4];
		strBuf[3] = strInMsgId[5];
		memcpy(&iTemp3, strBuf, 4);
		/*linux����������,solaris��Ҫ*/
		iTemp3 = ntohl(iTemp3);
		sprintf(strBuf, "%06d", iTemp3);
		strcat(strTmp, strBuf);
		/*��ˮ��*/
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
 * ��    ��: ConvertCharToHex
 * ��    ��: ����Ϣ��ת����16������
 * ��    ��: 
			 char 			*strDest	Ŀ���ַ���
 			 const ff_char 	*strSrc		��Դ�ַ���
 			 ff_int 		iLen		��Դ�ַ��������ת���ĳ���
 * ��    ��: 
			 ��
 * ��    ��: 
			 
 * ��    ע��
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
 * ��    ��: ConvertBinary
 * ��    ��: ����Ϣ��ת����16�����룬��ASCII����ʾ����
 * ��    ��: 
			 ff_char 	*strDest		Ŀ���ַ���
 			 ff_char 	*strSrc			��Դ�ַ���
 			 ff_int 	iLen			��Դ�ַ��������ת���ĳ���
 * ��    ��: 
			 ��
 * ��    ��: 
			 
 * ��    ע��
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
 * ��    ��: CheckNumber
 * ��    ��: �ַ��������ּ��
 * ��    ��: 
			 ff_char *strContent	������ַ���
 * ��    ��: 
			 ��
 * ��    ��: 
			 
 * ��    ע��
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
 * ��    ��: CharToHex
 * ��    ��: ת����16������
 * ��    ��: 
			 char 			*strDest	Ŀ���ַ���
 			 const ff_char 	*strSrc		��Դ�ַ���
 			 ff_int 		iLen		��Դ�ַ��������ת���ĳ���
 * ��    ��: 
			 ��
 * ��    ��: 
			 
 * ��    ע��
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
 * ��    ��: HexToChar
 * ��    ��: ת����char
 * ��    ��: 
			 char 			*strDest	Ŀ���ַ���
 			 const ff_char 	*strSrc		��Դ�ַ���
 			 ff_int 		iLen		��Դ�ַ��������ת���ĳ���
 * ��    ��: 
			 ��
 * ��    ��: 
			 
 * ��    ע��
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
 * ��   ��: CheckDirExist
 * ��   �ܣ�����Ƿ��ļ���·�����ڣ���������ڣ��򴴽���.            
 * ��   ��: 
 			const char *strFileName		�ļ���
 			mode_t		iMode			Ȩ���趨           
 * ��   ��: 
			��                               
 * ��   ��: =1: �ɹ�, -1: ʧ��
 * ��   ע: ������ļ�������/data/wuxi/wuxi_20050205.txt��
            Ҳ������·��������β����Ҫ��/����/data/wuxi/��
            �����Ŀ¼���������Ŀ¼,��../data/wuxi/wuxi_20050205.txt,
            ��./data/wuxi/wuxi_20050205.txt;
            ���Դ����༶Ŀ¼��
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
		strcpy(strTmp1, strTmp3+1);  /*ȥ����һ����б��*/
		if (strchr(strTmp1, '/') == NULL)  /*�������Ϊ���ļ��������*/
		{
			break;
		}

		strcat(strPath, "/");
		memset(strTmp2, '\0', sizeof(strTmp2));
		strncpy(strTmp2, strTmp1, strlen(strTmp1)-strlen(strchr(strTmp1,'/')));
		strcat(strPath, strTmp2);

		if (mkdir(strPath, iMode) == -1)
		{
			if (errno != EEXIST)   /*����Ŀ¼�Ѿ����ڵ���������*/
			{
				return -2;
			}
		}
		memset(strTmp3, 0, sizeof(strTmp3));
		strcpy(strTmp3, strTmp1+strlen(strTmp2));  /*ȥ��ǰһ��Ŀ¼*/
		if (strlen(strTmp3) == 1)  /*ֻʣ�����һ����б��,�������ΪĿ¼�����*/
		{
			break;
		}
	}

	return 1;
}


