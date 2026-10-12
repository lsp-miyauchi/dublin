/////////////////////////////////////////////////////////////////////////
// sizeclip.h

#pragma once
#ifndef __SIZECLIP_H__
#define __SIZECLIP_H__

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CSizeClipDlg ダイアログ

class CSizeClipDlg : public CDialogImpl<CSizeClipDlg>
{
public:
	enum { IDD = IDD_SIZECLIP };

	CListViewCtrl m_list;
	CSize m_sizeClip;

BEGIN_MSG_MAP(CSizeClipDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, OnNotifyItemChanged)
END_MSG_MAP()

protected:
	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnNotifyItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
};

#endif				// __SIZECLIP_H__
