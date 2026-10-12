// wildcard.cpp : implementation file
//

#include "stdafx.h"
#include "fv7.h"
#include "wildcard.h"

////////////////////////////////////////////////////////////////////////
//

class CFilterNameDlg : public CDialogImpl<CFilterNameDlg>
{
public:
	CFilterSettingList& m_rlist;
	CString m_strName;
	CComboBox m_combo;

	enum { IDD = IDD_FILTERNAME };

	CFilterNameDlg(CFilterSettingList& rlist) : m_rlist(rlist)
	{
	}

BEGIN_MSG_MAP(CFilterNameDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();
		m_combo = GetDlgItem(IDC_COMBO);
		CString str;
		int cnt = m_rlist.GetSize();
		if (cnt > 0) {
			for (int i = 0; i < cnt; i++)
				m_combo.AddString(m_rlist[i].m_strName);
		}
		else {
			str.LoadString(IDS_UNDEFINED);
			m_combo.AddString(str);
		}
		return TRUE;
	}
	void OnOK()
	{
		if (m_combo.GetWindowText(m_strName) < 1) {
			MessageBeep(0);
			m_combo.SetFocus();
			return;
		}

		CString str;
		if (m_rlist.GetSize() > 0) {
			int index = m_rlist.FindName(m_strName);
			if (index >= 0) {
				str.Format(IDS_ASK_ITEM_OVERWRITE, (LPCTSTR)m_strName);
				CString tit; GetWindowText(tit);
				if (MessageBox(str, tit, MB_QYESNO) != IDYES)
					return;
			}
		}
		else {
			str.LoadString(IDS_UNDEFINED);
			if (!str.Compare(m_strName)) {
				MessageBeep(0);
				m_combo.SetFocus();
				return;
			}
		}
		EndDialog(IDOK);
	}
	void OnCancel() { EndDialog(IDCANCEL); }
};


////////////////////////////////////////////////////////////////////////
//


class CManageFilterDlg : public CDialogImpl<CManageFilterDlg>
{
public:
	CFilterSettingList& m_rlist;
	CListViewCtrl m_list;
	bool m_bUpdate:1;

	enum { IDD = IDD_MANAGE_FILTERSETTING };

	CManageFilterDlg(CFilterSettingList& rlist) : m_rlist(rlist), m_bUpdate(false)
	{
	}

BEGIN_MSG_MAP(CManageFilterDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDC_UP, OnItemMoveUp)
	ON_BN_CLICKED(IDC_DOWN, OnItemMoveDown)
	ON_BN_CLICKED(IDC_DELETE, OnItemDelete)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();
		m_list = GetDlgItem(IDC_LIST);

		CString str;
		str.LoadString(IDS_COLUMN_NAME);
		m_list.InsertColumn(0, str, LVCFMT_LEFT, 120, 0);
		str.LoadString(IDS_COLUMN_INFO);
		m_list.InsertColumn(1, str, LVCFMT_LEFT, 360, 0);

		int cnt = m_rlist.GetSize();
		for (int i = 0; i < cnt; i++) {
			CFilterSetting fs = m_rlist[i];
			int index = m_list.InsertItem(i, fs.GetName());
			m_list.SetItemText(index, 1, fs.label());
		}

