//////////////////////////////////////////////////////////////////////
// reparse.cpp

#include "stdafx.h"
#include "fopstd.h"
#include "reparse.h"

//--------------------------------------------------------------------
//
// CreateJunction
//
// This routine creates a NTFS junction, using the undocumented
// FSCTL_SET_REPARSE_POINT structure Win2K uses for mount points
// and junctions.
//
//--------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////
//

class CMakeJunctionDlg : public CDialogImpl<CMakeJunctionDlg>
{
public:
	CMakeJunctionDlg() { m_src.Empty(); m_base.Empty(); m_path.Empty(); m_checkloop = TRUE; }
	~CMakeJunctionDlg() { }

	enum { IDD = IDD_MAKEJUNCTION };

BEGIN_MSG_MAP(CMakeJunctionDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
	ON_EN_CHANGE(IDC_EDIT, OnEditChange)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnEditChange();
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnHelpCall() { ShowHtmlHelp(m_hWnd, IDD); }

	void BuildPath();
	BOOL CheckTree(LPCTSTR path, CProgressDialog& progress);

	CString m_src;
	CString m_base;
	CString m_path;
	CString m_found;
	BOOL m_checkloop;
};

BOOL CMakeJunctionDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	if (m_base.IsEmpty() || m_src.IsEmpty())
		EndDialog(IDCANCEL);
	else {
		PathSetDlgItemPath(m_hWnd, IDC_TITLE, m_src);
		// 入力規則
		idl_LimitInputEdit(GetDlgItem(IDC_EDIT), m_src);

		TCHAR name[MAX_PATH * 2];
		_pathaddsuffix(PathFindFileName(m_src), _T("(junction)"), name, ARRAYSIZE(name));
		SetDlgItemText(IDC_EDIT, name);

		CheckDlgButton(IDC_CHECK, m_checkloop);
	}
	return TRUE;
}

void CMakeJunctionDlg::BuildPath()
{
	TCHAR buf[MAX_PATH * 2], path[MAX_PATH * 2];
	lstrcpy(path, m_base);
	if (GetDlgItemText(IDC_EDIT, buf, ARRAYSIZE(buf)) > 0) {
		PathAppend(path, buf);
	}
	m_path = path;
	SetDlgItemText(IDC_PATH, m_path);
}

void CMakeJunctionDlg::OnEditChange()
{
	BuildPath();
}

BOOL CMakeJunctionDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW) {
		ShowHtmlHelp(m_hWnd, IDD);
		return TRUE;
	}
	return FALSE;
}

BOOL CMakeJunctionDlg::CheckTree(LPCTSTR path, CProgressDialog& progress)
{
	if (progress.HasUserCancelled()) {
		m_found.Empty();
		return FALSE;
	}
	progress.SetLine(2, path, TRUE);

	TCHAR buf[MAX_PATH * 2];
	CFileFinder ff;
	if (ff.BeginFind(path, gszAst)) {
		do {
			if (ff.IsFolder() && !ff.IsDot()) {
				PathCombine(buf, path, ff.cFileName);
				if (ff.IsAttrib(FA_REPARSE)) {
					m_found = buf;
					ff.EndFind();
					return FALSE;
				}
				if (!CheckTree(buf, progress)) {
					ff.EndFind();
					return FALSE;
				}
			}
		} while (ff.NextFind());
		ff.EndFind();
	}
	return TRUE;
}

