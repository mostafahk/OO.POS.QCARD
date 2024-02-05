/*
 * qCard.c
 *
 *  Created on: May 20, 2019
 *      Author: f.karamiyar
 */
#include <define.h>


#ifndef T610_DEV
#include <BZ2/bzlib.h>
#include <QCard/qCard.h>
#include <QCard/sohaTrnSettFile.h>
#else
#include "qCard.h"
#endif

// this is a comment for qcard service
// in the ms Yaghmaee's computer.
// and for test it is in Ali Fallah's Computer
// and other firends.


BYTE byaTDesKey[16] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

/*static BYTE	gs_byaS0KB[7]	= "\x60\x54\xac\x95\x41\xc8";


static BYTE	gs_byaS3KA[7]	= "\x7b\xcb\x47\x74\xec\x8f";
static BYTE	gs_byaS3KB[7]	= "\xc5\x52\xc1\xb9\x23\x95";

static BYTE	gs_byaS4KA[7]	= "\x22\xec\xe9\x31\x64\x61";
static BYTE	gs_byaS4KB[7]	= "\xf4\xa4\xaa\x2f\x63\xa4";

static BYTE	gs_byaS5KA[7]	= "\xae\x4b\x49\x7a\x25\x27";
static BYTE	gs_byaS5KB[7]	= "\x25\xec\xb7\xb2\xba\xb1";

static BYTE	gs_byaS15KA[7]	= "\x16\xD2\xCE\xA1\x6A\xB6";
static BYTE	gs_byaS15KB[7]	= "\x65\x49\x1F\x24\x56\x6B";*/

static BYTE	byaQcardKey[7];

BYTE  TehranBKeys[][6] = {
{ 0x60, 0x54, 0xAC, 0x95, 0x41, 0xC8 },
{ 0x82, 0x8D, 0xDE, 0xEE, 0x4D, 0x98 },
{ 0xED, 0x2B, 0x22, 0x92, 0x91, 0x67 },
{ 0xC5, 0x52, 0xC1, 0xB9, 0x23, 0x95 },
{ 0xF4, 0xA4, 0xAA, 0x2F, 0x63, 0xA4 },
{ 0x25, 0xEC, 0xB7, 0xB2, 0xBA, 0xB1 },
{ 0x8B, 0x02, 0xEF, 0x84, 0xCD, 0xF1 },
{ 0xD7, 0x0A, 0x49, 0x4B, 0xBD, 0x4A },
{ 0x4E, 0xF5, 0x53, 0x17, 0xED, 0x30 },
{ 0xCD, 0x84, 0xA6, 0x96, 0xA7, 0x63 },
{ 0xFD, 0xB1, 0xC2, 0xB7, 0x94, 0xC1 },
{ 0x91, 0x8E, 0x43, 0x05, 0x92, 0xBD },
{ 0x1C, 0x15, 0x11, 0x61, 0xDA, 0x96 },
{ 0x56, 0x88, 0x25, 0xD6, 0x5A, 0x21 },
{ 0x17, 0x11, 0x5E, 0x59, 0x5B, 0x1C },
{ 0xE7, 0x2C, 0x6E, 0x8A, 0x3E, 0xB8 }
};


void logBlock(BYTE* byaBlockData){


	OsLog(LOG_DEBUG,"\n\n\n Block >>>>>> [%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x]\n",
			byaBlockData[0],byaBlockData[1],byaBlockData[2],byaBlockData[3],byaBlockData[4],byaBlockData[5],
			byaBlockData[6],byaBlockData[7],byaBlockData[8],byaBlockData[9],byaBlockData[10],byaBlockData[11],
			byaBlockData[12],byaBlockData[13],byaBlockData[14],byaBlockData[15]);


}

void logBlockSector(BYTE* byaBlockData,DWORD dwSector,DWORD dwBlock){


	OsLog(LOG_DEBUG,"\n@qCard.c:logBlockSector>Sector: %d  Block: %d >> [%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x][%02x]\n",
			dwSector,dwBlock,byaBlockData[0],byaBlockData[1],byaBlockData[2],byaBlockData[3],byaBlockData[4],byaBlockData[5],
			byaBlockData[6],byaBlockData[7],byaBlockData[8],byaBlockData[9],byaBlockData[10],byaBlockData[11],
			byaBlockData[12],byaBlockData[13],byaBlockData[14],byaBlockData[15]);


}

BYTE qCardCompareUid(BYTE* i_byaDetectedUid,BYTE* i_byaWrittenUid)
{
	if(i_byaDetectedUid[0] == i_byaWrittenUid[3] && i_byaDetectedUid[1] == i_byaWrittenUid[2] &&
		i_byaDetectedUid[2] == i_byaWrittenUid[1] && i_byaDetectedUid[3] == i_byaWrittenUid[0])
		return TRUE;
	else return FALSE;
}
BYTE getQcardAuthKeys(BYTE* byaCardUid, BYTE byUidLen, BYTE* byaQcardKeys){

	ST_APDU_REQ req;
	ST_APDU_RSP	rsp;
	int iRet = 0;
	BYTE byaAtr[128]={0};

	memset(&rsp,0,sizeof(ST_APDU_RSP));
	memset(&req,0,sizeof(ST_APDU_REQ));

	/*
	iRet=OsIccInit(g_stGlVar.m_sTerminalConfig.m_bySamslot,0x20,byaAtr);
	if (iRet!=0)
	{
		OsLog(LOG_DEBUG,"\n@sSam.c:ssamInit>ICC init Error:%d\n",iRet);
		return iRet;
	}
*/
	memcpy(req.Cmd,"\x91\x10\x00\x00",4);
	req.LC = 0x04;
	req.LE = 0x06;

	req.DataIn[0] = byaCardUid[3];
	req.DataIn[1] = byaCardUid[2];
	req.DataIn[2] = byaCardUid[1];
	req.DataIn[3] = byaCardUid[0];

	iRet = OsIccExchange(g_stGlVar.m_sTerminalConfig.m_bySamslot,0,&req,&rsp);
	OsLog(LOG_DEBUG, "\n@qCard.c:getQcardAuthKeys>GETKEY :Return %d  Operation Result:[%02x][%02x]\n",iRet,rsp.SWA,rsp.SWB);
	if( iRet != 0)
	{
			return ERR_READ_ETICKET_DATA;
	}


	if(rsp.SWA != 0x90 || rsp.SWB != 0x00)
		return ERR_READ_ETICKET_DATA;

	memcpy(byaQcardKeys,rsp.DataOut,6);
	return SUCCESSFUL;

}


//Sector Zero Block One

void qCardTransactionDataProcess(BYTE* byaBlockData,qCardTransactionData * qCardTranData){

	qCardTranData->q_wTransactionSectorPointer = byaBlockData[0]*256 +  byaBlockData[1] ;    //index 0 most significant

	qCardTranData->q_dwDecrementalCounter = (DWORD)(byaBlockData[16 + 3] << 24 | byaBlockData[16 + 2] << 16 | byaBlockData[16 + 1] << 8 | byaBlockData[16]); // index 3 most significant

}


void qCardTransactionDataGenerate(qCardTransactionData * qCardTranData,BYTE* byaBlockData){

	DWORD 	dwCurrentIndex = 0;

	OsLog(LOG_DEBUG,"\nqCard.c:qCardTransactionDataGenerate>q_wTransactionSectorPointer : %d\n", qCardTranData->q_wTransactionSectorPointer);

	byaBlockData[dwCurrentIndex] = qCardTranData->q_wTransactionSectorPointer / 256;
	byaBlockData[dwCurrentIndex + 1] = qCardTranData->q_wTransactionSectorPointer % 256;
	dwCurrentIndex += 2;


}


//Sector One Block Zero
void qCardDataProcess( BYTE* byaBlockData , qCardData * o_sqCardData ){

	DWORD 	dwCurrentIndex = 0;


	o_sqCardData->q_wContractType = byaBlockData[dwCurrentIndex] +  byaBlockData[dwCurrentIndex + 1]*256 ;    //index 1 most significant
	dwCurrentIndex += 2;
	OsLog(LOG_DEBUG,"\n@qCard.c:qCardDataProcess>contract type: %d\n",o_sqCardData->q_wContractType);

	memcpy(o_sqCardData->q_byaUid,byaBlockData + dwCurrentIndex, 4);
	dwCurrentIndex += 4;

	o_sqCardData->q_byExtraUid = byaBlockData[dwCurrentIndex] ;
	dwCurrentIndex += 1;

	o_sqCardData->q_wTagType = byaBlockData[dwCurrentIndex]*256 +  byaBlockData[dwCurrentIndex + 1] ;
	dwCurrentIndex += 2;

	utlByteArrayToDword(&o_sqCardData->q_dwDepositeValue,byaBlockData + dwCurrentIndex , 4);
	dwCurrentIndex += 4;

	o_sqCardData->q_byIsValid = byaBlockData[dwCurrentIndex] ;
	dwCurrentIndex += 1;

	o_sqCardData->q_wCity = byaBlockData[dwCurrentIndex]*256 +  byaBlockData[dwCurrentIndex + 1] ;
	dwCurrentIndex += 2;

	utlByteArrayToDword(&o_sqCardData->q_dwIssueDateTime,byaBlockData + dwCurrentIndex , 4);
	dwCurrentIndex += 2;

	utlByteArrayToDword(&o_sqCardData->q_dwIssueDateTime,byaBlockData + dwCurrentIndex , 4);
	dwCurrentIndex += 4;

	utlByteArrayToDword(&o_sqCardData->q_dwIssueOperator,byaBlockData + dwCurrentIndex , 4);
	dwCurrentIndex += 4;

	utlByteArrayToDword(&o_sqCardData->q_dwIssueStation,byaBlockData + dwCurrentIndex , 4);
	dwCurrentIndex += 4;

	utlByteArrayToDword(&o_sqCardData->q_dwIssuerMachine,byaBlockData + dwCurrentIndex , 4);

}




