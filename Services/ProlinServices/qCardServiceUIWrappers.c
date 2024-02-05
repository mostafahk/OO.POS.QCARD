/*
 * qCardServiceUIWrappers.c
 *
 *  Created on: Jun 10, 2019
 *      Author: f.karamiyar
 */


#include <QCard/Services/ProlinServices/qCardServiceUIWrappers.h>
#include <QCard/Services/ProlinServices/qCardService.h>
#ifdef PX7_DEV
extern directMagCardInfo g_sQMagInfo;
#endif

#ifdef S900_DEV
BYTE qCardMagCardServiceWrapper(BYTE i_byServiceType,BYTE byShowSwipe,trnRequestInfo* i_sTrnRequest,  DWORD* i_dwAmount, trnResponseInfo* i_sTrnResponse, stReceipt* i_sReceiptInfo , BYTE* byTransactionType, BYTE i_getAmountFromUser)
#else
BYTE qCardMagCardServiceWrapper(BYTE byShowSwipe,trnRequestInfo* i_sTrnRequest,  DWORD* i_dwAmount, trnResponseInfo* i_sTrnResponse, trnReceiptInfo* i_sReceiptInfo , BYTE* byTransactionType)
#endif
{
#ifdef PX7_DEV
	 BYTE  i_byPrinterStatus = TRUE;
	 BYTE* i_byaPhoneNumber = NULL;
	 BYTE bySendSms	 = FALSE;
	 BYTE byChargeType = 0 ;
	 BYTE byCardType = 0;
	 *byTransactionType = TRANSACTION_TYPE_ONLINE;

	 return MagCardService( i_byPrinterStatus, i_sTrnRequest,i_byaPhoneNumber, i_dwAmount,  i_sTrnResponse, &bySendSms, i_sReceiptInfo, &byCardType, byChargeType ,&g_sQMagInfo);
#endif

#ifdef S800_DEV
	 BYTE byServiceType =  ONLINE_SERVICE_TYPE_CHARGE_TICKET;
	 BYTE byTrnViaSellerCard = FALSE;
	 if(!cardSrvChargeAmountSelectionMenu(i_dwAmount))
		 return FALSE;
	 if(magCardService(byShowSwipe, i_dwAmount, byServiceType,i_sTrnRequest,  i_sTrnResponse,i_sReceiptInfo, &byTrnViaSellerCard,byTransactionType) == SUCCESSFUL)
		 return TRUE;
	 else
		 return FALSE;
#endif

#ifdef S900_DEV

	BYTE byRetVal = TRUE;
	BYTE byTranStatus = FALSE;
	BYTE byIsSellerBankCard = FALSE;
	BYTE byPaymentType = 0;
	BYTE byPinPadSwipeCardSelected = FALSE;
	BYTE byPinPadGetPinSelected = FALSE;
	BYTE byaPinBlock[9] = {0};
	BYTE cardNum = 0;
	char pinPadMsg[100] = {0};
	magCardInfo sMagCard;
	st_shift sShiftData;
	st_sellerCreditData	sSellerCreditData;
	st_sellerCardInfo	gs_sSelleCardInfo;

	memset(&gs_sSelleCardInfo, 0, sizeof(st_sellerCardInfo));
	memset(&sShiftData, 0, sizeof(st_shift));
	memset(&sSellerCreditData, 0, sizeof(st_sellerCreditData));



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
		//uiDisplayMsgWithId(MESSAGE_RETRIEVE_CONFIG_ERROR, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
		return FALSE;
	}




    if (i_getAmountFromUser){
        if(!customerSrv_GetChargeAmount(MENU_ID_CUSTOMER_OFFLINE_CREDIT, i_dwAmount))
            return FALSE;
    }
    else{
        *i_dwAmount = 0;
    }


	if( i_getAmountFromUser && (i_byServiceType == QCARD_SERVICE_TYPE_CARD_SALE || i_byServiceType == QCARD_SERVICE_TYPE_CARD_EXCHANGE)){

		message sMessage;
		char	szAmountStr[128] = {0};
		BYTE	byMsgLineCounter = 0;
		memset(&sMessage, 0, sizeof(message));

		if(g_stGlVar.m_sTerminalConfig.m_dwSaleDepositAmount > 0)
		{
			if(i_byServiceType == QCARD_SERVICE_TYPE_CARD_EXCHANGE)
				sprintf(szAmountStr, "%lu", 0);
			else
			sprintf(szAmountStr, "%lu", g_stGlVar.m_sTerminalConfig.m_dwSaleDepositAmount);

			utlFormatAmountSring(szAmountStr, szAmountStr);
			sprintf((char*)sMessage.m_byaLines[byMsgLineCounter++], "قیمت کارت: %s ریال", szAmountStr);
		}

		if(*i_dwAmount > 0)
		{
			sprintf(szAmountStr, "%lu", *i_dwAmount );
			utlFormatAmountSring(szAmountStr, szAmountStr);
			sprintf((char*)sMessage.m_byaLines[byMsgLineCounter++], "شارژ: %s ریال", szAmountStr);
		}

		if(i_byServiceType == QCARD_SERVICE_TYPE_CARD_SALE)
		*i_dwAmount += g_stGlVar.m_sTerminalConfig.m_dwSaleDepositAmount;

		strcpy((char*)sMessage.m_byaLines[byMsgLineCounter++], "مبلغ قابل پرداخت:");

		sprintf(szAmountStr, "%lu", *i_dwAmount);
		utlFormatAmountSring(szAmountStr, szAmountStr);
		sprintf((char*)sMessage.m_byaLines[byMsgLineCounter++], "%s ریال", szAmountStr);
		if(i_byServiceType == QCARD_SERVICE_TYPE_CARD_EXCHANGE)
			strcpy((char*)sMessage.m_byaLines[byMsgLineCounter++], "تعویض کارت انجام شود؟");
		else
		strcpy((char*)sMessage.m_byaLines[byMsgLineCounter++], "فروش کارت انجام شود؟");

		sMessage.m_byLineCount = byMsgLineCounter;

		if(uiDisplayMsg(&sMessage, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_INFORM, uiDisplay_getFontSize(UI_DISPLAY_FONT_SIZE)) != UI_DISPLAY_RETURN_TYPE_OK)
			return FALSE;


	}
	OsLog(LOG_DEBUG,"\n_________LOG4__________\n,*i_dwAmount:%d",*i_dwAmount);

    if (i_getAmountFromUser) {
        if (!customerSrv_selectPaymentType(MENU_ID_CUSTOMER_OFFLINE_CREDIT, *i_dwAmount, &byPaymentType, &sMagCard,
                                           &byPinPadSwipeCardSelected, &byPinPadGetPinSelected, NULL))
            return FALSE;
    }
    else{
        byPaymentType = CUSTOMER_SRV_PAYMENT_TYPE_CASH;
    }

    if(byPaymentType == CUSTOMER_SRV_PAYMENT_TYPE_CASH)
    {


        if(*i_dwAmount > sSellerCreditData.m_dwCurrentCredit)
        {
            logSetErrCodeCategory(ERRCODE_INSUFFICIENT_CURRENT_CREDIT);
            logSetErrCodeSubCategory(ERRCODE_SUB_NO_ERR);
            uiDisplayMsgWithId_ErrCodeDisplay(MESSAGE_INSUFFICIENT_CURRENT_CREDIT,UI_DISPLAY_WAIT_TYPE_OK_CANCEL,UI_DISPLAY_MSG_TYPE_ERROR_ALARM,TRUE);
            //uiDisplayMsgWithId(MESSAGE_INSUFFICIENT_CURRENT_CREDIT, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
            return FALSE;
        }


        if(!sellerCredit_decrementCurrent(sShiftData.m_byaSellerId,*i_dwAmount))
        {
            logSetErrCodeCategory(ERRCODE_CONN_SERVER_DOWN);
            logSetErrCodeSubCategory(ERRCODE_SUB_NO_ERR);
            uiDisplayMsgWithId_ErrCodeDisplay(MESSAGE_ERR_UPDATE_FILE,UI_DISPLAY_WAIT_TYPE_OK_CANCEL,UI_DISPLAY_MSG_TYPE_ERROR_ALARM,TRUE);
            return	FALSE;
        }




            if(i_byServiceType == QCARD_SERVICE_TYPE_CARD_SALE || i_byServiceType == QCARD_SERVICE_TYPE_CARD_EXCHANGE)
                i_sReceiptInfo->m_sGeneral.m_byServiceType = RECEIPT_TYPE_OFFLINE_TICKET_SALE;
            else
                i_sReceiptInfo->m_sGeneral.m_byServiceType = RECEIPT_TYPE_OFFLINE_TICKET_CHARGE;

            i_sReceiptInfo->m_sGeneral.m_dwDate = dmgBasic_GetDate();
            i_sReceiptInfo->m_sGeneral.m_dwTime = dmgBasic_GetTime();
            i_sReceiptInfo->m_sTicketTrnInfo.m_dwAmount= *i_dwAmount;

        }

    if(byPaymentType == CUSTOMER_SRV_PAYMENT_TYPE_ONLINE)
    {
        i_sTrnRequest->m_bySwitchVersion = g_stGlVar.m_sTerminalConfig.m_bySwitchVersion;
        if(strncmp((char*)gs_sSelleCardInfo.m_sCardInfo.m_byaPan,(char*)sMagCard.m_byaPan,sMagCard.m_byPanLen) == 0 && !byPinPadSwipeCardSelected && !byPinPadGetPinSelected)
        {

            memcpy(byaPinBlock,gs_sSelleCardInfo.m_byaPinBlock,sizeof(gs_sSelleCardInfo.m_byaPinBlock));
        }
        else
        {

            if(i_sTrnRequest->m_bySwitchVersion == NEGIN_SWITCH && (*i_dwAmount)  > 0 && !custCommonSrvCheckBankCard(*i_dwAmount , &sMagCard,&cardNum,FALSE)){

                if(g_stGlVar.m_sTerminalConfig.m_byParsianSwitchEnable && TermCfgCheckSwitchConfigExistance(PARSIAN_SWITCH)){
                    i_sTrnRequest->m_bySwitchVersion = PARSIAN_SWITCH;
                }else if((TermCfgCheckSwitchConfigExistance(SIPA_SWITCH)) && g_stGlVar.m_sTerminalConfig.m_byaSipaSwitchEnable)
                {
                    i_sTrnRequest->m_bySwitchVersion = SIPA_SWITCH;
                }else{

                    OsLog(LOG_DEBUG,"\n @MagCardService> switch can not change because back up files are not exist...cardnum:%d\n",cardNum);
                    custCommonSrvDisplayBankCards(g_stGlVar.m_sBankCardsConfig.m_sBankCardInfoInfo[cardNum].m_byaBankName,
                            g_stGlVar.m_sBankCardsConfig.m_sBankCardInfoInfo[cardNum].m_dwMinTrnAmount, 30);
                    return ERR_BANK_CARD_LIMITATION;

                }

            }
            byRetVal = sellerFindCard(&sMagCard);
            if(byRetVal)
                byIsSellerBankCard = TRUE;
            if(!uiKeypad_GetMagCardPin(&sMagCard, byaPinBlock, *i_dwAmount , byPinPadGetPinSelected,i_sTrnRequest->m_bySwitchVersion))
                return FALSE;
        }

        i_sTrnRequest->m_byaPinEntryDevice = (byPinPadGetPinSelected || byPinPadSwipeCardSelected) ? 1 : 0;

        i_sTrnRequest->m_byServiceType = g_stGlVar.m_sTerminalConfig.m_byBillPaymentVersion?ONLINE_SERVICE_TYPE_BILL_PAYMENT:ONLINE_SERVICE_TYPE_PURCHASE;
        if(strlen(sMagCard.m_byaPan)==0){
            uiDisplayMsgWithId(MESSAGE_INPUT_INVALID_TRY_AGAIN, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_ERROR);
            return FALSE;

        }

        memcpy(&i_sTrnRequest->m_sCardInfo, &sMagCard, sizeof(magCardInfo));
        memcpy(i_sTrnRequest->m_byaPinBlock, byaPinBlock, sizeof(byaPinBlock));
        sprintf((char*)i_sTrnRequest->m_byaAmount, "%ld", *i_dwAmount);

        if(i_byServiceType == QCARD_SERVICE_TYPE_CARD_SALE || i_byServiceType == QCARD_SERVICE_TYPE_CARD_EXCHANGE)
            i_sReceiptInfo->m_sGeneral.m_byServiceType = RECEIPT_TYPE_ONLINE_TICKET_SALE;
        else
            i_sReceiptInfo->m_sGeneral.m_byServiceType = RECEIPT_TYPE_ONLINE_TICKET_CHARGE;

        i_sTrnRequest->m_byServiceType = g_stGlVar.m_sTerminalConfig.m_byBillPaymentVersion?ONLINE_SERVICE_TYPE_BILL_PAYMENT:ONLINE_SERVICE_TYPE_PURCHASE;
        uiDisplayMsgWithId(MESSAGE_CONNECTING_TO_SERVER, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_INFORM);


        if(byPinPadGetPinSelected)
            dmgPinPadShowStatus(1);


        byRetVal = onlineSrvTrnDotransactionWrapper(i_sTrnRequest, i_sTrnResponse);
        if(byRetVal == ERR_CONNECTION_ERR)
        {

            uiDisplayMsgWithId_ErrCodeDisplay(MESSAGE_CONNECT_ERROR, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR, TRUE);
            return FALSE;
        }

        OsLog(LOG_DEBUG,"\n >>>>>>>>>>>>>>>>>>>>>>>>Do Tran : %d<<<<<<<<<<<<<<<<<<<<<<<<<<<", byRetVal);
        if((byRetVal == ONLINE_SERVICE_RESULT_SUCCESS) || (byRetVal == ONLINE_SERVICE_RESULT_UNSUCCESS))
        {

            strcpy((char*)i_sReceiptInfo->m_sOnlineGeneral.m_byaPan, (char*)i_sTrnRequest->m_sCardInfo.m_byaPan);
            strcpy((char*)i_sReceiptInfo->m_sOnlineTrn.m_byaAmount, (char*)i_sTrnResponse->m_byaAmount);
            strcpy((char*)i_sReceiptInfo->m_sOnlineTrn.m_byaRrn, (char*)i_sTrnResponse->m_byaRrn);
            strcpy((char*)i_sReceiptInfo->m_sOnlineTrn.m_dwAuditNumber, (char*)i_sTrnResponse->m_byaAuthId);
            i_sReceiptInfo->m_sOnlineTrn.m_byResponseCode = i_sTrnResponse->m_byResponseCode;
            i_sReceiptInfo->m_sOnlineTrn.m_dwDate = i_sTrnResponse->m_dwDate;
            i_sReceiptInfo->m_sOnlineTrn.m_dwTime = i_sTrnResponse->m_dwTime;
            i_sReceiptInfo->m_sOnlineTrn.m_byaPinEntryDevice = i_sTrnRequest->m_byaPinEntryDevice;
            i_sReceiptInfo->m_paymentType = byPaymentType;
        }

        if(byPinPadGetPinSelected)
        {

            memset(pinPadMsg, 0, sizeof(pinPadMsg));
            byTranStatus = FALSE;
            if (i_sTrnResponse->m_byResponseCode == 0 && byRetVal != ONLINE_SERVICE_RESULT_SUCCESS) //Response code is not filled!
                dmgPinpadResponseCodeMsg(63, pinPadMsg);
            else
                dmgPinpadResponseCodeMsg(i_sTrnResponse->m_byResponseCode, pinPadMsg);
            if(i_sTrnResponse->m_byResponseCode == 0 && byRetVal == ONLINE_SERVICE_RESULT_SUCCESS)
                byTranStatus = TRUE;
            dmgPinPadShowTransactionStatus(byTranStatus,pinPadMsg,1);
        }
        if(byIsSellerBankCard == TRUE && byRetVal == ONLINE_SERVICE_RESULT_SUCCESS)
        {

            memcpy((char*)&gs_sSelleCardInfo.m_sCardInfo, (char*)&sMagCard, sizeof(magCardInfo));
            memcpy(gs_sSelleCardInfo.m_byaPinBlock,byaPinBlock,sizeof(gs_sSelleCardInfo.m_byaPinBlock));
        }

        if(byRetVal != ONLINE_SERVICE_RESULT_SUCCESS)
        {

            if((byRetVal == ONLINE_SERVICE_RESULT_SEND_EXIST_ADVICE_ERROR)
                    || (byRetVal == ONLINE_SERVICE_RESULT_SEND_TRN_ERROR))
            {

                logSetErrCodeCategory(ERRCODE_PAYMENT_TYPE_ONLINE);
                logSetErrCodeSubCategory(ERRCODE_SOCKET_INIT);
                uiDisplayMsgWithId_ErrCodeDisplay(MESSAGE_CONNECT_ERROR,UI_DISPLAY_WAIT_TYPE_OK_CANCEL,UI_DISPLAY_MSG_TYPE_ERROR_ALARM,TRUE);
                //uiDisplayMsgWithId(MESSAGE_CONNECT_ERROR, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR);
                return FALSE;
            }

            if(byRetVal == ONLINE_SERVICE_RESULT_GENERATE_TRN_ERROR)
            {
                logSetErrCodeCategory(ERRCODE_PAYMENT_TYPE_ONLINE);
                logSetErrCodeSubCategory(ERRCODE_SOCKET_ONLINE_SERVICE_RESULT_GENERATE);
                uiDisplayMsgWithId_ErrCodeDisplay(MESSAGE_CONNECT_ERROR,UI_DISPLAY_WAIT_TYPE_OK_CANCEL,UI_DISPLAY_MSG_TYPE_ERROR,TRUE);
                //uiDisplayMsgWithId(MESSAGE_GENERATE_TRN_ERR, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR);
                return FALSE;
            }



            if((byRetVal == ONLINE_SERVICE_RESULT_RECEIVE_RES_ERROR)
                    || (byRetVal == ONLINE_SERVICE_RESULT_PROCESS_RES_ERROR))
            {

                i_sTrnResponse->m_byResponseCode = 253;
                i_sReceiptInfo->m_sOnlineTrn.m_byResponseCode = i_sTrnResponse->m_byResponseCode;
            }



            if(byRetVal == ONLINE_SERVICE_RESULT_INVALID_RES_ERROR)
            {
                i_sTrnResponse->m_byResponseCode = 252;
                i_sReceiptInfo->m_sOnlineTrn.m_byResponseCode = i_sTrnResponse->m_byResponseCode;
            }


            if((report_CheckUnprintReversalExistence()) && (report_IfPrintReverse(i_sTrnResponse->m_byResponseCode, 0)))
            {
                report_PrintReversal();
            }

            else if(report_IfPrintReverse(i_sTrnResponse->m_byResponseCode, 0))
            {
                logSetErrCodeCategory(ERRCODE_PAYMENT_TYPE_ONLINE);
                logSetErrCodeSubCategory(ERRCODE_SOCKET_TRANSACTION);
                uiDisplayMsgWithId_ErrCodeDisplay(MESSAGE_TRANSACTION_ERROR,UI_DISPLAY_WAIT_TYPE_OK_CANCEL,UI_DISPLAY_MSG_TYPE_ERROR_ALARM,TRUE);
                //uiDisplayMsgWithId(MESSAGE_TRANSACTION_ERROR, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR_ALARM);
            }
            else
            {
                report_PrintOnlineTrnReceipt(i_sReceiptInfo, REPORT_RECEIPT_TYPE_CUSTOMER);
                report_DeleteUnprintReversal();
            }

            onlineSrvTrnRemainAdvice();


        return FALSE;
    }

        i_sReceiptInfo->m_sGeneral.m_dwDate = i_sTrnResponse->m_dwDate;
        i_sReceiptInfo->m_sGeneral.m_dwTime = i_sTrnResponse->m_dwTime;
        OsLog(LOG_DEBUG,"\n >>>>>>>>>>>>>>>>>>>>>>>>Do Tran End<<<<<<<<<<<<<<<<<<<<<<<<<<<");
    }



	return TRUE;

