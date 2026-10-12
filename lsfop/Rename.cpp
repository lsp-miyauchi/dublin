// name.cpp : implementation file
//

#include "stdafx.h"

#include "fopstd.h"
#include "itemnamelimits.h"


/////////////////////////////////////////////////////////////////////////////
// RenameDlgFn

class CRenameDlg : public CDialogImpl<CRenameDlg>
{
private:
	CEdit m_edit;
	int m_renametype;
	bool m_bDirectory:1;
	bool m_bRenameDOS:1;		// DOS名を変更するには CheckNameLegalDOS8Dot3 と SetFileShortName を使う

public:
	BOOL m_bUseIgnoreButton;
	CString m_strName;

public:
	CRenameDlg()
		: m_bUseIgnoreButton(FALSE)
		, m_bRenameDOS(false)
		, m_bDirectory(false)
		, m_renametype(0)
	{
	}
	~CRenameDlg() { }

	enum { IDD = IDD_RENAME };

BEGIN_MSG_MAP(CRenameDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDIGNORE, OnIgnore)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_AUTORENAME, OnAutoRename)
	ON_NOTIFY(BCN_DROPDOWN, IDC_AUTORENAME, OnNotifyAutoRenameDropDown)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnOK();
	void OnIgnore() { EndDialog(IDIGNORE); }
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnAutoRename();
	void OnNotifyAutoRenameDropDown(LPNMHDR pnmh, LRESULT* pResult);

	void AutoRename(int renametype);
	bool SetSelFileBody();
};

BOOL CRenameDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	m_edit = GetDlgItem(IDC_EDIT);

	// スプリットボタン
	GetDlgItem(IDC_AUTORENAME).DestroyWindow();
	GetDlgItem(IDC_AUTORENAME_SPLIT).SetWindowLong(GWL_ID, IDC_AUTORENAME);

	// 無視ボタン
	GetDlgItem(IDIGNORE).EnableWindow(m_bUseIgnoreButton);

	if (m_strName.IsEmpty()) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	// ファイル情報を取得
	CWin32FindData wfd;
	DWORD code = _getfileinfo(m_strName, &wfd);
	if (code != NOERROR) {
		SetLastError(code);
		LastError(IDS_TITLE_RENAME, m_strName, MB_STOP);
		EndDialog(IDCANCEL);
		return TRUE;
	}

	CFileInfo fi;
	if (!GetFileInfo(m_strName, fi)) {
		EndDialog(IDCANCEL);
		return TRUE;
	}
	m_bDirectory = (fi.IsFolder() != FALSE);

	CDiskSpace ds(m_strName, TRUE);

	// 名前と場所
	SetDlgItemText(IDC_TITLE, fi.m_pszName);

	// 場所
	TCHAR buf[MAX_PATH * 2];
	fi.GetFullPathName(buf, ARRAYSIZE(buf));
	PathRemoveFileSpec(buf);
	PathSetDlgItemPath(m_hWnd, IDC_PATH, buf);

	// 入力規則
	idl_LimitInputEdit(m_edit, buf);

	// 種別・アイコン
	SHFILEINFO sfi;
	HIMAGELIST hi = (HIMAGELIST)SHGetFileInfo(m_strName, 0, &sfi, sizeof(sfi), SHGFI_LARGEICON | SHGFI_SYSICONINDEX | SHGFI_TYPENAME);
	SetDlgItemText(IDC_FILETYPE, sfi.szTypeName);
	HICON hic = ImageList_GetIcon(hi, sfi.iIcon, ILD_NORMAL);
	SendDlgItemMessage(IDC_FILEICON, STM_SETICON, (WPARAM)hic, 0);

	// 更新日時
	fi.FormatDateTime(buf, ARRAYSIZE(buf));
	SetDlgItemText(IDC_MODIFIED, buf);

	// サイズ
	if (!fi.IsFolder()) {
		CString str;
		FileSizeInformation(fi.m_dlSize, str);
		SetDlgItemText(IDC_FILESIZE, str);
	}

	// 所有者
	if (ds.isNTFS() && lccGetFileCreateOwner(m_strName, buf))
		SetDlgItemText(IDC_FILEOWNER, buf);

	// DOS名
	if (wfd.cAlternateFileName[0])
		SetDlgItemText(IDC_DOSNAME, wfd.cAlternateFileName);

	BOOL bSaveExtension = GetRegValue(_MKIR(IDS_SEC_OPERATION), _MKIR(IDS_KEY_SAVEEXTENSION), FALSE);
	CheckDlgButton(IDC_SAVEEXTENSION, bSaveExtension);

	m_edit.SetWindowText(fi.m_pszName);

	// 拡張子を保持する設定の場合には、選択範囲をファイルボディだけにする
	if (m_bDirectory)
		GetDlgItem(IDC_SAVEEXTENSION).EnableWindow(FALSE);
	else if (SetSelFileBody())
		return FALSE;
	return TRUE;
}

