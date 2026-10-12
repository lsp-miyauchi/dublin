// AddPack.cpp : CAddPack �̎���

#include "stdafx.h"
#include "AddPack.h"
#include "common.h"


// CAddPack

HRESULT CAddPack::Load(LPCOLESTR pszFileName, DWORD grfMode)
{
	m_userlevel = _check_user();
	if (m_userlevel == USRL_NOUSER) return E_INVALIDARG;

	lstrcpy(m_szFile, pszFileName);
	int type = SpecifyArchiveType(pszFileName);
	switch (type) {
	case ARC_LZH:
	case ARC_ZIP:
	case ARC_7ZIP:
	case ARC_BGA:
		return NOERROR;
	}
	return E_FAIL;
}

/////////////////////////////////////////////////////////////////////////////
// Methods of IDropTarget

HRESULT CAddPack::DragEnter(LPDATAOBJECT pdo, DWORD keyState, POINTL pt, LPDWORD pdwEffect)
{
	STGMEDIUM sm;
	FORMATETC fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	if (FAILED(pdo->GetData(&fe, &sm))) {
		*pdwEffect = DROPEFFECT_NONE;
		return E_INVALIDARG;
	}

	BOOL bOK = TRUE;
	int cnt = DragQueryFile((HDROP)sm.hGlobal, 0xFFFFFFFF, NULL, 0);
	TCHAR buf[MAX_PATH * 2];
	for (int i = 0; i < cnt; i++) {
		DragQueryFile((HDROP)sm.hGlobal, i, buf, ARRAYSIZE(buf));
		if (PathIsRoot(buf) || PathIsUNCServer(buf) || PathIsUNCServerShare(buf)) {
			bOK = FALSE;
			break;
		}
	}
	ReleaseStgMedium(&sm);

	if (!bOK) {
		*pdwEffect = DROPEFFECT_NONE;
		return E_INVALIDARG;
	}

	return DragOver(keyState, pt, pdwEffect);
}

HRESULT CAddPack::DragOver(DWORD keyState, POINTL pt, LPDWORD pdwEffect)
{
	*pdwEffect = (m_userlevel != USRL_EXPIRE) ? DROPEFFECT_COPY : DROPEFFECT_NONE;
	return S_OK;
}

HRESULT CAddPack::DragLeave()
{
	return S_OK;
}

HRESULT CAddPack::Drop(LPDATAOBJECT pdo, DWORD keyState, POINTL pt, LPDWORD pdwEffect)
{
	int res = AppMessage(IDS_ADDPACK, IDS_QUERY_ADDPACK, NULL, MB_QYESNO);
	if (res == IDYES) {
		FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM sm;
		if (SUCCEEDED(pdo->GetData(&fe, &sm))) {
			HWND hwnd = GetActiveWindow();
			TCHAR buf[MAX_PATH * 2];
			DragQueryFile((HDROP)sm.hGlobal, 0, buf, ARRAYSIZE(buf));
			PathRemoveFileSpec(buf);
			PackAddFiles(hwnd, m_szFile, buf, (HDROP)sm.hGlobal);
			ReleaseStgMedium(&sm);
		}
	}
	*pdwEffect = DROPEFFECT_NONE;
	return DragLeave();
}
