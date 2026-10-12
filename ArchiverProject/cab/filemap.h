/////////////////////////////////////////////////////////////////////
// filemap.h

#pragma once
#ifndef __FILEMAP_H__
#define __FILEMAP_H__

class filemap
{
public:
	filemap()
	{
		m_hf = INVALID_HANDLE_VALUE;
		m_map = NULL;
		m_base = NULL;
		m_size = 0;
	}

	~filemap()
	{
		close();
	}

	BOOL open(LPCWSTR name)
	{
		m_hf = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (m_hf != INVALID_HANDLE_VALUE) {
			m_size = GetFileSize(m_hf, NULL);
			m_map = CreateFileMapping(m_hf, NULL, PAGE_READONLY, 0, 0, NULL);
			if (m_map) {
				m_base = (BYTE*)MapViewOfFile(m_map, FILE_MAP_READ, 0, 0, 0);
				if (m_base)
					return TRUE;
				CloseHandle(m_map);
				m_map = NULL;
			}
			CloseHandle(m_hf);
			m_hf = INVALID_HANDLE_VALUE;
		}
		return FALSE;
	}

	void close()
	{
		if (m_hf != INVALID_HANDLE_VALUE) {
			if (m_map) {
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

	HANDLE m_hf;
	HANDLE m_map;
	BYTE* m_base;
	DWORD m_size;
};


#endif			// __FILEMAP_H__
