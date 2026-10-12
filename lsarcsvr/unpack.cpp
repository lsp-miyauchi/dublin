e// UnpackDlg.cpp : CUnpackDlg の実装

#include "stdafx.h"
#include "common.h"
#include "unpack.h"
#include "dialogtempl.h"

#define MAXACCESS		16

const TCHAR* szPathForm = _T(" %s %s");

static LPCTSTR szLzhCmd[] = {
#ifdef _WIN64
	_T("e -x0 %s %s"),		// none
	_T("e -x1 %s %s"),		// incfol
	_T("e -x0 %s %s"),		// new-or-notex
	_T("e -x1 %s %s"),		// incfol + new-or-notex
#else
	_T("e -c1r2x0a2m0p1s0 -jf0jp1 -+ %s %s"),			// none
	_T("e -c1r2x1a2m0p1s0 -jf0jp1 -jyc1 -+ %s %s"),		// incfol
	_T("e -c0r2x0a2m1p1s0 -jf0jp1 -+ %s %s"),			// new-or-notex
	_T("e -c0r2x1a2m1p1s0 -jf0jp1 -jyc1 -+ %s %s"),		// incfol + new-or-notex
#endif
};

static LPCTSTR szZipCmd[] = {
	_T("-x -j"),						// none
	_T("-x"),							// incfol
	_T("-u -j"),						// new-or-notex
	_T("-u"),							// incfol + new-or-notex
};

static LPCTSTR szArjCmd[] = {
	_T("e -c -hk %s %s"),				// none
	_T("x -hk %s %s"),					// incfol
	_T("e -f -hk %s %s"),				// new-or-notex
	_T("x -f -hk %s %s"),				// incfol + new-or-notex
};

static LPCTSTR szIshCmd[] = {
	_T("%s /f=%s"),						// none
	_T("%s /f=%s"),						// incfol
	_T("%s /r /f=%s"),					// new-or-notex
	_T("%s /r /f=%s"),					// incfol + new-or-notex
};

static LPCTSTR szCabCmd[] = {
	_T("x -j"),							// none
	_T("x"),							// incfol
	_T("x -j -n"),						// new-or-notex
	_T("x -n"),							// incfol + new-or-notex
};

static LPCTSTR szTarCmd[] = {
	_T("--use-directory=0 -xfo %s %s"),	// none
	_T("-xfo %s %s"),					// incfol
	_T("--use-directory=0 -xfo %s %s"),	// new-or-notex
	_T("-xfo %s %s"),					// incfol + new-or-notex
};

/* --- unrar32.dll 
static LPCTSTR szRarCmd[] = {
	_T("-e %s %s"),						// none
	_T("-x %s %s"),						// incfol
	_T("-e -u %s %s"),					// new-or-notex
	_T("-x -u %s %s"),					// incfol + new-or-notex
};
--- */
static LPCTSTR szRarCmd[] = {
	_T("x -j %s %s"),					// none
	_T("x %s %s"),						// incfol
	_T("x -j -n %s %s")					// new-or-notex
	_T("x -n %s %s"),					// incfol + new-or-notex
};

static LPCTSTR szBgaCmd[] = {
	_T("x -a -j %s %s"),				// none
	_T("x -a %s %s"),					// incfol
	_T("x -a -j -n %s %s"),				// new-or-notex
	_T("x -a -n %s %s"),				// incfol + new-or-notex
};

static LPCTSTR szYz1Cmd[] = {
	_T("x %s %s"),						// none
	_T("x %s %s"),						// incfol
	_T("x %s %s"),						// new-or-notex
	_T("x %s %s"),						// incfol + new-or-notex
};

static LPCTSTR szGcaCmd[] = {
	_T("-e -j %s %s"),					// none
	_T("-e %s %s"),						// incfol
	_T("-e -j -n %s %s"),				// new-or-notex
	_T("-e -n %s %s"),					// incfol + new-or-notex
};

static LPCTSTR sz7ZipCmd[] = {
	_T("e %s -aoa -o%s"),				// none
	_T("x %s -aoa -o%s"),				// incfol
	_T("e %s -aos -o%s"),				// new-or-notex
	_T("x %s -aos -o%s"),				// incfol + new-or-notex
};

////////////////////////////////////////////////////////////////////////
// CUnpackDlg

class CUnpackDlg : public CDialogImpl<CUnpackDlg>
{
public:
	enum { IDD = IDD_UNPACKDLG };
	UNPACKDATA* m_pud;

