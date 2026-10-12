///////////////////////////////////////////////////////////////////////////////
// CFileHandle

#pragma once
#ifndef __FILEHANDLE_H__
#define __FILEHANDLE_H__

HANDLE __fastcall _createfile(LPCTSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);

template <bool t_bManaged>
class CFileHandleT
{
public:
// Data members
	HANDLE m_hFile;

// Constructor/destructor/operators
	CFileHandleT(HANDLE hFile = INVALID_HANDLE_VALUE) : m_hFile(hFile)
	{ }

	~CFileHandleT()
	{
		if (t_bManaged && m_hFile != INVALID_HANDLE_VALUE)
			CloseHandle();
	}

	CFileHandleT<t_bManaged>& operator =(HANDLE hFile)
	{
		Attach(hFile);
		return *this;
	}

	void Attach(HANDLE hFile)
	{
		if (t_bManaged && m_hFile != INVALID_HANDLE_VALUE && m_hFile != hFile)
			::CloseHandle(m_hFile);
		m_hFile = hFile;
	}

	HANDLE Detach()
	{
		HANDLE hFile = m_hFile;
		m_hFile = NULL;
		return hFile;
	}

	operator HANDLE() const { return m_hFile; }

	bool IsInvalid() const { return (m_hFile == INVALID_HANDLE_VALUE); }

// Create methods
	HANDLE CreateFile(LPCTSTR pName, DWORD fdwAccess, DWORD fdwShareMode, DWORD fdwCreate, DWORD fdwAttrsAndFlags, HANDLE hTemplateFile = NULL)
	{
		ATLASSERT(m_hFile == INVALID_HANDLE_VALUE);
		m_hFile = _createfile(pName, fdwAccess, fdwShareMode, NULL, fdwCreate, fdwAttrsAndFlags, hTemplateFile);
		return m_hFile;
	}

	BOOL CloseHandle()
	{
		ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);
		BOOL bRet = ::CloseHandle(m_hFile);
		if (bRet)
			m_hFile = NULL;
		return bRet;
	}

	BOOL ReadFile(void* pBuffer, DWORD nNumberOfBytesToRead, DWORD* pNumberOfBytesRead, OVERLAPPED* pOverlapped = NULL)
	{
		ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);
		return ::ReadFile(m_hFile, pBuffer, nNumberOfBytesToRead, pNumberOfBytesRead, pOverlapped);
	}

	BOOL WriteFile(void* pBuffer, DWORD nNumberOfBytesToWrite, DWORD* pNumberOfBytesToWrite, OVERLAPPED* pOverlapped = NULL)
	{
		ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);
		return ::WriteFile(m_hFile, pBuffer, nNumberOfBytesToWrite, pNumberOfBytesToWrite, pOverlapped);
	}

	DWORD GetFileSize(DWORD* pdwFileSizeHigh)
	{
		ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);
		return ::GetFileSize(m_hFile, pdwFileSizeHigh);
	}

	BOOL GetFileSizeEx(PLARGE_INTEGER pFileSize)
	{
		ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);
		return ::GetFileSizeEx(m_hFile, pFileSize);
	}

	DWORD GetFileType()
	{
		ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);
		return ::GetFileType(m_hFile);
	}

	BOOL GetFileInformationByHandle(BY_HANDLE_FILE_INFORMATION* pFileInformation)
	{
		ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);
		return ::GetFileInformationByHandle(m_hFile, pFileInformation);
	}

	BOOL GetFileTime(LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime)
	{
		ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);
		return ::GetFileTime(m_hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime);
	}

	BOOL SetFileTime(const FILETIME* lpCreationTime, const FILETIME* lpLastAccessTime, const FILETIME* lpLastWriteTime)
	{
		ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);
		return ::SetFileTime(m_hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime);
	}

	BOOL FlushFileBuffers()
	{
		ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);
		return ::FlushFileBuffers(m_hFile);
	}

	BOOL SetEndOfFile()
	{
		ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);
		return ::SetEndOfFile(m_hFile);
	}

	DWORD SetFilePointer(LONG lDistanceToMove, PLONG plDistanceToMoveHigh, DWORD dwMoveMethod)
	{
		ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);
		return ::SetFilePointer(m_hFile, lDistanceToMove, plDistanceToMoveHigh, dwMoveMethod);
	}

	BOOL SetFilePointerEx(LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER pNewFilePointer, DWORD dwMoveMethod)
	{
		ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);
		return ::SetFilePointerEx(m_hFile, liDistanceToMove, pNewFilePointer, dwMoveMethod);
	}

	BOOL SetFileValidData(LONGLONG ValidDataLength)
	{
		ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);
		return ::SetFileValidData(m_hFile, ValidDataLength);
	}

	BOOL SetFileShortName(LPCTSTR lpShortName)
	{
		ATLASSERT(m_hFile != INVALID_HANDLE_VALUE);
		return ::SetFileShortName(m_hFile, lpShortName);
	}

};

typedef CFileHandleT<false>   CFileHandleHandle;
typedef CFileHandleT<true>    CFileHandle;


#endif		// __FILEHANDLE_H__
