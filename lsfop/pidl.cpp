////////////////////////////////////////////////////////////////
// pidl.cpp

#include "stdafx.h"
#include "data.h"
#include "pidl.h"

BOOL __fastcall _pathisdirectory(LPCTSTR pszName);

////////////////////////////////////////////////////////////////
//

#ifdef PIDL_LOCAL_FUNCTIONS

UINT __fastcall idl_GetSize(LPCITEMIDLIST pidl)
{
	UINT cbTotal = 0;
	if (pidl)
	{
		cbTotal += sizeof(pidl->mkid.cb);	// Null terminator
		while (pidl && pidl->mkid.cb)
		{
			cbTotal += pidl->mkid.cb;
			pidl = idl_Next(pidl);
		}
	}
	return cbTotal;
}

LPITEMIDLIST __fastcall idl_ConcatPidls(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
	UINT cb1;
	if (pidl1)								// May be NULL
		cb1 = idl_GetSize(pidl1) - sizeof(pidl1->mkid.cb);
	else
		cb1 = 0;

	UINT cb2 = idl_GetSize(pidl2);

	LPITEMIDLIST pidlNew = idl_Create(cb1 + cb2);
	if (pidlNew)
	{
		if (pidl1)
			memcpy(pidlNew, pidl1, cb1);
		memcpy(((BYTE*)pidlNew) + cb1, pidl2, cb2);
	}
	return pidlNew;
}

LPITEMIDLIST __fastcall idl_CopyITEMID(LPITEMIDLIST pidl)
{
	// アイテムリストの先頭アイテムをコピー
	if (!pidl)
		return NULL;

	int nSize = pidl->mkid.cb;
	LPITEMIDLIST pidlT;
	pidlT = (LPITEMIDLIST)idl_Alloc(nSize + sizeof(USHORT));
	memcpy(pidlT, pidl, nSize);
	*((USHORT*)(((LPBYTE)pidlT) + nSize)) = 0;
	return pidlT;
}

LPITEMIDLIST __fastcall idl_Copy(LPCITEMIDLIST pidlSource)
{
	// アイテムリストすべてをコピー
	UINT cbSource = 0;
	if (NULL == pidlSource)
		return (NULL);

	// Allocate the new ITEMIDLIST
	cbSource = idl_GetSize(pidlSource);
	LPITEMIDLIST pidlTarget = (LPITEMIDLIST)idl_Alloc(cbSource);
	if (!pidlTarget)
		return (NULL);

	// Copy the source to the target
	memcpy(pidlTarget, pidlSource, cbSource);
	return (pidlTarget);
}

LPITEMIDLIST __fastcall idl_GetFromPath(LPCTSTR pszFile)
{
	LPITEMIDLIST pidl = NULL;
	IShellFolder* psf;
	if (SHGetDesktopFolder(&psf) == NOERROR)
	{
		// Convert the path to an ITEMIDLIST
		ULONG eat;
		if (psf->ParseDisplayName(NULL, NULL, (LPTSTR)pszFile, &eat, &pidl, NULL) != NOERROR)
			pidl = NULL;
		psf->Release();
	}
	return pidl;
}

#endif		// PIDL_LOCAL_FUNCTIONS

LPITEMIDLIST __fastcall idl_Next(LPCITEMIDLIST pidl)
{
	LPITEMIDLIST pidlNext = ILGetNext(pidl);
	return ((pidlNext == NULL) || (pidlNext->mkid.cb == 0)) ? NULL : pidlNext;
}

LPITEMIDLIST __fastcall idl_Create(UINT cbSize)
{
	LPITEMIDLIST pidl = NULL;
	pidl = (LPITEMIDLIST)idl_Alloc(cbSize);
	if (pidl)
		memset(pidl, 0, cbSize);			// zero-init for external task alloc
	return pidl;
}

BOOL __fastcall idl_GetInfoTip(HWND hwnd, IShellFolder* psf, LPITEMIDLIST pidl, LPTSTR pTip)
{
	BOOL bOK = FALSE;
	IQueryInfo* pqi;
	if (psf->GetUIObjectOf(hwnd, 1, (LPCITEMIDLIST*)&pidl,
							IID_IQueryInfo, NULL, (void**)&pqi) == NOERROR)
	{
		LPWSTR pw;
		if (pqi->GetInfoTip(0, &pw) == NOERROR)
		{
			lstrcpy(pTip, pw);
			CoTaskMemFree(pw);
			bOK = TRUE;
		}
		pqi->Release();
	}
	return bOK;
}

BOOL __fastcall idl_GetName(IShellFolder* psf, LPITEMIDLIST pidl, DWORD dwFlags, CString& name)
{
	STRRET str;
	if (NOERROR == psf->GetDisplayNameOf(pidl, dwFlags, &str)) {
		LPTSTR psz;
		if (StrRetToStr(&str, pidl, &psz) == S_OK) {
			name = psz;
			CoTaskMemFree(psz);
			return TRUE;
		}
	}
	return FALSE;
}

