
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
		if (lpBuf == NULL) return 0;

		DWORD dw;
		if (ReadFile(m_hFile, lpBuf, nMax, &dw, NULL))
			return (UINT)dw;
		return 0;
	}

	DWORD Seek(LONG lDistanceToMove, DWORD dwMoveMethod, LONG* plDistanceToMoveHigh = NULL)
	{
		return SetFilePointer(m_hFile, lDistanceToMove, plDistanceToMoveHigh, dwMoveMethod);
	}

	BOOL Write(const void* lpBuf, UINT nMax)
	{
		if (nMax == 0) return TRUE;
		if (lpBuf == NULL) return FALSE;

		DWORD dw;
		return (WriteFile(m_hFile, lpBuf, nMax, &dw, NULL) && dw == nMax);
	}

	BOOL ReadString(CString& str)
	{
		str.Empty();

		BOOL ok = FALSE;
		BYTE marker[3];
		if (Read(&marker, sizeof(marker)) == sizeof(marker) && !memcmp(marker, m_cbMarker, sizeof(marker))) {
			BYTE bLength;
			WORD wLength;
			int lenStr;
			if (Read(&bLength, sizeof(BYTE)) == sizeof(BYTE)) {
				ok = TRUE;			// bLength が 0 の場合もあるので、いったんTRUEをセット
				if (bLength < 0xff)
					lenStr = bLength;
				else {
					Read(&wLength, sizeof(WORD));
					lenStr = wLength;
				}

				if (lenStr > 0) {
					TCHAR* p = str.GetBuffer(lenStr + 1);
					ok = (Read(p, lenStr * sizeof(TCHAR)) == lenStr * sizeof(TCHAR));
					p[lenStr] = 0;
					str.ReleaseBuffer();
				}
			}
		}
		return ok;
	}

	BOOL WriteString(LPCTSTR str)
	{
		BOOL ok = FALSE;
		int lenStr = lstrlen(str);
		BYTE* dataptr = new BYTE[sizeof(m_cbMarker) + 8 + lenStr * sizeof(TCHAR)];
		if (dataptr) {
			BYTE* pb = dataptr;
			memcpy(pb, m_cbMarker, sizeof(m_cbMarker));
			pb += sizeof(m_cbMarker);
			if (lenStr < 0xff)
				*pb++ = (BYTE)lenStr;
			else {
				*pb++ = 0xff;
				*(WORD*)pb = (WORD)lenStr;
				pb += sizeof(WORD);
			}
			memcpy(pb, str, lenStr * sizeof(TCHAR));
			pb += lenStr * sizeof(TCHAR);
			ok = Write(dataptr, (UINT)(pb - dataptr));
			delete [] dataptr;
		}
		return ok;
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