	CUnpackDlg() : m_pud(NULL) { }

/* -----
	INT_PTR DoModalEx(HWND hWndParent = ::GetActiveWindow(), LPARAM param = NULL)
	{
		INT_PTR res = IDCANCEL;
		if (::IsWindow(hWndParent) && ::IsWindowVisible(hWndParent))
			res = DoModal(hWndParent, param);
		else {
			CDialogTemplate dt(MAKEINTRESOURCE(IDD));
			if (!dt.IsNull()) {
				dt.ModifyStyle(0, WS_MINIMIZEBOX | WS_SYSMENU);
				dt.ModifyExStyle(0, WS_EX_APPWINDOW);

				_AtlWinModule.AddCreateWndData(&m_thunk.cd, (ATL::CDialogImplBase*)this);
#ifdef _DEBUG
				m_bModal = true;
#endif //_DEBUG
				res = ::DialogBoxIndirectParam(_AtlBaseModule.GetModuleInstance(), dt, hWndParent, StartDialogProc, param);
			}
		}
		return res;
	}
---- */

BEGIN_MSG_MAP(CUnpackDlg)
	ON_WM_INITDIALOG()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_CLEAR, OnClear)
	ON_CBN_SELCHANGE(IDC_EDIT, OnSelchange)
	ON_NOTIFY(FTWN_SELCHANGED, IDC_TREE, OnNotifyTreeChanged)
	ON_NOTIFY(FTWN_QUERYHIDDEN, IDC_TREE, OnNotifyTreeQueryHidden)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	__inline void OnCancel() { EndDialog(IDCANCEL); }
	void OnClear();
	void OnSelchange();
	void OnNotifyTreeChanged(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyTreeQueryHidden(NMHDR* pNmhdr, LRESULT* pResult);
	__inline void OnDestroy() { RemoveDialogIcon(m_hWnd); }

};


BOOL CUnpackDlg::OnInitDialog(LPARAM lParam)
{
	if (!m_pud) {
		EndDialog(IDCANCEL);
		return TRUE;
	}
	CenterWindow();
	SetDialogIcon(m_hWnd, IDI_EXTRACT);

	CComboBox combo = GetDlgItem(IDC_EDIT);
	combo.LimitText(MAX_PATH - 1);
	RegLoadHistoryEx(combo, IDS_SEC_OPERATION, IDS_KEY_HISTORY, MAXACCESS);

	// オートコンプリート
	SHAutoComplete(combo.GetDlgItem(1001), SHACF_AUTOSUGGEST_FORCE_ON | SHACF_FILESYS_DIRS);

	CheckDlgButton(IDC_INCFOLDER, TRUE);
	CheckDlgButton(IDC_NEWORNOTEXIST, FALSE);
	if (!ISARCBIT(ARC_CAB, m_pud->arcbits)) GetDlgItem(IDC_VOLEXTRACT).EnableWindow(FALSE);
	if (ISARCBIT(ARC_ISH, m_pud->arcbits)) GetDlgItem(IDC_INCFOLDER).EnableWindow(FALSE);
	if (!ISARCBIT(ARC_ZIP, m_pud->arcbits)) GetDlgItem(IDC_PASSWORD).EnableWindow(FALSE);

	TCHAR path[MAX_PATH];
	lstrcpy(path, m_pud->szPath);

	int nStat = GetRegValue(IDS_SEC_OPERATION, IDS_KEY_STATUS, 0);

	TCHAR buf[MAX_PATH];
	if (GetRegText(IDS_SEC_OPERATION, IDS_KEY_PLACEFOLDER, buf, ARRAYSIZE(buf)) > 0)
		lstrcpy(path, buf);
	combo.SetWindowText(path);

	CFolderTreeCtrl tree = GetDlgItem(IDC_TREE);
	tree.SelectByPath(path);
	return TRUE;
}

