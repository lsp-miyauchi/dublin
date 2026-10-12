/////////////////////////////////////////////////////////////////////
// mapping_file.h

#ifndef __MAPPING_FILE_H
#define __MAPPING_FILE_H

#define MF_EOF		(-1)

class mapping_file
{
private:
	HANDLE m_map;
	BYTE* m_base;
	BYTE* m_ptr;
	BYTE* m_endptr;
	ULARGE_INTEGER m_size;
public:
	mapping_file()
	{
		m_map = NULL;
		m_base = NULL;
		m_ptr = NULL;
		m_endptr = NULL;
	}
	~mapping_file()
	{
		close();
	}
	BOOL open(LPCTSTR name)
	{
		HANDLE hf = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (hf != INVALID_HANDLE_VALUE) {
			m_size.LowPart = GetFileSize(hf, &m_size.HighPart);
			m_map = CreateFileMapping(hf, NULL, PAGE_READONLY, 0, 0, NULL);
			if (m_map) {
				m_base = (BYTE*)MapViewOfFile(m_map, FILE_MAP_READ, 0, 0, 0);
				if (m_base) {
					m_ptr = (BYTE*)m_base;
					m_endptr = m_base + m_size.QuadPart;
				}
				else {
					CloseHandle(m_map);
					m_map = NULL;
				}
			}
			CloseHandle(hf);
		}
		return (m_map != NULL);
	}
	void close()
	{
		if (m_map) {
			if (m_base) {
				UnmapViewOfFile(m_base);
				m_base = NULL;
			}
			CloseHandle(m_map);
			m_map = NULL;
		}
	}
	long getchar()
	{
		if (m_endptr > m_ptr + sizeof(TCHAR)) {
			TCHAR ch = *(TCHAR*)m_ptr;
			m_ptr += sizeof(TCHAR);
			return (long)ch;
		}
		return MF_EOF;
	}
	short getbyte()
	{
		if (m_endptr > m_ptr) {
			BYTE cb = *m_ptr;
			m_ptr++;
			return (short)cb;
		}
		return MF_EOF;
	}
	TCHAR* getstr(LPTSTR pStr, size_t cchMax)
	{
		long ch;
		size_t numread = 0;
		LPTSTR p = pStr;
		while (numread < cchMax) {
			ch = getchar();
			if (ch == MF_EOF)
				break;
			numread++;
			if (ch == '\r' || ch == '\n') {
				*p++ = '\n';
				if (ch == '\r') {
					ch = getchar();
					if (ch == MF_EOF)
						break;
					if (ch != '\n')
						seek(-1, FILE_CURRENT);
				}
				break;
			}
			*p++ = (TCHAR)ch;
		}
		if (numread > 0) {
			*p = '\0';
			return pStr;
		}
		return NULL;
	}
	int getstr(CString& str)
	{
		int maxsize = 256;
		LPTSTR p = str.GetBuffer(maxsize);
		*p = 0;
		int numread = 0;
		while (1) {
			if (numread >= maxsize - 1) {
				maxsize += 256;
				p = str.GetBuffer(maxsize) + numread;
			}
			long ch = getchar();
			if (ch == MF_EOF)
				break;
			if (ch == '\r' || ch == '\n') {
				*p++ = '\n';
				if (ch == '\r') {
					ch = getchar();
					if (ch == MF_EOF)
						break;
					if (ch != '\n')
						seek(-1, FILE_CURRENT);
				}
				break;
			}
			*p++ = (TCHAR)ch;
			numread++;
		}
		if (numread > 0)
			*p = 0;
		str.ReleaseBuffer();
		return numread;
	}
	int read(void* pbuf, int len)
	{
		__int64 r = min((__int64)len, m_endptr - m_ptr);
		if (r > 0) {
			memcpy(pbuf, m_ptr, (size_t)r);
			m_ptr += r;
			return (int)r;
		}
		return 0;
	}
	long seek(long pos, int mode)
	{
		if (mode == FILE_CURRENT)
			m_ptr = max(m_base, min(m_endptr, m_ptr + pos));
		else if (mode == FILE_BEGIN)
			m_ptr = min(m_endptr, m_base + pos);
		else if (mode == FILE_END)
			m_ptr = max(m_base, min(m_endptr, m_endptr + pos));
		return (long)(m_ptr - m_base);
	}
	BOOL eof()
	{
		return (m_endptr <= m_ptr);
	}
	inline BYTE* GetDataPtr() { return m_base; }
	inline DWORD GetSize() { return ((m_size.HighPart > 0) ? 0xffffffff : m_size.LowPart); }
	inline DWORDLONG GetSize64() { return m_size.QuadPart; }
};

#endif				// __MAPPING_FILE_H
