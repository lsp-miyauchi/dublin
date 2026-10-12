// UnpackTo.cpp : CUnpackTo の実装

#include "stdafx.h"
#include "mainmodule.h"
#include "UnpackTo.h"


// CUnpackTo

#define SDS_UNPACKTO			0x00000001
#define SDS_UNPACKFOLDERTO		0x00000002
#define SDS_MAKESFXTO			0x00000004

HRESULT CUnpackTo::Initialize(LPCITEMIDLIST pidlFolder, IDataObject* pdo, HKEY hProgID)
{
	// Initialize can be called more than once
	if (pdo == NULL) return E_INVALIDARG;

	m_flags = 0;
	m_userlevel = _check_user();
	if (m_userlevel == USRL_NOUSER) return E_INVALIDARG;

	if (!SHGetPathFromIDList(pidlFolder, m_szPath)) return E_INVALIDARG;

	DWORD arc, etc, img;
	GetShellSetting(arc, etc, img);
	if (!(arc & SHLA_DROPUNPACK)) return E_INVALIDARG;

	// Get the data as CF_HDROP
	STGMEDIUM sm;
	FORMATETC fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	if (FAILED(pdo->GetData(&fe, &sm))) return E_INVALIDARG;

	m_df.Attach((HDROP)DuplicateMemoryHandle(sm.hGlobal, GHND | GMEM_SHARE));
	ReleaseStgMedium(&sm);

	BOOL bOK = TRUE;
	int type = ARC_ERROR;
	int cnt = m_df.GetCount();
	CString name;
	for (int i = 0; i < cnt; i++) {
		m_df.DragQueryFile(i, name);
		lstrcpy(m_szFile, name);

		if (PathIsRoot(name) || PathIsDirectory(name)) {
			bOK = FALSE;
			break;
		}

		type = SpecifyArchiveType(name);
		if (type == ARC_ERROR) {
			bOK = FALSE;
			break;
		}
	}

	if (!bOK) return E_INVALIDARG;

	m_flags = SDS_UNPACKTO;
	if (cnt == 1) {
		m_flags |= SDS_UNPACKFOLDERTO;
		if (type == ARC_LZH || type == ARC_CAB || type == ARC_BGA)
			m_flags |= SDS_MAKESFXTO;
	}
	return NOERROR;
}

// QueryContextMenu
HRESULT CUnpackTo::QueryContextMenu(HMENU hmenu, UINT index, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	if (!IsValidExecuteModule()) return E_FAIL;
	if (uFlags & CMF_DEFAULTONLY) return E_INVALIDARG;
	if (uFlags == CMF_VERBSONLY) return E_INVALIDARG;
	if (!m_flags) return MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_NULL, 0);

	// イメージリスト
	_AtlModule.InitializeImageList();

	UINT mflg = MF_STRING | MF_BYPOSITION;
	if (m_userlevel == USRL_EXPIRE) mflg |= MF_GRAYED;

	CString str;
	UINT idResource;
	UINT idCmd = idCmdFirst;
	m_idCmdFirst = idCmdFirst;
	if (m_flags & SDS_UNPACKTO) {
		idResource = IDM_CTX_UNPACKTO;
		str.LoadString(idResource);
		InsertMenu(hmenu, index++, mflg, idCmd, str);
		SetBitmapToMenuItem(hmenu, idCmd, idResource);
		m_arrCommand.Add(idCmd++, idResource);
	}
	if (m_flags & SDS_UNPACKFOLDERTO) {
		idResource = IDM_CTX_UNPACKTOFOLDER;
		str.LoadString(idResource);
		TCHAR buf[MAX_PATH * 2];
		lstrcpy(buf, PathFindFileName(m_szFile));
		PathRemoveExtension(buf);
		str.Replace(_T("%s"), buf);
		InsertMenu(hmenu, index++, mflg, idCmd, str);
		SetBitmapToMenuItem(hmenu, idCmd, IDM_CTX_UNPACKTO);
		m_arrCommand.Add(idCmd++, idResource);
	}
	if (m_flags & SDS_MAKESFXTO) {
		idResource = IDM_CTX_MAKESFXTO;
		str.LoadString(idResource);
		InsertMenu(hmenu, index++, mflg, idCmd, str);
		SetBitmapToMenuItem(hmenu, idCmd, idResource);
		m_arrCommand.Add(idCmd++, idResource);
	}
	return MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_NULL, (idCmd - idCmdFirst) + 1);
}

// GetCommandString
HRESULT CUnpackTo::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT *pwReserved, LPSTR pszText, UINT cchMax)
{
	return S_OK;
}

// InvokeCommand
HRESULT CUnpackTo::InvokeCommand(LPCMINVOKECOMMANDINFO pci)
{
	HRESULT hr = E_INVALIDARG;
	if (HIWORD(pci->lpVerb) == 0) {
		TCHAR buf[MAX_PATH * 2];
		UINT id = m_arrCommand.Lookup(LOWORD(pci->lpVerb) + m_idCmdFirst);
		switch (id) {
		case IDM_CTX_UNPACKTO:
			ExtractFiles(pci->hwnd, m_szPath, m_df);
			break;
		case IDM_CTX_UNPACKTOFOLDER:
		{
			TCHAR tit[MAX_PATH * 2];
			lstrcpy(tit, PathFindFileName(m_szFile));
			PathRemoveExtension(tit);
			lstrcpy(buf, m_szPath);
			PathAddBackslash(buf);
			lstrcat(buf, tit);
			ExtractFiles(pci->hwnd, buf, m_df);
			break;
		}
		case IDM_CTX_MAKESFXTO:
			m_df.DragQueryFile(0, buf, ARRAYSIZE(buf));
			MakeSFX(pci->hwnd, buf, m_szPath);
			break;
		}
		hr = S_OK;
	}
	return hr;
}