#endif
}


BYTE qCardIsPrinterRequiredWrapper(BYTE byTranState){

#if defined( S800_DEV) || defined( S900_DEV)
    if(!byTranState)
	return TRUE;
    return FALSE;
#endif

#ifdef PX7_DEV
	if( (g_stGlVar.m_byConnectToPrinter) &&
			((g_stGlVar.m_sTerminalConfig.m_byPrintSuccessReceipt && byTranState) || (g_stGlVar.m_sTerminalConfig.m_byPrintUnsuccessReceipt && !byTranState)))
		return TRUE;
	return FALSE;
#endif

}

BYTE qCardPrinterCheckWrapper(){

#if  defined( S800_DEV) || defined( S900_DEV)
	int iRet = 0;
	iRet = OsPrnCheck();
	if(iRet != 0)
	{
		logSetErrCodeStruct(ERRCODE_MANCARD_SRV,ERRCODE_MIF_SERVICE_PRINTER_ERR, iRet);
		uiDisplayMsgWithId_ErrCodeDisplay(MESSAGE_PLEASE_CHECK_PRINTER, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR_ALARM, TRUE);
		return FALSE;
	}
	return TRUE;
#endif

#ifndef DEBUGMODE
#ifdef PX7_DEV
			if(!dmgPrn_CheckPrinter())
			{
				logSetErrCodeService(ERRCODE_CARD_SERVICE);
				uiDisplayMsgWithId_ErrCodeDisplay(MESSAGE_PRINTER_IS_UNAVAILABLE_2, UI_DISPLAY_WAIT_TYPE_OK_CANCEL, UI_DISPLAY_MSG_TYPE_ERROR, TRUE);
				return FALSE;

			}
			return TRUE;
#endif
#endif

}