		return TRUE;
	}

	void OnItemDelete()
	{
		int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
		if (sel < 0) return;

		CFilterSetting fs = m_rlist[sel];
		CString str; str.Format(IDS_ASK_ITEM_DELETE, fs.GetName());
		CString tit; GetWindowText(tit);
		if (MessageBox(str, tit, MB_QYESNO) == IDYES) {
			m_rlist.RemoveAt(sel);
			m_list.DeleteItem(sel);
			m_bUpdate = true;
		}
	}

	void OnItemMoveUp()
	{
		int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
		if (sel > 0) {
			CFilterSetting fss = m_rlist[sel];
			CFilterSetting fsd = m_rlist[sel - 1];
			m_rlist.SetAtIndex(sel, fsd);
			m_rlist.SetAtIndex(sel - 1, fss);
			m_list.SetItemText(sel, 0, fsd.GetName());
			m_list.SetItemText(sel, 1, fsd.label());
			m_list.SetItemText(sel - 1, 0, fss.GetName());
			m_list.SetItemText(sel - 1, 1, fss.label());
			m_list.SelectItem(sel - 1);
			m_bUpdate = true;
		}
	}

	void OnItemMoveDown()
	{
		int sel = m_list.GetNextItem(-1, LVNI_SELECTED);
		if (sel != -1 && sel < m_list.GetItemCount() - 1) {
			CFilterSetting fss = m_rlist[sel];
			CFilterSetting fsd = m_rlist[sel + 1];
			m_rlist.SetAtIndex(sel, fsd);
			m_rlist.SetAtIndex(sel + 1, fss);
			m_list.SetItemText(sel, 0, fsd.GetName());
			m_list.SetItemText(sel, 1, fsd.label());
			m_list.SetItemText(sel + 1, 0, fss.GetName());
			m_list.SetItemText(sel + 1, 1, fss.label());
			m_list.SelectItem(sel + 1);
			m_bUpdate = true;
		}
	}

	inline void OnCancel() { EndDialog(IDCANCEL); }
};

////////////////////////////////////////////////////////////////////////
//

bool SelectFilterSetting(HWND hwnd, bool bAddClear, CFilterSetting& fs, POINT point)
{
	CFilterSettingList list;
	list.LoadXML();

	// 項目数
	int numFilters = list.GetSize();
	if (numFilters < 1) return false;

	// リストをメニューに登録
	CMenu pop;
	pop.CreatePopupMenu();
	CString str;
	for (int i = 0; i < numFilters; i++) {
		str.Format(_T("%s(%$s)\t&%C"), list[i].GetName(), list[i].label(), i + ((i < 10) ? '0' : ('A' - 10)));
		pop.AppendMenu(MF_STRING, 1 + i, (LPCTSTR)str);
	}

	// 項目が存在すればセパレータを追加
	if (numFilters > 0)
		pop.AppendMenu(MF_SEPARATOR, (UINT_PTR)0, (LPCTSTR)NULL);
	// 解除コマンド
	if (bAddClear)
		pop.AppendMenu(MF_STRING, 9002, _T("解除(&X)"));
	// 管理メニュー追加
	if (numFilters > 0)
		pop.AppendMenu(MF_STRING, 9001, _T("リストの管理(&O)"));

	int id = pop.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, hwnd);
	pop.DestroyMenu();

	if (id > 0) {
		if (id < 9000) {
			// 設定をウィンドウに反映
			fs = list[id - 1];
			return true;
		}
		else if (id == 9001) {
			CManageFilterDlg dlg(list);
			dlg.DoModal();
			if (dlg.m_bUpdate) {
				list.SaveXML();
			}
		}
		else if (id == 9002) {
			fs.Clear();
			fs.m_strFilter = gszAst;
			return true;
		}
	}
	return false;
}


////////////////////////////////////////////////////////////////////////
//

typedef struct tagFILTERDIALOGINFO {
	UINT idDlgResource;
	UINT idRegKey;
	BOOL bUseRelease;				// if WildCard then TRUE
} FILTERDIALOGINFO;

#define MAXWILDCARDHISTORY			16

class CFilterDlg : public CDialogImpl<CFilterDlg>
{
private:
	CFilterSettingList m_settinglist;
	CSimpleArray<CFilterSetting> m_history;
	CComboBox m_combo;

public:
	enum { IDD = IDD_WILDCARD };

	CFilterDlg()
		: m_pfs(NULL)
		, m_callback(NULL)
		, m_idDlgResource(0)
		, m_idRegKey(0)
	{
	}

