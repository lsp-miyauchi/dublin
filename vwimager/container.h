///////////////////////////////////////////////////////////////////////
// container.h

#pragma once
#ifndef __VW_CONTAINER_H__
#define __VW_CONTAINER_H__

#include "colordefs.h"


template <class T, class TBase = CWindow, class TWinTraits = CControlWinTraits>
class ATL_NO_VTABLE CContainerImpl : public CWindowImpl< T, TBase, TWinTraits>
{
private:
	CToolBarCtrl m_toolbar;
	CImageList m_imNorm;
	CImageList m_imHot;
	CImageList m_imGray;
	int m_cyHeader;
	bool m_bMinimized:1;
	int m_cyPane;
	int m_cyChar;
	COLORREF m_clrBkgnd;
	CString m_strHeader;

	enum {
		m_xyBtnImage = 8,
		m_cxyBtnMargin = 5,
		m_idCommand = ID_PANE_CLOSE
	};

public:
	CContainerImpl()
		: m_bMinimized(false)
		, m_cyPane(128)
		, m_clrBkgnd(GetSysColor(COLOR_APPWORKSPACE))
	{
		CClientDC dc(NULL);
		HFONT hfont = dc.SelectFont((HFONT)::GetStockObject(DEFAULT_GUI_FONT));
		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);
		m_cyChar = tm.tmHeight + tm.tmExternalLeading;
		int cy = (int)GetSystemMetrics(SM_CYSMICON);
		m_cyHeader = max(cy, m_cyChar) + 4;
		dc.SelectFont(hfont);
	}

