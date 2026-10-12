////////////////////////////////////////////////////////////////////////
// mypanecontainer.h  (base WTL7.5)

#ifndef __MYPANECONTAINER_H__
#define __MYPANECONTAINER_H__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
	#error atlctrlx.h requires atlapp.h to be included first
#endif

#ifndef __ATLCTRLS_H__
	#error mypanecontainer.h requires atlctrls.h to be included first
#endif


#include "gcdata.h"

///////////////////////////////////////////////////////////////////////////////
//

class CIconInfo : public ICONINFO
{
private:
	BITMAP m_bm;
public:
	CIconInfo(HICON hic)
	{
		memset(&m_bm, 0, sizeof(m_bm));
		memset(this, 0, sizeof(ICONINFO));
		if (GetIconInfo(hic, this)) {
			::GetObject(hbmColor, sizeof(m_bm), &m_bm);
		}
	}
	~CIconInfo()
	{
		if (hbmMask) ::DeleteObject(hbmMask);
		if (hbmColor) ::DeleteObject(hbmColor);
	}
	int Width() { return m_bm.bmWidth; }
	int Height() { return m_bm.bmHeight; }
};

///////////////////////////////////////////////////////////////////////////////
// Classes in this file:
//
// CMyPaneContainerImpl<T, TBase, TWinTraits>
// CMyPaneContainer


namespace WTL
{

///////////////////////////////////////////////////////////////////////////////
// CMyPaneContainer - provides header with title and close button for panes

// pane container extended styles

#ifndef PANECNT_NOCLOSEBUTTON
#define PANECNT_NOCLOSEBUTTON		0x00000001
#endif

template <class T, class TBase = ATL::CWindow, class TWinTraits = ATL::CControlWinTraits>
class ATL_NO_VTABLE CMyPaneContainerImpl : public ATL::CWindowImpl< T, TBase, TWinTraits >
{
public:
	DECLARE_WND_CLASS_EX(NULL, CS_DBLCLKS, -1)

// Constants
	enum
	{
		m_cxyBorder = 2,
		m_cxyTextOffset = 3,
		m_cxyBtnOffset = 1,
		m_xBtnImage = 9,
		m_yBtnImage = 8,
		m_cxyBtnAddTB = 6,
		m_cxToolBar = m_xBtnImage + m_cxyBtnAddTB + m_cxyBorder + m_cxyBtnOffset,

		m_nCloseBtnID = ID_PANE_CLOSE
	};

// Data members
	CToolBarCtrl m_tb;
	ATL::CWindow m_wndClient;
	int m_cxyHeader;
	CString m_strTitle;
	DWORD m_dwExtendedStyle;   // Pane container specific extended styles
	HIMAGELIST m_himNormal;
	HIMAGELIST m_himHot;
	float m_cyChar;
	Font* m_pLabelFont;
	HICON m_hIcon;

// Constructor
	CMyPaneContainerImpl()
		: m_cxyHeader(0)
		, m_dwExtendedStyle(0)
		, m_pLabelFont(NULL)
		, m_hIcon(NULL)
		, m_himNormal(NULL)
		, m_himHot(NULL)
	{
		// ヘッダテキストは8pointフォント
		CClientDC dc(NULL);
		m_cyChar = 9.0F * dc.GetDeviceCaps(LOGPIXELSY) / 72.0F;
	}

	~CMyPaneContainerImpl()
	{
		if (m_hIcon) ::DestroyIcon(m_hIcon);
	}

	// Attributes
	DWORD GetPaneContainerExtendedStyle() const
	{
		return m_dwExtendedStyle;
	}

	DWORD SetPaneContainerExtendedStyle(DWORD dwExtendedStyle, DWORD dwMask = 0)
	{
		DWORD dwPrevStyle = m_dwExtendedStyle;
		if (dwMask == 0)
			m_dwExtendedStyle = dwExtendedStyle;
		else
			m_dwExtendedStyle = (m_dwExtendedStyle & ~dwMask) | (dwExtendedStyle & dwMask);
		if (m_hWnd != NULL) {
			T* pT = static_cast<T*>(this);
			bool bUpdate = false;

			if(((dwPrevStyle & PANECNT_NOCLOSEBUTTON) != 0) && ((m_dwExtendedStyle & PANECNT_NOCLOSEBUTTON) == 0))   // add close button
			{
				pT->CreateCloseButton();
				bUpdate = true;
			}
			else if(((dwPrevStyle & PANECNT_NOCLOSEBUTTON) == 0) && ((m_dwExtendedStyle & PANECNT_NOCLOSEBUTTON) != 0))   // remove close button
			{
				pT->DestroyCloseButton();
				bUpdate = true;
			}

			if(bUpdate)
				pT->UpdateLayout();
		}
		return dwPrevStyle;
	}

