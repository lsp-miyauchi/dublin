////////////////////////////////////////////////////////////////////////
// tiphelp.h

#pragma once
#ifndef __TIPHELP_H__
#define __TIPHELP_H__

////////////////////////////////////////////////////////////////////////
// CTipHelp

class CTipHelp : public CToolTipCtrl
{
private:
	HWND m_hwndOwner;

public:
	CTipHelp() : m_hwndOwner(NULL)
	{
	}

	~CTipHelp()
	{
		Destroy();
	}

	HWND Create(HWND hwndParent, UINT idResource)
	{
		// Create the ToolTip control.
		m_hWnd = ::CreateWindowEx(WS_EX_TOOLWINDOW, TOOLTIPS_CLASS, NULL,
							WS_POPUP,
							CW_USEDEFAULT, CW_USEDEFAULT,
							CW_USEDEFAULT, CW_USEDEFAULT,
							NULL, (HMENU)NULL, ATL::_AtlBaseModule.GetModuleInstance(), NULL);
		if (!::IsWindow(m_hWnd))
			return NULL;

		SetMaxTipWidth(512);
		CRect rc(4, 4, 4, 4);
		SetMargin(&rc);

		m_hwndOwner = hwndParent;
		TOOLINFO ti;
		memset(&ti, 0, sizeof(ti));
		ti.cbSize = sizeof(ti);
		ti.uFlags = TTF_TRACK | TTF_ABSOLUTE | TTF_CENTERTIP;
		ti.uId    = (UINT_PTR)m_hwndOwner;

		CString str;
		if (!str.LoadString(idResource))
			str = _T("この項目に関連するトピックはありません。");
		ti.lpszText  = (LPTSTR)(LPCTSTR)str;

	    // Add the tool to the control, displaying an error if needed.
		if (!AddTool(&ti)) {
			DestroyWindow();
			return NULL;
		}

		TrackActivate(&ti, TRUE);
		return m_hWnd;
	}

	void Destroy()
	{
		if (::IsWindow(m_hWnd)) {
			Activate(FALSE);
			DestroyWindow();
		}
	}

	void Tracking(POINT point, LPTSTR pszTipString = NULL)
	{
		// point はスクリーン座標
		if (::IsWindow(m_hWnd)) {

			TrackPosition(point.x, point.y);
			if (pszTipString && *pszTipString) {
				TOOLINFO ti;
				memset(&ti, 0, sizeof(ti));
				ti.cbSize = sizeof(ti);
				ti.hwnd = m_hwndOwner;
				ti.uId = (UINT_PTR)m_hwndOwner;

				CString str;
				if (IS_INTRESOURCE(pszTipString))
					str.LoadString((UINT)(UINT_PTR)pszTipString);
				else
					str = pszTipString;

				ti.lpszText = (LPTSTR)(LPCTSTR)str;
				UpdateTipText(&ti);
			}

			DWORD timer = GetTickCount() + 6000;
			::SetCapture(m_hwndOwner);
			bool bCont = true;
			MSG msg;
			while (bCont && (timer > GetTickCount())) {
				while (::PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
					switch (msg.message) {
					case WM_KEYDOWN:
					case WM_SYSKEYDOWN:
					case WM_CANCELMODE:
					case WM_LBUTTONDOWN:
					case WM_MBUTTONDOWN:
					case WM_RBUTTONDOWN:
						bCont = false;
						break;
					default:
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
						break;
					}
				}
			}
			::ReleaseCapture();

			Destroy();
		}
	}

	void ShowTips(HWND hwndParent, UINT idResource)
	{
		HWND hwndCapture = GetCapture();
		if (hwndCapture != NULL)
			::SendMessage(hwndCapture, WM_CANCELMODE, 0, 0);

		CPoint point;
		::GetCursorPos(&point);
		if (Create(hwndParent, idResource) != NULL)
			Tracking(point);
	}
};

#endif		// __TIPHELP_H__