#ifndef S900_DEV
void qCardSuccessTrnPageWrapper(trnReceiptInfo* i_sReceiptInfo)
#else
void qCardSuccessTrnPageWrapper(stReceipt* i_sReceiptInfo)
#endif
{
#ifdef PX7_DEV
	cardSrvDisplayChargeSuccPage( i_sReceiptInfo);
#endif

#ifdef S800_DEV
	report_PrintOnlineTrnReceipt(i_sReceiptInfo, REPORT_RECEIPT_TYPE_CUSTOMER);
#endif

#ifdef S900_DEV

	if(i_sReceiptInfo->m_paymentType == CUSTOMER_SRV_PAYMENT_TYPE_ONLINE){

			report_PrintOnlineTrnReceipt(i_sReceiptInfo, REPORT_RECEIPT_TYPE_CUSTOMER);
			if(g_stGlVar.m_sTerminalConfig.m_byaSellerReceiptEnable == TRUE){
				if (!g_stGlVar.m_sTerminalConfig.m_byReceiptMenuEnable)
				{
					if(uiDisplayMsgWithId(MESSAGE_Q_ACCEPTOR_SELLER_RECEIPT, UI_DISPLAY_WAIT_TYPE_OK_CANCEL,
							UI_DISPLAY_MSG_TYPE_INFORM) != UI_DISPLAY_RETURN_TYPE_CANCEL)
						report_PrintOnlineTrnReceipt(i_sReceiptInfo, REPORT_RECEIPT_TYPE_MERCHANT);
				}
				else
					report_PrintOnlineTrnReceipt(i_sReceiptInfo, REPORT_RECEIPT_TYPE_MERCHANT);
			}
			}

	else if(i_sReceiptInfo->m_paymentType == CUSTOMER_SRV_PAYMENT_TYPE_CASH)
		{

			recPrintTicketOfflineTrn(i_sReceiptInfo);


//
//				//	// set last charge info
//				//
//				//	strcpy((char*)sMifareTrnLog.m_byPaymentInfo, (char*)sCardTrnInfo.m_byaPaymentInfo);
//				//	memcpy(&sMifareTrnLog.m_sCardData, i_sCard, sizeof(mifare));
//				//	sMifareTrnLog.m_dwAmount = dwAmount;
//				//	if(i_sCard->m_sCardAccInfo.m_byCardType != CARD_TYPE_CUSTOMER_ONEWAY)
//				//		mifare_updateLastChargeInfo(&sMifareTrnLog);
//				//

		}




#endif
}