BOOL __fastcall idl_GetName(IShellFolder* psf, LPITEMIDLIST pidl, DWORD dwFlags, LPTSTR pName, UINT cchName)
{
	CString str;
	if (idl_GetName(psf, pidl, dwFlags, str)) {
		lstrcpyn(pName, str, cchName);
		return TRUE;
	}
	return FALSE;
}

BOOL __fastcall idl_GetName(LPITEMIDLIST pidlFq, DWORD dwFlags, CString& name)
{
	BOOL bOK = FALSE;
	IShellFolder* psf;
	LPITEMIDLIST pidl;
	if (pidlFq &&
		SUCCEEDED(SHBindToParent(pidlFq, IID_IShellFolder, (void**)&psf, (LPCITEMIDLIST*)&pidl))) {
		bOK = idl_GetName(psf, pidl, dwFlags, name);
		psf->Release();
	}
	return bOK;
}

BOOL __fastcall idl_GetName(LPITEMIDLIST pidlFq, DWORD dwFlags, LPTSTR pName, UINT cchName)
{
	CString str;
	if (idl_GetName(pidlFq, dwFlags, str)) {
		lstrcpyn(pName, str, cchName);
		return TRUE;
	}
	return FALSE;
}

BOOL __fastcall idl_GetIcon(IShellFolder* lpsf, LPITEMIDLIST pidl, HICON* phLarge, HICON* phSmall)
{
	BOOL bOK = FALSE;
	*phLarge = NULL;
	*phSmall = NULL;
	LPEXTRACTICON lpxi;
	HRESULT hr = lpsf->GetUIObjectOf(NULL,
									1,
									(LPCITEMIDLIST*)&pidl,
									IID_IExtractIcon,
									0,
									(LPVOID*)&lpxi);
	if (SUCCEEDED(hr))
	{
		TCHAR buf[MAX_PATH];
		int nIndex;
		UINT nFlags;
		hr = lpxi->GetIconLocation(GIL_FORSHELL, buf, MAX_PATH, &nIndex, &nFlags);
		if (SUCCEEDED(hr))
		{
			hr = lpxi->Extract(buf, nIndex,	phLarge, phSmall, MAKELONG(32, 16));
			if (!SUCCEEDED(hr))
				ExtractIconEx(buf, nIndex, phLarge, phSmall, 1);
			bOK = TRUE;
			lpxi->Release();
		}
	}
	return bOK;
}

////////////////////////////////////////////////////////////////
//  This routine takes a fully qualified pidl for a folder and returns 
//	the IShellFolder pointer for that pidl
//
//	IN: pidl - Pointer to a fully qualified ITEMIDLIST for the folder
//
IShellFolder* __fastcall idl_GetFileFolder(LPITEMIDLIST pidl)
{
	IShellFolder* psf = NULL;
	IShellFolder* pFolder;
	if (SHGetDesktopFolder(&pFolder) == NOERROR)
	{
		if (pFolder->BindToObject(pidl, 0,
							IID_IShellFolder, (LPVOID*)&psf) != NOERROR)
			psf = NULL;
		pFolder->Release();
	}
	return psf;
}

LPVOID __fastcall idl_GetUIObjectOf(HWND hwnd, LPITEMIDLIST pidlTarget, REFIID riid)
{
	IShellFolder* psf;
	LPITEMIDLIST pidl;
	if (FAILED(SHBindToParent(pidlTarget,
					IID_IShellFolder, (void**)&psf, (LPCITEMIDLIST*)&pidl)))
	{
		if (SHGetDesktopFolder(&psf) != NOERROR)
			return NULL;
		pidl = pidlTarget;
	}

	void* pv = NULL;
	if (NOERROR != psf->GetUIObjectOf(hwnd, 1,
							(LPCITEMIDLIST*)&pidl, riid, 0, &pv))
		pv = NULL;
	psf->Release();
	return pv;
}

