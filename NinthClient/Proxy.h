#include "stdafx.h"
#pragma once
#include<string>
#include "Base64.h"
class CProxy
{
public:
	CProxy(void);
	~CProxy(void);
public:
// 	string m_proxyIp;
// 	u_short m_proxyPort;
// 	string m_proxyUserName;
// 	string m_proxyUserPwd;
	bool GetHttpProxy(std::wstring &ip,bool &bProxyEnable);
	bool Connect(SOCKET m_socket,string lpszHost, u_short nPort=0);
	bool ConnectProxyServer(SOCKET socket,string serverIP,u_short serverPort);
	bool ConnectHttp(SOCKET socket,string ip, u_short port,string m_proxyUserName,string m_proxyUserPwd);
	bool Send(SOCKET socket, const char* buf, int len);
	int  Receive(SOCKET socket, char* buf, int bufLen);
};

