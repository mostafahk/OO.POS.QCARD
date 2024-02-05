/*
 * sohaTrnSettFile.c
 *
 *  Created on: May 20, 2019
 *      Author: f.karamiyar
 */
#include <define.h>


#ifndef T610_DEV
#include <DeviceMng/dmgFile.h>
#include "qCardFileOperationWrapper.h"
#include "sohaTrnSettFile.h"
#else
#include <ui/uiMessage.h>
#include <ui/uiDisplay.h>
#include <qCardFileOperationWrapper.h>
#include <sohaTrnSettFile.h>
#include <QCardWrapper/qCardMessagewrapper.h>
#include <QCardWrapper/qCardWrapperFunctions.h>
#endif

#ifdef S900_DEV
#include "DesfireSam/sSam.h"
#endif

SohaSettConfig	gs_sSettConfig;
#ifdef T610_DEV
extern BYTE	gs_byInitWirelessModulet;
#endif



static BYTE gs_bySohaAutoSendFileResult = FALSE;
static BYTE gs_bySohaOpenFileAccessBlocked = FALSE;
static BYTE gs_bySohaOpenFileAccessFailed = FALSE;
static BYTE gs_bySohaInternalLock = FALSE;


DWORD gs_dwSohaOldTrnsCount = 0;
DWORD gs_dwSohaLoadSettConfigFlag	=	FALSE;


void SohaSettSetAutoSendFileResult(BYTE i_byStatus)
{
	gs_bySohaAutoSendFileResult = i_byStatus;
}


BYTE SohaSettGetAutoSendFileResult()
{
	return gs_bySohaAutoSendFileResult;
}


void SohaSettBlockOpenFileAccess()
{
	if(gs_bySohaInternalLock)
	{
		uiDisplayMsgWithId(MESSAGE_PLEASE_WAIT, UI_DISPLAY_WAIT_TYPE_NO_WAIT, 0);
		while(gs_bySohaInternalLock);
	}

	gs_bySohaOpenFileAccessBlocked = TRUE;
}


void SohaSettReleaseOpenFileAccess()
{
	gs_bySohaOpenFileAccessBlocked = FALSE;
}


BYTE SohaSettGetOpenFileAccessBlockStatus()
{
	return gs_bySohaOpenFileAccessBlocked;
}


void SohaSettSetOpenFileAccessFailed()
{
	gs_bySohaOpenFileAccessFailed = TRUE;
}


void SohaSettResetOpenFileAccessFailed()
{
	gs_bySohaOpenFileAccessFailed = FALSE;
}


BYTE SohaSettGetOpenFileAccessFailedStatus()
{
	return gs_bySohaOpenFileAccessFailed;
}


BYTE SohaSettArchiveFile(char* i_szFilename, char* i_szArchivedFile)
{
	HFILE	hFile							=	0;
	BZFILE* hBzFile							=	0;
	DWORD	dwFileSize						=	0;
	DWORD	dwIndex							=	0;
	WORD	wLen							=	0;
	char	szArchiveFilename[20]			=	{0};
	BYTE	byaBuffer[SOHA_SETT_SEGMENT_SIZE]	=	{0};

	strcpy(szArchiveFilename, i_szArchivedFile);
	utlMakeUpperStr(szArchiveFilename);
	OsLog(LOG_DEBUG,"\n1 -- %s -- %s --\n",i_szFilename,i_szArchivedFile);
    dwFileSize = qDmgFile_GetSize(i_szFilename);
	if(dwFileSize <= 0)
		return FALSE;
	else if(dwFileSize > SOHA_SETT_MAX_FILE_SIZE_FOR_ZIP){
		qDmgFile_Rename(i_szFilename,i_szArchivedFile);
		return TRUE;
	}

    if(!qDmgFile_Open(i_szFilename, DMG_FILE_OPEN_MODE_READ, &hFile))
		return FALSE;
	hBzFile = BZ2_bzopen(szArchiveFilename, "w");
	if(!hBzFile)
	{
        qDmgFile_Close(hFile);
		return FALSE;
	}
	while(dwIndex < dwFileSize)
	{
        qDmgFile_Seek(hFile, dwIndex, DMG_FILE_SEEK_SET);
		wLen = (dwFileSize - dwIndex > SOHA_SETT_SEGMENT_SIZE) ? SOHA_SETT_SEGMENT_SIZE : (WORD)(dwFileSize - dwIndex);
		dwIndex += SOHA_SETT_SEGMENT_SIZE;
        qDmgFile_Read(hFile, byaBuffer, &wLen);
		if(BZ2_bzwrite(hBzFile, byaBuffer, wLen) == -1){
			break;
		}


	}

    qDmgFile_Close(hFile);
	BZ2_bzclose(hBzFile);

	return TRUE;
}


BYTE SohaSettCheckArchivedFileTable(char *i_szFileName)
{
	BYTE	byFound		=	FALSE;
	HFILE	hFile		=	0;
	WORD	wLen		=	0;
	DWORD	dwCounter	=	0;
	DWORD	dwFileSize	=	0;
	sohaSettArchivedFile	sArchivedFile;

	memset(&sArchivedFile, 0, sizeof(sohaSettArchivedFile));

    dwFileSize = qDmgFile_GetSize(SOHA_SETT_ARCHIVE_TABLE_FILE_NAME);
	if(dwFileSize == 0)
		return FALSE;

    if(!qDmgFile_Open(SOHA_SETT_ARCHIVE_TABLE_FILE_NAME, DMG_FILE_OPEN_MODE_READ, &hFile))
		return FALSE;

	for(dwCounter = 0; dwCounter < dwFileSize / sizeof(sohaSettArchivedFile); dwCounter++)
	{
		memset(&sArchivedFile, 0, sizeof(sohaSettArchivedFile));

        qDmgFile_Seek(hFile, dwCounter * sizeof(sohaSettArchivedFile), DMG_FILE_SEEK_SET);
		wLen = sizeof(sohaSettArchivedFile);
        if(!qDmgFile_Read(hFile, (BYTE*)&sArchivedFile, &wLen))
			break;

		if(strcmp(sArchivedFile.m_szOrgFileName, i_szFileName) == 0)
		{
			byFound = TRUE;
			break;
		}
	}

    qDmgFile_Close(hFile);

	return byFound;
}

#ifdef T610_DEV
BYTE SohaSettAddFileToArchiveFileTable(char* i_szFileName)
{

    HFILE	hFile		=	0;
    DWORD	dwRetVal	=	0;
    sohaSettArchivedFile	sArchivedFile;
    memset(&sArchivedFile, 0, sizeof(sohaSettArchivedFile));

    BYTE i_szFilename[100]={0};
    sprintf(i_szFilename,"%s",SOHA_SETT_ARCHIVE_TABLE_FILE_NAME);


    strcpy(sArchivedFile.m_szOrgFileName, i_szFileName);


    if(SohaSettCheckArchivedFileTable(i_szFileName))
        return TRUE;

    Lcdprintf("filename**: %s  %d",sArchivedFile.m_szOrgFileName, strlen(sArchivedFile.m_szOrgFileName));getkey();
    Lcdprintf("size: %d",dmgFile_GetSize(i_szFilename));getkey();

    dwRetVal = dmgFile_Open(i_szFilename, DMG_FILE_OPEN_MODE_UPDATE, &hFile);
    if(dwRetVal != SUCCESSFUL)
        return FALSE;

    dmgFile_Seek(hFile, 0, DMG_FILE_SEEK_END);

    dwRetVal = dmgFile_Write(hFile, (BYTE*)&sArchivedFile, sizeof(sohaSettArchivedFile));
    dmgFile_Close(hFile);
    if(dwRetVal != SUCCESSFUL)
        return FALSE;

    Lcdprintf("size: %d",dmgFile_GetSize(i_szFilename));getkey();

    return TRUE;
}
#endif