void qCardDataGenerate(qCardData * i_sqCardData , BYTE* o_byaBlockData){

	DWORD 	dwCurrentIndex = 0;
	memset(o_byaBlockData , 0 , sizeof(o_byaBlockData));

	o_byaBlockData[dwCurrentIndex + 1] = i_sqCardData->q_wContractType / 256;
	o_byaBlockData[dwCurrentIndex] = i_sqCardData->q_wContractType % 256;
	dwCurrentIndex += 2;

	memcpy(o_byaBlockData,i_sqCardData->q_byaUid,4);
	dwCurrentIndex += 4;

	o_byaBlockData[dwCurrentIndex] = i_sqCardData->q_byExtraUid ;
	dwCurrentIndex += 1;

	o_byaBlockData[dwCurrentIndex] = i_sqCardData->q_wTagType / 256;
	o_byaBlockData[dwCurrentIndex + 1] = i_sqCardData->q_wTagType % 256;
	dwCurrentIndex += 2;


	utlIntToByteArray(i_sqCardData->q_dwDepositeValue, o_byaBlockData + dwCurrentIndex, 4);
	dwCurrentIndex += 4;


	o_byaBlockData[dwCurrentIndex] = i_sqCardData->q_byIsValid;
	dwCurrentIndex += 1;

	o_byaBlockData[dwCurrentIndex] = i_sqCardData->q_wCity / 256;
	o_byaBlockData[dwCurrentIndex + 1] = i_sqCardData->q_wCity%256;
	dwCurrentIndex += 2;

	utlIntToByteArray(i_sqCardData->q_dwIssueDateTime, o_byaBlockData + dwCurrentIndex, 4);
	dwCurrentIndex += 4;

	utlIntToByteArray(i_sqCardData->q_dwIssueOperator,o_byaBlockData + dwCurrentIndex , 4);
	dwCurrentIndex += 4;

	utlIntToByteArray(i_sqCardData->q_dwIssueStation,o_byaBlockData + dwCurrentIndex , 4);
	dwCurrentIndex += 4;

	utlIntToByteArray(i_sqCardData->q_dwIssuerMachine,o_byaBlockData + dwCurrentIndex , 4);
	dwCurrentIndex += 4;



}


//Sector 2/3/4/5
void qCardTrnLogProcess(BYTE* byaBlockData,qCardTrnLog * o_sqCardTrnLog){

	DWORD 	dwCurrentIndex = 0;

	logBlock(byaBlockData);

	utlByteArrayToDword(&o_sqCardTrnLog->q_dwSequenceNumber,byaBlockData  , 4);
	dwCurrentIndex += 4;
	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardTrnLogProcess> q_dwSequenceNumber: %d  \n\n",o_sqCardTrnLog->q_dwSequenceNumber);


	utlByteArrayToDword(&o_sqCardTrnLog->q_dwTotalAmount,byaBlockData + dwCurrentIndex  , 4);
	dwCurrentIndex += 4;

	OsLog(LOG_DEBUG, "\n\n@qCard.c:i_sqCardTrnLog->q_dwTotalAmount: %d  \n\n",o_sqCardTrnLog->q_dwTotalAmount);

	o_sqCardTrnLog->q_wTravelsCount = byaBlockData[dwCurrentIndex]*256 +  byaBlockData[dwCurrentIndex + 1] ;    //index 0 most significant
	dwCurrentIndex += 2;

	utlByteArrayToDword(&o_sqCardTrnLog->q_dwTranDateTime,byaBlockData + dwCurrentIndex  , 4);
	dwCurrentIndex += 4;

	o_sqCardTrnLog->q_wTranStation = byaBlockData[dwCurrentIndex]*256 +  byaBlockData[dwCurrentIndex + 1] ;    //index 0 most significant
	dwCurrentIndex += 2;

	o_sqCardTrnLog->q_sCardDiscountData.wTravelsCount = byaBlockData[dwCurrentIndex]*256 +  byaBlockData[dwCurrentIndex + 1] ;    //index 0 most significant
	dwCurrentIndex += 2;

	utlByteArrayToDword(&o_sqCardTrnLog->q_sCardDiscountData.dwDiscountAmount,byaBlockData + dwCurrentIndex, 4);
	dwCurrentIndex += 4;

	o_sqCardTrnLog->q_sCardDiscountData.wDiscountInitDate = byaBlockData[dwCurrentIndex]*256 +  byaBlockData[dwCurrentIndex + 1] ;    //index 0 most significant
	dwCurrentIndex += 2;

	o_sqCardTrnLog->q_sCardDiscountData.wDiscountExpDate = byaBlockData[dwCurrentIndex]*256 +  byaBlockData[dwCurrentIndex + 1] ;    //index 0 most significant
	dwCurrentIndex += 2;

	o_sqCardTrnLog->q_sCardServiceData[0].byServiceCode = byaBlockData[dwCurrentIndex];
	dwCurrentIndex += 1;

	o_sqCardTrnLog->q_sCardServiceData[1].byServiceCode = byaBlockData[dwCurrentIndex];
	dwCurrentIndex += 1;

	o_sqCardTrnLog->q_sCardServiceData[2].byServiceCode = byaBlockData[dwCurrentIndex];
	dwCurrentIndex += 1;

	o_sqCardTrnLog->q_sCardServiceData[3].byServiceCode = byaBlockData[dwCurrentIndex];
	dwCurrentIndex += 1;

	o_sqCardTrnLog->q_sCardServiceData[4].byServiceCode = byaBlockData[dwCurrentIndex];
	dwCurrentIndex += 1;

	o_sqCardTrnLog->q_byTranType = byaBlockData[dwCurrentIndex];
}

void qCardTrnLogGenerate(qCardTrnLog * i_sqCardTrnLog ,BYTE* o_byaBlockData){

	DWORD 	dwCurrentIndex = 0;
	memset(o_byaBlockData , 0 , sizeof(o_byaBlockData));

	utlIntToByteArray(i_sqCardTrnLog->q_dwSequenceNumber,o_byaBlockData  , 4);
	dwCurrentIndex += 4;

	utlIntToByteArray(i_sqCardTrnLog->q_dwTotalAmount,o_byaBlockData + dwCurrentIndex  , 4);
	dwCurrentIndex += 4;


	o_byaBlockData[dwCurrentIndex] = i_sqCardTrnLog->q_wTravelsCount / 256;
	o_byaBlockData[dwCurrentIndex + 1] = i_sqCardTrnLog->q_wTravelsCount % 256;
	dwCurrentIndex += 2;

	utlIntToByteArray(i_sqCardTrnLog->q_dwTranDateTime,o_byaBlockData + dwCurrentIndex ,4);
	dwCurrentIndex += 4;

	o_byaBlockData[dwCurrentIndex] = i_sqCardTrnLog->q_wTranStation / 256;
	o_byaBlockData[dwCurrentIndex + 1] = i_sqCardTrnLog->q_wTranStation % 256;
	dwCurrentIndex += 2;

	o_byaBlockData[dwCurrentIndex] = i_sqCardTrnLog->q_sCardDiscountData.wTravelsCount / 256;
	o_byaBlockData[dwCurrentIndex + 1] = i_sqCardTrnLog->q_sCardDiscountData.wTravelsCount % 256;
	dwCurrentIndex += 2;

	utlIntToByteArray(i_sqCardTrnLog->q_sCardDiscountData.dwDiscountAmount,o_byaBlockData + dwCurrentIndex, 4);
	dwCurrentIndex += 4;

	o_byaBlockData[dwCurrentIndex] = i_sqCardTrnLog->q_sCardDiscountData.wDiscountInitDate / 256;
	o_byaBlockData[dwCurrentIndex + 1] = i_sqCardTrnLog->q_sCardDiscountData.wDiscountInitDate % 256;
	dwCurrentIndex += 2;

	o_byaBlockData[dwCurrentIndex] = i_sqCardTrnLog->q_sCardDiscountData.wDiscountExpDate / 256;
	o_byaBlockData[dwCurrentIndex + 1] = i_sqCardTrnLog->q_sCardDiscountData.wDiscountExpDate % 256;
	dwCurrentIndex += 2;

	o_byaBlockData[dwCurrentIndex] = i_sqCardTrnLog->q_sCardServiceData[0].byServiceCode ;
	dwCurrentIndex += 1;

	o_byaBlockData[dwCurrentIndex] = i_sqCardTrnLog->q_sCardServiceData[1].byServiceCode ;
	dwCurrentIndex += 1;

	o_byaBlockData[dwCurrentIndex] = i_sqCardTrnLog->q_sCardServiceData[2].byServiceCode ;
	dwCurrentIndex += 1;

	o_byaBlockData[dwCurrentIndex] = i_sqCardTrnLog->q_sCardServiceData[3].byServiceCode ;
	dwCurrentIndex += 1;

	o_byaBlockData[dwCurrentIndex] = i_sqCardTrnLog->q_sCardServiceData[4].byServiceCode ;
	dwCurrentIndex += 1;

	o_byaBlockData[dwCurrentIndex] = i_sqCardTrnLog->q_byTranType ;

}



//Sector Six  Block Zero
void qCardPersonalizedDataProcess(BYTE* byaBlockData,qCardPersonalizedData * o_sqCardPersonalizedData){

	DWORD 	dwCurrentIndex = 0;
	memcpy(o_sqCardPersonalizedData->q_byaName, byaBlockData + dwCurrentIndex , 16);
	dwCurrentIndex += 16;

	memcpy(o_sqCardPersonalizedData->q_byaPhone, byaBlockData + dwCurrentIndex , 16);
	dwCurrentIndex += 16;

	memcpy(o_sqCardPersonalizedData->q_byaNationalId, byaBlockData + dwCurrentIndex , 6);
	dwCurrentIndex += 6;

	o_sqCardPersonalizedData->q_wBirthDay = byaBlockData[dwCurrentIndex]*256 +  byaBlockData[dwCurrentIndex + 1] ;    //index 0 most significant
	dwCurrentIndex += 2;

	memcpy(o_sqCardPersonalizedData->q_byaPersonalId, byaBlockData + dwCurrentIndex , 7);
	dwCurrentIndex += 6;
}

//Sector Seven
void qCardSellerDataProcess(BYTE* byaBlockData,qCardSellerData * o_sqCardSellerData){

	DWORD 	dwCurrentIndex = 0;
	memcpy(o_sqCardSellerData->q_byaPinBlock, byaBlockData + dwCurrentIndex , 8);
	dwCurrentIndex += 16;

	o_sqCardSellerData->q_byaSellerLevel = byaBlockData[dwCurrentIndex];//*256 +  byaBlockData[dwCurrentIndex + 1] ;    //index 0 most significant



}

