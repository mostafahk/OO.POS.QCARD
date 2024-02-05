//
// Created by f.karamiyar on 7/31/2019.
//

#include "define.h"
#include "qCardFileOperationWrapper.h"



DWORD qDmgFile_Open(char *i_lpszFileName, BYTE i_byOpenMode, HFILE* o_hFileHandle)
{
#ifdef T610_DEV
    int		iRetVal	=	0;
    BYTE	byMode	=	0;

    (*o_hFileHandle) = 0;

    switch(i_byOpenMode)
    {
        case DMG_FILE_OPEN_MODE_WRITE:
        case DMG_FILE_OPEN_MODE_READ:
            byMode = O_RDWR;
            break;
        case DMG_FILE_OPEN_MODE_UPDATE:
            byMode = O_RDWR | O_CREATE;
            break;
        case DMG_FILE_OPEN_MODE_CREATE:
            if(fexist(i_lpszFileName) != -1)
            {
                char	szFilename[50]	=	{0};
                strcpy(szFilename, i_lpszFileName);
                remove(szFilename);
            }
            byMode = O_CREATE;
            break;
    }

    iRetVal = open(i_lpszFileName, byMode);
    if(iRetVal == -1)
    {
        return GetLastError();
    }

    (*o_hFileHandle) = ++iRetVal;

    if(i_byOpenMode == DMG_FILE_OPEN_MODE_UPDATE)
        qDmgFile_Seek(iRetVal, 0, DMG_FILE_SEEK_END);
    else
        qDmgFile_Seek(iRetVal, 0, DMG_FILE_SEEK_SET);

    return TRUE;
#else
    return dmgFile_Open(i_lpszFileName,i_byOpenMode,o_hFileHandle);
#endif

}

DWORD qDmgFile_Close(HFILE i_hFileHandle)
{
#ifdef T610_DEV
    if(close(i_hFileHandle - 1) == 0)
        return TRUE;

    return FALSE;//GetLastError();
#else
    return dmgFile_Close(i_hFileHandle);
#endif
}

DWORD qDmgFile_Write(HFILE i_hFileHandle, BYTE* i_pbyBufferToWrite, WORD i_dwBufferLen)
{
#ifdef T610_DEV
    int	iRetVal	=	0;

    iRetVal = write(i_hFileHandle - 1, i_pbyBufferToWrite, (int)i_dwBufferLen);
    if(iRetVal == (int)i_dwBufferLen)
        return TRUE;

    return FALSE;//GetLastError();
#else
    return dmgFile_Write(i_hFileHandle, i_pbyBufferToWrite,i_dwBufferLen);
#endif
}

DWORD qDmgFile_Read(HFILE i_hFileHandle, BYTE* o_pbyReadBuffer, WORD* io_pwBufferLen)
{
#ifdef T610_DEV
    int	iLen	=	(*io_pwBufferLen);
    int	iRetVal	=	0;

    iRetVal = read(i_hFileHandle - 1, o_pbyReadBuffer, iLen);
    if(iRetVal < 0)
        return FALSE;//GetLastError();

    (*io_pwBufferLen) = (WORD)iRetVal;

    return TRUE;
#else
    return dmgFile_Read( i_hFileHandle,o_pbyReadBuffer,io_pwBufferLen);
#endif
}

DWORD qDmgFile_GetSize(char* i_lpszFileName)
{
#ifdef T610_DEV
    long	lSize	=	filesize(i_lpszFileName);

    if(lSize == -1)
        lSize = 0;

    return (DWORD)lSize;
#else
    return dmgFile_GetSize( i_lpszFileName);
#endif
}

DWORD qDmgFile_Seek(HFILE i_hFileHandle, DWORD i_dwPointerOffset, DWORD i_dwStartPosition)
{
#ifdef T610_DEV
    int	iRetVal	=	0;

    iRetVal = seek(i_hFileHandle - 1, (long)i_dwPointerOffset, (BYTE)i_dwStartPosition);
    if(iRetVal != 0)
        return FALSE;//GetLastError();

    return TRUE;
#else
   return  dmgFile_Seek( i_hFileHandle,i_dwPointerOffset,i_dwStartPosition);
#endif
}

DWORD qDmgFile_Truncate(HFILE i_hFileHandle, DWORD i_dwFileNewSize)
{
#ifdef T610_DEV
    int	iRetVal	=	0;

    iRetVal = truncate(i_hFileHandle - 1, i_dwFileNewSize);
    if(iRetVal != 0)
        return FALSE;//GetLastError();

    return TRUE;
#else
    return dmgFile_Truncate(i_hFileHandle, i_dwFileNewSize);
#endif
}

DWORD qDmgFile_Remove(char* i_lpszFileName)
{
#ifdef T610_DEV
    int	iRetVal	=	0;

    iRetVal = remove(i_lpszFileName);
    if(iRetVal != 0)
        return FALSE;//GetLastError();

    return TRUE;
#else
    return dmgFile_Remove(i_lpszFileName);
#endif
}




