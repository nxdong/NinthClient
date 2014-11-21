// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <winsock2.h>

#include <Windows.h>
//#include <afx.h>
#include <TlHelp32.h>

typedef	struct _HEADER_DATA
{
	int				ID;
	int				flag;
	int				datalength;
	_HEADER_DATA(){
		ID = NULL;
		flag = NULL;
		datalength = 0;
	};
}HEADER;
typedef struct _DATA_INFO
{
	HEADER	header;
}DATA;
//	LOGINDATA
typedef struct _LOGIN_DATA
{
	HEADER			header;
	OSVERSIONINFOEX	OsVerInfoEx;	// version
	IN_ADDR			IPAddress;		// 存储32位的IPv4的地址数据结构
	wchar_t			HostName[50];	// HostName
	wchar_t	        UserName[50];	// UserName
	int             Privilege;
	//LPUSER_INFO_2    aaa;
}LOGIN;
typedef struct _FILE_LIST_DATA
{
	HEADER header;
	WIN32_FIND_DATA findData[10];

	_FILE_LIST_DATA(){
		memset(findData,0,sizeof(findData));
	};
	void Clear(){
		memset(findData,0,sizeof(findData));
	};
}FILE_LIST_DATA;
typedef struct	_FILE_SIZE
{
	DWORD	dwSizeHigh;
	DWORD	dwSizeLow;
}FILESIZE;
typedef struct _PROCESS_INFO
{
	PROCESSENTRY32	pe;
	wchar_t			path[MAX_PATH];
	wchar_t			username[MAX_PATH];
};
typedef struct _PROCESS_LIST_SEND_DATA
{
	HEADER	header;
	_PROCESS_INFO	processdata[5];
	_PROCESS_LIST_SEND_DATA()
	{
		memset(processdata,0,sizeof(processdata));
	}
	void Clear()
	{
		memset(processdata,0,sizeof(processdata));
	}
}PROCESSDATA,*PPROCESSDATA;


//  协议标志
enum {
	CLIENT_LOGIN	=	20,
	CLIENT_SHELL_START,
	CLIENT_FILE_START,
	CLIENT_SHELL_DATA,
	CLIENT_FILE_DRIVE_LIST,
	CLIENT_FILE_LIST,
	CLIENT_CREATE_FOLDER_FINISH,
	CLIENT_CREATE_FOLDER_FAILED,
	CLIENT_DLELE_FILE_FINISH,
	CLIENT_DLELE_FILE_FAILED,
	CLIENT_CREATE_FILE_FINISH,
	CLIENT_CREATE_FILE_FAILED,
	CLIENT_DELETE_FOLDER_FAILED,
	CLIENT_DELETE_FOLDER_FINISH,
	CLIENT_FILE_SIZE,
	CLIENT_PROCESS_START,
	CLIENT_PROCESS_LIST,
	CLIENT_TRANSFER_FINISH,
	CLIENT_FILE_DATA,

	SERVER_ACTIVED	=	500,
	SERVER_NEXT,
	SERVER_SHELLMANAGER,
	SERVER_FILEMANAGER,
	SERVER_FILE_LIST,
	SERVER_CREATE_FOLDER,
	SERVER_DELETE_FILE,
	SERVER_CREATE_FILE,
	SERVER_DELETE_FOLDER,
	SERVER_DOWNLOAD_FILE,
	SERVER_PROCESSMANAGER,
	SERVER_PROCESS_LIST,
	SERVER_FILE_STOP,
	SERVER_FILE_CONTINUE,
	SERVER_DOWN_FILES

};