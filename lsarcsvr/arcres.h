// ArcResDlg.h : CArcResDlg の宣言

#pragma once
#ifndef __ARCRESDLG_H__
#define __ARCRESDLG_H__

#include "resource.h"       // メイン シンボル
#include <atlhost.h>


// CArcResDlg

class CArcResDlg : public CDialogImpl<CArcResDlg>, public CDialogResize<CArcResDlg>
{
private:
	CStringW m_strResult;

public:
	CArcResDlg()
	{
	}

	~CArcResDlg()
	{
	}

	enum { IDD = IDD_ARCRESULT };

BEGIN_MSG_MAP(CArcResDlg)
	ON_WM_INITDIALOG()
	CHAIN_MSG_MAP(CDialogResize<CArcResDlg>)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CArcResDlg)
	DLGRESIZE_CONTROL(IDC_LIST,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		DlgResize_Init();
		AddTextToListbox(m_strResult);
		return TRUE;
	}
	inline void OnCancel() { EndDialog(IDCANCEL); }

	void SetResult(LPCSTR resptr)
	{
		if (resptr && *resptr) StrToWStr(resptr, m_strResult);
	}
	void SetResult(LPCWSTR wresptr)
	{
		if (wresptr && *wresptr) m_strResult = wresptr;
	}

	BOOL AddTextToListbox(LPCTSTR pText)
	{
		CListBox list = GetDlgItem(IDC_LIST);
		list.ResetContent();
		if (!pText && *pText == 0) return TRUE;

		CString str = pText;
		str.Replace(_T("\x0d\x0a"), _T("\n"));
		str.Replace(_T('\x0d'), _T('\n'));
		str.Replace(_T('\x0a'), _T('\n'));
		str.Replace(_T('\t'), _T(' '));

		CClientDC dc(list);
		HFONT hFont = dc.SelectFont(list.GetFont());

		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);

		list.SetRedraw(FALSE);
		int pos = 0;
		int maxwidth = 0;
		CString token = str.Tokenize(_T("\n"), pos);
		while (token != _T("")) {
			if (list.AddString(token) < 0)
				break;

			CSize size;
			dc.GetTextExtent(token, token.GetLength(), &size);
			if (size.cx > maxwidth)
				maxwidth = size.cx;

			token = str.Tokenize(_T("\n"), pos);
		};

		dc.SelectFont(hFont);

		list.SetHorizontalExtent(maxwidth + tm.tmAveCharWidth);

		list.SetCurSel(0);
		list.SetRedraw(TRUE);
		list.UpdateWindow();
		return TRUE;
	}
};


#endif			// __ARCRESDLG_H__
