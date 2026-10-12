//////////////////////////////////////////////////////////////////////
// filemap.cpp

#include "stdafx.h"
#include "filemap.h"


BOOL filemap::open(const char* name)
{
	m_hf = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
					FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (m_hf != INVALID_HANDLE_VALUE)
	{
		m_size = GetFileSize(m_hf, NULL);
		m_map = CreateFileMapping(m_hf, NULL, PAGE_READONLY, 0, 0, NULL);
		if (m_map)
		{
			m_base = (BYTE*)MapViewOfFile(m_map, FILE_MAP_READ, 0, 0, 0);
			if (m_base)
			{
				m_ptr = (BYTE*)m_base;
				return TRUE;
			}
			CloseHandle(m_map);
			m_map = NULL;
		}
		CloseHandle(m_hf);
		m_hf = INVALID_HANDLE_VALUE;
	}
	return FALSE;
}

void filemap::close()
{
	if (m_hf != INVALID_HANDLE_VALUE)
	{
		if (m_map)
		{
			if (m_base)
			{
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

short filemap::getshort()
{
	if (m_base + m_size > m_ptr + 1)
	{
		register USHORT value = (USHORT)m_ptr[0];
		value |= (USHORT)m_ptr[1] << 8;
		m_ptr += 2;
		return (short)value;
	}
	return (short)~0;
}

short filemap::getmsbshort()
{
	if (m_base + m_size > m_ptr + 1)
	{
		register USHORT value = (USHORT)m_ptr[0] << 8;
		value |= (USHORT)m_ptr[1];
		m_ptr += 2;
		return (short)value;
	}
	return (short)~0;
}

ULONG filemap::getlong()
{
	if (m_base + m_size > m_ptr + 3)
	{
		ULONG value = ((ULONG)m_ptr[0]) | ((ULONG)m_ptr[1] << 8) |
					((ULONG)m_ptr[2] << 16) | (ULONG)m_ptr[3] << 24;
		m_ptr += 4;
		return value;
	}
	return (ULONG)~0;
}

ULONG filemap::getmsblong()
{
	if (m_base + m_size > m_ptr + 3)
	{
		ULONG value = ((ULONG)m_ptr[0] << 24) | ((ULONG)m_ptr[1] << 16) |
					((ULONG)m_ptr[2] << 8) | (ULONG)m_ptr[3];
		m_ptr += 4;
		return value;
	}
	return (ULONG)~0;
}

int filemap::read(void* pbuf, int len)
{
	int r = min(len, (m_base + m_size) - m_ptr);
	if (r > 0)
	{
		memcpy(pbuf, m_ptr, r);
		m_ptr += r;
		return r;
	}
	return 0;
}

long filemap::seek(long pos, int mode)
{
	if (mode == SEEK_CUR)
		m_ptr = max(0, min(m_base + m_size, m_ptr + pos));
	else if (mode == SEEK_SET)
		m_ptr = m_base + max(0, min(m_size, pos));
	else
		m_ptr = max(0, min(m_base + m_size, m_base + m_size + pos));
	return m_ptr - m_base;
}