BOOL __fastcall idl_FindTree(IShellFolder* psf, LPIDLFTPROC pfnCallback, LPVOID pv)
{
	LPENUMIDLIST pe;
	if (psf->EnumObjects(NULL,
					SHCONTF_NONFOLDERS | SHCONTF_FOLDERS | SHCONTF_INCLUDEHIDDEN,
					&pe) == NOERROR)
	{
		LPITEMIDLIST pidl;
		ULONG celt;
		while (pe->Next(1, &pidl, &celt) == NOERROR && celt > 0)
		{
			DWORD dwAttr = 0xF0000000 | SFGAO_CAPABILITYMASK;
			psf->GetAttributesOf(1, (LPCITEMIDLIST*)&pidl, &dwAttr);
			if (dwAttr & SFGAO_FOLDER)
			{
				IShellFolder* psfw;
				if (psf->BindToObject(pidl, NULL,
								IID_IShellFolder, (void**)&psfw) == NOERROR)
				{
					BOOL bOK = idl_FindTree(psfw, pfnCallback, pv);
					psfw->Release();
					if (!bOK)
					{
						CoTaskMemFree(pidl);
						pe->Release();
						return FALSE;
					}
				}
			}
			else
			{
				if (!(*pfnCallback)(psf, pidl, pv))
				{
					CoTaskMemFree(pidl);
					pe->Release();
					return FALSE;
				}
			}
			CoTaskMemFree(pidl);
		}
		pe->Release();
	}
	return TRUE;
}

BOOL __fastcall InvokeIDList(HWND hParent, LPITEMIDLIST pidl, LPCTSTR lpVerb)
{
	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(SHELLEXECUTEINFO));
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.fMask = SEE_MASK_IDLIST | SEE_MASK_INVOKEIDLIST | SEE_MASK_UNICODE;
	sei.hwnd = hParent;
	sei.nShow = SW_SHOWNORMAL;
	sei.lpIDList = pidl;
	sei.lpVerb = lpVerb;

	TCHAR buf[MAX_PATH];
	if (SHGetPathFromIDList(pidl, buf) && !_pathisdirectory(buf)) {
		LPTSTR p = PathFindFileName(buf);
		*p = '\0';
		sei.lpDirectory = buf;
	}
	return ShellExecuteEx(&sei);
}

BOOL __fastcall ContextVerb(HWND hwnd, LPCTSTR lpName, LPCTSTR lpVerb)
{
	BOOL bOK = FALSE;
	LPITEMIDLIST pidl = ILCreateFromPath(lpName);
	if (pidl)
	{
		bOK = InvokeIDList(hwnd, pidl, lpVerb);
		CoTaskMemFree(pidl);
	}
	return bOK;
}

HBITMAP __fastcall idl_ExtractImage(LPITEMIDLIST pidl, SIZE* prgSize, DWORD depth)
{
	HBITMAP hbm = NULL;
	IExtractImage* pie = (IExtractImage*)idl_GetUIObjectOf(GetFocus(), pidl, IID_IExtractImage);
	if (pie)
	{
		OLECHAR wsz[MAX_PATH * 2];
		DWORD prior = 0;			// IEI_PRIORITY_NORMAL is defined nowhere!
		DWORD flags = IEIFLAG_SCREEN | IEIFLAG_OFFLINE | IEIFLAG_CACHE | IEIFLAG_ASYNC;
		HRESULT hr = pie->GetLocation(wsz, ARRAYSIZE(wsz), &prior, prgSize, depth, &flags);

		// even if we've got shell v4.70+, not all files support thumbnails 
		if (NOERROR == hr)
			pie->Extract(&hbm);
		else if (E_PENDING == hr)
		{
			IRunnableTask* prt;
			BOOL bExtract = TRUE;
			if (NOERROR == pie->QueryInterface(IID_IRunnableTask, (void**)&prt))
			{
				if (NOERROR == prt->Run())
				{
					DWORD timeout = GetTickCount() + 10000;
					while (prt->IsRunning() != IRTIR_TASK_FINISHED)
					{
						if (timeout > GetTickCount())
						{
							bExtract = FALSE;
							break;
						}
					}
				}
				prt->Release();
			}

			if (bExtract)
				pie->Extract(&hbm);
		}
		pie->Release();
	}

	return hbm;
}

BOOL __fastcall idl_LimitInputEdit(HWND hEdit, IShellFolder* psf)
{
	BOOL bOK = FALSE;
	HRESULT (WINAPI* _fSHLimitInputEdit)(HWND, IShellFolder*);
	HMODULE hmod = GetModuleHandle(gszShell32);
	if (hmod) {
		(FARPROC&)_fSHLimitInputEdit = GetProcAddress(hmod, "SHLimitInputEdit");
		if (_fSHLimitInputEdit)
			bOK = ((*_fSHLimitInputEdit)(hEdit, psf) == S_OK);
	}
	return bOK;
}

BOOL __fastcall idl_LimitInputEdit(HWND hEdit, LPCTSTR pszPath)
{
	BOOL bOK = FALSE;
	LPITEMIDLIST pidl = ILCreateFromPath(pszPath);
	if (pidl) {
		IShellFolder* psf = idl_GetFileFolder(pidl);
		if (psf) {
			bOK = idl_LimitInputEdit(hEdit, psf);
			psf->Release();
		}
		CoTaskMemFree(pidl);
	}
	return bOK;
}

