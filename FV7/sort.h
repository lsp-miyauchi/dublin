////////////////////////////////////////////////////////////////////////
// sort.h

#pragma once
#ifndef __SORT_H__
#define __SORT_H__

class CSortDlg : public CDialogImpl<CSortDlg>
{
public:
	CSimpleArray<CString> m_arrSortTypes;
	int m_sortindex;
	int m_sortseq;

public:
	CSortDlg() { }
	~CSortDlg() { }

	enum { IDD = IDD_SORT };

	inline void AddSortType(LPCTSTR pszTypeName)
	{
		m_arrSortTypes.Add(pszTypeName);
	}

BEGIN_MSG_MAP(CSortDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		static TCHAR prx[] = _T("NSTEAO");

		CenterWindow();
		int i, cnt = min(6, m_arrSortTypes.GetSize());
		for (i = 0; i < cnt; i++) {
			CString str;
			str.Format(_T("%sでソート(&%C)"), m_arrSortTypes[i].GetString(), prx[i]);
			SetDlgItemText(IDC_SORT1 + i, str);
			if (m_sortindex == i)
				CheckDlgButton(IDC_SORT1 + i, TRUE);
		}
		for ( ; i < 6; i++)
			GetDlgItem(IDC_SORT1 + i).ShowWindow(SW_HIDE);

		if (m_sortindex == SORT_NONE)
			CheckDlgButton(IDC_SORT_NONE, TRUE);

		CheckDlgButton((m_sortseq == SORT_UP) ? IDC_SORT_UP : IDC_SORT_DOWN, TRUE);
		return TRUE;
	}

	BOOL OnHelpInfo(HELPINFO* pInfo) { fvApp.HelpTopic(NULL, HB_DLG, IDD); }

	void OnOK()
	{
		if (IsDlgButtonChecked(IDC_SORT_NONE))
			m_sortindex = -1;
		else {
			int i, cnt = min(6, m_arrSortTypes.GetSize());
			for (i = 0; i < cnt; i++) {
				if (IsDlgButtonChecked(IDC_SORT1 + i)) {
					m_sortindex = i;
					break;
				}
			}
			m_sortseq = (IsDlgButtonChecked(IDC_SORT_UP) ? SORT_UP : SORT_DOWN);
		}
		EndDialog(IDOK);
	}

	void OnCancel() { EndDialog(IDCANCEL); }
};



#endif		// __SORT_H__
