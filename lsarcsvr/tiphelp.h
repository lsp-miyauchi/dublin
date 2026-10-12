////////////////////////////////////////////////////////////////////////
// tiphelp.h

#pragma once
#ifndef __TIPHELP_H__
#define __TIPHELP_H__

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

	HWND Create(HWND hwndParent)
	{
		// Create the ToolTip control.
		HINSTANCE hInst = ATL::_AtlBaseModule.GetModuleInstance();
		m_hWnd = ::CreateWindow(TOOLTIPS_CLASS, TEXT(""),
							WS_POPUP,
							CW_USEDEFAULT, CW_USEDEFAULT,
							CW_USEDEFAULT, CW_USEDEFAULT,
							NULL, (HMENU)NULL, hInst, NULL);
		if (!::IsWindow(m_hWnd))
			return NULL;

		SetMaxTipWidth(512);
		CRect rc(2, 2, 2, 2);
		SetMargin(&rc);

		m_hwndOwner = hwndParent;
		TOOLINFO ti;
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
		ti.hwnd   = m_hwndOwner;
		ti.uId    = (UINT_PTR)m_hwndOwner;
		ti.hinst  = hInst;
		ti.lpszText  = _T(" ");
		ti.rect.left = ti.rect.top = ti.rect.bottom = ti.rect.right = 0; 

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

	void Tracking(POINT point, LPTSTR pszTipString)
	{
		// point はスクリーン座標
		if (::IsWindow(m_hWnd)) {
			if (!pszTipString || *pszTipString == 0)
				pszTipString = _T("この項目に関連するトピックはありません。");

			TrackPosition(point.x, point.y);
			TOOLINFO ti;
			memset(&ti, 0, sizeof(ti));
			ti.cbSize = sizeof(ti);
			ti.hwnd = m_hwndOwner;
			ti.uId = (UINT_PTR)m_hwndOwner;
			ti.lpszText = pszTipString;
			UpdateTipText(&ti);

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

	void Tracking(POINT point, HINSTANCE hInst, UINT idResource)
	{
		CString str;
		if (str.LoadString(hInst, idResource))
			Tracking(point, (LPTSTR)(LPCTSTR)str);
	}
};


#endif		// __TIPHELP_H__
