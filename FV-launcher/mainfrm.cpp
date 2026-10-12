////////////////////////////////////////////////////////////////////////
// mainfrm.cpp

#include "stdafx.h"
#include "mainfrm.h"

static TCHAR sszSection[] = _T("FvLauncher");
static TCHAR sszWndPos[] = _T("WindowPos");
static TCHAR sszStatus[] = _T("Status");

static void MapRectToMonitorPane(CRect& rc)
{
	// 指定矩形が含まれるモニタを取得する
	HMONITOR hmt = ::MonitorFromRect(&rc, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO mi;
	mi.cbSize = sizeof(mi);
	GetMonitorInfo(hmt, &mi);

	// モニタの作業領域に矩形が収まるように調整
	int dx = max(mi.rcWork.left, min(rc.left, mi.rcWork.right)) - rc.left;
	int dy = max(mi.rcWork.top, min(rc.top, mi.rcWork.bottom)) - rc.top;
	rc.OffsetRect(dx, dy);

	dx = max(mi.rcWork.left, min(rc.right, mi.rcWork.right)) - rc.right;
	dy = max(mi.rcWork.top, min(rc.bottom, mi.rcWork.bottom)) - rc.bottom;
	rc.OffsetRect(dx, dy);
}


void CMainFrame::UpdateUIAll()
{
	BOOL bAeroEnabled = FALSE;
#ifdef USE_VISTA_AERO
	// AERO
	m_DwmManager.IsCompositionEnabled(&bAeroEnabled);
#endif
	UIEnable(IDM_SHADING, !bAeroEnabled);
	UISetCheck(IDM_SHADING, (m_style & STS_SHADING) != 0);
	UISetCheck(IDM_BAR_VERT, (m_style & STS_VERT) != 0);
	UISetCheck(IDM_TOPMOST, (m_style & STS_TOPMOST) != 0);
	UISetCheck(IDM_ICON_SMALL, (m_style & STS_SMALLICON) != 0);
	UISetCheck(IDM_USELAYERED, (m_style & STS_ALPHA) != 0);
	UISetCheck(IDM_HIDEWINDOW, (m_style & STS_HIDEWINDOW) != 0);
}

int CMainFrame::OnCreate(LPCREATESTRUCT pcs)
{
#ifdef USE_VISUALSTYLE
	// Theme
//	OpenThemeData(L"globals");
	OpenThemeData(L"TOOLBAR");
#endif

	// ウィンドウスタイル
	int style = STS_SMALLICON | STS_SHADING;
	if (!lccRegGetInt(APP_PRODUCT, sszSection, sszStatus, &style))
		style = STS_SMALLICON | STS_SHADING;
	m_style = style;

	// System情報
	NONCLIENTMETRICS ncm;
	memset(&ncm, 0, sizeof(ncm));
	ncm.cbSize = RunTimeHelper::SizeOf_NONCLIENTMETRICS();
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, FALSE);
	m_hMenuFont = CreateFontIndirect(&ncm.lfMenuFont);

	LOGFONT lf;
	SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, FALSE);
	m_hIconFont = CreateFontIndirect(&lf);
	m_himDot = ImageList_LoadImage(RESINST,
						_MKIR(IDB_DOTCOLOR), 11, 0, RGB(255, 0, 255), IMAGE_BITMAP,
						LR_CREATEDIBSECTION | LR_DEFAULTCOLOR);

	if (m_style & STS_TOPMOST)
		SetWindowPos(HWND_TOPMOST, 0,0,0,0, SWP_NOSIZE | SWP_NOMOVE);
	if (m_style & STS_HIDEWINDOW)
		m_nCmdShow = SW_HIDE;

	DriveListup();
	TrayMessage(TRUE);

	::SetLayeredWindowAttributes(m_hWnd, 0, (m_style & STS_ALPHA) ? 160 : 255, LWA_ALPHA);

	// ドライブボタンサイズ
	HIMAGELIST himL, himS;
	Shell_GetImageLists(&himL, &himS);
	if (m_style & STS_SMALLICON)
		m_himDrive = himS;
	else
		m_himDrive = himL;
	ImageList_GetIconSize(m_himDrive, &m_xDrive, &m_yDrive);

	// ウィンドウ位置
	CRect rc;
	WINDOWPLACEMENT wpm;
	if (lccRegGetBinary(APP_PRODUCT, sszSection, sszWndPos, (BYTE*)&wpm, sizeof(wpm)) == sizeof(wpm)) {
		CRect rcw;
		::AdjustWindowRectEx(&rcw, pcs->style, FALSE, pcs->dwExStyle);
		rc = wpm.rcNormalPosition;
		rc.left -= rcw.left;
		rc.top -= rcw.top;
		rc.right -= rcw.right;
		rc.bottom -= rcw.bottom;
	}
	else
		rc.SetRect(pcs->x, pcs->y, pcs->cx,  pcs->cy);

	RecalcLayout(rc);
	MapRectToMonitorPane(rc);
	SetWindowPos(NULL, &rc, SWP_NOZORDER | SWP_NOREDRAW);

	BOOL bAeroEnabled = FALSE;
