// picker.h

#pragma once
#ifndef __PICKER_H__
#define __PICKER_H__

#include "buttonframe.h"


#define MY_THEME_CLASS			L"Toolbar"

#define PICKER_CENTERTEXT		0x00000001


template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CPickerCtrlImpl : public CWindowImpl< T, TBase, TWinTraits>, public CThemeImpl<T>
{
protected:
	BOOL m_bTrackMouseEvent;
	HFONT m_hFont;
	CRect m_rcText;
	CRect m_rcButton;
	CString m_strDisplayName;
	HIMAGELIST m_himButton;
	HIMAGELIST m_himLabel;
	int m_iLabelImage;
	int m_nDropDownWidth;
	CTheme m_themeButton;
	CSize m_sizeLabel;
	CSize m_sizeButton;
	bool m_bCaptured:1;
	bool m_bPressed:1;
	bool m_bHasFocus:1;
	bool m_bHot:1;
	bool m_bLeftButton:1;
	bool m_bViena:1;
	DWORD m_exPickerStyle;

public:
	CPickerCtrlImpl()
		: m_bTrackMouseEvent(FALSE)
		, m_himButton(NULL)
		, m_himLabel(NULL)
		, m_iLabelImage(-1)
		, m_nDropDownWidth(-1)
		, m_hFont(NULL)
		, m_bCaptured(false)
		, m_bPressed(false)
		, m_bHasFocus(false)
		, m_bHot(false)
		, m_bLeftButton(false)
		, m_exPickerStyle(0)
		, m_bViena((lccGetVersion() >= 0x700))
	{
		m_sizeLabel.SetSize(0, 0);
		m_sizeButton.SetSize(0, 0);

		// Theme
		SetThemeClassList(L"Edit");
		SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE, 0);
	}

	~CPickerCtrlImpl()
	{
		if (m_himButton) ImageList_Destroy(m_himButton);
	}

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

	DWORD GetExtendedStyle()
	{
		return m_exPickerStyle;
	}

	DWORD SetExtendedStyle(DWORD dwMask, DWORD dwExStyle)
	{
		DWORD newStyle = (m_exPickerStyle & ~dwMask) | dwExStyle;
		if (newStyle != m_exPickerStyle) {
			m_exPickerStyle = newStyle;
			if (::IsWindow(m_hWnd) && ::IsWindowVisible(m_hWnd))
				Invalidate(NULL);
		}
		return m_exPickerStyle;
	}

typedef CPickerCtrlImpl< T, TBase, TWinTraits >	thisClass;
BEGIN_MSG_MAP(this)
	CHAIN_MSG_MAP(CThemeImpl<T>)
	ON_WM_GETDLGCODE()
	MESSAGE_HANDLER(WM_CREATE, OnHandlerCreate)
	ON_WM_SIZE()
	ON_WM_SETFONT()
	ON_WM_ENABLE()
	ON_WM_PRINTCLIENT()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_CANCELMODE()
	ON_WM_DESTROY()
	ON_WM_THEMECHANGED()