#ifndef T610_DEV
BYTE SohaSettAddFileToArchiveFileTable(char* i_szFileName)
{
	HFILE	hFile		=	0;
	sohaSettArchivedFile	sArchivedFile;

	memset(&sArchivedFile, 0, sizeof(sohaSettArchivedFile));

	strcpy(sArchivedFile.m_szOrgFileName, i_szFileName);

	if(SohaSettCheckArchivedFileTable(i_szFileName))
		return TRUE;

	if(dmgFile_GetSize(SOHA_SETT_ARCHIVE_TABLE_FILE_NAME) == 0)
	{
		if(!dmgFile_Open(SOHA_SETT_ARCHIVE_TABLE_FILE_NAME, DMG_FILE_OPEN_MODE_CREATE, &hFile))
			return FALSE;
	}
	else
	{
		if(!dmgFile_Open(SOHA_SETT_ARCHIVE_TABLE_FILE_NAME, DMG_FILE_OPEN_MODE_UPDATE, &hFile))
			return FALSE;
	}

	dmgFile_Seek(hFile, 0, DMG_FILE_SEEK_END);

	if(!dmgFile_Write(hFile, (BYTE*)&sArchivedFile, sizeof(sohaSettArchivedFile)))
	{
		dmgFile_Close(hFile);
		return FALSE;
	}

	dmgFile_Close(hFile);


	return TRUE;
}
#endif

BYTE SohaSettRemoveFileFromArchiveTable(char *i_szFileName)
{
	HFILE	hFile		=	0;
	HFILE	hFileTemp	=	0;
	WORD	wLen		=	0;
	DWORD	dwCounter	=	0;
	DWORD	dwFileSize	=	0;
	sohaSettArchivedFile	sArchiveFile;

	memset(&sArchiveFile, 0, sizeof(sohaSettArchivedFile));

    dwFileSize = qDmgFile_GetSize(SOHA_SETT_ARCHIVE_TABLE_FILE_NAME);
	if(dwFileSize == 0)
		return FALSE;

	if(!SohaSettCheckArchivedFileTable(i_szFileName))
		return TRUE;

    if(!qDmgFile_Open(SOHA_SETT_ARCHIVE_TABLE_FILE_NAME, DMG_FILE_OPEN_MODE_READ, &hFile))
		return FALSE;


			BYTE	byOpenFileMode	=	0;
    dwFileSize = qDmgFile_GetSize("TEMP.DAT");

			if(dwFileSize == 0)
				byOpenFileMode = DMG_FILE_OPEN_MODE_CREATE;
			else
				byOpenFileMode = DMG_FILE_OPEN_MODE_UPDATE;

    if(!qDmgFile_Open("TEMP.DAT", byOpenFileMode, &hFileTemp))
	{
        qDmgFile_Close(hFile);
		return FALSE;
	}

	for(dwCounter = 0; dwCounter < dwFileSize / sizeof(sohaSettArchivedFile); dwCounter++)
	{
		memset(&sArchiveFile, 0, sizeof(sohaSettArchivedFile));
        qDmgFile_Seek(hFile, dwCounter * sizeof(sohaSettArchivedFile), DMG_FILE_SEEK_SET);
		wLen = sizeof(sohaSettArchivedFile);
        if(!qDmgFile_Read(hFile, (BYTE*)&sArchiveFile, &wLen))
		{
            qDmgFile_Close(hFile);
            qDmgFile_Close(hFileTemp);
            qDmgFile_Remove("TEMP");
			return FALSE;
		}

		if(strcmp(sArchiveFile.m_szOrgFileName, i_szFileName) != 0)
		{
            qDmgFile_Seek(hFileTemp, 0, DMG_FILE_SEEK_END);
            if(!qDmgFile_Write(hFileTemp, (BYTE*)&sArchiveFile, sizeof(sohaSettArchivedFile)))
			{
                qDmgFile_Close(hFile);
                qDmgFile_Close(hFileTemp);
                qDmgFile_Remove("TEMP");
				return FALSE;
			}
		}
	}

    qDmgFile_Close(hFile);
    qDmgFile_Close(hFileTemp);

    if(qDmgFile_GetSize("TEMP.DAT") <= 0)
        qDmgFile_Remove(SOHA_SETT_ARCHIVE_TABLE_FILE_NAME);
	else
        qDmgFile_Rename("TEMP.DAT", SOHA_SETT_ARCHIVE_TABLE_FILE_NAME);

	return TRUE;
}


BYTE SohaSettGetConfigFromFile(SohaSettConfig* o_sSettConfig)
{
	WORD	wLen		=	0;
	HFILE	hFile		=	0;
	BYTE	byRet		=	FALSE;
	BYTE	byRetVal	= 0;

	memset(o_sSettConfig, 0, sizeof(SohaSettConfig));

    if(!qDmgFile_Open(SOHA_SETT_CONFIG_FILE_NAME, DMG_FILE_OPEN_MODE_READ, &hFile))
		return FALSE;

    qDmgFile_Seek(hFile, 0, DMG_FILE_SEEK_SET);
	wLen = sizeof(SohaSettConfig);
    byRetVal = qDmgFile_Read(hFile, (BYTE*)o_sSettConfig, &wLen);
    qDmgFile_Close(hFile);
	if(byRetVal)
		byRet = TRUE;
	else
		byRet = FALSE;

	return byRet;
}


BYTE SohaSettLoadConfig(void)
{

	BYTE	byRetVal	=	0;
	SohaSettConfig	sSettConfig;

	memset(&sSettConfig, 0, sizeof(SohaSettConfig));

	if(!gs_dwSohaLoadSettConfigFlag)
	{

        if(qDmgFile_GetSize(SOHA_SETT_CONFIG_FILE_NAME) <= 0)
		{
			sSettConfig.m_dwTrnCounter = 1;
			sSettConfig.m_dwTrnFileCounter = 1;
			byRetVal =  SohaSettUpdateConfig(&sSettConfig);

		}
		else
		{
			byRetVal = SohaSettGetConfigFromFile(&sSettConfig);
		}

		if(byRetVal)
		{
			memcpy(&gs_sSettConfig, &sSettConfig, sizeof(SohaSettConfig));
			gs_dwSohaLoadSettConfigFlag = TRUE;
		}
	}

	return gs_dwSohaLoadSettConfigFlag;
}


BYTE SohaSettGetConfig(SohaSettConfig* o_sSettConfig)
{
    OsLog(LOG_DEBUG,"SohaSettGetConfig BEGIN>>gs_sSettConfig:%d\n",gs_sSettConfig.m_dwTrnCounter);


	if(!SohaSettLoadConfig())
		return FALSE;
	memcpy(o_sSettConfig, &gs_sSettConfig, sizeof(SohaSettConfig));

	return TRUE;
}


BYTE SohaSettUpdateConfig(SohaSettConfig* i_sSettConfig)
{
	WORD	wLen		=	0;
	HFILE	hFile		=	0;
	BYTE	byRet		=	FALSE;
	BYTE	byRetVal	=	0;

    if(qDmgFile_GetSize(SOHA_SETT_CONFIG_FILE_NAME) == 0)
	{
        if(!qDmgFile_Open(SOHA_SETT_CONFIG_FILE_NAME, DMG_FILE_OPEN_MODE_CREATE, &hFile))
			return FALSE;
	}
	else
	{

        if(!qDmgFile_Open(SOHA_SETT_CONFIG_FILE_NAME, DMG_FILE_OPEN_MODE_UPDATE, &hFile))
			return FALSE;

	}

    qDmgFile_Seek(hFile, 0, DMG_FILE_SEEK_SET);
	wLen = sizeof(SohaSettConfig);
    byRetVal = qDmgFile_Write(hFile, (BYTE*)i_sSettConfig, wLen);
    qDmgFile_Close(hFile);
	if(byRetVal)
	{
		byRet = TRUE;


	}
	else
		byRet = FALSE;

	memcpy(&gs_sSettConfig, i_sSettConfig, sizeof(SohaSettConfig));
	gs_dwSohaLoadSettConfigFlag = TRUE;


	return byRet;
}