void qCardPersonalizedDataGenerate(qCardPersonalizedData * i_sqCardPersonalizedData,BYTE* o_byaBlockData){

	DWORD 	dwCurrentIndex = 0;

	memcpy(o_byaBlockData + dwCurrentIndex,i_sqCardPersonalizedData->q_byaName , 16);
	dwCurrentIndex += 16;

	memcpy(o_byaBlockData + dwCurrentIndex,i_sqCardPersonalizedData->q_byaPhone , 16);
	dwCurrentIndex += 16;

	memcpy(o_byaBlockData + dwCurrentIndex ,&i_sqCardPersonalizedData->q_byaNationalId,  sizeof(i_sqCardPersonalizedData->q_byaNationalId));
	dwCurrentIndex += 6;

	o_byaBlockData[dwCurrentIndex] = 	i_sqCardPersonalizedData->q_wBirthDay / 256;
	o_byaBlockData[dwCurrentIndex + 1] = 	i_sqCardPersonalizedData->q_wBirthDay % 256;
	dwCurrentIndex += 2;

	memcpy(o_byaBlockData + dwCurrentIndex ,&i_sqCardPersonalizedData->q_byaPersonalId,  sizeof(i_sqCardPersonalizedData->q_byaPersonalId));
	dwCurrentIndex += 7;

}



void  qCardEncodeDecodeBlockData(BYTE* i_byaUid,BYTE i_byBlockNo,BYTE* i_byaDataIn,BYTE* o_byaDataOut)
{
	BYTE byaFixedBytes[5] = {0};
	BYTE byCounterA	=	0;
	BYTE byCounterB =	0;

	if(i_byBlockNo == BLOCK_ZERO)
		memcpy(byaFixedBytes,"\x04\x08\x0A\x0E\x0E",5);
	else
		memcpy(byaFixedBytes,"\x02\x06\x08\x0C\x0C",5);

	for(byCounterA = 0;byCounterA < 16; byCounterA++)
	{
		if(byCounterA == byaFixedBytes[byCounterB] && byCounterB <= 3)
		{
			o_byaDataOut[byCounterA] = i_byaDataIn[byCounterA];
			byCounterB++;

		}else
			o_byaDataOut[byCounterA] = i_byaUid[byCounterA%4]^i_byaDataIn[byCounterA]^i_byaDataIn[byaFixedBytes[byCounterB]];
	}

}

BYTE qCardVerifyLastTrn(qCard* i_sQcard)
{
	ST_APDU_REQ 	req;
	ST_APDU_RSP 	res;
	int				iRet				=	0;
	BYTE 			byIndex 			=	0;
	BYTE 			byaLastTrnLog[16] 	= 	{0};

	BYTE 			byaCardInfo[16] 	= 	{0};

	memset(&req,0,sizeof(ST_APDU_REQ));
	memset(&res,0,sizeof(ST_APDU_RSP));


	utlIntToByteArray(i_sQcard->q_sTranLog.q_sCardDiscountData.dwDiscountAmount,byaLastTrnLog + byIndex,2);
	byIndex +=2;

	utlIntToByteArray(i_sQcard->q_sTranLog.q_wTravelsCount,byaLastTrnLog + byIndex,2);
	byIndex +=2;

	utlIntToByteArray(i_sQcard->q_sTranLog.q_dwTotalAmount,byaLastTrnLog + byIndex,4);
	byIndex +=4;

	utlIntToByteArray(i_sQcard->q_sTranLog.q_sCardDiscountData.wTravelsCount,byaLastTrnLog + byIndex,2);
	byIndex +=2;

	utlIntToByteArray(i_sQcard->q_sTranLog.q_sCardDiscountData.wDiscountExpDate,byaLastTrnLog + byIndex,2);
	byIndex +=2;

	utlIntToByteArray(i_sQcard->q_sTranLog.q_dwTranDateTime,byaLastTrnLog + byIndex,4);
	byIndex +=4;


	byIndex = 0;
	memcpy(byaCardInfo + byIndex,&i_sQcard->q_sCardData.q_wContractType,2);
	byIndex +=2;

	memcpy(byaCardInfo + byIndex,i_sQcard->q_sCardData.q_byaUid,4);
	byIndex +=4;

	byaCardInfo[byIndex] = i_sQcard->q_sCardData.q_byExtraUid;
	byIndex++;

	byaCardInfo[byIndex] = i_sQcard->q_sCardData.q_byIsValid;
	byIndex++;

	memcpy(byaCardInfo + byIndex,"\x00\x00",2);
	byIndex +=2;

	OsLog(LOG_DEBUG,"\nCityCode: %d\n",i_sQcard->q_sCardData.q_wCity);
	utlIntToByteArray(i_sQcard->q_sCardData.q_wCity,byaCardInfo + byIndex,2);
	byIndex +=2;

	utlIntToByteArray(i_sQcard->q_sCardData.q_dwDepositeValue,byaCardInfo + byIndex,4);
	byIndex +=4;


	OsLog(LOG_DEBUG,"\n@qCard.c:qCardVerifyLastTrn>Last TXN\n");
	logBlock(byaLastTrnLog);

	OsLog(LOG_DEBUG,"\n@qCard.c:qCardVerifyLastTrn>Last TXN SIGN\n");
	logBlock(i_sQcard->q_sTranLog.q_byaTrnLogSignature);

	memcpy(req.Cmd,"\x91\x26",2);
	req.Cmd[2] = 0/*g_stGlVar.m_sTerminalConfig.m_bySamP1*/;
	req.Cmd[3] = 0/*g_stGlVar.m_sTerminalConfig.m_bySamP2*/;
	req.LC = 0x49;
	req.LE = 0x00;
	memcpy(req.DataIn,byaCardInfo,16);
	memcpy(req.DataIn + 16,i_sQcard->q_sCardData.q_byaCardDataSignature,16);
	memcpy(req.DataIn + 32,byaLastTrnLog,16);
	memcpy(req.DataIn + 48,i_sQcard->q_sTranLog.q_byaTrnLogSignature,16);
	memcpy(req.DataIn + 64,i_sQcard->q_sCardData.q_byaUid,4);

	req.DataIn[68] = (i_sQcard->q_sCardTransactionData.q_dwDecrementalCounter & 0xff000000)>>24;
	req.DataIn[69] = (i_sQcard->q_sCardTransactionData.q_dwDecrementalCounter & 0x00ff0000)>>16;
	req.DataIn[70] = (i_sQcard->q_sCardTransactionData.q_dwDecrementalCounter & 0x0000ff00)>>8;
	req.DataIn[71] = i_sQcard->q_sCardTransactionData.q_dwDecrementalCounter & 0x000000ff;
	req.DataIn[72] = (BYTE)i_sQcard->q_sCardData.q_wContractType & 0x00ff;;

	{
		BYTE byaBuffer[256] = {0};
		int	i	=	0;
		for(i = 0; i < 73; i++)
			sprintf(byaBuffer + strlen(byaBuffer),"%02X",req.DataIn[i]);

		OsLog(LOG_DEBUG,"\n@qCard.c:qCardVerifyLastTrn>req.cmd:%02X%02X%02X%02X%02X%02X\n\n",req.Cmd[0],req.Cmd[1],req.Cmd[2],req.Cmd[3],req.LC,req.LE);
		OsLog(LOG_DEBUG,"\n@qCard.c:qCardVerifyLastTrn>req.DataIn: %s\n",byaBuffer);
	}


	iRet = OsIccExchange(g_stGlVar.m_sTerminalConfig.m_bySamslot,0,&req,&res);
	OsLog(LOG_DEBUG,"\n@qCard.c:qCardVerifyLastTrn>Counter %ld RESULT: %02X%02X\n",i_sQcard->q_sCardTransactionData.q_dwDecrementalCounter,res.SWA,res.SWB);

	{
		BYTE byaBuffer[1024] = {0};
		BYTE byCounter = 0;
		sprintf(byaBuffer,"\ncmd: %02X%02X%02X%02X lc: %02X le: %02X\n DataIn:",req.Cmd[0],req.Cmd[1],req.Cmd[2],
		req.Cmd[3],req.LC,req.LE);
		for(byCounter = 0;byCounter < req.LC;byCounter++)
		{
			sprintf(byaBuffer + strlen(byaBuffer),"%02X",req.DataIn[byCounter]);
		}

		sprintf(byaBuffer + strlen(byaBuffer),"\nResponse %02X%02X Integer LenOut: %d\nDataOut:",res.SWA,res.SWB,res.LenOut);
		for(byCounter = 0;byCounter < req.LC;byCounter++)
		{
			sprintf(byaBuffer + strlen(byaBuffer),"%02X",res.DataOut[byCounter]);
		}

		OsLog(LOG_DEBUG,"\nIccExchange%s\n\n",byaBuffer);
	}
	if( iRet != 0 || res.SWA!= 0x90 || res.SWB!=00)
	{
		return FALSE;
	}

	return TRUE;
}