void qCardUnSuccessTrnPageWrapper(trnRequestInfo* i_sTrnRequest, trnResponseInfo* i_sTrnResponse){

	//Checking Printer
#ifdef S800_DEV
	if(!qCardPrinterCheckWrapper())
		uiDisplayMsgWithId(MESSAGE_REVERSE_TRANSACTION, UI_DISPLAY_WAIT_TYPE_ANY_KEY, UI_DISPLAY_MSG_TYPE_ERROR);
#endif

#ifdef PX7_DEV
	custCommonSrvDisplayUnsuccessTrnResultPage(i_sTrnRequest, i_sTrnResponse);
#endif

#ifdef S900_DEV
	if(!qCardPrinterCheckWrapper())
		uiDisplayMsgWithId(MESSAGE_REVERSE_TRANSACTION, UI_DISPLAY_WAIT_TYPE_ANY_KEY, UI_DISPLAY_MSG_TYPE_ERROR);
#endif

}

#ifdef PX7_DEV
void cardServiceBalancePage(qCard* i_sCard){
	int iHeight, iWidth;
	int titleHeight = 34;
	char title[] = "موجودی کارت بلیت";
	DWORD dwTime = 0;
	BYTE byTime[32] = {0};
	char szBuffer[512] = {0};
	DWORD	dwShamsiDate = 0;
	char szBalanceStr[512] = {0};
	DWORD dwBeginTime = 0;
	int iX = 0;
	int iY = 0;
	int key;

	OsLog(LOG_DEBUG, "\n\n@BBBBBBBBBBBBBBBBBBalance Service:  %ld ", i_sCard->q_sTranLog.q_dwTotalAmount );

	dmgDisplay_GetSize(&iWidth, &iHeight);
	dmgDisplay_SetBackColor(UI_DISPLAY_COLOR_SAP_GREEN);

	uiDisplay_DisplayStrAtY(title, 50, 5, UI_DISPLAY_ALIGNMENT_CENTER, UI_DISPLAY_COLOR_WHITE);

	dwTime = dmgBasic_GetTime();
	sprintf(byTime,"%02d:%02d:%02d", utlGetSecond(dwTime), utlGetMinute(dwTime), utlGetHour(dwTime));
	utlMiladi2Shamsi(dmgBasic_GetDate(), &dwShamsiDate);

	sprintf(szBuffer, "%04d/%02d/%02d      %s",utlGetYear(dwShamsiDate), utlGetMonth(dwShamsiDate), utlGetDay(dwShamsiDate),byTime);
	uiDisplay_DisplayStrAtY(szBuffer, titleHeight, 90, UI_DISPLAY_ALIGNMENT_CENTER, UI_DISPLAY_COLOR_WHITE);

	sprintf(szBuffer, " شماره پایانه: %s", g_stGlVar.m_sTerminalLocalConfig.m_byaTerminalId);
	uiDisplay_DisplayStrAtY(szBuffer, titleHeight, 140, UI_DISPLAY_ALIGNMENT_RIGHT, UI_DISPLAY_COLOR_WHITE);

	sprintf(szBuffer, "شماره پذیرنده : %s", g_stGlVar.m_MerchantLocalCfg.m_byaMrchntID);
	uiDisplay_DisplayStrAtY(szBuffer, titleHeight, 140, UI_DISPLAY_ALIGNMENT_LEFT, UI_DISPLAY_COLOR_WHITE);

	memset(szBuffer,0,sizeof(szBuffer));
	DWORD dwCardNo = 0;
	titleHeight = 44;
	utlByteArrayToDword(&dwCardNo,i_sCard->q_sCardData.q_byaUid,4);
	sprintf(szBuffer, "شماره کارت:%lu",dwCardNo);
	uiDisplay_DisplayStrAtY(szBuffer, titleHeight, 190, UI_DISPLAY_ALIGNMENT_RIGHT, UI_DISPLAY_COLOR_WHITE);


	memset(szBuffer,0,sizeof(szBuffer));
	sprintf(szBuffer, "%ld", i_sCard->q_sTranLog.q_dwTotalAmount);
	FormatAmount(szBuffer, szBalanceStr);
	sprintf(szBuffer, "موجودی : %s ریال", szBalanceStr);
	uiDisplay_DisplayStrAtY(szBuffer, titleHeight, 190, UI_DISPLAY_ALIGNMENT_LEFT, UI_DISPLAY_COLOR_WHITE);

	titleHeight = 34;
	char dateTimeStr[sizeof(ST_TIME) + 8];
	qCardUtilGetTimeStr(i_sCard->q_sTranLog.q_dwTranDateTime, dateTimeStr);
	sprintf(szBuffer, "تاریخ آخرین تراکنش : %s",dateTimeStr);
	uiDisplay_DisplayStrAtY(szBuffer, titleHeight, 250, UI_DISPLAY_ALIGNMENT_RIGHT, UI_DISPLAY_COLOR_WHITE);

//	memset(dateTimeStr,0,sizeof(dateTimeStr));
//	qCardUtilGetTimeStr(i_sCard->, dateTimeStr);
//	sprintf(szBuffer, "تاریخ تراکنش ماقبل آخر : %s", dateTimeStr);
//	uiDisplay_DisplayStrAtY(szBuffer, titleHeight, 320, UI_DISPLAY_ALIGNMENT_RIGHT, UI_DISPLAY_COLOR_WHITE);
//
//	memset(dateTimeStr,0,sizeof(dateTimeStr));
//	qCardUtilGetTimeStr(i_sCard->q_sLastChargeTrnLog.q_sCardServiceData, dateTimeStr);
//	sprintf(szBuffer, "تاریخ آخرین شارژ : %s", dateTimeStr);
//	uiDisplay_DisplayStrAtY(szBuffer, titleHeight, 320, UI_DISPLAY_ALIGNMENT_RIGHT, UI_DISPLAY_COLOR_WHITE);


	dmgTsFlush();

	dmgKeypad_Clear();
	dwBeginTime = OsGetTickCount();
	do
	{
		if(dmgTsGetInput(&iX, &iY))
		{
			dwBeginTime = OsGetTickCount();
			dmgTsFlush();
		}

		key = dmgKeypad_GetKey();
		if(key != -1)
			dwBeginTime = OsGetTickCount();
		if((key == KEY_CANCEL) || (key == KEY_ENTER))
		{
			break;
		}

	} while(((OsGetTickCount() - dwBeginTime) < (10 * 1000)) && (key != KEY_CANCEL) && (key != KEY_ENTER));

}
#endif





