/*------------------------------------------------------------
* FileName: qCardUtil.h
* Author: m.rahimi
* Date: 2019-07-17
------------------------------------------------------------*/
#ifndef QCARDUTIL_H
#define QCARDUTIL_H




DWORD qCardUtilGetEpochDateTime(DWORD i_dwDate,DWORD i_dwTime);
int qCardUtilGetTimeStr(time_t rawtime, char* dateTimeStr);

#endif