BYTE SohaSettIncTrnCounter(void)
{
	SohaSettConfig	sSettconfig;
	memset(&sSettconfig, 0, sizeof(SohaSettConfig));

	if(!SohaSettGetConfig(&sSettconfig))
		return FALSE;

	if(sSettconfig.m_dwTrnCounter < 999999)
	{
		sSettconfig.m_dwTrnCounter++;


	}
	else
	{
		sSettconfig.m_dwTrnCounter = 1;

	}

	return SohaSettUpdateConfig(&sSettconfig);
}


BYTE SohaSettDecTrnCounter(void)
{

	SohaSettConfig	sSettconfig;

	memset(&sSettconfig, 0, sizeof(SohaSettConfig));

	if(!SohaSettGetConfig(&sSettconfig))
		return FALSE;

	if(sSettconfig.m_dwTrnCounter > 1)
	{
		sSettconfig.m_dwTrnCounter--;

	}
	else
	{
		sSettconfig.m_dwTrnCounter = 999999;

	}

	return SohaSettUpdateConfig(&sSettconfig);
}


BYTE SohaSettIncTrnFileCounter(void)
{

	SohaSettConfig	sSettconfig;

	memset(&sSettconfig, 0, sizeof(SohaSettConfig));

	if(!SohaSettGetConfig(&sSettconfig))
		return FALSE;

	if(sSettconfig.m_dwTrnFileCounter < 999)
		sSettconfig.m_dwTrnFileCounter++;
	else
		sSettconfig.m_dwTrnFileCounter = 1;


	return SohaSettUpdateConfig(&sSettconfig);
}


BYTE SohaSettDecTrnFileCounter(void)
{

	SohaSettConfig	sSettconfig;

	memset(&sSettconfig, 0, sizeof(SohaSettConfig));

	if(!SohaSettGetConfig(&sSettconfig))
		return FALSE;

	if(sSettconfig.m_dwTrnFileCounter > 1)
		sSettconfig.m_dwTrnFileCounter--;
	else
		sSettconfig.m_dwTrnFileCounter = 999;



	return SohaSettUpdateConfig(&sSettconfig);
}


BYTE SohaSettOpenFileIsFull(char* i_szFileName)
{
    if(qDmgFile_GetSize(i_szFileName) > SOHA_SETT_MAX_FILE_SIZE_FOR_STORE)
		return TRUE;

	return FALSE;
}


BYTE SohaSettGetArchiveFileName(char* i_szFileName, char* o_szArchiveFileName)
{
	BYTE byaDateTimeStr[15] = {0};
	BYTE byaBuffer[1024] = {0};
	WORD wLen =	0;
	HFILE hFile = 0;

    if(!qDmgFile_Open(i_szFileName, DMG_FILE_OPEN_MODE_READ, &hFile))
		return FALSE;

    qDmgFile_Seek(hFile, 0, DMG_FILE_SEEK_SET);
	wLen = SOHA_SETT_TRN_FILE_HEADER_LEN;
    if(!qDmgFile_Read(hFile, (BYTE*)byaBuffer, &wLen))
	{
        qDmgFile_Close(hFile);
		return FALSE;
	}

    qDmgFile_Close(hFile);

    strncpy((char*)byaDateTimeStr, (char*)byaBuffer + SOHA_SETT_HEADER_DATE_TIME_INDEX + 1, 14);


    if(qDmgFile_GetSize(i_szFileName) > SOHA_SETT_MAX_FILE_SIZE_FOR_ZIP)
    	sprintf(o_szArchiveFileName, "%s.uz", byaDateTimeStr);
    else
    	sprintf(o_szArchiveFileName, "%s.BZ2", byaDateTimeStr);

	return TRUE;
}


BYTE SohaSettArchiveOpenFile( char* o_szArchiveName)
{
	char	szFileName[50]			=	{0};
	char	szReserveFileName[50]	=	{0};
	char	szArchiveFileName[20]	=	{0};
	HFILE	hFile					=	0;


	strcpy(szFileName, SOHA_SETT_OPEN_TRN_FILE_NAME);
	strcpy(szReserveFileName, SOHA_SETT_RESERVE_TRN_FILE_NAME);


    if(qDmgFile_GetSize(szFileName) <= 0)
		return TRUE;

	if(!SohaSettGetArchiveFileName(szFileName, szArchiveFileName))
		return FALSE;

//	if(!SohaSettAddFileToArchiveFileTable(szArchiveFileName))
//		return FALSE;


    qDmgFile_Remove(szReserveFileName);

    qDmgFile_Remove(szArchiveFileName);

	if(!SohaSettArchiveFile(szFileName, szArchiveFileName))
	{
        qDmgFile_Remove(szArchiveFileName);
		SohaSettRemoveFileFromArchiveTable(szArchiveFileName);
        if(qDmgFile_Open(szReserveFileName, DMG_FILE_OPEN_MODE_CREATE, &hFile))
            qDmgFile_Close(hFile);
		return FALSE;
	}

    qDmgFile_Remove(szFileName);


	SohaSettIncTrnFileCounter();

	if(o_szArchiveName)
		strcpy(o_szArchiveName, szArchiveFileName);

	return TRUE;
}


