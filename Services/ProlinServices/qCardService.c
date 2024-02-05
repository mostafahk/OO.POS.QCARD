/*
 * qCardService.c
 *
 *  Created on: May 22, 2019
 *      Author: f.karamiyar
 */


#include <QCard/Services/ProlinServices/qCardService.h>
#include <QCard/qCardUtil.h>
//#if defined(S800_DEV) || defined(PX7_DEV)|| defined(S900_DEV)
#include <QCard/sohaWebServiceTrnFile.h>
#include "define.h"
//#endif

#ifdef S900_DEV
#include <Seller/sellerConfig.h>
#include "seller/sellerService.h"
#include "seller/sellerMng.h"
#include <Log/Log.h>

#endif

DWORD getQcardNo(BYTE *byaUid)
{
	unsigned int	dwCardNo = 0;

	dwCardNo |= byaUid[0];
	dwCardNo |= byaUid[1] << 8;
	dwCardNo |= byaUid[2] << 16;
	dwCardNo |= byaUid[3] << 24;

	return dwCardNo;
}



DWORD qCardSrv_GetData(qCard *sQcard,BYTE* byaUid , BYTE byUidLen)
{
	return qCardGetData(sQcard , byaUid , byUidLen);
}

///
/// @detail This function checks input buffer
/// if it contains data or not ...
/// Empty block is recognized by being all 0x00 or 0xFF
///
BYTE qCardCheckBlockDataExists(char* byaBlockData)
{
	BYTE byaEmpty_1[16] = {0};
	BYTE byaEmpty_2[16] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	OsLog(LOG_DEBUG,"\n@qCardCheckBlockDataExists: byaEmpty_1 >> [%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x]\n",
			byaEmpty_1[0],byaEmpty_1[1],byaEmpty_1[2],byaEmpty_1[3],byaEmpty_1[4],byaEmpty_1[5],
			byaEmpty_1[6],byaEmpty_1[7],byaEmpty_1[8],byaEmpty_1[9],byaEmpty_1[10],byaEmpty_1[11],
			byaEmpty_1[12],byaEmpty_1[13],byaEmpty_1[14],byaEmpty_1[15]);

	OsLog(LOG_DEBUG,"\n@qCardCheckBlockDataExists: byaEmpty_2 >> [%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x]\n",
			byaEmpty_2[0],byaEmpty_2[1],byaEmpty_2[2],byaEmpty_2[3],byaEmpty_2[4],byaEmpty_2[5],
			byaEmpty_2[6],byaEmpty_2[7],byaEmpty_2[8],byaEmpty_2[9],byaEmpty_2[10],byaEmpty_2[11],
			byaEmpty_2[12],byaEmpty_2[13],byaEmpty_2[14],byaEmpty_2[15]);
	if (!memcmp(byaBlockData, byaEmpty_1, 16) ||
		!memcmp(byaBlockData, byaEmpty_2, 16))
	{
		OsLog(LOG_DEBUG,"\n __BLOCK is EMPTY!__ \n");
		return FALSE;
	}
	return TRUE;

}

