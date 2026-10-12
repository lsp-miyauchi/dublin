////////////////////////////////////////////////////////////////////////
// unrarmodule.h

#pragma once
#ifndef __UNRARMODULE_H__
#define __UNRARMODULE_H__

#include "osplatform.h"
#include "cdll.h"
#include "unrardll\unrar.h"

///////////////////////////////////////////////////////////
//

class CEnterFolder
{
public:
	CEnterFolder()
	{
		m_szOldPath[0] = 0;
	}
	~CEnterFolder()
	{
		if (m_szOldPath[0])
			SetCurrentDirectory(m_szOldPath);
	}

	BOOL enter(LPCWSTR pszNewPath)
	{
		if (m_szOldPath[0] == 0)
			GetCurrentDirectory(ARRAYSIZE(m_szOldPath), m_szOldPath);
		if (pszNewPath && *pszNewPath)
			return SetCurrentDirectory(pszNewPath);
		return FALSE;
	}

	WCHAR m_szOldPath[MAX_PATH * 2];
};

///////////////////////////////////////////////////////////
//

class CWaitingCursor
{
public:
	HCURSOR m_hcur;
	CWaitingCursor() { m_hcur = ::SetCursor(::LoadCursor(NULL, IDC_WAIT)); }
	~CWaitingCursor() { if (m_hcur) ::SetCursor(m_hcur); }
};

//////////////////////////////////////////////////////////////////////
// メモリアロケート用

template <class T>
class CTypeAlloc
{
private:
	T* m_aT;
	int m_size;
public:
	__inline CTypeAlloc(int size) { m_size = size; m_aT = (T*)malloc(sizeof(T) * size); }
	__inline ~CTypeAlloc() { if (m_aT) free(m_aT); }
	__inline operator T*() const { return m_aT; }
	__inline bool SetAt(int index, T t) { m_aT[index] = t; return true; }
	__inline const T& operator[] (int index) const { return m_aT[index]; }
	__inline T& operator[] (int index) { return m_aT[index]; }
};

///////////////////////////////////////////////////////////////////////
//

class CUnrarModule
{
public:
	COSPlatform m_osp;
	CWindow m_owner;
	long m_module_ref;
	DWORD m_module_lock;
	CDll m_dll;

	HANDLE (PASCAL* m_RAROpenArchive)(struct RAROpenArchiveData *);
	HANDLE (PASCAL* m_RAROpenArchiveEx)(struct RAROpenArchiveDataEx *);
	int    (PASCAL* m_RARCloseArchive)(HANDLE);
	int    (PASCAL* m_RARReadHeader)(HANDLE, struct RARHeaderData *);
	int    (PASCAL* m_RARReadHeaderEx)(HANDLE, struct RARHeaderDataEx *);
	int    (PASCAL* m_RARProcessFile)(HANDLE, int, const char *, const char *);
	int    (PASCAL* m_RARProcessFileW)(HANDLE, int, const wchar_t *, const wchar_t *);
	void   (PASCAL* m_RARSetCallback)(HANDLE, UNRARCALLBACK, LPARAM);
	void   (PASCAL* m_RARSetChangeVolProc)(HANDLE, CHANGEVOLPROC);
	void   (PASCAL* m_RARSetProcessDataProc)(HANDLE, PROCESSDATAPROC);
	void   (PASCAL* m_RARSetPassword)(HANDLE, char *);
	int    (PASCAL* m_RARGetDllVersion)();

	HRESULT Init(ATL::_ATL_OBJMAP_ENTRY* pObjMap, HINSTANCE hInstance, const GUID* pLibID = NULL)
	{
		UNREFERENCED_PARAMETER(pLibID);
		UNREFERENCED_PARAMETER(hInstance);
		UNREFERENCED_PARAMETER(pObjMap);

		m_module_ref = 0;
		m_module_lock = 0;
		m_owner = (HWND)NULL;

		// unrardll初期化
		InitUnrar();

		return S_OK;
	}
	void Term()
	{
		m_dll.FreeLibrary();
	}
	HWND SetOwnerWindow(HWND hwndOwner)
	{
		HWND hwndPrev = m_owner.m_hWnd;
		m_owner = hwndOwner;
		return hwndPrev;
	}
	HWND GetOwnerWindow()
	{
		return m_owner;
	}
	inline void AddModuleRef() { m_module_ref++; }
	inline void DecModuleRef() { m_module_ref--; }
	inline bool IsModuleRef() { return (m_module_ref > 0); }
	inline void SetModuleLock(DWORD lock) { m_module_lock = lock; }
	inline DWORD GetModuleLock() { return m_module_lock; }
	inline bool IsModuleLocked() { return (m_module_lock != 0); }
//	inline bool OsNT() { return m_osp.OsNT(); }

