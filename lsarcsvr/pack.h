// PackDlg.h : CPackDlg の宣言

#pragma once
#ifndef __PACKDLG_H__
#define __PACKDLG_H__

#include "resource.h"       // メイン シンボル
#include "dlgtab.h"
#include "dialogtempl.h"
#include "common.h"
#include "pack_types.h"
#include "archiver-page-imple.h"
#include "pack_lzh.h"
#include "pack_cab.h"
#include "pack_zip.h"
#include "pack_tar.h"
#include "pack_bga.h"
#include "pack_7zip.h"
#include "pack_ish.h"
#include "pack_upx.h"

////////////////////////////////////////////////////////////////////////
//

/* @@

typedef struct tagPACKDATA {
	CPageBar* pbar;
	HDROP hDrop;
	BOOL incdir;
	BOOL sfx;
	BOOL unicode;
	BOOL utf8;
	TCHAR cmd[MAX_PATH * 2];
	TCHAR path[MAX_PATH * 2];
	TCHAR arcname[MAX_PATH * 2];
	TCHAR temp[MAX_PATH * 2];
	TCHAR base[MAX_PATH * 2];
	int arctype;
} PACKDATA;

@@ */

////////////////////////////////////////////////////////////////////////
// CPackDlg

class CPackDlg : public CDialogImpl<CPackDlg>
{
public:
	enum { IDD = IDD_PACKDLG };
	CLccSearchPathCtrl m_spc;

	CPackInfo m_pd;
	CLzhPage m_pageLzh;
	CCabPage m_pageCab;
	CZipPage m_pageZip;
	CTarPage m_pageTar;
	CBgaPage m_pageBga;
	C7zPage m_page7z;
	CIshPage m_pageIsh;
#ifdef LSARC_UPX_SUPPORT
	CUpxPage m_pageUpx;
#endif

	CPackDlg() { }

/* ----
	INT_PTR DoModalEx(HWND hWndParent = ::GetActiveWindow(), LPARAM param = NULL)
	{
		INT_PTR res = IDCANCEL;
		if (::IsWindow(hWndParent) && ::IsWindowVisible(hWndParent))
			res = DoModal(hWndParent, param);
		else {
			CDialogTemplate dt(MAKEINTRESOURCE(IDD));
			if (!dt.IsNull()) {
				dt.ModifyStyle(0, WS_MINIMIZEBOX | WS_SYSMENU);
				dt.ModifyExStyle(0, WS_EX_APPWINDOW);

				_AtlWinModule.AddCreateWndData(&m_thunk.cd, (ATL::CDialogImplBase*)this);
#ifdef _DEBUG
				m_bModal = true;
#endif //_DEBUG
				res = ::DialogBoxIndirectParam(_AtlBaseModule.GetModuleInstance(), dt, hWndParent, StartDialogProc, param);
			}
		}
		return res;
	}
---- */

BEGIN_MSG_MAP(CPackDlg)
	ON_WM_INITDIALOG()
	ON_WM_DESTROY()
	ON_WM_DRAWITEM()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_NOTIFY(SEN_GETTEXT, IDC_PACK_PATH, OnNotifyBrowse)
	ON_MESSAGE(WM_CTLCOLORDLG, OnCtlColorDlg)
	ON_COMMAND_RANGE(IDD_PACK_LZH, IDD_PACK_UPX, OnTab)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnTab(UINT id, UINT code, HWND hwnd);
	void OnDestroy();
	void OnDrawItem(int idCtrl, LPDRAWITEMSTRUCT pdi);
	LRESULT OnCtlColorDlg(WPARAM wParam, LPARAM lParam);

	void UpdateTitleCaption();
};


#endif		// __PACKDLG_H__
