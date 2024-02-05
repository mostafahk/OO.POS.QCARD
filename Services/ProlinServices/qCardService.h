/*
 * qCardService.h
 *
 *  Created on: May 22, 2019
 *      Author: f.karamiyar
 */

#ifndef QCARDSERVICE_H_
#define QCARDSERVICE_H_

#include <define.h>
#include <DeviceMng/dmgPicc.h>
#include <DeviceMng/dmgKeypad.h>
#include <Message/message.h>
#include <UI/uiDisplay.h>
#include <QCard/qCard.h>
#include <QCard/Services/ProlinServices/qCardServiceUIWrappers.h>
#include <QCard/sohaTrnSettFile.h>
#include <DeviceMng/dmgDisplay.h>
#include <Report/report.h>
#ifndef S900_DEV
#include <Customer/cardService.h>

#endif
#include <Customer/customerCommonService.h>
#include <OnlineSrv/onlineSrvTransaction.h>
#include <SettFiles/TrnsSettFiles.h>
#include <DeviceMng/dmgBasic.h>


#ifdef S900_DEV
//#include <UI/uiDisplay.h>
#include <UI/uiMenu.h>
#include <Customer/customerService.h>
#include <Log/Log.h>
#include "seller/sellerService.h"
#include <Shift/shiftMng.h>
#endif



#define SUCCESSFUL									0
#define QCARD_SERVICE_ERRCODE_TIMEOUT				100
#define QCARD_SERVICE_ERRCODE_CANCEL_BY_USER		101
#define QCARD_SERVICE_ERRCODE_UNKNOWN				102

#define QCARD_SERVICE_TYPE_CARD_BALANCE				0x01
#define QCARD_SERVICE_TYPE_CARD_CHARGE				0x02
#define QCARD_SERVICE_TYPE_CARD_SALE				0x03
#define QCARD_SERVICE_TYPE_CARD_EXCHANGE				0x04


#ifdef S900_DEV
BYTE qCardSrv_CustomerService(BYTE byShowSwipe,BYTE i_byServiceType);
BYTE qCardSrv_getSellerCardData(DWORD i_dwMsgId,BYTE i_byCheckPin,BYTE i_byCheckSellerId, BYTE* i_szSellerId,sellerCard*	o_sellerCard);
BYTE qCardSrv_getPin(DWORD*	o_dwNewPin);
BYTE qCardSrv_writeNewPin(BYTE* i_byaPinBlock);
#else
	BYTE qCardSrv_CustomerService(BYTE byShowSwipe);
#endif

DWORD qCardSrv_ShowBalance();
BYTE qCardCheckBlockDataExists(char* byaBlockData);
#endif /* QCARDSERVICE_H_ */