bool CRenameDlg::SetSelFileBody()
{
	if (!m_bDirectory) {
		TCHAR buf[MAX_PATH * 2];
		if (m_edit.GetWindowText(buf, ARRAYSIZE(buf)) > 0) {
			LPTSTR p = StrRChr(buf, NULL, _T('.'));
			if (p) {
				m_edit.SetFocus();
#ifdef _WIN64
				m_edit.SetSel(0, (int)(p - buf));
#else
				m_edit.SetSel(0, PtrToInt(p - buf));
#endif // _WIN64
				return true;
			}
		}
	}
	return false;
}

BOOL CRenameDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW) {
		ShowHtmlHelp(m_hWnd, IDD);
		return TRUE;
	}
	return FALSE;
}

void CRenameDlg::OnOK()
{
	TCHAR buf[MAX_PATH * 2];
	if (m_edit.GetWindowText(buf, ARRAYSIZE(buf)) < 1)
		return;

/* @@@
// 入力規則 idl_LimitInputEdit の適用により不要
	if (StrPBrk(buf, gszInvalidToken)) {
		AppMessage(IDS_TITLE_RENAME, IDS_INVALIDFILECHAR);
		m_edit.SetFocus();
		return;
	}
@@@ */

	TCHAR name[MAX_PATH * 2];
	lstrcpy(name, m_strName);
	PathRemoveFileSpec(name);
	PathAppend(name, buf);

	LPTSTR pes = PathFindExtension(m_strName);
	LPTSTR ped = PathFindExtension(name);

	// 対象がディレクトリ以外のファイルの場合、元の拡張子を保持するか
	BOOL bSaveExtension = IsDlgButtonChecked(IDC_SAVEEXTENSION);
	if (!m_bDirectory && pes && (!ped || StrCmpI(pes, ped))) {
		if (bSaveExtension)
			lstrcat(name, pes);
		else {
			// 元の名前の拡張子に関連づけがあるかどうか
			if (lccRegCheckShellVerb(m_strName)) {
				if (AppMessage(IDS_TITLE_RENAME, IDS_CHANGEEXT_WARNING, NULL, MB_QYESNO) != IDYES) {
					m_edit.SetFocus();
					return;
				}
			}
		}
	}

	// 同一名（大文字小文字も一致）であれば
	if (!StrCmp(m_strName, name)) {
		MessageBeep(0);
		m_edit.SetFocus();
		return;
	}

	if (!validate_makefile(name)) {
		AppMessage(IDS_TITLE_RENAME, IDS_INVALIDFILENAME, name);
		m_edit.SetFocus();
		return;
	}

	if (_pathfileexists(name) && StrCmpI(name, m_strName)) {
		AppMessage(IDS_TITLE_RENAME, IDS_SAMENAME, buf);
		m_edit.SetFocus();
		return;
	}

	m_strName = name;
	SetRegValue(_MKIR(IDS_SEC_OPERATION), _MKIR(IDS_KEY_SAVEEXTENSION), bSaveExtension);
	EndDialog(IDOK);
}

void CRenameDlg::AutoRename(int renametype)
{
	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, m_strName);
	PathSetUnique(buf, renametype);
	SetDlgItemText(IDC_EDIT, PathFindFileName(buf));
	SetSelFileBody();

	m_renametype = renametype;
	if (++m_renametype >= 6)
		m_renametype = 0;
}

void CRenameDlg::OnAutoRename()
{
	AutoRename(m_renametype);
}

