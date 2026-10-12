// PackDlg.cpp : CPackDlg の実装

#include "stdafx.h"
#include "common.h"
#include "pack.h"

LPCTSTR EXTENSION_EXE = _T(".exe");
LPCTSTR EXTENSION_ZIP = _T(".zip");
LPCTSTR EXTENSION_CAB = _T(".cab");
LPCTSTR EXTENSION_TAR = _T(".tar");
LPCTSTR EXTENSION_7ZP = _T(".7z");
LPCTSTR EXTENSION_LZH = _T(".lzh");
LPCTSTR EXTENSION_BZA = _T(".bza");
LPCTSTR EXTENSION_GZA = _T(".gza");

////////////////////////////////////////////////////////////////////
// CPackDlg

BOOL CPackDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	SetDialogIcon(m_hWnd, IDI_COMPRESS);

	m_spc = GetDlgItem(IDC_PACK_PATH);

	int cnt;
	if (!m_pd.m_hdrop || (cnt = DragQueryFile(m_pd.m_hdrop, 0xFFFFFFFF, NULL, 0)) < 1) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	TCHAR buf[MAX_PATH * 2], ww[MAX_PATH * 2];
	bool bUnmatchdir = false;
	m_pd.m_incdir = FALSE;
	int numDir = 0;
	for (int i = 0; i < cnt; i++) {
		DragQueryFile(m_pd.m_hdrop, i, buf, ARRAYSIZE(buf));
		if (PathIsDirectory(buf)) {
			m_pd.m_incdir = TRUE;
			numDir++;
		}
		// 基準フォルダを抽出する
		PathGetParent(buf, ww);
		if (m_pd.m_base.IsEmpty())
			m_pd.m_base = ww;
		else {
			if (ComparePath(m_pd.m_base, ww))
				bUnmatchdir = false;
		}
	}

	if (bUnmatchdir)
		m_pd.m_base.Empty();

	if (cnt == 1 && m_pd.m_incdir) {
		DragQueryFile(m_pd.m_hdrop, 0, buf, ARRAYSIZE(buf));
		TCHAR* p = StrRChr(buf, NULL, _T('\\'));
		if (p)
			p++;
		else
			p = buf;
		m_spc.SetWindowText(p);
	}

	// フォルダのロードと設定
	m_pd.m_pbar = new CPageBar(GetDlgItem(IDC_TAB));

	// アイコンイメージリスト
/*
	HIMAGELIST him = NULL;
	UINT id = gosp.IsComm6() ? IDB_XP_ARCPACK : IDB_ARCPACK;
	HBITMAP hbm = (HBITMAP)LoadImage(ATL::_AtlBaseModule.GetResourceInstance(), MAKEINTRESOURCE(id), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
	if (hbm) {
		BITMAP bm;
		GetObject(hbm, sizeof(bm), &bm);
		UINT flags = ((bm.bmBitsPixel == 32) ? ILC_COLOR32 : ILC_COLORDDB) | ILC_MASK;
		const int cxIcon = 30;
		him = ImageList_Create(cxIcon, bm.bmHeight, flags, bm.bmWidth / cxIcon, 1);
		if (him)
			ImageList_AddMasked(him, hbm, RGB(255, 0, 255));
		DeleteObject(hbm);
	}
*/
	HIMAGELIST him = lccLoadImageList(ATL::_AtlBaseModule.GetResourceInstance(), RT_RCDATA, MAKEINTRESOURCE(IDB_ARCPACK), 24, RGB(255, 0, 255));

	// 展開先領域
	CRect rc;
	GetDlgItem(IDC_AREA).GetWindowRect(&rc);
	ScreenToClient(&rc);
	m_pd.m_pbar->Initialize(&rc, him);

	m_pd.m_pbar->AddPage(m_pageZip.IDD, m_pageZip.Create(m_hWnd, (LPARAM)&m_pd));
	m_pd.m_pbar->AddPage(m_pageCab.IDD, m_pageCab.Create(m_hWnd, (LPARAM)&m_pd));
	m_pd.m_pbar->AddPage(m_page7z.IDD,  m_page7z.Create(m_hWnd,  (LPARAM)&m_pd));
	m_pd.m_pbar->AddPage(m_pageTar.IDD, m_pageTar.Create(m_hWnd, (LPARAM)&m_pd));
	m_pd.m_pbar->AddPage(m_pageLzh.IDD, m_pageLzh.Create(m_hWnd, (LPARAM)&m_pd));
#ifndef _WIN64
	m_pd.m_pbar->AddPage(m_pageBga.IDD, m_pageBga.Create(m_hWnd, (LPARAM)&m_pd));
	m_pd.m_pbar->AddPage(m_pageIsh.IDD, m_pageIsh.Create(m_hWnd, (LPARAM)&m_pd));
#endif
#ifdef LSARC_UPX_SUPPORT
	m_pd.m_pbar->AddPage(m_pageUpx.IDD, m_pageUpx.Create(m_hWnd, (LPARAM)&m_pd));