#ifdef USE_VISTA_AERO
	// AERO
	m_DwmManager.IsCompositionEnabled(&bAeroEnabled);
	if (bAeroEnabled) {
		MARGINS m = { -1 };
		m_DwmManager.ExtendFrameIntoClientArea(m_hWnd, &m);
	}
#endif

	// シェル更新追跡機能の初期化
	m_ShellNotify.Start(m_hWnd,
						SHCNE_NETSHARE | SHCNE_NETUNSHARE |
						SHCNE_MEDIAINSERTED | SHCNE_MEDIAREMOVED |
						SHCNE_DRIVEREMOVED | SHCNE_DRIVEADD);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	return 0;
}

void CMainFrame::OnClose(BOOL& bHandled)
{
	SaveProfile();
}

void CMainFrame::OnDestroy()
{
	CleanUp();

	// タイトルバーが無いウィンドウはこれをポストしないと駄目、何故？
	PostQuitMessage(1);
}

BOOL CMainFrame::OnQueryEndSession(UINT nSession)
{
	SaveProfile();
	return TRUE;
}

void CMainFrame::SaveProfile()
{
	lccRegSetInt(APP_PRODUCT, sszSection, sszStatus, m_style);

	// 現在のウィンドウ位置情報を保存
	if ((GetStyle() & (WS_VISIBLE | WS_MINIMIZE | WS_MAXIMIZE)) == WS_VISIBLE) {
		WINDOWPLACEMENT wpm;
		wpm.length = sizeof(wpm);
		GetWindowPlacement(&wpm);
		lccRegSetBinary(APP_PRODUCT, sszSection, sszWndPos, (BYTE*)&wpm, sizeof(wpm));
	}
}

void CMainFrame::CleanUp()
{
	int i, numDrives = m_drives.GetSize();
	for (i = 0; i < numDrives; i++)
		delete m_drives[i];
	m_drives.RemoveAll();

	if (m_timerid) {
		KillTimer(m_timerid);
		m_timerid = 0;
	}

	m_ShellNotify.Stop();
	TrayMessage(FALSE);

	if (m_hMenuFont) {
		::DeleteObject(m_hMenuFont);
		m_hMenuFont = NULL;
	}
	if (m_hIconFont) {
		::DeleteObject(m_hIconFont);
		m_hIconFont = NULL;
	}
	if (m_himDot) {
		::ImageList_Destroy(m_himDot);
		m_himDot = NULL;
	}

	// unregister message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->RemoveMessageFilter(this);
	pLoop->RemoveIdleHandler(this);
}

void CMainFrame::OnActivate(UINT fActive, HWND hWndOther, BOOL fMinimized)
{
	bool bNeedToActive = (fActive == WA_ACTIVE);
	CClientDC dc(m_hWnd);
	DrawCaption(dc, bNeedToActive ? SYSBOX_ACTIVE : SYSBOX_INACTIVE);
	if (bNeedToActive)
		ResetLayeredWindow();
}

BOOL CMainFrame::OnMoving(UINT nSide, LPRECT prc)
{
	::DefWindowProc(m_hWnd, WM_MOVING, (WPARAM)nSide, (LPARAM)prc);

	CRect rc;
	GetWindowRect(&rc);
	int xscr = GetSystemMetrics(SM_CXSCREEN);
	int yscr = GetSystemMetrics(SM_CYSCREEN);
	prc->left = min(xscr - rc.Width(), max(0, prc->left));
	prc->top  = min(yscr - rc.Height(), max(0, prc->top));
	prc->right = prc->left + rc.Width();
	prc->bottom = prc->top + rc.Height();
	return TRUE;
}

