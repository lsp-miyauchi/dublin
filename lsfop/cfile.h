//////////////////////////////////////////////////////////////////////
// CFile

#pragma once
#ifndef __CFILE_H__
#define __CFILE_H__

//////////////////////////////////////////////////////////////////////
// CFile

class CFile
{
private:
	HANDLE m_hf;

public:
	__inline CFile() { m_hf = NULL; }
	__inline ~CFile() { Close(); }

	void Close()
	{
		if (m_hf) {
			CloseHandle(m_hf);
			m_hf = NULL;
		}
	}

	BOOL Create(LPCTSTR lpFile)
	{
		if (m_hf) Close();

		m_hf = CreateFile(lpFile, GENERIC_READ | GENERIC_WRITE, 0,
						NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
		if (m_hf != INVALID_HANDLE_VALUE) return TRUE;

		m_hf = NULL;
		return FALSE;
	}

	BOOL Open(LPCTSTR lpFile)
	{
		if (!m_hf) {
			m_hf = CreateFile(lpFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, NULL);
			if (m_hf != INVALID_HANDLE_VALUE)
				return TRUE;
			m_hf = NULL;
		}
		return FALSE;
	}

	BOOL Read(LPVOID lpBuffer, UINT cbRead)
	{
		if (m_hf) {
			DWORD dw;
			if (ReadFile(m_hf, lpBuffer, cbRead, &dw, NULL) && (UINT)dw == cbRead)
				return TRUE;
		}
		return FALSE;
	}

	BOOL Write(LPVOID lpWrite, UINT cbWrite)
	{
		if (m_hf) {
			DWORD dw;
			if (WriteFile(m_hf, lpWrite, cbWrite, &dw, NULL) && (UINT)dw == cbWrite)
				return TRUE;
		}
		return FALSE;
	}

	__inline long Seek(long lOff, int nFrom) { return (long)SetFilePointer(m_hf, lOff, NULL, nFrom); }

	long GetLength()
	{
		if (m_hf) {
			long cur = SetFilePointer(m_hf, 0, NULL, FILE_CURRENT);
			long ofs = SetFilePointer(m_hf, 0, NULL, FILE_END);
			SetFilePointer(m_hf, cur, NULL, FILE_BEGIN);
			return ofs;
		}
		return -1;
	}

	long GetReminder()
	{
		if (m_hf) {
			long cur = SetFilePointer(m_hf, 0, NULL, FILE_CURRENT);
			long ofs = SetFilePointer(m_hf, 0, NULL, FILE_END);
			SetFilePointer(m_hf, cur, NULL, FILE_BEGIN);
			return ofs - cur;
		}
		return -1;
	}

	inline BOOL Read_DWORD(DWORD& val) { return Read(&val, sizeof(DWORD)); }
	inline BOOL Write_DWORD(DWORD val) { return Write(&val, sizeof(DWORD)); }

	inline BOOL Read_GUID(GUID& val) { return Read(&val, sizeof(GUID)); }
	inline BOOL Write_GUID(GUID& val) { return Write(&val, sizeof(GUID)); }

	BOOL Read_bool(bool& val)
	{
		DWORD dw;
		if (!Read_DWORD(dw)) return FALSE;

		val = (dw != 0);
		return TRUE;
	}

	__inline BOOL Write_bool(bool val) { return Write_DWORD((DWORD)val); }
};

////////////////////////////////////////////////////////////////////////////
// Memory based file implementation

class CRamFile
{
protected:
	size_t m_nGrowBytes;
	size_t m_nPosition;
	size_t m_nBufferSize;
	size_t m_nFileSize;
	BYTE* m_lpBuffer;
	bool m_bAutoDelete;

public:
	enum SeekPosition { begin = 0x0, current = 0x1, end = 0x2 };

	CRamFile(UINT nGrowBytes = 1024)
		: m_nGrowBytes(nGrowBytes)
		, m_nPosition(0)
		, m_nBufferSize(0)
		, m_nFileSize(0)
		, m_lpBuffer(NULL)
		, m_bAutoDelete(true)
	{
	}

