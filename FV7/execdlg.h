// execdlg.h : header file
//

#pragma once

////////////////////////////////////////////////////////////////////////
//

class CExecuteInfo
{
public:
	CString m_strCommand;
	bool m_bWait;
	bool m_bExecType;
	bool m_bAsAdmin;

	CExecuteInfo()
		: m_bWait(false)
		, m_bExecType(false)
		, m_bAsAdmin(false)
	{
	}
};

////////////////////////////////////////////////////////////////////////
//

class CExecNameEqualHelper
{
public:
	static bool IsEqualKey(const LPCTSTR k1, const LPCTSTR k2) { return (lstrcmpi(k1, k2) == 0); }
	static bool IsEqualValue(const LPCTSTR k1, const LPCTSTR k2) { return (lstrcmpi(k1, k2) == 0); }
};

////////////////////////////////////////////////////////////////////////
//


class CExecDlg : public CDialogImpl<CExecDlg>, public CDialogResize<CExecDlg>
{
private:
	CLccSearchPathCtrl m_spc;
	CListViewCtrl m_list;
	CSimpleMap<CString, CString, CExecNameEqualHelper> m_mapPrograms;
	bool m_bIgnoreUpdateList:1;

public:
	CExecDlg()
		: m_pei(NULL)
		, m_bIgnoreUpdateList(false)
	{
	}

	enum { IDD = IDD_EXECUTE };

	CExecuteInfo* m_pei;

BEGIN_MSG_MAP(CExecDlg)
	ON_WM_INITDIALOG()
	CHAIN_MSG_MAP(CDialogResize<CExecDlg>)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_NOTIFY(SEN_GETTEXT, IDC_COMBO, OnNotifyBrowse)
	ON_NOTIFY(SEN_EDITCHANGE, IDC_COMBO, OnNotifyEditChange)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST, OnNotifyListViewGetDispInfo)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnNotifyListViewItemChanged)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CExecDlg)
	DLGRESIZE_CONTROL(IDC_COMBO,		DLSZ_SIZE_X)
	DLGRESIZE_CONTROL(IDC_LIST,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDOK,				DLSZ_MOVE_X)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_X)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyDropDown(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyEditChange(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyListViewGetDispInfo(NMHDR* pNmhdr, LRESULT* pResult);
	void OnNotifyListViewItemChanged(NMHDR* pNmhdr, LRESULT* pResult);

	void ListupStandardPrograms();
};

////////////////////////////////////////////////////////////////////////
//

BOOL CALLBACK ExecDlg(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