void CMainFrame::ZapTimer(DWORD zapcount)
{
	DWORD timeout = GetTickCount() + zapcount;
	MSG msg;
	while (timeout > GetTickCount()) {
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

void CMainFrame::OnTimer(UINT_PTR idEvent)
{
	if (idEvent == m_timerid) {
		if (m_bInLayered) {
			CPoint pt;
			GetCursorPos(&pt);
			CRect rc;
			GetWindowRect(&rc);
			if (!rc.PtInRect(pt) && GetCapture() != m_hWnd) {
				m_bInLayered = false;
				KillTimer(m_timerid);
				m_timerid = 0;

				m_bFadeOuting = true;
				int i = 0;
				while (m_bFadeOuting && i < 128) {
					::SetLayeredWindowAttributes(m_hWnd, 0, 255 - i, LWA_ALPHA);
					UpdateWindow();
					ZapTimer(60);
					i += 16;
				}
				m_bFadeOuting = false;
			}
		}
	}
}

void CMainFrame::OnMouseMove(UINT fwKeys, CPoint pt)
{
	if (m_bCaptured) return;

	if (!m_bTrackMouseEvent) {
		// マウストラック
		TRACKMOUSEEVENT te;
		te.cbSize = sizeof(te);
		te.dwFlags = TME_LEAVE;
		te.hwndTrack = m_hWnd;
		te.dwHoverTime = HOVER_DEFAULT;
		m_bTrackMouseEvent = _TrackMouseEvent(&te);
		ResetLayeredWindow();
	}

	if (m_bTrackMouseEvent) {
		CRect rc;
		int index = GetIndexFromPoint(pt, rc);
		if (index != m_hotIndex) {

			CClientDC dc(m_hWnd);
			if (m_hotIndex >= 0) {
				m_drives[m_hotIndex]->m_stat &= ~PTS_HOT;
				DrawDrive(dc, m_hotIndex);
			}
			if (index >= 0) {
				m_drives[index]->m_stat |= PTS_HOT;
				DrawDrive(dc, index);
			}
			m_hotIndex = index;
		}
	}
}

void CMainFrame::OnMouseLeave()
{
	if (m_bTrackMouseEvent) {
		if (m_hotIndex >= 0) {
			CClientDC dc(m_hWnd);
			m_drives[m_hotIndex]->m_stat &= ~PTS_HOT;
			DrawDrive(dc, m_hotIndex);
			m_hotIndex = -1;
		}
		m_bTrackMouseEvent = FALSE;
		ResetLayeredWindow();
	}
}

void CMainFrame::OnLButtonDown(UINT fwKeys, CPoint pt)
{
	// pt はクライアント座標
	CRect rc;
	int index = GetIndexFromPoint(pt, rc);
	if (index >= 0) {
		if (TrackInRect(index))
			Launch(index);
	}
}

void CMainFrame::OnContextMenu(HWND hWnd, CPoint point)
{
	if (HitTest(point) == HTCLIENT) {
		CRect rc;
		CPoint pt = point;
		ScreenToClient(&pt);
		int index = GetIndexFromPoint(pt, rc);
		if (index >= 0) {
			CClientDC dc(m_hWnd);
			CDriveItem* item = m_drives[index];
			item->m_stat |= PTS_PRESSED;
			DrawDrive(dc, index);

			ClientToScreen(&rc);
			pt = CPoint(rc.left, rc.bottom);
			lccContextMenuByFile(m_hWnd, item->m_strRoot, &pt);

			item->m_stat &= ~PTS_PRESSED;
			DrawDrive(dc, index);
		}
	}
}

BOOL CMainFrame::TrackInRect(int index)
{
	CRect rc;
	GetItemRect(index, rc);

	SetCapture();
	m_bCaptured = true;
	CClientDC dc(m_hWnd);

	CDriveItem* item = m_drives[index];
	item->m_stat |= PTS_PRESSED;
	DrawDrive(dc, index);

	BOOL bIn = FALSE;
	BOOL bTracking = TRUE;
	while (bTracking) {
		MSG msg;
		while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			switch(msg.message) {
				case WM_LBUTTONUP:
				case WM_RBUTTONUP:
				{
					CPoint pt((DWORD)msg.lParam);
					bIn = rc.PtInRect(pt);
					bTracking = FALSE;
					break;
				}

				default:
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					break;
			}
		}
	}
	item->m_stat &= ~PTS_PRESSED;
	DrawDrive(dc, index);

	m_bCaptured = false;
	ReleaseCapture();
    return bIn;
}

LRESULT CMainFrame::OnHandlerNcMouseMove(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ResetLayeredWindow();
	bHandled = FALSE;
	return 1;
}

UINT CMainFrame::HitTest(POINT pt)
{
	// pt はスクリーン座標
	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);

	// SysBox内かどうか
	rc.right = rc.left + SYSBOX_X;
	rc.bottom = rc.top + SYSBOX_Y;
	if (rc.PtInRect(pt)) return HTSYSMENU;

	ScreenToClient(&pt);
	return (GetIndexFromPoint(pt, rc) >= 0) ? HTCLIENT : HTCAPTION;
}

