// ThumbOutTo.cpp : CThumbOutTo �̎���

#include "stdafx.h"
#include "mainmodule.h"
#include "ThumbOutTo.h"


// CThumbOutTo

#include "..\lsimg\lsimg.h"

// Initialize
HRESULT CThumbOutTo::Initialize(LPCITEMIDLIST pidlFolder, IDataObject* pdo, HKEY hProgID)
{
	// Initialize can be called more than once
	if (pdo == NULL) return E_INVALIDARG;

	m_userlevel = _check_user();
	if (m_userlevel == USRL_NOUSER) return E_INVALIDARG;

	DWORD arc, etc, img;
	GetShellSetting(arc, etc, img);
	if (!(img & SHLI_DROPTHUMBOUT)) return E_INVALIDARG;

	if (!SHGetPathFromIDList(pidlFolder, m_szPath) || !PathIsDirectory(m_szPath))
		return E_INVALIDARG;

	// Get the data as CF_HDROP
	STGMEDIUM sm;
	FORMATETC fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	if (FAILED(pdo->GetData(&fe, &sm)))
		return E_INVALIDARG;

	m_df.Attach((HDROP)DuplicateMemoryHandle(sm.hGlobal, GHND | GMEM_SHARE));
	ReleaseStgMedium(&sm);

	bool bImageFiles = false;
	WIN32_FILE_ATTRIBUTE_DATA ad;
	CString name;

	int cnt = m_df.GetCount();
	for (int i = 0; i < cnt; i++) {
		m_df.DragQueryFile(i, name);
		if (GetFileAttributesEx(name, GetFileExInfoStandard, &ad) &&
			!(ad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			if (imgTypeIsExif(imgSpecifyImageType(name))) {
				bImageFiles = true;
				break;
			}
		}
	}
	return (bImageFiles) ? NOERROR : E_INVALIDARG;
}

// QueryContextMenu
HRESULT CThumbOutTo::QueryContextMenu(HMENU hmenu, UINT index, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	if (!IsValidExecuteModule()) return E_FAIL;
	if (uFlags & CMF_DEFAULTONLY) return E_INVALIDARG;
	if (uFlags == CMF_VERBSONLY) return E_INVALIDARG;

	UINT mflg = MF_STRING | MF_BYPOSITION;
	if (m_userlevel == USRL_EXPIRE) mflg |= MF_GRAYED;

	UINT idCmd = idCmdFirst;
	CString str;
	str.LoadString(IDS_CTX_DROPTHUMBOUT);		InsertMenu(hmenu, index++, mflg, idCmd++, str);
	return MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_NULL, (idCmd - idCmdFirst) + 1);
}

// GetCommandString
HRESULT CThumbOutTo::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT *pwReserved, LPSTR pszText, UINT cchMax)
{
	return S_OK;
}

// InvokeCommand
HRESULT CThumbOutTo::InvokeCommand(LPCMINVOKECOMMANDINFO pci)
{
	HRESULT hr = E_INVALIDARG;
	if (HIWORD(pci->lpVerb) == 0) {
		UINT id = (UINT)LOWORD(pci->lpVerb);
		if (id == 0) {
			GuidRunDll(pci->hwnd, 0, m_szPath, m_df, DLL_GMIL, _T("RunPickoutThumbnail"));
		}
	}
	return S_OK;
}