BYTE SohaSettAddTrnToOpenFile(SohaSettProductInfo* i_sProductInfo, SohaSettTrnInfo* i_sTrnInfo)
{

	SohaSettConfig sSettConfig;
	memset(&sSettConfig, 0, sizeof(SohaSettConfig));


	char szDateStr[9] = {0};
	char szTimeStr[7] = {0};

	BYTE byLoopCounter = 0;
	BYTE byaTerminalSn[33] = {0};
	BYTE byaSendTerminalSn[33] = {0};    //Last 8 digit of Device Serial#


	//Header
	BYTE byaReqestId[24] = {0};
	DWORD dwStationId = 0;
	char szTerminalId[11] = { 0 };
	char szMerchantId[11] = { 0 };
	BYTE byaSimCardId[36] = {0};
	BYTE bySwitchType = g_stGlVar.m_sTerminalConfig.m_bySwitchVersion;
	DWORD dwCityId = 0;
	BYTE byaTrnSignature[33] = {0};
	BYTE byaAgentType[5] = {0};

	//Body
	BYTE byaDriverCode[13] = {0};
	BYTE byaRouteCode[13] = {0};
	DWORD dwDeviceCreditAmount = 0;
	BYTE byaPhoneNumber[12] = {0};
	BYTE byaLatitude[21] = {0};
	BYTE byaLongitude[21] = {0};


	BYTE byaSettFileHeader[1024] = {0};
	BYTE byaTrnSettRecord[1024] = {0};

	BYTE byaPaymentInfo[21] = {0};
	BYTE byaCardSerial[21] = {0};
	BYTE byaSellerId[18]={0};
	WORD wLen = 0;
	DWORD dwCurrentTxnsCount = 0;
	DWORD dwFileSize = 0;
	HFILE hFile = 0;


	BYTE byaSimID[21]	=	{0};
	BYTE byaSimIMSI[21] = {0};
	BYTE byaExpireDate[9] = {0};
	BYTE byaDiscountExpireDate[9] = {0};
    char byaTrackingCode[13] = {0};



	if(i_sProductInfo->m_wDiscountExpireDate != 0)
	{
#ifndef T610_DEV
		time_t t_discountExpDateTime = 0;
		struct tm  ts;
		t_discountExpDateTime = i_sProductInfo->m_wDiscountExpireDate;
		t_discountExpDateTime = t_discountExpDateTime<<16;
		ts = *localtime(&t_discountExpDateTime);
		sprintf((char*)byaDiscountExpireDate,"%04d%02d%02d",1900 + ts.tm_year,ts.tm_mon,ts.tm_mday);
#else
        DWORD dwStartingPoint = utlSetDate(1900, 1 , 1 );
        DWORD dwDays = i_sProductInfo->m_wDiscountExpireDate  - ((i_sProductInfo->m_wDiscountExpireDate/365)/4  + 1 );
        DWORD dwTheDate = utlSetNextDate(dwStartingPoint, 0, 0, dwDays);
        sprintf((char*)byaDiscountExpireDate,"%04d%02d%02d", utlGetYear(dwTheDate),utlGetMonth(dwTheDate),utlGetDay(dwTheDate));
#endif
	}
	utlPadWithZero(byaDiscountExpireDate,8);

	if(!SohaSettGetConfig(&sSettConfig))
		return FALSE;

	OsLog(LOG_DEBUG,"\n\n\___________SOHA_SETT_CONFIG_FILE_NAME_::_DEVICE TRN NUM:%d____________  \n\n\n",sSettConfig.m_dwTrnCounter);

	sprintf((char*)szDateStr, "%04d%02d%02d",
		utlGetYear(i_sTrnInfo->m_dwDate), utlGetMonth(i_sTrnInfo->m_dwDate), utlGetDay(i_sTrnInfo->m_dwDate));

	sprintf((char*)szTimeStr, "%02d%02d%02d",
		utlGetHour(i_sTrnInfo->m_dwTime), utlGetMinute(i_sTrnInfo->m_dwTime), utlGetSecond(i_sTrnInfo->m_dwTime));

	dwStationId = g_stGlVar.m_sTerminalConfig.m_dwStationId;

#ifndef T610_DEV
	long int iTerminalIdTemp = atol((char*)g_stGlVar.m_sTerminalLocalConfig.m_byaTerminalId);
	for (byLoopCounter = 0; byLoopCounter < (sizeof(szTerminalId) - 1); byLoopCounter++)
	{
		if (iTerminalIdTemp != 0)
		{
			szTerminalId[10 - byLoopCounter - 1] = (iTerminalIdTemp % 10) + '0';
			iTerminalIdTemp /= 10;
		}
		else
		{
			szTerminalId[10 - byLoopCounter - 1] = '0';
		}
	}

	long int iMerchantIdTemp = atol((char*)g_stGlVar.m_MerchantLocalCfg.m_byaMrchntID);
	for (byLoopCounter = 0; byLoopCounter < (sizeof(szMerchantId) - 1); byLoopCounter++)
	{
		if (iMerchantIdTemp != 0)
		{
			szMerchantId[10 - byLoopCounter - 1] = (iMerchantIdTemp % 10) + '0';
			iMerchantIdTemp /= 10;
		}
		else
		{
			szMerchantId[10 - byLoopCounter - 1] = '0';
		}
	}
#else T610_DEV
    BYTE	byaComponentId[8]		=	{0};
    desfireSamRetrieveUid(byaComponentId);
#endif

	dmgBasic_GetTerminalSerialNumber((char*)byaTerminalSn);
	strcpy((char*)byaSendTerminalSn, (char*)byaTerminalSn + (strlen((char*)byaTerminalSn) - 8));
	utlPadWithZero(byaSendTerminalSn,10);

	sprintf(byaReqestId,"%s%s%s",byaSendTerminalSn,szDateStr,szTimeStr);
	utlPadWithZero(byaReqestId, 23);


//	utlPadWithZero(byaFaExpireDate, 8);
    dwFileSize = qDmgFile_GetSize(SOHA_SETT_OPEN_TRN_FILE_NAME);
	OsLog(LOG_DEBUG, "\n@SohaTrnFile.c > SettAddTrnToOpenFile > dwFileSize: %d\n", dwFileSize);

	if(dwFileSize == 0)
	{
        if(!qDmgFile_Open(SOHA_SETT_OPEN_TRN_FILE_NAME, DMG_FILE_OPEN_MODE_CREATE, &hFile))
			return FALSE;
		OsLog(LOG_DEBUG, "\n@SohaTrnFile.c > SohaSettAddTrnToOpenFile > TRN FILE created successfully.\n");

	}
	else
	{
        if(!qDmgFile_Open(SOHA_SETT_OPEN_TRN_FILE_NAME, DMG_FILE_OPEN_MODE_UPDATE, &hFile))
			return FALSE;
		OsLog(LOG_DEBUG, "\n@SohaTrnFile.c > SohaSettAddTrnToOpenFile > TRN FILE opened successfully.\n");

	}
	strcpy((char*)byaSimID,"00000000000000000000");   //TODO : fix
	strcpy((char*)byaSimIMSI,"000000000000000");

	sprintf((char*)byaSimCardId,"%s%s",byaSimID,byaSimIMSI);
	utlPadWithZero(byaSimCardId, 35);

	sprintf((char*)byaPaymentInfo,"%s",i_sTrnInfo->m_byaPaymentInfo);
	utlPadWithZero(byaPaymentInfo, 20);

	sprintf((char*)byaCardSerial,"%s",i_sProductInfo->m_byaSerial);
	utlPadWithZero(byaCardSerial, 20);


//	strcpy((char*)byaSellerId, (char*)i_sTrnInfo->m_byaSellerId);
	sprintf((char*)byaSellerId,"%s",i_sTrnInfo->m_byaSellerId);


	utlPadWithZero(byaSellerId, 17);

    strncpy((char*)byaTrackingCode, (char*)i_sTrnInfo->m_byaTrackingCode, 12);
    utlPadWithZero(byaTrackingCode, 12);

#ifdef S900_DEV
	if (strcmp((char *)i_sTrnInfo->m_byaTransactionType, "901411") == 0)
		strcpy((char*)byaPhoneNumber, (char*)g_byaChangeTransferDestinationMoNumber);
#endif



#if defined(S800_DEV) || defined(PX7_DEV) || defined(S900_DEV)
	sprintf(byaAgentType, "%04d", g_stGlVar.m_sTerminalConfig.m_dwAgentType);
#else
	sprintf(byaAgentType, "%04d", QOM_AGENT);
#endif

	strcpy(byaTrnSignature,i_sProductInfo->m_byaTrnSignature);
	utlPadWithZero(byaTrnSignature, 32);
	utlPadWithZero(byaPhoneNumber, 11);
	utlPadWithZero(byaDriverCode,12);
	utlPadWithZero(byaRouteCode,12);
	utlPadWithZero(byaLongitude,20);
	utlPadWithZero(byaLatitude,20);
	if(dwFileSize > 0){

		memset(byaSettFileHeader, 0, sizeof(byaSettFileHeader));
		wLen = SOHA_SETT_TRN_FILE_HEADER_LEN;
        qDmgFile_Seek(hFile, 0, DMG_FILE_SEEK_SET);
        if(!qDmgFile_Read(hFile, byaSettFileHeader, &wLen))
		{
        	qDmgFile_Close(hFile);
			return FALSE;
		}


        dwCurrentTxnsCount = utlStrToDword((char*)byaSettFileHeader + 113, 5);
		gs_dwSohaOldTrnsCount = dwCurrentTxnsCount;

	}

	if(dwFileSize == 0){
			sprintf((char*)byaSettFileHeader,
				"%02d," // Transaction Settlement Version :2
				"%s," 	// Request Id :24
				"%s,"   // AgentType  :4
				"%012lu,"  // Station Id :12
				"%s,"      // Sim Card Id   :35
				"%02d,"    // Switch type  :2
				"%05lu,"  // City Id  :5
				"%s,"     // Terminal Id :10
				"%s,"     // MerchantId :10
				"%05d\n", // Transaction Count :5
				SOHA_SETT_FILE_TRANSACTION_VERSION,
				byaReqestId,
				byaAgentType,
				dwStationId,
				(char*)byaSimCardId,
				bySwitchType,
				dwCityId,
				szTerminalId,
				szMerchantId,
				dwCurrentTxnsCount);

        qDmgFile_Seek(hFile, 0, DMG_FILE_SEEK_SET);
        if(!qDmgFile_Write(hFile,byaSettFileHeader, strlen((char*)byaSettFileHeader))){
            qDmgFile_Close(hFile);
			return FALSE;
		}
	}

	memset(byaTrnSettRecord, 0, sizeof(byaTrnSettRecord));
	sprintf((char*)byaTrnSettRecord,
		"%s,"       // Transaction Type :6
		"%06lu,"    // Device Transaction Num : 6
		"%012lu,"   // Transaction Amount :12
		"%03d,"     // Discount Percent :3
		"%04lu,"    // Discount Type :4
		"%s%s,"     // Transaction Date Time :14
		"%02d,"     // Payment Type :2
		"%s,"       // Payment Information :20 // It should be 12 according to document!
		"%012lu,"   // LastBalance :12
		"%012lu,"   // New Balance :12
		"%012lu,"   // Card Deposit Amount :12
		"%s,"       // Card Serial Number :20
		"%05lu,"    // Card Contract Type :5
		"%05lu,"    // Card Tag Type :5
		"%010lu,"   // Card Sequence Number : 10
		"%s,"  		// Driver Code :12
		"%s,"  		// Route Code :12
		"%02X%02X%02X,"  //SAM Id :6
		"%08d,"		// Exp Date  :8
		"%012lu,"  	// Device Credit :12
		"%03d,"   	// Service Code :3
		"%012lu,"  	// Service Duration (Minute) :12
		"%s,"  		// TrnSignature :32
		"%012lu,"   // Decremental Counter : 12
		"%06d,"	    // Transaction Pointer :6
		"%06d,"	    // Travels count : 6
		"%06d,"	    // Discount travels count: 6
		"%s,"	    // Discount expire date: 8
		"%08d,"     // Shift Number :8
	    "%08d,"     // Halfway number:8
	    "%012lu,"   // vehicle number:12
		"%s,"       // SellerId	:17
		"%09lu,"    // CurrentCrdit:9
		"%09lu,"    // CashCredit:9
		"%s,"       // mobile num:11
		"%s,"       // latitude :20
		"%s,"		// longitude:20
		"00000000000000000000,"	// NationalCode:20
		"%s,"   	// TrackingCode:12
		"%05ld,"	// ResultCode:5
		"00000\n",	// ProductCode: 5
		i_sTrnInfo->m_byaTransactionType,
		sSettConfig.m_dwTrnCounter,
		i_sTrnInfo->m_dwAmount,
		i_sTrnInfo->m_dwDiscountPercent,
		i_sTrnInfo->m_dwDiscountType,
		szDateStr, szTimeStr,
		i_sTrnInfo->m_byPaymentType,
		byaPaymentInfo,
		i_sProductInfo->m_dwLastBalance,
		i_sProductInfo->m_dwCurrentBalance,
		i_sProductInfo->m_dwDeposit,
		byaCardSerial,
		i_sProductInfo->m_dwProductId,
		i_sProductInfo->m_dwTagType,
		i_sProductInfo->m_dwSeqNumber,
		byaDriverCode,
		byaRouteCode,
#ifndef T610_DEV
		samInfo.m_byaSAMID[0],
		samInfo.m_byaSAMID[1],
		samInfo.m_byaSAMID[2],
#else
            byaComponentId[0],
            byaComponentId[1],
            byaComponentId[2],
#endif
		i_sProductInfo->m_dwExpirationDate,// byaFaExpireDate,
		dwDeviceCreditAmount,
		i_sTrnInfo->m_byServiceCode,
		i_sTrnInfo->m_dwServiceDuration,
		byaTrnSignature,
		i_sProductInfo->m_dwDecrementalCounter,
		i_sProductInfo->m_dwTransactionPointer,
		i_sProductInfo->m_wTravelsCount,
		i_sProductInfo->m_wDiscountTravelsCount,
		byaDiscountExpireDate,
		i_sTrnInfo->m_dwShiftId,
		i_sTrnInfo->m_byHalfWayNum,
		i_sTrnInfo->m_byVehicleNum,
		byaSellerId,
		i_sTrnInfo->m_dwCurrentCrdit,
		i_sTrnInfo->m_dwCashCredit,
		byaPhoneNumber,
		byaLatitude,
		byaLongitude,
        byaTrackingCode,
        i_sTrnInfo->m_dwResult
		);

		if(dwFileSize == 0)
		qDmgFile_Seek(hFile, 0, DMG_FILE_SEEK_END);
		else
		qDmgFile_Seek(hFile, dwFileSize , DMG_FILE_SEEK_SET);

//		memset(byaTrnSettRecord, 0, sizeof(byaTrnSettRecord));
//		sprintf((char*)byaTrnSettRecord,"%s","937034973047304703473094\n");
		OsLog(LOG_DEBUG, "\n@SohaTrnFile > byaTrnSettRecord content:\n %s\n", byaTrnSettRecord);
		if(!qDmgFile_Write(hFile, (BYTE*)byaTrnSettRecord, SOHA_SETT_TRN_FILE_RECORD_LEN ))
		{
			qDmgFile_Close(hFile);
			return FALSE;
		}

		OsLog(LOG_DEBUG, "\n@ SohaAddTrnToOpenFile > Data written into TRN FILE successfully.\n");

		dwCurrentTxnsCount++;

		memset(byaSettFileHeader, 0, sizeof(byaSettFileHeader));
		sprintf((char*)byaSettFileHeader, "%05lu", dwCurrentTxnsCount);
		OsLog(LOG_DEBUG, "\n@SohaTrnFile.c >update header with dwCurrentTxnsCount: :%d\n",dwCurrentTxnsCount);
		//@HERE
		OsLog(LOG_DEBUG, "\n@SohaTrnFile.c >update header with byaSettFileHeader: :%s\n",byaSettFileHeader);


		qDmgFile_Seek(hFile, SOHA_SETT_TRN_FILE_HEADER_LEN - 6, DMG_FILE_SEEK_SET);
		if(!qDmgFile_Write(hFile,byaSettFileHeader, 5 ))
		{
			qDmgFile_Close(hFile);
			return FALSE;
		}

    	qDmgFile_Close(hFile);

    	if (SohaSettOpenFileIsFull(SOHA_SETT_OPEN_TRN_FILE_NAME))
    	{
    		OsLog(LOG_DEBUG, "\n File is FUll! Going to add to archive.");
    		if (!SohaSettArchiveOpenFile(NULL))
    		{
    			OsLog(LOG_DEBUG, "@SohaWebServiceTrnFile.c > SohaWsSettAddTrnToOpenFile > Opening Archive Failed! \n");
    			return FALSE;
    		}
    	}


		SohaSettIncTrnCounter();

		return TRUE;
}


