#include "stdafx.h"
#include "BaseSet.h"

int CBaseSet::SetRun()
{
	HKEY   hKey; 
	wchar_t pFileName[MAX_PATH] = {0}; 
	//得到程序自身的全路径 
	DWORD dwRet = GetModuleFileNameW(NULL, (LPWCH)pFileName, MAX_PATH); 
	//找到系统的启动项 
	LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"); 
	//打开启动项Key 
	long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE, &hKey); 
	if(lRet== ERROR_SUCCESS)
	{
		//添加注册
		RegSetValueEx(hKey, _T("UilibDemo"), 0,REG_SZ,(const BYTE*)(LPCSTR)pFileName, MAX_PATH);
		RegCloseKey(hKey); 
	}


	return 0;
}

int CBaseSet::DeleteRun()
{
	HKEY   hKey; 
	wchar_t	pFileName[MAX_PATH] = {0}; 
	//得到程序自身的全路径 
	DWORD dwRet = GetModuleFileNameW(NULL, (LPWCH)pFileName, MAX_PATH); 
	//找到系统的启动项 
	LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"); 
	//打开启动项Key 
	long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpRun, 0, KEY_WRITE, &hKey); 
	if(lRet== ERROR_SUCCESS)
	{
		//删除注册
		RegDeleteValue(hKey,_T("UilibDemo"));
		RegCloseKey(hKey); 
	}

	return 0;
}