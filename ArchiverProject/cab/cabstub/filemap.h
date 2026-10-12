/////////////////////////////////////////////////////////////////////
// filemap.h

#pragma once
#ifndef __FILEMAP_H__
#define __FILEMAP_H__

#ifdef getc
#undef getc
#endif

#define EOF_SHORT		((short)~0)
#define EOF_ULONG		((ULONG)~0)


class filemap
{
public:
	BYTE* m_base;
	BYTE* m_ptr;
	LARGE_INTEGER m_size;

	filemap() : m_base(NULL), m_ptr(NULL)
	{
	}

	__inline ~filemap() { close(); }
	__inline bool isopen() { return (m_base != NULL); }

	bool open_mem(void* pbase, long size)
	{
		if (!pbase || size < 1) return false;

		m_base = (BYTE*)pbase;
		m_ptr = m_base;
		m_size.QuadPart = size;
		return true;
	}

	__inline void close() { }
	__inline bool eof() { return (m_base + m_size.QuadPart <= m_ptr); }
	__inline long tell() { return (long)(m_ptr - m_base); }
	__inline long filesize() { return (long)m_size.LowPart; }

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
};

#endif				// __FILEMAP_H__
