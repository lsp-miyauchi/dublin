//////////////////////////////////////////////////////////////////////
// mapfile.cpp

#include "stdafx.h"
#include "lsmif.h"


mapfile::mapfile()
{
	m_base = NULL;
	m_hfile = NULL;
	m_hmap = NULL;
}

mapfile::~mapfile()
{
	close();
}

void* mapfile::create(DWORD size)
{
	close();

	char tmp[MAX_PATH];
	GetTempPath(MAX_PATH, tmp);
	GetTempFileName(tmp, "~~mf", 0, m_tmpfile);
	m_hfile = CreateFile(m_tmpfile, GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							NULL, CREATE_ALWAYS,
							FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_RANDOM_ACCESS,
							NULL);

	if (m_hfile != INVALID_HANDLE_VALUE) {
		SetFilePointer(m_hfile, size, NULL, FILE_BEGIN);
		SetEndOfFile(m_hfile);
		SetFilePointer(m_hfile, 0, NULL, FILE_BEGIN);

		if (GetFileSize(m_hfile, NULL) == size) {
			m_hmap = CreateFileMapping(m_hfile, NULL, PAGE_READWRITE, 0, 0, NULL);
			if (m_hmap) {
				m_base = MapViewOfFile(m_hmap, FILE_MAP_WRITE, 0, 0, 0);
				if (!m_base) {
					CloseHandle(m_hmap);
					m_hmap = NULL;
				}
			}
		}
		if (!m_base) {
			CloseHandle(m_hfile);
			m_hfile = NULL;
			DeleteFile(m_tmpfile);
		}
	} else {
		m_hfile = NULL;
		m_hmap = NULL;
	}
	return m_base;
}

void mapfile::close()
{
	if (m_base) {
		UnmapViewOfFile(m_base);
		CloseHandle(m_hmap);
		CloseHandle(m_hfile);
		DeleteFile(m_tmpfile);
	}

	m_base = NULL;
	m_hfile = NULL;
	m_hmap = NULL;
}
