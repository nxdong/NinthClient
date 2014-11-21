#include "stdafx.h"
#include "ProcessManager.h"

CProcessManager::CProcessManager(CClientSocket *pClient,UINT ID)
	:CManager(pClient)
{
	m_ID = ID;
	//hNtDll = LoadLibrary(_T("ntdll.dll"));
	//m_zwQuery = (ZwQueryInformationProcessP)GetProcAddress(hNtDll, "ZwQueryInformationProcess");
	NtQuerySystemInformationX = (lpfnNtQuerySystemInformation)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQuerySystemInformation");
	HEADER	SendCmd;
	SendCmd.flag = CLIENT_PROCESS_START;
	SendCmd.ID = m_ID;
	SendCmd.datalength = 0;

	Send((LPBYTE)&SendCmd, sizeof(HEADER));

	WaitForDialogOpen();
}
CProcessManager::~CProcessManager()
{

}
void CProcessManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
{
	HEADER *pHeader = (HEADER*)lpBuffer;

	switch(pHeader->flag)
	{
	case SERVER_NEXT:
		NotifyDialogIsOpen();
		SendProcessList();
		break;
	default:
		break;
	}
}
void CProcessManager::SendProcessList()
{
	HANDLE			hSnapshot = NULL;
	HANDLE			hProcess = NULL;
	HMODULE			hModules = NULL;
	PROCESSDATA		processdata;
	PROCESSENTRY32	pe32;
	DWORD			cbNeeded;
	wchar_t			strProcessName[MAX_PATH];
	wchar_t			strUserName[MAX_PATH];
	DebugPrivilege(SE_DEBUG_NAME, TRUE);
	//pfnRtlAdjustPrivilege(0x13,1,0,NULL);
	// use snapshot
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(hSnapshot == INVALID_HANDLE_VALUE)
		return ;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	processdata.header.ID = m_ID;
	processdata.header.flag = CLIENT_PROCESS_LIST;
	processdata.header.datalength = 5 * sizeof(_PROCESS_INFO);

	if(Process32First(hSnapshot, &pe32))
	{	  
		int countFlag = 0; //each 5 data send one packet;
		do
		{   
			memset(strProcessName,0,sizeof(strProcessName));
			
			//hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
			if ((pe32.th32ProcessID !=0 ) && (pe32.th32ProcessID != 4) && (pe32.th32ProcessID != 8))
			{
				//EnumProcessModules(hProcess, &hModules, sizeof(hModules), &cbNeeded);
				//GetModuleFileNameEx(hProcess, hModules, strProcessName, sizeof(strProcessName));
				PVOID buffer = malloc(0x100);
				ULONG retLong = 0;
				SYSTEM_PROCESS_IMAGE_NAME_INFORMATION info;
				info.ProcessId = (HANDLE)pe32.th32ProcessID;
				info.ImageName.Length = 0;
				info.ImageName.MaximumLength = (USHORT)0x100;
				info.ImageName.Buffer = (PWSTR)buffer;
				//m_zwQuery(hProcess,ProcessImageFileName,strProcessName,sizeof(strProcessName),&retLong);
				NTSTATUS status = NtQuerySystemInformationX(88,&info,sizeof(info),NULL);
				if (status == 0xC0000004L)
				{
					free(buffer);
					buffer = malloc(info.ImageName.MaximumLength);
					info.ImageName.Buffer = (PWSTR)buffer;
					status = NtQuerySystemInformationX(88, &info, sizeof(info), NULL);
				}
				if (NT_SUCCESS(status))
				{
			
					wcsncpy_s(strProcessName, MAX_PATH, info.ImageName.Buffer, info.ImageName.Length / 2);
				}

				free(buffer);
				//memcpy(strProcessName,strProcessName+4,sizeof(strProcessName)-4);
				
				wchar_t strNtPath[MAX_PATH] = {0};
				DosPathToNtPath(strProcessName,strNtPath);

				processdata.processdata[countFlag].pe = pe32;
				wcscpy(processdata.processdata[countFlag].path,strNtPath);
				GetProcessUserName(pe32.th32ProcessID,strUserName);
				memcpy(processdata.processdata[countFlag].username,strUserName,sizeof(processdata.processdata[countFlag].username));
			}

			if (countFlag == 4)
			{
				Send((LPBYTE)&processdata,sizeof(PROCESSDATA));
				processdata.Clear();
				countFlag = 0;
				continue;
			}
			countFlag++;
		}
		while(Process32Next(hSnapshot, &pe32));
	}
	Send((LPBYTE)&processdata,sizeof(PPROCESSDATA));

	DebugPrivilege(SE_DEBUG_NAME, FALSE); 
	CloseHandle(hSnapshot);
	return ;
}
bool CProcessManager::DebugPrivilege(const wchar_t *PName,BOOL bEnable)
{
	BOOL              bResult = TRUE;
	HANDLE            hToken;
	TOKEN_PRIVILEGES  TokenPrivileges;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		bResult = FALSE;
		return bResult;
	}
	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

	LookupPrivilegeValue(NULL, PName, &TokenPrivileges.Privileges[0].Luid);
	AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	if (GetLastError() != ERROR_SUCCESS)
	{
		bResult = FALSE;
	}

	CloseHandle(hToken);
	return bResult;	
}