void CMakeJunctionDlg::OnOK()
{
	if (m_path.IsEmpty() || !_pathcompare(m_base, m_path)) {
		MessageBeep(0);
		GetDlgItem(IDC_EDIT).SetFocus();
		return;
	}

	// ジャンクションが参照元の子孫ならば中止
	if (!_pathisparent(m_src, m_path)) {
		AppMessage(IDS_TITLE_REPARSE, IDS_PATH_INCLUDE);
		GetDlgItem(IDC_EDIT).SetFocus();
		return;
	}

	// フォルダ名不適格文字のチェック
	LPTSTR p = PathFindFileName(m_path);

	// 文字列終端が ' ' または '.' で終わるフォルダ名の修正
	if (!validate_makefile(p)) {
		AppMessage(IDS_TITLE_REPARSE, IDS_INVALIDFILENAME, p);
		return;
	}

	if (_pathfileexists(m_path)) {
 		AppMessage(IDS_TITLE_REPARSE, IDS_SAMENAME, m_path, MB_STOP);
		return;
	}

	if (IsDlgButtonChecked(IDC_CHECK)) {
		GetDlgItem(IDOK).EnableWindow(FALSE);
		GetDlgItem(IDCANCEL).EnableWindow(FALSE);
		GetDlgItem(IDC_HELPCALL).EnableWindow(FALSE);

		CProgressDialog progress;
		progress.SetTitle(IDS_TITLE_REPARSE);
		progress.SetMessage(MAKEINTRESOURCE(IDS_DUPLICATE_CONFIRMING));
		progress.SetOperation(SPACTION_SEARCHING_FILES);
		progress.StartProgressDialog(m_hWnd, PROGDLG_NOTIME | PROGDLG_NOMINIMIZE | PROGDLG_NOPROGRESSBAR);

		m_found.Empty();
		CWaitCursor wc;
		BOOL bFound = (!CheckTree(m_src, progress) && !m_found.IsEmpty());
		wc.Restore();

		progress.StopProgressDialog();

		GetDlgItem(IDOK).EnableWindow(TRUE);
		GetDlgItem(IDCANCEL).EnableWindow(TRUE);
		GetDlgItem(IDC_HELPCALL).EnableWindow(TRUE);

		if (bFound) {
			// user canceled
			if (m_found.IsEmpty()) return;

 			if (AppMessage(IDS_TITLE_REPARSE, IDS_FOUND_JUNCTION, m_found, MB_QYESNO) != IDYES)
				return;
		}
	}
	EndDialog(IDOK);
}

////////////////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI fopCreateJunction(LPCTSTR pszLink, LPCTSTR pszSrc)
{
	WCHAR		dir[MAX_PATH * 2];
	WCHAR		name[MAX_PATH * 2];
    WCHAR       native[MAX_PATH * 2];
	PTCHAR		fpart;

	// Get the full path referenced by the target
	if (!GetFullPathName(pszSrc, ARRAYSIZE(name), name, &fpart)) {
		LastError(IDS_TITLE_REPARSE, pszSrc);
		return FALSE;
	}

	// Get the full path referenced by the directory
	if (!GetFullPathName(pszLink, ARRAYSIZE(dir), dir, &fpart)) {
		LastError(IDS_TITLE_REPARSE, pszLink);
		return FALSE;
	}

	// Make the native target name
	wsprintf(native, _T("\\??\\%s"), name);
	DWORD len = (DWORD)lstrlen(native);
	if ((native[len - 1] == '\\') && (native[len - 2] != ':'))
		native[len - 1] = 0;

	// Create the link - ignore errors since it might already exist
	CreateDirectory(pszLink, NULL);
	HANDLE hFile = CreateFile(pszLink, GENERIC_WRITE, 0,
						NULL, OPEN_EXISTING, 
						FILE_FLAG_OPEN_REPARSE_POINT|FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		LastError(IDS_TITLE_REPARSE, pszLink);
		return FALSE;
	}

	// Build the reparse info
	PREPARSE_MOUNTPOINT_DATA_BUFFER info = (PREPARSE_MOUNTPOINT_DATA_BUFFER)malloc(MAX_PATH * 3);
	memset(info, 0, MAX_PATH * 3);
	info->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
	info->ReparseTargetLength = (WORD)(lstrlen(native) * sizeof(WCHAR));
	info->ReparseTargetMaximumLength = info->ReparseTargetLength + sizeof(WCHAR);
	lstrcpy(info->ReparseTarget, native);
	info->ReparseDataLength = info->ReparseTargetLength + 12;

	// Set the link
	BOOL bOK = DeviceIoControl(hFile, FSCTL_SET_REPARSE_POINT,
				info, info->ReparseDataLength + REPARSE_MOUNTPOINT_HEADER_SIZE,
				NULL, 0, &len, NULL);
	CloseHandle(hFile);

	if (!bOK) {
		RemoveDirectory(pszLink);
		LastError(IDS_TITLE_REPARSE, pszLink);
	}
	free(info);
	return bOK;
}