void CRenameDlg::OnNotifyAutoRenameDropDown(LPNMHDR pnmh, LRESULT* pResult)
{
	CMenu pop; pop.CreatePopupMenu();

	CString str;
	TCHAR buf[MAX_PATH * 2];
	for (int i = 0; i < 6; i++) {
		lstrcpy(buf, m_strName);
		PathSetUnique(buf, i);
		str.Format(_T("%s\t(&%d)"), PathFindFileName(buf), i + 1);
		pop.AppendMenu(MF_STRING, i + 1, str);
	}

	NMBCDROPDOWN* pbdd = (NMBCDROPDOWN*)pnmh;
	CRect rc = pbdd->rcButton;
	GetDlgItem((int)pnmh->idFrom).ClientToScreen(&rc);

	int cmd = pop.TrackPopupMenu(TPM_RIGHTALIGN | TPM_RETURNCMD, rc.right, rc.bottom, m_hWnd);
	if (cmd > 0)
		AutoRename(cmd - 1);
}

/////////////////////////////////////////////////////////////////////////////
// NewNameDlgFn

class CNewNameDlg : public CDialogImpl<CNewNameDlg>
{
public:
	CNewNameDlg() { m_strName.Empty(); }
	~CNewNameDlg() { }

	enum { IDD = IDD_NEWNAME };

BEGIN_MSG_MAP(CNewNameDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }

	CString m_strName;
};

BOOL CNewNameDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	// 名前
	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, m_strName);
	LPCTSTR p = PathFindFileName(buf);
	if (!p) p = buf;
	SetDlgItemText(IDC_TITLE, p);

	// 新しい名前
	PathSetUnique(buf);
	p = PathFindFileName(buf);
	if (!p) p = buf;
	SetDlgItemText(IDC_EDIT, p);

	// 入力規則
	PathRemoveFileSpec(buf);
	idl_LimitInputEdit(GetDlgItem(IDC_EDIT), buf);

	return TRUE;
}

BOOL CNewNameDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW) {
		ShowHtmlHelp(m_hWnd, IDD);
		return TRUE;
	}
	return FALSE;
}

void CNewNameDlg::OnOK()
{
	TCHAR buf[MAX_PATH * 2];
	if (GetDlgItemText(IDC_EDIT, buf, ARRAYSIZE(buf)) < 1)
		return;

/* @@@
// 入力規則 idl_LimitInputEdit の適用により不要
	if (StrPBrk(buf, gszInvalidToken)) {
		AppMessage(IDS_TITLE_RENAME, IDS_INVALIDFILECHAR);
		GetDlgItem(IDC_EDIT).SetFocus();
		return;
	}
@@@ */

	if (!validate_makefile(buf)) {
		AppMessage(IDS_TITLE_RENAME, IDS_INVALIDFILENAME, buf);
		GetDlgItem(IDC_EDIT).SetFocus();
		return;
	}

	CPath file;
	file.m_strPath = m_strName;
	file.RemoveFileSpec();
	file.Append(buf);
	if (_pathfileexists(file)) {
		AppMessage(IDS_TITLE_RENAME, IDS_SAMENAME, buf);
		GetDlgItem(IDC_EDIT).SetFocus();
		return;
	}
	m_strName = file.m_strPath;
	EndDialog(IDOK);
}

/////////////////////////////////////////////////////////////////////////////
// MakedirDlgFn

class CMakedirDlg : public CDialogImpl<CMakedirDlg>
{
public:
	CMakedirDlg() { m_path.Empty(); }
	~CMakedirDlg() { }

	enum { IDD = IDD_MAKEDIR };

BEGIN_MSG_MAP(CMakedirDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_EN_CHANGE(IDC_EDIT, OnEditChange)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnEditChange();
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }

	int BuildPath(CString& str);

	CString m_path;
};

BOOL CMakedirDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	if (m_path.IsEmpty())
		EndDialog(IDCANCEL);
	else
		PathSetDlgItemPath(m_hWnd, IDC_TITLE, m_path);

	// 入力不可な文字列を定義
	CComPtr<ILimitEditChars> plec;
	plec = new CComObject<CLimitEditChars>;
	plec->SetLimitChars(L"/?\"*<>|");
	idl_LimitInputEdit(GetDlgItem(IDC_EDIT), (IShellFolder*)(ILimitEditChars*)plec);

	return TRUE;
}