UINT CMainFrame::OnNcHitTest(int xPos, int yPos)
{
	UINT hit = (UINT)::DefWindowProc(m_hWnd, WM_NCHITTEST, 0, MAKELPARAM(xPos, yPos));
	if (hit != HTCLIENT) {
		switch (hit) {
		case HTLEFT:
		case HTRIGHT:
		case HTTOP:
		case HTTOPLEFT:
		case HTTOPRIGHT:
		case HTBOTTOM:
		case HTBOTTOMLEFT:
		case HTBOTTOMRIGHT:
			hit = HTBORDER;
			break;
		}
		return hit;
	}

	CPoint pt(xPos, yPos);
	return HitTest(pt);
}

void CMainFrame::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	// pt はスクリーン座標
	CRect rc;
	switch (nHitTest) {
	case HTSYSMENU:
		GetClientRect(&rc);
		ClientToScreen(&rc);
		rc.DeflateRect(1, 1);
		point.x = rc.left;
		point.y = rc.top + SYSBOX_Y;
		ShowSysMenu(point);
		break;
	case HTLEFT:
	case HTRIGHT:
	case HTTOP:
	case HTTOPLEFT:
	case HTTOPRIGHT:
	case HTBOTTOM:
	case HTBOTTOMLEFT:
	case HTBOTTOMRIGHT:
		break;
	default:
		::DefWindowProc(m_hWnd, WM_NCLBUTTONDOWN, (WPARAM)nHitTest, MAKELPARAM(point.x, point.y));
		break;
	}
}

void CMainFrame::OnNcLButtonDblClk(UINT nHitTest, CPoint pt)
{
	if (nHitTest == HTCAPTION)
		PostMessage(WM_CLOSE, 0, 0L);
}

BOOL CMainFrame::OnEraseBkgnd(HDC hdc)
{
	CDCHandle dc = hdc;
	CRect rc;
	GetClientRect(&rc);

	BOOL bAeroEnabled = FALSE;
#ifdef USE_VISTA_AERO
	// AERO
	m_DwmManager.IsCompositionEnabled(&bAeroEnabled);
#endif
	if (bAeroEnabled)
		dc.FillSolidRect(&rc, RGB(0, 0, 0));
	else {
		if (m_style & STS_SHADING)
			FillShadeRect(dc, &rc, NULL, !(m_style & STS_VERT));
		else
			dc.FillSolidRect(&rc, GetSysColor(COLOR_3DFACE));
	}
	return TRUE;
}

void CMainFrame::OnPaint()
{
	CPaintDC dc(m_hWnd);
	DrawCaption(dc, (GetActiveWindow() == m_hWnd) ? SYSBOX_ACTIVE : SYSBOX_INACTIVE);
	int numDrives = m_drives.GetSize();
	for (int i = 0; i < numDrives; i++)
		DrawDrive(dc, i);
}

void CMainFrame::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT pmi)
{
	CDriveItem* item = (CDriveItem*)pmi->itemData;

	CClientDC dc(m_hWnd);
	HFONT hOldFont = dc.SelectFont(m_hMenuFont);
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	CSize csz;
	dc.GetTextExtent(item->m_strDisplayName, lstrlen(item->m_strDisplayName), &csz);
	dc.SelectFont(hOldFont);

	pmi->itemWidth = GetSystemMetrics(SM_CXSMICON) + csz.cx + 16;
	pmi->itemHeight = max(tm.tmHeight, GetSystemMetrics(SM_CYSMICON)) + 2;
}

void CMainFrame::OnDrawItem(int nIDCtrl, LPDRAWITEMSTRUCT pdi)
{
	HIMAGELIST himL, himS;
	Shell_GetImageLists(&himL, &himS);

	CDriveItem* item = (CDriveItem*)pdi->itemData;
	lccDrawButtonMenu(TRUE, pdi, item->m_strDisplayName, himS, item->m_iImage, CSize(16, 16), 0);
}