////////////////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI fopCreateJunctionUI(LPCTSTR pszLinkBase, LPCTSTR pszSrc)
{
	if (!pszLinkBase || *pszLinkBase == 0 || !_pathisdirectory(pszLinkBase) ||
		!pszSrc || *pszSrc == 0 || !_pathisdirectory(pszSrc))
		return FALSE;

	CMakeJunctionDlg dlg;
	dlg.m_base = pszLinkBase;
	dlg.m_src = pszSrc;
	if (dlg.DoModal() == IDOK)
		return fopCreateJunction(dlg.m_path, pszSrc);
	return FALSE;
}

extern "C"
DWORD WINAPI fopGetJunctionPointInfo(LPCTSTR pszMountDir, LPTSTR pszDestBuff, int cchMax)
{
	const DWORD attr = GetFileAttributes(pszMountDir);
	if (attr == 0xffffffff)
		return 0;
	if (!(attr & FILE_ATTRIBUTE_REPARSE_POINT)) {
		SetLastError(ERROR_BAD_FILE_TYPE);
		return 0;
	}

	CReparseDir rdir(pszMountDir);
	if (!rdir.IsValid())
		return 0;

	TCHAR buf[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
	REPARSE_DATA_BUFFER& rdb = *(REPARSE_DATA_BUFFER*)buf;

	DWORD numBytes;
	if (!DeviceIoControl(rdir, FSCTL_GET_REPARSE_POINT, NULL, 0, (LPVOID)&rdb, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &numBytes, 0) ||
		(IO_REPARSE_TAG_MOUNT_POINT != rdb.ReparseTag)) {
		return 0;
	}

	if ((unsigned)cchMax < rdb.MountPointReparseBuffer.SubstituteNameLength / sizeof(TCHAR) + sizeof(TCHAR)) {
		return rdb.MountPointReparseBuffer.SubstituteNameLength / sizeof(TCHAR) + sizeof(TCHAR);
	}

	lstrcpy(pszDestBuff, rdb.MountPointReparseBuffer.PathBuffer);
	return rdb.MountPointReparseBuffer.SubstituteNameLength / sizeof(TCHAR);
}

extern "C"
DWORD WINAPI fopGetReparsePointInfo(LPCTSTR pszReparsePointFile, LPTSTR* ppszOriginalFile, DWORD* pdwTagType)
{
	if (!pszReparsePointFile || !ppszOriginalFile) {
		SetLastError(ERROR_BAD_ARGUMENTS);
		return ERROR_BAD_ARGUMENTS;
	}

	const DWORD attr = GetFileAttributes(pszReparsePointFile);
	if (attr == 0xffffffff)
		return GetLastError();
	if (!(attr & FILE_ATTRIBUTE_REPARSE_POINT)) {
		SetLastError(ERROR_BAD_FILE_TYPE);
		return ERROR_BAD_FILE_TYPE;
	}


	CReparseDir rdir(pszReparsePointFile);
	if (!rdir.IsValid())
		return GetLastError();

	TCHAR buf[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
	REPARSE_DATA_BUFFER& rdb = *(REPARSE_DATA_BUFFER*)buf;

	DWORD numBytes;
	if (!DeviceIoControl(rdir, FSCTL_GET_REPARSE_POINT, NULL, 0, (LPVOID)&rdb, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &numBytes, 0))
		return GetLastError();

	int len;
	LPCWSTR p;
	switch (rdb.ReparseTag) {
	case IO_REPARSE_TAG_MOUNT_POINT:
		len = rdb.MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR) + sizeof(WCHAR);
		*ppszOriginalFile = (LPTSTR)CoTaskMemAlloc(len * sizeof(WCHAR));
		if (!*ppszOriginalFile) {
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return ERROR_NOT_ENOUGH_MEMORY;
		}
		p = rdb.MountPointReparseBuffer.PathBuffer + rdb.MountPointReparseBuffer.SubstituteNameOffset / sizeof(WCHAR);
		p = StrStr(p, _T("\\??\\"));
		if (p) p += 4;
		lstrcpy(*ppszOriginalFile, p);
		break;
	case IO_REPARSE_TAG_SYMLINK:
		len = rdb.SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(WCHAR) + sizeof(WCHAR);
		*ppszOriginalFile = (LPTSTR)CoTaskMemAlloc(len * sizeof(WCHAR));
		if (!*ppszOriginalFile) {
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return ERROR_NOT_ENOUGH_MEMORY;
		}
		// rdb.SymbolicLinkReparseBuffer.Flags が SYMLINK_FLAG_RELATIVE なら相対リンクなので
		// そのときは、さてどうしよう...
		p = rdb.SymbolicLinkReparseBuffer.PathBuffer + rdb.SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(WCHAR);
		p = StrStr(p, _T("\\??\\"));
		if (p) p += 4;
		lstrcpy(*ppszOriginalFile, p);
		break;
	default:
		len = rdb.ReparseDataLength * sizeof(TCHAR);
		*ppszOriginalFile = (LPTSTR)CoTaskMemAlloc(len * sizeof(WCHAR));
		if (!*ppszOriginalFile) {
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return ERROR_NOT_ENOUGH_MEMORY;
		}
		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)rdb.GenericReparseBuffer.DataBuffer, -1, *ppszOriginalFile, len);
		break;
	}
	if (pdwTagType)
		*pdwTagType = rdb.ReparseTag;
	SetLastError(NOERROR);
	return NOERROR;
}