BYTE SohaSettDelLastRec(void)
{
	HFILE hFile = 0;
	DWORD dwFileSize = 0;
	BYTE byaHold[1024] = {0};

    dwFileSize = qDmgFile_GetSize(SOHA_SETT_OPEN_TRN_FILE_NAME);
	if(dwFileSize == 0)
		return TRUE;

	if(dwFileSize <= SOHA_SETT_TRN_FILE_HEADER_LEN + SOHA_SETT_TRN_FILE_RECORD_LEN)
	{
        qDmgFile_Remove(SOHA_SETT_OPEN_TRN_FILE_NAME);
		SohaSettDecTrnCounter();
		return TRUE;
	}

    if(!qDmgFile_Open(SOHA_SETT_OPEN_TRN_FILE_NAME, DMG_FILE_OPEN_MODE_UPDATE, &hFile))
		return FALSE;

    if(!qDmgFile_Truncate(hFile, dwFileSize - SOHA_SETT_TRN_FILE_RECORD_LEN))
	{
        qDmgFile_Close(hFile);
		return FALSE;
	}

	memset(byaHold, 0, sizeof(byaHold));
	sprintf((char*)byaHold, "%06lu\n",gs_dwSohaOldTrnsCount);
    qDmgFile_Seek(hFile, SOHA_SETT_TRN_FILE_HEADER_LEN - 7, DMG_FILE_SEEK_SET);
    if(!qDmgFile_Write(hFile, byaHold, 6 ))
	{
        qDmgFile_Close(hFile);
		return FALSE;
	}


    qDmgFile_Close(hFile);

	SohaSettDecTrnCounter();

	return TRUE;
}