void CUnpackDlg::OnOK()
{
	TCHAR buf[MAX_PATH * 2], w[MAX_PATH * 2];
	CComboBox combo = GetDlgItem(IDC_EDIT);
	if (combo.GetWindowText(buf, ARRAYSIZE(buf)) < 1) {
		MessageBeep(0);
		combo.SetFocus();
		return;
	}

	PathBuild(buf, m_pud->szPath, w);
	lstrcpy(m_pud->szPath, w);
	m_pud->bNamedir = (IsDlgButtonChecked(IDC_MKFOLDER) != FALSE) ? true : false;
	m_pud->bMultivol = (IsDlgButtonChecked(IDC_VOLEXTRACT) != FALSE) ? true : false;
	m_pud->nStat = 0;
	m_pud->nStat |= IsDlgButtonChecked(IDC_INCFOLDER) ? UNPACK_INCLUDEFOLDER : 0;
	m_pud->nStat |= IsDlgButtonChecked(IDC_NEWORNOTEXIST) ? UNPACK_NEWORNOTEXIST : 0;

	// コマンドラインパスワード
	if (GetDlgItem(IDC_PASSWORD).IsWindowEnabled())
		GetDlgItemText(IDC_PASSWORD, m_pud->szPassword, ARRAYSIZE(m_pud->szPassword));

	RegSaveHistoryEx(IDS_SEC_OPERATION, IDS_KEY_HISTORY, MAXACCESS, buf);
	EndDialog(IDOK);
}

void CUnpackDlg::OnClear()
{
	RegClearHistoryEx(GetDlgItem(IDC_EDIT), IDS_SEC_OPERATION, IDS_KEY_HISTORY);
}

void CUnpackDlg::OnSelchange()
{
	CComboBox combo = GetDlgItem(IDC_EDIT);
	int sel = combo.GetCurSel();
	if (sel >= 0) {
		TCHAR buf[MAX_PATH];
		if (combo.GetLBText(sel, buf) > 0) {
			CFolderTreeCtrl tree = GetDlgItem(IDC_TREE);
			tree.SelectByPath(buf);
		}
	}
}

void CUnpackDlg::OnNotifyTreeChanged(NMHDR*, LRESULT*)
{
	CFolderTreeCtrl tree = GetDlgItem(IDC_TREE);
	DWORD attr;
	LPITEMIDLIST pidl = tree.GetSelItem(&attr);
	if (pidl) {
		TCHAR buf[MAX_PATH * 2];
		if (!SHGetPathFromIDList(pidl, buf))
			buf[0] = '\0';
		else if (attr & SFGAO_LINK) {
			// ターゲットフォルダがリンク属性を持っていることがあるので
			// 実際のリンク先を取得する
			TCHAR target[MAX_PATH * 2];
			if (GetLinkTarget(buf, target, ARRAYSIZE(target)))
				lstrcpy(buf, target);
		}
		GetDlgItem(IDC_EDIT).SetWindowText(buf);
	}
}

void CUnpackDlg::OnNotifyTreeQueryHidden(NMHDR*, LRESULT* pResult)
{
	*pResult = FTH_SYSHIDDEN;
}


////////////////////////////////////////////////////////////////////////
//

BOOL __fastcall GetUnpackInfo(HWND hwnd, UNPACKDATA* pud)
{
	CUnpackDlg dlg;
	dlg.m_pud = pud;
//@	return (dlg.DoModalEx(hwnd) == IDOK);
	return (dlg.DoModal(hwnd) == IDOK);
}

BOOL __fastcall MakeUnpackCommand(int arctype, UNPACKDATA* pud, LPTSTR pszCmd, int cchMax)
{
	CString cmd;
	switch (arctype) {
	case ARC_ZIP:
		cmd = szZipCmd[pud->nStat];
		if (pud->szPassword[0])
			cmd.AppendFormat(_T(" \"-P%s\""), pud->szPassword);
		cmd += szPathForm;
		break;

	case ARC_CAB:
		cmd = szCabCmd[pud->nStat];
		if (pud->bMultivol)
			cmd += _T(" -k");
		cmd += szPathForm;
		break;

	case ARC_LZH:
		cmd = szLzhCmd[pud->nStat];
		break;

	case ARC_GCA:
		cmd = szGcaCmd[pud->nStat];
		break;

	case ARC_TAR:
		cmd = szTarCmd[pud->nStat];
		break;

	case ARC_RAR:
		cmd = szRarCmd[pud->nStat];
		break;

	case ARC_BGA:
		cmd = szBgaCmd[pud->nStat];
		break;

	case ARC_ARJ:
		cmd = szArjCmd[pud->nStat];
		break;

	case ARC_YZ1:
		cmd = szYz1Cmd[pud->nStat];
		break;

	case ARC_ISO:
	case ARC_RPM:
	case ARC_7ZIP:
		cmd = sz7ZipCmd[pud->nStat];
		break;

	case ARC_ISH:
		cmd = szIshCmd[pud->nStat];
		break;

	default:
		return FALSE;
	}
	lstrcpyn(pszCmd, cmd, cchMax);
	return TRUE;
}