DWORD qDmgFile_Rename(char* i_lpszOldFileName, char* i_lpszNewFileName)
{
#ifdef T610_DEV
    HFILE	hFile		=	0;
    HFILE	hTempFile	=	0;
    BYTE*	pFileData;
    WORD	wLen		=	0;
    DWORD	dwSegment	=	512;
    DWORD	dwFileLen	=	qDmgFile_GetSize(i_lpszOldFileName);
    DWORD	i			=	0;
    int		iRetVal		=	0;

    if(!dwFileLen)
        return TRUE;

    if(!qDmgFile_Open(i_lpszOldFileName, DMG_FILE_OPEN_MODE_READ, &hFile)) {
        qDmgFile_Close(hFile);
        return FALSE;
    }

    if(!qDmgFile_Open(i_lpszNewFileName, DMG_FILE_OPEN_MODE_CREATE, &hTempFile))
    {
        qDmgFile_Close(hFile);
        return FALSE;
    }

    pFileData = (BYTE*)malloc(dwSegment);

    wLen = (WORD)dwSegment;

    while(i < dwFileLen)
    {
        qDmgFile_Seek(hFile, i, DMG_FILE_SEEK_SET);
        if(!qDmgFile_Read(hFile, pFileData, &wLen))
        {
            free(pFileData);
            qDmgFile_Close(hFile);
            qDmgFile_Close(hTempFile);
            qDmgFile_Remove(i_lpszNewFileName);
            return FALSE;
        }


        if(!qDmgFile_Write(hTempFile, pFileData, wLen))
        {
            free(pFileData);
            qDmgFile_Close(hFile);
            qDmgFile_Close(hTempFile);
            qDmgFile_Remove(i_lpszNewFileName);
            return FALSE;
        }

        i += wLen;
    }

    free(pFileData);
    qDmgFile_Close(hFile);
    qDmgFile_Close(hTempFile);
    qDmgFile_Remove(i_lpszOldFileName);

    return TRUE;
#else
  return  dmgFile_Rename(i_lpszOldFileName, i_lpszNewFileName);
#endif

}

#ifdef T610_DEV
DWORD qDmgFile_ClearContent(char* i_lpszFileName)
{
    HFILE	hFile	=	0;
    int		iRetVal	=	0;

    if(!qDmgFile_Open(i_lpszFileName, DMG_FILE_OPEN_MODE_CREATE, &hFile))
    {
        return FALSE;
    }

    qDmgFile_Close(hFile);

    return TRUE;
}



DWORD qDmgFile_GetTermFilesInfo(DWORD* o_dwFilesCount, FilesInfoSt* o_sFilesInfo)
{

    int			iRetVal		=	0;
    DWORD		dwCounter	=	0;
    FILE_INFO	sFileInfo[DMG_FILE_MAX_FILE_COUNT];

    memset(sFileInfo, 0, sizeof(FILE_INFO) * DMG_FILE_MAX_FILE_COUNT);

    (*o_dwFilesCount) = 0;

    iRetVal = GetFileInfo(sFileInfo);
    if(iRetVal != 0)
        return GetLastError();

    (*o_dwFilesCount) = iRetVal;

    for(dwCounter = 0; dwCounter < (*o_dwFilesCount); dwCounter++)
    {
        o_sFilesInfo[dwCounter].m_byFileId = sFileInfo->fid;
        o_sFilesInfo[dwCounter].m_byFileAttr = sFileInfo->attr;
        o_sFilesInfo[dwCounter].m_byFileType = sFileInfo->type;
        strcpy(o_sFilesInfo[dwCounter].m_szFileName, sFileInfo->name);
        o_sFilesInfo[dwCounter].m_dwFileLength = sFileInfo->length;
    }

    return TRUE;

}


DWORD qDmgFile_RemoveFromBegin(HFILE i_hFileHandle,char *i_lpszFileName,DWORD i_dwFileNewSize)
{
    int	iRetVal	=	0;
    int	iLen	=	0;
    BYTE*	o_pbyReadBuffer;


    long	lSize	=	filesize(i_lpszFileName);

    o_pbyReadBuffer = (BYTE*)malloc(lSize + 1);

    qDmgFile_Seek(i_hFileHandle, 0, DMG_FILE_SEEK_SET);
    iRetVal = read(i_hFileHandle - 1, o_pbyReadBuffer, lSize);

    if(iRetVal < 0)
    {
        free(o_pbyReadBuffer);
        return FALSE;//GetLastError();
    }

    if(i_dwFileNewSize > 0)
    {

        qDmgFile_Open(i_lpszFileName,DMG_FILE_OPEN_MODE_UPDATE,&i_hFileHandle);
        qDmgFile_Seek(i_hFileHandle, 0, DMG_FILE_SEEK_SET);
        iRetVal = write(i_hFileHandle - 1,(BYTE*)(o_pbyReadBuffer + lSize - i_dwFileNewSize), (int)i_dwFileNewSize);

        if(iRetVal != (int)i_dwFileNewSize)
        {
            free(o_pbyReadBuffer);
            return FALSE;//GetLastError();
        }
    }
    truncate(i_hFileHandle - 1, i_dwFileNewSize);
    free(o_pbyReadBuffer);
    return TRUE;


}
#endif