void CMainFrame::DrawCaption(HDC hdc, int clrIndex)
{
	COLORREF clrSysBox;
	switch (clrIndex) {
	case SYSBOX_INACTIVE:	clrSysBox = SYSBOXCOLOR_INACTIVE; break;
	case SYSBOX_ACTIVE:		clrSysBox = SYSBOXCOLOR_ACTIVE; break;
	case SYSBOX_ONMENU:
	default:				clrSysBox = SYSBOXCOLOR_ONMENU; break;
	}

	BOOL bAeroEnabled = FALSE;
#ifdef USE_VISTA_AERO
	// AERO
	m_DwmManager.IsCompositionEnabled(&bAeroEnabled);
#endif

	CDCHandle dc = hdc;

	COLORREF crFace = GetSysColor(COLOR_3DFACE);
	COLORREF crHilight = GetSysColor(COLOR_3DHILIGHT);
	COLORREF crShadow = GetSysColor(COLOR_3DSHADOW);

	CRect rcc;
	GetClientRect(&rcc);

	rcc.InflateRect(-1, -1);
	CRect rcSysBox = rcc;
	rcSysBox.right = rcSysBox.left + SYSBOX_X;
	rcSysBox.bottom = rcSysBox.top + SYSBOX_Y;

	rcc.InflateRect(1, 1);

	if (bAeroEnabled) {
		dc.FillSolidRect(&rcSysBox, RGB(0, 0, 0));
	}
	else {
		if (m_style & STS_SHADING) {
			FillShadeRect(dc, &rcc, &rcSysBox, !(m_style & STS_VERT));
		}
		else {
			dc.FillSolidRect(&rcSysBox, crFace);
		}
	}

	if (m_himDot) {
		ImageList_Draw(m_himDot, clrIndex, hdc, rcSysBox.left, rcSysBox.top, ILD_TRANSPARENT);
	}
	else {
		Graphics gr(hdc);
		SolidBrush br(Color(255, GetRValue(clrSysBox), GetGValue(clrSysBox), GetBValue(clrSysBox)));
		gr.SetSmoothingMode(SmoothingModeHighQuality);
		gr.FillEllipse(&br, rcSysBox.left, rcSysBox.top, rcSysBox.Width(), rcSysBox.Height());
	}
}

void CMainFrame::DrawDrive(HDC hdc, int index)
{
	CDCHandle dc = hdc;

	CRect rc;
	if (!GetItemRect(index, rc)) return;

	CRect rcc;
	GetClientRect(&rcc);

	int states;
	BOOL bAeroEnabled = FALSE;
#ifdef USE_VISTA_AERO
	// AERO
	m_DwmManager.IsCompositionEnabled(&bAeroEnabled);
#endif

	CDriveItem* item = m_drives[index];

	if (bAeroEnabled) {
		dc.FillSolidRect(&rc, RGB(0, 0, 0));

		if      (item->IsHot())		states = TS_HOT;
		else if (item->IsPressed())	states = TS_PRESSED;
		else						states = TS_NORMAL;
		DrawThemeBackground(dc, TP_BUTTON, states, &rc, NULL);
	}
	else {
		if (item->IsHot() || item->IsPressed())
			fill_glass_rect(dc, rc, item->IsHot(), item->IsPressed());
		else {
			if (m_style & STS_SHADING)
				FillShadeRect(dc, &rcc, &rc, !(m_style & STS_VERT));
			else
				dc.FillSolidRect(&rc, GetSysColor(COLOR_3DFACE));
		}
	}

	rc.InflateRect(-1, -1);
	if (item->IsPressed())
		rc.OffsetRect(1, 0);

	rc.left += 2;

	UINT flags = ILD_TRANSPARENT;
	if (item->m_iOverlayImage > 0)
		flags |= INDEXTOOVERLAYMASK(item->m_iOverlayImage);
	ImageList_Draw(m_himDrive, item->m_iImage, dc, rc.left, (rc.top + rc.bottom - m_yDrive) / 2, flags);
	rc.left += m_xDrive + 1;

	UINT fmt = DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP;
	if (bAeroEnabled) {
		BITMAPINFO dib = { 0 };
		dib.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
		dib.bmiHeader.biWidth       = rc.Width();
		dib.bmiHeader.biHeight      = -rc.Height();
		dib.bmiHeader.biPlanes      = 1;
		dib.bmiHeader.biBitCount    = 32;
		dib.bmiHeader.biCompression = BI_RGB;

		CBitmap bmp;
		bmp.CreateDIBSection(dc, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
		if (!bmp.IsNull()) {
			HTHEME hTheme = ::OpenThemeData(NULL, L"ExplorerBar");
			if (hTheme) {
				CDC dcw;
				dcw.CreateCompatibleDC(dc);
				HBITMAP hbmw = dcw.SelectBitmap(bmp);

				CRect rcc(0, 0, rc.Width(), rc.Height());
				dcw.BitBlt(0, 0, rc.Width(), rc.Height(), dc, rc.left, rc.top, SRCCOPY);	// 直前のイメージをコピーバック

				DTTOPTS dto = {sizeof(DTTOPTS)};
				dto.dwFlags = DTT_COMPOSITED | DTT_GLOWSIZE;
				dto.iGlowSize = 8;							// Default value
				::DrawThemeTextEx(hTheme, dcw, EBP_HEADERBACKGROUND, 0, item->m_strRoot, 1, fmt, &rcc, &dto);
				dc.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), dcw, 0, 0, SRCCOPY);
				dcw.SelectBitmap(hbmw);
				::CloseThemeData(hTheme);
			}
		}
	}
	else {
		HFONT hfont = dc.SelectFont(m_hIconFont);
		int mode = dc.SetBkMode(TRANSPARENT);
		dc.DrawText(item->m_strRoot, 1, &rc, fmt);
		dc.SetBkMode(mode);
		dc.SelectFont(hfont);
	}
}

