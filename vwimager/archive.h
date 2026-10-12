#pragma once
#ifndef __TARCHIVE_H__
#define __TARCHIVE_H__


#define _ISFILEHANDLE(a)	((a) != NULL && (a) != INVALID_HANDLE_VALUE)

class TArchive
{
public:
	BYTE m_cbMarker[3];
	BOOL m_bWrite;
	HANDLE m_hFile;

	TArchive(HANDLE hFile, BOOL bWrite)
	{
		ATLASSERT(_ISFILEHANDLE(hFile));

		// initialize members not dependent on allocated buffer
		m_bWrite = bWrite;
		m_hFile = hFile;
		m_cbMarker[0] = 0xff;
		m_cbMarker[1] = 0xfe;
		m_cbMarker[2] = 0xff;
	}

	~TArchive()
	{
	}


// Attributes
	BOOL IsLoading() { return (m_bWrite == 0); }
	BOOL IsStoring() { return (m_bWrite != 0); }

// Operations
	UINT Read(void* lpBuf, UINT nMax)
	{
		if (nMax == 0) return 0;
		if(lpBuf == NULL) return 0;

		DWORD dw;
		if (ReadFile(m_hFile, lpBuf, nMax, &dw, NULL))
			return (UINT)dw;
		return 0;
	}

	void Write(const void* lpBuf, UINT nMax)
	{
		if (nMax == 0) return;
		if (lpBuf == NULL) return;

		DWORD dw;
		WriteFile(m_hFile, lpBuf, nMax, &dw, NULL);
	}

	BOOL ReadString(CString& str)
	{
		str = _T("");

		DWORD dw;
		BYTE marker[3];
		ReadFile(m_hFile, &marker, sizeof(marker), &dw, NULL);
		if (memcmp(marker, m_cbMarker, sizeof(marker)))
			return FALSE;

		BYTE bLength;
		WORD wLength;
		int lenStr;
		Read(&bLength, sizeof(BYTE));
		if (bLength < 0xff)
			lenStr = bLength;
		else {
			Read(&wLength, sizeof(WORD));
			lenStr = wLength;
		}

		if (lenStr > 0) {
			TCHAR* p = new TCHAR[lenStr + 1];
			Read(p, lenStr * sizeof(TCHAR));
			p[lenStr] = 0;
			str = p;
			delete [] p;
		}
		return TRUE;
	}

	BOOL WriteString(LPCTSTR str)
	{
		Write(m_cbMarker, sizeof(m_cbMarker));

		BYTE bLength;
		WORD wLength;
		int lenStr = lstrlen(str);
		if (lenStr < 0xff)
			bLength = (BYTE)lenStr;
		else
			bLength = 0xff;
		Write(&bLength, sizeof(BYTE));

		if (lenStr >= 0xff) {
			wLength = (WORD)lenStr;
			Write(&wLength, sizeof(WORD));
		}
		Write(str, lenStr * sizeof(TCHAR));
		return TRUE;
	}


public:
	// insertion operations
	TArchive& operator<<(int i);
	TArchive& operator<<(unsigned u);
	TArchive& operator<<(DWORD dw);
	TArchive& operator<<(LONG l)
	{
		LONG lw = l;
		Write(&lw, sizeof(lw));
		return *this;
	}

	TArchive& operator<<(short w);
	TArchive& operator<<(WORD w)
	{
		WORD ww = w;
		Write(&ww, sizeof(ww));
		return *this;
	}

	TArchive& operator<<(char ch);
	TArchive& operator<<(bool b);
	TArchive& operator<<(BYTE by)
	{
		BYTE bw = by;
		Write(&bw, sizeof(bw));
		return *this;
	}

	TArchive& operator<<(CString str);

	// extraction operations
	TArchive& operator>>(int& i);
	TArchive& operator>>(unsigned& u);
	TArchive& operator>>(DWORD& dw);
	TArchive& operator>>(LONG& l)
	{
		LONG lw;
		Read(&lw, sizeof(lw));
		l = lw;
		return *this;
	}

	TArchive& operator>>(char& ch);
	TArchive& operator>>(bool& b)
	{
		BYTE by;
		TArchive& ar = TArchive::operator>>(by);
		b = (by ? true : false);
		return ar;
	}
	TArchive& operator>>(BYTE& by)
	{
		BYTE bw;
		Read(&bw, sizeof(bw));
		by = bw;
		return *this;
	}

	TArchive& operator>>(short& w);
	TArchive& operator>>(WORD& w)
	{
		WORD ww;
		Read(&ww, sizeof(ww));
		w = ww;
		return *this;
	}

	TArchive& operator>>(CString& str);
};

// TArchive write
__inline TArchive& TArchive::operator<<(int i)
	{ return TArchive::operator<<((LONG)i); }
__inline TArchive& TArchive::operator<<(unsigned u)
	{ return TArchive::operator<<((LONG)u); }
__inline TArchive& TArchive::operator<<(DWORD dw)
	{ return TArchive::operator<<((LONG)dw); }
__inline TArchive& TArchive::operator<<(short w)
	{ return TArchive::operator<<((WORD)w); }
__inline TArchive& TArchive::operator<<(char ch)
	{ return TArchive::operator<<((BYTE)ch); }
__inline TArchive& TArchive::operator<<(bool b)
	{ return TArchive::operator <<((BYTE)(b ? 1 : 0)); }
__inline TArchive& TArchive::operator<<(CString str)
	{ WriteString(str); return *this; }

// TArchive read
__inline TArchive& TArchive::operator>>(int& i)
	{ return TArchive::operator>>((LONG&)i); }
__inline TArchive& TArchive::operator>>(unsigned& u)
	{ return TArchive::operator>>((LONG&)u); }
__inline TArchive& TArchive::operator>>(DWORD& dw)
	{ return TArchive::operator>>((LONG&)dw); }
__inline TArchive& TArchive::operator>>(short& w)
	{ return TArchive::operator>>((WORD&)w); }
__inline TArchive& TArchive::operator>>(char& ch)
	{ return TArchive::operator>>((BYTE&)ch); }
__inline TArchive& TArchive::operator>>(CString& str)
	{ ReadString(str); return *this; }


#endif			// __TARCHIVE_H__
