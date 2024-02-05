/*
 * qCardServiceUIWrappers.h
 *
 *  Created on: Jun 10, 2019
 *      Author: f.karamiyar
 */

#ifndef QCARDSERVICEUIWRAPPERS_H_
#define QCARDSERVICEUIWRAPPERS_H_


#include <define.h>
#include <Message/message.h>
#include <OnlineSrv/onlineSrvTransaction.h>
#include <UI/uiDisplay.h>
#include <Report/report.h>
#include <DeviceMng/dmgReceipt.h>
#include <QCard/qCard.h>

#ifndef S900_DEV
#include <Customer/cardService.h>
#endif


BYTE qCardPrinterCheckWrapper();
#ifdef S900_DEV
BYTE qCardMagCardServiceWrapper(BYTE i_byServiceType,BYTE byShowSwipe,trnRequestInfo* i_sTrnRequest,  DWORD* i_dwAmount, trnResponseInfo* i_sTrnResponse, stReceipt* i_sReceiptInfo , BYTE* byTransactionType, BYTE i_getAmountFromUser);
#else
BYTE qCardMagCardServiceWrapper(BYTE byShowSwipe, trnRequestInfo* i_sTrnRequest,  DWORD* i_dwAmount, trnResponseInfo* i_sTrnResponse, trnReceiptInfo* i_sReceiptInfo , BYTE* byTransactionType);
#endif
#ifndef S900_DEV
void qCardSuccessTrnPageWrapper(trnReceiptInfo* i_sReceiptInfo);
#else
void qCardSuccessTrnPageWrapper(stReceipt* i_sReceiptInfo);
#endif
void qCardUnSuccessTrnPageWrapper(trnRequestInfo* i_sTrnRequest, trnResponseInfo* i_sTrnResponse);
BYTE qCardIsPrinterRequiredWrapper(BYTE byTranState);
void qCardBalanceMenuWrapper(qCard* sQcard);
BYTE qCardDetectWrapper(BYTE i_byTimeoutSec, BYTE* o_byaUid,BYTE* o_byUidLen, BYTE* o_byCardType);
void qCardExaminSamCreditWrapper();

#endif /* QCARDSERVICEUIWRAPPERS_H_ */
