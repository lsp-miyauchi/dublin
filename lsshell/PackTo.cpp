// PackTo.cpp : CPackTo の実装

#include "stdafx.h"
#include "mainmodule.h"
#include "PackTo.h"


// CPackTo

// Initialize
HRESULT CPackTo::Initialize(LPCITEMIDLIST pidlFolder, IDataObject* pdo, HKEY hProgID)
{
	// Initialize can be called more than once
	if (pdo == NULL) return E_INVALIDARG;

	m_userlevel = _check_user();
	if (m_userlevel == USRL_NOUSER) return E_INVALIDARG;

	if (!SHGetPathFromIDList(pidlFolder, m_szPath) || !PathIsDirectory(m_szPath))
		return E_INVALIDARG;

	DWORD arc, etc, img;
	GetShellSetting(arc, etc, img);
	if (!(arc & SHLA_DROPPACK))
		return E_INVALIDARG;

	// Get the data as CF_HDROP
	STGMEDIUM sm;
	FORMATETC fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	if (FAILED(pdo->GetData(&fe, &sm)))
		return E_INVALIDARG;

	m_df.Attach((HDROP)DuplicateMemoryHandle(sm.hGlobal, GHND | GMEM_SHARE));
	ReleaseStgMedium(&sm);

	BOOL bOK = TRUE;
	int type = ARC_ERROR;
	int cnt = m_df.GetCount();
	CString name;
	for (int i = 0; i < cnt; i++) {
		m_df.DragQueryFile(i, name);
		if (PathIsRoot(name) || PathIsUNCServer(name) || PathIsUNCServerShare(name)) {
			bOK = FALSE;
			break;
		}
		type = SpecifyArchiveType(name);
	}

	if (!bOK) return E_INVALIDARG;
	if (cnt == 1 && type != ARC_ERROR) return E_INVALIDARG;

	return NOERROR;
}

// QueryContextMenu
HRESULT CPackTo::QueryContextMenu(HMENU hmenu, UINT index,
		UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	if (!IsValidExecuteModule()) return E_FAIL;
	if (uFlags & CMF_DEFAULTONLY) return E_INVALIDARG;
	if (uFlags == CMF_VERBSONLY) return E_INVALIDARG;

	// イメージリスト
	_AtlModule.InitializeImageList();

	UINT mflg = MF_STRING | MF_BYPOSITION;
	if (m_userlevel == USRL_EXPIRE) mflg |= MF_GRAYED;
	CString str;
	str.LoadString(IDM_CTX_PACKTO);
	InsertMenu(hmenu, index, mflg, idCmdFirst, str);
	SetBitmapToMenuItem(hmenu, idCmdFirst, IDM_CTX_PACKTO);
	return MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_NULL, 1 + 1);
}

// GetCommandString
HRESULT CPackTo::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT *pwReserved, LPSTR pszText, UINT cchMax)
{
	return S_OK;
}

// InvokeCommand
HRESULT CPackTo::InvokeCommand(LPCMINVOKECOMMANDINFO pci)
{
	HRESULT hr = E_INVALIDARG;
	if (HIWORD(pci->lpVerb) == 0) {
		UINT id = (UINT)LOWORD(pci->lpVerb);
		switch (id) {
			case 0:
				CompressFiles(pci->hwnd, m_szPath, m_df);
				break;
			default:
				break;
		}
	}
	return S_OK;
}