BYTE SohaSettCheckFileExistForSend(void)
{
//	BYTE byOpenFileExistance = 0;
//	BYTE byArchiveFileCount = 0;
//
//    if(qDmgFile_GetSize(SOHA_SETT_OPEN_TRN_FILE_NAME) > 0)
//		byOpenFileExistance = TRUE;
//
//    byArchiveFileCount = (BYTE)(qDmgFile_GetSize(SOHA_SETT_ARCHIVE_TABLE_FILE_NAME) / sizeof(sohaSettArchivedFile));
//	OsLog(LOG_DEBUG,"/n__________________________SOHA_SETT_ARCHIVE_TABLE_FILE_NAME size:%d________________________________/n/n",byArchiveFileCount);
//
//	if((byOpenFileExistance) || (byArchiveFileCount > 0))
//		return TRUE;
//
//	return FALSE;

	BYTE byOpenFileExistance = 0;
	DWORD dwArchiveFileCount = 0;
	char* dirPath=".";

	if(dmgFile_GetSize(SOHA_SETT_OPEN_TRN_FILE_NAME) > 0)
		byOpenFileExistance = TRUE;

	dwArchiveFileCount = utlDirectoryFileCount(dirPath,".BZ2");
	dwArchiveFileCount += utlDirectoryFileCount(dirPath,".uz");
//		dwArchiveFileCount = dwArchiveFileCount + utlDirectoryFileCount(dirPath,".DAT");

	OsLog(LOG_DEBUG, "dwArchiveFileCount = %d", dwArchiveFileCount);
	OsLog(LOG_DEBUG, "byOpenFileExistance = %d", byOpenFileExistance);

	if((byOpenFileExistance) || (dwArchiveFileCount > 0))
		return TRUE;

	return FALSE;
}

