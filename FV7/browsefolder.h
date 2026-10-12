////////////////////////////////////////////////////////////////////////
// browsefolder.h

#pragma once

#ifndef __BROWSEFOLDER_H__
#define __BROWSEFOLDER_H__

BOOL getlinkinfo(LPCTSTR pszName, CString& target);

////////////////////////////////////////////////////////////////////////
//

#define DONT_DISMISS		1

class CBrowseFolder
{
private:
	BROWSEINFO m_bi;

	static int CALLBACK browse_pidl_callback(HWND hwnd, UINT msg, LPARAM /*lParam*/, LPARAM lpData)
	{
		if (msg == BFFM_INITIALIZED)
			SendMessage(hwnd, BFFM_SETSELECTION, FALSE, lpData);
		return 0;
	}

	static int CALLBACK browse_path_callback(HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData)
	{
		switch (msg) {
		case BFFM_INITIALIZED:
			if (lpData && PathFileExists((LPCTSTR)lpData))
				SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
			break;

		case BFFM_SELCHANGED:
		{
			TCHAR buf[MAX_PATH * 2];
			BOOL bOK = SHGetPathFromIDList((LPITEMIDLIST)lParam, buf);
			SendMessage(hwnd, BFFM_ENABLEOK, 0, bOK);
			break;
		}

		case BFFM_VALIDATEFAILEDA:
		case BFFM_VALIDATEFAILEDW:
			return DONT_DISMISS;

		default:
			break;
		}
		return 0;
	}

	LPITEMIDLIST Browse(HWND hwnd, BFFCALLBACK pfnCallback, LPARAM lParam, BOOL bSysDirOnly = FALSE)
	{
		memset(&m_bi, 0, sizeof(m_bi));
		m_bi.hwndOwner = hwnd;
		m_bi.pidlRoot = NULL;
		m_bi.lpszTitle = NULL;
		m_bi.pszDisplayName = NULL;
		m_bi.ulFlags = BIF_USENEWUI | BIF_VALIDATE | BIF_NEWDIALOGSTYLE | BIF_UAHINT;
		if (bSysDirOnly)
			m_bi.ulFlags |= BIF_RETURNONLYFSDIRS;
		m_bi.lParam = lParam;
		m_bi.lpfn = pfnCallback;

		TCHAR dir[MAX_PATH * 2];
		GetCurrentDirectory(ARRAYSIZE(dir), dir);
		LPITEMIDLIST pidl = SHBrowseForFolder(&m_bi);
		SetCurrentDirectory(dir);

		return pidl;
	}

public:
	LPITEMIDLIST BrowsePidl(HWND hwnd, LPITEMIDLIST pidl)
	{
		return Browse(hwnd, browse_pidl_callback, (LPARAM)pidl);
	}

	BOOL BrowsePath(HWND hwnd, CString& path)
	{
		BOOL bOK = FALSE;
		CIDListPtr idl = Browse(hwnd, browse_path_callback, (LPARAM)path.GetString());
		if (!idl.isNull()) {
			CString str;
			if (idl.GetPath(str)) {
				SHFILEINFO sfi;
				sfi.dwAttributes = SFGAO_LINK;
				idl.GetFileInfo(0, &sfi, sizeof(sfi), SHGFI_ATTR_SPECIFIED | SHGFI_ATTRIBUTES);
				if (sfi.dwAttributes & SFGAO_LINK) {
					CString tmp;
					if (getlinkinfo(str, tmp))
						str = tmp;
				}
				path = str;
				bOK = TRUE;
			}
		}
		return bOK;
	}
};

#endif			// __BROWSEFOLDER_H__