DWORD qCardGetData(qCard* o_sQcard , BYTE* byaUid , BYTE byUidLen)
{

	BYTE	byaBlockData[48]							=	{0};
	BYTE	byaBlockZeroData[16]						=	{0};
	BYTE	byaBlockOneData[16]							=	{0};
	BYTE	byaBlockTwoData[16]							=	{0};

	BYTE 	byTrnLogSector								=	0;
	DWORD	dwRetVal									=	0;
	BYTE 	byaTranBlock[48]							=	{0};
	BYTE 	byaDecTranBlock[48]							=	{0};


	memset(byaQcardKey,0,sizeof(byaQcardKey));
	memset(o_sQcard, 0, sizeof(qCard));

	OsLog(LOG_DEBUG, "\n@qCardGetData before AuthKeys\n");

	//Get Key From Sam
	if(getQcardAuthKeys(byaUid,byUidLen, byaQcardKey)){
		return ERR_SAM_ERROR;
	}


	OsLog(LOG_DEBUG,"\nCard Keys: [%02x][%02x][%02x][%02x][%02x][%02x]\n",byaQcardKey[0],byaQcardKey[1],byaQcardKey[2],
	byaQcardKey[3],byaQcardKey[4],byaQcardKey[5]);


	// Decremental counter + Sector Pointer
	OsLog(LOG_DEBUG, "\n@qCard.c:qCardGetData> Authenticating Sector Zero\n");
	dwRetVal = dmgPicc_MifareAuthenticate(DMG_PICC_MIFARE_KEY_TYPE_B, byaQcardKey, SECTOR_ZERO, byUidLen , byaUid);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Authenticating Sector Zero Failed\n\n");
		return	ERR_READ_ETICKET_DATA;
	}

	memset(byaBlockData, 0, sizeof(byaBlockData));
	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Reading  Sector Zero\n\n");
	dwRetVal = dmgPicc_MifareRead(SECTOR_ZERO, BLOCK_ONE, byaBlockData);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Reading  Sector Zero Failed\n\n");
		return	ERR_READ_ETICKET_DATA;
	}
	logBlockSector(byaBlockData,0,1);

    dwRetVal = dmgPicc_MifareRead(SECTOR_ZERO, BLOCK_TWO , byaBlockData + 16);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Reading  Sector Zero Failed\n\n");
		return	ERR_READ_ETICKET_DATA;
	}

	logBlockSector(byaBlockData + 16,0,2);
	qCardTransactionDataProcess(byaBlockData,&o_sQcard->q_sCardTransactionData);

	OsLog(LOG_DEBUG, "\n\n@qCard.c: pointer >>  %d \n\n",o_sQcard->q_sCardTransactionData.q_wTransactionSectorPointer);
	OsLog(LOG_DEBUG, "\n\nM@HCheck: @qCard.c: Main decremental counter >>  %ld \nContractType=%d\n",o_sQcard->q_sCardTransactionData.q_dwDecrementalCounter,o_sQcard->q_sCardData.q_wContractType);

	////////////////////
	//Checking Value Block
	//@To be Tested
	//@Temp
	//if (g_stGlVar.m_sTerminalConfig.m_byValueBlckBckupTestEnable==1){
	//	if (1/*!o_sQcard->q_sCardTransactionData.q_dwDecrementalCounter*/){
	//		o_sQcard->q_sCardTransactionData.q_bValueBlockIsLost=TRUE;
	//	}
	//}
	//else{
		//normal state!
		if (!o_sQcard->q_sCardTransactionData.q_dwDecrementalCounter){
			o_sQcard->q_sCardTransactionData.q_bValueBlockIsLost=TRUE;
		}
	//}


	//if (o_sQcard->q_sCardTransactionData.q_bValueBlockIsLost){

	//Reading Sector 8 : Decremental Counter Backup
	memset(byaBlockData,0,sizeof(byaBlockData));
	OsLog(LOG_DEBUG, "\n@qCard.c:qCardGetData> Authenticating Sector Eight\n");
	dwRetVal = dmgPicc_MifareAuthenticate(DMG_PICC_MIFARE_KEY_TYPE_B, byaQcardKey, SECTOR_EIGHT, byUidLen , byaUid);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Authenticating Sector Eight Failed\n\n");
		return	ERR_READ_ETICKET_DATA;
	}
	//Block 0 : Backup pointer
	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Reading  Sector Eight, Block 0 : Backup pointer\n\n");
	dwRetVal = dmgPicc_MifareRead(SECTOR_EIGHT, BLOCK_ZERO, byaBlockData);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Reading  Sector Eight Failed\n\n");
		return	ERR_READ_ETICKET_DATA;
	}
	logBlockSector(byaBlockData,8,0);
	OsLog(LOG_DEBUG, "\n\nM@Hcheck #1 Backup pointer=%d\n\n",byaBlockData[0]);

	if (byaBlockData[0]){
		o_sQcard->q_sCardTransactionData.q_wDecrementalCounterBckUpPointer=1;
	}
	else{
		o_sQcard->q_sCardTransactionData.q_wDecrementalCounterBckUpPointer=0;
	}
	OsLog(LOG_DEBUG, "\n\nM@Hcheck #2 DecrementalCounterBckUpPointer=%d\n\n",o_sQcard->q_sCardTransactionData.q_wDecrementalCounterBckUpPointer);

	//Read Backup 0: Block 1
	//Let's read backup0 & backup1
	//*************
	//According to CMS, BackupValue Block Byte[0] is the most significant and Byte[3] is the least significant!
	//*************
	OsLog(LOG_DEBUG, "\n\nM@Hcheck #3 Let's read Backup0: Sector Eight, Block 1\n\n");
	dwRetVal = dmgPicc_MifareRead(SECTOR_EIGHT, BLOCK_ONE, byaBlockData + 16);
	if(dwRetVal != SUCCESSFUL){
				OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Reading  Sector Eight Block 1 Failed\n\n");
				return	ERR_READ_ETICKET_DATA;
	}
	logBlockSector(byaBlockData + 16,8,1);
	o_sQcard->q_sCardTransactionData.q_dwDecrementalCounterBckUp0= (DWORD)(byaBlockData[16] << 24 | byaBlockData[17] << 16 | byaBlockData[18] << 8 | byaBlockData[19]); // index 0 most significant
	OsLog(LOG_DEBUG, "\n\nM@Hcheck #4 DecrementalCounterBckUp0=%ld\n\n",o_sQcard->q_sCardTransactionData.q_dwDecrementalCounterBckUp0);

		//}

	//Read Backup 1: Block 2
	OsLog(LOG_DEBUG, "\n\nM@Hcheck #5 Let's read Backup1: Sector Eight, Block 2\n\n");
	dwRetVal = dmgPicc_MifareRead(SECTOR_EIGHT, BLOCK_TWO, byaBlockData + 32);
	if(dwRetVal != SUCCESSFUL){
				OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Reading  Sector Eight Block 2 Failed\n\n");
				return	ERR_READ_ETICKET_DATA;
	}
	logBlockSector(byaBlockData + 32,8,2);
	o_sQcard->q_sCardTransactionData.q_dwDecrementalCounterBckUp1= (DWORD)(byaBlockData[32] << 24 | byaBlockData[33] << 16 | byaBlockData[34] << 8 | byaBlockData[35]); // index 0 most significant
	OsLog(LOG_DEBUG, "\n\nM@Hcheck #6 DecrementalCounterBckUp1=%ld\n\n",o_sQcard->q_sCardTransactionData.q_dwDecrementalCounterBckUp1);

		//}

	if (o_sQcard->q_sCardTransactionData.q_bValueBlockIsLost){
		OsLog(LOG_DEBUG, "\n*****>>>>>Decremental Counter is lost!<<<<<<****\n => Let's get its backup\n");
		if(o_sQcard->q_sCardTransactionData.q_wDecrementalCounterBckUpPointer)
			o_sQcard->q_sCardTransactionData.q_dwDecrementalCounter=o_sQcard->q_sCardTransactionData.q_dwDecrementalCounterBckUp0;
		else
			o_sQcard->q_sCardTransactionData.q_dwDecrementalCounter=o_sQcard->q_sCardTransactionData.q_dwDecrementalCounterBckUp1;
	}

	OsLog(LOG_DEBUG, "\n\nM@HCheck: @qCard.c: Main decremental counter >>  %ld \n\n",o_sQcard->q_sCardTransactionData.q_dwDecrementalCounter);

	if (!o_sQcard->q_sCardTransactionData.q_dwDecrementalCounter){
			OsLog(LOG_DEBUG, "\n\nM@HCheck: Invalid Backup of Decremental Counter, Break!!!");
			return ERR_INVALID_CARD;
	}

	////////////////////
	if(o_sQcard->q_sCardTransactionData.q_wTransactionSectorPointer < 16){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> invalid transaction sector pointer\n\n");
		return	ERR_INVALID_CARD;
	}

	// qCard Data
	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Authenticating Sector One\n\n");
	dwRetVal = dmgPicc_MifareAuthenticate(DMG_PICC_MIFARE_KEY_TYPE_B,byaQcardKey, SECTOR_ONE, byUidLen , byaUid);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Authenticating Sector One Failed \n\n");
		return	ERR_READ_ETICKET_DATA;
	}

	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Reading Sector One Block Zero\n\n");
	memset(byaBlockData, 0, sizeof(byaBlockData));
	dwRetVal = dmgPicc_MifareRead(SECTOR_ONE, BLOCK_ZERO , byaBlockData);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Reading Sector One Block Zero Failed\n\n");
		return	ERR_READ_ETICKET_DATA;
	}


	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Reading Sector One Block One\n\n");
	dwRetVal = dmgPicc_MifareRead(SECTOR_ONE, BLOCK_ONE , byaBlockData + 16);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Reading Sector One Block One Failed\n\n");
		return	ERR_READ_ETICKET_DATA;
	}

	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Reading Sector One Block Two\n\n");
	dwRetVal = dmgPicc_MifareRead(SECTOR_ONE, BLOCK_TWO , byaBlockData + 32);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Reading Sector One Block Two Failed\n\n");
		return	ERR_READ_ETICKET_DATA;
	}

	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Processing Sector One Card Data\n\n");
	logBlockSector(byaBlockData,1,0);
	qCardDataProcess(byaBlockData,&o_sQcard->q_sCardData);

	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> ExtraUid : %d \n\n",o_sQcard->q_sCardData.q_byExtraUid);
	if(o_sQcard->q_sCardData.q_byExtraUid == 0)
		o_sQcard->q_sCardData.q_byUidLen = 4;


	OsLog(LOG_DEBUG,"\n@qCard.c:qCardGetData> UID: %02X%02X%02X%02X\n",byaUid[0],byaUid[1],byaUid[2],byaUid[3]);
	if(!qCardCompareUid(byaUid,o_sQcard->q_sCardData.q_byaUid))
		return ERR_INVALID_CARD;




	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Processing Sector One Block Two\n\n");
	logBlockSector(byaBlockData,1,2);
	memcpy(o_sQcard->q_sCardData.q_byaCardDataSignature,byaBlockData + 32,sizeof(o_sQcard->q_sCardData.q_byaCardDataSignature));




	switch (/*o_sQcard->q_sCardTransactionData.q_wTransactionSectorPointer*/o_sQcard->q_sCardTransactionData.q_dwDecrementalCounter%3) {
		case /*16*/0:    // 0x0010
			byTrnLogSector = 2;
			break;

		case /*256*/1:   //  0x0100
			byTrnLogSector = 3;
			break;

		case /*4096*/2:	 //  0x1000
			byTrnLogSector = 4;
			break;
		}


	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> q_wTransactionSectorPointer : %d  tran log sector : %d\n\n",o_sQcard->q_sCardTransactionData.q_wTransactionSectorPointer,byTrnLogSector);
	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Authenticating Log Sector... \n\n");
	dwRetVal = dmgPicc_MifareAuthenticate(DMG_PICC_MIFARE_KEY_TYPE_B, byaQcardKey, byTrnLogSector, byUidLen , byaUid);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Authenticating Log Sector Failed \n\n");
		return	ERR_READ_ETICKET_DATA;
	}

	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Reading Log Sector Block zero... \n\n");
	memset(byaBlockZeroData, 0, sizeof(byaBlockZeroData));
	memset(byaBlockOneData , 0, sizeof(byaBlockOneData));
	memset(byaBlockTwoData , 0, sizeof(byaBlockTwoData));

	dwRetVal = dmgPicc_MifareRead(byTrnLogSector, BLOCK_ZERO , byaBlockZeroData);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Reading Log Sector block zero Failed \n\n");
		return	ERR_READ_ETICKET_DATA;
	}

	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Reading Log Sector Block one... \n\n");
	dwRetVal = dmgPicc_MifareRead(byTrnLogSector, BLOCK_ONE , byaBlockOneData);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Reading Log Sector Block one Failed \n\n");
		return	ERR_READ_ETICKET_DATA;
	}

	dwRetVal = dmgPicc_MifareRead(byTrnLogSector, BLOCK_TWO , byaBlockTwoData);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Reading Log Sector Block Two Failed \n\n");
		return	ERR_READ_ETICKET_DATA;
	}

	OsLog(LOG_DEBUG, "\n@qCard.c:qCardGetData> Sector %d data before decode\n",byTrnLogSector);
	logBlockSector(byaBlockZeroData,byTrnLogSector,0);
	logBlockSector(byaBlockOneData,byTrnLogSector,1);
	logBlockSector(byaBlockTwoData,byTrnLogSector,2);

	memset(byaBlockData,0,sizeof(byaBlockData));
	qCardEncodeDecodeBlockData(o_sQcard->q_sCardData.q_byaUid,BLOCK_ZERO,byaBlockZeroData,byaBlockData);
	qCardEncodeDecodeBlockData(o_sQcard->q_sCardData.q_byaUid,BLOCK_ONE,byaBlockOneData,byaBlockData + 16);
	qCardEncodeDecodeBlockData(o_sQcard->q_sCardData.q_byaUid,BLOCK_TWO,byaBlockTwoData,byaBlockData + 32);

	OsLog(LOG_DEBUG, "\n@qCard.c:qCardGetData> Sector %d data after decode\n",byTrnLogSector);
	logBlockSector(byaBlockData,byTrnLogSector,0);
	logBlockSector(byaBlockData + 16,byTrnLogSector,1);
	logBlockSector(byaBlockData + 32,byTrnLogSector,2);


	qCardTrnLogProcess(byaBlockData,&(o_sQcard->q_sTranLog));
	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Processing Log Sector Block ...: Amount: %d \n\n",o_sQcard->q_sTranLog.q_dwTotalAmount);

	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Processing Log Sector Block Two ... \n\n");
	memcpy(o_sQcard->q_sTranLog.q_byaTrnLogSignature,byaBlockData + 32,sizeof(o_sQcard->q_sTranLog.q_byaTrnLogSignature));


	// Last Charge Tran Log
	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Authenticating Sector Five ... \n\n");
	dwRetVal = dmgPicc_MifareAuthenticate(DMG_PICC_MIFARE_KEY_TYPE_B, byaQcardKey, SECTOR_FIVE, byUidLen , byaUid);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Authenticating Sector Five Failed \n\n");
		return	ERR_READ_ETICKET_DATA;
	}



	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Reading Sector Five Block Zero... \n\n");
	memset(byaBlockZeroData, 0, sizeof(byaBlockZeroData));
	memset(byaBlockOneData , 0, sizeof(byaBlockOneData));
	memset(byaBlockTwoData , 0, sizeof(byaBlockTwoData));
	dwRetVal = dmgPicc_MifareRead(SECTOR_FIVE, BLOCK_ZERO , byaBlockZeroData);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Reading Sector Five Failed \n\n");
		return	ERR_READ_ETICKET_DATA;
	}

	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Reading Sector Five Block One ... \n\n");
	dwRetVal = dmgPicc_MifareRead(SECTOR_FIVE, BLOCK_ONE , byaBlockOneData);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Reading Sector Five Block One Failed \n\n");
		return	ERR_READ_ETICKET_DATA;
	}

	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Reading Sector Five Block Two \n\n");
	dwRetVal = dmgPicc_MifareRead(SECTOR_FIVE, BLOCK_TWO , byaBlockTwoData);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Reading Sector Five Block Two Failed\n\n");
		return	ERR_READ_ETICKET_DATA;
	}

	memset(byaBlockData,0,sizeof(byaBlockData));
	qCardEncodeDecodeBlockData(o_sQcard->q_sCardData.q_byaUid,BLOCK_ZERO,byaBlockZeroData,byaBlockData);
	qCardEncodeDecodeBlockData(o_sQcard->q_sCardData.q_byaUid,BLOCK_ONE,byaBlockOneData,byaBlockData + 16);
	qCardEncodeDecodeBlockData(o_sQcard->q_sCardData.q_byaUid,BLOCK_TWO,byaBlockTwoData,byaBlockData + 32);


	logBlockSector(byaBlockData,SECTOR_FIVE,0);
	logBlockSector(byaBlockData + 16,SECTOR_FIVE,1);
	logBlockSector(byaBlockData + 32,SECTOR_FIVE,2);

	BYTE byaNotIssuedData[32] = {0};
	if(memcmp(byaBlockData,byaNotIssuedData,sizeof(byaNotIssuedData)) == 0) {
		o_sQcard->q_byIsIssued = FALSE;
	}else{
		o_sQcard->q_byIsIssued = TRUE;
	}
	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Processing Sector Five ... \n\n");

	qCardTrnLogProcess(byaBlockData,&o_sQcard->q_sLastChargeTrnLog);

	memcpy(o_sQcard->q_sLastChargeTrnLog.q_byaTrnLogSignature,byaBlockData + 32,sizeof(o_sQcard->q_sLastChargeTrnLog.q_byaTrnLogSignature));

	memset(byaBlockData , 0, sizeof(byaBlockData));

	// Personalized Data
	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Authentication Sector Six \n\n");
	dwRetVal = dmgPicc_MifareAuthenticate(DMG_PICC_MIFARE_KEY_TYPE_B,byaQcardKey, SECTOR_SIX, byUidLen , byaUid);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Authentication Sector Six Failed \n\n");
		return	ERR_READ_ETICKET_DATA;
	}

	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Reading Sector six Block zero \n\n");
	dwRetVal = dmgPicc_MifareRead(SECTOR_SIX, BLOCK_ZERO , byaBlockData);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Reading Sector six Block zero Failed \n\n");
		return	ERR_READ_ETICKET_DATA;
	}

	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Reading Sector six Block One \n\n");
	dwRetVal = dmgPicc_MifareRead(SECTOR_SIX, BLOCK_ONE , byaBlockData + 16);
	if(dwRetVal != SUCCESSFUL)
		return	ERR_READ_ETICKET_DATA;

	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Reading Sector six Block TWo \n\n");
	dwRetVal = dmgPicc_MifareRead(SECTOR_SIX, BLOCK_TWO , byaBlockData + 32);
	if(dwRetVal != SUCCESSFUL)
		return	ERR_READ_ETICKET_DATA;



	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Processing Sector six Block zero \n\n");
	logBlockSector(byaBlockData     ,SECTOR_SIX,BLOCK_ZERO);
	logBlockSector(byaBlockData + 16,SECTOR_SIX,BLOCK_ONE);
	logBlockSector(byaBlockData + 32,SECTOR_SIX,BLOCK_TWO);

	qCardPersonalizedDataProcess(byaBlockData,&o_sQcard->q_sPersonalizedData);

	if(o_sQcard->q_sCardData.q_wContractType == QCARD_TYPE_SELLER)
	{

	// Seller Data
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Authentication Sector Seven \n\n");
		dwRetVal = dmgPicc_MifareAuthenticate(DMG_PICC_MIFARE_KEY_TYPE_B,byaQcardKey, SECTOR_SEVEN, byUidLen , byaUid);
		if(dwRetVal != SUCCESSFUL){
			OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Authentication Sector Seven Failed \n\n");
			return	ERR_READ_ETICKET_DATA;
		}

		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Reading Sector Seven Block zero \n\n");
		dwRetVal = dmgPicc_MifareRead(SECTOR_SEVEN, BLOCK_ZERO , byaBlockData);
		if(dwRetVal != SUCCESSFUL){
			OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Reading Sector Seven Block zero Failed \n\n");
			return	ERR_READ_ETICKET_DATA;
		}

		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Reading Sector Seven Block One \n\n");
		dwRetVal = dmgPicc_MifareRead(SECTOR_SEVEN, BLOCK_ONE , byaBlockData + 16);
		if(dwRetVal != SUCCESSFUL)
			return	ERR_READ_ETICKET_DATA;

		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Reading Sector Seven Block TWo \n\n");
			dwRetVal = dmgPicc_MifareRead(SECTOR_SEVEN, BLOCK_TWO , byaBlockData + 32);
			if(dwRetVal != SUCCESSFUL)
				return	ERR_READ_ETICKET_DATA;


		logBlockSector(byaBlockData,SECTOR_SEVEN,BLOCK_ZERO);
		logBlockSector(byaBlockData + 16,SECTOR_SEVEN,BLOCK_ONE);
		logBlockSector(byaBlockData + 32,SECTOR_SEVEN,BLOCK_TWO);

		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Processing Sector seven  \n\n");
		qCardSellerDataProcess(byaBlockData,&o_sQcard->q_sqCardSellerData);
	}

    //-----------------------------------------------------------------------
    //Reading Sector 9
    OsLog(LOG_DEBUG, "\n@qCard.c:qCardGetData> Authenticating Sector 9\n");
    dwRetVal = dmgPicc_MifareAuthenticate(DMG_PICC_MIFARE_KEY_TYPE_B,
                                          byaQcardKey, SECTOR_NINE, byUidLen , byaUid);
    if(dwRetVal != SUCCESSFUL){
        OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> "
                         "Authenticating Sector 9 Failed \n\n");
        return	ERR_READ_ETICKET_DATA;
    }
    OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> "
                     "Reading Sector 9 Block Zero\n\n");
    memset(byaBlockData, 0, sizeof(byaBlockData));
    dwRetVal = dmgPicc_MifareRead(SECTOR_NINE, BLOCK_ZERO, byaBlockData);

    if(dwRetVal != SUCCESSFUL)
    {	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> "
                          "Reading Sector 9 Block Zero Failed\n\n");
        return	ERR_READ_ETICKET_DATA;
    }

    OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> "
                     "Reading Sector 9 Block One\n\n");
    dwRetVal = dmgPicc_MifareRead(SECTOR_NINE, BLOCK_ONE, byaBlockData + 16);
    if(dwRetVal != SUCCESSFUL)
    {
        OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> "
                         "Reading Sector 9 Block One Failed\n\n");
        return	ERR_READ_ETICKET_DATA;
    }

    OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> "
                     "Reading Sector 9 Block Two\n\n");
    dwRetVal = dmgPicc_MifareRead(SECTOR_NINE, BLOCK_TWO, byaBlockData + 32);
    if(dwRetVal != SUCCESSFUL)
    {
        OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> "
                         "Reading Sector 9 Block Two Failed\n\n");
        return	ERR_READ_ETICKET_DATA;
    }

    /* if we are in online mode check sector 9
     * if sector 9 is empty (0x00 or 0xFF) override the q_byIsIssued*/
    if(g_stGlVar.m_sTerminalConfig.m_byOnlineCardModeEnable)
    {
        BYTE *byPointer = (BYTE *) byaBlockData;

        OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>check sector 9 to be 0\n\n");

        //--------------- show sector 9 data ------------------
        int z = 0;
        uint8_t buffer[100] = {0};
        for(z = 0; z < 48; z++)
            sprintf(buffer + strlen(buffer),"%02X",byaBlockData[z]);
        OsLog(LOG_DEBUG,"sector 9 is --> %s",buffer);
        //--------------- end show sector 9 data ------------------


        // check all 48 byte is zero
       if (((*byPointer == 0) || (*byPointer == 255)) && memcmp(byPointer, byPointer+1,48-1) == 0)
        {
            OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>all sector 9 is 0 (q_byIsIssued = FALSE)\n\n");
            o_sQcard->q_byIsIssued = FALSE;
        }
        else
        {
            OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>all sector 9 is NOT 0 (q_byIsIssued = TRUE)\n\n");
            o_sQcard->q_byIsIssued = TRUE;
        }
    }
    OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> "
                     "Processing Sector 9 Card Data\n\n");
    logBlockSector(byaBlockData,9,0);
    logBlockSector(byaBlockData,9,1);
    logBlockSector(byaBlockData,9,2);

    //-----------------------------------------------------------------------

	if(!qCardVerifyLastTrn(o_sQcard)){
        OsLog(LOG_DEBUG, "ERROR: can not verify Last transaction");
		return ERR_INVALID_CARD;
    }


	OsLog(LOG_DEBUG, "\n ____________________@qCard.c:qCardGetData>  return succesfull\n");

	return SUCCESSFUL;
}