void qCardBalanceMenuWrapper(qCard* sQcard){

#ifdef PX7_DEV
	cardServiceBalancePage(sQcard);
#endif

#ifdef S800_DEV
	uiDisplay_ShowCardBalance(sQcard->q_sTranLog.q_dwTotalAmount,CARD_SRV_SERVICE_ID_BALANCE);
#endif

#ifdef S900_DEV
	uiDisplay_ShowCardBalance(sQcard->q_sTranLog.q_dwTotalAmount,MENU_ID_CUSTOMER_REVERSE_OFFLINE_CREDIT);
#endif
}


#ifdef PX7_DEV

BYTE qCardServiceUIdetectCard(BYTE i_byTimeoutSec, BYTE* o_byaUid ,BYTE* o_byUidLen, BYTE *o_byCardType,BYTE byStatusEnable)
{
	BYTE	chCardType	= 	0;
	BYTE	byUidLen 	= 	0;
	BYTE	byaUid[11]	=	{0};
	DWORD	dwBeginTime	= 0;
	int iRemainingTime = 0;

	//XuiWindow* gif1Win;
	//XuiWindow* gif2Win;
	char *xui_argv[10];
	int xui_argc = 0;

	dmgPicc_CheckRemoveCard();

	if(dmgPicc_Detect(DMG_PICC_DETECT_MODE_MIFARE, &chCardType, &byUidLen, byaUid) == SUCCESSFUL)
	{
		(*o_byUidLen) = byUidLen;
		(*o_byCardType) =  chCardType;
		memcpy(o_byaUid, byaUid, byUidLen);
		return SUCCESSFUL;
	}

	if(i_byTimeoutSec == 0)
		return QCARD_SERVICE_ERRCODE_TIMEOUT;


	cusCommonPlayAudio("./data/detectcard.wav");
/////////////////////////////////
	//if(!XuiIsRunning())
	//	XuiOpen(xui_argc, xui_argv);

	//gif1Win = XuiCreateGif (XuiRootCanvas(),XuiRootCanvas()->width-200,XuiRootCanvas()->height-240,200,200,"./data/readcard.gif");
	//gif2Win = XuiCreateGif (XuiRootCanvas(),0,XuiRootCanvas()->height-140,300,100,"./data/ArrowD.gif");
	//XuiShowWindow(gif1Win,1,0);
	//XuiShowWindow(gif2Win,1,0);

	//XuiImg *stateImg = NULL;//(XuiImg *)malloc( sizeof(XuiImg));
	//stateImg = XuiImgLoadFromFile("./data/state4.png");
	//if(byStatusEnable){
	//	XuiCanvasDrawImg(XuiRootCanvas(), 0 , 440 ,800,40,0,stateImg);
	//	XuiShowWindow(XuiRootCanvas(),1,0);
	//}


/*	XuiImg *arrowImg = (XuiImg *)malloc( sizeof(XuiImg));
	arrowImg = XuiImgLoadFromFile("./data/ArrowD.png");
	XuiCanvasDrawImg(XuiRootCanvas(),0,XuiRootCanvas()->height-140,300,100,0,arrowImg);
	XuiShowWindow(XuiRootCanvas(),1,0);*/



	BYTE byRetval	=	0;
	dmgKeypad_Clear();
	dwBeginTime = OsGetTickCount();
	do
	{
		if(((OsGetTickCount() - dwBeginTime) /1000) == iRemainingTime)
		{
			if(iRemainingTime%2)
				dmgBasic_RfLogoPowerOn();
			else
				dmgBasic_RfLogoPowerOff();
			iRemainingTime++;
		}

		if(dmgPicc_Detect(DMG_PICC_DETECT_MODE_MIFARE, &chCardType, &byUidLen, byaUid) == SUCCESSFUL)
		{
			(*o_byUidLen) = byUidLen;
			(*o_byCardType) =  chCardType;
			memcpy(o_byaUid, byaUid, byUidLen);
			//XuiDestroyWindow(gif1Win);
			//gif1Win = NULL;
			//XuiDestroyWindow(gif2Win);
			//gif2Win = NULL;

			if(g_stGlVar.m_sTerminalConfig.m_byDetectBeepEnable)
				cusCommonPlayAudio("./data/beep.wav");
			//XuiClearArea(XuiRootCanvas(), 0 , 440 , 800 , 40);
			//XuiImgFree(stateImg);
			return SUCCESSFUL;
		}

		if(dmgKeypad_GetKey() == KEY_CANCEL)
		{
			byRetval = QCARD_SERVICE_ERRCODE_CANCEL_BY_USER;
			break;
		}

	} while((OsGetTickCount() - dwBeginTime) < (i_byTimeoutSec * 1000));

	if((OsGetTickCount() - dwBeginTime) >= (i_byTimeoutSec * 1000))
		byRetval = QCARD_SERVICE_ERRCODE_TIMEOUT;

	//XuiDestroyWindow(gif1Win);
	//gif1Win = NULL;
	//XuiDestroyWindow(gif2Win);
	//gif2Win = NULL;


	//XuiClearArea(XuiRootCanvas(), 0 , 440 , 800 , 40);
	//XuiImgFree(stateImg);
	return byRetval;
}
#endif

