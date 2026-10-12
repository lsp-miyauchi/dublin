// RenExt.cpp : implementation file
//

#include "stdafx.h"
#include "fopstd.h"


/////////////////////////////////////////////////////////////////////////////
//

class CRenExt
{
public:
	CRenExt();
	~CRenExt() {}

	CString m_csPath;
	CString m_csSrcExt;
	CString m_csDstExt;
	BOOL m_bIncFolder;
};

CRenExt::CRenExt()
{
	m_csPath.Empty();
	m_csSrcExt.Empty();
	m_csDstExt.Empty();
	m_bIncFolder = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// RenExtDlgFn

class CRenExtDlg : public CDialogImpl<CRenExtDlg>
{
public:
	CRenExtDlg() { m_pre = NULL; }
	~CRenExtDlg() { }

	enum { IDD = IDD_RENEXT };

BEGIN_MSG_MAP(CRenExtDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_NOTIFY(SEN_GETTEXT, IDC_PATH, OnNotifyBrowse)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult);
	void OnHelpCall() { ShowHtmlHelp(m_hWnd, IDD); }

	CRenExt* m_pre;
};

BOOL CRenExtDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	m_pre = (CRenExt*)lParam;
	SetDlgItemText(IDC_PATH, m_pre->m_csPath);
	SetDlgItemText(IDC_SRCEXT, m_pre->m_csSrcExt);
	SetDlgItemText(IDC_DSTEXT, m_pre->m_csDstExt);
	CheckDlgButton(IDC_INCSUBFOLDER, m_pre->m_bIncFolder);

	// 入力規則
	idl_LimitInputEdit(GetDlgItem(IDC_DSTEXT), m_pre->m_csPath);

	return TRUE;
}

BOOL CRenExtDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW) {
		ShowHtmlHelp(m_hWnd, IDD);
		return TRUE;
	}
	return FALSE;
}

void CRenExtDlg::OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;

	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, psgt->pszText);
	if (fopBrowseForFolder(m_hWnd, buf) == IDOK) {
		lstrcpyn(psgt->pszText, buf, psgt->cchTextMax);
		*pResult = TRUE;
	}
}

void CRenExtDlg::OnOK()
{
	TCHAR buf[MAX_PATH];
	GetDlgItemText(IDC_PATH, buf, ARRAYSIZE(buf));
	m_pre->m_csPath = buf;
	GetDlgItemText(IDC_SRCEXT, buf, ARRAYSIZE(buf));
	m_pre->m_csSrcExt = buf;
	GetDlgItemText(IDC_DSTEXT, buf, ARRAYSIZE(buf));
	m_pre->m_csDstExt = buf;

	if (m_pre->m_csPath.IsEmpty() ||
		m_pre->m_csSrcExt.IsEmpty() ||
		m_pre->m_csDstExt.IsEmpty())
		MessageBeep(0);
	else {
		m_pre->m_bIncFolder = IsDlgButtonChecked(IDC_INCSUBFOLDER);
		EndDialog(IDOK);
	}
}

/////////////////////////////////////////////////////////////////////////////
//

int RenameExtension(LPCTSTR pszPath, LPCTSTR pszSrcExt, LPCTSTR pszDstExt, BOOL bIncFolder)
{
	CFileInfo* pf;
	CFileArray array;
	TCHAR src[MAX_PATH * 2], dst[MAX_PATH * 2], body[MAX_PATH];

	int nDone = 0;
	wsprintf(src, _T("*.%s"), pszSrcExt);
	int i, cnt = array.BrowseFiles(pszPath, src);
	for (i = 0; i < cnt; i++) {
		pf = array[i];
		lstrcpy(body, pf->m_pszName);
		LPTSTR p = StrRChr(body, NULL, '.');
		if (p)
			*p = '\0';
		if (pszDstExt && *pszDstExt)
			wsprintf(src, _T("%s.%s"), body, pszDstExt);
		else
			lstrcpy(src, body);

		PathCombine(dst, pszPath, src);

		// 変更後ファイル名の存在チェック
		if (StrStrI(src, dst) && _pathfileexists(dst)) {
			// 存在すれば body(nnn).ext 形式で名前を構成
			CString str;
			for (int n = 1; ; n++) {
				str.Format(_T("%s(%d)"), body, n);
				if (pszDstExt && *pszDstExt) {
					str += _T('.');
					str += pszDstExt;
				}
				PathCombine(dst, pszPath, str);
				if (!_pathfileexists(dst))
					break;
			}
		}

		pf->GetFullPathName(src, ARRAYSIZE(src));
		if (!_movefile(src, dst))
			break;
		nDone++;
	}

	if (!bIncFolder)
		return nDone;

	array.DeleteAll();
	cnt = array.EnumSubFolder(pszPath);
	for (i = 0; i < cnt; i++) {
		pf = array[i];
		PathCombine(src, pszPath, pf->m_pszName);
		nDone += RenameExtension(src, pszSrcExt, pszDstExt, bIncFolder);
	}
	return nDone;
}

extern "C"
BOOL WINAPI fopRenameExtension(HWND hwnd, LPCTSTR pszFileName)
{
	CRenExt re;
	TCHAR buf[MAX_PATH * 2];
	GetCurrentDirectory(ARRAYSIZE(buf), buf);
	re.m_csPath = buf;

	if (pszFileName && *pszFileName) {
		if (_pathisdirectory(pszFileName)) {
			re.m_csPath = pszFileName;
			re.m_bIncFolder = TRUE;
		}
		else {
			PathGetParent(pszFileName, buf);
			re.m_csPath = buf;
			LPTSTR p = PathFindExtension(pszFileName);
			if (*p == '.')
				re.m_csSrcExt = ++p;
		}
	}

	CRenExtDlg dlg;
	if (dlg.DoModal(hwnd, (LPARAM)&re) == IDOK)
		return (BOOL)RenameExtension(re.m_csPath, re.m_csSrcExt, re.m_csDstExt, re.m_bIncFolder);
	return FALSE;
}
