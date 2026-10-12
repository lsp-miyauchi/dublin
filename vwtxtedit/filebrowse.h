//////////////////////////////////////////////////////////////////////
// filebrowse.h

#pragma once
#include "resource.h"
#include "textfile.h"

#define IDC_FILETYPE_COMBOBOX			1136


//////////////////////////////////////////////////////////////////////
// for open

class CCharSetOpenDlg : public CDialogImpl<CCharSetOpenDlg>
{
public:
	CCharSetOpenDlg() { m_charset = CS_AUTODETECT; }
	~CCharSetOpenDlg() { }

	enum { IDD = IDD_FILEOPEN };
	CCharSetList m_list;
	CSimpleArray<int> m_arrcs;
	int m_charset;

BEGIN_MSG_MAP(CCharSetOpenDlg)
	ON_WM_INITDIALOG()
	ON_CBN_SELCHANGE(IDC_CHARSET, OnSelChangeCharSet);
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CComboBox combo = GetDlgItem(IDC_CHARSET);
		int cnt = m_list.GetSize();
		int sel = 0;
		for (int i = 0; i < cnt; i++) {
			if (m_list[i].type & 2) {
				combo.AddString(m_list[i].longName);
				m_arrcs.Add(m_list[i].ID);
				if (m_charset == m_list[i].ID)
					sel = m_arrcs.GetSize() - 1;
			}
		}
		combo.SetCurSel(sel);
		return TRUE;
	}

	void OnSelChangeCharSet()
	{
		CComboBox combo = GetDlgItem(IDC_CHARSET);
		m_charset = m_arrcs[combo.GetCurSel()];
	}
};

//////////////////////////////////////////////////////////////////////
// for save

class CCharSetSaveDlg : public CDialogImpl<CCharSetSaveDlg>
{
public:
	CCharSetSaveDlg() { m_charset = CS_AUTODETECT; }
	~CCharSetSaveDlg() { }

	enum { IDD = IDD_FILESAVE };
	CCharSetList m_list;
	CSimpleArray<int> m_arrcs;
	CSimpleArray<int> m_arrlb;
	int m_charset;
	int m_linebreak;

BEGIN_MSG_MAP(CCharSetSaveDlg)
	ON_WM_INITDIALOG()
	ON_CBN_SELCHANGE(IDC_CHARSET, OnSelChangeCharSet);
	ON_CBN_SELCHANGE(IDC_LINEBREAK, OnSelChangeLineBreak);
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CComboBox combo = GetDlgItem(IDC_CHARSET);
		int cnt = m_list.GetSize();
		int sel = 0;
		for (int i = 0; i < cnt; i++) {
			if (m_list[i].ID != CS_AUTODETECT && m_list[i].type & CSF_SAVE) {
				combo.AddString(m_list[i].longName);
				m_arrcs.Add(m_list[i].ID);
				if (m_charset == m_list[i].ID)
					sel = m_arrcs.GetSize() - 1;
			}
		}
		combo.SetCurSel(sel);

		m_arrlb.Add(CRLF);
		m_arrlb.Add(CR);
		m_arrlb.Add(LF);

		combo = GetDlgItem(IDC_LINEBREAK);
		InitDlgCtrl(combo, IDS_COMBOLIST_LINEBREAK);
		combo.SetCurSel(m_arrlb.Find(m_linebreak));

		return TRUE;
	}

	void OnSelChangeCharSet()
	{
		CComboBox combo = GetDlgItem(IDC_CHARSET);
		m_charset = m_arrcs[combo.GetCurSel()];
	}

	void OnSelChangeLineBreak()
	{
		CComboBox combo = GetDlgItem(IDC_LINEBREAK);
		m_linebreak = m_arrlb[combo.GetCurSel()];
	}
};

//////////////////////////////////////////////////////////////////////
//

class CFileOpenSaveDialog : public CFileDialogImpl<CFileOpenSaveDialog>
{
public:
	CFileOpenSaveDialog(BOOL bOpenFileDialog,
		int charset = CS_AUTODETECT,
		int linebreak = CRLF,
		LPCTSTR lpszFileName = NULL,
		LPCTSTR lpszFilter = NULL,
		HWND hWndParent = NULL)
		: CFileDialogImpl<CFileOpenSaveDialog>(bOpenFileDialog, NULL, lpszFileName, OFN_HIDEREADONLY | OFN_EXPLORER, lpszFilter, hWndParent)
	{
		m_bOpenMode = bOpenFileDialog;
		m_open.m_charset = charset;
		m_save.m_charset = charset;
		m_save.m_linebreak = linebreak;
	}

	BOOL m_bOpenMode;
	CCharSetOpenDlg m_open;
	CCharSetSaveDlg m_save;

	BEGIN_MSG_MAP(CFileOpenSaveDialog)
		CHAIN_MSG_MAP(CFileDialogImpl<CFileOpenSaveDialog>)
	END_MSG_MAP()

	void OnInitDone(LPOFNOTIFY /*lpon*/)
	{
		CRect rc;
		CRect rcc;
		CWindow child;
		CWindow par = GetParent();
		par.GetWindowRect(&rc);
		if (m_bOpenMode)
			child = m_open.Create(par);
		else
			child = m_save.Create(par);
		child.GetWindowRect(&rcc);
		rc.bottom += rcc.Height();
		par.SetWindowPos(NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOMOVE | SWP_NOZORDER);

		int x = (rc.Width() - rcc.Width()) / 2;
		par.GetDlgItem(IDC_FILETYPE_COMBOBOX).GetWindowRect(&rcc);
		par.ScreenToClient(&rcc);

		CRect rcw;
		child.GetDlgItem(IDC_CHARSET).GetWindowRect(&rcw);
		child.ScreenToClient(&rcw);

		child.SetWindowPos(NULL, rcc.left - rcw.left, rcc.bottom, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	bool GetSelectOpenFileName(CString& name, int& charset)
	{
		if (!m_bOpenMode) return false;

		name = m_szFileName;
		charset = m_open.m_charset;
		return true;
	}

	bool GetSelectSaveFileName(CString& name, int& charset, int& linebreak)
	{
		if (m_bOpenMode) return false;

		name = m_szFileName;
		charset = m_save.m_charset;
		linebreak = m_save.m_linebreak;
		return true;
	}
};

