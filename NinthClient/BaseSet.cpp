#include "stdafx.h"
#include "BaseSet.h"

int CBaseSet::SetRun()
{
	HKEY   hKey; 
	wchar_t pFileName[MAX_PATH] = {0}; 
	//�õ����������ȫ·�� 
	DWORD dwRet = GetModuleFileNameW(NULL, (LPWCH)pFileName, MAX_PATH); 
	//�ҵ�ϵͳ�������� 
	LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"); 
	//��������Key 
	long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE, &hKey); 
	if(lRet== ERROR_SUCCESS)
	{
		//���ע��
		RegSetValueEx(hKey, _T("UilibDemo"), 0,REG_SZ,(const BYTE*)(LPCSTR)pFileName, MAX_PATH);
		RegCloseKey(hKey); 
	}


	return 0;
}

int CBaseSet::DeleteRun()
{
	HKEY   hKey; 
	wchar_t	pFileName[MAX_PATH] = {0}; 
	//�õ����������ȫ·�� 
	DWORD dwRet = GetModuleFileNameW(NULL, (LPWCH)pFileName, MAX_PATH); 
	//�ҵ�ϵͳ�������� 
	LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"); 
	//��������Key 
	long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE, &hKey); 
	if(lRet== ERROR_SUCCESS)
	{
		//ɾ��ע��
		RegDeleteValue(hKey,_T("UilibDemo"));
		RegCloseKey(hKey); 
	}

	return 0;
}