BYTE qCardDetectWrapper(BYTE i_byTimeoutSec, BYTE* o_byaUid,BYTE* o_byUidLen, BYTE* o_byCardType){

#ifdef PX7_DEV
	return qCardServiceUIdetectCard(i_byTimeoutSec,o_byaUid,o_byUidLen,o_byCardType,TRUE);
#endif

#ifdef S800_DEV
		 DWORD dwRet = SUCCESSFUL;
		 dwRet = detectCard(30,o_byaUid,o_byUidLen,o_byCardType);
		 if(dwRet == ERR_CANCEL_BY_USER)
			 return QCARD_SERVICE_ERRCODE_CANCEL_BY_USER;
		 return dwRet;
#endif

#ifdef S900_DEV

		 DWORD dwRet = 102;
		 dwRet = commonSrv_detectCard(TAP_CARD_TIMEOUT_SEC, MESSAGE_TAP_CARD, &o_byUidLen, o_byaUid,&o_byCardType);//detectCard(30,o_byaUid,o_byUidLen,o_byCardType);
		 if(dwRet==TRUE)
			 return SUCCESSFUL;
		 else
			 return dwRet;

#endif

}



#if defined(S900_DEV) || defined(PX7_DEV)
BYTE qCardDetectWrapperWithMsgID(BYTE i_byTimeoutSec, BYTE* o_byaUid,BYTE* o_byUidLen, BYTE* o_byCardType,DWORD i_dwMsgId){

		 DWORD dwRet = 102;
		 dwRet = commonSrv_detectCard(TAP_CARD_TIMEOUT_SEC, i_dwMsgId, &o_byUidLen, o_byaUid,&o_byCardType);//detectCard(30,o_byaUid,o_byUidLen,o_byCardType);
		 if(dwRet==TRUE)
			 return SUCCESSFUL;
		 else
			 return dwRet;


}
#endif



