////////////////////////////////////////////////////////////////////////
// pidl.cpp

#include "stdafx.h"

BOOL idl_GetName(LPSHELLFOLDER psf, LPCITEMIDLIST pidl, DWORD flags, CString& strName)
{
	BOOL bOK = FALSE;
	STRRET str;
	if (psf && pidl && NOERROR == psf->GetDisplayNameOf(pidl, flags, &str)) {
		LPTSTR psz;
		if (StrRetToStr(&str, pidl, &psz) == S_OK) {
			strName = psz;
			CoTaskMemFree(psz);
			bOK = TRUE;
		}
	}
	return bOK;
}

BOOL idl_GetName(LPCITEMIDLIST pidlFq, DWORD flag, CString& str)
{
	BOOL bOK = FALSE;
	if (pidlFq) {
		IShellFolder* psf;
		LPCITEMIDLIST pidl;
		if (SUCCEEDED(SHBindToParent(pidlFq, IID_IShellFolder, (void**)&psf, &pidl))) {
			bOK = idl_GetName(psf, pidl, flag, str);
			psf->Release();
		}
	}
	return bOK;
}

BOOL getlinkinfo(LPCTSTR pszName, CString& target)
{
	BOOL bOK = FALSE;
	CComPtr<IShellLink> psl;
	if (psl.CoCreateInstance(CLSID_ShellLink) == S_OK) {
		CComQIPtr<IPersistFile, &IID_IPersistFile> ppf(psl);
		if (ppf.p != NULL) {
			if (ppf->Load(pszName, STGM_READ) == S_OK) {
				LPITEMIDLIST pidl;
				if (NOERROR == psl->GetIDList(&pidl)) {
					TCHAR buf[MAX_PATH * 2];
					if (SHGetPathFromIDList(pidl, buf)) {
						target = buf;
						bOK = TRUE;
					}
					else
						bOK = idl_GetName(pidl, SHGDN_FORPARSING | SHGDN_NORMAL, target);
					CoTaskMemFree(pidl);
				}
			}
		}
	}
	return bOK;
}

BOOL idl_GetPathFromIDList(LPCITEMIDLIST pidlFq, CString& strPath)
{
	BOOL ok = FALSE;
	strPath.Empty();
	if (pidlFq) {
		ok = idl_GetName(pidlFq, SHGDN_FORPARSING, strPath);
		if (ok) {
			SHFILEINFO sfi;
			sfi.dwAttributes = SFGAO_FOLDER | SFGAO_LINK;
			SHGetFileInfo((LPCTSTR)pidlFq, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_ATTR_SPECIFIED | SHGFI_ATTRIBUTES);
			if ((sfi.dwAttributes & SFGAO_LINK) && (sfi.dwAttributes & SFGAO_FOLDER)) {
				CString tmp;
				if (getlinkinfo(strPath, tmp)) {
					if (!PathIsURL(tmp)) {
						strPath = tmp;
					}
				}
			}

			WIN32_FILE_ATTRIBUTE_DATA wfad;
			if (!GetFileAttributesEx(strPath, GetFileExInfoStandard, &wfad))
				ok = FALSE;
		}
	}
	return ok;
}
