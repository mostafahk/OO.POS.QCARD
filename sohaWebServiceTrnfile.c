/*
 * sohaWebServiceTrnfile.c
 *
 *  Created on: Aug 26, 2019
 *      Author: f.fereydounian
 */
#include "define.h"
#include "sohaWebServiceTrnfile.h"
#include "dirent.h"
#include "Utility/myCurl.h"
#include <SettFiles/TrnsSettFiles.h>


//#if defined(PX7_DEV) || defined(S800_DEV) || defined(S900_DEV)
DWORD gs_dwSohaWsOldTrnsCount = 0;
DWORD gs_dwSohaWsLoadSettConfigFlag = FALSE;
SohaWsSettConfig gs_swSettConfig;
static BYTE gs_bySohaWsAutoSendFileResult = FALSE;
static BYTE gs_bySohaWsOpenFileAccessBlocked = FALSE;
static BYTE gs_bySohaWsOpenFileAccessFailed = FALSE;
static BYTE gs_bySohaWsInternalLock = FALSE;



void SohaWsSettSetAutoSendFileResult(BYTE i_byStatus)
{
	gs_bySohaWsAutoSendFileResult = i_byStatus;
}

BYTE SohaWsSettGetAutoSendFileResult()
{
	return gs_bySohaWsAutoSendFileResult;
}

void SohaWsSettBlockOpenFileAccess()
{
	if (gs_bySohaWsInternalLock)
	{
		uiDisplayMsgWithId(MESSAGE_PLEASE_WAIT, UI_DISPLAY_WAIT_TYPE_NO_WAIT, 0);
		while (gs_bySohaWsInternalLock)
			;
	}

	gs_bySohaWsOpenFileAccessBlocked = TRUE;
}

void SohaWsSettReleaseOpenFileAccess()
{
	gs_bySohaWsOpenFileAccessBlocked = FALSE;
}

BYTE SohaWsSettGetOpenFileAccessBlockStatus()
{
	return gs_bySohaWsOpenFileAccessBlocked;
}

void SohaWsSettSetOpenFileAccessFailed()
{
	gs_bySohaWsOpenFileAccessFailed = TRUE;
}

void SohaWsSettResetOpenFileAccessFailed()
{
	gs_bySohaWsOpenFileAccessFailed = FALSE;
}

BYTE SohaWsSettGetOpenFileAccessFailedStatus()
{
	return gs_bySohaWsOpenFileAccessFailed;
}


BYTE SohaWsSettIncTrnCounter(void)
{

	SohaWsSettConfig	sSettconfig;
	memset(&sSettconfig, 0, sizeof(SohaWsSettConfig));
	OsLog(LOG_DEBUG, "@SohaWebServiceTrnFile.c > SohaWsSettIncTrnCounter #1\n");

	if(!SohaWsSettGetConfig(&sSettconfig))
		return FALSE;

	if(sSettconfig.m_dwTrnCounter < 999999)
	{
		sSettconfig.m_dwTrnCounter++;
	}
	else
	{
		sSettconfig.m_dwTrnCounter = 1;
	}
	return SohaWsSettUpdateConfig(&sSettconfig);
}


BYTE SohaWsSettDecTrnCounter(void)
{

	SohaWsSettConfig	sSettconfig;

	memset(&sSettconfig, 0, sizeof(SohaWsSettConfig));

	if(!SohaWsSettGetConfig(&sSettconfig))
		return FALSE;

	if(sSettconfig.m_dwTrnCounter > 1)
	{
		sSettconfig.m_dwTrnCounter--;

	}
	else
	{
		sSettconfig.m_dwTrnCounter = 999999;

	}


	return SohaWsSettUpdateConfig(&sSettconfig);
}


BYTE SohaWsSettIncTrnFileCounter(void)
{

	SohaWsSettConfig	sSettconfig;

	memset(&sSettconfig, 0, sizeof(SohaWsSettConfig));

	if(!SohaWsSettGetConfig(&sSettconfig))
		return FALSE;

	if(sSettconfig.m_dwTrnFileCounter < 999)
		sSettconfig.m_dwTrnFileCounter++;
	else
		sSettconfig.m_dwTrnFileCounter = 1;



	return SohaWsSettUpdateConfig(&sSettconfig);
}


BYTE SohaWsSettDecTrnFileCounter(void)
{

	SohaWsSettConfig	sSettconfig;

	memset(&sSettconfig, 0, sizeof(SohaWsSettConfig));

	if(!SohaWsSettGetConfig(&sSettconfig))
		return FALSE;

	if(sSettconfig.m_dwTrnFileCounter > 1)
		sSettconfig.m_dwTrnFileCounter--;
	else
		sSettconfig.m_dwTrnFileCounter = 999;



	return SohaWsSettUpdateConfig(&sSettconfig);
}


BYTE SohaWsSettOpenFileIsFull(char* i_szFileName)
{
	OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsSettOpenFileIsFull > dmgFile_GetSize(i_szFileName): %d",qDmgFile_GetSize(i_szFileName));
	if (qDmgFile_GetSize(i_szFileName) > SOHA_WS_SETT_MAX_FILE_SIZE_FOR_STORE)
		return TRUE;

	return FALSE;
}