	HWND GetClient() const
	{
		return m_wndClient;
	}

	HWND SetClient(HWND hWndClient)
	{
		HWND hWndOldClient = m_wndClient;
		m_wndClient = hWndClient;
		if (m_hWnd != NULL) {
			T* pT = static_cast<T*>(this);
			pT->UpdateLayout();
		}
		return hWndOldClient;
	}

	BOOL GetTitle(LPTSTR lpstrTitle, int cchLength) const
	{
		ATLASSERT(lpstrTitle != NULL);
		return (lstrcpyn(lpstrTitle, m_strTitle, cchLength) != NULL);
	}

	void SetTitle(LPCTSTR lpstrTitle)
	{
		ATLASSERT(lpstrTitle != NULL);
		m_strTitle = lpstrTitle;
		m_hIcon = hIcon;
		if (m_hWnd != NULL) {
			T* pT = static_cast<T*>(this);
			pT->UpdateLayout();
		}
	}

	int GetTitleLength() const
	{
		return m_strTitle.GetLength();
	}

	HICON SetIcon(HICON hIcon)
	{
		HICON hicPrev = m_hIcon;
		m_hIcon = hIcon;
		return hicPrev;
	}

// Methods
	HWND Create(HWND hWndParent, ATL::_U_STRINGorID title, DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			DWORD exStyle = 0, UINT nID = 0, LPVOID lpCreateParam = NULL)
	{
		m_strTitle = title.m_lpstr;
#if (_MSC_VER >= 1300)
		return ATL::CWindowImpl< T, TBase, TWinTraits >::Create(hWndParent, rcDefault, NULL, style, exStyle, nID, lpCreateParam);
#else //!(_MSC_VER >= 1300)
		typedef ATL::CWindowImpl< T, TBase, TWinTraits >   _baseClass;
		return _baseClass::Create(hWndParent, rcDefault, NULL, style, exStyle, nID, lpCreateParam);
#endif //!(_MSC_VER >= 1300)
	}

	BOOL EnableCloseButton(BOOL bEnable)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		T* pT = static_cast<T*>(this);
		pT;   // avoid level 4 warning
		return (m_tb.m_hWnd != NULL) ? m_tb.EnableButton(pT->m_nCloseBtnID, bEnable) : FALSE;
	}

	void UpdateLayout()
	{
		RECT rcClient = { 0 };
		GetClientRect(&rcClient);
		T* pT = static_cast<T*>(this);
		pT->UpdateLayout(rcClient.right, rcClient.bottom);
	}

// Message map and handlers
	BEGIN_MSG_MAP(CMyPaneContainerImpl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_PRINTCLIENT, OnPaint)
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		pT->CalcSize();

		if ((m_dwExtendedStyle & PANECNT_NOCLOSEBUTTON) == 0)
			pT->CreateCloseButton();

		return 0;
	}

	LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		pT->UpdateLayout(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if (m_pLabelFont) {
			delete m_pLabelFont;
			m_pLabelFont = NULL;
		}
		return 0;
	}

	LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		if(m_wndClient.m_hWnd != NULL)
			m_wndClient.SetFocus();
		return 0;
	}

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		CDCHandle dc = (HDC)wParam;

		CRect rcc;
		GetClientRect(&rcc);

		CRect rc = rcc;
		rc.bottom = rc.top + m_cxyHeader;

		// using gdi+