	CRamFile(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes = 0)
		: m_nGrowBytes(nGrowBytes)
		, m_nPosition(0)
		, m_nBufferSize(nBufferSize)
		, m_nFileSize(nGrowBytes == 0 ? nBufferSize : 0)
		, m_lpBuffer(lpBuffer)
		, m_bAutoDelete(false)
	{
	}

	virtual ~CRamFile()
	{
		// Close should have already been called, but we check anyway
		if (m_lpBuffer)
			Close();
		ATLASSERT(m_lpBuffer == NULL);

		m_nGrowBytes = 0;
		m_nPosition = 0;
		m_nBufferSize = 0;
		m_nFileSize = 0;
	}

	void Attach(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes = 0)
	{
		ATLASSERT(m_lpBuffer == NULL);

		m_nGrowBytes = nGrowBytes;
		m_nPosition = 0;
		m_nBufferSize = nBufferSize;
		m_nFileSize = nGrowBytes == 0 ? nBufferSize : 0;
		m_lpBuffer = lpBuffer;
		m_bAutoDelete = FALSE;
	}

	BYTE* Detach()
	{
		BYTE* lpBuffer = m_lpBuffer;
		m_lpBuffer = NULL;
		m_nFileSize = 0;
		m_nBufferSize = 0;
		m_nPosition = 0;

		return lpBuffer;
	}

// Advanced Overridables
protected:
	virtual BYTE* Alloc(size_t nBytes)
	{
		return (BYTE*)malloc(nBytes);
	}

	virtual BYTE* Realloc(BYTE* lpMem, size_t nBytes)
	{
		ATLASSERT(nBytes > 0);	// nBytes == 0 means free
		return (BYTE*)realloc(lpMem, nBytes);
	}

	virtual BYTE* Memcpy(BYTE* lpMemTarget, const BYTE* lpMemSource, size_t nBytes)
	{
		ATLASSERT(lpMemTarget != NULL);
		ATLASSERT(lpMemSource != NULL);
		Checked::memcpy_s(lpMemTarget, nBytes, lpMemSource, nBytes);
		return lpMemTarget;
	}

	virtual void Free(BYTE* lpMem)
	{
		ATLASSERT(lpMem != NULL);
		free(lpMem);
	}

	virtual bool GrowFile(size_t dwNewLen)
	{
		if (dwNewLen > m_nBufferSize) {
			// grow the buffer
			size_t dwNewBufferSize = m_nBufferSize;

			// watch out for buffers which cannot be grown!
			ATLASSERT(m_nGrowBytes != 0);
			if (m_nGrowBytes == 0)
				return false;

			// determine new buffer size
			while (dwNewBufferSize < dwNewLen)
				dwNewBufferSize += m_nGrowBytes;

			// allocate new buffer
			BYTE* lpNew;
			if (m_lpBuffer == NULL)
				lpNew = Alloc(dwNewBufferSize);
			else
				lpNew = Realloc(m_lpBuffer, dwNewBufferSize);

			if (lpNew == NULL)
				return false;

			m_lpBuffer = lpNew;
			m_nBufferSize = dwNewBufferSize;
		}
		return true;
	}

	virtual ULONGLONG GetPosition() const
	{
		return m_nPosition;
	}

public:
	virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom)
	{
		ATLASSERT(nFrom == begin || nFrom == end || nFrom == current);

		LONGLONG lNewPos = m_nPosition;
		if (nFrom == FILE_BEGIN)
			lNewPos = lOff;
		else if (nFrom == FILE_CURRENT)
			lNewPos += lOff;
		else if (nFrom == FILE_END) {
			if (lOff > 0)  {
				// これ以上先へ進められない
				return m_nPosition;
			}
			lNewPos = m_nFileSize + lOff;
		}
		else
			return m_nPosition;

		if (lNewPos < 0)
			lNewPos = 0;
		if (static_cast<DWORD>(lNewPos) > m_nFileSize)
			GrowFile((size_t)lNewPos);

		m_nPosition = (size_t)lNewPos;

		return m_nPosition;
	}

	virtual bool SetLength(ULONGLONG dwNewLen)
	{
#ifdef WIN32
		if (dwNewLen > ULONG_MAX)
			return false;
#endif  // WIN32
		if (dwNewLen > m_nBufferSize)
			GrowFile((size_t)dwNewLen);

		if (dwNewLen < m_nPosition)
			m_nPosition = (size_t)dwNewLen;

		m_nFileSize = (size_t)dwNewLen;
		return true;
	}