BYTE SohaWsSettAddTrnToOpenFile(SohaWsSettProductInfo* i_sProductInfo, SohaWsSettTrnInfo* i_sTrnInfo)
{
	SohaWsSettConfig sSettConfig;
	memset(&sSettConfig, 0, sizeof(SohaWsSettConfig));

	char szDateStr[9] = { 0 };
	char szTimeStr[7] = { 0 };

	BYTE byLoopCounter = 0;
	BYTE byaTerminalSn[33] = { 0 };
	BYTE byaSendTerminalSn[33] = { 0 };    //Last 8 digit of Device Serial#

	//Header
	BYTE byaReqestId[24] = { 0 };
	DWORD dwStationId = 0;
	BYTE byaSimCardId[36] = { 0 };
	BYTE bySwitchType = g_stGlVar.m_sTerminalConfig.m_bySwitchVersion;
	DWORD dwCityId = 0;
	char szTerminalId[11] = { 0 };
	char szMerchantId[11] = { 0 };
	BYTE byaTrnSignature[33] = { 0 };
	BYTE byaAgentType[5] = {0};

	//Body
	BYTE byaDriverCode[13] = { 0 };
	BYTE byaRouteCode[13] = { 0 };
	BYTE byaSellerId[18]={0};
	BYTE byaPhoneNumber[12] = {0};
	BYTE byaLatitude[21] = {0};
	BYTE byaLongitude[21] = {0};
	DWORD dwDeviceCreditAmount = 0;

	BYTE byaSettFileHeader[1024] = { 0 };
	BYTE byaTrnSettRecord[1024] = { 0 };

	BYTE byaPaymentInfo[21] = { 0 };
	BYTE byaCardSerial[21] = { 0 };
	WORD wLen = 0;
	DWORD dwCurrentTxnsCount = 0;
	DWORD dwFileSize = 0;
	HFILE hFile = 0;

	BYTE byaSimID[21] = { 0 };
	BYTE byaSimIMSI[21] = { 0 };
	BYTE byaExpireDate[9] = { 0 };
	BYTE byaDiscountExpireDate[9] = { 0 };
    char byaTrackingCode[13] = {0};

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


#else
	BYTE byaComponentId[8] = { 0 };
	desfireSamRetrieveUid(byaComponentId);
#endif

	if (i_sProductInfo->m_wDiscountExpireDate != 0)
	{
#ifndef T610_DEV
		time_t t_discountExpDateTime = 0;
		struct tm ts;
		t_discountExpDateTime = i_sProductInfo->m_wDiscountExpireDate;
		t_discountExpDateTime = t_discountExpDateTime << 16;
		ts = *localtime(&t_discountExpDateTime);
		sprintf((char*) byaDiscountExpireDate, "%04d%02d%02d", 1900 + ts.tm_year, ts.tm_mon, ts.tm_mday);

#else
		DWORD dwStartingPoint = utlSetDate(1900, 1 , 1 );
		DWORD dwDays = i_sProductInfo->m_wDiscountExpireDate - ((i_sProductInfo->m_wDiscountExpireDate/365)/4 + 1 );
		DWORD dwTheDate = utlSetNextDate(dwStartingPoint, 0, 0, dwDays);
		sprintf((char*)byaDiscountExpireDate,"%04d%02d%02d", utlGetYear(dwTheDate),utlGetMonth(dwTheDate),utlGetDay(dwTheDate));
#endif
	}
	utlPadWithZero(byaDiscountExpireDate, 8);

	if (!SohaWsSettGetConfig(&sSettConfig))
		return FALSE;

	OsLog(LOG_DEBUG, "\n SohaWsSettAddTrnToOpenFile > m_dwTrnCounter:%d, m_dwTrnFileCounter=%d\n", sSettConfig.m_dwTrnCounter, sSettConfig.m_dwTrnFileCounter);


	sprintf((char*) szDateStr, "%04d%02d%02d", utlGetYear(i_sTrnInfo->m_dwDate), utlGetMonth(i_sTrnInfo->m_dwDate),
			utlGetDay(i_sTrnInfo->m_dwDate));

	sprintf((char*) szTimeStr, "%02d%02d%02d", utlGetHour(i_sTrnInfo->m_dwTime), utlGetMinute(i_sTrnInfo->m_dwTime),
			utlGetSecond(i_sTrnInfo->m_dwTime));

	dwStationId = g_stGlVar.m_sTerminalConfig.m_dwStationId;

#ifdef T610_DEV
	BYTE byaComponentId[8] =
	{	0};
	desfireSamRetrieveUid(byaComponentId);
#endif

	dmgBasic_GetTerminalSerialNumber((char*) byaTerminalSn);
	strcpy((char*) byaSendTerminalSn, (char*) byaTerminalSn + (strlen((char*) byaTerminalSn) - 8));
	utlPadWithZero(byaSendTerminalSn, 10);

	sprintf(byaReqestId, "%s%s%s", byaSendTerminalSn, szDateStr, szTimeStr);
	utlPadWithZero(byaReqestId, 23);

#if defined(S800_DEV) || defined(PX7_DEV)|| defined(S900_DEV)
	sprintf(byaAgentType, "%04d", g_stGlVar.m_sTerminalConfig.m_dwAgentType);
#else
	sprintf(byaAgentType, "%04d", QOM_AGENT);
#endif


	utlPadWithZero(byaExpireDate, 8);
	dwFileSize = qDmgFile_GetSize(SOHA_WS_SETT_OPEN_TRN_FILE_NAME);
	OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsSettAddTrnToOpenFile > #1 dwFileSize : %d\n", dwFileSize);

	if (dwFileSize == 0)
	{
		if (!qDmgFile_Open(SOHA_WS_SETT_OPEN_TRN_FILE_NAME, DMG_FILE_OPEN_MODE_CREATE, &hFile))
			return FALSE;
		OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsSettAddTrnToOpenFile > TRN FILE created successfully.\n");
	}
	else
	{
		if (!qDmgFile_Open(SOHA_WS_SETT_OPEN_TRN_FILE_NAME, DMG_FILE_OPEN_MODE_UPDATE, &hFile))
			return FALSE;
		OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsSettAddTrnToOpenFile > TRN FILE opened for update successfully.\n");

	}
	strcpy((char*) byaSimID, "00000000000000000000");   //TODO : fix
	strcpy((char*) byaSimIMSI, "000000000000000");

	sprintf((char*) byaSimCardId, "%s%s", byaSimID, byaSimIMSI);
	utlPadWithZero(byaSimCardId, 35);

	sprintf((char*) byaPaymentInfo, "%s", i_sTrnInfo->m_byaPaymentInfo);
	utlPadWithZero(byaPaymentInfo, 20);

	sprintf((char*) byaCardSerial, "%s", i_sProductInfo->m_byaSerial);
	utlPadWithZero(byaCardSerial, 20);


	strcpy((char*)byaSellerId, (char*)i_sTrnInfo->m_byaSellerId);
	utlPadWithZero(byaSellerId, 17);

    strncpy((char*)byaTrackingCode, (char*)i_sTrnInfo->m_byaTrackingCode, 12);
    utlPadWithZero(byaTrackingCode, 12);

#ifdef S900_DEV
	if (strcmp((char *)i_sTrnInfo->m_byaTransactionType, "901411") == 0)
		strcpy((char*)byaPhoneNumber, (char*)g_byaChangeTransferDestinationMoNumber);
#endif
	strcpy(byaTrnSignature, i_sProductInfo->m_byaTrnSignature);
	utlPadWithZero(byaPhoneNumber,11);
	utlPadWithZero(byaTrnSignature,32);
	utlPadWithZero(byaDriverCode,12);
	utlPadWithZero(byaRouteCode,12);
	utlPadWithZero(byaLongitude,20);
	utlPadWithZero(byaLatitude,20);


	OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsSettAddTrnToOpenFile >>>>> Let's continue\n");



	if (dwFileSize > 0)
	{
		OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsSettAddTrnToOpenFile >>>>> dwFileSize>0\n");

		memset(byaSettFileHeader, 0, sizeof(byaSettFileHeader));
		wLen = SOHA_WS_SETT_TRN_FILE_HEADER_LEN;
		qDmgFile_Seek(hFile, 0, DMG_FILE_SEEK_SET);
		if (!qDmgFile_Read(hFile, byaSettFileHeader, &wLen))
		{
			qDmgFile_Close(hFile);
			return FALSE;
		}

		dwCurrentTxnsCount = utlStrToDword((char*) byaSettFileHeader + SOHA_WS_SETT_TRN_FILE_TRANSACTION_COUNTER_INDEX, 5);
		gs_dwSohaWsOldTrnsCount = dwCurrentTxnsCount;
		OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsSettAddTrnToOpenFile >>>>> gs_dwSohaWsOldTrnsCount=%ld\n",gs_dwSohaWsOldTrnsCount);

	}

	if (dwFileSize == 0)
	{
		OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsSettAddTrnToOpenFile >>>>> dwFileSize=0\n");

		sprintf((char*) byaSettFileHeader,
				"%02d," // Transaction Settlement Version :2
						"%s,"// Request Id :24
						"%s,"// AgentType  :4
						"%012lu,"// Station Id :12
						"%s,"// Sim Card Id   :35
						"%02d,"// Switch type  :2
						"%05lu,"// City Id  :5
						"%s,"// Terminal Id :10
						"%s,"// MerchantId :10
						"%05d\n",// Transaction Count :5
				SOHA_WS_SETT_FILE_TRANSACTION_VERSION, byaReqestId,
				byaAgentType,
						dwStationId, (char*) byaSimCardId, bySwitchType, dwCityId, szTerminalId, szMerchantId,
				dwCurrentTxnsCount);

		qDmgFile_Seek(hFile, 0, DMG_FILE_SEEK_SET);
		if (!qDmgFile_Write(hFile, byaSettFileHeader, strlen((char*) byaSettFileHeader)))
		{
			qDmgFile_Close(hFile);
			return FALSE;
		}
		OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsSettAddTrnToOpenFile >>>>> byaSettFileHeader=%s\n",byaSettFileHeader);

	}
	memset(byaTrnSettRecord, 0, sizeof(byaTrnSettRecord));


	sprintf((char*)byaTrnSettRecord,
		"%s,"      // Transaction Type :6
		"%06lu,"    // Device Transaction Num : 6
		"%012lu,"   // Transaction Amount :12
		"%03d,"     // Discount Percent :3
		"%04lu,"    // Discount Type :4
		"%s%s,"     // Transaction Date Time :14
		"%02d,"     // Payment Type :2
		"%s,"       // Payment Information :20; it's 12 char in document, change doc
		"%012lu,"   // LastBalance :12
		"%012lu,"   // New Balance :12
		"%012lu,"   // Card Deposit Amount :12
		"%s,"       // Card Serial Number :21
		"%05lu,"    // Card Contract Type :5
		"%05lu,"    // Card Tag Type :5
		"%010lu,"   // Card Sequence Number : 10
		"%s,"  		// Driver Code :20
		"%s,"  		// Route Code :10
		"%02X%02X%02X,"  //SAM Id :6
		"%08d,"		// Exp Date  :14
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
		"%09lu,"  // CashCredit:9
		"%s,"       // mobile num:11
		"%s,"       // latitude :20
        "%s,"		// longitude:20
        "00000000000000000000,"	// NationalCode:20
        "%s,"   	// TrackingCode:12
        "%05d,"	// ResultCode:5
        "00000\n",	// productCode:5
				i_sTrnInfo->m_byaTransactionType,
				sSettConfig.m_dwTrnCounter,
				i_sTrnInfo->m_dwAmount,
				i_sTrnInfo->m_dwDiscountPercent,
				i_sTrnInfo->m_dwDiscountType,
				szDateStr,	szTimeStr,
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


	if (dwFileSize == 0)
		qDmgFile_Seek(hFile, 0, DMG_FILE_SEEK_END);

	else
		qDmgFile_Seek(hFile, dwFileSize, DMG_FILE_SEEK_SET);

	OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsSettAddTrnToOpenFile > byaTrnSettRecord content:\n%sEND\n", byaTrnSettRecord);
	OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsSettAddTrnToOpenFile > Transaction Length = %d, real Transaction length = %d", SOHA_WS_SETT_TRN_FILE_RECORD_LEN, strlen(byaTrnSettRecord));

	if (!qDmgFile_Write(hFile, (BYTE*) byaTrnSettRecord, SOHA_WS_SETT_TRN_FILE_RECORD_LEN))
	{
		qDmgFile_Close(hFile);
		return FALSE;
	}

	OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsSettAddTrnToOpenFile > Data written into TRN FILE successfully.\n");
	dwCurrentTxnsCount++;

	memset(byaSettFileHeader, 0, sizeof(byaSettFileHeader));
	sprintf((char*) byaSettFileHeader, "%05lu", dwCurrentTxnsCount);
	OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c >update header with dwCurrentTxnsCount:%d,%s\n",dwCurrentTxnsCount,byaSettFileHeader);
	qDmgFile_Seek(hFile, SOHA_WS_SETT_TRN_FILE_TRANSACTION_COUNTER_INDEX, DMG_FILE_SEEK_SET);
	if (!qDmgFile_Write(hFile, byaSettFileHeader, 5))
	{
		qDmgFile_Close(hFile);
		return FALSE;
	}
	OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsSettAddTrnToOpenFile > Header has been updated\n");

	//Additional check to make sure "," exists before "Transaction counter"
	memset(byaSettFileHeader, 0, sizeof(byaSettFileHeader));
	wLen = 4;
	qDmgFile_Seek(hFile, SOHA_WS_SETT_TRN_FILE_TRANSACTION_COUNTER_INDEX-2, DMG_FILE_SEEK_SET);
	if (!qDmgFile_Read(hFile, byaSettFileHeader, &wLen))
	{
		qDmgFile_Close(hFile);
		return FALSE;
	}
	if(strchr(byaSettFileHeader,',')==NULL){
		OsLog(LOG_DEBUG, "\n!!!!=> \",\" can not be found!!!!\n");
		qDmgFile_Seek(hFile, SOHA_WS_SETT_TRN_FILE_TRANSACTION_COUNTER_INDEX-1, DMG_FILE_SEEK_SET);
		if (!qDmgFile_Write(hFile, ",", 1))
		{
			qDmgFile_Close(hFile);
			return FALSE;
		}
		OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > comma was inserted\n");

	}


	/////////////////////////////////////////////////////////////////


	qDmgFile_Close(hFile);

	if (SohaWsSettOpenFileIsFull(SOHA_WS_SETT_OPEN_TRN_FILE_NAME))
	{
		OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsSettAddTrnToOpenFile > File is FUll! Going to add to archive.");
		if (!SohaWsSettArchiveOpenFile(NULL))
		{
			OsLog(LOG_DEBUG, "@SohaWebServiceTrnFile.c > SohaWsSettAddTrnToOpenFile > Opening Archive Failed! \n");
			return FALSE;
		}
	}

	SohaWsSettIncTrnCounter();

	return TRUE;
}

BYTE SohaWsSettArchiveFile(char* i_szFilename, char* i_szArchivedFile)
{
	HFILE hFile = 0;
	DWORD dwFileSize = 0;
	DWORD dwIndex = 0;
	WORD wLen = 0;
	char szArchiveFilename[20] = { 0 };
	BYTE byaBuffer[SOHA_WS_SETT_SEGMENT_SIZE] = { 0 };

	strcpy(szArchiveFilename, i_szArchivedFile);
	utlMakeUpperStr(szArchiveFilename);

	dwFileSize = qDmgFile_GetSize(i_szFilename);
	if (dwFileSize <= 0)
		return FALSE;

	if (!qDmgFile_Open(i_szFilename, DMG_FILE_OPEN_MODE_READ, &hFile))
		return FALSE;

	while (dwIndex < dwFileSize)
	{
		qDmgFile_Seek(hFile, dwIndex, DMG_FILE_SEEK_SET);
		wLen = (dwFileSize - dwIndex > SOHA_WS_SETT_SEGMENT_SIZE) ? SOHA_WS_SETT_SEGMENT_SIZE : (WORD) (dwFileSize - dwIndex);
		dwIndex += SOHA_WS_SETT_SEGMENT_SIZE;
		qDmgFile_Read(hFile, byaBuffer, &wLen);
	}

	qDmgFile_Close(hFile);

	return TRUE;
}

BYTE SohaWsSettRemoveFileFromArchiveTable(char *i_szFileName)
{
	HFILE hFile = 0;
	HFILE hFileTemp = 0;
	WORD wLen = 0;
	DWORD dwCounter = 0;
	DWORD dwFileSize = 0;
	sohaWsSettArchivedFile sArchiveFile;

	memset(&sArchiveFile, 0, sizeof(sohaWsSettArchivedFile));

	dwFileSize = qDmgFile_GetSize(SOHA_WS_SETT_ARCHIVE_TABLE_FILE_NAME);
	if (dwFileSize == 0)
		return FALSE;

	if (!SohaWsSettCheckArchivedFileTable(i_szFileName))
		return TRUE;

	if (!qDmgFile_Open(SOHA_WS_SETT_ARCHIVE_TABLE_FILE_NAME, DMG_FILE_OPEN_MODE_READ, &hFile))
		return FALSE;

	BYTE byOpenFileMode = 0;
	dwFileSize = qDmgFile_GetSize("TEMP.DAT");

	if (dwFileSize == 0)
		byOpenFileMode = DMG_FILE_OPEN_MODE_CREATE;
	else
		byOpenFileMode = DMG_FILE_OPEN_MODE_UPDATE;

	if (!qDmgFile_Open("TEMP.DAT", byOpenFileMode, &hFileTemp))
	{
		qDmgFile_Close(hFile);
		return FALSE;
	}

	for (dwCounter = 0; dwCounter < dwFileSize / sizeof(sohaWsSettArchivedFile); dwCounter++)
	{
		memset(&sArchiveFile, 0, sizeof(sohaWsSettArchivedFile));
		qDmgFile_Seek(hFile, dwCounter * sizeof(sohaWsSettArchivedFile), DMG_FILE_SEEK_SET);
		wLen = sizeof(sohaWsSettArchivedFile);
		if (!qDmgFile_Read(hFile, (BYTE*) &sArchiveFile, &wLen))
		{
			qDmgFile_Close(hFile);
			qDmgFile_Close(hFileTemp);
			qDmgFile_Remove("TEMP");
			return FALSE;
		}

		if (strcmp(sArchiveFile.m_szOrgFileName, i_szFileName) != 0)
		{
			qDmgFile_Seek(hFileTemp, 0, DMG_FILE_SEEK_END);
			if (!qDmgFile_Write(hFileTemp, (BYTE*) &sArchiveFile, sizeof(sohaWsSettArchivedFile)))
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

	if (qDmgFile_GetSize("TEMP.DAT") <= 0)
		qDmgFile_Remove(SOHA_WS_SETT_ARCHIVE_TABLE_FILE_NAME);
	else
		qDmgFile_Rename("TEMP.DAT", SOHA_WS_SETT_ARCHIVE_TABLE_FILE_NAME);
	return TRUE;
}

BYTE SohaWsSettCheckArchivedFileTable(char *i_szFileName)
{
	BYTE byFound = FALSE;
	HFILE hFile = 0;
	WORD wLen = 0;
	DWORD dwCounter = 0;
	DWORD dwFileSize = 0;
	sohaWsSettArchivedFile sArchivedFile;

	memset(&sArchivedFile, 0, sizeof(sohaWsSettArchivedFile));
	dwFileSize = qDmgFile_GetSize(SOHA_WS_SETT_ARCHIVE_TABLE_FILE_NAME);
	if (dwFileSize == 0)
		return FALSE;

	if (!qDmgFile_Open(SOHA_WS_SETT_ARCHIVE_TABLE_FILE_NAME, DMG_FILE_OPEN_MODE_READ, &hFile))
		return FALSE;

	for (dwCounter = 0; dwCounter < dwFileSize / sizeof(sohaWsSettArchivedFile); dwCounter++)
	{
		memset(&sArchivedFile, 0, sizeof(sohaWsSettArchivedFile));
		qDmgFile_Seek(hFile, dwCounter * sizeof(sohaWsSettArchivedFile), DMG_FILE_SEEK_SET);
		wLen = sizeof(sohaWsSettArchivedFile);
		if (!qDmgFile_Read(hFile, (BYTE*) &sArchivedFile, &wLen))
			break;
		if (strcmp(sArchivedFile.m_szOrgFileName, i_szFileName) == 0)
		{
			byFound = TRUE;
			break;
		}
	}

	qDmgFile_Close(hFile);
	return byFound;
}

BYTE SohaWsSettAddFileToArchiveFileTable(char* i_szFileName)
{
	HFILE hFile = 0;
	sohaWsSettArchivedFile sArchivedFile;
	memset(&sArchivedFile, 0, sizeof(sohaWsSettArchivedFile));
	strcpy(sArchivedFile.m_szOrgFileName, i_szFileName);

	if (SohaWsSettCheckArchivedFileTable(i_szFileName))
		return TRUE;

	if (dmgFile_GetSize(SOHA_WS_SETT_ARCHIVE_TABLE_FILE_NAME) == 0)
	{
		if (!dmgFile_Open(SOHA_WS_SETT_ARCHIVE_TABLE_FILE_NAME, DMG_FILE_OPEN_MODE_CREATE, &hFile))
			return FALSE;
	}
	else
	{
		if (!dmgFile_Open(SOHA_WS_SETT_ARCHIVE_TABLE_FILE_NAME, DMG_FILE_OPEN_MODE_UPDATE, &hFile))
			return FALSE;
	}

	dmgFile_Seek(hFile, 0, DMG_FILE_SEEK_END);
	if (!dmgFile_Write(hFile, (BYTE*) &sArchivedFile, sizeof(sohaWsSettArchivedFile)))
	{
		dmgFile_Close(hFile);
		return FALSE;
	}

	dmgFile_Close(hFile);
	return TRUE;
}

BYTE SohaWsSettArchiveOpenFile(char* o_szArchiveName)
{
	char szFileName[50] = { 0 };
	char szReserveFileName[50] = { 0 };
	char szArchiveFileName[20] = { 0 };
	HFILE hFile = 0;

	strcpy(szFileName, SOHA_WS_SETT_OPEN_TRN_FILE_NAME);
	strcpy(szReserveFileName, SOHA_WS_SETT_RESERVE_TRN_FILE_NAME);

	OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsSettArchiveOpenFile > Let's archive open file...");

	if (qDmgFile_GetSize(szFileName) <= 0)
		return TRUE;

	if (!SohaWsSettGetArchiveFileName(szFileName, szArchiveFileName))
		return FALSE;

	if (!SohaWsSettAddFileToArchiveFileTable(szArchiveFileName))
		return FALSE;

	qDmgFile_Remove(szReserveFileName);
	qDmgFile_Remove(szArchiveFileName);
	qDmgFile_Rename(szFileName, szArchiveFileName);
	qDmgFile_Remove(szFileName);

	SohaWsSettIncTrnFileCounter();

	if (o_szArchiveName)
		strcpy(o_szArchiveName, szArchiveFileName);

	return TRUE;
}

BYTE SohaWsSettGetArchiveFileName(char* i_szFileName, char* o_szArchiveFileName)
{
	BYTE byaDateTimeStr[15] = { 0 };
	BYTE byaBuffer[1024] = { 0 };
	WORD wLen = 0;
	HFILE hFile = 0;
	OsLog(LOG_DEBUG, "\n@SohaWsSettGetArchiveFileName\n");

	if (!qDmgFile_Open(i_szFileName, DMG_FILE_OPEN_MODE_READ, &hFile))
		return FALSE;

	OsLog(LOG_DEBUG, "\n@SohaWsSettGetArchiveFileName #1\n");

	qDmgFile_Seek(hFile, 0, DMG_FILE_SEEK_SET);

	////////////
	//@AggHere
	if (g_stGlVar.m_sTerminalConfig.m_byQCardEnable) {
		wLen = SOHA_WS_SETT_TRN_FILE_HEADER_LEN;
		if (!qDmgFile_Read(hFile, (BYTE*) byaBuffer, &wLen))
		{
			qDmgFile_Close(hFile);
			return FALSE;
		}
		OsLog(LOG_DEBUG, "@@SohaWsSettGetArchiveFileName> QCard: byaBuffer= %s", byaBuffer);

		qDmgFile_Close(hFile);
		strncpy((char*) byaDateTimeStr, (char*) byaBuffer + SOHA_WS_SETT_HEADER_DATE_TIME_INDEX + 1, 14);
		sprintf(o_szArchiveFileName, "%s.DAT", byaDateTimeStr);
	}
	else{
		wLen = SETT_TRN_FILE_HEADER_LEN;
		if (!qDmgFile_Read(hFile, (BYTE*) byaBuffer, &wLen))
		{
			qDmgFile_Close(hFile);
			return FALSE;
		}
		OsLog(LOG_DEBUG, "@@SohaWsSettGetArchiveFileName> Tehran: byaBuffer= %s", byaBuffer);

		qDmgFile_Close(hFile);
		strncpy((char*) byaDateTimeStr, (char*) byaBuffer + SETT_HEADER_DATE_TIME_INDEX - 2 , 14);
		sprintf(o_szArchiveFileName, "%s.DAT", byaDateTimeStr);
	}
	return TRUE;
}

BYTE SohaWsSettCheckFileExistsToSend(void)
{

	BYTE byOpenFileExistance = 0;
	DWORD dwArchiveFileCount = 0;
	char* dirPath=".";

	if(dmgFile_GetSize(SOHA_WS_SETT_OPEN_TRN_FILE_NAME) > 0)
		byOpenFileExistance = TRUE;

	dwArchiveFileCount = utlDirectoryFileCount(dirPath,".DAT");
//	dwArchiveFileCount = dwArchiveFileCount + utlDirectoryFileCount(dirPath,".uz");

	OsLog(LOG_DEBUG, "dwArchiveFileCount = %d", dwArchiveFileCount);
	OsLog(LOG_DEBUG, "byOpenFileExistance = %d", byOpenFileExistance);



	if((byOpenFileExistance) || (dwArchiveFileCount > 0))
		return TRUE;

	return FALSE;
}

BYTE SohaWsSettGetConfigFromFile(SohaWsSettConfig* o_sSettConfig)
{
	WORD wLen = 0;
	HFILE hFile = 0;
	BYTE byRet = FALSE;
	BYTE byRetVal = 0;
	OsLog(LOG_DEBUG, "@SohaWsSettGetConfigFromFile #1");

	memset(o_sSettConfig, 0, sizeof(SohaWsSettConfig));

	if (!qDmgFile_Open(SOHA_WS_SETT_CONFIG_FILE_NAME, DMG_FILE_OPEN_MODE_READ, &hFile))
		return FALSE;
	OsLog(LOG_DEBUG, "@SohaWsSettGetConfigFromFile #2");

	qDmgFile_Seek(hFile, 0, DMG_FILE_SEEK_SET);
	wLen = sizeof(SohaWsSettConfig);
	OsLog(LOG_DEBUG, "sizeof(SohaWsSettConfig)=%d",sizeof(SohaWsSettConfig));

	byRetVal = qDmgFile_Read(hFile, (BYTE*) o_sSettConfig, &wLen);
	qDmgFile_Close(hFile);

	OsLog(LOG_DEBUG, "@SohaWsSettGetConfigFromFile #3 => let's check");
	OsLog(LOG_DEBUG, "o_sSettConfig->m_dwTrnCounter=%ld,\no_sSettConfig->m_dwTrnFileCounter=%ld",o_sSettConfig->m_dwTrnCounter,o_sSettConfig->m_dwTrnFileCounter);


	if (byRetVal)
		byRet = TRUE;
	else
		byRet = FALSE;

	return byRet;
}

BYTE SohaWsSettLoadConfig(void)
{
	BYTE byRetVal = 0;
	SohaWsSettConfig sSettConfig;
	OsLog(LOG_DEBUG, "@SohaWebServiceTrnFile.c > SohaWsSettLoadConfig #1,gs_dwSohaWsLoadSettConfigFlag=%ld\n",gs_dwSohaWsLoadSettConfigFlag);
	OsLog(LOG_DEBUG, "\nqDmgFile_GetSize(SOHA_WS_SETT_CONFIG_FILE_NAME)=%ld ",qDmgFile_GetSize(SOHA_WS_SETT_CONFIG_FILE_NAME));

	memset(&sSettConfig, 0, sizeof(SohaWsSettConfig));

	if (!gs_dwSohaWsLoadSettConfigFlag)
	{
		if (qDmgFile_GetSize(SOHA_WS_SETT_CONFIG_FILE_NAME) <= 0)
		{
			OsLog(LOG_DEBUG, "SohaWsSettLoadConfig> SOHA_WS_SETT_CONFIG_FILE_NAME does not exist!");

			sSettConfig.m_dwTrnCounter = 1;
			sSettConfig.m_dwTrnFileCounter = 1;
			byRetVal = SohaWsSettUpdateConfig(&sSettConfig);
		}
		else
		{
			OsLog(LOG_DEBUG, "SohaWsSettLoadConfig> SOHA_WS_SETT_CONFIG_FILE_NAME exists! let's load sSettConfig");
			byRetVal = SohaWsSettGetConfigFromFile(&sSettConfig);
		}

		if (byRetVal)
		{
			memcpy(&gs_swSettConfig, &sSettConfig, sizeof(SohaWsSettConfig));
			gs_dwSohaWsLoadSettConfigFlag = TRUE;
		}
	}

	return gs_dwSohaWsLoadSettConfigFlag;
}

BYTE SohaWsSettGetConfig(SohaWsSettConfig* o_sSettConfig)
{
	if (!SohaWsSettLoadConfig())
		return FALSE;

	memcpy(o_sSettConfig, &gs_swSettConfig, sizeof(SohaWsSettConfig));

	return TRUE;
}

BYTE SohaWsSettUpdateConfig(SohaWsSettConfig* i_sSettConfig)
{
	WORD wLen = 0;
	HFILE hFile = 0;
	BYTE byRet = FALSE;
	BYTE byRetVal = 0;
	OsLog(LOG_DEBUG, "@SohaWebServiceTrnFile.c > SohaWsSettUpdateConfig #1\n");

	if (qDmgFile_GetSize(SOHA_WS_SETT_CONFIG_FILE_NAME) == 0)
	{
		if (!qDmgFile_Open(SOHA_WS_SETT_CONFIG_FILE_NAME, DMG_FILE_OPEN_MODE_CREATE, &hFile))
			return FALSE;
	}
	else
	{
		if (!qDmgFile_Open(SOHA_WS_SETT_CONFIG_FILE_NAME, DMG_FILE_OPEN_MODE_UPDATE, &hFile))
			return FALSE;
	}

	qDmgFile_Seek(hFile, 0, DMG_FILE_SEEK_SET);
	wLen = sizeof(SohaWsSettConfig);
	byRetVal = qDmgFile_Write(hFile, (BYTE*) i_sSettConfig, wLen);
	qDmgFile_Close(hFile);
	if (byRetVal)
		byRet = TRUE;
	else
		byRet = FALSE;

	memcpy(&gs_swSettConfig, i_sSettConfig, sizeof(SohaWsSettConfig));
	gs_dwSohaWsLoadSettConfigFlag = TRUE;

	return byRet;
}

BYTE SohaWsSettDelLastRec(void)
{
	HFILE hFile = 0;
	DWORD dwFileSize = 0;
	BYTE byaHold[1024] = { 0 };

	dwFileSize = qDmgFile_GetSize(SOHA_WS_SETT_OPEN_TRN_FILE_NAME);
	if (dwFileSize == 0)
		return TRUE;

	if (dwFileSize <= SOHA_WS_SETT_TRN_FILE_HEADER_LEN + SOHA_WS_SETT_TRN_FILE_RECORD_LEN)
	{
		qDmgFile_Remove(SOHA_WS_SETT_OPEN_TRN_FILE_NAME);
		SohaWsSettDecTrnCounter();
		return TRUE;
	}

	if (!qDmgFile_Open(SOHA_WS_SETT_OPEN_TRN_FILE_NAME, DMG_FILE_OPEN_MODE_UPDATE, &hFile))
		return FALSE;

	if (!qDmgFile_Truncate(hFile, dwFileSize - SOHA_WS_SETT_TRN_FILE_RECORD_LEN))
	{
		qDmgFile_Close(hFile);
		return FALSE;
	}

	memset(byaHold, 0, sizeof(byaHold));
	sprintf((char*) byaHold, "%06lu\n", gs_dwSohaWsOldTrnsCount);
	qDmgFile_Seek(hFile, SOHA_WS_SETT_TRN_FILE_HEADER_LEN - 7, DMG_FILE_SEEK_SET);
	if (!qDmgFile_Write(hFile, byaHold, 6))
	{
		qDmgFile_Close(hFile);
		return FALSE;
	}

	qDmgFile_Close(hFile);
	SohaWsSettDecTrnCounter();

	return TRUE;
}

BYTE SohaWsMakeWebServiceRequest(BYTE i_byShowMsg, BYTE i_byAutoSend)
{
	OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsMakeWebServiceRequest > Execution started. \n");
	BYTE byOpenFileExistance = 0;
	BYTE byArchiveFileCount = 0;
	BYTE byaTerminalSerialNumber[33] = { 0 };


	sohaWsSettArchivedFile sArchivedFile;

#ifndef S900_DEV
	//In S900_DEV mode (CreditSale mode) this function has already been called in a thread!
	OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile:  Disabled in S900_DEV mode... \n");
	if (i_byShowMsg )
	{
		if (getBoOperationStatus())
		{
			if (i_byShowMsg)
				uiDisplayMsgWithId(MESSAGE_PBG_OPERATION_IS_RUNNING, UI_DISPLAY_WAIT_TYPE_OK_CANCEL,
				UI_DISPLAY_MSG_TYPE_INFORM);
			return FALSE;
		}
	}
#endif

	//TermCfgSaveTerminalInternalConfig(&g_stGlVar.m_sTermIntConfig);
	memset(&sArchivedFile, 0, sizeof(sohaWsSettArchivedFile));
	if (g_stGlVar.m_sTerminalConfig.m_byQCardEnable){
		OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsMakeWebServiceRequest > Checking File existence... \n");
		if (!SohaWsSettCheckFileExistsToSend())
		{
			OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsMakeWebServiceRequest > No file exists to send. \n");
			if (i_byAutoSend)
				SohaWsSettSetAutoSendFileResult(TRUE);
			if (i_byShowMsg)
			{
				uiDisplayMsgWithId(MESSAGE_ERR_NO_WS_SETT_FILE_TO_SEND, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_INFORM);
			}
			return TRUE;
		}
		OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile:  File Existed... \n");
	}
	if (i_byAutoSend)
		SohaWsSettSetAutoSendFileResult(FALSE);

	dmgBasic_GetTerminalSerialNumber((char*) byaTerminalSerialNumber);

	if (g_stGlVar.m_sTerminalConfig.m_byQCardEnable) {

		if (qDmgFile_GetSize(SOHA_WS_SETT_OPEN_TRN_FILE_NAME) > 0)
			byOpenFileExistance = TRUE;
	}


	if (i_byShowMsg)
		uiDisplayMsgWithId(MESSAGE_SEND_WS_SETT_FILE_PLEASE_WAIT, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_INFORM);

	OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile: Let's Archive Open File.\n");
	if (byOpenFileExistance)
	{
		if (!SohaWsSettArchiveOpenFile(NULL))
		{
			OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile: Archive Open File Failed!\n");

			FinalizeSocket(&(sohaSck.SocketId));
			if (i_byShowMsg)
			{
				uiDisplayMsgWithId(MESSAGE_ERR_PREPARE_WS_SETT_FILES, UI_DISPLAY_WAIT_TYPE_OK_CANCEL,
				UI_DISPLAY_MSG_TYPE_ERROR);
			}
			return FALSE;
		}
	}
	OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile: Archive OpenFile finished successfully.\n");

	if ((SohaWsSettGetOpenFileAccessFailedStatus()) && (qDmgFile_GetSize(SOHA_WS_SETT_OPEN_TRN_FILE_NAME) <= 0))
	{
		OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile: OpenFileAccessFailed!\n");
		SohaWsSettResetOpenFileAccessFailed();
	}

	OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile: Let's send WS Archived Files!\n");

	BYTE retVal = SohaWsSettSendArchivedFiles(".DAT",NULL,i_byShowMsg,i_byAutoSend);

	OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile: WSSendArchivedFiles finished: Result=%d!\n",retVal);

	if(retVal)
	{
	if (i_byAutoSend)
		SohaWsSettSetAutoSendFileResult(TRUE);
			if (i_byShowMsg)
		uiDisplayMsgWithId(MESSAGE_SEND_WS_SETT_FILES_INFORM, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_SUCCESS);
			}
	return TRUE;
}

BYTE SohaWsSettSendArchivedFiles(char* filePattern,  char* rootFolder ,BYTE i_byShowMsg, BYTE i_byAutoSend){

	DWORD fileCount = 0 ;
	BYTE byRetVal = FALSE;
	char* dirPath = ".";

	DIR *directory;
	struct dirent *dir;
	directory = opendir(dirPath);

	if(!directory)
			return FALSE;

	 while ((dir = readdir(directory)) != NULL){

		if(utlEndsWith( dir->d_name , filePattern )){

					if(SohaWsSettSendOneFile((char*)dir->d_name, i_byShowMsg, i_byAutoSend)){
						//FIle Would Be Removed Inside settSendOneFile
						settRemoveFileFromArchiveTable((char*)dir->d_name);
						byRetVal = TRUE;
					}
		}
	 }
  closedir(directory);
	return byRetVal;
}

BYTE SohaWsSettSendOneFile(char* i_szOrgFileName,BYTE i_byShowMsg, BYTE i_byAutoSend )
{

	BYTE byArchiveFileCount = 0;
	BYTE byCounter = 0;
	BYTE byaTerminalSerialNumber[33] = { 0 };
	BYTE byResult = FALSE;
	BYTE byaAuthInfo[100] = {0};
	BYTE byaDevice[100] = {0};
	BYTE byaDeviceCode[100] = {0};
	BYTE byaAuthentication[300] = {0};
	DWORD wLen = 0;
	WORD dwFileSize = 0;
	DWORD dwTrnCount = 0;
	//HFILE hFile = 0;
	CHAR* SubStr;
	CHAR szTrnCount[6] = {0};
	BYTE* byaSendBuffer = NULL;
	BYTE* byaReceiveBuffer = NULL;
	OsLog(LOG_DEBUG, "\nInside SohaWsSettSendOneFile #1\n");

	FinalizeSocket(&(sohaSck.SocketId));
	OsLog(LOG_DEBUG, "\nInside SohaWsSettSendOneFile #2, sohaSck.SocketId=%d\n",sohaSck.SocketId);

	sohaSck.Port = g_stGlVar.m_sConnConfig.m_wSohaWebServiceServerPort;
	strcpy(sohaSck.IP, g_stGlVar.m_sConnConfig.m_byaSohaWebServiceServerIpAddr);
	sohaSck.SocketId = -1;
	OsLog(LOG_DEBUG, "\nInside SohaWsSettSendOneFile #3\n");

	OsLog(LOG_DEBUG, "\nSohaWsSettSendOneFile > Socket IP: %s PORT: %d \n", sohaSck.IP,sohaSck.Port);
	byResult = InitializeSocketWrapper(&sohaSck, NORMAL_TIMEOUT);
	OsLog(LOG_DEBUG, "\n@SohaWsMakeWebServiceRequest > Socket Initialize result = %d \n",byResult);
	if (byResult == FALSE)
	{
				OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsMakeWebServiceRequest > Socket Initialize Error.\n");
				FinalizeSocket(&(sohaSck.SocketId));
				//byResult = -1;
				return FALSE;
	}

	OsLog(LOG_DEBUG, "\nFile to send: Name=%s \n",i_szOrgFileName);

	dwFileSize = qDmgFile_GetSize(i_szOrgFileName);
	OsLog(LOG_DEBUG,"\nFileSize: %d\n",dwFileSize);

	if (dwFileSize <= 0)
	{
		OsLog(LOG_DEBUG,"\nFile does not exist!\n");
		FinalizeSocket(&(sohaSck.SocketId));
		//byResult = -1;
		return TRUE;
	}

	if (!g_stGlVar.m_sTerminalConfig.m_byQCardEnable) {
			dwFileSize-=SETT_TRN_FILE_FOOTER_LEN;
			//dwFileSize--;
			OsLog(LOG_DEBUG,"\n Not in Qcard Mode : Remove Footer Size: dwFileSize=%d\n",dwFileSize);
		}



	BYTE* byaFileBuffer = NULL;
	HFILE hTxnFile;

	byaFileBuffer = (BYTE*) malloc(dwFileSize + 100);
	memset(byaFileBuffer,0,dwFileSize + 100);
	OsLog(LOG_DEBUG,"\nInitialized byaFileBuffer:%s\n",byaFileBuffer);

	if (!dmgFile_Open(i_szOrgFileName, DMG_FILE_OPEN_MODE_READ, &hTxnFile))
	{
		OsLog(LOG_DEBUG, "\n Could not open %s!\n",	i_szOrgFileName);
		FinalizeSocket(&(sohaSck.SocketId));
		//byResult = -1;
		free(byaFileBuffer);
		return FALSE;
	}


	dwFileSize--;
	////////////

	if (!dmgFile_Read(hTxnFile, byaFileBuffer, &dwFileSize))
	{
					OsLog(LOG_DEBUG,"\n Could not Read the file: %s!\n",i_szOrgFileName);
					FinalizeSocket(&(sohaSck.SocketId));
					qDmgFile_Close(hTxnFile);
					//byResult = -1;
					free(byaFileBuffer);
					return FALSE;
	}


	OsLog(LOG_DEBUG,"\n---------Read size:%d---------------\n%s\n\n\n---------------\n",dwFileSize,byaFileBuffer);
	OsLog(LOG_DEBUG,"\n part 1 : FileBuffer=%s!\n",byaFileBuffer);
	OsLog(LOG_DEBUG,"\n part 2 : Last Charachters\nFileBuffer=%sEnd\n",byaFileBuffer+dwFileSize-10);
	qDmgFile_Close(hTxnFile);

	if (!g_stGlVar.m_sTerminalConfig.m_byQCardEnable) {
		//byaFileBuffer should be adjusted for !Qcard Mode
		memset(szTrnCount, 0, sizeof(szTrnCount));
		strncpy(szTrnCount, byaFileBuffer + SETT_HEADER_TRANSACTION_COUNTER_INDEX, SETT_HEADER_TRANSACTION_COUNTER_LENGTH);


		BYTE* byaFileBufferTeh = NULL;
		dwFileSize+=3;
		OsLog(LOG_DEBUG,"\n Not in Qcard Mode : New dwFileSize=%d\n",dwFileSize);

		byaFileBufferTeh = (BYTE*) malloc(dwFileSize + 100 );
		memset(byaFileBufferTeh,0,dwFileSize + 100);
		strcpy(byaFileBufferTeh,SETT_TEHRAN_TRANSACTION_SCHEMA_VERSION);
		strcat(byaFileBufferTeh,",");
		strcat(byaFileBufferTeh,byaFileBuffer);
		OsLog(LOG_DEBUG,"\n Tehran Mode : byaFileBufferTeh=%sEnd\n",byaFileBufferTeh);
		strcpy(byaFileBuffer,byaFileBufferTeh);
		OsLog(LOG_DEBUG,"\n Tehran Mode : byaFileBuffer=%sEnd\n",byaFileBuffer);
		free(byaFileBufferTeh);



	}
	else{
		memset(szTrnCount, 0, sizeof(szTrnCount));
		strncpy(szTrnCount, byaFileBuffer + SOHA_WS_SETT_HEADER_TRANSACTION_COUNTER_INDEX, SOHA_WS_SETT_HEADER_TRANSACTION_COUNTER_LENGTH);

	}

    // it's work currently with Curl (if you want to rewrite this section help form these code)
//    char *byaBodyResponse;
//    int responseLen;
//    int httpStatusCode = 0;
//
//    myCurlRequestWithHeader(METHOD_POST, "http://46.209.140.12:8066/api/Transaction", byaFileBuffer, &byaBodyResponse, &responseLen, &httpStatusCode, 30,
//                            1, "Content-Type", "text/plain");
//    OsLog(LOG_DEBUG, "http Status Code is %d", httpStatusCode);
//    if (byaBodyRespons)
//    {
//        OsLog(LOG_DEBUG, "body Response is %s", byaBodyRespons);
//
//    }


    OsLog(LOG_DEBUG,"\n@WS Transaction Count-> szTrnCount: %s\n",szTrnCount);
	dwTrnCount = atoi(szTrnCount);
	OsLog(LOG_DEBUG,"\n Transaction Count converted to DWORD -> dwTrnCount: %d\n",dwTrnCount);
	byaSendBuffer = (BYTE*) malloc(dwFileSize * sizeof(BYTE) + 1026);

	#ifdef PX7_DEV
				sprintf(byaDevice, "Pax:px7:");
	#endif
	#if defined(S800_DEV) || defined(S900_DEV)
				sprintf((char *) byaDevice, "Pax:S800:");
	#else
				sprintf(byaDevice, "Pax:S900:");
	#endif

	strcat((char *) byaDevice, (char *) byaTerminalSerialNumber);
	OsLog(LOG_DEBUG, "\n@Device in header is: %s\n", byaDevice);
	Base64encode((char *) byaDeviceCode, (char *) byaDevice, strlen(byaDevice));

	sprintf((char *) byaAuthInfo, "%s:%s", g_stGlVar.m_sConnConfig.m_byaWebServiceUser, g_stGlVar.m_sConnConfig.m_byaWebServicePass);
	OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsMakeWebServiceRequest > User and Pass in header is: %s\n", byaAuthInfo);
	Base64encode((char *) byaAuthentication, (char *) byaAuthInfo, strlen(byaAuthInfo));
	OsLog(LOG_DEBUG, "\n@Temp 1\n");

	sprintf(byaSendBuffer, "POST %s HTTP/1.1\r\n"
						"Host: %s:%d\r\n"
						"Device:%s\r\n"
						"Content-Type: text/plain\r\n"
						"Authorization:Basic %s\r\n"
						"Content-Length:%d "
						"\r\n\r\n%s",
						g_stGlVar.m_sConnConfig.m_byaSohaWebServicePath,
						g_stGlVar.m_sConnConfig.m_byaSohaWebServiceServerIpAddr,
						g_stGlVar.m_sConnConfig.m_wSohaWebServiceServerPort,
						byaDeviceCode,
						byaAuthentication,
						dwFileSize/*strlen(byaFileBuffer)*/,
						byaFileBuffer);
	OsLog(LOG_DEBUG, "\n@Temp 2\n");

	wLen = strlen(byaSendBuffer) + 5;
	OsLog(LOG_DEBUG, "\n@Temp 3\n");

	free(byaFileBuffer);
	OsLog(LOG_DEBUG,"\n***************\nSendBuffer length: %d\n****************\n",wLen);
	OsLog(LOG_DEBUG,"\n***************\nSendBuffer part 1:\n%s\n****************\n",byaSendBuffer);
	OsLog(LOG_DEBUG,"\n***************\nSendBuffer part 2:\n%s\n****************\n",byaSendBuffer+500);

	if (!sendDataSck(&sohaSck, byaSendBuffer, &wLen))
				{
					if (i_byShowMsg)
					{
						uiDisplayMsgWithId(MESSAGE_ERR_SEND_WS_SETT_FILES, UI_DISPLAY_WAIT_TYPE_OK_CANCEL,
						UI_DISPLAY_MSG_TYPE_ERROR);
					}

					FinalizeSocket(&(sohaSck.SocketId));
					//byResult = -1;
					OsLog(LOG_DEBUG,"\n Error in sending data.. \n");
					free(byaSendBuffer);
					return FALSE;
				}
	free(byaSendBuffer);
	OsLog(LOG_DEBUG,
						"\n@SohaWebServiceTrnFile.c > SohaWsMakeWebServiceRequest > Data was sent successfully.\n");

	wLen = 0;
	byaReceiveBuffer = (BYTE*) malloc(300 * sizeof(BYTE));
	memset(byaReceiveBuffer,0,sizeof(BYTE) * 300);
	if (!dmgConnReceiveDataSck(&sohaSck, byaReceiveBuffer, &wLen, 20))
				{

					OsLog(LOG_DEBUG,
							"\n@SohaWebServiceTrnFile.c > SohaWsMakeWebServiceRequest > Error in receiving data.\n");
					FinalizeSocket(&(sohaSck.SocketId));
					//byResult = -1;
					free(byaReceiveBuffer);
					if (i_byShowMsg)
					{
						uiDisplayMsgWithId(MESSAGE_ERR_SEND_WS_SETT_FILES, UI_DISPLAY_WAIT_TYPE_OK_CANCEL,
						UI_DISPLAY_MSG_TYPE_ERROR);
					}
					return FALSE;
				}

				OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsMakeWebServiceRequest > Received data -> byaReceiveBuffer: \n%s \n",
						byaReceiveBuffer);

				if (memcmp(byaReceiveBuffer + 9, "200", 3) != 0)
				{
					FinalizeSocket(&sohaSck.SocketId);
					//byResult = -1;
					OsLog(LOG_DEBUG,
							"\n@SohaWebServiceTrnFile.c > SohaWsMakeWebServiceRequest > Successful (200) was not received in the response.\n");
					free(byaReceiveBuffer);
					if (i_byShowMsg)
					{
						uiDisplayMsgWithId(MESSAGE_ERR_SEND_WS_SETT_FILES, UI_DISPLAY_WAIT_TYPE_OK_CANCEL,
						UI_DISPLAY_MSG_TYPE_ERROR);
					}
					return FALSE;
				}
				OsLog(LOG_DEBUG,
						"\n@SohaWebServiceTrnFile.c > SohaWsMakeWebServiceRequest > Successful (200) was received in the response. \n");

				DWORD dwResIndex = 20;
				while (!strstr(byaReceiveBuffer, "\r\n\r\n"))
				{
					if (!dmgConnReceiveDataSck(&sohaSck, byaReceiveBuffer + dwResIndex, &wLen, 1))
					{
						OsLog(LOG_DEBUG,"\n@SohaWebServiceTrnFile Error in receiving the rest of data #1\n");

						FinalizeSocket(&(sohaSck.SocketId));
						//byResult = -1;
						free(byaReceiveBuffer);
						if (i_byShowMsg)
						{
							uiDisplayMsgWithId(MESSAGE_ERR_SEND_WS_SETT_FILES, UI_DISPLAY_WAIT_TYPE_OK_CANCEL,
							UI_DISPLAY_MSG_TYPE_ERROR);
						}
						return FALSE;
					}
					dwResIndex++;
				}
				OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsMakeWebServiceRequest #1 > byaReceiveBuffer: \n%s \n", byaReceiveBuffer);

				if (!dmgConnReceiveDataSck(&sohaSck, byaReceiveBuffer + dwResIndex, &wLen, 10))
				{
					OsLog(LOG_DEBUG,"\n@SohaWebServiceTrnFile Error in receiving the rest of data #2\n");

					FinalizeSocket(&(sohaSck.SocketId));
					//byResult = -1;

					free(byaReceiveBuffer);
					if (i_byShowMsg)
					{
						uiDisplayMsgWithId(MESSAGE_ERR_SEND_WS_SETT_FILES, UI_DISPLAY_WAIT_TYPE_OK_CANCEL,
						UI_DISPLAY_MSG_TYPE_ERROR);
					}
					return FALSE;
				}
				OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsMakeWebServiceRequest #2 > byaReceiveBuffer: \n%s \n", byaReceiveBuffer);

				SubStr = utlMYstrstr(byaReceiveBuffer, "OK");
				if(strstr(byaReceiveBuffer, "OK") == NULL)
				{
					OsLog(LOG_DEBUG,"\n@SohaWebServiceTrnFile.c > SohaWsMakeWebServiceRequest > Response does not contain OK!\n");
					FinalizeSocket(&(sohaSck.SocketId));
					//byResult = -1;
					free(byaReceiveBuffer);
					if (i_byShowMsg)
					{
						uiDisplayMsgWithId(MESSAGE_ERR_SEND_WS_SETT_FILES, UI_DISPLAY_WAIT_TYPE_OK_CANCEL,
						UI_DISPLAY_MSG_TYPE_ERROR);
					}
					return FALSE;
				}
				OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile SUCCESSFUL...\n");

				OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsMakeWebServiceRequest > Removing the sent data...\n");
				free(byaReceiveBuffer);
		if (!qDmgFile_Remove(i_szOrgFileName))
					OsLog(LOG_DEBUG, "\n qDmgFile_Remove GONE WRONG...\n");

				//qDmgFile_Close(hTxnFile);
				//OsLog(LOG_DEBUG, "\ntruncate file %d len %d\n",hTxnFile,(byArchiveFileCount - byCounter - 1) * sizeof(sohaWsSettArchivedFile));
//		if(!qDmgFile_Truncate(hFile, (byArchiveFileCount - byCounter - 1) * sizeof(sohaWsSettArchivedFile)))
//			OsLog(LOG_DEBUG, "\n@SohaWebServiceTrnFile.c > SohaWsMakeWebServiceRequest > qDmgFile_Truncate GONE WRONG....\n");
		//}

		//qDmgFile_Close(hFile);
		FinalizeSocket(&(sohaSck.SocketId));
}

//#endif