END_MSG_MAP()

	LRESULT OnHandlerCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		if (IsThemingSupported() && IsThemeActive()) {
			m_themeButton.OpenThemeData(m_hWnd, MY_THEME_CLASS);
		}
		return 0;
	}

	LRESULT OnGetDlgCode()
	{
		return DLGC_BUTTON;
	}

	void OnSize(UINT flags, int cx, int cy)
	{
		int btnWidth = m_sizeButton.cx + 6;
		if (!m_bLeftButton) {
			m_rcText.SetRect(0, 0, cx - btnWidth - 1, cy);
			m_rcText.InflateRect(-1, -1);
			m_rcButton.SetRect(cx - btnWidth, 0, cx, cy);
		}
		else {
			m_rcButton.SetRect(0, 0, btnWidth, cy);
			m_rcText.SetRect(0, 0, cx, cy);
			m_rcText.left += btnWidth + 1;
			m_rcText.InflateRect(-1, -1);
		}
	}

	void OnSetFocus(HWND hOldWnd)
	{
		ChangeFocus(true);
	}

	void OnKillFocus(HWND hNewWnd)
	{
		ChangeFocus(false);
	}

	void OnEnable(BOOL bEnable)
	{
		DefWindowProc(); Invalidate();
	}

	void OnSetFont(HFONT hFont, BOOL bRedraw)
	{
		m_hFont = hFont;
		if (bRedraw)
			Invalidate(TRUE);
	}

	void OnPrintClient(HDC hdc, LPARAM flags)
	{
		if (flags & PRF_ERASEBKGND)
			SendMessage(WM_ERASEBKGND, (WPARAM)hdc, 0);

		if (flags & (PRF_CHECKVISIBLE | PRF_CLIENT | PRF_OWNED)) {
			DrawFace(hdc);
		}
	}

	void OnPaint()
	{
		CPaintDC dc(m_hWnd);
		DrawFace(dc);
	}

	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if ((nChar == VK_SPACE) & !m_bCaptured) {
			SetCapture();
			m_bCaptured = true;
			m_bPressed = true;

			InvalidateRect(&m_rcButton, TRUE);
		}
		else
			DefWindowProc();
	}

	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		if (nChar == VK_SPACE && m_bCaptured) {
			ReleaseCapture();
			m_bCaptured = false;

			T* pT = static_cast<T*>(this);
			pT->Browse();

			m_bPressed = false;
			InvalidateRect(&m_rcButton, TRUE);
		}
		else
			DefWindowProc();
	}

	void OnLButtonDown(UINT nFlags, CPoint pt)
	{
		// フォーカスを持ってなければ取得する
		if (!m_bHasFocus) {
			SetFocus();
			ChangeFocus(true);
		}

		if (m_rcButton.PtInRect(pt)) {
			SetCapture();
			m_bCaptured = true;
			m_bPressed = true;
			InvalidateRect(&m_rcButton, TRUE);
		}
	}

	void OnLButtonUp(UINT nFlags, CPoint pt)
	{
		if (m_bCaptured) {
			ReleaseCapture();
			m_bCaptured = false;

			T* pT = static_cast<T*>(this);
			if (m_rcButton.PtInRect(pt))
				pT->Browse();
			m_bPressed = false;
			InvalidateRect(&m_rcButton, TRUE);
		}
	}

	void OnMouseMove(UINT nFlags, CPoint point)
	{
		if (m_bCaptured) return;

		if (!m_bTrackMouseEvent) {
			TRACKMOUSEEVENT te;
			te.cbSize = sizeof(te);
			te.dwFlags = TME_LEAVE;
			te.hwndTrack = m_hWnd;
			te.dwHoverTime = HOVER_DEFAULT;
			m_bTrackMouseEvent = _TrackMouseEvent(&te);
		}

		if (m_bTrackMouseEvent) {
			bool bHot = (m_rcButton.PtInRect(point) != FALSE);
			if (m_bHot != bHot) {
				m_bHot = bHot;
				InvalidateRect(&m_rcButton, TRUE);
			}
		}
	}

	LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam)
	{
		m_bTrackMouseEvent = FALSE;
		m_bHot = false;
		InvalidateRect(&m_rcButton, TRUE);
		return 0;
	}

	void OnCancelMode()
	{
		if (!m_bCaptured) return;

		ReleaseCapture();
		m_bCaptured = false;
	}

	void OnDestroy()
	{
		m_themeButton.CloseThemeData();
	}

	void OnThemeChanged()
	{
		m_themeButton.CloseThemeData();
		m_themeButton.OpenThemeData(m_hWnd, MY_THEME_CLASS);
	}

	void DrawFace(HDC hdc)
	{
		CDCHandle dc = hdc;
		BOOL bEnabled = IsWindowEnabled();

		CRect rc;
		GetClientRect(&rc);
		dc.FillRect(&rc, GetSysColorBrush(bEnabled ? COLOR_WINDOW : COLOR_BTNFACE));

		rc = m_rcButton;
		if (!m_themeButton.IsThemeNull()) {
			int iState = TS_NORMAL;
			if (!bEnabled)						iState = TS_DISABLED;
			else {
				if (m_bPressed)					iState = TS_PRESSED;
				else if (m_bHot)				iState = TS_HOT;
			}
			m_themeButton.DrawThemeBackground(hdc, TP_BUTTON, iState, &rc, NULL);
		}
		else {
			UINT state = DFCS_BUTTONPUSH;
			if (!bEnabled)						state |= DFCS_INACTIVE;
			else {
				if (m_bPressed)					state |= DFCS_PUSHED;
				else if (m_bHot)				state |= DFCS_HOT;
			}
			dc.DrawFrameControl(&rc, DFC_BUTTON, state);
		}

		if (m_himButton) {
			int numImage = ImageList_GetImageCount(m_himButton);
			int x = rc.left + (rc.Width() - m_sizeButton.cx) / 2;
			int y = rc.top + (rc.Height() - m_sizeButton.cy) / 2;
			if (m_bPressed) {
				x++;
				y++;
			}
			int index = 0;
			if (numImage >= 3)
				index = bEnabled ? (m_bHot ? 1 : 0) : 2;
			else if (numImage >= 2)
				index = bEnabled ? 0 : 1;
			ImageList_Draw(m_himButton, index, dc, x, y, ILD_TRANSPARENT);
		}

		T* pT = static_cast<T*>(this);
		pT->DrawTextFace(dc, &m_rcText);
	}

	virtual void DrawTextFace(HDC hdc, RECT* prc)
	{
		CDCHandle dc = hdc;
		HFONT hfOld = NULL;
		if (m_hFont)
			hfOld = dc.SelectFont(m_hFont);

		// フォーカスありならば
		if (m_bHasFocus && m_bViena)
			lccDrawSelectionFrame(hdc, prc, TRUE, -1);

		// テキスト長の計算
		CSize sizeText(0, 0);
		if (!m_strDisplayName.IsEmpty()) {
			CRect rct(0, 0, 0, 0);
			sizeText.cy = dc.DrawText(m_strDisplayName, -1, &rct, DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX);
			sizeText.cx = rct.Width();
		}

		CRect rc = *prc;
		rc.InflateRect(-2, 0);

		// 中央配置の際の位置計算
		if (m_bViena && (m_exPickerStyle & PICKER_CENTERTEXT)) {
			int label_cx = m_himLabel ? (m_sizeLabel.cx + 4) : 0;
			rc.left += (rc.Width() - (label_cx + sizeText.cx)) / 2;
		}

		// ラベル描画
		if (m_himLabel) {
			ImageList_Draw(m_himLabel, m_iLabelImage, dc,
								rc.left, rc.top +  (rc.Height() - m_sizeLabel.cy) / 2, ILD_TRANSPARENT);
			rc.left += m_sizeLabel.cx + 2;
		}

		// テキスト
		if (sizeText.cx) {
			rc.left += 2;
			if (m_bHasFocus && !m_bViena) {
				CRect rcl = rc;
				rcl.right = rcl.left + sizeText.cx;
				rcl.top += (rc.Height() - sizeText.cy) / 2;
				rcl.bottom = rcl.top + sizeText.cy;
				rcl.InflateRect(1, 1);

				rcl.top = rcl.top + (rcl.Height() * 2 / 3);
				FillShadeRect(dc, rcl, GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), TRUE);
			}

			int bkmode = dc.SetBkMode(TRANSPARENT);
			COLORREF clr = dc.SetTextColor(GetSysColor(IsWindowEnabled() ? COLOR_WINDOWTEXT : COLOR_GRAYTEXT));
			dc.DrawText(m_strDisplayName, -1, &rc, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
			dc.SetTextColor(clr);
			dc.SetBkMode(bkmode);

		}

		if (hfOld)
			dc.SelectFont(hfOld);
	}

	void ChangeFocus(bool bFocus)
	{
		m_bHasFocus = bFocus;
		CClientDC dc(m_hWnd);
		T* pT = static_cast<T*>(this);
		pT->DrawFace(dc);
	}

	BOOL IsCaptured()
	{
		return m_bCaptured;
	}

	HIMAGELIST SetButtonImageList(HIMAGELIST him)
	{
		HIMAGELIST htemp = m_himButton;
		m_himButton = him;
		if (m_himButton) {
			int cx, cy;
			ImageList_GetIconSize(m_himButton, &cx, &cy);
			m_sizeButton.SetSize(cx, cy);
		}
		else
			m_sizeButton.SetSize(0, 0);
		return htemp;
	}

	HIMAGELIST SetLabelImageList(HIMAGELIST him)
	{
		HIMAGELIST htemp = m_himLabel;
		m_himLabel = him;
		if (m_himLabel) {
			int cx, cy;
			ImageList_GetIconSize(m_himLabel, &cx, &cy);
			m_sizeLabel.SetSize(cx, cy);
		}
		else
			m_sizeLabel.SetSize(0, 0);
		return htemp;
	}

	virtual void Browse()
	{
	}
};



#endif				// __PICKER_H__

