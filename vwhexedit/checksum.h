////////////////////////////////////////////////////////////////////////
// checksum.h
//

#pragma once
#ifndef __CHECKSUMDLG_H__
#define __CHECKSUMDLG_H__

/////////////////////////////////////////////////////////////////////////////
// CCheckSumDlg

class CCheckSumDlg : public CDialogImpl<CCheckSumDlg>
{
public:
	CSimpleMap<CString, CString> m_sum;

public:
	CCheckSumDlg() { }
	~CCheckSumDlg() { }

	enum { IDD = IDD_CHECKSUM };

BEGIN_MSG_MAP(CColorEditDialog)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();

		CListViewCtrl list(GetDlgItem(IDC_LIST));
		list.SetExtendedListViewStyle(list.GetExtendedListViewStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

		list.InsertColumn(0, _T("Method"), LVCFMT_LEFT, 8, 0);
		list.InsertColumn(1, _T("Result"), LVCFMT_LEFT, 20, 1);

		CString str;
		int cnt = m_sum.GetSize();
		for (int i = 0; i < cnt; i++) {
			str = m_sum.GetKeyAt(i);
			int index = list.InsertItem(i, str);
			str = m_sum.GetValueAt(i);
			list.SetItemText(index, 1, str);
		}
		list.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
		list.SetColumnWidth(1, LVSCW_AUTOSIZE);
		return TRUE;
	}
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnCopy()
	{
		CString str;
		int cnt = m_sum.GetSize();
		for (int i = 0; i < cnt; i++) {
			str += m_sum.GetKeyAt(i);
			str += _T(": ");
			str += m_sum.GetValueAt(i);
			str += _T("\n");
		}

		lccSetStringToClipboard(m_hWnd, str);
	}
};

#endif		// __CHECKSUMDLG_H__
