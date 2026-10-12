// LSDecrypt.cpp : CLSDecrypt の実装

#include "stdafx.h"
#include "mainmodule.h"
#include "LSDecrypt.h"


////////////////////////////////////////////////////////////////////////
// CLSDecrypt

HRESULT CLSDecrypt::FinalConstruct()
{
	m_pDataObj = NULL;
	m_path[0] = 0;
	return S_OK;
}

void CLSDecrypt::FinalRelease()
{
	if (m_pDataObj) {
		m_pDataObj->Release();
		m_pDataObj = NULL;
	}
	m_cmdmap.RemoveAll();
}

// IShellExtInit
HRESULT CLSDecrypt::Initialize(LPCITEMIDLIST pidlFolder, IDataObject* pdo, HKEY /*hProgID*/)
{
	// Initialize can be called more than once
	if (m_pDataObj) {
		m_pDataObj->Release();
		m_pDataObj = NULL;
	}

	// Get the data as CF_HDROP
	STGMEDIUM sm;
	FORMATETC fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	if (FAILED(pdo->GetData(&fe, &sm))) return E_INVALIDARG;
	if (!SHGetPathFromIDList(pidlFolder, m_path) || !PathIsDirectory(m_path)) m_path[0] = 0;

	bool unsupp = false;
	int cnt = DragQueryFile((HDROP)sm.hGlobal, 0xFFFFFFFF, NULL, 0);
	TCHAR buf[MAX_PATH * 2];
	for (int i = 0; i < cnt; i++) {
		DragQueryFile((HDROP)sm.hGlobal, i, buf, ARRAYSIZE(buf));

		if (!m_fop.GetCryptFileInfo(buf, NULL, NULL, -1)) {
			unsupp = true;
			break;
		}
	}
	ReleaseStgMedium(&sm);

	if (unsupp) return E_INVALIDARG;

	// duplicate the object pointer and registry handle
	m_pDataObj = pdo;
	m_pDataObj->AddRef();
	return NOERROR;
}

// IContextMenu
HRESULT CLSDecrypt::GetCommandString(UINT_PTR /*idCmd*/, UINT /*uFlags*/, UINT* /*pwReserved*/, LPSTR /*pszText*/, UINT /*cchMax*/)
{
	return S_OK;
}

HRESULT CLSDecrypt::QueryContextMenu(HMENU hmenu, UINT index, UINT idCmdFirst, UINT /*idCmdLast*/, UINT uFlags)
{
	if (uFlags & CMF_DEFAULTONLY)	return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
	if (uFlags == CMF_VERBSONLY)	return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);

	// イメージリスト
	_AtlModule.InitializeImageList();

	UINT idCmd = idCmdFirst;
	m_idCmdFirst = idCmdFirst;

	//
	DWORD arc, etc, img;
	GetShellSetting(arc, etc, img);

	// メニューフラグ
	UINT flags = MF_STRING | MF_BYPOSITION;
	CString str;
	if (m_path[0]) {
		// for DragDropHandler
		if (etc & SHLE_DROPCRYPTTO)
			InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_DECRYPT, IDS_CTX_DECRYPTTO);
	}
	else {
		if (etc & SHLE_CRYPT)
			InsertCommandMenu(hmenu, index++, idCmd++, IDM_CTX_DECRYPT, IDS_CTX_DECRYPT);
	}
	return MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_NULL, idCmd - idCmdFirst);
}

HRESULT CLSDecrypt::InvokeCommand(LPCMINVOKECOMMANDINFO pci)
{
	if (m_pDataObj && HIWORD(pci->lpVerb) == 0) {
		UINT idRealCommand = m_cmdmap.Lookup(LOWORD(pci->lpVerb) + m_idCmdFirst);
		FORMATETC fmte = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM sm;
		if (SUCCEEDED(m_pDataObj->GetData(&fmte, &sm))) {
			int cnt = DragQueryFile((HDROP)sm.hGlobal, 0xFFFFFFFF, 0, 0);
			if (cnt > 0) {
				switch (idRealCommand) {
				case IDM_CTX_DECRYPT:
					m_fop.DecryptFilesTo(pci->hwnd, (HDROP)sm.hGlobal, m_path[0] ? m_path : NULL);
					break;
				}
			}
			ReleaseStgMedium(&sm);
		}
	}
	return S_OK;
}

