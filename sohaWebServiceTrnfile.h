/*
 * sohaWebServiceTrnfile.h
 *
 *  Created on: Aug 26, 2019
 *      Author: f.fereydounian
 */

#ifndef SOHAWEBSERVICETRNFILE_H_
#define SOHAWEBSERVICETRNFILE_H_

#include <define.h>
#include <DeviceMng/dmgBasic.h>
#include <Utility/utility.h>
#include <Message/message.h>
#include <UI/uiDisplay.h>
#ifdef PX7_DEV
#include <Sam/sohaSam.h>
#endif
#include <DeviceMng/dmgConnection.h>
#include <QCard/qCardFileOperationWrapper.h>
#if defined(S800_DEV) || defined (S900_DEV)
#include "DesfireSam/sSam.h"
#endif


#define SUBWAY_AGENT	"1111"
#define BUS_AGENT		"1112"
#define TAXI_AGENT		"1113"
#define QOM_AGENT		"1114"

#define SOHA_WS_SETT_FILE_TRANSACTION_VERSION		1
#define SOHA_WS_SETT_FILE_TYPE_TRANSACTION			1
#define SOHA_WS_SETT_FILE_TYPE_SHIFT_EVENTS			2

#define SOHA_WS_SETT_TRANSACTION_TYPE_PAYMENT		1
#define SOHA_WS_SETT_TRANSACTION_TYPE_REJECT		2
#define SOHA_WS_SETT_TRANSACTION_TYPE_BLOCK			3
#define SOHA_WS_SETT_TRANSACTION_TYPE_BLOCK_TR		4

#define SOHA_WS_SETT_EVENT_TYPE_OPEN_SHIFT			5001
#define SOHA_WS_SETT_EVENT_TYPE_END_SHIFT			5014
#define SOHA_WS_SETT_EVENT_TYPE_SET_ROUTE			9001
#define SOHA_WS_SETT_EVENT_TYPE_CHANGE_CONFIG		9002




#define SOHA_WS_SETT_ARCHIVE_TABLE_FILE_NAME	"./data/SOHAWSSETTAFT.DAT"
#define SOHA_WS_SETT_CONFIG_FILE_NAME			"./data/SOHAWSSETTAFC.DAT"
#define SOHA_WS_SETT_OPEN_TRN_FILE_NAME		 	"./data/SOHAWSSETTOF.DAT"
#define SOHA_WS_SETT_RESERVE_TRN_FILE_NAME		"./data/SOHAWSSETTRF.DAT"
#define SOHA_WS_SETT_MAX_FILE_SIZE_FOR_STORE		3 * 1024

#define SOHA_WS_SETT_MAX_FILE_SIZE_FOR_STORE	30 * 1024
#define SOHA_WS_SETT_MAX_FILE_SIZE_FOR_ZIP		31 * 1024
#define SOHA_WS_SETT_SEGMENT_SIZE				512 * 10
#define SOHA_WS_SETT_TRN_FILE_HEADER_LEN		119
#define SOHA_WS_SETT_TRN_FILE_TRANSACTION_COUNTER_INDEX		113
#define SOHA_WS_SETT_TRN_FILE_RECORD_LEN		475 // 447 //388    //6+6+12+3+12+14+2+20+12+12+12+21+6+10+4+12+12+6+12+12+2+12+1 +22  +20 + 41 + TracingCode(12) + ResultCode(5) + productCode(5)
#ifndef T610_DEV
#define SOHA_WS_SETT_HEADER_DATE_TIME_INDEX		12      //TO BE CHANGED
#define SOHA_WS_SETT_HEADER_TRANSACTION_COUNTER_INDEX	113
#define SOHA_WS_SETT_HEADER_TRANSACTION_COUNTER_LENGTH	5
#else
#define SOHA_WS_SETT_HEADER_DATE_TIME_INDEX		13      //TO BE CHANGED
#endif

#ifdef S900_DEV
extern BYTE 		g_byaChangeTransferDestinationMoNumber[12];
#endif

typedef struct
{
	char	m_szOrgFileName[20];
#ifdef S800_DEV
	DWORD   m_dwTrnCount;
#endif
}sohaWsSettArchivedFile;