#endif

	m_pd.m_pbar->RecalcLayout();

	// 前回のページを表示
	UINT id = GetRegValue(IDS_SEC_OPERATION, IDS_KEY_PREVPACK, IDD_PACK_ZIP);
	if (!m_pd.m_pbar->SetActivePage(id))
		m_pd.m_pbar->SetActivePage(IDD_PACK_ZIP);
	UpdateTitleCaption();

	m_spc.SetFocus();
	return FALSE;
}

void CPackDlg::OnOK()
{
	UINT id = m_pd.m_pbar->GetActivePage();
	switch (id) {
		case IDD_PACK_CAB:	m_pd.m_arctype = ARC_CAB;	break;
		case IDD_PACK_ZIP:	m_pd.m_arctype = ARC_ZIP;	break;
		case IDD_PACK_TAR:	m_pd.m_arctype = ARC_TAR;	break;
		case IDD_PACK_7ZIP:	m_pd.m_arctype = ARC_7ZIP;	break;
		case IDD_PACK_LZH:	m_pd.m_arctype = ARC_LZH;	break;
		case IDD_PACK_BGA:	m_pd.m_arctype = ARC_BGA;	break;
		case IDD_PACK_ISH:	m_pd.m_arctype = ARC_ISH;	break;
#ifdef LSARC_UPX_SUPPORT
		case IDD_PACK_UPX:	m_pd.m_arctype = ARC_UPX;	break;
#endif
		default: return;
	}

	TCHAR buf[MAX_PATH * 2], ww[MAX_PATH * 2], *p, *q;
	// アーカイバDLLの存在チェック

#ifdef LSARC_UPX_SUPPORT
	if (m_pd.m_arctype != ARC_UPX)
#endif
	{
		if (!__GetArchiverTypeInfo(m_pd.m_arctype, ww, buf, m_pd.m_unicode, m_pd.m_utf8)) {
			p = buf[0] ? PathFindFileName(buf) : NULL;
			AppMessage(IDS_LIBRARY_NOT_FOUND, p);
			return;
		}
	}

	if (m_spc.GetWindowText(buf, ARRAYSIZE(buf)) < 1) {
		if (
#ifdef LSARC_UPX_SUPPORT
			m_pd.m_arctype == ARC_UPX ||
#endif
			m_pd.m_arctype == ARC_ISH) {

			// ISH, UPX
			lstrcpy(buf, m_pd.m_path);
		}
		else {
			// 必要ならば書庫毎の拡張子を付加する
			DragQueryFile(m_pd.m_hdrop, 0, buf, ARRAYSIZE(buf));
			p = PathFindFileName(buf);
			q = PathFindExtension(p);
			if (*q == '.' && q > p)
				*q = 0;
			switch (m_pd.m_arctype) {
				case ARC_ZIP:
					lstrcat(p, EXTENSION_ZIP);
					break;
				case ARC_LZH:
					lstrcat(p, EXTENSION_LZH);
					break;
				case ARC_CAB:
					lstrcat(p, EXTENSION_CAB);
					break;
				case ARC_TAR:
					lstrcat(p, EXTENSION_TAR);
					break;
				case ARC_7ZIP:
					lstrcat(p, m_pd.m_sfx ? EXTENSION_EXE : EXTENSION_7ZP);
					break;
			}

			lstrcpy(ww, m_pd.m_path);
			PathAppend(ww, p);
			lstrcpy(buf, ww);
		}
	}
	else {
		if (!StrRChr(buf, NULL, _T('\\'))) {
			PathCombine(ww, m_pd.m_path, buf);
			lstrcpy(buf, ww);
		}

		if (
#ifdef LSARC_UPX_SUPPORT
			m_pd.m_arctype == ARC_UPX ||
#endif
			m_pd.m_arctype == ARC_ISH) {
			// ディレクトリのチェック
			if (!PathIsDirectory(buf)) {
				AppMessage(IDS_ISH_DIRECTORYSPECIFIED);
				m_spc.SetFocus();
				return;
			}
		}

		// 必要ならば書庫毎の拡張子を付加する
		p = PathFindFileName(buf);
		q = PathFindExtension(buf);
		bool bAddExtension = (*q != '.');
		if (!bAddExtension) {
			switch (m_pd.m_arctype) {
				case ARC_LZH:
					bAddExtension = (lstrcmpi(q, EXTENSION_LZH) != 0);
					break;
				case ARC_ZIP:
					bAddExtension = (lstrcmpi(q, EXTENSION_ZIP) != 0);
					break;
				case ARC_CAB:
					bAddExtension = (lstrcmpi(q, EXTENSION_CAB) != 0);
					break;
	//@			case ARC_TAR:
	//				break;
				case ARC_7ZIP:
					bAddExtension = (lstrcmpi(q, (m_pd.m_sfx ? EXTENSION_EXE : EXTENSION_7ZP)) != 0);
					break;
			}
		}

		if (bAddExtension) {
			switch (m_pd.m_arctype) {
				case ARC_ZIP:
					lstrcat(buf, EXTENSION_ZIP);
					break;
				case ARC_LZH:
					lstrcat(buf, EXTENSION_LZH);
					break;
				case ARC_CAB:
					lstrcat(buf, EXTENSION_CAB);
					break;
	//@			case ARC_TAR:
	//				break;
				case ARC_7ZIP:
					lstrcat(buf, m_pd.m_sfx ? EXTENSION_EXE : EXTENSION_7ZP);
					break;
			}
		}
	}

	if (buf[0] == 0)
		PathCombine(buf, m_pd.m_path, _T("~arcpack"));
	m_pd.m_arcname = buf;

	if (m_pd.m_pbar->OnOK(id)) {
		SetRegValue(IDS_SEC_OPERATION, IDS_KEY_PREVPACK, id);
		EndDialog(IDOK);
	}
}

