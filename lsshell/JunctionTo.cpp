// JunctionTo.cpp : CJunctionTo の実装

#include "stdafx.h"
#include "mainmodule.h"
#include "JunctionTo.h"
#include "..\lsfop\lsfop_i.h"

static DWORD GetVolumeSysFlag(int driveNumber)
{
	TCHAR buf[MAX_PATH];
	PathBuildRoot(buf, driveNumber);
	DWORD flen, sflag;
	return GetVolumeInformation(buf, NULL, 0, NULL, &flen, &sflag, NULL, 0) ? sflag : 0;
}

static bool CheckVolumeSysFlag(int driveNumber, DWORD flag)
{
	return (GetVolumeSysFlag(driveNumber) & flag) != 0;
}

static BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	BOOL bOK = FALSE;
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		LUID luid;
		if (LookupPrivilegeValue(NULL, lpszPrivilege, &luid)) {
			TOKEN_PRIVILEGES tp;
			tp.PrivilegeCount = 1;
			tp.Privileges[0].Luid = luid;
			if (bEnablePrivilege)
				tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			else
				tp.Privileges[0].Attributes = 0;

			// Enable the privilege or disable all privileges.
			bOK = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL);
			if (bOK) {
				if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
					// The token does not have the specified privilege.
					bOK = FALSE;
				}
			}
		}
		CloseHandle(hToken);
	}
	return bOK;
}

// CJunctionTo

// Initialize
HRESULT CJunctionTo::Initialize(LPCITEMIDLIST pidlFolder, IDataObject* pdo, HKEY hProgID)
{
	// Initialize can be called more than once
	m_commands.RemoveAll();
	m_strName.Empty();
	if (pdo == NULL) return E_INVALIDARG;

	m_userlevel = _check_user();
	if (m_userlevel == USRL_NOUSER) return E_INVALIDARG;

	// オプションでOFF？
	DWORD arc, etc, img;
	GetShellSetting(arc, etc, img);
	if (etc & (SHLE_DROPREPARSE | SHLE_DROPSYMLINK)) {
		// リパース or シンボリックなら、システムリンクフラグを立てる（旧バージョン補完）
		etc |= SHLE_DROPSYSLINK;
	}
	if (!(etc & (SHLE_DROPREPARSE | SHLE_DROPSYMLINK | SHLE_DROPSYSLINK))) return E_INVALIDARG;

	// Get the data as CF_HDROP
	STGMEDIUM sm;
	FORMATETC fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	if (FAILED(pdo->GetData(&fe, &sm))) return E_INVALIDARG;
	// 複数個のファイル・フォルダは対応外
	m_df.Attach((HDROP)DuplicateMemoryHandle(sm.hGlobal, GHND | GMEM_SHARE));
	ReleaseStgMedium(&sm);
	if (m_df.GetCount() != 1) return E_INVALIDARG;
	// ドロップされるファイル・パス
	CString dropFile;
	if (m_df.DragQueryFile(0, dropFile) < 1) return E_INVALIDARG;
	// 既にjunction？
	WIN32_FILE_ATTRIBUTE_DATA ad;
	if (!GetFileAttributesEx(dropFile, GetFileExInfoStandard, &ad) ||
		(ad.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
		return E_INVALIDARG;
	// ローカルパス・ファイル？
	TCHAR buf[MAX_PATH * 2];
	if (!SHGetPathFromIDList(pidlFolder, buf) || buf[1] != ':') return E_INVALIDARG;

	// ドロップ先のフォルダに、その上位の親のフォルダ・ファイルをドロップしようとしていないかをチェック
	m_strName = buf;
	DWORD vsflg = GetVolumeSysFlag(PathGetDriveNumber(m_strName));
/*
	if (PathIsDirectory(dropFile)) {
		// このボリュームがリパースポイントをサポートしているか確認
		if ((etc & SHLE_DROPREPARSE) && (vsflg & FILE_SUPPORTS_REPARSE_POINTS)) {
			// 親子関係
			if (PathComparePath(dropFile, m_strName) < 0) {
				m_commands.Add(IDM_CTX_REPARSEPOINT);
			}
		}
	}
	if ((etc & SHLE_DROPSYMLINK) && PathComparePath(dropFile, m_strName) != 0 && _AtlModule.OsVISTA()) {
		m_commands.Add(IDM_CTX_MKSYMLINK);
	}
*/
	if ((etc & SHLE_DROPSYSLINK) && PathComparePath(dropFile, m_strName) != 0) {
		m_commands.Add(IDM_CTX_MKSYSLINK);
	}
	return (m_commands.GetSize() > 0) ? NOERROR : E_INVALIDARG;
}

// QueryContextMenu
HRESULT CJunctionTo::QueryContextMenu(HMENU hmenu, UINT index, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	if (!IsValidExecuteModule()) return E_FAIL;
	if (uFlags & CMF_DEFAULTONLY) return E_INVALIDARG;
	if (uFlags == CMF_VERBSONLY) return E_INVALIDARG;

	// イメージリスト
	_AtlModule.InitializeImageList();

	UINT mflg = MF_STRING | MF_BYPOSITION;
	if (m_userlevel == USRL_EXPIRE) mflg |= MF_GRAYED;

	CString str;
	int numCommands = m_commands.GetSize();
	for (int i = 0; i < numCommands; i++) {
		UINT idResource = m_commands[i];
		str.LoadString(idResource);
		InsertMenu(hmenu, index, mflg, idCmdFirst, str);
		UINT id = (idResource == IDM_CTX_MKSYMLINK && !_AtlModule.IsAdmin()) ? PtrToUint(IDI_SHIELD) : idResource;
		SetBitmapToMenuItem(hmenu, idCmdFirst, id);
		index++;
		idCmdFirst++;
	}
	return MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_NULL, numCommands + 1);
}

// GetCommandString
HRESULT CJunctionTo::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT *pwReserved, LPSTR pszText, UINT cchMax)
{
	return S_OK;
}

// InvokeCommand
HRESULT CJunctionTo::InvokeCommand(LPCMINVOKECOMMANDINFO pci)
{
	HRESULT hr = E_INVALIDARG;
	if (HIWORD(pci->lpVerb) == 0) {
		CFopLib fop;
		CString name;
		m_df.DragQueryFile(0, name);
		switch (m_commands[LOWORD(pci->lpVerb)]) {
		case IDM_CTX_REPARSEPOINT:
			fop.CreateJunctionUI(m_strName, name);
			break;
		case IDM_CTX_MKSYMLINK:
			if (_AtlModule.IsAdmin() || SetPrivilege(SE_CREATE_SYMBOLIC_LINK_NAME, TRUE))
				fop.CreateSymbolicLinkUI(pci->hwnd, m_strName, name);
			else {
				ILSFileOperation* pfo;
				if (CoCreateInstanceAsAdmin(pci->hwnd, true, __uuidof(LSFileOperation), __uuidof(ILSFileOperation), (void**)&pfo) == S_OK) {
					pfo->MakeSymLinkUI(pci->hwnd, m_strName, name);
					pfo->Release();
				}
			}
			break;
		case IDM_CTX_MKSYSLINK:
			fop.CreateSystemLinkUI(pci->hwnd, m_strName, name);
			break;
		default:
			break;
		}
	}
	return S_OK;
}

