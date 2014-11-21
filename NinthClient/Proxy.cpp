#include "StdAfx.h"
#include "Proxy.h"


CProxy::CProxy(void)
{
}


CProxy::~CProxy(void)
{
}
bool CProxy::GetHttpProxy(std::wstring &ip,bool &bProxyEnable)
{
	HKEY hKeyIn = HKEY_CURRENT_USER, hKeyOut;
	if(RegOpenKeyEx(hKeyIn, 
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), 
		0, 
		KEY_CREATE_LINK | KEY_WRITE | KEY_READ | KEY_NOTIFY, 
		&hKeyOut) != ERROR_SUCCESS )
	{
		bProxyEnable = FALSE;
		return false;
	}

	//检测是否已经允许使用代理服务器

	ULONG regsize = 0;
	if(RegQueryValueEx(hKeyOut, 
		_T("ProxyEnable"), 
		NULL, 
		NULL, 
		NULL, 
		&regsize)!=ERROR_SUCCESS)
	{
		bProxyEnable = FALSE;
		return FALSE;
	}

	LPBYTE pValue = new BYTE[regsize];
	memset(pValue, 0x00, regsize);

	if(RegQueryValueEx(hKeyOut, 
		_T("ProxyEnable"), 
		NULL, 
		NULL, 
		pValue, 
		&regsize)!=ERROR_SUCCESS)
	{
		bProxyEnable = FALSE;
		return FALSE;
	}

	bProxyEnable=(int)*pValue;
	delete[] pValue;
	pValue = NULL;


	/************************************************************/
	if(bProxyEnable)
	{
		regsize = 0;
		if(RegQueryValueEx(hKeyOut, _T("ProxyServer"), NULL, NULL, NULL, &regsize)!=ERROR_SUCCESS)
		{
			bProxyEnable = FALSE;
			return FALSE;
		}

		pValue = new BYTE[regsize];
		memset(pValue, 0x00, regsize);

		if(RegQueryValueEx(hKeyOut, _T("ProxyServer"), NULL, NULL, pValue, &regsize)!=ERROR_SUCCESS)
		{
			bProxyEnable = FALSE;
			return FALSE;
		}

		ip = (wchar_t*) pValue;
		delete[] pValue;
		pValue = NULL;
	}
	else
		ip=_T("");

	RegCloseKey(hKeyOut);

	return TRUE;
}

bool CProxy::Connect(SOCKET m_socket,string lpszHost, u_short nPort)
{
	sockaddr_in	ClientAddr;
	//nPort = 9527;
	ClientAddr.sin_family	= AF_INET;
	ClientAddr.sin_port	= htons(nPort);
	ClientAddr.sin_addr.s_addr = inet_addr(lpszHost.c_str());
	//connect(m_socket, (SOCKADDR *)&ClientAddr, sizeof(ClientAddr));
	if (connect(m_socket, (SOCKADDR *)&ClientAddr, sizeof(ClientAddr)) == SOCKET_ERROR)   
		return false;
	return true;
}
bool CProxy::ConnectProxyServer(SOCKET socket,string serverIP,u_short serverPort)
{
	int ret;
	struct timeval timeout ;
	fd_set r;

	sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.S_un.S_addr = inet_addr(serverIP.c_str());
	servAddr.sin_port = htons(serverPort);

	//设置非阻塞方式连接
	unsigned long ul = 1;
	ret = ioctlsocket(socket, FIONBIO, (unsigned long*)&ul);
	if(ret == SOCKET_ERROR) 
	{
		return false;
	}

	connect(socket, (sockaddr*)&servAddr, sizeof(sockaddr));

	FD_ZERO(&r);
	FD_SET(socket, &r);
	timeout.tv_sec = 5; 
	timeout.tv_usec =0;
	ret = select(0, 0, &r, 0, &timeout);

	if (ret <= 0)
	{
		
		return false;
	}
	else
	{
		return true;
	}
}


bool CProxy::ConnectHttp(SOCKET socket,string ip, u_short port,
	string m_proxyUserName,string m_proxyUserPwd)
{
	int nTimeout = 5000;
	setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&nTimeout, sizeof(int));	//设置接收超时

	unsigned long ul = 0;
	int ret = ioctlsocket(socket, FIONBIO, (unsigned long*)&ul);	//设置阻塞方式连接

	char buf[512];

	if (m_proxyUserName != "")
	{
		string str;
		string strBase64;
		str = m_proxyUserName + ":" + m_proxyUserPwd;
		strBase64 = CBase64::Encode((unsigned char*)str.c_str(), str.length());
		sprintf_s(buf, sizeof(buf), "CONNECT %s:%d HTTP/1.1\r\nHost: %s:%d\r\nAuthorization: Basic %s\r\n\r\nProxy-Authorization: Basic %s\r\n\r\n", 
			ip.c_str(), port, ip.c_str(), port, strBase64.c_str(), strBase64.c_str());
	}
	else
	{
		//sprintf_s(buf, 512, "CONNECT %s:%d HTTP/1.1\r\nHost: %s:%d\r\n\r\n", ip.c_str(), port, ip.c_str(), port);
		sprintf_s(buf, sizeof(buf), "CONNECT %s:%d HTTP/1.1\r\nUser-Agent: MyApp/0.1\r\n\r\n", ip.c_str(), port);
	}

	Send(socket, buf, strlen(buf));
	Receive(socket, buf, sizeof(buf));

	if (strstr(buf, "HTTP/1.0 200 Connection established") != NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int CProxy::Receive(SOCKET socket, char* buf, int bufLen)
{
	return recv(socket, buf, bufLen, 0);
}

bool CProxy::Send(SOCKET socket, const char* buf, int len)
{
	long ilen = len;
	int sendCnt = 0;
	int ret;

	while(sendCnt < ilen)
	{
		if((ret = send(socket, buf + sendCnt, ilen - sendCnt, 0)) == SOCKET_ERROR)
		{
			return false;
		}
		else
		{
			sendCnt += ret;
		}
	}
	return true;
}
