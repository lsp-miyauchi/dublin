////////////////////////////////////////////////////////////////////////
// sharedmem.h

#pragma once

enum {
	SM_NOT_CREATED = -1,
	SM_CREATED_NEW,				// マッピングファイルは存在しなかったため､ 作成されました。
	SM_OPENED_EXISTING,			// マッピングファイルは存在しているため､ 変更されることなく単にオープンされました。
};

class CSharedMemory
{
private:
	HANDLE m_map;
	void* m_baseptr;
	CString m_name;
	ULARGE_INTEGER m_size;

public:
	CSharedMemory()
	{
		m_map = NULL;
		m_baseptr = NULL;
		m_name.Empty();
		m_size.QuadPart = 0;
	}

	~CSharedMemory()
	{
		Close();
	}

	void Close()
	{
		UnmapPointer();
		if (m_map) {
			CloseHandle(m_map);
			m_map = NULL;
		}
		m_name.Empty();
		m_size.QuadPart = 0;
	}

	bool Open(LPCTSTR pszMapName, size_t memSize, int& dispo)
	{
		if (m_map) {
			if (!m_name.CompareNoCase(pszMapName)) {
				dispo = SM_OPENED_EXISTING;
				return true;
			}
			Close();
		}

		// 割り当てサイズをページサイズの倍数に修正
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		size_t numBlock = (memSize / (size_t)si.dwPageSize);
		if (memSize % (size_t)si.dwPageSize) numBlock++;
		m_size.QuadPart = numBlock * (size_t)si.dwPageSize;

		m_map = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, m_size.HighPart, m_size.LowPart, pszMapName);
		if (!m_map)
			dispo = SM_NOT_CREATED;
		else
			dispo = (GetLastError() == ERROR_ALREADY_EXISTS) ? SM_OPENED_EXISTING : SM_CREATED_NEW;
		return (m_map != NULL);
	}

	void* MapPointer()
	{
		if (!m_baseptr)
			m_baseptr = MapViewOfFile(m_map, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		return m_baseptr;
	}

	void UnmapPointer()
	{
		if (m_baseptr) {
			UnmapViewOfFile(m_baseptr);
			m_baseptr = NULL;
		}
	}
};