void qCardExaminSamCreditWrapper(){
#ifdef PX7_DEV

	sSamGetTehranCredit(&tehranAppletInfo.m_byaCredit1);
	if(tehranAppletInfo.m_byaCredit1 < g_stGlVar.m_sTerminalConfig.m_dwTehranChargeThresholdCredit)
		g_stGlVar.m_sTermIntConfig.m_dwNextCreditRequestTime = 0;

	OsLog(LOG_DEBUG,"\n@qCardServiceUIWrappers.c:qCardExaminSamCreditWrapper>New Sam Credit : [%lu] \n",tehranAppletInfo.m_byaCredit1);
#endif

#if defined(S800_DEV)
	sSamGetTehranCredit(&tehranClient.m_byaCredit1);
	if(tehranClient.m_byaCredit1 < g_stGlVar.m_sTerminalConfig.m_dwTehranChargeThresholdCredit)
		g_stGlVar.m_sTermIntConfig.m_dwNextCreditRequestTime = 0;

#endif

#if  defined(S900_DEV)
	DWORD dwCurrentCredit = 0;
		DWORD	creditStatus	= 0;


			ssamExamineSamCredit(TEHRAN,WALLET_1,&dwCurrentCredit,&creditStatus);
		OsLog(LOG_DEBUG,"\___________________________Tehran Thresh : %d,%d \n",g_stGlVar.m_sTerminalConfig.m_dwTehranChargeThresholdCredit,dwCurrentCredit);
			if(dwCurrentCredit < g_stGlVar.m_sTerminalConfig.m_dwTehranChargeThresholdCredit)
			{
			 ssamCreditIncrement(TEHRAN,WALLET_1,dwCurrentCredit);

			}
#endif


}
