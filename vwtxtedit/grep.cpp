////////////////////////////////////////////////////////////
// grep.cpp

#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "grep.h"
#include "reg.h"

#define MAX_HISTORY			16

BOOL CGrepDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	HINSTANCE hInst = ATL::_AtlBaseModule.GetResourceInstance();
	HICON hic;
	hic = (HICON)LoadImage(hInst, _MKIR(IDI_GREP), IMAGE_ICON,
							GetSystemMetrics(SM_CXSMICON),
							GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	SetIcon(hic, FALSE);
	hic = (HICON)LoadImage(hInst, _MKIR(IDI_GREP), IMAGE_ICON,
							GetSystemMetrics(SM_CXICON),
							GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	SetIcon(hic, TRUE);

	CFind* pFinder = m_pGrep->GetFinder();
	SetDlgItemText(IDC_TEXT, pFinder->m_strText);
	if (pFinder->m_stat & FIS_CASE)	CheckDlgButton(IDC_CAPS, TRUE);
	if (pFinder->m_stat & FIS_JRE)	CheckDlgButton(IDC_USEJRE, TRUE);
	CheckDlgButton(IDC_SUBDIR, m_pGrep->m_bSubdir);

	// 検索文字列ヒストリの設定
	CAppReg reg;
	CComboBox combo = GetDlgItem(IDC_TEXT);
	reg.LoadComboHistory(combo, IDS_SEC_SETTING, IDS_KEY_FINDSTRING, MAX_HISTORY);

	// フィルタ文字列ヒストリの設定
	combo = GetDlgItem(IDC_FILTER);
	reg.LoadComboHistory(combo, IDS_SEC_SETTING, IDS_KEY_FINDFILTER, MAX_HISTORY);
	if (combo.GetCount() > 0)
		combo.SetCurSel(0);

	OnUseJre();

	TCHAR buf[MAX_PATH * 2];
	if (m_pGrep->m_path)
		lstrcpy(buf, m_pGrep->m_path);
	else
		GetCurrentDirectory(ARRAYSIZE(buf), buf);
	PathSetDlgItemPath(m_hWnd, IDC_PATH, buf);

	CFolderTreeCtrl tree = GetDlgItem(IDC_TREE);
	tree.SelectByPath(buf);

	return TRUE;
}

void CGrepDlg::OnDestroy()
{
	HICON hic = GetIcon(TRUE);
	if (hic) DestroyIcon(hic);
	hic = GetIcon(FALSE);
	if (hic) DestroyIcon(hic);
}

LRESULT CGrepDlg::OnHelpCall(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	HELPINFO* phi = (HELPINFO*)lParam;
	if (phi->iContextType == HELPINFO_WINDOW && phi->iCtrlId) {
		UINT id = 0;
		if (phi->iCtrlId == IDC_TEXT)			id = IDS_HELP_GREPSTRING;
		else if (phi->iCtrlId == IDC_FILTER)	id = IDS_HELP_GREPFILTER;
		else if (phi->iCtrlId == IDC_CAPS)		id = IDS_HELP_GREPCASE;
		else if (phi->iCtrlId == IDC_USEJRE)	ResHelp(m_hWnd, IDR_HTML_REGEXP);
		else if (phi->iCtrlId == IDC_SUBDIR)	id = IDS_HELP_GREPSUBDIR;
		if (id)
			PopupHint(m_hWnd, id);
		bHandled = TRUE;
	}
	return 0;
}

void CGrepDlg::OnUseJre()
{
	BOOL b = IsDlgButtonChecked(IDC_USEJRE);
	GetDlgItem(IDC_CAPS).EnableWindow(!b);
}

void CGrepDlg::OnTreeChanged()
{
	CFolderTreeCtrl tree = GetDlgItem(IDC_TREE);
	DWORD attr;
	LPITEMIDLIST pidl = tree.GetSelItem(&attr);
	if (pidl) {
		TCHAR buf[MAX_PATH * 2];
		if (!SHGetPathFromIDList(pidl, buf))
			buf[0] = '\0';

		PathSetDlgItemPath(m_hWnd, IDC_PATH, buf);
		lstrcpy(m_pGrep->m_path, buf);
		GetDlgItem(IDOK).EnableWindow(buf[0] != '\0');
	}
}

void CGrepDlg::OnOK()
{
	CComboBox comboText = GetDlgItem(IDC_TEXT);
	CComboBox comboFilter = GetDlgItem(IDC_FILTER);
	TCHAR str[MAX_PATH], flt[MAX_PATH];
	if (comboText.GetWindowText(str, ARRAYSIZE(str)) < 1) {
		MessageBeep(0);
		return;
	}
	if (comboFilter.GetWindowText(flt, ARRAYSIZE(flt)) < 1) {
		MessageBeep(0);
		return;
	}
	lstrcpy(m_pGrep->m_filter, flt);
	m_pGrep->m_pFinder->SetText(str);
	m_pGrep->m_bSubdir = IsDlgButtonChecked(IDC_SUBDIR);
	m_pGrep->m_pFinder->m_stat = FIS_NEXT;
	if (IsDlgButtonChecked(IDC_CAPS))
		m_pGrep->m_pFinder->m_stat |= FIS_CASE;
	if (IsDlgButtonChecked(IDC_USEJRE)) {
		// 正規表現の確認
		CAtlRegExp<> jre;
		REParseError status = jre.Parse(m_pGrep->m_pFinder->m_strText);
		if (REPARSE_ERROR_OK != status) {
			MessageBeep(0);
			comboText.SetFocus();
			return;
		}
		m_pGrep->m_pFinder->m_stat |= FIS_JRE;
	}

	// 検索文字列ヒストリの保存
	CAppReg reg;
	reg.SaveComboHistory(comboText, IDS_SEC_SETTING, IDS_KEY_FINDSTRING, MAX_HISTORY, str);
	// フィルタ文字列ヒストリの保存
	reg.SaveComboHistory(comboFilter, IDS_SEC_SETTING, IDS_KEY_FINDFILTER, MAX_HISTORY, flt);

	EndDialog(IDOK);
}