DWORD qCardWriteTrnLog(qCard* i_sQcard,BYTE* i_byaUid,BYTE i_byUidLen, BYTE* i_byaOnlineCardSectorData)
{
	BYTE	byaBlockData[48]							=	{0};
	BYTE	byaTmpBlockData[48]							=	{0};
	BYTE	byNextTrnLogSector							=	 0;
	BYTE	byaTranBlock[48]							=	{0};
	BYTE	byaEncTranBlock[48]							=	{0};
	BYTE	byEncTranLen								=	0;

	BYTE	byaBlockZero[16]							=	{0};
	BYTE	byaBlockOne[16]								=	{0};
	BYTE	byaBlockTwo[16]								=	{0};
	DWORD	dwRetVal									=	 0;


////////
	switch (/*i_sQcard->q_sCardTransactionData.q_wTransactionSectorPointer*/i_sQcard->q_sCardTransactionData.q_dwDecrementalCounter%3) {
		case /*16*/0:    // 0x0010
			byNextTrnLogSector = 4;
			break;

		case /*256*/1:   //  0x0100
			byNextTrnLogSector = 2;
			break;

		case /*4096*/2:	 //  0x1000
			byNextTrnLogSector = 3;
			break;
		}

	qCardTrnLogGenerate(&i_sQcard->q_sNewTrn,byaBlockData);

	qCardEncodeDecodeBlockData(i_sQcard->q_sCardData.q_byaUid,BLOCK_ZERO,byaBlockData,byaBlockZero);
	qCardEncodeDecodeBlockData(i_sQcard->q_sCardData.q_byaUid,BLOCK_ONE,byaBlockData + 16,byaBlockOne);
	qCardEncodeDecodeBlockData(i_sQcard->q_sCardData.q_byaUid,BLOCK_TWO,i_sQcard->q_sNewTrn.q_byaTrnLogSignature,byaBlockTwo);


	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> UID Len : %d  %02x%02x%02x%02x\n\n",i_byUidLen, i_byaUid[0], i_byaUid[1], i_byaUid[2], i_byaUid[3] );
	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Authenticating  LogSector: %d  \n\n",byNextTrnLogSector);
	OsLog(LOG_DEBUG,"\nCard Keys: [%02x][%02x][%02x][%02x][%02x][%02x]\n",byaQcardKey[0],byaQcardKey[1],byaQcardKey[2],
	byaQcardKey[3],byaQcardKey[4],byaQcardKey[5]);
	dwRetVal = dmgPicc_MifareAuthenticate(DMG_PICC_MIFARE_KEY_TYPE_B,byaQcardKey, byNextTrnLogSector , i_byUidLen , i_byaUid);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  Authenticating Log Sector Failed \n\n");
		return	ERR_READ_ETICKET_DATA;
	}

	memset(byaBlockData, 0, sizeof(byaBlockData));
	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  writing Log Sector block zero... \n\n");
	logBlockSector(byaBlockData,byNextTrnLogSector,BLOCK_ZERO);
	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  writing Log Sector block one... \n\n");
	logBlockSector(byaBlockData  + 16,byNextTrnLogSector,BLOCK_ONE);
	dwRetVal = dmgPicc_MifareWrite(byNextTrnLogSector, BLOCK_ZERO , byaBlockZero);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  writing Log Sector block zero failed  : %d\n\n",dwRetVal);
		return	ERR_UPDATE_ETICKET_DATA;
	}


	dwRetVal = dmgPicc_MifareWrite(byNextTrnLogSector, BLOCK_ONE , byaBlockOne);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> writing Log Sector block one failed \n\n");
		return	ERR_READ_ETICKET_DATA;
	}


	dwRetVal = dmgPicc_MifareWrite(byNextTrnLogSector, BLOCK_TWO , byaBlockTwo);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> writing Log Sector block two  failed\n\n");
		return	ERR_READ_ETICKET_DATA;
	}

	//Write  Last Charge Tran Log
	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Authenticating Sector Five \n\n");
	dwRetVal = dmgPicc_MifareAuthenticate(DMG_PICC_MIFARE_KEY_TYPE_B, byaQcardKey , SECTOR_FIVE, i_byUidLen , i_byaUid);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Authenticating Sector Five failed \n\n");
		return	ERR_UPDATE_ETICKET_DATA;
	}

	dwRetVal = dmgPicc_MifareWrite(SECTOR_FIVE, BLOCK_ZERO , byaBlockZero);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  writing Log Sector block zero failed  : %d\n\n",dwRetVal);
		return	ERR_UPDATE_ETICKET_DATA;
	}


	dwRetVal = dmgPicc_MifareWrite(SECTOR_FIVE, BLOCK_ONE , byaBlockOne);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> writing Log Sector block one failed \n\n");
		return	ERR_READ_ETICKET_DATA;
	}


	dwRetVal = dmgPicc_MifareWrite(SECTOR_FIVE, BLOCK_TWO , byaBlockTwo);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> writing Log Sector block two  failed\n\n");
		return	ERR_READ_ETICKET_DATA;
	}

    // TODO: if qCardisIssue{
    //  update sector 9
    // card is issued