	void InitUnrar()
	{
		// unrar.dllのロードとapiの解決
		m_RAROpenArchive        = NULL;
		m_RAROpenArchiveEx      = NULL;
		m_RARCloseArchive       = NULL;
		m_RARReadHeader         = NULL;
		m_RARReadHeaderEx       = NULL;
		m_RARProcessFile        = NULL;
		m_RARProcessFileW       = NULL;
		m_RARSetCallback        = NULL;
		m_RARSetChangeVolProc   = NULL;
		m_RARSetProcessDataProc = NULL;
		m_RARSetPassword        = NULL;
		m_RARGetDllVersion      = NULL;
#ifdef _WIN64
		if (m_dll.LoadLibrary(_T("unrar64.dll")) != NULL) {
#else
		if (m_dll.LoadLibrary(_T("unrar.dll")) != NULL) {
#endif
			(FARPROC&)m_RAROpenArchive        = m_dll.FindFunction("RAROpenArchive");
			(FARPROC&)m_RAROpenArchiveEx      = m_dll.FindFunction("RAROpenArchiveEx");
			(FARPROC&)m_RARCloseArchive       = m_dll.FindFunction("RARCloseArchive");
			(FARPROC&)m_RARReadHeader         = m_dll.FindFunction("RARReadHeader");
			(FARPROC&)m_RARReadHeaderEx       = m_dll.FindFunction("RARReadHeaderEx");
			(FARPROC&)m_RARProcessFile        = m_dll.FindFunction("RARProcessFile");
			(FARPROC&)m_RARProcessFileW       = m_dll.FindFunction("RARProcessFileW");
			(FARPROC&)m_RARSetCallback        = m_dll.FindFunction("RARSetCallback");
			(FARPROC&)m_RARSetChangeVolProc   = m_dll.FindFunction("RARSetChangeVolProc");
			(FARPROC&)m_RARSetProcessDataProc = m_dll.FindFunction("RARSetProcessDataProc");
			(FARPROC&)m_RARSetPassword        = m_dll.FindFunction("RARSetPassword");
			(FARPROC&)m_RARGetDllVersion      = m_dll.FindFunction("RARGetDllVersion");
		}
	}

	HANDLE RAROpenArchive(struct RAROpenArchiveData *oad)
	{
		return (m_RAROpenArchive != NULL) ? m_RAROpenArchive(oad) : NULL;
	}
	HANDLE RAROpenArchiveEx(struct RAROpenArchiveDataEx *oad)
	{
		return (m_RAROpenArchiveEx != NULL) ? m_RAROpenArchiveEx(oad) : NULL;
	}
	int RARCloseArchive(HANDLE had)
	{
		return (m_RARCloseArchive != NULL) ? m_RARCloseArchive(had) : -1;
	}
	int RARReadHeader(HANDLE had,struct RARHeaderData *hdr)
	{
		return (m_RARReadHeader != NULL) ? m_RARReadHeader(had, hdr) : -1;
	}
	int RARReadHeaderEx(HANDLE had,struct RARHeaderDataEx *hdr)
	{
		return (m_RARReadHeaderEx != NULL) ? m_RARReadHeaderEx(had, hdr) : -1;
	}
	int RARProcessFile(HANDLE had,int Operation,const char *DestPath,const char *DestName)
	{
		return (m_RARProcessFile != NULL) ? m_RARProcessFile(had, Operation, DestPath, DestName) : -1;
	}
	int RARProcessFileW(HANDLE had,int Operation,const wchar_t *DestPath,const wchar_t *DestName)
	{
		return (m_RARProcessFileW != NULL) ? m_RARProcessFileW(had, Operation, DestPath, DestName) : -1;
	}
	void RARSetCallback(HANDLE had,UNRARCALLBACK Callback,LPARAM UserData)
	{
		if (m_RARSetCallback) m_RARSetCallback(had, Callback, UserData);
	}
	void RARSetChangeVolProc(HANDLE had,CHANGEVOLPROC ChangeVolProc)
	{
		if (m_RARSetChangeVolProc) m_RARSetChangeVolProc(had, ChangeVolProc);
	}
	void RARSetProcessDataProc(HANDLE had,PROCESSDATAPROC ProcessDataProc)
	{
		if (m_RARSetProcessDataProc) m_RARSetProcessDataProc(had, ProcessDataProc);
	}
	void RARSetPassword(HANDLE had,char *Password)
	{
		if (m_RARSetPassword) m_RARSetPassword(had, Password);
	}
	int RARGetDllVersion()
	{
		return (m_RARGetDllVersion != NULL) ? m_RARGetDllVersion() : 0;
	}
};

extern CUnrarModule _G;


#endif			// __CABMODULE_H__
