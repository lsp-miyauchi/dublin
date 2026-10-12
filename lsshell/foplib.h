////////////////////////////////////////////////////////////////////////
// foplib.h

#pragma once
#ifndef __FOPLIB_H__
#define __FOPLIB_H__


#include "..\lsfop\lsfop.h"

class CFopLib
{
private:
	HMODULE m_hFop;

public:
	CFopLib() : m_hFop(NULL)
	{
		lccGetVersion();
		Initialize();
	}

	~CFopLib()
	{
		Uninitialize();
	}

	bool Initialize()
	{
		// このDllのパス
		TCHAR path[MAX_PATH];
		GetModuleFileName(ATL::_AtlBaseModule.GetModuleInstance(), path, ARRAYSIZE(path));
		PathRemoveFileSpec(path);

		// lsfop.dllのロード
		TCHAR buf[MAX_PATH];
		PathCombine(buf, path, DLL_FOPSRV);
		m_hFop = LoadLibraryEx(buf, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
		return (m_hFop != NULL);
	}

	void Uninitialize()
	{
		if (m_hFop != NULL) {
			FreeLibrary(m_hFop);
			m_hFop = NULL;
		}
	}

	__inline FARPROC FindFunction(LPCSTR pszFunc)
	{
		return ((m_hFop != NULL) ? GetProcAddress(m_hFop, pszFunc) : NULL);
	}

	BOOL CreateJunctionUI(LPCTSTR pszLinkBase, LPCTSTR pszSrc)
	{
		BOOL (WINAPI* pfn)(LPCTSTR, LPCTSTR);
		(FARPROC&)pfn = FindFunction("fopCreateJunctionUI");
		if (pfn)
			return (*pfn)(pszLinkBase, pszSrc);
		return FALSE;
	}

	BOOL RemoveJunction(LPCTSTR pszPath)
	{
		BOOL (WINAPI* pfn)(LPCTSTR);
		(FARPROC&)pfn = FindFunction("fopRemoveJunction");
		if (pfn)
			return (*pfn)(pszPath);
		return FALSE;
	}

	BOOL FlushDrive(LPCTSTR pszRoot)
	{
		BOOL (WINAPI* pfn)(LPCTSTR);
		(FARPROC&)pfn = FindFunction("fopFlushDrive");
		if (pfn)
			return (*pfn)(pszRoot);
		return FALSE;
	}

	void DriveInfo(HWND hwnd, int nDrive)
	{
		void (WINAPI* pfn)(HWND, int);
		(FARPROC&)pfn = FindFunction("fopDriveInfo");
		if (pfn)
			(*pfn)(hwnd, nDrive);
	}

	void FilterDelete(HWND hParent, HDROP hdrop)
	{
		void (WINAPI* pfn)(HWND, HDROP);
		(FARPROC&)pfn = FindFunction("fopFilterDeleteEx");
		if (pfn)
			(*pfn)(hParent, hdrop);
	}

	void EnumFileSize(HWND hParent, HDROP hDrop)
	{
		void (WINAPI* pfn)(HWND, HDROP);
		(FARPROC&)pfn = FindFunction("fopEnumFileSize");
		if (pfn)
			(*pfn)(hParent, hDrop);
	}

	void EnumPathSize(HWND hParent, LPCTSTR lpDir)
	{
		void (WINAPI* pfn)(HWND, LPCTSTR);
		(FARPROC&)pfn = FindFunction("fopEnumPathSize");
		if (pfn)
			(*pfn)(hParent, lpDir);
	}

	BOOL MergeFile(HWND hwnd, HDROP hDrop)
	{
		BOOL (WINAPI* pfn)(HWND, HDROP);
		(FARPROC&)pfn = FindFunction("fopMergeFile");
		if (pfn)
			return (*pfn)(hwnd, hDrop);
		return FALSE;
	}

	BOOL FileShredder(HWND hwnd, HDROP hDrop)
	{
		BOOL (WINAPI* pfn)(HWND, HDROP, BOOL);
		(FARPROC&)pfn = FindFunction("fopFileShredder");
		if (pfn)
			return (*pfn)(hwnd, hDrop, TRUE);
		return FALSE;
	}

	BOOL QueryDivide(HWND hwnd, LPDIVIDEDATA lpdd)
	{
		BOOL (WINAPI* pfn)(HWND, LPDIVIDEDATA);
		(FARPROC&)pfn = FindFunction("fopQueryDivide");
		if (pfn)
			return (*pfn)(hwnd, lpdd);
		return FALSE;
	}

	BOOL DivideFile(HWND hwnd, LPDIVIDEDATA lpdd)
	{
		BOOL (WINAPI* pfn)(HWND, LPDIVIDEDATA);
		(FARPROC&)pfn = FindFunction("fopDivideFile");
		if (pfn)
			return (*pfn)(hwnd, lpdd);
		return FALSE;
	}

	BOOL FileAttributes(HWND hwnd, HDROP hDrop)
	{
		BOOL (WINAPI* pfn)(HWND, HDROP);
		(FARPROC&)pfn = FindFunction("fopFileAttributes");
		if (pfn)
			return (*pfn)(hwnd, hDrop);
		return FALSE;
	}

	void ShowFileInfo(HWND hwnd, HDROP hdrop)
	{
		void (WINAPI* pfn)(HWND, HDROP);
		(FARPROC&)pfn = FindFunction("fopShowFileInfo");
		if (pfn)
			(*pfn)(hwnd, hdrop);
	}

	void ShowFileInfoEx(HWND hwnd, HDROP hdrop, int nPageType)
	{
		void (WINAPI* pfn)(HWND, HDROP, int);
		(FARPROC&)pfn = FindFunction("fopShowFileInfoEx");
		if (pfn)
			(*pfn)(hwnd, hdrop, nPageType);
	}

	BOOL RenameFiles(HWND hwnd, HDROP hdrop)
	{
		BOOL (WINAPI* pfn)(HWND, HDROP);
		(FARPROC&)pfn = FindFunction("fopRenameFiles");
		if (pfn)
			return (*pfn)(hwnd, hdrop);
		return FALSE;
	}

	BOOL SeriesName(HWND hwnd, HDROP hDrop)
	{
		BOOL (WINAPI* pfn)(HWND, HDROP);
		(FARPROC&)pfn = FindFunction("fopSeriesName");
		if (pfn)
			return (*pfn)(hwnd, hDrop);
		return FALSE;
	}

	BOOL RepStrNames(HWND hwnd, HDROP hDrop)
	{
		BOOL (WINAPI* pfn)(HWND, HDROP);
		(FARPROC&)pfn = FindFunction("fopRepStrNames");
		if (pfn)
			return (*pfn)(hwnd, hDrop);
		return FALSE;
	}

	BOOL ChangeFileCaps(HWND hwnd, HDROP hDrop)
	{
		BOOL (WINAPI* pfn)(HWND, HDROP);
		(FARPROC&)pfn = FindFunction("fopChangeFileCaps");
		if (pfn)
			return (*pfn)(hwnd, hDrop);
		return FALSE;
	}

	BOOL DropFileOperation(DROPFILEOPERATION* pdfo)
	{
		BOOL (WINAPI* pfn)(DROPFILEOPERATION*);
		(FARPROC&)pfn = FindFunction("fopDropFileOperation");
		if (pfn)
			return pfn(pdfo);
		return FALSE;
	}

	BOOL GetCryptFileInfo(LPCTSTR pszFile, WIN32_FIND_DATA* pwfd, LPTSTR pszText, int cchText)
	{
		BOOL (WINAPI* pfn)(LPCTSTR, WIN32_FIND_DATA*, LPTSTR, int);
		(FARPROC&)pfn = FindFunction("fopGetCryptFileInfo");
		if (pfn)
			return pfn(pszFile, pwfd, pszText, cchText);
		return FALSE;
	}

	BOOL EncryptFilesTo(HWND hwnd, HDROP hDrop, LPCTSTR pszTo)
	{
		BOOL (WINAPI* pfn)(HWND, HDROP, LPCTSTR);
		(FARPROC&)pfn = FindFunction("fopEncryptFilesTo");
		if (pfn)
			return pfn(hwnd, hDrop, pszTo);
		return FALSE;
	}

	BOOL DecryptFilesTo(HWND hwnd, HDROP hDrop, LPCTSTR pszTo)
	{
		BOOL (WINAPI* pfn)(HWND, HDROP, LPCTSTR);
		(FARPROC&)pfn = FindFunction("fopDecryptFilesTo");
		if (pfn)
			return pfn(hwnd, hDrop, pszTo);
		return FALSE;
	}

	BOOL CreateSymbolicLinkUI(HWND hwnd, LPCTSTR pszLinkBase, LPCTSTR pszSrc)
	{
		BOOL (WINAPI* pfn)(HWND, LPCTSTR, LPCTSTR);
		(FARPROC&)pfn = FindFunction("fopCreateSymbolicLinkUI");
		if (pfn)
			return (*pfn)(hwnd, pszLinkBase, pszSrc);
		return FALSE;
	}

	BOOL CreateSystemLinkUI(HWND hwnd, LPCTSTR pszLinkBase, LPCTSTR pszSrc)
	{
		BOOL (WINAPI* pfn)(HWND, LPCTSTR, LPCTSTR);
		(FARPROC&)pfn = FindFunction("fopCreateSystemLinkUI");
		if (pfn)
			return (*pfn)(hwnd, pszLinkBase, pszSrc);
		return FALSE;
	}

	DWORD GetReparsePointInfo(LPCTSTR pszReparsePointFile, LPTSTR* ppszOriginalFile, DWORD* pdwTagType)
	{
		DWORD (WINAPI* pfn)(LPCTSTR, LPTSTR*, DWORD*);
		(FARPROC&)pfn = FindFunction("fopGetReparsePointInfo");
		if (pfn)
			return (*pfn)(pszReparsePointFile, ppszOriginalFile, pdwTagType);
		return ERROR_INVALID_FUNCTION;
	}

	void CalcFileHash(HWND hParent, HDROP hDrop)
	{
		void (WINAPI* pfn)(HWND, HDROP);
		(FARPROC&)pfn = FindFunction("fopCalcFileHash");
		if (pfn)
			(*pfn)(hParent, hDrop);
	}

	BOOL WINAPI BrowseForFolder(HWND hwnd, LPTSTR pszPath)
	{
		BOOL (WINAPI* pfn)(HWND, LPTSTR);
		(FARPROC&)pfn = FindFunction("fopBrowseForFolder");
		if (pfn) {
			return (*pfn)(hwnd, pszPath);
		}
		return FALSE;
	}

	BOOL ShowIconDetail(HWND hwnd, LPCTSTR pszPath, int index)
	{
		BOOL (WINAPI* pfn)(HWND, LPCTSTR, int);
		(FARPROC&)pfn = FindFunction("fopShowIconDetail");
		if (pfn)
			return pfn(hwnd, pszPath, index);
		return FALSE;
	}

	void ExportIconFromFile(HWND hwnd, LPCTSTR pszName, LPCTSTR pszTo)
	{
		BOOL (WINAPI* pfn)(HWND, LPCTSTR, LPCTSTR);
		(FARPROC&)pfn = FindFunction("fopExportIconFromFile");
		if (pfn)
			(*pfn)(hwnd, pszName, pszTo);
	}

	BOOL ExportSingleIcon(LPCTSTR pszName, LPCTSTR pszExportIconName, int index)
	{
		BOOL (WINAPI* pfn)(LPCTSTR, LPCTSTR, int);
		(FARPROC&)pfn = FindFunction("fopExportSingleIcon");
		if (pfn)
			return (*pfn)(pszName, pszExportIconName, index);
		return FALSE;
	}

	BOOL ExportMultipleIcons(LPCTSTR pszName, LPCTSTR pszTo, int* pnIndex, int entries)
	{
		BOOL (WINAPI* pfn)(LPCTSTR, LPCTSTR, int*, int);
		(FARPROC&)pfn = FindFunction("fopExportMultipleIcons");
		if (pfn)
			return (*pfn)(pszName, pszTo, pnIndex, entries);
		return FALSE;
	}

	HANDLE IcmOpen(LPCTSTR pszFileName)
	{
		HANDLE (WINAPI* pfn)(LPCTSTR);
		(FARPROC&)pfn = FindFunction("fopIcmOpen");
		if (pfn)
			return (*pfn)(pszFileName);
		return NULL;
	}

	void IcmClose(HANDLE h)
	{
		void (WINAPI* pfn)(HANDLE);
		(FARPROC&)pfn = FindFunction("fopIcmClose");
		if (pfn)
			(*pfn)(h);
	}

	HIMAGELIST IcmGetImageList(HANDLE h, int cx)
	{
		HIMAGELIST (WINAPI* pfn)(HANDLE, int);
		(FARPROC&)pfn = FindFunction("fopIcmGetImageList");
		if (pfn)
			return (*pfn)(h, cx);
		return NULL;
	}

	int IcmGetIconCount(HANDLE h)
	{
		int (WINAPI* pfn)(HANDLE);
		(FARPROC&)pfn = FindFunction("fopIcmGetIconCount");
		if (pfn)
			return (*pfn)(h);
		return 0;
	}

	LPCTSTR IcmGetIconPath(HANDLE h)
	{
		LPCTSTR (WINAPI* pfn)(HANDLE);
		(FARPROC&)pfn = FindFunction("fopIcmGetIconPath");
		if (pfn)
			return (*pfn)(h);
		return NULL;
	}

	HRESULT IcmCreateDataObject(HANDLE h, const int* pIndex, int entries, IDataObject** ppdo)
	{
		HRESULT (WINAPI* pfn)(HANDLE, const int*, int, IDataObject**);
		(FARPROC&)pfn = FindFunction("fopIcmCreateDataObject");
		if (pfn)
			return (*pfn)(h, pIndex, entries, ppdo);
		return E_FAIL;
	}

};



#endif		// __FOPLIB_H__