int CMakedirDlg::BuildPath(CString& str)
{
	str.Empty();
	CEdit edit = GetDlgItem(IDC_EDIT);
	if (edit.GetWindowText(str) > 0) {
/*
		// フォルダ名に使用できない文字を削除
		int n = str.Remove(_T('/'));
		n += str.Remove(_T('|'));
		n += str.Remove(_T('*'));
		n += str.Remove(_T('?'));
		n += str.Remove(_T('\"'));
		n += str.Remove(_T('<'));
		n += str.Remove(_T('>'));
		if (n > 0) {
			int s, t;
			edit.GetSel(s, t);
			edit.SetWindowText(str);
			edit.SetSel(s - n, s - n);
		}
*/
		TCHAR path[MAX_PATH * 2];
		if (PathIsUNC(str) || (ISALPHA(str[0]) && str[1] == _T(':')))
			lstrcpy(path, str);
		else if (str[0] == _T('\\')) {
			if (!PathIsRoot(m_path))
				PathCombine(path, m_path, str);
			else {
				lstrcpy(path, m_path);
				PathStripToRoot(path);
				PathAppend(path, str);
			}
			str = path;
		}
		else {
			PathCombine(path, m_path, str);
			LPTSTR p = str.GetBuffer(lstrlen(path) + 1);
			PathCanonicalize(p, path);
			str.ReleaseBuffer();
		}
	}
	return str.GetLength();
}

void CMakedirDlg::OnEditChange()
{
	CString str;
	BuildPath(str);
	SetDlgItemText(IDC_PATH, str);
}

BOOL CMakedirDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW) {
		ShowHtmlHelp(m_hWnd, IDD);
		return TRUE;
	}
	return FALSE;
}

void CMakedirDlg::OnOK()
{
	CString str;
	if (GetDlgItemText(IDC_PATH, str) < 1) return;

	// フォルダ名不適格文字のチェック
	LPTSTR p = PathFindFileName(str);
	if (!p) return;

/*
	if (StrPBrk(p, gszInvalidToken + 2)) {		// skip gszInvalidToken's "\/"
		AppMessage(IDS_TITLE_MKDIR, IDS_INVALIDFILECHAR);
		GetDlgItem(IDC_EDIT).SetFocus();
		return;
	}
*/

	// 文字列終端が ' ' または '.' で終わるフォルダ名の修正
	if (!validate_makefile(p)) {
		AppMessage(IDS_TITLE_MKDIR, IDS_INVALIDFILENAME, p);
		return;
	}

	if (str.GetLength() >= MAX_PATH) {
		SetLastError(ERROR_FILENAME_EXCED_RANGE);
		LastError(IDS_TITLE_MKDIR, NULL, MB_STOP);
		GetDlgItem(IDC_EDIT).SetFocus();
		return;
	}

	if (_pathfileexists(str)) {
 		AppMessage(IDS_TITLE_MKDIR, IDS_SAMENAME, str, MB_STOP);
		return;
	}

	m_path = str;
	EndDialog(IDOK);
}

/////////////////////////////////////////////////////////////////////////////
//

class CShellItemInformation
{
public:
	IShellFolder2* m_psf2;
	LPITEMIDLIST m_pidl;
	CString m_strName;
	CString m_strTitle;
	CString m_strType;
	CString m_strFolder;
	SFGAOF m_attr;
	FILETIME m_ftLastWriteTime;
	ULONGLONG m_ullSize;

	CShellItemInformation() : m_psf2(NULL), m_pidl(NULL), m_attr(0), m_ullSize(0)
	{
		memset(&m_ftLastWriteTime, 0, sizeof(FILETIME));
	}
	~CShellItemInformation()
	{
		if (m_psf2) m_psf2->Release();
		if (m_pidl) CoTaskMemFree(m_pidl);
	}

	inline bool isFolder() { return ((m_attr & SFGAO_FOLDER) != 0); }

