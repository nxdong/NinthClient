// Buffer.h: interface for the CBuffer class.
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include "math.h"
//#include <windows.h>
class CBuffer  
{
	// Attributes
protected:
	PBYTE	m_pBase;
	PBYTE	m_pPtr;
	UINT	m_nSize;
	// Methods
protected:
	UINT ReAllocateBuffer(UINT nRequestedSize);
	UINT DeAllocateBuffer(UINT nRequestedSize);
	UINT GetMemSize();
public:
	void ClearBuffer();
	UINT Delete(UINT nSize);
	UINT Read(PBYTE pData, UINT nSize);
	BOOL Write(PBYTE pData, UINT nSize);
	UINT GetBufferLen();
	int Scan(PBYTE pScan,UINT nPos);
	BOOL Insert(PBYTE pData, UINT nSize);
	void Copy(CBuffer& buffer);	
	PBYTE GetBuffer(UINT nPos=0);
	CBuffer();
	virtual ~CBuffer();
private:
	CRITICAL_SECTION m_cs;
};
