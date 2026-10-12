////////////////////////////////////////////////////////////////////////
// navigator.h

#pragma once
#ifndef __NAVIGATOR_H__
#define __NAVIGATOR_H__

#include <math.h>
#include "view_histogram.h"
#include "view_navigate.h"

#define IDW_HISTOGRAM						102
#define IDW_NAVIGATE						103

#define MIN_THUMBSIZE_X						128
#define MIN_THUMBSIZE_Y						128
#define MAX_THUMBSIZE_X						320
#define MAX_THUMBSIZE_Y						320

#define ROUND_INT(n)						((int)floor(0.5 + (n)))
#define FLOOR_INT(n)						((int)floor((n)))
#define CEIL_INT(n)							((int)ceil((n)))

#define NFS_HISTOGRAM						0x00000001
#define NFS_NAVIGATOR						0x00000002

#define NFV_IMAGE_CHANGED					0x00000001
#define NFV_VISIBLEPOSITION_CHANGED			0x00000002
#define NFV_SCALE_CHANGED					0x00000003


////////////////////////////////////////////////////////////////////////
// information for registry saving

typedef struct tagNAVIPOSINFO {
	WINDOWPLACEMENT wpm;
	int page;
} NAVIPOSINFO;

///////////////////////////////////////////////////////////////////////

#define IDC_TABCTRL				100
#define IDC_TRACKCTRL			101

#ifndef __EXIFINFO_H__

#define NVN_CLICK				1
#define NVN_CLOSE				2

typedef struct _NAVICLICK {
	NMHDR hdr;
	POINT pt;
} NAVICLICK;

#endif	// __EXIFINFO_H__


class CNaviWindow : public CWindowImpl<CNaviWindow>
{
private:
	CHistogramView m_histogram;					// Histogram情報ウィンドウ
	CNavigateView m_navi;						// ナビゲータウィンドウ

public:
	CNaviWindow()
	{
	}
	~CNaviWindow()
	{
	}

	DECLARE_WND_CLASS_EX(_T("NaviFrame"), CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, COLOR_WINDOW)

BEGIN_MSG_MAP(CNaviWindow)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_COMMAND()
	ON_MESSAGE(WM_GETMINMAXINFO, OnGetMinMaxInfo)
	MESSAGE_HANDLER(WM_CLOSE, OnHandlerClose)
	ON_WM_DESTROY()
END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT pcs)
	{
		CAppReg reg;
		DWORD stat = reg.GetValue(IDS_SEC_MAIN_EDITOR, IDS_KEY_INFOVIEWSTATUS, NFS_HISTOGRAM | NFS_NAVIGATOR);

		DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS;
		// ヒストグラム情報ウィンドウ
		m_histogram.Create(m_hWnd, rcDefault, NULL, style, 0, IDW_HISTOGRAM);
		m_histogram.ResizeWindow(!(stat & NFS_HISTOGRAM));
		// ナビウィンドウ
		m_navi.Create(m_hWnd, rcDefault, NULL, style, 0, IDW_NAVIGATE);
		m_navi.ResizeWindow(!(stat & NFS_NAVIGATOR));
		m_navi.SetOwnerInfo(pcs->hwndParent, PtrToLong(pcs->lpCreateParams));

		// これをおこなってautocalc
		SetWindowPos(NULL, pcs->x, pcs->y, pcs->cx, pcs->cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		return 0;
	}

	LRESULT OnGetMinMaxInfo(WPARAM wParam, LPARAM lParam)
	{
		::DefWindowProc(m_hWnd, WM_GETMINMAXINFO, wParam, lParam);

		MINMAXINFO* pMMI = (MINMAXINFO*)lParam;

		int height = m_histogram.GetWindowHeight() + m_navi.GetWindowHeight();
		// don't allow sizing smaller than the non-client area
		CRect rectWindow, rectClient;
		GetWindowRect(rectWindow);
		GetClientRect(rectClient);
		pMMI->ptMinTrackSize.x = rectWindow.Width() - rectClient.Width() + 128;
		pMMI->ptMinTrackSize.y = rectWindow.Height() - rectClient.Height() + height;
		pMMI->ptMaxTrackSize.x = rectWindow.Width() - rectClient.Width() + 360;
		pMMI->ptMaxTrackSize.y = rectWindow.Height() - rectClient.Height() + height;
		return 0;
	}

	LRESULT OnHandlerClose(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (IsWindowVisible())
			ShowWindow(SW_HIDE);
		return 0;
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
		int height = m_histogram.GetWindowHeight();
		m_histogram.MoveWindow(rc.left, rc.top, rc.Width(), height, TRUE);
		rc.top += height;
		height = m_navi.GetWindowHeight();
		m_navi.MoveWindow(rc.left, rc.top, rc.Width(), height, TRUE);
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
			GetWindowRect(&rc);
			SetWindowPos(NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			return TRUE;
		}
		return FALSE;
	}

	LRESULT OnUpdate(UINT code, LPARAM param)
	{
		switch (code) {
			case NFV_IMAGE_CHANGED:
				m_navi.SetImage((HDIB)param);
				m_histogram.CalcHistogram((HDIB)param);
				return 0;
			case NFV_VISIBLEPOSITION_CHANGED:
				m_navi.UpdateVisiblePosition((RECT*)param);
				return 0;
			case NFV_SCALE_CHANGED:
				m_navi.UpdateScale((double*)param);
				return 0;
			default:
				break;
			}
		return 0;
	}

	void OnDestroy()
	{
		DWORD stat = 0;
		if (!m_histogram.IsMinimized())	stat |= NFS_HISTOGRAM;
		if (!m_navi.IsMinimized())		stat |= NFS_NAVIGATOR;
		CAppReg reg;
		reg.SetValue(IDS_SEC_MAIN_EDITOR, IDS_KEY_INFOVIEWSTATUS, stat);
	}
};

////////////////////////////////////////////////////////////////////////
//


#endif			// __NAVIGATOR_H__