DWORD qCardSrv_ShowBalance()
{
	DWORD dwRetVal = FALSE;
	BYTE  byCardType = 0;
	BYTE bySuccess = FALSE;
	BYTE byaUid[9] = {0};
	BYTE byUidLen = 0;
	BYTE byCounter	=	0;
	DWORD	dwMessageId = 0;
	qCard sQcard;
	memset(&sQcard,0,sizeof(qCard));



	dmgBasic_RfLogoPowerOn();

	uiDisplayMsgWithId(MESSAGE_TAP_CARD, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_INFORM_WHITE_SCREEN);
	while(byCounter < 3)
	{

	dmgPicc_Reset();
		if(qCardDetectWrapper(30,byaUid,&byUidLen,&byCardType) != SUCCESSFUL)
	{
		OsLog(LOG_DEBUG,"\n@qCardService.c:qCardSrv_CustomerService> Detect Failed3 \n");
			bySuccess = FALSE;
			break;
	}

		uiDisplayMsgWithId(MESSAGE_DO_RANSACTION, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_INFORM_WHITE_SCREEN);


	// Get Card Data
	dwRetVal = qCardSrv_GetData(&sQcard,byaUid ,byUidLen);
	if( dwRetVal == SUCCESSFUL) {
		bySuccess = TRUE;
			break;
		}else{
			if (dwRetVal == ERR_SAM_ERROR){
				uiDisplayMsgWithId(MESSAGE_ERR_SAM_OPERATION, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
				break;
			}
			else if (dwRetVal == ERR_INVALID_CARD){
				uiDisplayMsgWithId(MESSAGE_INVALID_CARD, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
				break;
			}
			else if(byCounter < 2)
			{
				uiDisplayMsgWithId(MESSAGE_READ_CARD_ERROR, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_INFORM_WHITE_SCREEN);
			}else
				uiDisplayMsgWithId(MESSAGE_ERR_READ_CARD_FAILED, UI_DISPLAY_WAIT_TYPE_ANY_KEY, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);

			byCounter++;
		}
	}


	dmgBasic_RfLogoPowerOff();
	if(bySuccess){

        // check city Code
        OsLog(LOG_DEBUG, "\nM@H Balance check: CityCode: %d \n CityCode in config: %d\n",
              sQcard.q_sCardData.q_wCity,
              g_stGlVar.m_sTerminalConfig.m_wCityCode);
        if(sQcard.q_sCardData.q_wCity!=g_stGlVar.m_sTerminalConfig.m_wCityCode)
        {
            OsLog(LOG_DEBUG,"\n ____City Code not match \n");

            uiDisplayMsgWithId(MESSAGE_CARD_NOT_ACCEPT,
                               UI_DISPLAY_WAIT_TYPE_ANY_KEY,
                               UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
            return FALSE;
        }


        qCardBalanceMenuWrapper(&sQcard);
	}


	return bySuccess;

}

#ifdef S900_DEV
int getExchangeAmount(DWORD* i_dwChargeAmount) {
    message	sMessage;

    memset(&sMessage, 0, sizeof(message));

    strcpy(sMessage.m_byaLines[sMessage.m_byLineCount++], "لطفا میزان شارژ کارت قدیمی");
    strcpy(sMessage.m_byaLines[sMessage.m_byLineCount++], "را به ریال وارد نمایید");

    sMessage.m_byLineCount = 4;;

    if(!uiKeypad_GetAmount(&sMessage, 4, i_dwChargeAmount))
        return FALSE;

    return TRUE;

}
#endif

#ifdef S900_DEV
BYTE qCardSrv_CustomerService(BYTE byShowSwipe,BYTE i_byServiceType){
#else
BYTE qCardSrv_CustomerService(BYTE byShowSwipe){
#endif
	BYTE	byCardType  =	0;
	DWORD	dwAmount		=	0;
	BYTE	byRetVal		=	0;
	BYTE	dwRetVal		=	0;
	BYTE	byTransactionType = 0;
	BYTE	byTranAddedFlag	=	FALSE;
	DWORD dwCardNo = 0;
	DWORD dwNewCredit		=	0;
	SohaSettProductInfo sProductInfo;
	SohaSettTrnInfo sTrnInfo;
	qcardLastTrnLog sQcardLastTrnLog;
	DWORD	dwCurrentDate	=	dmgBasic_GetDate();
	DWORD	dwCurrentTime	=	dmgBasic_GetTime();
    BYTE sectorNineOnlineCardData[16 * 3 + 1]; // 32 * 3 + 1
    BYTE onlineCardPhoneNumber[20];
    BYTE onlineCardNationalCode[20];
    BYTE onlineCardOldSerialCard[20] = {0};
    BYTE *onlineCardOldSerialCardPointer = NULL;


#ifndef S900_DEV
	trnReceiptInfo			sReceiptInfo;

#else
	stReceipt			sReceiptInfo;
	st_sellerCreditSale sCreditSaleLog;
	st_shift sShiftData;
	eTicketTrnInfo sCardTrnInfo;
	st_sellerCreditData sSellerCreditData;

	memset(&sShiftData, 0, sizeof(st_shift));
	memset(&sCreditSaleLog,0,sizeof(st_sellerCreditSale));
	memset(&sSellerCreditData, 0, sizeof(st_sellerCreditData));
	memset(&sCardTrnInfo, 0, sizeof(eTicketTrnInfo));
	OsLog(LOG_DEBUG,"\n\n___ Inside qCardSrv_CustomerService ___\n\n");

	if(!shift_getInfo(&sShiftData))
	{
		logSetErrCodeCategory(ERRCODE_SHIFT_NO_INFO);
		logSetErrCodeSubCategory(ERRCODE_SUB_NO_ERR);
		uiDisplayMsgWithId_ErrCodeDisplay(MESSAGE_RETRIEVE_CONFIG_ERROR,UI_DISPLAY_WAIT_TYPE_OK_CANCEL,UI_DISPLAY_MSG_TYPE_ERROR_ALARM,TRUE);
		//uiDisplayMsgWithId(MESSAGE_RETRIEVE_CONFIG_ERROR, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
		return FALSE;
	}

	if(!sellerCredit_getInfo(sShiftData.m_byaSellerId, &sSellerCreditData))
		{
			logSetErrCodeCategory(ERRCODE_SELLER_CREDIT_GET_INFO);
			logSetErrCodeSubCategory(ERRCODE_SUB_NO_ERR);
			uiDisplayMsgWithId_ErrCodeDisplay(MESSAGE_RETRIEVE_CONFIG_ERROR,UI_DISPLAY_WAIT_TYPE_OK_CANCEL,UI_DISPLAY_MSG_TYPE_ERROR_ALARM,TRUE);
			uiDisplayMsgWithId(MESSAGE_RETRIEVE_CONFIG_ERROR, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
			return FALSE;
		}


#endif

	qCard sQcard;
	trnRequestInfo sTrnRequest;
	trnResponseInfo sTrnResponse;
	memset(&sReceiptInfo,0,sizeof(trnReceiptInfo));
	memset(&sTrnInfo,0,sizeof(SohaSettTrnInfo));
	memset(&sProductInfo,0,sizeof(SohaSettProductInfo));
	memset(&sQcard,0,sizeof(qCard));
	memset(&sTrnRequest, 0, sizeof(trnRequestInfo));
	memset(&sTrnResponse, 0, sizeof(trnResponseInfo));
	BYTE bySuccess = FALSE;
	BYTE byCounter = 0;
	DWORD	dwChargeAmount = 0;


	//Checking Printer
	if(qCardIsPrinterRequiredWrapper(TRUE))
		if(!qCardPrinterCheckWrapper())
			return FALSE;


#ifdef S900_DEV


    if (g_stGlVar.m_sTerminalConfig.m_byOnlineCardModeEnable) {
        if (i_byServiceType == QCARD_SERVICE_TYPE_CARD_SALE || i_byServiceType == QCARD_SERVICE_TYPE_CARD_EXCHANGE) {
            if (!onlineCardGetPersonalInformation(onlineCardPhoneNumber, onlineCardNationalCode)) {
                OsLog(LOG_DEBUG, "\n@qCardService> can not get sector personal information from user!\n");
//                uiDisplayMsgWithId(MESSAGE_ERROR_IN_ISSUE_CARD, UI_DISPLAY_WAIT_TYPE_OK_CANCEL,
//                                   UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
                return FALSE;
            }
        }

        if (i_byServiceType == QCARD_SERVICE_TYPE_CARD_EXCHANGE) {
            if (!onlineCardGetOldCardSerial(onlineCardOldSerialCard)) {
                OsLog(LOG_DEBUG, "\n@qCardService> can not get onlineCardOldSerialCard from user!\n");
//                uiDisplayMsgWithId(MESSAGE_ERROR_IN_ISSUE_CARD, UI_DISPLAY_WAIT_TYPE_OK_CANCEL,
//                                   UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
                return FALSE;
            }
            onlineCardOldSerialCardPointer = onlineCardOldSerialCard;

        }
    }


    // if in offline mode, payment always should be performed
    // else if in online mode only in sale service should payment
//    if(!g_stGlVar.m_sTerminalConfig.m_byOnlineCardModeEnable || i_byServiceType != QCARD_SERVICE_TYPE_CARD_EXCHANGE) {
//        if (!qCardMagCardServiceWrapper(i_byServiceType, byShowSwipe, &sTrnRequest, &dwAmount,
//                                        &sTrnResponse,&sReceiptInfo, &byTransactionType,TRUE))
//            return FALSE;
//    }

    BYTE getAmountFromUser;
    getAmountFromUser = (!g_stGlVar.m_sTerminalConfig.m_byOnlineCardModeEnable || i_byServiceType != QCARD_SERVICE_TYPE_CARD_EXCHANGE);
    if (!qCardMagCardServiceWrapper(i_byServiceType, byShowSwipe, &sTrnRequest, &dwAmount,
                                    &sTrnResponse,&sReceiptInfo, &byTransactionType,getAmountFromUser))
        return FALSE;


    if (i_byServiceType == QCARD_SERVICE_TYPE_CARD_SALE)
    {
        // subtract deposit price from Amount paid. use it for charge card.
        dwChargeAmount = dwAmount - g_stGlVar.m_sTerminalConfig.m_dwSaleDepositAmount;
    }
    // if online card mode is enabled and i_byServiceType != QCARD_SERVICE_TYPE_CARD_SALE
    else if (g_stGlVar.m_sTerminalConfig.m_byOnlineCardModeEnable && i_byServiceType == QCARD_SERVICE_TYPE_CARD_EXCHANGE){
        // without any switch transaction, get amount from user and charge it.
        byRetVal = getExchangeAmount(&dwChargeAmount);
         if (byRetVal != TRUE)
             return FALSE;
    }
    else { // if i_byServiceType != QCARD_SERVICE_TYPE_CARD_SALE && (!g_stGlVar.m_sTerminalConfig.m_byOnlineCardModeEnable || i_byServiceType != QCARD_SERVICE_TYPE_CARD_EXCHANGE)
            dwChargeAmount = dwAmount;
    }


#else
//@Temp
	if(!qCardMagCardServiceWrapper(byShowSwipe,&sTrnRequest , &dwAmount, &sTrnResponse ,  &sReceiptInfo, &byTransactionType))
			return FALSE;
	dwChargeAmount = dwAmount;
//	dwChargeAmount=1;
#endif
    // You have to define a new config for online card
    // TODO: if(OnlineCard)
    // TODO: if(exchange || sale)
    // TODO: Get national code and phone number


	//Start Read
	BYTE byaUid[9] = {0};
	BYTE byUidLen = 4;
	dmgBasic_RfLogoPowerOn();

	OsLog(LOG_DEBUG,"\n ____before MESSAGE_TAP_CARD_______ \n");
	uiDisplayMsgWithId(MESSAGE_TAP_CARD, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_INFORM_WHITE_SCREEN);

    // read the mifare card and check it
	while(byCounter < 3){
		dmgPicc_Reset();
		byRetVal = qCardDetectWrapper(30,byaUid,&byUidLen,&byCardType);
		if(byRetVal == QCARD_SERVICE_ERRCODE_CANCEL_BY_USER)
		{
			bySuccess = FALSE;
			break;
		}else if(byRetVal != SUCCESSFUL)
		{
			bySuccess = FALSE;
			uiDisplayMsgWithId(MESSAGE_READ_CARD_ERROR, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_INFORM_WHITE_SCREEN);
			byCounter++;
			continue;
		}

        // TODO: if online card && (exchange || sale)
        // TODO: Call API
        OsLog(LOG_DEBUG,"UID is: %u",(*(int*)byaUid));
		uiDisplayMsgWithId(MESSAGE_DO_RANSACTION, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_INFORM_WHITE_SCREEN);

		// Get Card Data
		dwRetVal = qCardSrv_GetData(&sQcard,byaUid ,byUidLen);

		if( dwRetVal != SUCCESSFUL) {
			bySuccess = FALSE;


			if(dwRetVal == ERR_READ_ETICKET_DATA )
			{
				if(byCounter < 2)
				uiDisplayMsgWithId(MESSAGE_READ_CARD_ERROR, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_INFORM_WHITE_SCREEN);
				else
					uiDisplayMsgWithId(MESSAGE_ERR_READ_CARD_FAILED, UI_DISPLAY_WAIT_TYPE_ANY_KEY, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);

				byCounter++;
				continue;
			}
			else{
				if (dwRetVal == ERR_SAM_ERROR)
					uiDisplayMsgWithId(MESSAGE_ERR_SAM_OPERATION, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
				else if (dwRetVal == ERR_INVALID_CARD)
					uiDisplayMsgWithId(MESSAGE_INVALID_CARD, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);

				break;
			}
		}
		else{
#ifndef	S900_DEV
			if(sQcard.q_byIsIssued == FALSE)
            {
				uiDisplayMsgWithId(MESSAGE_INVALID_CARD, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
                OsLog(LOG_DEBUG, "ERROR we are in online card mode, but card is not issued yet");
                bySuccess = FALSE;
                break;
            }
#endif
#ifdef S900_DEV
			if(sQcard.q_byIsIssued == FALSE && i_byServiceType == QCARD_SERVICE_TYPE_CARD_CHARGE){
				bySuccess = FALSE;
				uiDisplayMsgWithId(MESSAGE_CARD_IS_NEW, UI_DISPLAY_WAIT_TYPE_ANY_KEY, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
				break;
			}else if(sQcard.q_byIsIssued == TRUE && (i_byServiceType == QCARD_SERVICE_TYPE_CARD_SALE || i_byServiceType == QCARD_SERVICE_TYPE_CARD_EXCHANGE)){
				bySuccess = FALSE;
				uiDisplayMsgWithId(MESSAGE_ERR_CARD_IS_SOLD, UI_DISPLAY_WAIT_TYPE_ANY_KEY, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
				break;
			}

#endif
            bySuccess = TRUE;
            break;
		}
		

	}

	if(bySuccess){

        // check city Code
        OsLog(LOG_DEBUG, "\nM@H check: CityCode: %d \n CityCode in config: %d\n",
              sQcard.q_sCardData.q_wCity,
              g_stGlVar.m_sTerminalConfig.m_wCityCode);
        if(sQcard.q_sCardData.q_wCity!=g_stGlVar.m_sTerminalConfig.m_wCityCode)
        {
            OsLog(LOG_DEBUG,"\n ____City Code not match \n");

            uiDisplayMsgWithId(MESSAGE_CARD_NOT_ACCEPT,
                               UI_DISPLAY_WAIT_TYPE_ANY_KEY,
                               UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
            bySuccess=FALSE;
            goto UNSUCCESS;
        }

        OsLog(LOG_DEBUG, "\n@qCardService> q_wContractType of card is %d!\n", sQcard.q_sCardData.q_wContractType);
		if(!accTableFindCard(sQcard.q_sCardData.q_wContractType, &sQcard.m_sCardAccInfo))
		{
			OsLog(LOG_DEBUG, "\n@qCardService> Could not find it in the acc Table!\n");
			uiDisplayMsgWithId(MESSAGE_INVALID_CARD, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
			//return ERR_TICKET_NOT_ACCEPTED;
			bySuccess=FALSE;
			goto UNSUCCESS;
		}

		if(!eTicketIsValidCardType(sQcard.m_sCardAccInfo.m_byCardType))
		{
			logSetErrCodeCategory(ERRCODE_MIF_TCKT_GET_DATA_ERR);
			uiDisplayMsgWithId(MESSAGE_INVALID_CARD, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
			//return ERR_TICKET_NOT_ACCEPTED;
			bySuccess=FALSE;
			goto UNSUCCESS;
		}



		if((sQcard.m_sCardAccInfo.m_byCardType == CARD_TYPE_SUPPORT))
		{
			logSetErrCodeCategory(ERRCODE_MIF_TCKT_GET_DATA_ERR);
			logSetErrCodeSubCategory(ERR_TICKET_NOT_ACCEPTED);

			message sMessage;
			memset(&sMessage, 0, sizeof(message));
			sMessage.m_byLineCount = 1;
			sprintf(sMessage.m_byaLines[0], "امکان شارژ وجود ندارد");
#if defined(PX7_DEV)
			uiDisplayMsg(&sMessage, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR, 40, 1);
#else
			uiDisplayMsg(&sMessage, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR, uiDisplay_getFontSize(UI_DISPLAY_FONT_SIZE));
#endif
			bySuccess=FALSE;
			goto UNSUCCESS;
			//return ERR_TICKET_NOT_ACCEPTED;
		}

#ifndef T610_DEV
		if (sQcard.m_sCardAccInfo.m_dwMaxBalance < dwChargeAmount + sQcard.q_sTranLog.q_dwTotalAmount) //TODO -> Block the card
		{
			OsLog(LOG_DEBUG, "\n\n m_dwMaxBalance: %d    + sQcard.q_sTranLog.q_dwTotalAmount: %d  \n\n", sQcard.m_sCardAccInfo.m_dwMaxBalance,
					dwChargeAmount + sQcard.q_sTranLog.q_dwTotalAmount);
			message sMessage;
			memset(&sMessage, 0, sizeof(message));
			sMessage.m_byLineCount = 2;
			sprintf(sMessage.m_byaLines[0], "سقف شارژ مجاز");
			sprintf(sMessage.m_byaLines[1], "امکان شارژ وجود ندارد");
#if defined(PX7_DEV)
			uiDisplayMsg(&sMessage, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR, 40, 1);
#else
			uiDisplayMsg(&sMessage, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR, uiDisplay_getFontSize(UI_DISPLAY_FONT_SIZE));
#endif
			
			//return FALSE;
			bySuccess=FALSE;
			goto UNSUCCESS;
		}
#endif

#ifdef S900_DEV
        // if service is issue, send data to server and get block 9 data
        // store block 9 that received from server into card (sector 9) in qCardWriteTrnLog() function
        if (g_stGlVar.m_sTerminalConfig.m_byOnlineCardModeEnable) {
            if (i_byServiceType == QCARD_SERVICE_TYPE_CARD_SALE || i_byServiceType == QCARD_SERVICE_TYPE_CARD_EXCHANGE) {


                message	sMessage;
                if (!onlineCardIssue(byaUid,onlineCardPhoneNumber,onlineCardNationalCode,onlineCardOldSerialCard, sectorNineOnlineCardData, &sMessage))
                {
                    OsLog(LOG_DEBUG, "\n@qCardService> can not get sector 9 from server!\n");
                    uiDisplayMsg(&sMessage, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR_ALARM, uiDisplay_getFontSize(UI_DISPLAY_FONT_SIZE));
                    bySuccess=FALSE;
                    goto UNSUCCESS;
                }



            }
        }
#endif
		bySuccess = FALSE;
		memcpy(&(sQcard.q_sNewTrn),&(sQcard.q_sTranLog),sizeof(qCardTrnLog));
		sQcard.q_sNewTrn.q_dwSequenceNumber++;
		OsLog(LOG_DEBUG,"\n@qCardService.c:qCardSrv_CustomerService> New Sequence %d\n",sQcard.q_sNewTrn.q_dwSequenceNumber);
		OsLog(LOG_DEBUG,"\n@qCardService.c:qCardSrv_CustomerService> CHARGE AMOUNT %d\n",dwChargeAmount);
//@Strange
		sQcard.q_sNewTrn.q_dwTotalAmount = dwChargeAmount + sQcard.q_sTranLog.q_dwTotalAmount;
		OsLog(LOG_DEBUG,"\n@qCardService.c:qCardSrv_CustomerService> New amount %d\n",sQcard.q_sNewTrn.q_dwTotalAmount);

		 sQcard.q_sNewTrn.q_dwTranDateTime = qCardUtilGetEpochDateTime(dwCurrentDate,dwCurrentTime);


		utlByteArrayToDword(&dwCardNo,sQcard.q_sCardData.q_byaUid,4);
		OsLog(LOG_DEBUG,"\n@qCardService.c:qCardSrv_CustomerService> New dateTime %d\n",sQcard.q_sNewTrn.q_dwTranDateTime);
		if(qCardGenerateTrnSignature(&sQcard))
		{

			//AddSett Transactions


			sProductInfo.m_dwCurrentBalance = sQcard.q_sNewTrn.q_dwTotalAmount;
			sProductInfo.m_dwLastBalance = sQcard.q_sTranLog.q_dwTotalAmount;
			sprintf((char*)sProductInfo.m_byaSerial, "%lu", dwCardNo);
			sProductInfo.m_dwDeposit = sQcard.q_sCardData.q_dwDepositeValue;
			sProductInfo.m_dwProductId = sQcard.q_sCardData.q_wContractType;
			sProductInfo.m_dwSeqNumber = sQcard.q_sNewTrn.q_dwSequenceNumber;
			sProductInfo.m_dwTagType = sQcard.q_sCardData.q_wTagType;
			sProductInfo.m_dwDecrementalCounter = sQcard.q_sCardTransactionData.q_dwDecrementalCounter;
			sProductInfo.m_dwTransactionPointer = sQcard.q_sCardTransactionData.q_wTransactionSectorPointer;

			BYTE byCounter = 0;
			for(byCounter = 0;byCounter < 16;byCounter++)
				sprintf(sProductInfo.m_byaTrnSignature + byCounter * 2,"%02X",sQcard.q_sNewTrn.q_byaTrnLogSignature[byCounter]);

			sTrnInfo.m_dwDate = dwCurrentDate;
			sTrnInfo.m_dwTime = dwCurrentTime;
			sTrnInfo.m_dwAmount = dwChargeAmount;
			strcpy((char*)sTrnInfo.m_byaTransactionType , "550022" );
			sTrnInfo.m_byServiceCode= ONLINE_SERVICE_TYPE_BILL_PAYMENT;



#ifndef S900_DEV
			strcpy((char*)sTrnInfo.m_byaPaymentInfo,(char*)sReceiptInfo.m_byaRrn);
			sTrnInfo.m_byPaymentType = PAYMENT_TYPE_ONLINE;
#else
			sTrnInfo.m_dwShiftId = sShiftData.m_dwShiftId;
			strcpy((char*)sTrnInfo.m_byaSellerId, (char*)sShiftData.m_byaSellerId);
			sTrnInfo.m_dwCurrentCrdit = sSellerCreditData.m_dwCurrentCredit;
			sTrnInfo.m_dwCashCredit= sSellerCreditData.m_dwCashCredit;
			strcpy((char*)sTrnInfo.m_byaPaymentInfo,(char*)sReceiptInfo.m_sOnlineTrn.m_byaRrn);
			sTrnInfo.m_byPaymentType = sReceiptInfo.m_paymentType;
#endif


#if  defined(PX7_DEV) || defined(S800_DEV) || defined(S900_DEV)
			OsLog(LOG_DEBUG, "\n >>>_______________________________________sTrnInfo.m_byaSellerId:%s\n",sTrnInfo.m_byaSellerId);

			OsLog(LOG_DEBUG, "\n >>> FTP >> GOING TO DO THE ADD TRANSACTION SECTION. . .\n");
			if(g_stGlVar.m_sTerminalConfig.m_bySohaFTPEnable)
				SohaSettAddTrnToOpenFile(&sProductInfo, &sTrnInfo);
			if (g_stGlVar.m_sTerminalConfig.m_bySohaWebServiceEnable || !g_stGlVar.m_sTerminalConfig.m_bySohaFTPEnable)
			{
				OsLog(LOG_DEBUG, "\n >>> Web Service >> GOING TO DO THE ADD TRANSACTION SECTION. . .\n");
				SohaWsSettAddTrnToOpenFile(&sProductInfo, &sTrnInfo);
			}
#else
			OsLog(LOG_DEBUG, "\n >>> FTP >> GOING TO DO THE ADD TRANSACTION SECTION. . .\n");
			SohaSettAddTrnToOpenFile(&sProductInfo, &sTrnInfo);
#endif

			byTranAddedFlag = TRUE;
			// Write new transaction into qCard
			BYTE byaUidStep2[9] = {0};
			BYTE byUidLen	=	0;
			bySuccess	=	FALSE;
			byCounter	=	0;
			while(byCounter < 3){
				if(byCounter > 0)
				{
					dmgPicc_Reset();

					byRetVal = qCardDetectWrapper(30,byaUidStep2,&byUidLen,&byCardType);
					OsLog(LOG_DEBUG,"\n@qCardService.c:qCardSrv_CustomerService> Detect : %d\n",byRetVal);
					if(byRetVal == QCARD_SERVICE_ERRCODE_CANCEL_BY_USER)
					{
						bySuccess = FALSE;
						break;
					}else if(byRetVal != SUCCESSFUL)
					{
						bySuccess = FALSE;
						if(byCounter < 2){
							uiDisplayMsgWithId(MESSAGE_TAP_CARD_FOR_COMPLETE_TRN, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_INFORM_WHITE_SCREEN);
							byCounter++;
							continue;
						}else
							break;

					}
				}
				if((memcmp(byaUid,byaUidStep2,byUidLen) == 0 && byCounter > 0) || byCounter == 0)
				{
                    // todo: don't pass sector nine to this function, instead set sector nine to sQcard and access it in the function
					byRetVal = qCardWriteTrnLog(&sQcard,byaUid ,byUidLen,sectorNineOnlineCardData);
					OsLog(LOG_DEBUG,"\n@qCardService.c:qCardSrv_CustomerService> qCardWriteTrnLog: %d \n",byRetVal);
					if(byRetVal != SUCCESSFUL)
					{

						if(byCounter > 2)
						{
							bySuccess = FALSE;
							break;
						}else{
							uiDisplayMsgWithId(MESSAGE_UPDATE_CARD_ERROR_RETAP, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_INFORM_WHITE_SCREEN);
							byCounter++;
							continue;
						}
					}else{
						bySuccess = TRUE;
								break;
							}

				}else{
					OsLog(LOG_DEBUG,"\n@qCardService.c:qCardSrv_CustomerService> UID mismatch %02X%02X%02X%02X -- %02X%02X%02X%02X\n",
						byaUid[0],byaUid[1],byaUid[2],byaUid[3],byaUidStep2[0],byaUidStep2[1],byaUidStep2[2],byaUidStep2[3]);
					uiDisplayMsgWithId(MESSAGE_INVALID_CARD, UI_DISPLAY_WAIT_TYPE_ANY_KEY, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
					uiDisplayMsgWithId(MESSAGE_UPDATE_CARD_ERROR_RETAP, UI_DISPLAY_WAIT_TYPE_ANY_KEY, UI_DISPLAY_MSG_TYPE_INFORM_WHITE_SCREEN);
					byCounter++;
					continue;
				}
			}

		}
        else{
			OsLog(LOG_DEBUG,"\n@qCardService.c:qCardSrv_CustomerService> Last signature isn't verified\n");
			bySuccess = FALSE;
			uiDisplayMsgWithId(MESSAGE_INVALID_CARD, UI_DISPLAY_WAIT_TYPE_ANY_KEY, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);

		}

	}

UNSUCCESS:

	if(!bySuccess)
	{
		dmgBasic_RfLogoPowerOff();
		sTrnResponse.m_byResponseCode = 255;

#ifndef S900_DEV
		sReceiptInfo.m_byResponseCode = sTrnResponse.m_byResponseCode;
#else
		sReceiptInfo.m_sOnlineTrn.m_byResponseCode = sTrnResponse.m_byResponseCode;
#endif

		if(qCardIsPrinterRequiredWrapper(bySuccess)){

			if( qCardPrinterCheckWrapper()){

				if((report_CheckUnprintReversalExistence()) && (report_IfPrintReverse(sTrnResponse.m_byResponseCode, 0)))
				{
					report_PrintReversal();
				}
				else
				{
#ifdef S900_DEV
					if(sReceiptInfo.m_paymentType != CUSTOMER_SRV_PAYMENT_TYPE_CASH)
					{
#endif

						report_PrintOnlineTrnReceipt(&sReceiptInfo, REPORT_RECEIPT_TYPE_CUSTOMER);
						report_DeleteUnprintReversal();
#ifdef S900_DEV
					}
#endif
				}
			}
		}

		OsLog(LOG_DEBUG, "\n >>> Unsuccess: byTranAddedFlag=%d\n",byTranAddedFlag);

		if(byTranAddedFlag)
		{
			SohaSettDelLastRec();
			if (g_stGlVar.m_sTerminalConfig.m_bySohaWebServiceEnable)
				SohaWsSettDelLastRec();

		}
		qCardUnSuccessTrnPageWrapper(&sTrnRequest, &sTrnResponse);


#ifndef S900_DEV
	//if(g_stGlVar.m_sTerminalConfig.m_bySendMTIConfirmation)
		onlineSrvTrnRemainAdvice();
#else
		if (sReceiptInfo.m_paymentType == CUSTOMER_SRV_PAYMENT_TYPE_CASH)
			sellerCredit_incrementCurrent(sShiftData.m_byaSellerId,dwAmount);
		onlineSrvTrnRemainAdvice();

#endif
		dmgBasic_RfLogoPowerOff();
		return FALSE;
	}

	dmgBasic_RfLogoPowerOff();

#ifndef S900_DEV
	report_DeleteUnprintReversal();
	onlineSrvTrnDelReversal(g_stGlVar.m_sTerminalConfig.m_bySendMTIConfirmation, &sTrnResponse);
	sReceiptInfo.m_byTicketCategory = CARD_TYPE_CUSTOMER_CREDIT;
	sReceiptInfo.m_byTicketContractType = 0;
	sReceiptInfo.m_byTicketBalance = sQcard.q_sNewTrn.q_dwTotalAmount;

	sprintf((char*)sReceiptInfo.m_byaTicketNo, "%lu",dwCardNo);
#else

	OsLog(LOG_DEBUG,"\n@qCardService.c:qCardSrv_CustomerService> CHARGE AMOUNT: %d\n",dwAmount);
	OsLog(LOG_DEBUG,"\n@qCardService.c:qCardSrv_CustomerService>ServiceType: %d\n",sReceiptInfo.m_sGeneral.m_byServiceType);
	if (sReceiptInfo.m_paymentType == CUSTOMER_SRV_PAYMENT_TYPE_ONLINE){
		report_DeleteUnprintReversal();
		onlineSrvTrnDelReversal(g_stGlVar.m_sTerminalConfig.m_bySendMTIConfirm, &sTrnResponse);
	}

    // aggregation
	if(i_byServiceType == QCARD_SERVICE_TYPE_CARD_SALE || i_byServiceType == QCARD_SERVICE_TYPE_CARD_EXCHANGE  ){


		sProductInfo.m_dwCurrentBalance = 0;
		sProductInfo.m_dwLastBalance = 0;
		sprintf((char*)sProductInfo.m_byaSerial, "%lu", dwCardNo);
		sProductInfo.m_dwProductId = sQcard.q_sCardData.q_wContractType;
		sProductInfo.m_dwSeqNumber = sQcard.q_sNewTrn.q_dwSequenceNumber;
		sProductInfo.m_dwTagType = sQcard.q_sCardData.q_wTagType;
		sProductInfo.m_dwDecrementalCounter = sQcard.q_sCardTransactionData.q_dwDecrementalCounter;
		sProductInfo.m_dwTransactionPointer = sQcard.q_sCardTransactionData.q_wTransactionSectorPointer;
		if(i_byServiceType == QCARD_SERVICE_TYPE_CARD_SALE   )
			sProductInfo.m_dwDeposit = sQcard.q_sCardData.q_dwDepositeValue;

		BYTE byCounter = 0;
		for(byCounter = 0;byCounter < 16;byCounter++)
			sprintf(sProductInfo.m_byaTrnSignature + byCounter * 2,"%02X",sQcard.q_sNewTrn.q_byaTrnLogSignature[byCounter]);

		sTrnInfo.m_dwDate = dwCurrentDate;
		sTrnInfo.m_dwTime = dwCurrentTime;

		if(i_byServiceType == QCARD_SERVICE_TYPE_CARD_SALE   )
		{
			strcpy((char*)sTrnInfo.m_byaTransactionType , "550011" );
			sTrnInfo.m_byServiceCode = ONLINE_SERVICE_TYPE_PURCHASE;
		sTrnInfo.m_dwAmount = g_stGlVar.m_sTerminalConfig.m_dwSaleDepositAmount;

		}
		if( i_byServiceType == QCARD_SERVICE_TYPE_CARD_EXCHANGE  )
		{
			strcpy((char*)sTrnInfo.m_byaTransactionType , "550010" );
			sTrnInfo.m_byServiceCode = ONLINE_SERVICE_TYPE_EXCHANGE;
			sTrnInfo.m_dwAmount = 0;

		}

		sTrnInfo.m_dwShiftId = sShiftData.m_dwShiftId;
		strcpy((char*)sTrnInfo.m_byaSellerId, (char*)sShiftData.m_byaSellerId);
		sTrnInfo.m_dwCurrentCrdit = sSellerCreditData.m_dwCurrentCredit;
		sTrnInfo.m_dwCashCredit= sSellerCreditData.m_dwCashCredit;
		strcpy((char*)sTrnInfo.m_byaPaymentInfo,(char*)sReceiptInfo.m_sOnlineTrn.m_byaRrn);
		sTrnInfo.m_byPaymentType = sReceiptInfo.m_paymentType;



		OsLog(LOG_DEBUG, "\n >>> FTP >> GOING TO DO THE ADD TRANSACTION SECTION. . .\n");
		if(!g_stGlVar.m_sTerminalConfig.m_bySohaWebServiceEnable || g_stGlVar.m_sTerminalConfig.m_bySohaFTPEnable)
			SohaSettAddTrnToOpenFile(&sProductInfo, &sTrnInfo);
		if (g_stGlVar.m_sTerminalConfig.m_bySohaWebServiceEnable)
		{
			OsLog(LOG_DEBUG, "\n >>> Web Service >> GOING TO DO THE ADD TRANSACTION SECTION. . .\n");
			SohaWsSettAddTrnToOpenFile(&sProductInfo, &sTrnInfo);
		}
	}


    // calculate information and print ticket
	sReceiptInfo.m_sTicketInfo.m_byTicketCategory = CARD_TYPE_CUSTOMER_CREDIT;
	sReceiptInfo.m_sTicketInfo.m_byTicketContractType = sQcard.q_sCardData.q_wContractType;
	sReceiptInfo.m_sTicketInfo.m_dwTicketBalance  = sQcard.q_sNewTrn.q_dwTotalAmount;

	sprintf((char*)sReceiptInfo.m_sTicketInfo.m_byaTicketNo, "%lu",dwCardNo);
	sShiftData.m_dwTrnCount++;
	sShiftData.m_dwTrnOrgAmount += dwAmount;
	sShiftData.m_dwTrnAmount += dwAmount;

	if(sReceiptInfo.m_sGeneral.m_byServiceType == RECEIPT_TYPE_ONLINE_TICKET_CHARGE)
	{
		sShiftData.m_dwChargeCount_Online++;
		sShiftData.m_dwChargeOrgAmount_Online += dwAmount;
		sShiftData.m_dwChargeAmount_Online += dwAmount;

	}else if(sReceiptInfo.m_sGeneral.m_byServiceType == RECEIPT_TYPE_OFFLINE_TICKET_CHARGE)
	{
		sShiftData.m_dwChargeCount_Cash++;
		sShiftData.m_dwChargeOrgAmount_Cash += dwAmount;
		sShiftData.m_dwChargeAmount_Cash += dwAmount;
	}else	if(sReceiptInfo.m_sGeneral.m_byServiceType == RECEIPT_TYPE_ONLINE_TICKET_SALE)
	{
		sShiftData.m_dwSaleCount_Online++;
		sShiftData.m_dwSaleOrgAmount_Online += dwAmount;
		sShiftData.m_dwSaleAmount_Online += dwAmount;

	}else if(sReceiptInfo.m_sGeneral.m_byServiceType == RECEIPT_TYPE_OFFLINE_TICKET_SALE)
	{
		sShiftData.m_dwSaleCount_Cash++;
		sShiftData.m_dwSaleOrgAmount_Cash += dwAmount;
		sShiftData.m_dwSaleAmount_Cash += dwAmount;
	}



	shift_UpdateData(&sShiftData);

	// update credit sale log
	if(sReceiptInfo.m_sGeneral.m_byServiceType == RECEIPT_TYPE_VOUCHER_OFFLINE)
	{
		sCreditSaleLog.m_dwTrnCount = 1;
		sCreditSaleLog.m_dwTrnOrgAmount = dwAmount;
		sCreditSaleLog.m_dwTrnAmount = dwAmount;

		sCreditSaleLog.m_dwVoucherSaleCount_cash= 1;
		sCreditSaleLog.m_dwVoucherSaleAmount_cash = dwAmount;
		sCreditSaleLog.m_dwVoucherSaleOrgAmount_cash = dwAmount;

		sellerCreditRpt_updateCreditSaleInfo(sShiftData.m_byaSellerId, &sCreditSaleLog);
		decrementOfflineTransactionsCount();
	}

	// set last charge info
	if(sReceiptInfo.m_paymentType == CUSTOMER_SRV_PAYMENT_TYPE_ONLINE)
	{


		sCardTrnInfo.m_dwAmount          = dwAmount;
		sCardTrnInfo.m_dwDate            = sReceiptInfo.m_sGeneral.m_dwDate;
		sCardTrnInfo.m_dwTime            = sReceiptInfo.m_sGeneral.m_dwTime;
		sCardTrnInfo.m_dwStationId       = g_stGlVar.m_sTerminalConfig.m_dwStationId;
		sCardTrnInfo.m_byPaymentType     = sReceiptInfo.m_paymentType;
		sCardTrnInfo.m_byTrnType         = MIFARE_TRN_TYPE_CHARGE;
		sCardTrnInfo.m_byaPinEntryDevice =  sTrnRequest.m_byaPinEntryDevice;
		strcpy((char*)&sCardTrnInfo.m_byaPaymentInfo[8], (char*)sTrnResponse.m_byaRrn);

		strcpy((char*)sQcardLastTrnLog.m_byPaymentInfo, (char*)sCardTrnInfo.m_byaPaymentInfo);
		memcpy(&sQcardLastTrnLog.m_sCardData,&sQcard , sizeof(qCard));
		sQcardLastTrnLog.m_dwAmount = dwAmount;
		qCardUpdateLastChargeInfo(&sQcardLastTrnLog);
	}



#endif


	utlSuccessAlarm();
	qCardSuccessTrnPageWrapper(&sReceiptInfo);
	OsLog(LOG_DEBUG, "\n\n\nSuccess Transaction page Done ...\n\n");

#ifndef S900_DEV
	if(g_stGlVar.m_sTerminalConfig.m_bySendMTIConfirmation)
		onlineSrvTrnRemainAdvice();
#else

	onlineSrvTrnRemainAdvice();
#endif


	qCardExaminSamCreditWrapper();


	OsLog(LOG_DEBUG, "\n\n\nSuccess Transaction page Done1 ...\n\n");
	return TRUE;
}

#if defined(S900_DEV) || defined(PX7_DEV)
BYTE qCardSrv_getSupportCardData(DWORD i_dwMsgId, BYTE i_checkCityCode)
{
	DWORD dwRetVal = FALSE;
	BYTE  byCardType = 0;
	BYTE byaUid[9] = {0};
	BYTE byUidLen = 0;
	qCard sQcard;


	memset(&sQcard,0,sizeof(qCard));


		dmgPicc_Reset();
		if(qCardDetectWrapperWithMsgID(30,byaUid,&byUidLen,&byCardType,i_dwMsgId) != SUCCESSFUL)
		{
			OsLog(LOG_DEBUG,"\n@qCardService.c:qCardSrv_CustomerService> Detect Failed2 \n");
			return FALSE;
		}

			uiDisplayMsgWithId(MESSAGE_DO_RANSACTION, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_INFORM_WHITE_SCREEN);

			dwRetVal = qCardSrv_GetData(&sQcard,& byaUid ,  byUidLen);

			if( dwRetVal == SUCCESSFUL) {

				OsLog(LOG_DEBUG,"\n@sQcard.q_sCardData.q_wContractType:%d \n",sQcard.q_sCardData.q_wContractType);

				if(sQcard.q_sCardData.q_wContractType != QCARD_TYPE_SUPPORT)
				{
					uiDisplayMsgWithId(MESSAGE_INVALID_CARD, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
					return FALSE;
				}

                if (i_checkCityCode)
                {
                    if (sQcard.q_sCardData.q_wCity != g_stGlVar.m_sTerminalConfig.m_wCityCode)
                    {
                        OsLog(LOG_DEBUG, "\n ____City Code not match for support card (card %d device %d) \n",
                              sQcard.q_sCardData.q_wCity, g_stGlVar.m_sTerminalConfig.m_wCityCode);
                        uiDisplayMsgWithId(MESSAGE_INVALID_CARD, UI_DISPLAY_WAIT_TYPE_OK_CANCEL,
                                           UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
                        return FALSE;
                    }
                }

			}else{
				if (dwRetVal == ERR_SAM_ERROR)
					uiDisplayMsgWithId(MESSAGE_ERR_SAM_OPERATION, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);

				else if (dwRetVal == ERR_INVALID_CARD)
					uiDisplayMsgWithId(MESSAGE_INVALID_CARD, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);

				else
					uiDisplayMsgWithId(MESSAGE_ERR_READ_CARD_FAILED, UI_DISPLAY_WAIT_TYPE_ANY_KEY, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);

				return FALSE;
			}

	return TRUE;

}
#endif

#ifdef S900_DEV
static BYTE	byaQcardKey[7];
BYTE qCardSrv_getSellerCardData(DWORD i_dwMsgId,BYTE i_byCheckPin,BYTE i_byCheckSellerId, BYTE* i_szSellerId,sellerCard*	o_sellerCard)
{
	DWORD	dwMessageId	=	0;
	BYTE	byUidLen 	= 	0;
	BYTE	byaUid[11]	=	{0};
	BYTE  	byCardType = 0;
	BYTE	byaTermSerialNo[9] = {0};
	DWORD	dwRetVal	=	0;
	BYTE 	byaPinBlock[8]	=	{0};
	DWORD dwCardNo = 0;
	sellerCard	sellerCard;
	qCard 			sQcard;
	st_seller		sSeller;

	memset(&sSeller, 0, sizeof(st_seller));

	memset(&sQcard,0,sizeof(qCard));

	dmgPicc_Reset();
	if(qCardDetectWrapperWithMsgID(30,byaUid,&byUidLen,&byCardType,i_dwMsgId) != SUCCESSFUL)
	{
		OsLog(LOG_DEBUG,"\n@qCardService.c:qCardSrv_CustomerService> Detect Failed1 \n");
		return FALSE;
	}

	uiDisplayMsgWithId(MESSAGE_DO_RANSACTION, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_INFORM_WHITE_SCREEN);

	dwRetVal = qCardSrv_GetData(&sQcard,& byaUid ,byUidLen);//sellermifare_GetData(&sCard);
	if( dwRetVal == SUCCESSFUL) {

		OsLog(LOG_DEBUG,"\n@sQcard.q_sCardData.q_wContractType:%d \n",sQcard.q_sCardData.q_wContractType);

		if(sQcard.q_sCardData.q_wContractType != QCARD_TYPE_SELLER)
		{
			uiDisplayMsgWithId(MESSAGE_INVALID_CARD, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
			return FALSE;
		}

	}else{
		if (dwRetVal == ERR_SAM_ERROR){
			OsLog(LOG_DEBUG, "\n@1\n");
			uiDisplayMsgWithId(MESSAGE_ERR_SAM_OPERATION, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
		}
		else if (dwRetVal == ERR_INVALID_CARD){
			OsLog(LOG_DEBUG, "\n@2\n");
			uiDisplayMsgWithId(MESSAGE_INVALID_CARD, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
		}
		else{
			OsLog(LOG_DEBUG, "\n@3\n");
			uiDisplayMsgWithId(MESSAGE_ERR_READ_CARD_FAILED, UI_DISPLAY_WAIT_TYPE_ANY_KEY, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
		}
		return FALSE;
	}

////	/////////////////////////////////////////////////////////////
//	BYTE byaBlockPinData[16] = {0};
//
//	memset(byaBlockPinData, 0, sizeof(byaBlockPinData));
//	//Get Key From Sam
//	if(getQcardAuthKeys(byaUid,byUidLen, byaQcardKey)){
//		return ERR_SAM_ERROR;
//	}
//	OsLog(LOG_DEBUG,"\nCard Keys: [%02x][%02x][%02x][%02x][%02x][%02x]\n",byaQcardKey[0],byaQcardKey[1],byaQcardKey[2],
//				byaQcardKey[3],byaQcardKey[4],byaQcardKey[5]);
//
//	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> writing Sector Seven ...  \n\n");
//	dwRetVal = dmgPicc_MifareAuthenticate(DMG_PICC_MIFARE_KEY_TYPE_B, byaQcardKey, SECTOR_SEVEN, byUidLen , byaUid);
//	if(dwRetVal != SUCCESSFUL){
//		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> writing Sector Seven failed  \n\n");
//		return	ERR_UPDATE_ETICKET_DATA;
//	}
//	dwRetVal = dmgPicc_MifareWrite(SECTOR_SEVEN, BLOCK_ZERO , byaBlockPinData);
//		if(dwRetVal != SUCCESSFUL){
//			OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  writing Log Sector block Seven failed  : %d\n\n",dwRetVal);
//			return	ERR_UPDATE_ETICKET_DATA;
//		}
////	/////////////////////////////////////////////////////////////

	memcpy(&sellerCard.m_byaNationalCode, &sQcard.q_sPersonalizedData.q_byaNationalId, sizeof(sQcard.q_sPersonalizedData.q_byaNationalId));
	memcpy(&sellerCard.m_byaPinBlock, &sQcard.q_sqCardSellerData.q_byaPinBlock, sizeof(sQcard.q_sqCardSellerData.q_byaPinBlock));
	memcpy(&sellerCard.m_byaCardUid, &sQcard.q_sCardData.q_byaUid, sizeof(sQcard.q_sCardData.q_byaUid));
	memcpy(&sellerCard.m_byaStoredUid, &sQcard.q_sCardData.q_byaUid, sizeof(sQcard.q_sCardData.q_byaUid));

	sellerCard.m_byUidLen = sQcard.q_sCardData.q_byUidLen;
	sellerCard.m_byLevel=sQcard.q_sqCardSellerData.q_byaSellerLevel;
	sellerCard.m_byType = QCARD;

	dwCardNo |= sQcard.q_sPersonalizedData.q_byaNationalId[0] << 40;
	dwCardNo |= sQcard.q_sPersonalizedData.q_byaNationalId[1] << 32;
	dwCardNo |= sQcard.q_sPersonalizedData.q_byaNationalId[2] << 24;
	dwCardNo |= sQcard.q_sPersonalizedData.q_byaNationalId[3] << 16;
	dwCardNo |= sQcard.q_sPersonalizedData.q_byaNationalId[4] << 8;
	dwCardNo |= sQcard.q_sPersonalizedData.q_byaNationalId[5] ;

	OsLog(LOG_DEBUG,"\n\n _________dwCardNo:%lu_____________",dwCardNo);
	sprintf((char*)sellerCard.m_byaCardNo, "%lu", dwCardNo);

	if(!sellerSrv_CheckCardValidity(&sellerCard))
		return FALSE;


	if(i_byCheckSellerId)
	{


		if((strlen((char*)i_szSellerId) != strlen((char*)sellerCard.m_byaCardNo))
				|| (strcmp((char*)i_szSellerId, (char*)sellerCard.m_byaCardNo) != 0))
		{
			uiDisplayMsgWithId(MESSAGE_INVALID_CARD, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
			return FALSE;
		}
	}

	strcpy((char*)sSeller.m_byaSellerId, (char*)sellerCard.m_byaCardNo);
	if(!sellerMng_find(&sSeller))
	{

		OsLog(LOG_DEBUG,"\n\n _________sellerMng_find_____________");

		uiDisplayMsgWithId(MESSAGE_SELLER_NOT_DEFINED, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
		return FALSE;
	}

	 utlSuccessAlarm();

	if((strlen((char*)sellerCard.m_byaPinBlock) == 0))
	{
		BYTE 	byaPin[5]	=	{0};
		DWORD   dwNewPin;
		if(!qCardSrv_getPin(&dwNewPin))
			return FALSE;

		sprintf((char*)byaPin, "%lu", dwNewPin);

		OsLog(LOG_DEBUG,"\n byaPin   %s\n",byaPin);

		if(!sellermifare_GeneratePinBlock(&sellerCard, &byaPin, byaPinBlock))
		{
			uiDisplayMsgWithId(MESSAGE_ERR_OPERATION, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
			return FALSE;
		}

		qCardSrv_writeNewPin(&byaPinBlock);
		return FALSE;
	}

	else if(i_byCheckPin && !g_stGlVar.m_byTermUsableStatus)
	{
		if(!sellerSrv_checkCardPin(&sellerCard))
		{
			return FALSE;
		}
	}

	memcpy(o_sellerCard, &sellerCard, sizeof(sellerCard));

	return TRUE;

}

BYTE qCardSrv_getPin(DWORD*	o_dwNewPin)
{

	BYTE 	byaPinBlock[8]	=	{0};
	DWORD	dwNewPin		=	0;
	DWORD	dwNewPinReEnter	=	0;
	BYTE	byUidLen 	= 	0;
	BYTE	byaUid[11]	=	{0};
	DWORD	dwRetVal	=	0;
	DWORD	dwMessageId	=	0;
	BYTE	byCardType	=	0;
	message	sMessage;

	memset(&sMessage, 0, sizeof(message));

	strcpy(sMessage.m_byaLines[0], "لطفا رمز جدید را وارد نمایید");
	sMessage.m_byLineCount = 2;

	if(!uiKeypad_GetPin(&sMessage, 2, 4, 4, &dwNewPin))
		return FALSE;

	OsLog(LOG_DEBUG, "dwNewPin = %d", dwNewPin);

	memset(&sMessage, 0, sizeof(message));

	strcpy(sMessage.m_byaLines[0], "لطفا تکرار رمز جدید را");
	strcpy(sMessage.m_byaLines[1], "وارد نمایید");
	sMessage.m_byLineCount = 3;

	if(!uiKeypad_GetPin(&sMessage, 3, 4, 4, &dwNewPinReEnter))
		return FALSE;

	OsLog(LOG_DEBUG, "\ndwNewPinReEnter = %d\n", dwNewPinReEnter);

	if(dwNewPin != dwNewPinReEnter)
	{
		uiDisplayMsgWithId(MESSAGE_ERR_ENTERED_PIN_NOT_EQUAL, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
		return FALSE;
	}

	*o_dwNewPin = dwNewPin;

	return TRUE;
}



BYTE qCardSrv_writeNewPin(BYTE* i_byaPinBlock)
{
	BYTE	byUidLen 	= 	0;
	BYTE	byaUid[11]	=	{0};
	BYTE  	byCardType  =   0;
	DWORD	dwRetVal	=	0;
	qCard sQcard;


	memset(byaQcardKey,0,sizeof(byaQcardKey));
	memset(&sQcard,0,sizeof(qCard));


	dmgPicc_Reset();
	if(qCardDetectWrapperWithMsgID(30,byaUid,&byUidLen,&byCardType,MESSAGE_INSERT_SELLER_CARD) != SUCCESSFUL)
	{
		OsLog(LOG_DEBUG,"\n@qCardService.c:qCardSrv_CustomerService> Detect Failed4 \n");
		return FALSE;
	}

	uiDisplayMsgWithId(MESSAGE_DO_RANSACTION, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_INFORM_WHITE_SCREEN);

	dwRetVal = qCardSrv_GetData(&sQcard,& byaUid ,byUidLen);
	if( dwRetVal == SUCCESSFUL) {

		OsLog(LOG_DEBUG,"\n@sQcard.q_sCardData.q_wContractType:%d \n",sQcard.q_sCardData.q_wContractType);

		if(sQcard.q_sCardData.q_wContractType != QCARD_TYPE_SELLER)
		{
			uiDisplayMsgWithId(MESSAGE_INVALID_CARD, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
			return FALSE;
		}

	}else{
		if (dwRetVal == ERR_SAM_ERROR)
			uiDisplayMsgWithId(MESSAGE_ERR_SAM_OPERATION, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);

		else if (dwRetVal == ERR_INVALID_CARD)
			uiDisplayMsgWithId(MESSAGE_INVALID_CARD, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);

		else
			uiDisplayMsgWithId(MESSAGE_ERR_READ_CARD_FAILED, UI_DISPLAY_WAIT_TYPE_ANY_KEY, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);

		return FALSE;
	}


	//Get Key From Sam
	if(getQcardAuthKeys(byaUid,byUidLen, byaQcardKey)){
		return ERR_SAM_ERROR;
	}
	OsLog(LOG_DEBUG,"\nCard Keys: [%02x][%02x][%02x][%02x][%02x][%02x]\n",byaQcardKey[0],byaQcardKey[1],byaQcardKey[2],
				byaQcardKey[3],byaQcardKey[4],byaQcardKey[5]);

	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> writing Sector Seven ...  \n\n");
	dwRetVal = dmgPicc_MifareAuthenticate(DMG_PICC_MIFARE_KEY_TYPE_B, byaQcardKey, SECTOR_SEVEN, byUidLen , byaUid);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> writing Sector Seven failed  \n\n");
		return	ERR_UPDATE_ETICKET_DATA;
	}


	dwRetVal = dmgPicc_MifareWrite(SECTOR_SEVEN, BLOCK_ZERO , i_byaPinBlock);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  writing Log Sector block Seven failed  : %d\n\n",dwRetVal);
		return	ERR_UPDATE_ETICKET_DATA;
	}

	utlSuccessAlarm();
	uiDisplayMsgWithId(MESSAGE_SUCCESS_CARD_CHANGE_PIN, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_SUCCESS);

	return TRUE;

}
#endif