#ifndef T610_DEV
BYTE SohaSettSendFiles(BYTE i_byShowMsg, BYTE i_byAutoSend)
{
	BYTE byOpenFileExistance = 0;
	BYTE byArchiveFileCount = 0;
	BYTE byCounter = 0;
	HFILE hFile = 0;
	WORD wLen = 0;
	char szSendFileName[50] = {0};
	BYTE byaTerminalSn[33] = {0};
	DWORD	dwCurrentDate		=	0;
	DWORD	dwCurrentTime		=	0;
	sohaSettArchivedFile sArchivedFile;


	if(i_byShowMsg)
	{
		
		if(getBoOperationStatus())
		{
			if(i_byShowMsg)
				uiDisplayMsgWithId(MESSAGE_PBG_OPERATION_IS_RUNNING, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_INFORM);
			return FALSE;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Update settlement info in terminal internal config
	dwCurrentDate = dmgBasic_GetDate();
	dwCurrentTime = dmgBasic_GetTime();

	if (i_byAutoSend)
	{
		g_stGlVar.m_sTermIntConfig.m_dwLastAutoSettTryDate = dwCurrentDate;
		g_stGlVar.m_sTermIntConfig.m_dwLastAutoSettTryTime = dwCurrentTime;
	}
	else
	{
		g_stGlVar.m_sTermIntConfig.m_dwLastManualSettTryDate = dwCurrentDate;
		g_stGlVar.m_sTermIntConfig.m_dwLastManualSettTryTime = dwCurrentTime;
	}

#ifndef S900_DEV
	TermCfgSaveTerminalInternalConfig(&g_stGlVar.m_sTermIntConfig);
#endif

	OsLog(LOG_DEBUG,"/n log 1/n/n");
	ftp_closeConnectComplete();

	memset(&sArchivedFile, 0, sizeof(sohaSettArchivedFile));
	if(!SohaSettCheckFileExistForSend())
	{
		if (i_byAutoSend)
			SohaSettSetAutoSendFileResult(TRUE);
		if(i_byShowMsg)
			uiDisplayMsgWithId(MESSAGE_ERR_NO_SETT_FILE_FOR_SEND, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_INFORM);

		return TRUE;
	}
	if (i_byAutoSend)
		SohaSettSetAutoSendFileResult(FALSE);

	dmgBasic_GetTerminalSerialNumber((char*)byaTerminalSn);

    if(qDmgFile_GetSize(SOHA_SETT_OPEN_TRN_FILE_NAME) > 0)
		byOpenFileExistance = TRUE;

//    byArchiveFileCount = (BYTE)(qDmgFile_GetSize(SOHA_SETT_ARCHIVE_TABLE_FILE_NAME) / sizeof(sohaSettArchivedFile));


	if(i_byShowMsg)
		uiDisplayMsgWithId(MESSAGE_SEND_SETT_FILE_PLEASE_WAIT, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_INFORM);

	if(!ftp_login())
	{
		OsLog(LOG_DEBUG,"/n log : ftp_login failed!/n/n");

		ftp_closeConnectComplete();

		if(i_byShowMsg)
		{
			uiDisplayMsgWithId(MESSAGE_CONNECT_ERROR, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR);
		}

		return FALSE;
	}

	if(byOpenFileExistance)
	{
		if (i_byAutoSend)
		{
			if (!SohaSettGetOpenFileAccessBlockStatus())
			{
				gs_bySohaInternalLock = TRUE;
				OsLog(LOG_DEBUG,"/n log : In AutoSend: Let's Archive openfile!/n/n");

				if (!SohaSettArchiveOpenFile(NULL))
				{
					OsLog(LOG_DEBUG,"/n log : In AutoSend: Archive openfile Failed!/n/n");

					ftp_closeConnectComplete();

					if (i_byShowMsg)
						uiDisplayMsgWithId_ErrCodeDisplay(MESSAGE_ERR_PREPARE_SETT_FILES, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR,TRUE);

					return FALSE;
				}

				gs_bySohaInternalLock = FALSE;
				settResetOpenFileAccessFailed();
			}
			else
			{
				OsLog(LOG_DEBUG,"/n log : AccessFailed!/n/n");
				settSetOpenFileAccessFailed();
			}
		}
		else
		{

			if (!SohaSettArchiveOpenFile(NULL))
			{
				OsLog(LOG_DEBUG,"/n log : In Manual Send: Let's Archive openfile!/n/n");

				ftp_closeConnectComplete();

				if (i_byShowMsg)
					uiDisplayMsgWithId(MESSAGE_ERR_PREPARE_SETT_FILES, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR);

				return FALSE;
			}
		}
	}

    if((SohaSettGetOpenFileAccessFailedStatus()) && (qDmgFile_GetSize(SOHA_SETT_OPEN_TRN_FILE_NAME) <= 0))
	{
		OsLog(LOG_DEBUG,"/n log : In Manualsend: Let's Archive openfile!/n/n");

		SohaSettResetOpenFileAccessFailed();
	}


	BYTE retValFtp = settSendArchivedFiles(".BZ2", "TEH",NULL);

	OsLog(LOG_DEBUG,"/n log : settSendArchivedFiles Done!/n/n");
	retValFtp = settSendArchivedFiles(".uz", "TEH",NULL);
	ftp_closeConnectComplete();

	if(retValFtp)
					{
	if (i_byAutoSend)
		SohaSettSetAutoSendFileResult(TRUE);
	if(i_byShowMsg)
		uiDisplayMsgWithId(MESSAGE_SEND_SETT_FILES_INFORM, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_SUCCESS);
	}

	return TRUE;
}
#endif




#ifdef S800_DEV
BYTE SohaSettGetArchiveFileInfo(char* i_szFileName, sohaSettArchivedFile* o_sArchiveFileInfo)
{
	BYTE	byaDateTimeStr[11]			=	{0};
	BYTE	byaBuffer[1024]			=	{0};
	WORD	wLen					=	0;
	HFILE	hFile					=	0;

	memset(o_sArchiveFileInfo, 0, sizeof(sohaSettArchivedFile));

	if(!dmgFile_Open(i_szFileName, DMG_FILE_OPEN_MODE_READ, &hFile))
		return FALSE;

	dmgFile_Seek(hFile, 0, DMG_FILE_SEEK_SET);
	wLen = SOHA_SETT_TRN_FILE_HEADER_LEN;
	if(!dmgFile_Read(hFile, (BYTE*)byaBuffer, &wLen))
	{
		dmgFile_Close(hFile);
		return FALSE;
	}
	//OsPrnReset();OsPrnPrintf("HEADER: %s",byaBuffer);OsPrnStart();
	strncpy((char*)byaDateTimeStr, (char*)byaBuffer + SOHA_SETT_HEADER_DATE_TIME_INDEX, 14);
	byaDateTimeStr[14] = 0;



	o_sArchiveFileInfo->m_dwTrnCount = utlStrToDword((char*)byaBuffer + 113, 5);


	if(dmgFile_GetSize(i_szFileName) > SOHA_SETT_MAX_FILE_SIZE_FOR_ZIP)
		sprintf(o_sArchiveFileInfo->m_szOrgFileName, "%s.TXT.uz", byaDateTimeStr);
	else
		sprintf(o_sArchiveFileInfo->m_szOrgFileName, "%s.BZ2", byaDateTimeStr);

	return TRUE;
}


BYTE SohaSettGetNotSentTrnsInfo(DWORD* o_dwTrnsCount, DWORD* o_dwTrnsAmount)
{
	WORD wLen =	0;
	DWORD dwCounter = 0;
	DWORD dwFileSize = 0;
	DWORD dwTrnsCount = 0;
	DWORD dwTrnsAmount = 0;
	HFILE hFile = 0;
	sohaSettArchivedFile sArchivedFile;

	memset(&sArchivedFile, 0, sizeof(sohaSettArchivedFile));

	dwFileSize = dmgFile_GetSize(SOHA_SETT_ARCHIVE_TABLE_FILE_NAME);

	if(dwFileSize > 0)
	{
		if(!dmgFile_Open(SOHA_SETT_ARCHIVE_TABLE_FILE_NAME, DMG_FILE_OPEN_MODE_READ, &hFile)){
			(*o_dwTrnsCount) = dwTrnsCount;
			(*o_dwTrnsAmount) = dwTrnsAmount;
			return FALSE;
		}
		for(dwCounter = 0; dwCounter < dwFileSize / sizeof(settArchivedFile); dwCounter++)
		{
			memset(&sArchivedFile, 0, sizeof(sohaSettArchivedFile));

			dmgFile_Seek(hFile, dwCounter * sizeof(sohaSettArchivedFile), DMG_FILE_SEEK_SET);
			wLen = sizeof(sohaSettArchivedFile);
			if(!dmgFile_Read(hFile, (BYTE*)&sArchivedFile, &wLen))
				break;

			dwTrnsCount += sArchivedFile.m_dwTrnCount;
		}

		dmgFile_Close(hFile);
	}

	memset(&sArchivedFile, 0, sizeof(sohaSettArchivedFile));
	if(dmgFile_GetSize(SOHA_SETT_OPEN_TRN_FILE_NAME) > 0)
	{
		settGetArchiveFileInfo(SOHA_SETT_OPEN_TRN_FILE_NAME, &sArchivedFile);
		dwTrnsCount += sArchivedFile.m_dwTrnCount;
	}

	(*o_dwTrnsCount) = dwTrnsCount;
	(*o_dwTrnsAmount) = dwTrnsAmount;

	return TRUE;
}


void supervisorShowQcardTrnFilesInfo()
{
	DWORD dwFilesCount 	= 0;
	DWORD dwFilesSize 	= 0;
	sohaSettArchivedFile sArchivedFile;
	message	sMessage;
	HFILE hFile	 	=	0;
	DWORD dwCounter	=	0;
	WORD  wLen		=	0;

	if(dmgFile_GetSize(SOHA_SETT_OPEN_TRN_FILE_NAME) > 0)
	{
		if(!SohaSettArchiveOpenFile(NULL))
			return;
	}

	dwFilesCount = (BYTE)(dmgFile_GetSize(SOHA_SETT_ARCHIVE_TABLE_FILE_NAME) / sizeof(sohaSettArchivedFile));

	if(dwFilesCount > 0)
	{
		if(!dmgFile_Open(SOHA_SETT_ARCHIVE_TABLE_FILE_NAME, DMG_FILE_OPEN_MODE_UPDATE, &hFile))
			return;

		for(dwCounter = 0; dwCounter < dwFilesCount; dwCounter++)
		{
			memset(&sArchivedFile, 0, sizeof(sohaSettArchivedFile));
			dmgFile_Seek(hFile, (dwFilesCount - dwCounter - 1) * sizeof(sohaSettArchivedFile), DMG_FILE_SEEK_SET);
			wLen = sizeof(sohaSettArchivedFile);
			if(!dmgFile_Read(hFile, (BYTE*)&sArchivedFile, &wLen))
			{
				dmgFile_Close(hFile);
				return;
			}

			dwFilesSize += dmgFile_GetSize(sArchivedFile.m_szOrgFileName);
		}

		dmgFile_Close(hFile);
	}

	if(dwFilesCount == 0)
	{
		uiDisplayMsgWithId(MESSAGE_ERR_NO_SETT_FILE_FOR_SEND,UI_DISPLAY_WAIT_TYPE_ANY_KEY,UI_DISPLAY_MSG_TYPE_INFORM);
	}
	else
	{
		memset(&sMessage,0,sizeof(message));
		strcpy(sMessage.m_byaLines[0],":اطلاعات فایل های تسویه");
				sprintf(sMessage.m_byaLines[1],"تعداد : %lu سایز: %lu کیلوبایت",dwFilesCount,!dwFilesSize?0:dwFilesSize/1024 + 1);
		sMessage.m_byLineCount = 2;
		uiDisplayMsg(&sMessage,UI_DISPLAY_WAIT_TYPE_ANY_KEY,UI_DISPLAY_MSG_TYPE_INFORM,uiDisplay_getFontSize(UI_DISPLAY_FONT_SIZE));
	}
	return;

}



#endif


#ifdef T610_DEV
DWORD SohaSettSendFiles(BYTE i_byAutoSend)
{
	BYTE	byOpenFileExistance		=	0;
	BYTE	byArchiveFileCount		=	0;
	BYTE	byCounter				=	0;
	HFILE	hFile					=	0;
	BYTE	byRet					=	0;
	WORD	wLen					=	0;
	char	szArchiveFileName[50]	=	{0};
	char	szSendFileName[50]		=	{0};
	BYTE	byaTerminalSn[33]		=	{0};
	DWORD	dwRetVal				=	0;
	DWORD	dwCurrentDate		=	0;
	DWORD	dwCurrentTime		=	0;
	sohaSettArchivedFile	sArchivedFile;
	terminalConfigSt	sTerminalConfig;
	sTerminalInternalConfig sTermIntConfig;

	memset(&sTerminalConfig, 0, sizeof(terminalConfigSt));
	memset(&sTermIntConfig, 0, sizeof(sTerminalInternalConfig));

	if(!termConfigGet(&sTerminalConfig))
		return FALSE;
	if(!TermCfgGetTerminalInternalConfig(&sTermIntConfig))
		return FALSE;

	memset(&sTerminalConfig, 0, sizeof(terminalConfigSt));


	if(!termConfigGet(&sTerminalConfig))
		return FALSE;



	memset(&sArchivedFile, 0, sizeof(sohaSettArchivedFile));

	dmgBasic_GetTerminalSerialNumber(byaTerminalSn);

	dwCurrentDate = dmgBasic_GetDate();
	dwCurrentTime = dmgBasic_GetTime();

	if(i_byAutoSend)
	{
		sTermIntConfig.m_dwLastAutoSettTryDate = dwCurrentDate;
		sTermIntConfig.m_dwLastAutoSettTryTime = dwCurrentTime;
	}
	else
	{
		sTermIntConfig.m_dwLastManualSettTryDate = dwCurrentDate;
		sTermIntConfig.m_dwLastManualSettTryTime = dwCurrentTime;
	}

	TermCfgUpdateTerminalInternalConfig(&sTermIntConfig);

	if((dmgFile_GetSize(SOHA_SETT_OPEN_TRN_FILE_NAME) > 0) )
		byOpenFileExistance = TRUE;

	byArchiveFileCount = (BYTE)(dmgFile_GetSize(SOHA_SETT_ARCHIVE_TABLE_FILE_NAME) / sizeof(sohaSettArchivedFile));

	if((!byOpenFileExistance) && (byArchiveFileCount == 0)){
		if(i_byAutoSend)
			settSetAutoSendFileResult(TRUE);
		return SUCCESSFUL;
	}

	//Lcdprintf("sett send files 11");getkey();

	if(i_byAutoSend)
		settSetAutoSendFileResult(FALSE);

	uiDisplayShowMessageWithId_New(TUI_MENU_MESSAGE_TYPE_NOTIFICATION, TUI_MSG_INFO_CONNECTING_TO_SWITCH, TUI_DISPLAY_WAIT_TYPE_NO_WAIT);

	if(!gs_byInitWirelessModulet){
		//Lcdprintf("!gs_byInitWirelessModulet");getkey();
		if(!connInitWirelessModule()){
			//Lcdprintf("!connInitWirelessModule()");getkey();
			uiDisplayShowMessageWithId_New(TUI_MENU_MESSAGE_TYPE_ERROR, TUI_MSG_INFO_CONNECTION_ERROR, TUI_DISPLAY_WAIT_TYPE_FOR_ENTER);
			return ERR_CONNECTION_ERR;
		}
	}
	WlPppLogout ();
	connClosePort();
	while(WlPppCheck() == 1);

	if(!connInitGPRSModule()){
		//Lcdprintf("!connInitGPRSModule()");getkey();
		uiDisplayShowMessageWithId_New(TUI_MENU_MESSAGE_TYPE_ERROR, TUI_MSG_INFO_CONNECTION_ERROR, TUI_DISPLAY_WAIT_TYPE_FOR_ENTER);
		return ERR_CONNECTION_ERR;
	}

	if(!FTPLogin()){
		uiDisplayShowMessageWithId_New(TUI_MENU_MESSAGE_TYPE_ERROR, TUI_MSG_INFO_CONNECTION_ERROR, TUI_DISPLAY_WAIT_TYPE_FOR_ENTER);
		//  Lcdprintf("ftp login fail 1");getkey();
		return ERR_CONNECTION_ERR;
	}

	if(byOpenFileExistance)
	{

		if(!SohaSettArchiveOpenFile( NULL))
			return ERR_SEND_FILE_ERR;

	}

	uiDisplayShowMessageWithId_New(TUI_MENU_MESSAGE_TYPE_NOTIFICATION, TUI_MSG_INFO_SENDING_FILE, TUI_DISPLAY_WAIT_TYPE_NO_WAIT);

	byArchiveFileCount = (BYTE)(dmgFile_GetSize(SOHA_SETT_ARCHIVE_TABLE_FILE_NAME) / sizeof(sohaSettArchivedFile));
	//("Count : %d size: %d",byArchiveFileCount,dmgFile_GetSize(SOHA_SETT_ARCHIVE_TABLE_FILE_NAME));getkey();
	if(byArchiveFileCount > 0)
	{

		for(byCounter = 0; byCounter < byArchiveFileCount; byCounter++)
		{
			//Lcdprintf(" %d from %d sent",byCounter,byArchiveFileCount);getkey();
			dwRetVal = dmgFile_Open(SOHA_SETT_ARCHIVE_TABLE_FILE_NAME, DMG_FILE_OPEN_MODE_READ, &hFile);
			if(dwRetVal != SUCCESSFUL)
			{

				return ERR_SEND_FILE_ERR;
			}
			memset(&sArchivedFile, 0, sizeof(sohaSettArchivedFile));
			dmgFile_Seek(hFile, /*(byArchiveFileCount - byCounter - 1)* sizeof(settArchivedFile)*/0, DMG_FILE_SEEK_SET);
			wLen = sizeof(sohaSettArchivedFile);
			dwRetVal = dmgFile_Read(hFile, (BYTE*)&sArchivedFile, &wLen);
			if(dwRetVal != SUCCESSFUL)
			{

				dmgFile_Close(hFile);
				return ERR_SEND_FILE_ERR;
			}
			if(dmgFile_GetSize(sArchivedFile.m_szOrgFileName) > 0)
			{
				memset(szSendFileName, 0, sizeof(szSendFileName));
				strcpy((char*)szSendFileName, (char*)byaTerminalSn + (strlen((char*)byaTerminalSn) - 8));
				strcat(szSendFileName, "_");
				strcat(szSendFileName, sArchivedFile.m_szOrgFileName);
				byRet	= FTPPutFile(szSendFileName, sArchivedFile.m_szOrgFileName);
				if(!byRet)
				{
					return ERR_SEND_FILE_ERR;
				}
				byRet = dmgFile_RemoveFromBegin(hFile,SOHA_SETT_ARCHIVE_TABLE_FILE_NAME,(byArchiveFileCount - byCounter - 1)* sizeof(sohaSettArchivedFile));
				if(byRet == SUCCESSFUL)
					dmgFile_Remove(sArchivedFile.m_szOrgFileName);
			}
		}

		dmgFile_Close(hFile);
	}
	if(i_byAutoSend)
		settSetAutoSendFileResult(TRUE);
	uiDisplayShowMessageWithId_New(TUI_MENU_MESSAGE_TYPE_NOTIFICATION, TUI_MSG_INFO_SEND_FILE_SUCCESS, TUI_DISPLAY_WAIT_TYPE_FOR_ENTER);

	return SUCCESSFUL;
}
#endif