	BOOL GetInformation(LPCITEMIDLIST pidlTarget)
	{
		LPCITEMIDLIST pidl;
		if (SHBindToParent(pidlTarget, IID_IShellFolder2, (void**)&m_psf2, &pidl) != S_OK) return FALSE;

		m_pidl = ILClone(pidl);
		VARIANT vr;
		// 名前
		if (m_psf2->GetDetailsEx(m_pidl, &PKEY_FileName, &vr) == S_OK) {
			m_strName = vr.bstrVal;
			VariantClear(&vr);
		}
		// タイトル
		if (m_psf2->GetDetailsEx(m_pidl, &PKEY_ItemNameDisplay, &vr) == S_OK) {
			m_strTitle = vr.bstrVal;
			VariantClear(&vr);
		}
		// 属性
		SFGAOF attr = SFGAO_CAPABILITYMASK | SFGAO_CONTENTSMASK;
		if (m_psf2->GetAttributesOf(1, (LPCITEMIDLIST*)&m_pidl, &attr) == S_OK)
			m_attr = attr;
		// 種別
		if (m_psf2->GetDetailsEx(m_pidl, &PKEY_ItemTypeText, &vr) == S_OK) {
			m_strType = vr.bstrVal;
			VariantClear(&vr);
		}
		// 更新日付
		if (m_psf2->GetDetailsEx(m_pidl, &PKEY_DateModified, &vr) == S_OK) {
			VariantTimeToFileTime(&vr, &m_ftLastWriteTime);
			VariantClear(&vr);
		}
		// サイズ
		if (!(m_attr & SFGAO_FOLDER) && m_psf2->GetDetailsEx(m_pidl, &PKEY_Size, &vr) == S_OK) {
			m_ullSize = vr.ullVal;
			VariantClear(&vr);
		}
		// 場所
		if (m_psf2->GetDetailsEx(m_pidl, &PKEY_ItemFolderPathDisplay, &vr) == S_OK) {
			m_strFolder = vr.bstrVal;
			VariantClear(&vr);
		}
		if (m_strFolder.IsEmpty()) {
			LPITEMIDLIST pidlw = ::ILClone(pidlTarget);
			if (pidlw) {
				void* baseptr = pidlw;
				while (pidlw) {
					if (!ILRemoveLastID(pidlw) || idl_isDesktop(pidlw)) break;

					CString infolder;
					idl_GetName(pidlw, SHGDN_INFOLDER, infolder);
					if (!m_strFolder.IsEmpty()) {
						infolder.AppendFormat(_T("\\%s"), m_strFolder.GetString());
					}
					m_strFolder = infolder;
				}
				CoTaskMemFree(baseptr);
			}
		}
		return TRUE;
	}
};

/////////////////////////////////////////////////////////////////////////////
// ShellRenameDlgFn

class CShellRenameDlg : public CDialogImpl<CShellRenameDlg>
{
private:
	CEdit m_edit;

public:
	BOOL m_bUseIgnoreButton;
	LPCITEMIDLIST m_pidlFq;
	CString m_strNewName;
	CShellItemInformation m_sii;

public:
	CShellRenameDlg()
		: m_bUseIgnoreButton(FALSE)
		, m_pidlFq(NULL) { }
	~CShellRenameDlg() { }

	enum { IDD = IDD_SHELLRENAME };

BEGIN_MSG_MAP(CShellRenameDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDIGNORE, OnIgnore)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnOK();
	void OnIgnore() { EndDialog(IDIGNORE); }
	void OnCancel() { EndDialog(IDCANCEL); }
};

BOOL CShellRenameDlg::OnInitDialog(LPARAM lParam)
{
	if (m_pidlFq == NULL) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	CenterWindow();
	m_edit = GetDlgItem(IDC_EDIT);

	// 無視ボタン
	GetDlgItem(IDIGNORE).EnableWindow(m_bUseIgnoreButton);

	// オブジェクト情報を取得
	m_sii.GetInformation(m_pidlFq);
	// 名前
	SetDlgItemText(IDC_TITLE, m_sii.m_strName);
	m_edit.SetWindowText(m_sii.m_strName);
	// 種別
	SetDlgItemText(IDC_FILETYPE, m_sii.m_strType);
	// 場所
	PathSetDlgItemPath(m_hWnd, IDC_PATH, m_sii.m_strFolder);
	// 更新日時
	TCHAR buf[MAX_PATH];
	FormatFileDateTime(&m_sii.m_ftLastWriteTime, buf, ARRAYSIZE(buf));
	SetDlgItemText(IDC_MODIFIED, buf);
	// サイズ
	if (!(m_sii.m_attr & SFGAO_FOLDER)) {
		CString str;
		FileSizeInformation(m_sii.m_ullSize, str);
		SetDlgItemText(IDC_FILESIZE, str);
	}
	// 入力規則
	idl_LimitInputEdit(m_edit, (IShellFolder*)m_sii.m_psf2);
	// アイコン
	SHFILEINFO sfi;
	HIMAGELIST hi = (HIMAGELIST)SHGetFileInfo((LPCTSTR)m_pidlFq, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_LARGEICON | SHGFI_SYSICONINDEX);
	HICON hic = ImageList_GetIcon(hi, sfi.iIcon, ILD_NORMAL);
	SendDlgItemMessage(IDC_FILEICON, STM_SETICON, (WPARAM)hic, 0);

	return TRUE;
}