#ifdef S900_DEV
    // this is write sector nine on card (but only on S900 ahvaz is enable)
    if (g_stGlVar.m_sTerminalConfig.m_byOnlineCardModeEnable){
        if (i_sQcard->q_byIsIssued == FALSE){
            //Write  OnlineCardData
            OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog> Authenticating Sector 9 \n\n");
            dwRetVal = dmgPicc_MifareAuthenticate(DMG_PICC_MIFARE_KEY_TYPE_B, byaQcardKey , SECTOR_NINE, i_byUidLen , i_byaUid);
            if(dwRetVal != SUCCESSFUL){
                OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Authenticating Sector 9 failed \n\n");
                return	ERR_UPDATE_ETICKET_DATA;
            }

            dwRetVal = dmgPicc_MifareWrite(SECTOR_NINE, BLOCK_ZERO , i_byaOnlineCardSectorData + 16 * 0);
            if(dwRetVal != SUCCESSFUL){
                OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  writing Log Sector block zero failed  : %d\n\n",dwRetVal);
                return	ERR_UPDATE_ETICKET_DATA;
            }

            dwRetVal = dmgPicc_MifareWrite(SECTOR_NINE, BLOCK_ONE , i_byaOnlineCardSectorData + 16 * 1);
            if(dwRetVal != SUCCESSFUL){
                OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  writing Log Sector block one failed  : %d\n\n",dwRetVal);
                return	ERR_UPDATE_ETICKET_DATA;
            }

            dwRetVal = dmgPicc_MifareWrite(SECTOR_NINE, BLOCK_TWO , i_byaOnlineCardSectorData + 16 * 2);
            if(dwRetVal != SUCCESSFUL){
                OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData>  writing Log Sector block two failed  : %d\n\n",dwRetVal);
                return	ERR_UPDATE_ETICKET_DATA;
            }
        }

    }
#endif

    // Update Transaction Sig + Sector Pointer

	switch (byNextTrnLogSector) {
		case 2:
			i_sQcard->q_sCardTransactionData.q_wTransactionSectorPointer = 16 ;  //0x0010
		break;

		case 3:
			i_sQcard->q_sCardTransactionData.q_wTransactionSectorPointer = 256 ;  //0x0100
		break;

		case 4:
			i_sQcard->q_sCardTransactionData.q_wTransactionSectorPointer = 4096 ;  //0x1000
		break;
	}


	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> writing Sector Zero  \n\n");
	dwRetVal = dmgPicc_MifareAuthenticate(DMG_PICC_MIFARE_KEY_TYPE_B, byaQcardKey, SECTOR_ZERO, i_byUidLen , i_byaUid);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> writing Sector Zero failed  \n\n");
		return	ERR_UPDATE_ETICKET_DATA;
	}

	memset(byaBlockData, 0, sizeof(byaBlockData));
	qCardTransactionDataGenerate( &i_sQcard->q_sCardTransactionData, byaBlockData);

	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> writing sector zero  block One  \n\n");
	logBlock(byaBlockData);
	logBlockSector(byaBlockData,SECTOR_ZERO,BLOCK_ONE);
	dwRetVal = dmgPicc_MifareWrite(SECTOR_ZERO, BLOCK_ONE, byaBlockData);
	if(dwRetVal != SUCCESSFUL){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> writing sector zero  block One  failed : %d \n\n",dwRetVal);
		return	ERR_UPDATE_ETICKET_DATA;
	}

	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> writing sector zero  block One finished. \n\n");
	OsLog(LOG_DEBUG, "\n\nM@HCHeck => i_sQcard->q_sCardTransactionData=%d,DecrementalCounter=%ld\n\n",i_sQcard->q_sCardTransactionData.q_wTransactionSectorPointer,i_sQcard->q_sCardTransactionData.q_dwDecrementalCounter);
