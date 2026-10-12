// Brief.cpp : implementation file
//

#include "stdafx.h"
#include "fopstd.h"

///////////////////////////////////////////////////////////
//

typedef struct tagBRIEFINFO {
	TCHAR path[MAX_PATH];
	TCHAR name[MAX_PATH];
} BRIEFINFO;


/////////////////////////////////////////////////////////////////////////////
// CMakeBriefDlg dialog

class CMakeBriefDlg : public CDialogImpl<CMakeBriefDlg>
{
public:
	CMakeBriefDlg() { m_pbi = NULL; }
	~CMakeBriefDlg() { }

	enum { IDD = IDD_MKBRIEF };

BEGIN_MSG_MAP(CMakeBriefDlg)
	ON_WM_INITDIALOG()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_NOTIFY(SEN_GETTEXT, IDC_PATH, OnNotifyBrowse)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDestroy();
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult);

	BRIEFINFO* m_pbi;
};

BOOL CMakeBriefDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	m_pbi = (BRIEFINFO*)lParam;
	SetDlgItemText(IDC_PATH, m_pbi->path);

	// ブリーフアイコン
	HICON hic = ExtractIcon(RESINST, _T("syncui.dll"), 0);
	if (hic) {
		CStatic stc = GetDlgItem(IDC_LABEL);
		stc.SetIcon(hic);
	}
	return TRUE;
}

void CMakeBriefDlg::OnDestroy()
{
	CStatic stc = GetDlgItem(IDC_LABEL);
	HICON hic = stc.SetIcon(NULL);
	if (hic)
		DestroyIcon(hic);
}

void CMakeBriefDlg::OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;

	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, psgt->pszText);
	if (fopBrowseForFolder(m_hWnd, buf) == IDOK) {
		lstrcpyn(psgt->pszText, buf, psgt->cchTextMax);
		*pResult = TRUE;
	}
}

void CMakeBriefDlg::OnOK()
{
	if (GetDlgItemText(IDC_PATH, m_pbi->path, ARRAYSIZE(m_pbi->path)) < 1 ||
		GetDlgItemText(IDC_NAME, m_pbi->name, ARRAYSIZE(m_pbi->name)) < 1)
		return;

	if (StrPBrk(m_pbi->name, gszInvalidToken)) {
		AppMessage(IDS_TITLE_MKDIR, IDS_INVALIDFILECHAR);
		return;
	}

	if (lstrlen(m_pbi->path) + lstrlen(m_pbi->name) >= MAX_PATH) {
		MessageBeep(0);
		return;
	}
	EndDialog(IDOK);
}

///////////////////////////////////////////////////////////
//

BOOL InvokeFile(HWND hwndOwner, LPCTSTR pVerb, LPCTSTR pFile, LPCTSTR pParam, LPCTSTR pDir, bool bWaitExitProgram);

extern "C"
BOOL WINAPI fopMakeBriefcase(HWND hwnd, LPTSTR pszPath)
{
	BRIEFINFO bi;
	ZeroMemory(&bi, sizeof(bi));
	lstrcpy(bi.path, pszPath);

	CMakeBriefDlg dlg;
	if (dlg.DoModal(hwnd, (LPARAM)&bi) == IDOK) {
		TCHAR brief[MAX_PATH], buf[MAX_PATH * 2];
		PathCombine(brief, bi.path, bi.name);
		CString str;
		str.Format(_T("syncui.dll,Briefcase_Create 1!d! %s"), brief);
		InvokeFile(hwnd, NULL, _T("rundll32.exe"), str.GetString(), NULL, true);

		if (PathIsDirectory(brief)) {
			// SendToフォルダにリンクを作成する
			if (SHGetSpecialFolderPath(NULL, buf, CSIDL_SENDTO, FALSE)) {
				str = PathFindFileName(brief);
				PathAppend(buf, str);
				lstrcat(buf, _T(".lnk"));
				CreateLinkObject(brief, buf, str.GetString(), NULL, FALSE);
			}
			return TRUE;
		}
	}
	return FALSE;
}
