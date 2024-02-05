//
// Created by f.karamiyar on 7/31/2019.
//

#ifndef PAYMENT_QCARDFILEOPERATIONWRAPPER_H
#define PAYMENT_QCARDFILEOPERATIONWRAPPER_H


#ifdef T610_DEV
#define	HFILE	DWORD

#define DMG_FILE_OPEN_MODE_READ			 0x00000001
#define DMG_FILE_OPEN_MODE_WRITE		 0x00000002
#define DMG_FILE_OPEN_MODE_UPDATE		 0x00000004
#define DMG_FILE_OPEN_MODE_CREATE		 0x00000008
#define DMG_FILE_OPEN_MODE_READWRITE	 0x00000010

#define DMG_FILE_SEEK_END				SEEK_END
#define DMG_FILE_SEEK_CUR				SEEK_CUR
#define DMG_FILE_SEEK_SET				SEEK_SET

#define DMG_FILE_ERR_FILE_EXIST			1
#define DMG_FILE_ERR_FILE_NOT_EXIST		2
#define DMG_FILE_ERR_MEM_OVERFLOW		3
#define DMG_FILE_ERR_TOO_MANY_FILES		4
#define DMG_FILE_ERR_INVALID_HANDLE		5
#define DMG_FILE_ERR_INVALID_MODE		6
#define DMG_FILE_ERR_NO_FILE_SYSTEM		7
#define DMG_FILE_ERR_FILE_NOT_OPENED	8
#define DMG_FILE_ERR_FILE_OPENED		9
#define DMG_FILE_ERR_END_OVER_FLOW		10
#define DMG_FILE_ERR_TOP_OVER_FLOW		11
#define DMG_FILE_ERR_NO_PERMISSION		12
#define DMG_FILE_ERR_FS_CORRUPT			13

#define DMG_FILE_MAX_FILE_COUNT			256

typedef struct
{
    BYTE	m_byFileId;
    BYTE	m_byFileAttr;
    BYTE	m_byFileType;
    char	m_szFileName[17];
    DWORD	m_dwFileLength;
}FilesInfoSt;

DWORD	qDmgFile_Rename(char* i_lpszOldFileName, char* i_lpszNewFileName);
DWORD	qDmgFile_ClearContent(char* i_lpszFileName);
DWORD	qDmgFile_GetTermFilesInfo(DWORD* o_dwFilesCount, FilesInfoSt* o_sFilesInfo);
DWORD	qDmgFile_RemoveFromBegin(HFILE i_hFileHandle,char *i_lpszFileName,DWORD i_dwFileNewSize);
#else

#include <DeviceMng/dmgFile.h>
#endif

DWORD	qDmgFile_Open(char *i_lpszFileName, BYTE i_byOpenMode, HFILE* o_hFileHandle);
DWORD	qDmgFile_Close(HFILE i_hFileHandle);
DWORD	qDmgFile_Write(HFILE i_hFileHandle, BYTE* i_pbyBufferToWrite, WORD i_dwBufferLen);
DWORD	qDmgFile_Read(HFILE i_hFileHandle, BYTE* o_pbyReadBuffer, WORD* io_pwBufferLen);
DWORD	qDmgFile_GetSize(char* i_lpszFileName);
DWORD	qDmgFile_Seek(HFILE i_hFileHandle, DWORD i_dwPointerOffset, DWORD i_dwStartPosition);
DWORD	qDmgFile_Truncate(HFILE i_hFileHandle, DWORD i_dwFileNewSize);
DWORD	qDmgFile_Remove(char* i_lpszFileName);



#endif //PAYMENT_QCARDFILEOPERATIONWRAPPER_H