/*#ifndef S900_DEV
	BYTE byaCounterVB[5] = {0};
	memset(byaCounterVB, 0, sizeof(byaCounterVB));
	byaCounterVB[0] = 0x01;
	dwRetVal =  OsMifareOperate ('-',2, byaCounterVB,2);*/
//#else
	BYTE byaValueBlock[16] = {0};
	BYTE byaTemp[16] = {0};
	DWORD dwCurrentVB = 0;
	DWORD dwCurrentBckUpVB = 0;
	BYTE NormalVBWriten = 0;
	BYTE BackUpVBWriten = 0;
	BYTE BackUpVBPointerWriten=0;
	BYTE BackUpPointer = 0;
	BYTE byCounter = 0;
	BYTE o_byUidLen = 0;
	BYTE o_byaUid[16] = {0};
	BYTE byaCompareUid[16] = {0};
	BYTE o_byCardType = 0;
	BYTE byRetval = 0;
	BYTE bySuccess = FALSE;
	int i = 0;


    while(byCounter < 4 && !bySuccess){
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog> writing value block counter %d state %d\n\n",byCounter,bySuccess);
		////////////////////
		/*
		uiDisplayMsgWithId(MESSAGE_REMOVE_CARD, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_INFORM_WHITE_SCREEN);
		OsSleep(1000);
		*/
		///////////////////
		if(byCounter >= 1){

			utlUnsuccessAlarm();
			dmgPicc_Reset();

			OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog> tap card for writing value block try count %d\n\n",byCounter);
			memset(byaValueBlock,0,sizeof(byaValueBlock));
#ifdef S900_DEV
			byRetval = commonSrv_detectCard(TAP_CARD_TIMEOUT_SEC,MESSAGE_UPDATE_CARD_ERROR_RETAP , &o_byUidLen, o_byaUid,&o_byCardType);//detectCard(30,o_byaUid,o_byUidLen,o_byCardType);
#endif

#ifdef PX7_DEV
			byRetval = detectCard(30,o_byaUid,&o_byUidLen,&o_byCardType,TRUE);
#endif

#ifdef S800_DEV
			byRetval =	detectCard(30,o_byaUid,&o_byUidLen,&o_byCardType);
#endif


			for(i = 0;i<i_sQcard->q_sCardData.q_byUidLen;i++){
				byaCompareUid[i] = i_sQcard->q_sCardData.q_byaUid[i_sQcard->q_sCardData.q_byUidLen - i - 1];
			}

			OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog> try %d - ret %d:\ncard UID:[%02X]%02X[%02X]%02X -- last UID: [%02X]%02X[%02X]%02X\n\n",byCounter,byRetval,
					o_byaUid[0],o_byaUid[1],o_byaUid[2],o_byaUid[3],
					byaCompareUid[0],byaCompareUid[1],byaCompareUid[2],byaCompareUid[3]);


			if(byRetval == TRUE && memcmp(o_byaUid,byaCompareUid,4) == 0){

				//@Here
				//Read and check sector 8
				//if (i_sQcard->q_sCardTransactionData.q_bValueBlockIsLost){
					OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog> Let's read backup: Sector 8");
					dwRetVal = dmgPicc_MifareAuthenticate(DMG_PICC_MIFARE_KEY_TYPE_B, byaQcardKey, SECTOR_EIGHT, i_byUidLen , i_byaUid);
					if(dwRetVal != SUCCESSFUL){
						OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Authenticating Sector Eight Failed\n\n");
						byCounter++;
						continue;
					}

					if(i_sQcard->q_sCardTransactionData.q_wDecrementalCounterBckUpPointer){
						OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog>BckUpPointer=%d, Let's read backup: Sector 8, Block 2",i_sQcard->q_sCardTransactionData.q_wDecrementalCounterBckUpPointer);
						//Check if backup 1 has been updated
						dwRetVal = dmgPicc_MifareRead(SECTOR_EIGHT, BLOCK_TWO , byaValueBlock);
						if(dwRetVal != SUCCESSFUL){
							OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog>  reading value block failed in try %d  \n\n",byCounter);

							byCounter++;
							continue;
						}
						logBlockSector(byaValueBlock,SECTOR_EIGHT,BLOCK_TWO);

					}
					else{
						//Check if backup 0 has been updated
						OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog>BckUpPointer=%d, Let's read backup: Sector 8, Block 1",i_sQcard->q_sCardTransactionData.q_wDecrementalCounterBckUpPointer);
						dwRetVal = dmgPicc_MifareRead(SECTOR_EIGHT, BLOCK_ONE , byaValueBlock);
						if(dwRetVal != SUCCESSFUL){
							OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog>  reading value block failed in try %d  \n\n",byCounter);

							byCounter++;
							continue;
						}
						logBlockSector(byaValueBlock,SECTOR_EIGHT,BLOCK_ONE);
					}
					dwCurrentBckUpVB = (DWORD)(byaValueBlock[0] << 24 | byaValueBlock[1] << 16 | byaValueBlock[2] << 8 | byaValueBlock[3]); // index 0 most significant

					OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog> Compare dwCurrentBckUpVB = %ld - initial state= %ld\n\n",dwCurrentBckUpVB,i_sQcard->q_sCardTransactionData.q_dwDecrementalCounter);
					if(dwCurrentBckUpVB < i_sQcard->q_sCardTransactionData.q_dwDecrementalCounter)
						BackUpVBWriten=1;
					////////////
					//check if DecrementalCounterBckUpPointer has been updated
					memset(byaValueBlock,0,sizeof(byaValueBlock));
					OsLog(LOG_DEBUG, "\n\nPrevious BckUpPointer=%d, Let's read new: Sector 8, Block 0",i_sQcard->q_sCardTransactionData.q_wDecrementalCounterBckUpPointer);
					dwRetVal = dmgPicc_MifareRead(SECTOR_EIGHT, BLOCK_ZERO , byaValueBlock);
					if(dwRetVal != SUCCESSFUL){
						OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog>  reading value block failed in try %d  \n\n",byCounter);

						byCounter++;
						continue;
					}
					logBlockSector(byaValueBlock,SECTOR_EIGHT,BLOCK_ONE);
					OsLog(LOG_DEBUG, "\n\New BckUpPointer=%d",byaValueBlock[0]);
					if (byaValueBlock[0]!= i_sQcard->q_sCardTransactionData.q_wDecrementalCounterBckUpPointer)
						BackUpVBPointerWriten=1;
					////////////
					//check if main valueblock has been updated
					OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog>Let's read Normal Value Block");
					memset(byaValueBlock,0,sizeof(byaValueBlock));
					dwRetVal = dmgPicc_MifareAuthenticate(DMG_PICC_MIFARE_KEY_TYPE_B, byaQcardKey, SECTOR_ZERO, i_byUidLen , i_byaUid);
					if(dwRetVal != SUCCESSFUL){
						OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog>  authenticate sector 0 failed in try %d  \n\n",byCounter);
						byCounter++;
						continue;
					}

					dwRetVal = dmgPicc_MifareRead(SECTOR_ZERO, BLOCK_TWO , byaValueBlock);
					if(dwRetVal != SUCCESSFUL){
						OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog>  reading value block failed in try %d  \n\n",byCounter);

						byCounter++;
						continue;
					}
				//}
				dwCurrentVB = (DWORD)(byaValueBlock[3] << 24 | byaValueBlock[2] << 16 | byaValueBlock[1] << 8 | byaValueBlock[0]); // index 3 most significant
				//For cards that their Value block are lost, reading value block returns 0
				//So following check works for both valid and invalid ValueBlocks
				OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog> Compare vb current %ld - initial state %d\n\n",dwCurrentVB,i_sQcard->q_sCardTransactionData.q_dwDecrementalCounter);
				if(dwCurrentVB < i_sQcard->q_sCardTransactionData.q_dwDecrementalCounter)
					NormalVBWriten=1;

				if(NormalVBWriten && BackUpVBWriten && BackUpVBPointerWriten){
					bySuccess = TRUE;
					break;
				}

			}else{
				OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog>card detection failed in try count %d\n\n",byCounter);
				byCounter++;
				continue;
			}
		}

		//Check if DecrementalCounter(ValueBlock) is lost!
		//Write
		if (!i_sQcard->q_sCardTransactionData.q_bValueBlockIsLost && !NormalVBWriten){
			OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog> ValueBlock is ok and has not been decreased! \n\n");
			//Value Block is Ok
			BYTE byaCounterVB[5] = {0};
			memset(byaCounterVB, 0, sizeof(byaCounterVB));
			byaCounterVB[0] = 0x01;
			OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog> Decreasing Normal value block \n\n");
			dwRetVal =  OsMifareOperate ('-',2, byaCounterVB,2);
			if(dwRetVal != SUCCESSFUL){
				OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog> Decreasing value block failed %d \n\n",dwRetVal);
				byCounter++;
				continue;
			}

		}
		OsLog(LOG_DEBUG, "\n\nM@H Check BackUpVBWriten=%d, DecrementalCounter=%d\n\n",BackUpVBWriten, i_sQcard->q_sCardTransactionData.q_dwDecrementalCounter);
		OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog> writing value block SUCCESS\n\n");

		//Writing Value Block Backups and its pointer
		if(!BackUpVBWriten){
			OsLog(LOG_DEBUG, "\n\nM@HCheck: qCard.c:qCardWriteTrnLog> Backups are not written!\nWriting Backup value block: \n\n");
			//Creating Decremental Counter Block
			dwCurrentBckUpVB=i_sQcard->q_sCardTransactionData.q_dwDecrementalCounter-1;
			OsLog(LOG_DEBUG, "\n\nM@HCheck: qCard.c:qCardWriteTrnLog> new DecrementalCounter=%ld \n\n",dwCurrentBckUpVB);
			memset(byaValueBlock,0,sizeof(byaValueBlock));
			memset(byaTemp,0,sizeof(byaTemp));
			memcpy(byaTemp,&dwCurrentBckUpVB,sizeof(dwCurrentBckUpVB));
			//According to CMS, ValueBlock Byte[0] is most significant
			//Reverse the order of ValueBlock
			memcpy(byaValueBlock,byaTemp+3,1);
			memcpy(byaValueBlock+1,byaTemp+2,1);
			memcpy(byaValueBlock+2,byaTemp+1,1);
			memcpy(byaValueBlock+3,byaTemp,1);
			//////////////////
			logBlockSector(byaValueBlock,8,1);
			OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardWriteTrnLog> Let's write backup: Sector 8");
			dwRetVal = dmgPicc_MifareAuthenticate(DMG_PICC_MIFARE_KEY_TYPE_B, byaQcardKey, SECTOR_EIGHT, i_byUidLen , i_byaUid);
			if(dwRetVal != SUCCESSFUL){
				OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Authenticating Sector Eight Failed\n\n");
				byCounter++;
				continue;
			}
			OsLog(LOG_DEBUG, "===> DecrementalCounterBckUpPointer=%d\n\n",i_sQcard->q_sCardTransactionData.q_wDecrementalCounterBckUpPointer);

			if(i_sQcard->q_sCardTransactionData.q_wDecrementalCounterBckUpPointer){
				OsLog(LOG_DEBUG, "\n\nLet's write on BLOCK_TWO");
				dwRetVal = dmgPicc_MifareWrite(SECTOR_EIGHT, BLOCK_TWO, byaValueBlock);
				if(dwRetVal != SUCCESSFUL){
					OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> writing sector eight  block One  failed : %d \n\n",dwRetVal);
					byCounter++;
					continue;
				}
				BackUpPointer=0;
			}
			else{
				OsLog(LOG_DEBUG, "\n\nLet's write on BLOCK_ONE");
				dwRetVal = dmgPicc_MifareWrite(SECTOR_EIGHT, BLOCK_ONE, byaValueBlock);
				if(dwRetVal != SUCCESSFUL){
					OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> writing sector eight  block One  failed : %d \n\n",dwRetVal);
					byCounter++;
					continue;
				}
				BackUpPointer=1;
			}
			////////////////////
			/*
			uiDisplayMsgWithId(MESSAGE_REMOVE_CARD_1, UI_DISPLAY_WAIT_TYPE_NO_WAIT, UI_DISPLAY_MSG_TYPE_INFORM_WHITE_SCREEN);
			OsSleep(3000);
			///////////////////
			*/
			OsLog(LOG_DEBUG, "\n\nLet's write Backup Pointer");
			memset(byaValueBlock,0,sizeof(byaValueBlock));
			memcpy(byaValueBlock,&BackUpPointer,sizeof(BackUpPointer));
			logBlockSector(byaValueBlock,8,0);
			dwRetVal = dmgPicc_MifareWrite(SECTOR_EIGHT, BLOCK_ZERO, byaValueBlock);
			if(dwRetVal != SUCCESSFUL){
				OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> writing sector eight  BLOCK_ZERO  failed : %d \n\n",dwRetVal);
				byCounter++;
				continue;
			}
			OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> writing sector eight  BLOCK_ZERO  SUCCESSFUL : %d \n\n",dwRetVal);

		}
		bySuccess = TRUE;
		break;
	}


	OsLog(LOG_DEBUG, "\n\n@qCard.c:qCardGetData> Dooooooooooooooone \n\n");
	return SUCCESSFUL;
}


