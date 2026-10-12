////////////////////////////////////////////////////////////
// findstring.h

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CFindStringDlg ダイアログ

class CFindStringDlg : public CDialogImpl<CFindStringDlg>
{
public:
	CFind* m_pFinder;
	CString m_strSearchFor;
	bool m_bSearchForward;

	CFindStringDlg()
		: m_pFinder(NULL)
		, m_bSearchForward(true)
	{ }
	~CFindStringDlg()
	{ }

	enum { IDD = IDD_FIND };

BEGIN_MSG_MAP(CFindStringDlg)
	ON_WM_INITDIALOG()
	MESSAGE_HANDLER(WM_HELP, OnHelpCall)
	ON_BN_CLICKED(IDC_FINDNEXT, OnFindNext)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_FINDPREV, OnFindPrev)
	ON_BN_CLICKED(IDC_USEJRE, OnUseJre)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	LRESULT OnHelpCall(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnFindNext();
	void OnFindPrev();
	void OnUseJre();
	void OnHelpCall();

	BOOL Checkup();
};


/////////////////////////////////////////////////////////////////////////////
// CReplaceStringDlg ダイアログ

class CReplaceStringDlg : public CDialogImpl<CReplaceStringDlg>
{
public:
	CReplaceStringDlg() { m_pFinder = NULL; }
	~CReplaceStringDlg() { }

	enum { IDD = IDD_REPLACE };

	CFind* m_pFinder;

BEGIN_MSG_MAP(CReplaceStringDlg)
	ON_WM_INITDIALOG()
	MESSAGE_HANDLER(WM_HELP, OnHelpCall)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_REPLACEALL, OnReplaceAll)
	ON_BN_CLICKED(IDC_USEJRE, OnUseJre)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	LRESULT OnHelpCall(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnReplaceAll();
	void OnUseJre();
	void OnHelpCall();

	BOOL Checkup();
};


/////////////////////////////////////////////////////////////////////////////
// CReplaceProg ダイアログ

class CReplaceProg : public CDialogImpl<CReplaceProg>
{
public:
	CReplaceProg() { }
	~CReplaceProg() { }

	enum { IDD = IDD_REPLACEPROG };

BEGIN_MSG_MAP(CReplaceProg)
	ON_WM_INITDIALOG()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_REPLACEALL, OnReplaceAll)
	ON_BN_CLICKED(IDC_SKIP, OnSkip)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CRect rc;
		GetParent().GetWindowRect(&rc);
		SetWindowPos(NULL, rc.left + 10, rc.top + 10, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		return TRUE;
	}
	void OnDestroy() { GetParent().SendMessage(nRM_FINDNOTIFY, REP_EXIT, 0); }
	void OnOK() { GetParent().SendMessage(nRM_FINDNOTIFY, REP_REPLACE, 0); }
	void OnSkip() { GetParent().SendMessage(nRM_FINDNOTIFY, REP_SKIP, 0); }
	void OnCancel() { DestroyWindow(); }
	void OnReplaceAll()
	{
		ShowWindow(SW_HIDE);
		UpdateWindow();
		GetParent().SendMessage(nRM_FINDNOTIFY, REP_REPLACEALL, 0);
		DestroyWindow();
	}
};

/////////////////////////////////////////////////////////////////////////////
// CFunctionListDlg ダイアログ

class CFunctionListDlg : public CDialogImpl<CFunctionListDlg>, public CDialogResize<CFunctionListDlg>
{
public:
	CFunctionListDlg()
		: m_map(NULL)
		, m_curline(0)
	{
	}
	~CFunctionListDlg() { }

	enum { IDD = IDD_FUNCLIST };

	CSimpleMap<ULONG, CString>* m_map;
	ULONG m_curline;
	CListViewCtrl m_list;

BEGIN_MSG_MAP(CFunctionListDlg)
	ON_WM_INITDIALOG()
	CHAIN_MSG_MAP(CDialogResize<CFunctionListDlg>)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnListDblClick)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CFunctionListDlg)
	DLGRESIZE_CONTROL(IDC_LIST,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDOK,				DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_X)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		if (!m_map) {
			EndDialog(IDCANCEL);
			return TRUE;
		}

		DlgResize_Init();
		CenterWindow();

		m_list = GetDlgItem(IDC_LIST);
		m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | m_list.GetExtendedListViewStyle());
		m_list.InsertColumn(0, _T(""), LVCFMT_RIGHT, -1, 0);
		m_list.InsertColumn(1, _T(""), LVCFMT_LEFT, -1, 1);

		int sel = 0;
		CString str;
		int i, cnt = m_map->GetSize();
		for (i = 0; i < cnt; i++) {
			// 行番号
			ULONG n = m_map->GetKeyAt(i);
			if (n <= m_curline) {
				sel = i;
			}
			str.Format(_T("%d"), n);
			int index = m_list.InsertItem(i, str);
			// 関数名
			str = m_map->GetValueAt(i);
			m_list.SetItemText(index, 1, str);
		}
		m_list.SetColumnWidth(0, LVSCW_AUTOSIZE);
		m_list.SetColumnWidth(1, LVSCW_AUTOSIZE);
		m_list.SelectItem(sel);
		return TRUE;
	}
	void OnOK()
	{
		if (m_map->GetSize() > 0) {
			int index = m_list.GetSelectedIndex();
			m_curline = m_map->GetKeyAt(index);
			EndDialog(IDOK);
		}
	}
	void OnCancel()
	{
		EndDialog(IDCANCEL);
	}
	void OnListDblClick(LPNMHDR pnmh, LRESULT* pResult)
	{
		CPoint pt;
		GetCursorPos(&pt);
		m_list.ScreenToClient(&pt);
		UINT flags;
		int index = m_list.HitTest(pt, &flags);
		if (index >= 0 && (flags & LVHT_ONITEM)) {
			m_curline = m_map->GetKeyAt(index);
			EndDialog(IDOK);
		}
	}
};
