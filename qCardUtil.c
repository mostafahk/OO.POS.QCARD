/*------------------------------------------------------------
* FileName: qCardUtil.c
* Author: m.rahimi
* Date: 2019-07-17
------------------------------------------------------------*/

#include "define.h"
#include <Utility/utility.h>
#include "qCardUtil.h"

DWORD qCardUtilGetEpochDateTime(DWORD i_dwDate,DWORD i_dwTime)
{
	struct tm t;
	time_t time;
    t.tm_year = utlGetYear(i_dwDate)-1900;
    t.tm_mon  = utlGetMonth(i_dwDate) - 1;           // Month, 0 - jan
    t.tm_mday = utlGetDay(i_dwDate);          // Day of the month
    t.tm_hour = utlGetHour(i_dwTime);
    t.tm_min = utlGetMinute(i_dwTime);
    t.tm_sec = utlGetSecond(i_dwTime);
    t.tm_isdst = -1;        // Is DST on? 1 = yes, 0 = no, -1 = unknown
    time = mktime(&t);

    return time;
}

WORD qCardUtilGetEpochDate(DWORD i_dwDate)
{
	struct tm t;
	time_t time;
	WORD	dwTemp	=	0;
    t.tm_year = utlGetYear(i_dwDate)-1900;
    t.tm_mon  = utlGetMonth(i_dwDate) - 1;           // Month, 0 - jan
    t.tm_mday = utlGetDay(i_dwDate);          // Day of the month

    t.tm_isdst = -1;        // Is DST on? 1 = yes, 0 = no, -1 = unknown
    time = mktime(&t);

    dwTemp = ((time & 0xff000000) << 8) | (time & 0x00ff0000);
    return dwTemp;
}


int qCardUtilGetTimeStr(time_t rawtime, char* dateTimeStr)
{
	struct tm  ts;
	ts = *localtime(&rawtime);

	ST_TIME time;
	time.Day=ts.tm_mday;
	time.Hour=ts.tm_hour;
	time.Minute=ts.tm_min;
	time.Month=ts.tm_mon+1;
	time.Second=ts.tm_sec;
	time.Year=ts.tm_year+1900;


	DWORD	dwMiladiDate = 0;
	DWORD	dwShamsiDate = 0;

	dwMiladiDate = utlSetDate(time.Year, time.Month, time.Day);
	utlMiladi2Shamsi(dwMiladiDate, &dwShamsiDate);

	sprintf(dateTimeStr, "%02d:%02d:%02d - %04d/%02d/%02d",ts.tm_sec,time.Minute, time.Hour,
			utlGetYear(dwShamsiDate), utlGetMonth(dwShamsiDate), utlGetDay(dwShamsiDate));

	return 0;

}