void CMainFrame::ResetLayeredWindow()
{
	m_bFadeOuting = false;
	if (m_style & STS_ALPHA) {
		if (!m_bInLayered) {
			::SetLayeredWindowAttributes(m_hWnd, 0, 255, LWA_ALPHA);
			m_bInLayered = true;
			KillTimer(m_timerid);
		}
		m_timerid = SetTimer(TIMERID_STOPALPHA, 2000, NULL);
	}
	else {
		::SetLayeredWindowAttributes(m_hWnd, 0, 255, LWA_ALPHA);
		m_bInLayered = false;
		if (m_timerid) {
			KillTimer(m_timerid);
			m_timerid = 0;
		}
	}
}

BOOL CMainFrame::TrayMessage(BOOL bAdd)
{
	BOOL res;
	NOTIFYICONDATA tnd;

	tnd.cbSize = sizeof(NOTIFYICONDATA);
	tnd.hWnd = m_hWnd;
	tnd.uID = IDM_NOTIFYICON;

	tnd.uFlags = NIF_MESSAGE | NIF_ICON;
	tnd.uCallbackMessage = m_nRM_NOTIFYICON;
	tnd.szTip[0] = 0;

	HICON hIcon = (HICON)LoadImage(RESINST, MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 16, 16, 0);
	tnd.hIcon = hIcon;
	res = Shell_NotifyIcon((bAdd != FALSE) ? NIM_ADD : NIM_DELETE, &tnd);
	if (hIcon)
	    DestroyIcon(hIcon);
	return res;
}

void CMainFrame::TrackDriveMenu()
{
	CMenu menu;
	menu.CreatePopupMenu();

	int i, numDrives = m_drives.GetSize();
	for (i = 0; i < numDrives; i++)
		menu.AppendMenu(MF_OWNERDRAW, i + 1, (LPCTSTR)m_drives[i]);

	::SetForegroundWindow(m_hWnd);
	CPoint pt;
	GetCursorPos(&pt);
	int n = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RETURNCMD, pt.x, pt.y, m_hWnd);
	if (n > 0)
		PostMessage(m_nRM_LAUNCH, (WPARAM)(n - 1), 0);
}

void CMainFrame::DriveListup()
{
	CDriveItem* item;
	int i, numDrives = m_drives.GetSize();
	for (i = 0; i < numDrives; i++) {
		item = m_drives[i];
		m_drives[i] = NULL;
		if (item)
			delete item;
	}
	m_drives.RemoveAll();

	BYTE cb[26];
	if (lccRegGetBinary(APP_PRODUCT, _T("Drive"), _T("MaskDrive"), cb, 26) != 26)
		memset(cb, 0, 26);

	IShellFolder* psf = idl_GetSpecialFolder(CSIDL_DRIVES, NULL);

	CComQIPtr<IShellIconOverlay, &IID_IShellIconOverlay> psio(psf);

	IEnumIDList* pe;
	HRESULT hr = psf->EnumObjects(NULL, SHCONTF_FOLDERS|SHCONTF_INCLUDEHIDDEN, &pe);
	if (hr == S_OK) {
		LPITEMIDLIST pidl;
		ULONG celt;
		while (pe->Next(1, &pidl, &celt) == NOERROR && celt > 0) {
			ULONG ula = SFGAO_FILESYSTEM | SFGAO_FILESYSANCESTOR;
			if (psf->GetAttributesOf(1, (LPCITEMIDLIST*)&pidl, &ula) == NOERROR && ula) {
				TCHAR buf[MAX_PATH * 2];
				if (idl_GetName(psf, pidl, SHGDN_FORPARSING, buf, ARRAYSIZE(buf)) &&
					PathIsRoot(buf) && cb[toupper(buf[0]) - 'A'] == 0) {
					item = new CDriveItem(buf);
					if (psio.p != NULL) {
						int overlay = 0;
//@						if (psio->GetOverlayIndex(pidl, &overlay) == S_OK)
//@							item->m_iOverlayImage = overlay;
					}
					m_drives.Add(item);
				}
			}
			idl_Free(pidl);
		}
		pe->Release();
	}
	psf->Release();
}