BYTE qCardGenerateTrnSignature(qCard* io_sQcard)
{
	ST_APDU_REQ 	req;
	ST_APDU_RSP 	res;
	int				iRet				=	0;
	BYTE 			byIndex 			=	0;
	BYTE 			byaLastTrnLog[16] 	= 	{0};
	BYTE 			byaNewTrnLog[16] 	= 	{0};

	memset(&req,0,sizeof(ST_APDU_REQ));
	memset(&res,0,sizeof(ST_APDU_RSP));

    //Get sam slot
#ifdef T610_DEV
    terminalConfigSt	sTerminalConfig;
    memset(&sTerminalConfig, 0, sizeof(terminalConfigSt));
    if(!termConfigGet(&sTerminalConfig))
        return ERR_GET_TERMINAL_CONFIG;
#endif
	utlIntToByteArray(io_sQcard->q_sTranLog.q_sCardDiscountData.dwDiscountAmount,byaLastTrnLog + byIndex,2);
	byIndex +=2;

	utlIntToByteArray(io_sQcard->q_sTranLog.q_wTravelsCount,byaLastTrnLog + byIndex,2);
	byIndex +=2;

	utlIntToByteArray(io_sQcard->q_sTranLog.q_dwTotalAmount,byaLastTrnLog + byIndex,4);
	byIndex +=4;

	utlIntToByteArray(io_sQcard->q_sTranLog.q_sCardDiscountData.wTravelsCount,byaLastTrnLog + byIndex,2);
	byIndex +=2;

	utlIntToByteArray(io_sQcard->q_sTranLog.q_sCardDiscountData.wDiscountExpDate,byaLastTrnLog + byIndex,2);
	byIndex +=2;

	utlIntToByteArray(io_sQcard->q_sTranLog.q_dwTranDateTime,byaLastTrnLog + byIndex,4);
	byIndex +=4;

	byIndex = 0;
	utlIntToByteArray(io_sQcard->q_sNewTrn.q_sCardDiscountData.dwDiscountAmount,byaNewTrnLog + byIndex,2);
	byIndex +=2;

	utlIntToByteArray(io_sQcard->q_sNewTrn.q_wTravelsCount,byaNewTrnLog + byIndex,2);
	byIndex +=2;

	utlIntToByteArray(io_sQcard->q_sNewTrn.q_dwTotalAmount,byaNewTrnLog + byIndex,4);
	byIndex +=4;

	utlIntToByteArray(io_sQcard->q_sNewTrn.q_sCardDiscountData.wTravelsCount,byaNewTrnLog + byIndex,2);
	byIndex +=2;

	utlIntToByteArray(io_sQcard->q_sNewTrn.q_sCardDiscountData.wDiscountExpDate,byaNewTrnLog + byIndex,2);
	byIndex +=2;

	utlIntToByteArray(io_sQcard->q_sNewTrn.q_dwTranDateTime,byaNewTrnLog + byIndex,4);
	byIndex +=4;

#ifdef T610_DEV
    memcpy(req.Cmd,"\x91\x12",2);
    req.LC = 0x39;
    req.Cmd[2] = sTerminalConfig.m_bySamP1;
    req.Cmd[3] = sTerminalConfig.m_bySamP2;
#else
	memcpy(req.Cmd,"\x91\x22",2);
    req.LC = 0x3A;
	req.Cmd[2] = g_stGlVar.m_sTerminalConfig.m_bySamP1;
	req.Cmd[3] = g_stGlVar.m_sTerminalConfig.m_bySamP2;
#endif

	req.LE = 0x10;
	memcpy(req.DataIn,byaLastTrnLog,16);
	memcpy(req.DataIn+16,io_sQcard->q_sTranLog.q_byaTrnLogSignature,16);
	memcpy(req.DataIn+32,byaNewTrnLog,16);

	OsLog(LOG_DEBUG,"\n@qCard.c:qCardGenerateTrnSignature>New TXN\n");
	logBlock(byaNewTrnLog);

	OsLog(LOG_DEBUG,"\n@qCard.c:qCardGenerateTrnSignature>Last TXN\n");
	logBlock(byaLastTrnLog);

	OsLog(LOG_DEBUG,"\n@qCard.c:qCardGenerateTrnSignature>Last TXN SIGN\n");
	logBlock(io_sQcard->q_sTranLog.q_byaTrnLogSignature);

	memcpy(req.DataIn + 48,io_sQcard->q_sCardData.q_byaUid,4);


	req.DataIn[52] = (io_sQcard->q_sCardTransactionData.q_dwDecrementalCounter & 0xff000000)>>24;
	req.DataIn[53] = (io_sQcard->q_sCardTransactionData.q_dwDecrementalCounter & 0x00ff0000)>>16;
	req.DataIn[54] = (io_sQcard->q_sCardTransactionData.q_dwDecrementalCounter & 0x0000ff00)>>8;
	req.DataIn[55] = io_sQcard->q_sCardTransactionData.q_dwDecrementalCounter  & 0x000000ff;

	req.DataIn[56] = (BYTE)(io_sQcard->q_sCardData.q_wContractType & 0x00ff);
#ifndef T610_DEV
	req.DataIn[57] = g_stGlVar.m_sTerminalConfig.m_byChargeCreditIdx;// config
#endif
	{
		char byaBuffer[256] = {0};
		int	i	=	0;
        for(i = 0; i < req.LC; i++)
			sprintf(byaBuffer + strlen(byaBuffer),"%02X",req.DataIn[i]);

		OsLog(LOG_DEBUG,"\n@qCard.c:qCardGenerateTrnSignature>req.DataIn: %s\n",byaBuffer);
	}
	iRet = OsIccExchange(g_stGlVar.m_sTerminalConfig.m_bySamslot,0,&req,&res);
    OsLog(LOG_DEBUG,"\n@qCard.c:qCardGenerateTrnSignature>Signature for Return %d -- %02X%02X\n",iRet,res.SWA,res.SWB);
	if( iRet != 0 || ( res.SWA!= 0x90 || res.SWB!=0x00 ))
		return	FALSE;
	OsLog(LOG_DEBUG,"\nNew Signature\n");
	memcpy(io_sQcard->q_sNewTrn.q_byaTrnLogSignature,res.DataOut,16);
	logBlock(io_sQcard->q_sNewTrn.q_byaTrnLogSignature);
	return TRUE;
}


BYTE qCardUpdateLastChargeInfo(qcardLastTrnLog* i_sLastChargeInfo)
{
	WORD wLen = 0;
	HFILE hFile = 0;

	if(!dmgFile_Open(QCARD_LAST_CHARGE_LOG, DMG_FILE_OPEN_MODE_CREATE, &hFile))
		return FALSE;

	dmgFile_Seek(hFile, 0, DMG_FILE_SEEK_SET);
	wLen = (WORD)sizeof(qcardLastTrnLog);
	if(!dmgFile_Write(hFile, (BYTE*)i_sLastChargeInfo, wLen))
	{
		dmgFile_Close(hFile);
		return FALSE;
	}

	dmgFile_Close(hFile);

	return TRUE;
}

BYTE qCardCheckLastChargeInfoExistance()
{
	return (dmgFile_GetSize(QCARD_LAST_CHARGE_LOG) > 0);
}