protected:
	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		m_imNorm.Destroy();
		m_imHot.Destroy();
		m_imGray.Destroy();
	}

	void CreateButtonImageList()
	{
		m_imNorm.Destroy();
		m_imHot.Destroy();
		m_imGray.Destroy();

		CClientDC dc(NULL);
		CDC dcw; dcw.CreateCompatibleDC(dc);
		HBITMAP hbm = ::CreateCompatibleBitmap(dc, m_xyBtnImage * 2, m_xyBtnImage);
		if (hbm) {
			m_imNorm.Create(m_xyBtnImage, m_xyBtnImage, ILC_COLORDDB | ILC_MASK, 2, 1);
			m_imHot.Create(m_xyBtnImage, m_xyBtnImage, ILC_COLORDDB | ILC_MASK, 2, 1);
			m_imGray.Create(m_xyBtnImage, m_xyBtnImage, ILC_COLORDDB | ILC_MASK, 2, 1);

			// Marlettフォント作成
			CFont font;
			font.CreateFont(12, 0, 0, 0, FW_NORMAL, 0, 0, 0, SYMBOL_CHARSET, 0, 0, 0, 0, _T("Marlett"));
			HFONT hfontw = dcw.SelectFont(font);

			COLORREF clrs[3] = { GetSysColor(COLOR_HIGHLIGHTTEXT), RGB(192, 0, 0), GetSysColor(COLOR_GRAYTEXT) };
			HIMAGELIST hims[3] = { m_imNorm, m_imHot, m_imGray };

			dcw.SetBkMode(TRANSPARENT);
			CRect rc(0, 0, m_xyBtnImage, m_xyBtnImage);
			for (int i = 0; i < 3; i++) {
				HBITMAP hbmw = dcw.SelectBitmap(hbm);
				dcw.FillSolidRect(0, 0, m_xyBtnImage * 2, m_xyBtnImage, RGB(255, 0, 255));
				dcw.SetTextColor(clrs[i]);
				dcw.DrawText(_T("6"), 1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				rc.OffsetRect(m_xyBtnImage, 0);
				dcw.DrawText(_T("4"), 1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				rc.OffsetRect(-m_xyBtnImage, 0);
				dcw.SelectBitmap(hbmw);
				ImageList_AddMasked(hims[i], hbm, RGB(255, 0, 255));
			}
			DeleteObject(hbm);
		}
	}

	void CreateButton()
	{
		// create imagelist
		CreateButtonImageList();

		// create toolbar
		m_toolbar.Create(m_hWnd, rcDefault, NULL,
					WS_CHILD | WS_VISIBLE | /*WS_CLIPCHILDREN | WS_CLIPSIBLINGS |*/
					CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NOMOVEY |
					TBSTYLE_FLAT | TBSTYLE_TRANSPARENT /*| TBSTYLE_TOOLTIPS*/,
					0);
		ATLASSERT(m_toolbar.IsWindow());
		if (m_toolbar.m_hWnd) {
			m_toolbar.SetButtonStructSize();

			TBBUTTON tbbtn = { 0 };
			tbbtn.iBitmap = m_bMinimized ? 1 : 0;
			tbbtn.idCommand = m_idCommand;
			tbbtn.fsState = TBSTATE_ENABLED;
			tbbtn.fsStyle = TBSTYLE_BUTTON;
			m_toolbar.AddButtons(1, &tbbtn);
			m_toolbar.SetBitmapSize(m_xyBtnImage, m_xyBtnImage);

			CSize sizeButton(m_xyBtnImage + m_cxyBtnMargin, m_xyBtnImage + m_cxyBtnMargin);
			m_toolbar.SetButtonSize(sizeButton);
			m_toolbar.SetImageList(m_imNorm);
			m_toolbar.SetHotImageList(m_imHot);
			m_toolbar.SetDisabledImageList(m_imGray);
			m_toolbar.SetWindowPos(NULL, 0, 0, sizeButton.cx + 1, sizeButton.cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		}
	}

	void ChangeButton()
	{
		if (m_toolbar.m_hWnd)
			m_toolbar.ChangeBitmap(m_idCommand, m_bMinimized ? 1 : 0);
	}

	void GetPaneRect(LPRECT prc)
	{
		ATLASSERT(prc != NULL);
		GetClientRect(prc);
		prc->top += m_cyHeader;
	}

	void SetPaneHeight(int height)
	{
		if (height > 0 && m_cyPane != height) {
			m_cyPane = height;
			if (::IsWindow(m_hWnd)) {
				// 親に通知して、親からレイアウト変更をおこなう
				GetParent().SendMessage(WM_COMMAND, ID_WINDOW_ARRANGE, (LPARAM)m_hWnd);
			}
		}
	}

BEGIN_MSG_MAP(CContainerImpl)
	MESSAGE_HANDLER(WM_CREATE, OnCTHandlerCreate)
	MESSAGE_HANDLER(WM_SIZE, OnCTHandlerSize)
	MESSAGE_HANDLER(WM_COMMAND, OnCTHandlerCommand)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnCTHandlerEraseBkgnd)
	MESSAGE_HANDLER(WM_PAINT, OnCTHandlerPaint)
END_MSG_MAP()

	LRESULT OnCTHandlerCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CreateButton();
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnCTHandlerSize(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		if (wParam != SIZE_MINIMIZED) {
			UpdateLayout(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		}
		return 0;
	}

	void UpdateLayout(int cx, int cy)
	{
		if (m_toolbar.m_hWnd) {
			CSize sizeButton(m_xyBtnImage + m_cxyBtnMargin, m_xyBtnImage + m_cxyBtnMargin);
			m_toolbar.SetWindowPos(NULL, cx - sizeButton.cx - 2, (m_cyHeader - sizeButton.cy) / 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
		}
		CRect rc(0, 0, cx, cy);
		rc.top += m_cyHeader;
		T* pT = static_cast<T*>(this);
		pT->RecalcLayout(rc);
	}

	virtual void RecalcLayout(CRect& rc)
	{
	}

	LRESULT OnCTHandlerCommand(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_toolbar.m_hWnd && (HWND)lParam == m_toolbar.m_hWnd) {
			return ::SendMessage(GetParent(), WM_COMMAND, wParam, (LPARAM)m_hWnd);
		}
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnCTHandlerEraseBkgnd(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		DrawHeader((HDC)wParam);
		return TRUE;
	}

	void DrawHeader(HDC hdc)
	{
		CDCHandle dc = hdc;

		// ヘッダ部背景
		CRect rc;
		GetClientRect(&rc);

		// ヘッダ矩形
		rc.bottom = rc.top + m_cyHeader;

		// ヘッダ描画
/*
		CRect rcw = rc;
		rcw.bottom = rcw.top + rc.Height() / 2;
		FillShadeRect(dc, &rcw, RGB(158, 158, 158), RGB(115, 115, 115));					// 上半分
		dc.FillSolidRect(rc.left, rcw.bottom, rc.right, rc.bottom, RGB(103, 103, 103));		// 下半分
		dc.FillSolidRect(rc.left, rc.top, rc.right, rc.top + 1, RGB(173, 173, 173));		// top border line
		dc.FillSolidRect(rc.left, rc.bottom - 1, rc.right, rc.bottom, RGB(149, 149, 149));	// bottom border line
*/
		dc.FillSolidRect(&rc, GetSysColor(COLOR_3DDKSHADOW));
		// ヘッダテキストを描画
		if (!m_strHeader.IsEmpty()) {
			Graphics g(dc);
			Font myFont(L"Segoe UI", (REAL)m_cyChar - 1, FontStyleRegular, UnitPixel);
			RectF r((REAL)rc.left + 4, (REAL)rc.top, (REAL)(rc.Width() - m_xyBtnImage - m_cxyBtnMargin - 2), (REAL)rc.Height());
			StringFormat fmt;
			fmt.SetFormatFlags(StringFormatFlagsNoWrap);				// DT_SINGLELINE
			fmt.SetAlignment(StringAlignmentNear);						// DT_LEFT
			fmt.SetTrimming(StringTrimmingEllipsisCharacter);			// followed by "..."
			fmt.SetLineAlignment(StringAlignmentCenter);				// DT_VCENTER
			SolidBrush sbr(Color(255, 255, 255, 255));
			g.SetTextRenderingHint(TextRenderingHintAntiAlias);
			g.DrawString(m_strHeader, m_strHeader.GetLength(), &myFont, r, &fmt, &sbr);
		}
	}

	LRESULT OnCTHandlerPaint(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		CPaintDC dc(m_hWnd);

		// ヘッダ部分を除去
		CRect rc;
		GetClientRect(&rc);
		rc.bottom = rc.top + m_cyHeader;
		dc.ExcludeClipRect(&rc);

		// 最小化していれば、これ以上は何もしない
		if (!m_bMinimized) {
			GetPaneRect(&rc);
			T* pT = static_cast<T*>(this);
			pT->OnDraw(dc.m_hDC, rc);
		}
		return 0;
	}

	virtual void OnDraw(CDCHandle dc, CRect& rc)
	{
	}

public:
	COLORREF SetBkColor(COLORREF clr, bool bRedraw = true)
	{
		COLORREF prev = m_clrBkgnd;
		if (m_clrBkgnd != clr) {
			m_clrBkgnd = clr;
			if (::IsWindow(m_hWnd) && bRedraw)
				Invalidate();
		}
		return prev;
	}

	void SetTitle(LPCTSTR pszTitle)
	{
		m_strHeader = pszTitle;
		if (m_hWnd && ::IsWindowVisible(m_hWnd)) {
			CRect rc;
			GetClientRect(&rc);
			rc.bottom = rc.top + m_cyHeader;
			InvalidateRect(&rc, TRUE);
		}
	}

	bool IsMinimized()
	{
		return m_bMinimized;
	}

	long GetWindowHeight()
	{
		return (m_bMinimized ? m_cyHeader : m_cyHeader + m_cyPane);
	}

	void ResizeWindow(bool bMinimized)
	{
		if (m_bMinimized ^ bMinimized) {
			m_bMinimized = bMinimized;
			ChangeButton();
		}
	}
};


#endif		// __VW_CONTAINER_H__
