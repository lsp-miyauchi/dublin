/////////////////////////////////////////////////////////////////////////////
// iconpath.cpp アイコン取得 ダイアログ

#include "stdafx.h"
#include <atldlgs.h>
#include "fopstd.h"
#include "resource.h"


typedef struct tagICGINFO {
	TCHAR szIconPath[MAX_PATH * 2];
	int nIconIndex;
	BOOL bSmall;
} ICGINFO, *LPICGINFO;

class CHIconArray : public CSimpleArray<HICON>
{
public:
	~CHIconArray() { DeleteAll(); }
	void DeleteAll()
	{
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++) {
			HICON hic = (*this)[i];
			if (hic != NULL)
				DestroyIcon(hic);
		}
		RemoveAll();
	}
	void SetSize(int cnt)
	{
		DeleteAll();
		while (cnt > 0) {
			Add((HICON)NULL);
			cnt--;
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
// CIconPathDlg dialog

class CIconPathDlg : public CDialogImpl<CIconPathDlg>, public CDialogResize<CIconPathDlg>
{
private:
	CHIconArray m_iconSmall;
	CHIconArray m_iconLarge;
	CDrawListCtrl m_list;

public:
	CIconPathDlg() { memset(&m_info, 0, sizeof(ICGINFO)); }
	~CIconPathDlg() { }

	enum { IDD = IDD_ICONPATH };

BEGIN_MSG_MAP(CIconPathDlg)
	ON_WM_INITDIALOG()
	CHAIN_MSG_MAP(CDialogResize<CIconPathDlg>)
	ON_WM_DRAWITEM()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_CHECK, OnCheck)
	ON_NOTIFY(SEN_GETTEXT, IDC_EDIT, OnNotifyBrowse)
	ON_EN_KILLFOCUS(IDC_EDIT, OnUpdatePath)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CIconPathDlg)
	DLGRESIZE_CONTROL(IDC_EDIT,			DLSZ_SIZE_X)
	DLGRESIZE_CONTROL(IDC_CHECK,		DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDC_LIST,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDOK,				DLSZ_MOVE_X | DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_X | DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDrawItem(int idCtrl, LPDRAWITEMSTRUCT pdi);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnCheck();
	void OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult);
	void OnUpdatePath();

	void IconStamp(LPCTSTR pszName, int nIndex);

	ICGINFO m_info;
};

BOOL CIconPathDlg::OnInitDialog(LPARAM lParam)
{
	DlgResize_Init();

	m_list = GetDlgItem(IDC_LIST);
	CenterWindow();
	OnCheck();

	if (m_info.szIconPath[0]) {
		SetDlgItemText(IDC_EDIT, m_info.szIconPath);
		IconStamp(m_info.szIconPath, m_info.nIconIndex);
		m_list.SetFocus();
	}

	return FALSE;
}