	virtual UINT Read(void* lpBuf, UINT nCount)
	{
		if (nCount == 0) return 0;

		ATLASSERT(lpBuf != NULL);

		if (lpBuf == NULL) {
			return 0;
		}

		ATLASSERT(IsBadReadPtr(lpBuf, nCount) == FALSE);
		if (m_nPosition > m_nFileSize) return 0;

		UINT nRead;
		if (m_nPosition + nCount > m_nFileSize || m_nPosition + nCount < m_nPosition)
			nRead = (UINT)(m_nFileSize - m_nPosition);
		else
			nRead = nCount;

		Memcpy((BYTE*)lpBuf, (BYTE*)m_lpBuffer + m_nPosition, nRead);
		m_nPosition += nRead;

		return nRead;
	}

	virtual UINT Write(const void* lpBuf, UINT nCount)
	{
		if (nCount == 0) return 0;

		ATLASSERT(lpBuf != NULL);
		ATLASSERT(IsBadWritePtr((void*)lpBuf, nCount) == FALSE);

		if (lpBuf == NULL) {
			return 0;
		}

		if (m_nPosition + nCount > m_nBufferSize)
			GrowFile(m_nPosition + nCount);

		Memcpy((BYTE*)m_lpBuffer + m_nPosition, (BYTE*)lpBuf, nCount);
		m_nPosition += nCount;

		if (m_nPosition > m_nFileSize)
			m_nFileSize = m_nPosition;
		return nCount;
	}

	virtual void Abort()
	{
		Close();
	}

	virtual void Flush() { }

	virtual void Close()
	{
		ATLASSERT((m_lpBuffer == NULL && m_nBufferSize == 0) || !m_bAutoDelete || IsBadWritePtr(m_lpBuffer, (UINT)m_nBufferSize) == FALSE);
		ATLASSERT(m_nFileSize <= m_nBufferSize);

		m_nGrowBytes = 0;
		m_nPosition = 0;
		m_nBufferSize = 0;
		m_nFileSize = 0;
		if (m_lpBuffer && m_bAutoDelete)
			Free(m_lpBuffer);
		m_lpBuffer = NULL;
	}

	virtual ULONGLONG GetLength() const
	{
		return m_nFileSize;
	}

	void Read_String(CString& str)
	{
		// ANSI で読み込んで UNICODE 変換
		str.Empty();
		BYTE len;
		Read(&len, sizeof(BYTE));
		if (len > 0) {
			if (len != 0xff) {			// for ANSI
				char cb[256];
				Read(cb, len);
				cb[len] = 0;

				str = cb;
			}
			else {						// for UNICODE
				WORD wlen;
				Read(&wlen, sizeof(WORD));
				if (wlen > 0) {
					WCHAR* pw = str.GetBuffer(wlen + 1);
					Read(pw, wlen * sizeof(WCHAR));
					pw[wlen] = 0;
					str.ReleaseBuffer();
				}
			}
		}
	}

	void Write_String(LPCWSTR pstr)
	{
		// UNICODEで書き出す
		BYTE len = 0xff;
		Write(&len, sizeof(BYTE));		// 0xff は UNICODEマーカー

		WORD wlen = lstrlenW(pstr);
		Write(&wlen, sizeof(WORD));
		if (wlen > 0)
			Write((void*)pstr, wlen * sizeof(WCHAR));
	}

	inline BOOL Read_DWORD(DWORD& val) { return Read(&val, sizeof(DWORD)); }
	inline BOOL Write_DWORD(DWORD val) { return Write(&val, sizeof(DWORD)); }

	inline BOOL Read_GUID(GUID& val) { return Read(&val, sizeof(GUID)); }
	inline BOOL Write_GUID(GUID& val) { return Write(&val, sizeof(GUID)); }

	BOOL Read_bool(bool& val)
	{
		DWORD dw;
		if (!Read_DWORD(dw)) return FALSE;

		val = (dw != 0);
		return TRUE;
	}
	inline BOOL Write_bool(bool val) { return Write_DWORD((DWORD)val); }

	void* GetData() const { return (void*)m_lpBuffer; }
};

#endif		// __CFILE_H__