BOOL CShellRenameDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW) {
		ShowHtmlHelp(m_hWnd, IDD);
		return TRUE;
	}
	return FALSE;
}

void CShellRenameDlg::OnOK()
{
	CString name;
	if (m_edit.GetWindowText(name) > 0) {
		// 同一名（大文字小文字も一致）であれば
		if (!m_sii.m_strName.CompareNoCase(name)) {
			MessageBeep(0);
			m_edit.SetFocus();
		}
		else {
			m_strNewName = name;
			EndDialog(IDOK);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI fopGetNewName(HWND hwnd, LPTSTR pszPathName)
{
	CNewNameDlg dlg;
	dlg.m_strName = pszPathName;
	if (dlg.DoModal(hwnd) == IDOK) {
		lstrcpy(pszPathName, dlg.m_strName);
		return TRUE;
	}
	return FALSE;
}

BOOL RenameFileOrDirectory(LPCTSTR oldname, LPCTSTR newname)
{
	if (!_movefile(oldname, newname)) {
		LastError(IDS_TITLE_RENAME, PathFindFileName(newname));
		return FALSE;
	}
	SHChangeNotify(
			_pathisdirectory(newname) ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM,
			SHCNF_PATH | SHCNF_FLUSHNOWAIT, oldname, newname);
	return TRUE;
}

static BOOL DoRename(HWND hwnd, LPTSTR pName, BOOL bMultiple)
{
	CRenameDlg dlg;
	dlg.m_strName = pName;
	dlg.m_bUseIgnoreButton = bMultiple;
	INT_PTR res = dlg.DoModal(hwnd);

	if (res == IDOK) {
		BOOL ok = RenameFileOrDirectory(pName, dlg.m_strName);
		if (ok)
			lstrcpy(pName, dlg.m_strName);
		return ok;
	}
	return (res == IDIGNORE) ? TRUE : FALSE;
}

extern "C"
BOOL WINAPI fopRename(HWND hwnd, LPTSTR pszName)
{
	return DoRename(hwnd, pszName, FALSE);
}

extern "C"
BOOL WINAPI fopRenameFiles(HWND hwnd, HDROP hdrop)
{
	BOOL bOK = FALSE;
	int cnt = hdrop ? DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0) : 0;
	for (int i = 0; i < cnt; i++) {
		TCHAR name[MAX_PATH * 2];
		DragQueryFile(hdrop, i, name, ARRAYSIZE(name));
		if (!PathIsRoot(name)) {
			bOK = DoRename(hwnd, name, (cnt > 1) ? TRUE : FALSE);
			if (!bOK) break;
		}
	}
	return bOK;
}

extern "C"
BOOL WINAPI fopCreateNewFolder(HWND hwnd, LPTSTR pName)
{
	CMakedirDlg dlg;
	dlg.m_path = pName;
	if (dlg.DoModal(hwnd) == IDOK) {
		lstrcpy(pName, dlg.m_path);
		return fopMakeTree(pName);
	}
	return FALSE;
}

static BOOL DoShellRename(HWND hwnd, LPCITEMIDLIST pidlTarget, BOOL bMultiple)
{
	CShellRenameDlg dlg;
	dlg.m_pidlFq = pidlTarget;
	dlg.m_bUseIgnoreButton = bMultiple;
	INT_PTR res = dlg.DoModal(hwnd);
	if (res == IDOK) {
		return (dlg.m_sii.m_psf2->SetNameOf(hwnd, dlg.m_sii.m_pidl, dlg.m_strNewName, SHGDN_INFOLDER | SHGDN_FOREDITING, NULL) == S_OK);
	}
	return (res == IDIGNORE) ? TRUE : FALSE;
}

extern "C"
BOOL WINAPI fopShellRename(HWND hwnd, LPCITEMIDLIST pidlTarget)
{
	return DoShellRename(hwnd, pidlTarget, FALSE);
}

extern "C"
BOOL WINAPI fopShellRenameItems(HWND hwnd, IDataObject* pdo)
{
	BOOL bOK = FALSE;
	CSimpleIDListArray array;
	int i, cnt = GetIDListsFromDataObject(pdo, array);
	for (i = 0; i < cnt; i++) {
		bOK = DoShellRename(hwnd, array[i], (cnt > 1));
		if (!bOK) break;
	}
	return bOK;
}