typedef struct
{
	DWORD	m_dwTrnCounter;
	DWORD	m_dwTrnFileCounter;
}SohaWsSettConfig;
typedef struct
{
	DWORD	m_dwLastBalance;
	DWORD	m_dwCurrentBalance;
	DWORD	m_dwDeposit;
	BYTE	m_byaSerial[20];      //Card Serial  or UID Reverse  or QR serial
	DWORD	m_dwProductId;        // Card ContractType
	DWORD	m_dwTagType;
	DWORD	m_dwSeqNumber;
	DWORD	m_dwExpirationDate;
	DWORD	m_dwDecrementalCounter;
	DWORD	m_dwTransactionPointer;
	BYTE	m_byaTrnSignature[33];   //Card Validation Signature Flag
	WORD	m_wTravelsCount;
	WORD	m_wDiscountTravelsCount;
	WORD	m_wDiscountExpireDate;

}SohaWsSettProductInfo;
typedef struct
{
	BYTE	m_byaTransactionType[7];
	DWORD	m_dwAmount;
	DWORD	m_dwDiscountPercent;
	DWORD	m_dwDiscountType;
	DWORD	m_dwDate;
	DWORD	m_dwTime;
	BYTE	m_byPaymentType;
	BYTE	m_byaPaymentInfo[21];
	BYTE	m_byServiceCode;
	DWORD 	m_dwServiceDuration;

	//BYTE	m_byEventCode[7];
	DWORD	m_dwShiftId;
	DWORD	m_byHalfWayNum;
	DWORD	m_byVehicleNum;
	BYTE	m_byaSellerId[18];
	DWORD	m_dwCurrentCrdit;
	DWORD	m_dwCashCredit;

	// new field
    BYTE    m_byaTrackingCode[12+1]; // 12 + 1(null) for Credit service use to return tracking code that received from server
	DWORD   m_dwResult; // for Credit Service shows the result was success or failure (0unknown 1success 2>failure)

}SohaWsSettTrnInfo;


typedef struct{
	BYTE	byaTransactionContent[320];
} SohaWsTransactionRecord;





void SohaWsSettSetAutoSendFileResult(BYTE i_byStatus);
BYTE SohaWsSettGetAutoSendFileResult();
void SohaWsSettBlockOpenFileAccess();
void SohaWsSettReleaseOpenFileAccess();
BYTE SohaWsSettGetOpenFileAccessBlockStatus();
void SohaWsSettSetOpenFileAccessFailed();
void SohaWsSettResetOpenFileAccessFailed();
BYTE SohaWsSettGetOpenFileAccessFailedStatus();
BYTE SohaWsSettArchiveFile(char* i_szFilename, char* i_szArchivedFile);
BYTE SohaWsSettRemoveFileFromArchiveTable(char *i_szFileName);
BYTE SohaWsSettCheckArchivedFileTable(char *i_szFileName);
BYTE SohaWsSettAddFileToArchiveFileTable(char* i_szFileName);
BYTE SohaWsSettArchiveOpenFile( char* o_szArchiveName);
BYTE SohaWsSettCheckFileExistsToSend(void);
BYTE SohaWsSettGetConfigFromFile(SohaWsSettConfig* o_sSettConfig);
BYTE SohaWsSettLoadConfig(void);
BYTE SohaWsSettGetConfig(SohaWsSettConfig* o_sSettConfig);
BYTE SohaWsSettUpdateConfig(SohaWsSettConfig* i_sSettConfig);
BYTE SohaWsSettGetArchiveFileName(char* i_szFileName, char* o_szArchiveFileName);
BYTE SohaWsMakeWebServiceRequest(BYTE i_byShowMsg, BYTE i_byAutoSend);
BYTE SohaWsSettOpenFileIsFull(char* i_szFileName);
BYTE SohaWsSettAddTrnToOpenFile(SohaWsSettProductInfo* i_sProductInfo, SohaWsSettTrnInfo* i_sTrnInfo);
BYTE SohaWsSettSendArchivedFiles(char* filePattern,  char* rootFolder ,BYTE i_byShowMsg, BYTE i_byAutoSend);
BYTE SohaWsSettSendOneFile(char* i_szOrgFileName,BYTE i_byShowMsg, BYTE i_byAutoSend );
BYTE SohaWsSettDelLastRec(void);

#endif /* SOHAWEBSERVICETRNFILE_H_ */