//获取进程用户函数： 
void CProcessManager::GetProcessUserName(DWORD dwID,wchar_t* pusername) // 进程ID 
{ 
	HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,dwID); 
	if( hProcess==NULL ) 
		return ; 
	HANDLE hToken =NULL; 
	BOOL bResult =FALSE; 
	DWORD dwSize =0; 

	TCHAR szDomain[MAX_PATH]={0}; 
	DWORD dwDomainSize=MAX_PATH; 
	DWORD dwNameSize=MAX_PATH; 

	SID_NAME_USE    SNU; 
	PTOKEN_USER pTokenUser=NULL; 
	__try 
	{ 
		if( !OpenProcessToken(hProcess,TOKEN_QUERY,&hToken) ) 
		{ 
			bResult = FALSE; 
			__leave; 
		} 

		if( !GetTokenInformation(hToken,TokenUser,pTokenUser,dwSize,&dwSize) ) 
		{ 
			if( GetLastError() != ERROR_INSUFFICIENT_BUFFER ) 
			{ 
				bResult = FALSE ; 
				__leave; 
			} 
		} 

		pTokenUser = NULL; 
		pTokenUser = (PTOKEN_USER)malloc(dwSize); 
		if( pTokenUser == NULL ) 
		{ 
			bResult = FALSE; 
			__leave; 
		} 

		if( !GetTokenInformation(hToken,TokenUser,pTokenUser,dwSize,&dwSize) ) 
		{ 
			bResult = FALSE; 
			__leave; 
		} 

		if( LookupAccountSid(NULL,pTokenUser->User.Sid,pusername,&dwNameSize,szDomain,&dwDomainSize,&SNU) != 0 ) 
		{ 
			return ; 
		} 
	} 
	__finally 
	{ 
		if( pTokenUser!=NULL ) 
			free(pTokenUser); 
	} 

	return; 
} 


BOOL CProcessManager::DosPathToNtPath(LPTSTR pszDosPath, LPTSTR pszNtPath)  
{  
	TCHAR           szDriveStr[500];  
	TCHAR           szDrive[3];  
	TCHAR           szDevName[100];  
	INT             cchDevName;  
	INT             i;  

	//检查参数  
	if(!pszDosPath || !pszNtPath )  
		return FALSE;  

	//获取本地磁盘字符串  
	if(GetLogicalDriveStrings(sizeof(szDriveStr), szDriveStr))  
	{  
		for(i = 0; szDriveStr[i]; i += 4)  
		{  
			if(!lstrcmpi(&(szDriveStr[i]), _T("A:\\")) || !lstrcmpi(&(szDriveStr[i]), _T("B:\\")))  
				continue;  

			szDrive[0] = szDriveStr[i];  
			szDrive[1] = szDriveStr[i + 1];  
			szDrive[2] = '\0';  
			if(!QueryDosDevice(szDrive, szDevName, 100))//查询 Dos 设备名  
				return FALSE;  

			cchDevName = lstrlen(szDevName);  
			if(_tcsnicmp(pszDosPath, szDevName, cchDevName) == 0)//命中  
			{  
				lstrcpy(pszNtPath, szDrive);//复制驱动器  
				lstrcat(pszNtPath, pszDosPath + cchDevName);//复制路径  

				return TRUE;  
			}             
		}  
	}  

	lstrcpy(pszNtPath, pszDosPath);  

	return FALSE;  
}
