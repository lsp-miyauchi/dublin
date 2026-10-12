/////////////////////////////////////////////////////////////////////////////
// CFolderItem

#include "stdafx.h"
#include "fv7.h"
#include "shellexec.h"


// #define SHOW_ATTR_VALUE

BOOL CFolderItem::GetRealPathName(CString& strPath)
{
	if (GetDescriptionId() == SHDID_FS_DIRECTORY)
		return GetPathName(strPath);

	TCHAR buf[MAX_PATH * 2];
	if (SHGetPathFromIDList(m_pidlFq, buf)) {
		strPath = buf;
		return TRUE;
	}
	return FALSE;
}

BOOL CFolderItem::GetPathName(CString& strPath)
{
	if (idl_GetName(m_pidlFq, SHGDN_FORPARSING, strPath) &&
		(PathIsRoot(strPath) || _pathfileexists(strPath))) {
		// ターゲットフォルダがリンク属性を持っていることがあるので
		// 実際のリンク先を取得する
		if (GetAttributes() & SFGAO_LINK) {
			CString target;
			if (getlinkinfo(strPath, target)) {
				if (PathIsURL(target)) {
					strPath.Empty();
					return FALSE;
				}
				strPath = target;
			}
		}
		return TRUE;
	}
	return FALSE;
}

void CFolderItem::SetPathInfo()
{
	m_fileattr = 0;
	WIN32_FIND_DATA wfd;
	if (SHGetDataFromIDList(m_psf, m_pidl, SHGDFIL_FINDDATA, &wfd, sizeof(wfd)) == NOERROR) {
		m_fileattr = wfd.dwFileAttributes;
		if (m_fileattr & FAT_RDONLY) m_attr |= SFGAO_READONLY;
		if (m_fileattr & FAT_HIDDEN) m_attr |= SFGAO_HIDDEN;
	}
}

LPCTSTR CFolderItem::GetDisplayName(CString& strName, bool bUseFullyQualPidl)
{
	if (!bUseFullyQualPidl && m_psf)
		idl_GetName(m_psf, m_pidl, SHGDN_INFOLDER, strName);
	else {
		SHFILEINFO sfi;
		SHGetFileInfo((LPCTSTR)m_pidlFq, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME | SHGFI_PIDL);
		strName = sfi.szDisplayName;
	}

#ifdef _DEBUG
#ifdef SHOW_ATTR_VALUE
	strName.AppendFormat(_T(" (%08X)(%02X:%02X)"), m_attr, GetDescriptionId(), *((BYTE*)m_pidl + 2));
#endif
#endif
	return (LPCTSTR)strName;
}

BOOL CFolderItem::ContextMenu(HWND hwnd, LPPOINT ppt)
{
/*
	BOOL bOK = FALSE;
	CComPtr<IContextMenu> pcm = (IContextMenu*)GetUIObjectOf(hwnd, IID_IContextMenu);
	if (pcm.p != NULL) {
		int cmdBack = TrackContextMenu(hwnd, pcm, ppt, m_attr);

		if (cmdBack > 0) {
			CIDListPtr idl;
			if (idl.Clone(m_pidlFq)) {
				switch (cmdBack) {
				case IDM_TAB_NEW:
					// アクティブで開く。firefox, chrome 等がそのような動作
					fvApp.OpenTabFolder(idl, TRUE);
					break;
				case IDM_DIR_ADDFAVORITE:
					fvApp.AddFavorite(idl);
					break;
				default:
					break;
				}
			}
		}
		bOK = (cmdBack >=0);
	}
	return bOK;
*/
	int cmdBack = TrackContextMenuByIDList(hwnd, m_pidlFq, ppt);
	if (cmdBack > 0) {
		CIDListPtr idl;
		if (idl.Clone(m_pidlFq)) {
			switch (cmdBack) {
			case IDM_TAB_NEW:
				// アクティブで開く。firefox, chrome 等がそのような動作
				fvApp.OpenTabFolder(idl, TRUE);
				break;
			case IDM_DIR_ADDFAVORITE:
				fvApp.AddFavorite(idl);
				break;
			default:
				break;
			}
		}
	}
	return (cmdBack >= 0);
}

HDROP CFolderItem::MakeDropFiles()
{
	HDROP hdrop = NULL;
	CComPtr<IDataObject> pdo = (IDataObject*)GetUIObjectOf(NULL, IID_IDataObject);
	if (pdo.p != NULL) {
		hdrop = (HDROP)GetDataFromDataObject(pdo, CF_HDROP);
	}
	return hdrop;
}

LPTSTR CFolderItem::MakeFileNames(LPTSTR pszNames, int cchMax)
{
	CString str;
	if (pszNames && cchMax > 1 && m_fileattr && GetPathName(str)) {
		StrCpyN(pszNames, str, cchMax - 1);
		pszNames[lstrlen(pszNames) + 1] = 0;
		return pszNames;
	}
	return NULL;
}

#ifdef USE_DPA_ARRAY

//////////////////////////////////////////////////////////////////////
// CDpa - DPA_xxx Dynamic Pointer Array Funcstion Wrapper
//

void CDpa::Init()
{
	m_handle = NULL;
}

BOOL CDpa::Create(int delta)
{
	m_handle = _fDPA_Create(delta);
	return (m_handle != NULL);
}

BOOL CDpa::CreateEx(int delta, HANDLE hHeap)
{
	m_handle = _fDPA_CreateEx(delta, hHeap);
	return (m_handle != NULL);
}

BOOL CDpa::Destroy()
{
	BOOL bOK = TRUE;
	if (m_handle) {
		bOK = _fDPA_Destroy(m_handle);
		m_handle = NULL;
	}
	return bOK;
}

#endif			// USE_DPA_ARRAY
