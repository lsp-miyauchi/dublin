#include "stdafx.h"

BOOL CFile::Create(LPCTSTR lpFile)
{
	if (m_hf)
		Close();

	m_hf = CreateFile(lpFile, GENERIC_READ | GENERIC_WRITE, 0,
						NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (m_hf != INVALID_HANDLE_VALUE)
		return TRUE;
	m_hf = NULL;
	return FALSE;
}

BOOL CFile::Open(LPCTSTR lpFile)
{
	if (!m_hf)
	{
		m_hf = CreateFile(lpFile, GENERIC_READ, FILE_SHARE_READ,
						NULL, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, NULL);
		if (m_hf != INVALID_HANDLE_VALUE)
			return TRUE;
		m_hf = NULL;
	}
	return FALSE;
}

BOOL CFile::Read(LPVOID lpBuffer, UINT cbRead)
{
	if (m_hf)
	{
		DWORD dw;
		if (ReadFile(m_hf, lpBuffer, cbRead, &dw, NULL) && (UINT)dw == cbRead)
			return TRUE;
	}
	return FALSE;
}

BOOL CFile::Write(LPVOID lpWrite, UINT cbWrite)
{
	if (m_hf)
	{
		DWORD dw;
		if (WriteFile(m_hf, lpWrite, cbWrite, &dw, NULL) && (UINT)dw == cbWrite)
			return TRUE;
	}
	return FALSE;
}

void CFile::Close()
{
	if (m_hf)
	{
		CloseHandle(m_hf);
		m_hf = NULL;
	}
}

long CFile::GetLength()
{
	if (m_hf)
	{
		long cur = SetFilePointer(m_hf, 0, NULL, FILE_CURRENT);
		long ofs = SetFilePointer(m_hf, 0, NULL, FILE_END);
		SetFilePointer(m_hf, cur, NULL, FILE_BEGIN);
		return ofs;
	}
	return -1;
}

long CFile::GetReminder()
{
	if (m_hf)
	{
		long cur = SetFilePointer(m_hf, 0, NULL, FILE_CURRENT);
		long ofs = SetFilePointer(m_hf, 0, NULL, FILE_END);
		SetFilePointer(m_hf, cur, NULL, FILE_BEGIN);
		return ofs - cur;
	}
	return -1;
}

long CFile::Seek(long lOff, int nFrom)
{
	return (long)SetFilePointer(m_hf, lOff, NULL, nFrom);
}