void CMainFrame::DriveUpdate()
{
	CDriveItem* item;
	int i, numDrives = m_drives.GetSize();
	for (i = 0; i < numDrives; i++) {
		item = m_drives[i];
		if (item) {
			item->Parse();
		}
	}
	Invalidate();
}

void CMainFrame::RecalcLayout(CRect& rc)
{
	CClientDC dc(m_hWnd);

	HFONT hOldFont = dc.SelectFont(m_hIconFont);
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	dc.SelectFont(hOldFont);

	m_xBlock = m_xDrive + tm.tmMaxCharWidth + CX_MARGIN * 2;
	m_yBlock = max(m_yDrive, tm.tmHeight) + CY_MARGIN * 2;

	int numDrives = m_drives.GetSize();
	if (m_style & STS_VERT) {
		rc.right = rc.left + m_xBlock;
		rc.bottom = rc.top + numDrives * m_yBlock +
						CXY_BLOCKSPACER * (numDrives - 1) +
						SYSBOX_Y + EDGE_SPACER;
	}
	else {
		rc.right = rc.left + numDrives * m_xBlock +
						CXY_BLOCKSPACER * (numDrives - 1) +
						SYSBOX_X + EDGE_SPACER;
		rc.bottom = rc.top + m_yBlock;
	}
	rc.right += GetSystemMetrics(SM_CXBORDER) * 2;
	rc.bottom += GetSystemMetrics(SM_CYBORDER) * 2;

	::AdjustWindowRectEx(&rc, GetStyle(), FALSE, GetExStyle());
}

BOOL CMainFrame::GetItemRect(int index, CRect& rc)
{
	if (index < 0 || index >= m_drives.GetSize())
		return FALSE;

	GetClientRect(&rc);
	rc.DeflateRect(1, 1);

	if (m_style & STS_VERT) {
		rc.right = rc.left + m_xBlock;
		rc.top += SYSBOX_Y + 1;
		rc.bottom = rc.top + m_yBlock;
		if (index > 0)
			rc.OffsetRect(0, index * (m_yBlock + CXY_BLOCKSPACER));
	}
	else {
		rc.left += SYSBOX_X + 1;
		rc.right = rc.left + m_xBlock;
		rc.bottom = rc.top + m_yBlock;
		if (index > 0)
			rc.OffsetRect(index * (m_xBlock + CXY_BLOCKSPACER), 0);
	}
	return TRUE;
}

int CMainFrame::GetIndexFromPoint(POINT pt, CRect& rc)
{
	int numDrives = m_drives.GetSize();
	for (int i = 0; i < numDrives; i++) {
		GetItemRect(i, rc);
		if (rc.PtInRect(pt))
			return i;
	}
	return -1;
}

void CMainFrame::ShowSysMenu(POINT pt)
{
	CMenu menu;
	if (!menu.CreatePopupMenu())
		return;

	CMenu pop;
	pop.LoadMenu(IDR_SYSMENU);
	int cnt = pop.GetMenuItemCount();
	if (cnt > 0) {
		TCHAR buf[MAX_PATH];
		for (int i = 0; i < cnt; i++) {
			int id = pop.GetMenuItemID(i);
			if (id > 0) {
				pop.GetMenuString(i, buf, ARRAYSIZE(buf), MF_BYPOSITION);
				menu.InsertMenu(i, MF_BYPOSITION, (UINT_PTR)id, buf);
			}
			else if (!id)
				menu.InsertMenu(i, MF_BYPOSITION|MF_SEPARATOR, (UINT_PTR)0, (LPCTSTR)NULL);
		}
	}

	CClientDC dc(m_hWnd);
	DrawCaption(dc, SYSBOX_ONMENU);
	menu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, m_hWnd);
	DrawCaption(dc, SYSBOX_ACTIVE);
}

void CMainFrame::Launch(int index)
{
	int drive = PathGetDriveNumber(m_drives[index]->m_strRoot);
	TCHAR name[MAX_PATH * 2], path[MAX_PATH * 2];

	if (!fopGetDirRemind(drive, path) || GetFileAttributes(path) == 0xFFFFFFFF)
		PathBuildRoot(path, drive);

	if (!lccRegGetAppPath(APP_EXENAME, name, ARRAYSIZE(name)))
		lstrcpy(name, APP_EXENAME);

	ShellExecute(m_hWnd, NULL, name, path, NULL, SW_SHOW);
}

void CMainFrame::OnCommandClose()
{
	PostMessage(WM_CLOSE, 0, 0);
}

void CMainFrame::OnCommandAbout()
{
	APPABOUTINFO ai;
	memset(&ai, 0, sizeof(ai));
	ai.szUser[0] = ' ';
	ai.szCompany[0] = ' ';
	lccShowAppAbout(m_hWnd, &ai);
}

