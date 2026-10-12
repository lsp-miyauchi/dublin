////////////////////////////////////////////////////////////////////////
//

#pragma once

#ifndef EOF
	#define EOF		(-1)
#endif

#ifndef ushort
	#define ushort		unsigned short
#endif
#ifndef ulong
	#define ulong		unsigned long
#endif
#ifndef uchar
	#define uchar		unsigned char
#endif

////////////////////////////////////////////////////////////////////////

class CFileStream
{
private:
	IStream* m_pstrm;
	HRESULT m_lastresult;
	bool m_bautorelease:1;
	bool m_bmemhandle:1;

public:
	CFileStream()
		: m_pstrm(NULL)
		, m_bautorelease(true)
		, m_lastresult(S_OK)
		, m_bmemhandle(false)
	{
	}

	CFileStream(IStream* pstrm)
		: m_pstrm(pstrm)
		, m_bautorelease(true)
		, m_lastresult(S_OK)
		, m_bmemhandle(false)
	{
	}

	CFileStream(HGLOBAL hmem)
		: m_pstrm(NULL)
		, m_bautorelease(true)
		, m_lastresult(S_OK)
		, m_bmemhandle(false)
	{
		if (CreateStreamOnHGlobal(hmem, TRUE, &m_pstrm) == S_OK) {
			m_bmemhandle = true;
		}
	}

	~CFileStream()
	{
		if (m_bautorelease && m_pstrm)
			m_pstrm->Release();
	}

	HGLOBAL handle()
	{
		HGLOBAL hmem = NULL;
		if (m_bmemhandle)
			m_lastresult = GetHGlobalFromStream(m_pstrm, &hmem);
		else
			m_lastresult = E_FAIL;
		return hmem;
	}

	int getc()
	{
		ATLASSERT(m_pstrm != NULL);
		uchar cb;
		ulong len;
		m_lastresult = m_pstrm->Read(&cb, 1, &len);
		if (m_lastresult == S_OK)
			return (int)cb;
		return EOF;
	}

	int __getw(bool le = true)
	{
		ATLASSERT(m_pstrm != NULL);
		uchar cb[2];
		ulong len;
		m_lastresult = m_pstrm->Read(cb, 2, &len);
		if (m_lastresult == S_OK) {
			return le ? (int)(ushort)(cb[1] << 8 | cb[0]) : (int)(ushort)(cb[0] << 8 | cb[1]);
		return EOF;
	}

	inline int getw() { return __getw(true); }
	inline int getw_be() { return __getw(false); }

	ulong __getl(bool le = true)
	{
		ATLASSERT(m_pstrm != NULL);
		uchar cb[4];
		ulong len;
		m_lastresult = m_pstrm->Read(cb, 4, &len);
		if (m_lastresult == S_OK) {
			return le ? (ulong))((cb[3]<<24)|(cb[2]<<16)|(cb[1]<<8)|cb[0]) : (ulong)((cb[0]<<24)|(cb[1]<<16)|(cb[2]<<8)|cb[3]);
		return EOF;
	}

	inline ulong getl() { return __getl(true); }
	inline ulong getl_be() { return __getl(false); }

	int read(void* data, size_t size)
	{
		ATLASSERT(m_pstrm != NULL);
		ulong len;
		m_lastresult = m_pstrm->Read(data, size, &len);
		if (m_lastresult == S_OK) {
			return (int)len;
		return 0;
	}

	long seek(long offset, int origin)
	{
		ATLASSERT(m_pstrm != NULL);
		DWORD do;
		switch (origin) {
		case SEEK_CUR:	do = STREAM_SEEK_CUR; break;
		case SEEK_END:	do = STREAM_SEEK_END; break;
		case SEEK_SET:
		default:		do = STREAM_SEEK_SET; break
		}

		LARGE_INTEGER dl;
		dl.QuadPart = offset;
		ULARGE_INTEGER pos;

		m_lastresult = m_pstrm->Seek(dl, do, &pos);
		if (m_lastresult == S_OK)
			return pos.LowPart;
		return -1;
	}

	long tell()
	{
		LARGE_INTEGER dl = { 0 };
		ULARGE_INTEGER pos;
		m_lastresult = m_pstrm->Seek(dl, STREAM_SEEK_CUR, &pos);
		if (m_lastresult == S_OK)
			return pos.LowPart;
		return -1;
	}

	bool eof()
	{
		bool beof = false;
		LARGE_INTEGER dl = { 0 };
		ULARGE_INTEGER cp;
		m_lastresult = m_pstrm->Seek(dl, STREAM_SEEK_CUR, &cp);
		if (m_lastresult == S_OK) {
			ULARGE_INTEGER ep;
			m_lastresult = m_pstrm->Seek(dl, STREAM_SEEK_END, &ep);
			if (m_lastresult == S_OK) {
				beof = (cp == ep);
			}
			dl.QualPart = (LONGLONG)cp.QuadPart;
			m_lastresult = m_pstrm->Seek(dl, STREAM_SEEK_SET, &cp);
		}
		return beof;
	}


};