void CLSDecrypt::InsertCommandMenu(HMENU hmenu, UINT iInsertAfter, UINT idCtxCommand, UINT idRealCommand, UINT idResource)
{
	m_cmdmap.Add(idCtxCommand, idRealCommand);
	CString str; str.LoadString(idResource);
	InsertMenu(hmenu, iInsertAfter, MF_STRING | MF_BYPOSITION, idCtxCommand, str);
	SetBitmapToMenuItem(hmenu, idCtxCommand, idRealCommand);
}

///////////////////////////////////////////////////////////
// IPersistFile
//

HRESULT CLSDecrypt::Load(LPCOLESTR wszFile, DWORD /*dwMode*/)
{
	USES_CONVERSION;
	lstrcpy(m_szFile, OLE2T((WCHAR*)wszFile));

	HRESULT hr = E_FAIL;
	if (m_fop.GetCryptFileInfo(m_szFile, &m_wfd, m_comment.GetBuffer(MAX_PATH * 2), MAX_PATH * 2))
		hr = S_OK;
	else
		m_szFile[0] = 0;
	m_comment.ReleaseBuffer();
	return hr;
};

///////////////////////////////////////////////////////////
// IInitializeWithFile

HRESULT CLSDecrypt::Initialize(LPCWSTR pszFilePath, DWORD grfMode)
{
	return Load(pszFilePath, grfMode);
}

///////////////////////////////////////////////////////////
// IInitializeWithItem

HRESULT CLSDecrypt::Initialize(IShellItem *psi, DWORD grfMode)
{
	LPWSTR pwz = NULL;
	HRESULT hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pwz);
	if (SUCCEEDED(hr)) {
		hr = Load(pwz, grfMode);
		CoTaskMemFree(pwz);
	}
	return hr;
}

////////////////////////////////////////////////////////////////////////
// IQueryInfo

HRESULT CLSDecrypt::GetInfoFlags(LPDWORD pdwFlags)
{
	*pdwFlags = 0;
	return E_NOTIMPL;
}

HRESULT CLSDecrypt::GetInfoTip(DWORD /*flags*/, LPWSTR* ppszTip)
{
	HRESULT hr = E_FAIL;
	if (m_szFile[0] && ppszTip) {
		// 名前
		CString str;
		str.Format(_T("元の名前: %s"), m_wfd.cFileName);
		// 種類
		SHFILEINFO sfi;
		SHGetFileInfo(m_szFile, m_wfd.dwFileAttributes, &sfi, sizeof(sfi), SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME);
		str.AppendFormat(_T("\n種類: %s"), sfi.szTypeName);
		// サイズ
		TCHAR buf[128];
		StrFormatByteSize(m_wfd.nFileSizeLow, buf, ARRAYSIZE(buf));
		str.AppendFormat(_T("\nサイズ: %s"), buf);
		// 日付
		FILETIME ft;
		FileTimeToLocalFileTime(&m_wfd.ftLastWriteTime, &ft);
		SYSTEMTIME st;
		FileTimeToSystemTime(&ft, &st);
		str.AppendFormat(_T("\n更新日時: %04d/%02d/%02d %02d:%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);

		*ppszTip = (LPTSTR)CoTaskMemAlloc((str.GetLength() + 1) * sizeof(TCHAR));
		if (*ppszTip) {
			lstrcpy(*ppszTip, str.GetString());
			hr = S_OK;
		}
	}
	return hr;
}