	CFilterSetting* m_pfs;
	APPLYFILTERPROC m_callback;
	UINT m_idDlgResource;
	UINT m_idRegKey;

BEGIN_MSG_MAP(CFilterDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_RELEASE, OnRelease)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
	ON_BN_CLICKED(IDC_LIST, OnSettingList)
	ON_CBN_SELCHANGE(IDC_WILDCARD, OnSelChange)
	ON_BN_CLICKED(IDC_FOLDER, OnChangeSetting)
	ON_BN_CLICKED(IDC_USEREGEXP, OnChangeSetting)
	ON_BN_CLICKED(IDC_CASESENSE, OnChangeSetting)
	ON_BN_CLICKED(IDC_UNMATCHFILTER, OnChangeSetting)
	ON_CBN_EDITCHANGE(IDC_WILDCARD, OnChangeSetting)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		FILTERDIALOGINFO* pfdi = (FILTERDIALOGINFO*)lParam;
		if (pfdi) {
			m_idDlgResource = pfdi->idDlgResource;
			m_idRegKey = pfdi->idRegKey;

			CString str; str.LoadString(m_idDlgResource);
			SetWindowText(str);

			if (!pfdi->bUseRelease)
				GetDlgItem(IDC_RELEASE).ShowWindow(SW_HIDE);
		}
		return InitDialog();
	}

	void OnOK()
	{
		if (CorrectSettings(*m_pfs, true)) {
			EndDialog(IDOK);
		}
	}

	void OnCancel() { EndDialog(IDCANCEL); }
	void OnRelease()
	{
		m_pfs->m_strFilter = gszAst;
		m_pfs->m_bUnmatchFilter = FALSE;
		m_pfs->m_bUseRegExp = ((gnFopStat & FOS_REXFINDER) != 0);
		m_pfs->m_bUseRegExp = FALSE;
		m_pfs->m_bCaseSense = FALSE;
		m_pfs->m_bIncludeFolder = FALSE;
		EndDialog(IDOK);
	}

	void OnSettingList()
	{
		CMenu pop;
		pop.CreatePopupMenu();

		// リストをメニューに登録
		CFilterSetting fs;
		int numFilters = m_settinglist.GetSize();
		CString str;
		for (int i = 0; i < numFilters; i++) {
			fs = m_settinglist[i];
			str.Format(_T("%s(%s)\t&%C"), fs.GetName(), fs.label(), i + ((i < 10) ? '0' : ('A' - 10)));
			pop.AppendMenu(MF_STRING, 1 + i, (LPCTSTR)str);
		}
		// 項目が存在すればセパレータを追加
		if (numFilters > 0)
			pop.AppendMenu(MF_SEPARATOR, (UINT_PTR)0, (LPCTSTR)NULL);
		// 管理メニュー追加
		str.LoadString(IDS_SELFILTER_ADD);
		pop.AppendMenu(MF_STRING, 9000, str.GetString());
		if (numFilters > 0) {
			str.LoadString(IDS_SELFILTER_MANAGE);
			pop.AppendMenu(MF_STRING, 9001, str.GetString());
		}
		CRect rc;
		GetDlgItem(IDC_LIST).GetWindowRect(&rc);
		int id = pop.TrackPopupMenu(TPM_RIGHTALIGN | TPM_RETURNCMD, rc.right, rc.bottom, m_hWnd);
		pop.DestroyMenu();

		if (id > 0) {
			if (id < 9000) {
				// 設定をウィンドウに反映
				fs = m_settinglist[id - 1];
				StoreSetting(fs);
			}
			else {
				if (id == 9000) {
					if (CorrectSettings(fs, true)) {
						CFilterNameDlg dlg(m_settinglist);
						if (dlg.DoModal() == IDOK) {
							fs.m_strName = dlg.m_strName;
							int index = m_settinglist.FindName(fs.m_strName);
							if (index < 0)
								m_settinglist.Add(fs);
							else
								m_settinglist.SetAtIndex(index, fs);
							m_settinglist.SaveXML();
						}
					}
				}
				else if (id == 9001) {
					CManageFilterDlg dlg(m_settinglist);
					dlg.DoModal();
					if (dlg.m_bUpdate) {
						str.LoadString(IDS_SEC_FILTERSETTING);
						m_settinglist.SaveXML();
					}
				}
			}
		}
	}

	void OnSelChange()
	{
		int sel = m_combo.GetCurSel();
		if (sel != CB_ERR) {
			StoreSetting(m_history[sel]);
		}
	}

	void OnChangeSetting()
	{
		// 必要な場合だけコールバックする
		CFilterSetting fs;
		if (m_callback) {
			if (CorrectSettings(fs, false))
				m_callback(&fs);
			else
				m_callback(NULL);
		}
	}

	void OnHelpCall() { fvApp.HelpTopic(NULL, HB_DLG, m_idDlgResource); }

	BOOL InitDialog()
	{
		CenterWindow();

		m_combo = GetDlgItem(IDC_WILDCARD);
		m_combo.LimitText(MAX_PATH - 1);
		// 既存の設定リスト
		m_settinglist.LoadXML();
		// 履歴
		LoadHistory();

		CheckDlgButton(IDC_UNMATCHFILTER, m_pfs->m_bUnmatchFilter);
		CheckDlgButton(IDC_FOLDER, m_pfs->m_bIncludeFolder);
		CheckDlgButton(IDC_USEREGEXP, m_pfs->m_bUseRegExp);
		CheckDlgButton(IDC_CASESENSE, m_pfs->m_bCaseSense);
		m_combo.SetWindowText(m_pfs->GetFilter());

		return TRUE;
	}

	bool CorrectSettings(CFilterSetting& fs, bool noNeedUI = true)
	{
		BOOL bUnmatchFilter = IsDlgButtonChecked(IDC_UNMATCHFILTER);
		BOOL bUseRegExp = IsDlgButtonChecked(IDC_USEREGEXP);
		CString wilds;
		if (m_combo.GetWindowText(wilds) < 1) {
			//		wilds = gszAst;
			if (!noNeedUI) {
				MessageBeep(0);
				m_combo.SetFocus();
			}
			return false;
		}
		else {
			if (!bUseRegExp && !lccUniqueFilter((LPTSTR)(LPCTSTR)wilds)) {
				if (!noNeedUI) {
					MessageBeep(0);
					m_combo.SetFocus();
				}
				return false;
			}
			if (!wilds.Compare(_T("*")))
				wilds = gszAst;
		}

		// 間違った設定は排除
		if (bUnmatchFilter && !wilds.Compare(gszAst)) {
			if (!noNeedUI) {
				MessageBeep(0);
				m_combo.SetFocus();
			}
			return false;
		}

		fs.m_strFilter = wilds;
		fs.m_bUnmatchFilter = bUnmatchFilter;
		fs.m_bUseRegExp = bUseRegExp;
		fs.m_bCaseSense = IsDlgButtonChecked(IDC_CASESENSE);
		fs.m_bIncludeFolder = IsDlgButtonChecked(IDC_FOLDER);
		return true;
	}

	void StoreSetting(const CFilterSetting& fs)
	{
		m_combo.SetWindowText(fs.m_strFilter);
		CheckDlgButton(IDC_UNMATCHFILTER, fs.m_bUnmatchFilter);
		CheckDlgButton(IDC_FOLDER, fs.m_bIncludeFolder);
		CheckDlgButton(IDC_USEREGEXP, fs.m_bUseRegExp);
		CheckDlgButton(IDC_CASESENSE, fs.m_bCaseSense);

		// 必要ならコールバック
		if (m_callback)
			m_callback(&fs);
	}

	static int LoadFilterHistory(_U_STRINGorID key, CSimpleArray<CFilterSetting>& list)
	{
		list.RemoveAll();

		void* pv = malloc(2048);
		ATLASSERT(pv != NULL);
		if (!pv) return 0;

		CString strKey;
		RegKeyCombine(IDS_SEC_HISTORY, key, strKey);

		CMRUProc mru;
		if (mru.Create(HKEY_CURRENT_USER, strKey, MAXWILDCARDHISTORY, TRUE)) {
			int n = mru.GetCount();
			for (int i = 0; i < n; i++) {
				int size = mru.Enum(i, NULL, 0);
				if (size > 0) {
					mru.Enum(i, pv, size);
					CComPtr<IStream> pstrm = SHCreateMemStream((BYTE*)pv, size);
					if (pstrm) {
						CFilterSetting fs;
						fs.Load(pstrm);
						list.Add(fs);
					}
				}
			}
			mru.Close();
		}
		free(pv);
		return list.GetSize();
	}

	void LoadHistory()
	{
		int cnt = CFilterDlg::LoadFilterHistory(m_idRegKey, m_history);
		for (int i = 0; i < cnt; i++) {
			m_combo.AddString(m_history[i].GetFilter());
		}
	}

	static void UpdateFilterHistory(_U_STRINGorID key, const CFilterSetting& fs)
	{
		CString strKey;
		RegKeyCombine(IDS_SEC_HISTORY, key, strKey);

		CMRUProc mru;
		if (mru.Create(HKEY_CURRENT_USER, strKey, MAXWILDCARDHISTORY, TRUE)) {
			CComPtr<IStream> pstrm;
			if (CreateStreamOnHGlobal(NULL, TRUE, &pstrm) == S_OK) {
				int size = fs.Save(pstrm);
				if (size > 0) {
					HGLOBAL hmem;
					if (GetHGlobalFromStream(pstrm, &hmem) == S_OK) {
						void* pv = GlobalLock(hmem);
						mru.AddBinary(pv, size);
						GlobalUnlock(hmem);
					}
				}
			}
			mru.Close();
		}
	}

	static void ResetFilterHistory(_U_STRINGorID key)
	{
		DeleteRegSection(IDS_SEC_HISTORY, key);
	}

	void SaveHistory()
	{
		UpdateFilterHistory(m_idRegKey, *m_pfs);
	}
};


