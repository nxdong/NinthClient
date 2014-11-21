
// Buffer.cpp: implementation of the CBuffer class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Buffer.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBuffer::CBuffer()
{
	// Initial size
	m_nSize = 0;

	m_pPtr = m_pBase = NULL;
	InitializeCriticalSection(&m_cs);
}

CBuffer::~CBuffer()
{
	if (m_pBase)
		VirtualFree(m_pBase, 0, MEM_RELEASE);
	DeleteCriticalSection(&m_cs);
}


BOOL CBuffer::Write(PBYTE pData, UINT nSize)
{
	EnterCriticalSection(&m_cs);
	if (ReAllocateBuffer(nSize + GetBufferLen()) == -1)
	{
		LeaveCriticalSection(&m_cs);
		return false;
	}
	CopyMemory(m_pPtr,pData,nSize);
	// Advance Pointer
	m_pPtr+=nSize;
	LeaveCriticalSection(&m_cs);
	return nSize;
}

BOOL CBuffer::Insert(PBYTE pData, UINT nSize)
{
	EnterCriticalSection(&m_cs);
	if (ReAllocateBuffer(nSize + GetBufferLen()) == -1)
	{
		LeaveCriticalSection(&m_cs);
		return false;
	}
	MoveMemory(m_pBase+nSize,m_pBase,GetMemSize() - nSize);
	CopyMemory(m_pBase,pData,nSize);
	// Advance Pointer
	m_pPtr+=nSize;
	LeaveCriticalSection(&m_cs);
	return nSize;
}


UINT CBuffer::Read(PBYTE pData, UINT nSize)
{
	EnterCriticalSection(&m_cs);
	// Trying to byte off more than ya can chew - eh?
	if (nSize > GetMemSize())
	{
		LeaveCriticalSection(&m_cs);
		return 0;
	}
	// all that we have 
	if (nSize > GetBufferLen())
		nSize = GetBufferLen();

	if (nSize)
	{
		// Copy over required amount and its not up to us
		// to terminate the buffer - got that!!!
		CopyMemory(pData,m_pBase,nSize);

		// Slide the buffer back - like sinking the data
		MoveMemory(m_pBase,m_pBase+nSize,GetMemSize() - nSize);

		m_pPtr -= nSize;
	}

	DeAllocateBuffer(GetBufferLen());

	LeaveCriticalSection(&m_cs);
	return nSize;
}

UINT CBuffer::GetMemSize() 
{
	return m_nSize;
}

UINT CBuffer::GetBufferLen() 
{
	if (m_pBase == NULL)
		return 0;

	int nSize = 
		m_pPtr - m_pBase;
	return nSize;
}

UINT CBuffer::ReAllocateBuffer(UINT nRequestedSize)
{
	if (nRequestedSize < GetMemSize())
		return 0;

	// Allocate new size
	UINT nNewSize = (UINT) ceil(nRequestedSize / 1024.0) * 1024;

	// New Copy Data Over
	PBYTE pNewBuffer = (PBYTE) VirtualAlloc(NULL,nNewSize,MEM_COMMIT,PAGE_READWRITE);
	if (pNewBuffer == NULL)
		return -1;

	UINT nBufferLen = GetBufferLen();
	CopyMemory(pNewBuffer,m_pBase,nBufferLen);

	if (m_pBase)
		VirtualFree(m_pBase,0,MEM_RELEASE);

	// Hand over the pointer
	m_pBase = pNewBuffer;
	// Realign position pointer
	m_pPtr = m_pBase + nBufferLen;
	m_nSize = nNewSize;
	return m_nSize;
}

UINT CBuffer::DeAllocateBuffer(UINT nRequestedSize)
{
	if (nRequestedSize < GetBufferLen())
		return 0;

	// Allocate new size
	UINT nNewSize = (UINT) ceil(nRequestedSize / 1024.0) * 1024;

	if (nNewSize < GetMemSize())
		return 0;

	// New Copy Data Over
	PBYTE pNewBuffer = (PBYTE) VirtualAlloc(NULL,nNewSize,MEM_COMMIT,PAGE_READWRITE);

	UINT nBufferLen = GetBufferLen();
	CopyMemory(pNewBuffer,m_pBase,nBufferLen);

	VirtualFree(m_pBase,0,MEM_RELEASE);
	// Hand over the pointer
	m_pBase = pNewBuffer;
	// Realign position pointer
	m_pPtr = m_pBase + nBufferLen;
	m_nSize = nNewSize;
	return m_nSize;
}

int CBuffer::Scan(PBYTE pScan,UINT nPos)
{
	if (nPos > GetBufferLen() )
		return -1;

	PBYTE pStr = (PBYTE) strstr((char*)(m_pBase+nPos),(char*)pScan);

	int nOffset = 0;

	if (pStr)
		nOffset = (pStr - m_pBase) + strlen((char*)pScan);

	return nOffset;
}

void CBuffer::ClearBuffer()
{
	EnterCriticalSection(&m_cs);
	// Force the buffer to be empty
	m_pPtr = m_pBase;

	DeAllocateBuffer(1024);
	LeaveCriticalSection(&m_cs);
}


void CBuffer::Copy(CBuffer& buffer)
{
	int nReSize = buffer.GetMemSize();
	int nSize = buffer.GetBufferLen();
	ClearBuffer();
	if (ReAllocateBuffer(nReSize) == -1)
		return;

	m_pPtr = m_pBase + nSize;

	CopyMemory(m_pBase,buffer.GetBuffer(),buffer.GetBufferLen());
}

PBYTE CBuffer::GetBuffer(UINT nPos)
{
	return m_pBase+nPos;
}


UINT CBuffer::Delete(UINT nSize)
{
	// Trying to byte off more than ya can chew - eh?
	if (nSize > GetMemSize())
		return 0;

	// all that we have 
	if (nSize > GetBufferLen())
		nSize = GetBufferLen();


	if (nSize)
	{
		// Slide the buffer back - like sinking the data
		MoveMemory(m_pBase,m_pBase+nSize,GetMemSize() - nSize);

		m_pPtr -= nSize;
	}
	DeAllocateBuffer(GetBufferLen());
	return nSize;
}