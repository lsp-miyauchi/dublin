////////////////////////////////////////////////////////////////////////
// view_information.h

#pragma once
#ifndef __VIW_INFORMATION_H__
#define __VIW_INFORMATION_H__

#include "..\lspexif\lspexifapi.h"
#include "resource.h"
#include "histogram_calc.h"

#include "view_navigate.h"
#include "view_histogram.h"
#include "exifinfo.h"

#define IDW_EXIF							100
#define IDW_TAB								101
#define IDW_HISTOGRAM						102
#define IDW_NAVIGATE						103

#define IFS_HISTOGRAM						0x00000001
#define IFS_NAVIGATOR						0x00000002

///////////////////////////////////////////////////////////////////////
//

#define IFV_IMAGE_CHANGED					0x00000001
#define IFV_VISIBLEPOSITION_CHANGED			0x00000002
#define IFV_SCALE_CHANGED					0x00000003

class CInformationView : public CWindowImpl<CInformationView>
{
private:
	CExifView m_exif;							// EXIF情報リストウィンドウ
	CHistogramView m_histogram;					// Histogram情報ウィンドウ
	CNavigateView m_navi;						// ナビゲータウィンドウ
	CString m_strImageFile;

public:
	CInformationView()
	{
	}

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
	}

BEGIN_MSG_MAP(CInformationView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_COMMAND()
	ON_NOTIFY(NM_CUSTOMDRAW, IDW_EXIF, OnNotifyCustomDrawExif)
END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT pcs)
	{
		CAppReg reg;
		DWORD stat = reg.GetValue(IDS_SEC_MAIN_VIEWER, IDS_KEY_INFOVIEWSTATUS, IFS_HISTOGRAM | IFS_NAVIGATOR);

		DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		// ヒストグラム情報ウィンドウ
		m_histogram.Create(m_hWnd, rcDefault, NULL, style, 0, IDW_HISTOGRAM);
		m_histogram.ResizeWindow(!(stat & IFS_HISTOGRAM));
		// ナビウィンドウ
		m_navi.Create(m_hWnd, rcDefault, NULL, style, 0, IDW_NAVIGATE);
		m_navi.ResizeWindow(!(stat & IFS_NAVIGATOR));
		m_navi.SetOwnerInfo(pcs->hwndParent, HandleToULong(pcs->hMenu));
		// Exif情報ウィンドウ
		m_exif.Create(m_hWnd, rcDefault, NULL, style | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER | LVS_NOCOLUMNHEADER, 0, IDW_EXIF);
		m_exif.Initialize();
		return 0;
	}

	BOOL OnEraseBkgnd(HDC hdc)
	{
		return TRUE;
	}

	void OnDestroy()
	{
		DWORD stat = 0;
		if (!m_histogram.IsMinimized())	stat |= IFS_HISTOGRAM;
		if (!m_navi.IsMinimized())		stat |= IFS_NAVIGATOR;
		CAppReg reg;
		reg.SetValue(IDS_SEC_MAIN_VIEWER, IDS_KEY_INFOVIEWSTATUS, stat);
	}

	void OnSize(UINT nType, int cx, int cy)
	{
		if (nType != SIZE_MINIMIZED) {
			CRect rc(0, 0, cx, cy);
			RecalcLayout(rc);
		}
	}

	void RecalcLayout(CRect& rc)
	{
		HDWP hd = ::BeginDeferWindowPos(0);
		int height = m_histogram.GetWindowHeight();
		hd = ::DeferWindowPos(hd, m_histogram, NULL, rc.left, rc.top, rc.Width(), height, SWP_NOACTIVATE|SWP_NOZORDER);
		rc.top += height;
		height = m_navi.GetWindowHeight();
		hd = ::DeferWindowPos(hd, m_navi, NULL, rc.left, rc.top, rc.Width(), height, SWP_NOACTIVATE|SWP_NOZORDER);
		rc.top += height;
		hd = ::DeferWindowPos(hd, m_exif, NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOACTIVATE|SWP_NOZORDER);
		::EndDeferWindowPos(hd);
	}

	BOOL OnCommand(UINT id, UINT code, HWND hwnd)
	{
		if (id == ID_PANE_CLOSE) {
			if (hwnd == m_histogram)
				m_histogram.ResizeWindow(!m_histogram.IsMinimized());
			else if (hwnd == m_navi)
				m_navi.ResizeWindow(!m_navi.IsMinimized());
			else
				return FALSE;

			CRect rc;
			GetClientRect(&rc);
			RecalcLayout(rc);
			return TRUE;
		}
		else if (id == ID_WINDOW_ARRANGE) {
			if (hwnd == m_histogram)
				m_histogram.ResizeWindow(m_histogram.IsMinimized());
			else if (hwnd == m_navi)
				m_navi.ResizeWindow(m_navi.IsMinimized());
			else
				return FALSE;

			CRect rc;
			GetClientRect(&rc);
			RecalcLayout(rc);
			return TRUE;
		}
		return FALSE;
	}

	inline void OnNotifyCustomDrawExif(NMHDR* pNmhdr, LRESULT* pResult) { m_exif.ProcessCustomDraw((NMLVCUSTOMDRAW*)pNmhdr, pResult); }

	void ParseInformation(LPCTSTR pszFileName, HDIB hdib)
	{
		m_strImageFile = pszFileName;

		// EXIF情報抽出
		m_exif.ParseExif(pszFileName);
		// ナビ
		m_navi.SetImage(hdib);
		// ヒストグラム計算
		m_histogram.CalcHistogram(hdib);
	}

	LRESULT OnUpdate(UINT code, LPARAM param)
	{
		switch (code) {
		case IFV_IMAGE_CHANGED:
			m_navi.SetImage((HDIB)param);
			return 0;
		case IFV_VISIBLEPOSITION_CHANGED:
			m_navi.UpdateVisiblePosition((RECT*)param);
			return 0;
		case IFV_SCALE_CHANGED:
			m_navi.UpdateScale((double*)param);
			return 0;
		default:
			break;
		}
		return 0;
	}





};

#endif			// __VIW_INFORMATION_H__