void CMainFrame::OnCommandBarVert()
{
	if (m_style & STS_VERT)
		m_style &= ~STS_VERT;
	else
		m_style |= STS_VERT;

	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);
	RecalcLayout(rc);

	int cxScr = GetSystemMetrics(SM_CXSCREEN);
	int cyScr = GetSystemMetrics(SM_CYSCREEN);
	if (rc.right > cxScr)
		rc.OffsetRect(cxScr - rc.right, 0);
	if (rc.bottom > cyScr)
		rc.OffsetRect(0, cyScr - rc.bottom);

	SetWindowPos(NULL, &rc, SWP_NOZORDER | SWP_NOCOPYBITS);
}

void CMainFrame::OnCommandTopMost()
{
	if (m_style & STS_TOPMOST) {
		m_style &= ~STS_TOPMOST;
		SetWindowPos(HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
	else {
		m_style |= STS_TOPMOST;
		SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
}

void CMainFrame::OnCommandRefresh()
{
	DriveListup();

	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);
	RecalcLayout(rc);

	SetWindowPos(NULL, &rc, SWP_NOZORDER | SWP_NOCOPYBITS);
}

void CMainFrame::OnCommandIconSmall()
{
	HIMAGELIST himL, himS;
	Shell_GetImageLists(&himL, &himS);

	if (m_style & STS_SMALLICON) {
		m_style &= ~STS_SMALLICON;
		m_himDrive = himL;
	}
	else {
		m_style |= STS_SMALLICON;
		m_himDrive = himS;
	}

	ImageList_GetIconSize(m_himDrive, &m_xDrive, &m_yDrive);

	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);
	RecalcLayout(rc);

	int cxScr = GetSystemMetrics(SM_CXSCREEN);
	int cyScr = GetSystemMetrics(SM_CYSCREEN);
	if (rc.right > cxScr)
		rc.OffsetRect(cxScr - rc.right, 0);
	if (rc.bottom > cyScr)
		rc.OffsetRect(0, cyScr - rc.bottom);

	SetWindowPos(NULL, &rc, SWP_NOZORDER | SWP_NOCOPYBITS);
}

void CMainFrame::OnCommandNotifyIcon()
{
}

void CMainFrame::OnCommandHideWindow()
{
	if (m_style & STS_HIDEWINDOW) {
		m_style &= ~STS_HIDEWINDOW;
		ShowWindow(SW_SHOWNORMAL);
	}
	else {
		m_style |= STS_HIDEWINDOW;
		ShowWindow(SW_HIDE);
	}
}

void CMainFrame::OnCommandShading()
{
	if (m_style & STS_SHADING)
		m_style &= ~STS_SHADING;
	else
		m_style |= STS_SHADING;
	Invalidate(TRUE);
}

void CMainFrame::OnCommandLayered()
{
	if (m_style & STS_ALPHA)
		m_style &= ~STS_ALPHA;
	else
		m_style |= STS_ALPHA;
	ResetLayeredWindow();
}

void CMainFrame::ProcessShellNotify(DWORD id, LPITEMIDLIST* pidls)
{
	switch (id) {
	case SHCNE_NETUNSHARE:
	case SHCNE_NETSHARE:
	case SHCNE_MEDIAREMOVED:
	case SHCNE_MEDIAINSERTED:
		DriveUpdate();
		break;
	case SHCNE_DRIVEADD:
	case SHCNE_DRIVEREMOVED:
		PostMessage(WM_COMMAND, IDM_REFRESH, 0);
		break;
	default:
		break;
	}
}

LRESULT CMainFrame::OnRegisterShellNotify(WPARAM wParam, LPARAM lParam)
{
	LPITEMIDLIST* pidls;
	LONG event_id;
	HANDLE h = m_ShellNotify.LockEvent((HANDLE)wParam, (DWORD)lParam, &pidls, &event_id);
	if (h) {
		ProcessShellNotify(event_id, pidls);
		m_ShellNotify.UnlockEvent(h);
	}
	return 0;
}

LRESULT CMainFrame::OnRegisterNotifyIcon(WPARAM wParam, LPARAM lParam)
{
	switch (lParam) {
	case WM_LBUTTONDOWN:
		if (wParam == IDM_NOTIFYICON)
			TrackDriveMenu();
		break;

	case WM_RBUTTONDOWN:
		if (wParam == IDM_NOTIFYICON) {
			::SetForegroundWindow(m_hWnd);
			CPoint pt;
			GetCursorPos(&pt);
			ShowSysMenu(pt);
		}
		break;

	default:
		break;
	}
	return 0;
}

