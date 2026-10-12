// startup.h : header file
//

#pragma once
#ifndef __STARTUP_H__
#define __STARTUP_H__

#include <commoncontrols.h>
#include "resource.h"


HBITMAP LoadBitmapFromResource(HINSTANCE hInst, LPCTSTR typeResource, LPCTSTR pszResource);


#define	STARTUPTIMER_ID		1

////////////////////////////////////////////////////////////////
//

class CSplashWindow : public CWindowImpl<CSplashWindow>
{
private:
	UINT m_idResource;
	CBitmap m_bmp;
	int m_bmx;
	int m_bmy;
	UINT_PTR m_timerid;

public:
	CSplashWindow(UINT idResource = 0)
			: m_timerid(0)
	{
		m_idResource = idResource;
		m_bmx = m_bmy = 0;
	}

	HWND Create(HWND hwndParent)
	{
		return CWindowImpl::Create(hwndParent, NULL, 0, WS_POPUP | WS_VISIBLE, 0);
	}

BEGIN_MSG_MAP(CSplashWindow)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MSG_MAP()

	void MakeSplashBitmap()
	{
		BITMAP bm;
		m_bmp.GetBitmap(&bm);
		m_bmx = bm.bmWidth;
		m_bmy = bm.bmHeight;

		CClientDC dc(NULL);
		CDC dcw; dcw.CreateCompatibleDC(dc);
		HBITMAP hbmw = dcw.SelectBitmap(m_bmp);

		CRect rc(12, 20, m_bmx - 12, 68);

		TCHAR buf[MAX_PATH];
		GetModuleFileName(NULL, buf, ARRAYSIZE(buf));
		SHFILEINFO sfi;
		SHGetFileInfo(buf, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX);
		CComPtr<IImageList> pii;
		if (SHGetImageList(SHIL_EXTRALARGE, IID_PPV_ARGS(&pii)) == S_OK) {
			HICON hic;
			if (pii->GetIcon(sfi.iIcon, ILD_TRANSPARENT, &hic) == S_OK) {
				ICONINFO ii;
				if (GetIconInfo(hic, &ii)) {
					BITMAP bmw;
					GetObject(ii.hbmColor, sizeof(bmw), &bmw);
					if (ii.hbmColor) ::DeleteObject(ii.hbmColor);
					if (ii.hbmMask) ::DeleteObject(ii.hbmMask);
					dcw.DrawIconEx(rc.left, rc.top, hic, bmw.bmWidth, bmw.bmHeight, 0, NULL, DI_NORMAL);
					rc.left += bmw.bmWidth + 8;
					rc.bottom = rc.top + bmw.bmHeight;
				}
				DestroyIcon(hic);
			}
		}

		CFileVersion ver;
		if (ver.Open(buf)) {
			Graphics g(dcw);
			StringFormat fmt;
			fmt.SetFormatFlags(StringFormatFlagsNoWrap);				// DT_SINGLELINE
			fmt.SetAlignment(StringAlignmentNear);						// DT_LEFT
			fmt.SetTrimming(StringTrimmingEllipsisCharacter);			// followed by "..."
			fmt.SetLineAlignment(StringAlignmentCenter);				// DT_VCENTER
			SolidBrush sbr(Color(255, 255, 255, 255));
			g.SetTextRenderingHint(TextRenderingHintAntiAlias);

			// 説明
			if (ver.GetFileDescription(buf, ARRAYSIZE(buf))) {
				RectF r((REAL)rc.left, (REAL)rc.top, (REAL)rc.Width(), (REAL)rc.Height());
				Font myFont(L"Verdana", 30, FontStyleRegular, UnitPixel);
				g.DrawString(buf, lstrlen(buf), &myFont, r, &fmt, &sbr);
			}

			RectF r(12, (REAL)m_bmy - 34, (REAL)m_bmx - 24, 14);
			Font myFont(L"Tahoma", 12, FontStyleRegular, UnitPixel);
			// 著作権
			if (ver.GetLegalCopyright(buf, ARRAYSIZE(buf)))
				g.DrawString(buf, lstrlen(buf), &myFont, r, &fmt, &sbr);

			r.Offset(0, -14);
			// ファイルバージョン
			if (ver.GetFileVersion(buf)) {
				CString str;
#ifdef _WIN64
				str.Format(_T("version %s x64 edition"), buf);
#else
				str.Format(_T("version %s x86 edition"), buf);
#endif
				g.DrawString(str, lstrlen(str), &myFont, r, &fmt, &sbr);
			}

			ver.Close();
		}
		dcw.SelectBitmap(hbmw);
	}

	int OnCreate(LPCREATESTRUCT pcs)
	{
		m_bmp.Attach(LoadBitmapFromResource(RESINST, RT_RCDATA, MAKEINTRESOURCE(m_idResource)));
		if (m_bmp.IsNull())
			return -1;

		MakeSplashBitmap();

		CPoint pt;
		HMONITOR hmon = fvApp.StartMonitorHandle();
		if (hmon) {
			MONITORINFO mi = {0};
			mi.cbSize = sizeof(mi);
			GetMonitorInfo(hmon, &mi);
			pt.x = mi.rcWork.left + (mi.rcWork.right - mi.rcWork.left - m_bmx) / 2;
			pt.y = mi.rcWork.top + (mi.rcWork.bottom - mi.rcWork.top - m_bmy) / 3;
		} else {
			pt.x = (GetSystemMetrics(SM_CXSCREEN) - m_bmx) / 2;
			pt.y = (GetSystemMetrics(SM_CYSCREEN) - m_bmy) / 3;
		}
		SetWindowPos(HWND_TOPMOST, pt.x, pt.y, m_bmx, m_bmy, 0);

		m_timerid = SetTimer(STARTUPTIMER_ID, 1600, NULL);
		return 0;
	}

	void OnTimer(UINT_PTR timerid)
	{
		if (FindWindow(APP_FRAMECLASS, NULL)) {
			KillTimer(m_timerid);
			m_timerid = 0;
			DestroyWindow();
		}
	}

	BOOL OnEraseBkgnd(HDC hdc)
	{
		CDCHandle dc = hdc;
		CRect rc;
		GetClientRect(&rc);
		if (!m_bmp.IsNull()) {
			CDC dcw;
			dcw.CreateCompatibleDC(dc);
			HBITMAP hbm = dcw.SelectBitmap(m_bmp);
			dc.BitBlt(0, 0, m_bmx, m_bmy, dcw, 0, 0, SRCCOPY);
			dcw.SelectBitmap(hbm);
		}
		else
			dc.FillRect(&rc, GetSysColorBrush(COLOR_WINDOW));
		return TRUE;
	}

	void OnPaint()
	{
		CPaintDC dc(m_hWnd);
		CRect rc;
		GetClientRect(&rc);

/*
*/
	}

	void OnDestroy()
	{
		if (m_timerid) {
			KillTimer(m_timerid);
			m_timerid = 0;
		}
	}

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}
};


#endif				// __STARTUP_H__
