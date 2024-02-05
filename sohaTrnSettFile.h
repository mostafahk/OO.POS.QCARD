//
// Created by f.karamiyar on 8/14/2019.
//

#ifndef PAYMENT_SOHATRNSETTFILE_H
#define PAYMENT_SOHATRNSETTFILE_H




#include <define.h>
#include <DeviceMng/dmgBasic.h>
#include <Utility/utility.h>
#include <BZ2/bzlib.h>
#ifndef T610_DEV
#include <Message/message.h>
#include <DeviceMng/dmgFile.h>
#include <Ftp/ftp.h>
#include <UI/uiDisplay.h>
#else
#include "terminalConfig.h"
#endif
#ifdef PX7_DEV
#include <Sam/sohaSam.h>
#endif
#ifdef S800_DEV
#include <DesfireSam/sSam.h>
#endif
#define SUBWAY_AGENT	"1111"
#define BUS_AGENT		"1112"
#define TAXI_AGENT		"1113"
#define QOM_AGENT		"1114"

#define SOHA_SETT_FILE_TRANSACTION_VERSION		1
#define SOHA_SETT_FILE_TYPE_TRANSACTION			1
#define SOHA_SETT_FILE_TYPE_SHIFT_EVENTS		2

#define SOHA_SETT_TRANSACTION_TYPE_PAYMENT		1
#define SOHA_SETT_TRANSACTION_TYPE_REJECT		2
#define SOHA_SETT_TRANSACTION_TYPE_BLOCK		3
#define SOHA_SETT_TRANSACTION_TYPE_BLOCK_TR		4

#define SOHA_SETT_EVENT_TYPE_OPEN_SHIFT			5001
#define SOHA_SETT_EVENT_TYPE_END_SHIFT			5014
#define SOHA_SETT_EVENT_TYPE_SET_ROUTE			9001
#define SOHA_SETT_EVENT_TYPE_CHANGE_CONFIG		9002

#ifndef T610_DEV
#define SOHA_SETT_ARCHIVE_TABLE_FILE_NAME	 "./data/SOHASETTAFT.DAT"
#define SOHA_SETT_CONFIG_FILE_NAME			 "./data/SOHASETTAFC.DAT"
#define SOHA_SETT_OPEN_TRN_FILE_NAME		 "./data/SOHASETTOF.DAT"
#define SOHA_SETT_RESERVE_TRN_FILE_NAME		 "./data/SOHASETTRF.DAT"
#else
#define SOHA_SETT_ARCHIVE_TABLE_FILE_NAME	 "SOHASETTAFT.DAT"
#define SOHA_SETT_CONFIG_FILE_NAME			 "SOHASETTAFC.DAT"
#define SOHA_SETT_OPEN_TRN_FILE_NAME		 "SOHASETTOF.DAT"
#define SOHA_SETT_RESERVE_TRN_FILE_NAME		 "SOHASETTRF.DAT"
#endif
#define SOHA_SETT_MAX_FILE_SIZE_FOR_STORE	30 * 1024
#define SOHA_SETT_MAX_FILE_SIZE_FOR_ZIP		31 * 1024
#define SOHA_SETT_SEGMENT_SIZE				512 * 10
#define SOHA_SETT_TRN_FILE_HEADER_LEN		119
#define SOHA_SETT_TRN_FILE_RECORD_LEN		475// 447 //387//312    //6+6+12+3+12+14+2+20+12+12+12+21+6+10+4+12+12+6+12+12+2+12+1 +22  +20 + 41+ TracingCode(12) + ResultCode(5) + productCode(5)
#ifndef T610_DEV
#define SOHA_SETT_HEADER_DATE_TIME_INDEX		12      //TO BE CHANGED
#else
#define SOHA_SETT_HEADER_DATE_TIME_INDEX		13      //TO BE CHANGED
#endif


typedef struct
{
	char	m_szOrgFileName[20];
#ifdef S800_DEV
	DWORD   m_dwTrnCount;
#endif
}sohaSettArchivedFile;


#ifdef S900_DEV
extern BYTE 		g_byaChangeTransferDestinationMoNumber[12];
#endif


typedef struct
{
	DWORD	m_dwTrnCounter;
	DWORD	m_dwTrnFileCounter;
}SohaSettConfig;
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

}SohaSettProductInfo;


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
    
}SohaSettTrnInfo;


//typedef struct
//{
//	BYTE	m_byEventCode[7];
//	BYTE	m_byPaymentType;
//	BYTE	m_byaPaymentInfo[21];
//	BYTE	m_byRedemptionPercent;
//	WORD	m_wTrnType;
//	DWORD	m_dwDate;
//	DWORD	m_dwTime;
//	DWORD	m_dwAmount;
//	BYTE	m_byaSellerId[18];
//	DWORD	m_dwStoredCredit;
//	DWORD	m_dwCurrentCrdit;
//	DWORD	m_dwShiftId;
//	BYTE	m_byServiceType;
//	DWORD	m_dwCashCredit;
//	BYTE	m_byaUserName[15];
//	BYTE	m_byaPassWord[15];
//	BYTE	m_byaEncodedRrn[15];
//}settTrnInfo;

typedef struct{
	BYTE	byaTransactionContent[320];
} SohaTransactionRecord;


BYTE SohaSettUpdateConfig(SohaSettConfig* i_sSettConfig);
BYTE SohaSettDelLastRec(void);
BYTE SohaSettAddTrnToOpenFile(SohaSettProductInfo* i_sProductInfo, SohaSettTrnInfo* i_sTrnInfo);
void SohaSettSetAutoSendFileResult(BYTE i_byStatus);
BYTE SohaSettGetAutoSendFileResult();
void SohaSettBlockOpenFileAccess();
void SohaSettReleaseOpenFileAccess();
BYTE SohaSettGetOpenFileAccessBlockStatus();
void SohaSettSetOpenFileAccessFailed();
void SohaSettResetOpenFileAccessFailed();
BYTE SohaSettGetOpenFileAccessFailedStatus();
BYTE SohaSettArchiveOpenFile( char* o_szArchiveName);
BYTE SohaSettRemoveFileFromArchiveTable(char *i_szFileName);
BYTE SohaSettArchiveFile(char* i_szFilename, char* i_szArchivedFile);
BYTE SohaSettCheckFileExistForSend(void);

#endif //PAYMENT_SOHATRNSETTFILE_H