BOOL GetReparseTarget(LPCTSTR pszName, CString& str)
{
	BOOL ok = FALSE;
	str.Empty();
	LPITEMIDLIST pidlSrc = ILCreateFromPath(pszName);
	if (pidlSrc) {
		IShellFolder* psf;
		LPCITEMIDLIST pidl;
		if (SHBindToParent(pidlSrc, IID_PPV_ARGS(&psf), &pidl) == S_OK) {
			CComQIPtr<IShellFolder2, &IID_IShellFolder2> psf2(psf);
			if (psf2.p != NULL) {
				VARIANT vr;
				SHCOLUMNID scid = PKEY_Link_TargetParsingPath;
				if (psf2->GetDetailsEx(pidl, &scid, &vr) == S_OK) {
					str = vr.bstrVal;
					VariantClear(&vr);
					ok = TRUE;
				}
			}
			psf->Release();
		}
		CoTaskMemFree(pidlSrc);
	}
	return ok;
}

extern "C"
BOOL WINAPI fopRemoveJunction(LPCTSTR pszPath)
{
	CWin32FindData wfd;
	if (!GetFileData(pszPath, wfd) || !wfd.IsAttrib(FA_REPARSE))
		return FALSE;

	CString src = pszPath;
	DWORD type = 0;
	LPTSTR pszOriginalName;
	if (fopGetReparsePointInfo(src, &pszOriginalName, &type) == NOERROR) {
		src.Format(IDS_LINK_FROMTO, pszPath, pszOriginalName);
		CoTaskMemFree(pszOriginalName);
	}
	else
		return FALSE;

	// vista以降はTaskDialogを利用
	TASKDIALOGCONFIG tdc;
	memset(&tdc, 0, sizeof(tdc));
	tdc.cbSize =				sizeof(tdc);
	tdc.hwndParent =			GetActiveWindow();
	tdc.hInstance =				RESINST;
	tdc.dwFlags =				TDF_ALLOW_DIALOG_CANCELLATION;
	tdc.dwCommonButtons =		TDCBF_YES_BUTTON | TDCBF_NO_BUTTON;
	tdc.pszWindowTitle =		MAKEINTRESOURCE((type == IO_REPARSE_TAG_SYMLINK) ? IDS_TITLE_SYMLINK : IDS_TITLE_REPARSE);
	tdc.pszMainIcon =			MAKEINTRESOURCE(IDI_MBX_QUESTION);
	tdc.pszMainInstruction =	MAKEINTRESOURCE((type == IO_REPARSE_TAG_SYMLINK) ? IDS_REMOVE_SYMLINK : IDS_REMOVE_JUNCTION);
	tdc.pszContent =			src.GetString();
	int res = 0;
	if ((TaskDialogIndirect(&tdc, &res, NULL, NULL) != S_OK) || (res != IDYES))
		res = IDNO;
	if (res == IDYES) {
		if (wfd.IsAttrib(FA_HIDDEN | FA_RDONLY | FA_SYSTEM))
			SetFileAttributes(pszPath, FA_NORMAL);
		if (wfd.IsFolder())
			return RemoveDirectory(pszPath);
		else
			return fopRemoveFileNoConfirm(pszPath);
	}
	return FALSE;
}

