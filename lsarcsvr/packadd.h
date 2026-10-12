// PAckAddDlg.h : CPackAddDlg の宣言

#pragma once
#ifndef __PACKADD_H__
#define __PACKADD_H__

#include "resource.h"       // メイン シンボル
#include "common.h"

typedef struct tagPACKADD {
	TCHAR arc_name[MAX_PATH];
	int path_mode;
} PACKADD;



// CPackAddDlg

class CPackAddDlg : public CDialogImpl<CPackAddDlg>
{
public:
	CString m_strArchive;
	CString m_strBasePath;
	int m_pathMode;
	HDROP m_hdrop;


public:
	CPackAddDlg()
		: m_pathMode(BASEDIR_NOPATH)
		, m_hdrop(NULL)
	{
	}

	~CPackAddDlg()
	{
	}

	enum { IDD = IDD_PACKADDDLG };

BEGIN_MSG_MAP(CPackAddDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_CBN_SELCHANGE(IDC_PATHMODE_LIST, OnSelChange)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow(GetParent());
		SetDlgItemText(IDC_ARCNAME, m_strArchive);

		CComboBox combo = GetDlgItem(IDC_PATHMODE_LIST);
		InitDlgCtrl(combo, IDS_PATHMODE_LIST);
		combo.SetCurSel(m_pathMode);

		if (!m_strBasePath.IsEmpty()) {
			TCHAR buf[MAX_PATH * 2];
			if (GetPathCommonPrefix(m_hdrop, buf))
				m_strBasePath = buf;
		}
		SetDlgItemText(IDC_PATH, m_strBasePath);

		OnSelChange();
		return TRUE;
	}

	void OnSelChange()
	{
		CComboBox combo = GetDlgItem(IDC_PATHMODE_LIST);
		BOOL bEnable = (combo.GetCurSel() == BASEDIR_RELATIVE);
		GetDlgItem(IDC_PATH).EnableWindow(bEnable);
	}

	void OnOK()
	{
		CComboBox combo = GetDlgItem(IDC_PATHMODE_LIST);
		m_pathMode = combo.GetCurSel();

		if (m_pathMode == BASEDIR_RELATIVE) {
			TCHAR buf[MAX_PATH * 2];
			CEdit edit = GetDlgItem(IDC_PATH);
			if (edit.GetWindowText(buf, ARRAYSIZE(buf)) < 1) {
				MessageBeep(0);
				edit.SetWindowText(m_strBasePath);
				edit.SetFocus();
				return;
			}
			PathAddBackslash(buf);

			TCHAR base[MAX_PATH * 2];
			lstrcpy(base, m_strBasePath);
			PathAddBackslash(base);

			if (lstrlen(base) < lstrlen(buf) ||
				StrCmpNI(buf, base, lstrlen(buf))) {

				MessageBeep(0);
				edit.SetWindowText(m_strBasePath);
				edit.SetFocus();
				return;
			}
			m_strBasePath = buf;
		}
		EndDialog(IDOK);
	}

	void OnCancel() { EndDialog(IDCANCEL); }
};

#endif		// __PACKADD_H__