/*
		Graphics g(dc);
		Rect r(rc.left, rc.top, rc.Width(), rc.Height());
		Color cF; cF.SetFromCOLORREF(GetSysColor(COLOR_3DHILIGHT));
		Color cT; cT.SetFromCOLORREF(GetSysColor(COLOR_3DFACE));
		LinearGradientBrush lbr(r, cF, cT, LinearGradientModeVertical);
		g.FillRectangle(&lbr, r);
*/
		dc.FillSolidRect(&rc, RGB(0xf8, 0xf8, 0xf8));
		rc.top = rc.bottom - 1;
		dc.FillSolidRect(&rc, GetSysColor(COLOR_3DFACE));

		rc = rcc;
		rc.top += m_cxyHeader;
		dc.FillSolidRect(&rc, GetSysColor(COLOR_APPWORKSPACE));

		return 1;   // no background needed
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		T* pT = static_cast<T*>(this);
		if (wParam != NULL) {
			pT->DrawPaneTitle((HDC)wParam);
			if (m_wndClient.m_hWnd == NULL)   // no client window
				pT->DrawPane((HDC)wParam);
		}
		else {
			CPaintDC dc(m_hWnd);
			pT->DrawPaneTitle(dc.m_hDC);
			if (m_wndClient.m_hWnd == NULL)   // no client window
				pT->DrawPane(dc.m_hDC);
		}

		return 0;
	}

	LRESULT OnNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		if(m_tb.m_hWnd == NULL)
		{
			bHandled = FALSE;
			return 1;
		}

		T* pT = static_cast<T*>(this);
		pT;
		LPNMHDR lpnmh = (LPNMHDR)lParam;
		LRESULT lRet = 0;

		// tooltip notifications come with the tooltip window handle and button ID,
		// pass them to the parent if we don't handle them
		if(lpnmh->code == TTN_GETDISPINFO && lpnmh->idFrom == pT->m_nCloseBtnID)
			bHandled = pT->GetToolTipText(lpnmh);
		// only let notifications not from the toolbar go to the parent
		else if(lpnmh->hwndFrom != m_tb.m_hWnd && lpnmh->idFrom != pT->m_nCloseBtnID)
			bHandled = FALSE;

		return lRet;
	}

	LRESULT OnLButtonDblClk(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

		CRect rc;
		GetClientRect(&rc);
		rc.bottom = rc.top + m_cxyHeader;
		if (rc.PtInRect(pt)) {
			NMMOUSE nmm = { 0 };
			nmm.hdr.hwndFrom = m_hWnd;
			nmm.hdr.idFrom = GetDlgCtrlID();
			nmm.hdr.code = NM_DBLCLK;
			nmm.pt = pt;
			ClientToScreen(&nmm.pt);
			::SendMessage(GetParent(), WM_NOTIFY, nmm.hdr.idFrom, (LPARAM)&nmm);
			return 0;
		}
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// if command comes from the close button, substitute HWND of the pane container instead
		if (m_tb.m_hWnd && (HWND)lParam == m_tb.m_hWnd)
			return ::SendMessage(GetParent(), WM_COMMAND, wParam, (LPARAM)m_hWnd);

		bHandled = FALSE;
		return 1;
	}

