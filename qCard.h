/*
 * qCard.h
 *
 *  Created on: May 20, 2019
 *      Author: f.karamiyar
 */

#ifndef QCARD_H_
#define QCARD_H_


#include "define.h"
#include <Utility/utility.h>
#include <DeviceMng/dmgKeypad.h>
#include <DeviceMng/dmgDisplay.h>
#include <DeviceMng/dmgPicc.h>
#include <Mifare/mifareConfig.h>

//#define LOG_ENABLE    1


#ifdef T610_DEV
#include <QCardWrapper/qCardWrapperFunctions.h>
#include <QCardWrapper/qCardMessageWrapper.h>
#endif

#define	SECTOR_ZERO		0
#define	SECTOR_ONE		1
#define	SECTOR_TWO		2
#define	SECTOR_THREE	3
#define	SECTOR_FOUR		4
#define	SECTOR_FIVE		5
#define	SECTOR_SIX		6
#define	SECTOR_SEVEN    7
#define	SECTOR_EIGHT    8
#define	SECTOR_NINE     9

#define	BLOCK_ZERO		0
#define BLOCK_ONE		1
#define	BLOCK_TWO		2

#define QCARD_LAST_CHARGE_LOG				"./data/QLCHARGEI.TXT"

#define QCARD_TYPE_SUPPORT			1
#define QCARD_TYPE_SELLER			2
#define  QCARD                      111
typedef struct
{
	WORD	q_wTransactionSectorPointer;
	DWORD	q_dwDecrementalCounter;
	DWORD	q_dwDecrementalCounterBckUp0;
	DWORD	q_dwDecrementalCounterBckUp1 ;
	BYTE	q_wDecrementalCounterBckUpPointer;
	BYTE	q_bValueBlockIsLost;

}qCardTransactionData;


typedef struct
{
	WORD	q_wContractType;
	BYTE	q_byaUid[9];
	BYTE	q_byUidLen;
	BYTE	q_byExtraUid;
	WORD	q_wTagType;
	DWORD	q_dwDepositeValue;
	BYTE	q_byIsValid;
	DWORD	q_dwIssueDateTime;
	DWORD	q_dwIssueOperator;
	DWORD	q_dwIssueStation;
	DWORD 	q_dwIssuerMachine;
	WORD 	q_wCity;
	BYTE	q_byaCardDataSignature[16];
}qCardData;


typedef struct
{

	WORD	wTravelsCount;
	DWORD	dwDiscountAmount;
	WORD	wDiscountInitDate;
	WORD	wDiscountExpDate;

}qCardDiscountData;


typedef struct
{
	BYTE	byServiceCode;

}qCardServiceData;


typedef struct
{

	DWORD				q_dwSequenceNumber;
	DWORD 				q_dwTotalAmount;
	WORD				q_wTravelsCount;
	DWORD				q_dwTranDateTime;
	WORD 				q_wTranStation;
	BYTE				q_byTranType;
	qCardDiscountData	q_sCardDiscountData;
	qCardServiceData	q_sCardServiceData[5];
	BYTE				q_byaTrnLogSignature[16];
}qCardTrnLog;


typedef struct
{
	BYTE 	q_byaName[17];
	BYTE 	q_byaPhone[17];
	BYTE	q_byaNationalId[6];
	WORD	q_wBirthDay;
	BYTE 	q_byaPersonalId[7];

}qCardPersonalizedData;

typedef struct
{
	BYTE 	q_byaPinBlock[8];
	BYTE 	q_byaSellerLevel;


}qCardSellerData;


typedef struct
{
	qCardTransactionData	q_sCardTransactionData;           // Signature & Sector Pointer
	qCardData				q_sCardData;
	qCardTrnLog				q_sLastChargeTrnLog;
	qCardTrnLog				q_sTranLog;
	qCardPersonalizedData	q_sPersonalizedData;
	qCardTrnLog				q_sNewTrn;
	qCardSellerData         q_sqCardSellerData;
	BYTE					q_byValidSignature;
	accCardConfig			m_sCardAccInfo;
	BYTE					q_byIsIssued;
}qCard;


typedef struct
{
	BYTE	m_byPaymentType;
	BYTE	m_byPaymentInfo[21];
	qCard	m_sCardData;
	DWORD	m_dwAmount;
}qcardLastTrnLog;


DWORD qCardGetData(qCard* o_sQcard , BYTE* byaUid , BYTE byUidLen);
DWORD qCardWriteTrnLog(qCard* i_sQcard,BYTE* i_byaUid,BYTE i_byUidLen, BYTE* i_byaOnlineCardSectorData);
BYTE qCardGenerateTrnSignature(qCard* io_sQcard);
BYTE qCardUpdateLastChargeInfo(qcardLastTrnLog* i_sLastChargeInfo);
#endif /* QCARD_H_ */