void CIconPathDlg::OnDrawItem(int idCtrl, DRAWITEMSTRUCT* pdi)
{
	if (idCtrl != IDC_LIST) return;

	CRect rc = pdi->rcItem;
	int index  = (int)pdi->itemData;
	if (pdi->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) {
		// 選択状態の場合
		if (pdi->itemState & ODS_SELECTED)
			SetBkColor(pdi->hDC, GetSysColor(COLOR_HIGHLIGHT));
		else
			SetBkColor(pdi->hDC, GetSysColor(COLOR_WINDOW));
		ExtTextOut(pdi->hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

		HICON hic;
		int dx;
		if (m_info.bSmall) {
			dx = 16;
			hic = m_iconSmall[index];
		} else {
			dx = 32;
			hic = m_iconLarge[index];
		}
		DrawIconEx(pdi->hDC,
					rc.left + (rc.Width() - dx) / 2,
					rc.top + (rc.Height() - dx) / 2,
					hic, dx, dx, 0, NULL, DI_NORMAL);
	}
	if (GetFocus() == pdi->hwndItem && pdi->itemState & ODS_FOCUS)
		DrawFocusRect(pdi->hDC, &pdi->rcItem);
}

void CIconPathDlg::OnOK()
{
	TCHAR path[MAX_PATH * 2];
	if (GetDlgItemText(IDC_EDIT, path, ARRAYSIZE(path)) > 0) {
		if (!_pathfileexists(path)) {
			MessageBeep(0);
			GetDlgItem(IDC_EDIT).SetFocus();
			return;
		}

		int nIndex = m_list.GetCurSel();
		if (nIndex < 0) {
			MessageBeep(0);
			m_list.SetFocus();
			return;
		}
		lstrcpy(m_info.szIconPath, path);
		m_info.nIconIndex = nIndex;
	}
	else {
		m_info.szIconPath[0] = '\0';
		m_info.nIconIndex = -1;
	}
	EndDialog(IDOK);
}

void CIconPathDlg::OnCheck()
{
	m_info.bSmall = IsDlgButtonChecked(IDC_CHECK);
	CWindow list(GetDlgItem(IDC_LIST));
	int dx = m_info.bSmall ? 16 + 8 : 32 + 8;
	list.SendMessage(DLM_SETITEMSIZE, 0, MAKELPARAM(dx, dx));
}

void CIconPathDlg::OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;

	TCHAR buf[MAX_PATH * 2], path[MAX_PATH * 2];
	path[0] = '\0';
	if (m_info.szIconPath[0]) {
		lstrcpy(path, m_info.szIconPath);
		PathRemoveFileSpec(path);
	}
	if (!_pathisdirectory(path))
		GetCurrentDirectory(ARRAYSIZE(path), path);

	buf[0] = 0;
	if (FileBrowse(TRUE, m_hWnd, IDS_TITLE_ICONPATH, IDS_FILTER_ICONPATH, path, buf)) {
		lstrcpyn(psgt->pszText, buf, psgt->cchTextMax);
		IconStamp(buf, 0);
		*pResult = TRUE;
	}
}

void CIconPathDlg::OnUpdatePath()
{
	TCHAR buf[MAX_PATH * 2];
	if (GetDlgItemText(IDC_EDIT, buf, ARRAYSIZE(buf)) > 0 &&
		StrCmpI(m_info.szIconPath, buf) &&
		_pathfileexists(buf))
		IconStamp(buf, 0);
}

void CIconPathDlg::IconStamp(LPCTSTR pszName, int nIndex)
{
	m_list.ResetContent();
	m_iconSmall.DeleteAll();
	m_iconLarge.DeleteAll();
	int cnt = (int)(INT_PTR)ExtractIcon(RESINST, pszName, (UINT)-1);
	if (cnt < 1)
		return;

	m_iconSmall.SetSize(cnt);
	m_iconLarge.SetSize(cnt);
	m_list.SetRedraw(FALSE);
	ExtractIconEx(pszName, 0, m_iconLarge.GetData(), m_iconSmall.GetData(), cnt);
	for (int i = 0; i < cnt; i++)
		m_list.AddItem(IntToPtr(i));

	if (nIndex >= 0)
		m_list.SetCurSel(nIndex);
	m_list.SetRedraw(TRUE);
	m_list.Invalidate();

	lstrcpy(m_info.szIconPath, pszName);
}

//////////////////////////////////////////////////////////////////////
//

extern "C"
BOOL WINAPI fopGetIconPath(HWND hwnd, LPTSTR pszPath, int* pnIndex)
{
	CIconPathDlg dlg;
	if (*pszPath) {
		lstrcpy(dlg.m_info.szIconPath, pszPath);
		dlg.m_info.nIconIndex = *pnIndex;
	} else {
		HMODULE hmod = GetModuleHandle(gszShell32);
		if (hmod) {
			TCHAR buf[MAX_PATH * 2];
			GetModuleFileName(hmod, buf, ARRAYSIZE(buf));
			lstrcpy(dlg.m_info.szIconPath, buf);
			dlg.m_info.nIconIndex = 0;
		}
	}
	if (dlg.DoModal(hwnd) == IDOK) {
		lstrcpy(pszPath, dlg.m_info.szIconPath);
		*pnIndex = dlg.m_info.nIconIndex;
		return TRUE;
	}
	return FALSE;
}
