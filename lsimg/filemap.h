/////////////////////////////////////////////////////////////////////
// filemap.h

#pragma once
#ifndef __FILEMAP_H__
#define __FILEMAP_H__


#define EOF_SHORT		((short)~0)
#define EOF_ULONG		((ULONG)~0)


class filemap
{
public:
	HANDLE m_hf;
	HANDLE m_map;
	BYTE* m_base;
	BYTE* m_ptr;
	LARGE_INTEGER m_size;

	filemap() : m_hf(INVALID_HANDLE_VALUE), m_map(NULL), m_base(NULL)
	{
	}

	~filemap()
	{
		close();
	}

	inline bool isopen()
	{
		return (m_base != NULL);
	}

	bool open(LPCTSTR name)
	{
		m_hf = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (m_hf != INVALID_HANDLE_VALUE) {
			GetFileSizeEx(m_hf, &m_size);
			m_map = CreateFileMapping(m_hf, NULL, PAGE_READONLY, 0, 0, NULL);
			if (m_map) {
				m_base = (BYTE*)MapViewOfFile(m_map, FILE_MAP_READ, 0, 0, 0); 
				if (m_base) {
					m_ptr = (BYTE*)m_base;
					return true;
				}
				CloseHandle(m_map);
				m_map = NULL;
			}
			CloseHandle(m_hf);
			m_hf = INVALID_HANDLE_VALUE;
		}
		return false;
	}

	bool open_mem(void* pbase, long size)
	{
		if (!pbase || size < 1) return false;

		m_base = (BYTE*)pbase;
		m_ptr = m_base;
		m_size.QuadPart = size;
		return true;
	}

	void close()
	{
		if (m_hf != INVALID_HANDLE_VALUE) {
			if (m_map) 	{
				if (m_base) {
					UnmapViewOfFile(m_base);
					m_base = NULL;
				}
				CloseHandle(m_map);
				m_map = NULL;
			}
			CloseHandle(m_hf);
			m_hf = INVALID_HANDLE_VALUE;
		}
	}

	inline bool eof()
	{
		return (m_base + m_size.QuadPart <= m_ptr);
	}

	inline long tell()
	{
		return (long)(m_ptr - m_base);
	}

	inline long filesize()
	{
		return (long)m_size.LowPart;
	}

	inline long size()
	{
		return (long)m_size.LowPart;
	}

	inline BYTE* baseptr()
	{
		return m_base;
	}

	inline BYTE* dataptr()
	{
		return m_ptr;
	}

	int getc()
	{
		if (m_base + m_size.QuadPart > m_ptr)
			return (int)*m_ptr++;
		return EOF;
	}

	short getshort()
	{
		if (m_base + m_size.QuadPart > m_ptr + 1) {
			register USHORT value = (USHORT)m_ptr[0];
			value |= (USHORT)m_ptr[1] << 8;
			m_ptr += 2;
			return (short)value;
		}
		return EOF_SHORT;
	}

	short getmsbshort()
	{
		if (m_base + m_size.QuadPart > m_ptr + 1) {
			register USHORT value = (USHORT)m_ptr[0] << 8;
			value |= (USHORT)m_ptr[1];
			m_ptr += 2;
			return (short)value;
		}
		return EOF_SHORT;
	}

	unsigned long getlong()
	{
		if (m_base + m_size.QuadPart > m_ptr + 3) {
			ULONG value = ((ULONG)m_ptr[0]) | ((ULONG)m_ptr[1] << 8) |
						((ULONG)m_ptr[2] << 16) | (ULONG)m_ptr[3] << 24;
			m_ptr += 4;
			return value;
		}
		return EOF_ULONG;
	}

	unsigned long getmsblong()
	{
		if (m_base + m_size.QuadPart > m_ptr + 3) {
			ULONG value = ((ULONG)m_ptr[0] << 24) | ((ULONG)m_ptr[1] << 16) |
						((ULONG)m_ptr[2] << 8) | (ULONG)m_ptr[3];
			m_ptr += 4;
			return value;
		}
		return EOF_ULONG;
	}

	ULONG read(void* pbuf, ULONG len)
	{
		ULONG r = min(len, (ULONG)((m_base + m_size.QuadPart) - m_ptr));
		if (r > 0) {
			memcpy(pbuf, m_ptr, r);
			m_ptr += r;
			return (ULONG)r;
		}
		return 0;
	}

	long seek(long pos, DWORD mode)
	{
		switch (mode) {
			case SEEK_CUR:	m_ptr = max(0, min(m_base + m_size.QuadPart, m_ptr + pos)); break;
			case SEEK_SET:	m_ptr = m_base + max(0, min(m_size.QuadPart, pos)); break;
			default:		m_ptr = max(0, min(m_base + m_size.QuadPart, m_base + m_size.QuadPart + pos)); break;
		}
		return (long)(m_ptr - m_base);
	}

	char* gets(char* pstr, int cchMax)
	{
		if (!pstr || cchMax < 1) return NULL;

		char* p = pstr;
		int ch;
		while (cchMax--) {
			ch = getc();
			if (ch == EOF)
				break;
			if (ch == '\r') {
				if (*m_ptr == '\n') m_ptr++;
				ch = '\n';
			}
			*p++ = (char)ch;
			if (ch == '\n')
				break;
		}
		*p = 0;
		return pstr;
	}
};

#endif				// __FILEMAP_H__
