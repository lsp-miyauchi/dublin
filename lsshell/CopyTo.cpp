// CopyTo.cpp : CCopyTo の実装

#include "stdafx.h"
#include "mainmodule.h"
#include "CopyTo.h"
#include "common.h"


// CCopyTo

// Initialize
HRESULT CCopyTo::Initialize(LPCITEMIDLIST pidlFolder, IDataObject* pdo, HKEY hProgID)
{
	// Initialize can be called more than once
	if (pdo == NULL) return E_INVALIDARG;

	m_userlevel = _check_user();
	if (m_userlevel == USRL_NOUSER) return E_INVALIDARG;

	if (!SHGetPathFromIDList(pidlFolder, m_szPath) || !PathIsDirectory(m_szPath))
		return E_INVALIDARG;

	DWORD arc, img;
	GetShellSetting(arc, m_operation, img);
	if (!(m_operation & (SHLE_DROPCOPYTO | SHLE_DROPCRYPTTO | SHLE_DROPICONPICK))) return E_INVALIDARG;

	// Get the data as CF_HDROP
	STGMEDIUM sm;
	FORMATETC fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	if (FAILED(pdo->GetData(&fe, &sm)))
		return E_INVALIDARG;

	m_df.Attach((HDROP)DuplicateMemoryHandle(sm.hGlobal, GHND | GMEM_SHARE));
	ReleaseStgMedium(&sm);

	return (m_df.GetCount() > 0) ? NOERROR : E_INVALIDARG;
}

void CCopyTo::InsertCommandMenu(HMENU hmenu, UINT iInsertAfter, UINT idCommand, UINT idResource, UINT flags)
{
	CString str; str.LoadString(idResource);
	InsertMenu(hmenu, iInsertAfter, flags, idCommand, str);
	m_cmdmap.Add(idCommand, idResource);
	SetBitmapToMenuItem(hmenu, idCommand, idResource);
}

// QueryContextMenu
HRESULT CCopyTo::QueryContextMenu(HMENU hmenu, UINT index, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	if (!IsValidExecuteModule()) return E_FAIL;
	if (uFlags & CMF_DEFAULTONLY) return E_INVALIDARG;
	if (uFlags == CMF_VERBSONLY) return E_INVALIDARG;

	// イメージリスト
	_AtlModule.InitializeImageList();

	UINT mflg = MF_STRING | MF_BYPOSITION;
	if (m_userlevel == USRL_EXPIRE) mflg |= MF_GRAYED;

	UINT idCmd = idCmdFirst;
	m_idCmdFirst = idCmdFirst;

	if (m_operation & SHLE_DROPCOPYTO) {
		if (GetKeyState(VK_SHIFT) >= 0) {
			InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_COPYTO, mflg);
//@			InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_COPYTO_FLAT, mflg);
		}
		else {
			InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_MOVETO, mflg);
//@			InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_MOVETO_FLAT, mflg);
		}
	}
	if (m_operation & SHLE_DROPCRYPTTO) {
		InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_ENCRYPTTO, mflg);
	}
	if (m_operation & SHLE_DROPICONPICK) {
		if (m_df.GetCount() == 1) {
			TCHAR buf[MAX_PATH * 2];
			m_df.DragQueryFile(0, buf, ARRAYSIZE(buf));
			LPCTSTR pext = PathFindExtension(buf);
			if (!StrCmpI(pext, _T(".exe")) || !StrCmpI(pext, _T(".dll")) || !StrCmpI(pext, _T(".cpl")) || !StrCmpI(pext, _T(".ico"))) {
				InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_ICONPICKTO, mflg);
			}
		}
	}
	return MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_NULL, (idCmd - idCmdFirst) + 1);
}

// GetCommandString
HRESULT CCopyTo::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT *pwReserved, LPSTR pszText, UINT cchMax)
{
	HRESULT hr = E_INVALIDARG;
	switch (uFlags) {
	case GCS_VALIDATEA:
	case GCS_VALIDATEW:
		hr = S_OK;
		break;
	}
	return hr;
}

// InvokeCommand
HRESULT CCopyTo::InvokeCommand(LPCMINVOKECOMMANDINFO pci)
{
	HRESULT hr = E_INVALIDARG;
	if (HIWORD(pci->lpVerb) == 0) {
		UINT id = m_cmdmap.Lookup(LOWORD(pci->lpVerb) + m_idCmdFirst);
		switch (id) {
		case IDM_CTX_COPYTO:
		case IDM_CTX_MOVETO:
		case IDM_CTX_COPYTO_FLAT:
		case IDM_CTX_MOVETO_FLAT:
			{
			SHFILEINFO sfi;
			SHGetFileInfo(m_szPath, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME);
			CString str;
			bool bCopy = true;
			bool bFlat = false;
			switch(id) {
			case IDM_CTX_COPYTO:		str.LoadString(IDS_QUERYCOPY); break;
			case IDM_CTX_COPYTO_FLAT:	str.LoadString(IDS_QUERYCOPY_FLAT); bFlat = true; break;
			case IDM_CTX_MOVETO:		str.LoadString(IDS_QUERYMOVE); bCopy = false; break;
			case IDM_CTX_MOVETO_FLAT:	str.LoadString(IDS_QUERYMOVE_FLAT); bCopy = false; bFlat = true; break;
			}
			CString msg;
			msg.Format(str, sfi.szDisplayName);
			str.LoadString(IDS_COPYCAPTION);
			HWND hwnd = ::IsWindow(pci->hwnd) ? pci->hwnd : ::GetActiveWindow();
			if (MessageBox(hwnd, msg, str, MB_OKCANCEL | MB_ICONINFORMATION) == IDOK) {
				CFopLib fop;

				DROPFILEOPERATION dfo;
				dfo.cbSize = sizeof(dfo);
				dfo.hwnd = hwnd;
				dfo.hDrop = m_df;
				dfo.dwEffect = bCopy ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
				dfo.pszDropPath = m_szPath;
				dfo.bAsAdmin = FALSE;
				dfo.bAsyncOperation = FALSE;
				dfo.bSaveFolderHierarchy = (bFlat == false);
				fop.DropFileOperation(&dfo);
			}
			break;
			}
		case IDM_CTX_ENCRYPTTO:
			{
			CFopLib fop;
			fop.EncryptFilesTo(pci->hwnd, m_df, m_szPath);
			break;
			}
		case IDM_CTX_ICONPICKTO:
			{
			TCHAR buf[MAX_PATH * 2];
			m_df.DragQueryFile(0, buf, ARRAYSIZE(buf));
			CFopLib fop;
			fop.ExportIconFromFile(pci->hwnd, buf, m_szPath);
			break;
			}
		}
	}
	return S_OK;
}