/////////////////////////////////////////////////////////////////////
//

class CFilterSelectDlg : public CFilterDlg
{
public:
BEGIN_MSG_MAP(CFilterSelectDlg)
	CHAIN_MSG_MAP(CFilterDlg)
END_MSG_MAP()
};

/////////////////////////////////////////////////////////////////////
//

INT_PTR DoFilterSelect(HWND hWnd, CFilterSetting* filter, APPLYFILTERPROC callback)
{
	CFilterSelectDlg dlg;
	dlg.m_pfs = filter;
	dlg.m_callback = callback;

	FILTERDIALOGINFO fdi;
	fdi.idDlgResource = IDD_FILTERSELECT;
	fdi.idRegKey = IDS_KEY_FILTERSELECT;
	fdi.bUseRelease = FALSE;
	return dlg.DoModal(hWnd, (LPARAM)&fdi);
}

/////////////////////////////////////////////////////////////////////
//

class CFileFilterDlg : public CFilterDlg
{
public:
	CString m_strCurrentFilter;

BEGIN_MSG_MAP(CFileFilterDlg)
	MESSAGE_HANDLER(WM_INITDIALOG, OnHandleInitDialog)
	CHAIN_MSG_MAP(CFilterDlg)
END_MSG_MAP()

	LRESULT OnHandleInitDialog(UINT msg, WPARAM wp, LPARAM lp, BOOL& bHandled)
	{
		SetDlgItemText(IDC_TITLE, m_strCurrentFilter);
		bHandled = FALSE;
		return 0;
	}
};

/////////////////////////////////////////////////////////////////////
//

INT_PTR FileFilterDialog(HWND hWnd, CFilterSetting* filter, LPCTSTR pszCurrentFilter)
{
	CFileFilterDlg dlg;
	dlg.m_pfs = filter;
	dlg.m_strCurrentFilter = pszCurrentFilter;

	FILTERDIALOGINFO fdi;
	fdi.idDlgResource = IDD_WILDCARD;
	fdi.idRegKey = IDS_KEY_WILDCARD;
	fdi.bUseRelease = TRUE;
	return dlg.DoModal(hWnd, (LPARAM)&fdi);
}

int GetFilterHistory(CSimpleArray<CFilterSetting>& list)
{
	return CFilterDlg::LoadFilterHistory(IDS_KEY_WILDCARD, list);
}

void UpdateFilterHistory(CFilterSetting& fs)
{
	return CFilterDlg::UpdateFilterHistory(IDS_KEY_WILDCARD, fs);
}

void ResetFilterHistory()
{
	return CFilterDlg::ResetFilterHistory(IDS_KEY_WILDCARD);
}