// Implementation - overrideable methods
	void UpdateLayout(int cxWidth, int cyHeight)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		CRect rc(0, 0, cxWidth, m_cxyHeader);
		if (m_tb.m_hWnd)
			m_tb.SetWindowPos(NULL, rc.right - m_cxToolBar - 1, 1 + m_cxyBtnOffset, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);

		if (m_wndClient.m_hWnd)
			m_wndClient.SetWindowPos(NULL, 0, m_cxyHeader, cxWidth, cyHeight - m_cxyHeader, SWP_NOZORDER);
		else
			rc.bottom = cyHeight;

		InvalidateRect(&rc);
	}

	void CreateCloseButton()
	{
		ATLASSERT(m_tb.m_hWnd == NULL);
		// create toolbar for the "x" button
		m_tb.Create(m_hWnd, rcDefault, NULL,
					WS_CHILD | WS_VISIBLE | /*WS_CLIPCHILDREN | WS_CLIPSIBLINGS |*/
					CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NOMOVEY |
					TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT,
					0);
		ATLASSERT(m_tb.IsWindow());

		if (m_tb.m_hWnd) {
			T* pT = static_cast<T*>(this);
			pT;   // avoid level 4 warning

			m_tb.SetButtonStructSize();

			TBBUTTON tbbtn = { 0 };
			tbbtn.idCommand = pT->m_nCloseBtnID;
			tbbtn.fsState = TBSTATE_ENABLED;
			tbbtn.fsStyle = TBSTYLE_BUTTON;
			m_tb.AddButtons(1, &tbbtn);

			m_tb.SetBitmapSize(m_xBtnImage, m_yBtnImage);

			CSize sizeButton(m_xBtnImage + m_cxyBtnAddTB, m_yBtnImage + m_cxyBtnAddTB);
			m_tb.SetButtonSize(sizeButton);

			BuildButtonImages();
			m_tb.SetImageList(m_himNormal);
			m_tb.SetHotImageList(m_himHot);

			m_tb.SetWindowPos(NULL, 0, 0, sizeButton.cx + 1, sizeButton.cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		}
	}

	void DestroyCloseButton()
	{
		if(m_tb.m_hWnd != NULL)
			m_tb.DestroyWindow();
	}

	void CalcSize()
	{
/* ---------
		T* pT = static_cast<T*>(this);
		CFontHandle font = pT->GetTitleFont();
		LOGFONT lf = { 0 };
		font.GetLogFont(lf);
		int cyFont = abs(lf.lfHeight) + 2 * m_cxyTextOffset;
		int cyBtn = m_yBtnImage + m_cxyBtnAddTB + 2 * m_cxyBtnOffset;
---------- */
		int cyFont = (int)ceil(m_cyChar) + 2 * m_cxyTextOffset;
		int cyBtn = m_yBtnImage + m_cxyBtnAddTB + 2 * m_cxyBtnOffset;
		m_cxyHeader = max(cyFont, cyBtn);
	}

	int GetHeaderHeight()
	{
		return m_cxyHeader;
	}

	HFONT GetTitleFont() const
	{
		return AtlGetDefaultGuiFont();
	}

	BOOL GetToolTipText(LPNMHDR /*lpnmh*/)
	{
		return FALSE;
	}

	void DrawPaneTitle(CDCHandle dc)
	{
		CRect rc;
		GetClientRect(&rc);
		rc.bottom = rc.top + m_cxyHeader;

		// draw title only for horizontal pane container
		rc.left += m_cxyTextOffset;
		rc.right -= m_cxyTextOffset;
		if (m_tb.m_hWnd != NULL)
			rc.right -= m_cxToolBar;;

		if (m_hIcon) {
			CIconInfo ii(m_hIcon);
			if (rc.Width() < ii.Width()) return;

			// アイコンスペースがあれば描く
			dc.DrawIconEx(rc.left, rc.top + (rc.Height() - ii.Height()) / 2, m_hIcon, 0, 0, 0, NULL, DI_NORMAL);
			rc.left += ii.Width() + 2;
		}

		if (rc.Width() > 0) {
			Graphics g(dc);
			RectF r((REAL)rc.left, (REAL)rc.top, (REAL)rc.Width(), (REAL)rc.Height());
			StringFormat fmt;
			fmt.SetFormatFlags(StringFormatFlagsNoWrap);				// DT_SINGLELINE
			fmt.SetAlignment(StringAlignmentNear);						// DT_LEFT
			fmt.SetTrimming(StringTrimmingEllipsisCharacter);			// followed by "..."
			fmt.SetLineAlignment(StringAlignmentCenter);				// DT_VCENTER
			COLORREF clr = ::GetSysColor(COLOR_WINDOWTEXT);
			SolidBrush sbr(Color(255, GetRValue(clr), GetGValue(clr), GetBValue(clr)));
			g.SetTextRenderingHint(TextRenderingHintAntiAlias);

			if (!m_pLabelFont)
				m_pLabelFont = new Font(L"MS UI Gothic", m_cyChar, FontStyleRegular, UnitPixel);
			g.DrawString(m_strTitle, m_strTitle.GetLength(), m_pLabelFont, r, &fmt, &sbr);
		}
	}

	// called only if pane is empty
	void DrawPane(CDCHandle dc)
	{
		CRect rc;
		GetClientRect(&rc);
		rc.top += m_cxyHeader;
		if ((GetExStyle() & WS_EX_CLIENTEDGE) == 0)
			dc.DrawEdge(&rc, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
		dc.FillRect(&rc, COLOR_APPWORKSPACE);
	}

	HIMAGELIST BuildButtonImageCore(const CRect& rcButton, HPEN hpen)
	{
		CRect rc = rcButton;
		rc.OffsetRect(-rcButton.left, -rcButton.top);

		HIMAGELIST him = ImageList_Create(rc.Width(), rc.Height(), ILC_COLORDDB | ILC_MASK, 1, 1);

		CClientDC dc(NULL);
		HBITMAP hbm = CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
		CDC dcw;
		dcw.CreateCompatibleDC(dc);
		HBITMAP hbmw = dcw.SelectBitmap(hbm);
		dcw.FillSolidRect(&rc, RGB(255, 0, 255));

		rc.InflateRect(-1, -1);
		HPEN hpenw = dcw.SelectPen(hpen);
		dcw.MoveTo(rc.left, rc.top);
		dcw.LineTo(rc.right - 1, rc.bottom);
		dcw.MoveTo(rc.left + 1, rc.top);
		dcw.LineTo(rc.right, rc.bottom);
		dcw.MoveTo(rc.left, rc.bottom - 1);
		dcw.LineTo(rc.right - 1, rc.top - 1);
		dcw.MoveTo(rc.left + 1, rc.bottom - 1);
		dcw.LineTo(rc.right, rc.top - 1);
		dcw.SelectPen(hpenw);
		dcw.SelectBitmap(hbmw);

		ImageList_AddMasked(him, hbm, RGB(255, 0, 255));
		::DeleteObject(hbm);

		return him;
	}

	void BuildButtonImages()
	{
		CRect rc(0, 0, m_xBtnImage, m_yBtnImage);
		HPEN hpen;
		if (!m_himNormal) {
			hpen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_3DSHADOW));
			m_himNormal = BuildButtonImageCore(rc, hpen);
			::DeleteObject(hpen);
		}
		if (!m_himHot) {
			hpen = CreatePen(PS_SOLID, 0, RGB(192, 0, 0));
			m_himHot = BuildButtonImageCore(rc, hpen);
			::DeleteObject(hpen);
		}
	}
};

class CMyPaneContainer : public CMyPaneContainerImpl<CMyPaneContainer>
{
public:
	DECLARE_WND_CLASS_EX(_T("LSP_PaneContainer"), 0, -1)
};


}; //namespace WTL

#endif // __MYPANECONTAINER_H__
