#pragma once
#ifndef __SELRANGE_H__
#define __SELRANGE_H__

// selrange.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CSelRangeDlg ダイアログ

class CSelRangeDlg : public CDialogImpl<CSelRangeDlg>
{
public:
	CSelRangeDlg() { m_start = m_length = 0; }
	~CSelRangeDlg() { }

	enum { IDD = IDD_SELRANGE };

BEGIN_MSG_MAP(CJumpDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void Warn(int idControl)
	{
		MessageBeep(0);
		GetDlgItem(idControl).SetFocus();
	}

	DWORD_PTR m_start;
	size_t m_length;
};


#endif				// __SELRANGE_H__