void CPackDlg::OnNotifyBrowse(NMHDR* pNmhdr, LRESULT* pResult)
{
	NMSEGETTEXT* psgt = (NMSEGETTEXT*)pNmhdr;

	TCHAR buf[MAX_PATH * 2];
	lstrcpy(buf, m_pd.m_path);

	UINT id = m_pd.m_pbar->GetActivePage();
	switch (id) {
		case IDD_PACK_ISH:
		case IDD_PACK_UPX:
			lstrcpy(buf, m_pd.m_path);
			if (!BrowseForFolder(m_hWnd, buf))
				return;
			break;
		default:
			if (!FileBrowse(TRUE, m_hWnd, IDS_CAPTION_CREATEARCHIVE, IDS_FILTER_ARCHIVE, buf))
				return;
			break;
	}
	lstrcpyn(psgt->pszText, buf, psgt->cchTextMax);
	*pResult = TRUE;
}

BOOL CPackDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW && pInfo->iCtrlId) {
		ShowCtrlHelp(GetDlgItem(pInfo->iCtrlId), pInfo->iCtrlId);
		return TRUE;
	}
	return FALSE;
}

void CPackDlg::UpdateTitleCaption()
{
	UINT id = m_pd.m_pbar->GetActivePage();
	CString str;
	str.LoadString(id);

	CString cap;
	cap.LoadString(IDS_CAPTION_CREATEARCHIVE);

	str.AppendFormat(_T(" - %s"), (LPCTSTR)cap);
	SetWindowText(str);
}

void CPackDlg::OnTab(UINT id, UINT code, HWND hwnd)
{
	if (m_pd.m_pbar) {
		m_pd.m_pbar->SetActivePage(id);
		UpdateTitleCaption();
	}
}

void CPackDlg::OnDestroy()
{
	RemoveDialogIcon(m_hWnd);
}

#define COLOR_PANELFRAME			RGB(180, 180, 180)
#define COLOR_PANELLIGHT			RGB(149, 149, 149)
#define COLOR_PANELSHADOW			RGB(70, 70, 70)

static void _DrawArea(HDC hdc, RECT* prc)
{
	ATLASSERT(prc != NULL);
	CDCHandle dc = hdc;
	dc.FillSolidRect(prc, GetSysColor(COLOR_MENU));
/*
	COLORREF clrFrom = GetSysColor(COLOR_WINDOW);
	COLORREF clrTo   = GetSysColor(COLOR_3DLIGHT);

	TRIVERTEX vert[2];
	GRADIENT_RECT grc;
	vert[0].x     = prc->left;
	vert[0].y     = prc->top;
	vert[0].Red   = GetRValue(clrFrom) << 8;
	vert[0].Green = GetGValue(clrFrom) << 8;
	vert[0].Blue  = GetBValue(clrFrom) << 8;
	vert[0].Alpha = 0x0000;

	vert[1].x     = prc->right;
	vert[1].y     = prc->bottom;
	vert[1].Red   = GetRValue(clrTo) << 8;
	vert[1].Green = GetGValue(clrTo) << 8;
	vert[1].Blue  = GetBValue(clrTo) << 8;
	vert[1].Alpha = 0x0000;

	grc.UpperLeft  = 0;
	grc.LowerRight = 1;
	dc.GradientFill(vert, 2, &grc, 1, GRADIENT_FILL_RECT_V);
*/
}

void CPackDlg::OnDrawItem(int idCtrl, LPDRAWITEMSTRUCT pdi)
{
	if (idCtrl == IDC_PACK_BARAREA)
		_DrawArea(pdi->hDC, &pdi->rcItem);
}

LRESULT CPackDlg::OnCtlColorDlg(WPARAM wParam, LPARAM lParam)
{
	CDCHandle dc = (HDC)wParam;
	CRect rc;
	GetDlgItem(IDC_PACK_BARAREA).GetWindowRect(&rc);
	ScreenToClient(&rc);
//@	dc.FillSolidRect(&rc, RGB(255, 255, 255));
	_DrawArea(dc, &rc);


	int save = dc.SaveDC();
	dc.ExcludeClipRect(&rc);
	GetClientRect(&rc);
	dc.FillSolidRect(&rc, GetSysColor(COLOR_3DFACE));
	dc.RestoreDC(save);

	return (LRESULT)::GetStockObject(NULL_BRUSH);
}
