//////////////////////////////////////////////////////////////////////
// flattab.cpp 
//

//////////////////////////////////////////////////////////////////////
// CTabItem class header

class CTabItem
{
public:
	DWORD m_stat;
	CString m_strText;
	CString m_strTips;
	CRect m_rcItem;
	int m_iImage;
public:
	CTabItem()
	{
		m_stat = 0;
		m_strText.Empty();
		m_strTips.Empty();
		m_rcItem.SetRectEmpty();
		m_iImage = -1;
	};

	virtual ~CTabItem()
	{
	}

	CTabItem(const CTabItem& rhs)
	{
		*this = rhs;
	}

	const CTabItem& operator=(const CTabItem& rhs)
	{
		if (&rhs != this) {
			m_stat    = rhs.m_stat;
			m_strText = rhs.m_strText;
			m_strTips = rhs.m_strTips;
			m_rcItem  = rhs.m_rcItem;
			m_iImage  = rhs.m_iImage;
		}
		return *this;
	}

	__inline RECT GetRect() const { return m_rcItem; }
	__inline LPCRECT GetRectRef() const { return &m_rcItem; }
	__inline void SetRect(RECT rcItem) { m_rcItem = rcItem; }
	__inline BOOL PtInRect(POINT point) { return m_rcItem.PtInRect(point); }
	__inline int GetImageIndex() const { return m_nImage; }
	__inline void SetImageIndex(int nImage = -1) { m_nImage = nImage; }
	__inline CString GetText() const { return m_strText; }
	__inline LPCTSTR GetTextRef() const { return (LPCTSTR)m_strText; }
	__inline void SetText(LPCTSTR sNewText) { m_strText = sNewText; }
	__inline CString GetToolTip() const { return m_strTips; }
	__inline LPCTSTR GetToolTipRef() const { return (LPCTSTR)m_strTips; }
	__inline void SetToolTip(LPCTSTR sNewText) { m_strTips = sNewText; }


};

//////////////////////////////////////////////////////////////////////
// CFlatTabCtrl class header

class CFlatTabCtrl : public CWindowImpl<CFlatTabCtrl>,
					public CThemeImpl<CFlatTabCtrl>
{
private:
	HIMAGELIST m_hImage;
	int m_cxBitmap;
	int m_cyBitmap;
	HFONT m_hFont;
	HBITMAP m_hbm;
	COLORREF m_clrBack;
	COLORREF m_clrFocus;
	bool m_bIn:1;
	bool m_bFocus:1;
	bool m_bClicked:1;
	bool m_bTrackMouseEvent:1;
	bool m_bCapture:1;
	bool m_bAlphaImage:1;
	CAtlArray<CTabItem*> m_array;

public:
	CFlatTabCtrl();
	~CFlatTabCtrl() { }

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

BEGIN_MSG_MAP(CFlatTabCtrl)
	CHAIN_MSG_MAP(CThemeImpl<CFlatTabCtrl>)
	ON_WM_CREATE()
	ON_WM_GETDLGCODE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SETFONT()
	ON_MESSAGE_VOID(WM_ENABLE, OnEnable)
	ON_MESSAGE_VOID(BM_CLICK, OnTabClick)
	ON_WM_SIZE()
	ON_WM_PRINTCLIENT()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_DESTROY()
END_MSG_MAP()


protected:
	// メッセージマップ
	int OnCreate(LPCREATESTRUCT lpcs);
	__inline UINT OnGetDlgCode() { return DLGC_BUTTON; }
	__inline void OnSetFocus(HWND /*hwndOld*/) { OnChangeFocus(TRUE); }
	__inline void OnKillFocus(HWND /*hwndNew*/) { OnChangeFocus(FALSE); }
	__inline void OnEnable(WPARAM wParam, LPARAM lParam) { Invalidate(TRUE); }
	void OnChangeFocus(bool bFocus);
	void OnSetFont(HFONT hFon, BOOL bRedraw);
	void OnSize(UINT nType, int cx, int cy);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnPrintClient(HDC hdc, LPARAM flags);
	void OnPaint();
	void OnLButtonDown(UINT flags, CPoint point);
	void OnMouseMove(UINT flags, CPoint point);
	void OnMouseLeave();
	bool OnTrackMouse(const CRect& rcr);
 	void OnDestroy();
	void OnTabClick(WPARAM, LPARAM);

protected:
	void DrawTab(HDC hdc);
	void DrawThemeTab(HDC hdc);
	__inline void SendToParent() { GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED), (LPARAM)m_hWnd); }
	int ItemFromPoint(POINT point);
};

////////////////////////////////////////////////////////////////////////
// CFlatTabCtrl member function

CFlatTabCtrl::CFlatTabCtrl()
{
	m_hImage = NULL;
	m_cxBitmap = m_cyBitmap = 0;
	m_bIn = false;
	m_bFocus = false;
	m_bClicked = false;
	m_bTrackMouseEvent = false;
	m_bCapture = false;
	m_bAlphaImage = false;
	m_hFont = NULL;
}

int CFlatTabCtrl::OnCreate(LPCREATESTRUCT pcs)
{
	// Theme
	OpenThemeData(L"Tab");
	return 0;
}

void CFlatTabCtrl::OnSize(UINT nType, int cx, int cy)
{
	if (nType != SIZE_MINIMIZED) {
		;
	}
}

void CFlatTabCtrl::OnPrintClient(HDC hdc, LPARAM flags)
{
	if (flags & PRF_ERASEBKGND)
		DefWindowProc(WM_ERASEBKGND, (WPARAM)hdc, 0);

	if (flags & (PRF_CHECKVISIBLE | PRF_CLIENT | PRF_OWNED))
		DrawTab(hdc);
}

void CFlatTabCtrl::OnPaint()
{
	CPaintDC dc(m_hWnd);
	DrawTab(dc);
}

void CFlatTabCtrl::DrawThemeTab(HDC hdc)
{
	CRect rcc;
	GetClientRect(&rcc);
	CRect rc = rcc;

	BOOL bEnabled = IsWindowEnabled();
	DWORD style = GetStyle();

	CDCHandle dc = hdc;
	CDC dcw;
	dcw.CreateCompatibleDC(dc);
	HBITMAP hbmo = dcw.SelectBitmap(m_hbm);

	// 背面を塗りつぶす
	HBRUSH hbr = (HBRUSH)GetParent().SendMessage(WM_CTLCOLORSTATIC, (WPARAM)dcw.m_hDC, (LPARAM)m_hWnd);
	if (hbr)
		dcw.FillRect(&rc, hbr);
	else {
		COLORREF clr = dcw.SetBkColor(GetSysColor(COLOR_3DFACE));
		dcw.ExtTextOut(0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
		dcw.SetBkColor(clr);
	}

	CPoint pt(0, 0);
	int iState;
	if (style & FBS_PUSHBUTTON) {
		if (!bEnabled)
			iState = PBS_DISABLED;
		else if (m_bClicked) {
			iState = PBS_PRESSED;
			pt.Offset(1, 1);
		}
		else {
			iState = m_bIn ? PBS_HOT : (m_bFocus ? PBS_DEFAULTED : PBS_NORMAL);
		}
		DrawThemeBackground(dcw, BP_PUSHBUTTON, iState, &rc, NULL);
	}
	else {
		if (!bEnabled)
			iState = TS_DISABLED;
		else if (m_bClicked) {
			iState = TS_PRESSED; //HOTCHECKED;
			pt.Offset(1, 1);
		}
		else {
			if (style & FBS_FRAMEBORDER)
				iState = (m_bFocus || m_bIn) ? TS_HOTCHECKED : TS_HOT;
			else
				iState = (m_bFocus || m_bIn) ? TS_HOT : TS_NORMAL;
		}
		DrawThemeBackground(dcw, TP_BUTTON, iState, &rc, NULL);
	}

	int iImage = 0;
	if (!bEnabled)
		iImage = 2;
	else if (m_bIn || m_bFocus)
		iImage = 1;

	rc = rcc;
	rc.DeflateRect(1, 1);
	if (style & FBS_DROPDOWN) {
		rc.right -= GetDropDownWidth();
	}

	CString str;
	if (GetWindowText(str) > 0) {
		HFONT hf = NULL;
		if (m_hFont)
			hf = dcw.SelectFont(m_hFont);

		CRect rcw(0, 0, 0, 0);
		dcw.DrawText(str, -1, &rcw, DT_CALCRECT | DT_SINGLELINE);
		int width = rcw.Width() + 2;		// 行の長さを計算
		int hole_width = width;				// イメージを含めた全体の長さ

		if (m_hImage) {
			hole_width += m_cxBitmap + 2;
			pt.x += (rc.Width() - hole_width) >> 1;
			pt.y += (rc.Height() - m_cyBitmap) >> 1;
			DrawAlphaImageList(m_hImage, iImage, dcw, rc.left + pt.x, rc.top + pt.y, m_bAlphaImage);
		}

		// テキスト描画矩形の計算
		rcw = rc;
		rcw.left += (hole_width - width);
		if (m_bClicked)
			rcw.OffsetRect(1, 1);

		dcw.SetBkMode(TRANSPARENT);
		if (!bEnabled) {
			rcw.OffsetRect(1, 1);
			dcw.SetTextColor(GetSysColor(COLOR_BTNHIGHLIGHT));
			dcw.DrawText(str, -1, &rcw, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			rcw.OffsetRect(-1, -1);
			dcw.SetTextColor(GetSysColor(COLOR_BTNSHADOW));
		}
		else
			dcw.SetTextColor(GetSysColor(COLOR_BTNTEXT));
		dcw.DrawText(str, -1, &rcw, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		if (hf)
			dcw.SelectFont(hf);
	}
	else {
		if (m_hImage) {
			pt.x += (rc.Width() - m_cxBitmap) >> 1;
			pt.y += (rc.Height() - m_cyBitmap) >> 1;
			DrawAlphaImageList(m_hImage, iImage, dcw, rc.left + pt.x, rc.top + pt.y, m_bAlphaImage);
		}
	}

	if (style & FBS_DROPDOWN) {
		rc.left = rc.right;
		rc.right = rcc.right - 1;
		if (m_bClicked)
			rc.OffsetRect(1, 1);
		DrawDropDown(dcw, rc);
	}

	dc.BitBlt(rcc.left, rcc.top, rcc.Width(), rcc.Height(), dcw, 0, 0, SRCCOPY);
	dcw.SelectBitmap(hbmo);
}

void CFlatTabCtrl::DrawTab(HDC hdc)
{
	if (!IsThemeNull()) {
		DrawThemeButton(hdc);
		return;
	}

	CRect rcc;
	GetClientRect(&rcc);
	CRect rc = rcc;

	CDCHandle dc = hdc;
	CDC dcw;
	dcw.CreateCompatibleDC(dc);
	HBITMAP hbmo = dcw.SelectBitmap(m_hbm);

	DWORD style = GetStyle();

	// 背面を塗りつぶす
	COLORREF clr = (m_clrBack == INVALID_COLOR) ? GetSysColor(COLOR_3DFACE) : m_clrBack;
	if ((style & FBS_COLOREDFOCUS) &&
		(m_clrFocus != INVALID_COLOR) &&
		(m_bIn || m_bFocus))
		clr = m_clrFocus;

	clr = dcw.SetBkColor(clr);
	dcw.ExtTextOut(0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	dcw.SetBkColor(clr);

	// 枠描画
	CPoint pt(0, 0);
	if (m_bClicked) {
		draw_rect(dcw, rc, SEL_PUSH);
		pt.Offset(1, 1);
	}
	else {
		if (m_bFocus || m_bIn)				draw_rect(dcw, rc, SEL_POP);
		else if (style & FBS_FRAMEBORDER)	draw_rect(dcw, rc, SEL_BORDER);
		else								draw_rect(dcw, rc, SEL_NORMAL);
	}

	int iImage = 0;
	BOOL bEnabled = IsWindowEnabled();
	if (!bEnabled)
		iImage = 2;
	else if (m_bIn || m_bFocus)
		iImage = 1;

	rc = rcc;
	rc.DeflateRect(1, 1);
	if (style & FBS_DROPDOWN) {
		rc.right -= GetDropDownWidth();
	}

	CString str;
	if (GetWindowText(str) > 0) {
		HFONT hf = NULL;
		if (m_hFont)
			hf = dcw.SelectFont(m_hFont);

		CRect rcw(0, 0, 0, 0);
		dcw.DrawText(str, -1, &rcw, DT_CALCRECT | DT_SINGLELINE);
		int width = rcw.Width() + 2;		// 行の長さを計算
		int hole_width = width;				// イメージを含めた全体の長さ

		if (m_hImage) {
			hole_width += m_cxBitmap + 2;
			pt.x += (rc.Width() - hole_width) >> 1;
			pt.y += (rc.Height() - m_cyBitmap) >> 1;
			DrawAlphaImageList(m_hImage, iImage, dcw, rc.left + pt.x, rc.top + pt.y, m_bAlphaImage);
		}

		// テキスト描画矩形の計算
		rcw = rc;
		rcw.left += (hole_width - width);
		if (m_bClicked)
			rcw.OffsetRect(1, 1);

		dcw.SetBkMode(TRANSPARENT);
		if (!bEnabled) {
			rcw.OffsetRect(1, 1);
			dcw.SetTextColor(GetSysColor(COLOR_BTNHIGHLIGHT));
			dcw.DrawText(str, -1, &rcw, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			rcw.OffsetRect(-1, -1);
			dcw.SetTextColor(GetSysColor(COLOR_BTNSHADOW));
		}
		else
			dcw.SetTextColor(GetSysColor(COLOR_BTNTEXT));
		dcw.DrawText(str, -1, &rcw, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		if (hf)
			dcw.SelectFont(hf);
	}
	else {
		if (m_hImage) {
			pt.x += (rc.Width() - m_cxBitmap) >> 1;
			pt.y += (rc.Height() - m_cyBitmap) >> 1;
			DrawAlphaImageList(m_hImage, iImage, dcw, rc.left + pt.x, rc.top + pt.y, m_bAlphaImage);
		}
	}

	if (style & FBS_DROPDOWN) {
		rc.left = rc.right;
		rc.right = rcc.right - 1;
		if (m_bClicked)
			rc.OffsetRect(1, 1);
		DrawDropDown(dcw, rc);
	}

	dc.BitBlt(rcc.left, rcc.top, rcc.Width(), rcc.Height(), dcw, 0, 0, SRCCOPY);
	dcw.SelectBitmap(hbmo);
}

void CFlatTabCtrl::OnChangeFocus(bool bFocus)
{
	m_bFocus = bFocus;
	CClientDC dc(m_hWnd);
	DrawTab(dc);
}

void CFlatTabCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	int cnt = (int)m_array.GetSize();
	if (cnt == 0 || m_iTab < 0) return;

	int iTab = m_iTab;
	switch (nChar) {
	case VK_LEFT:
		if (iTab > 0) iTab--;
		break;
	case VK_RIGHT:
		if (iTab < cnt - 1) iTab++;
		break;
	}

	if (iTab != m_iTab) {
		SelectTab(iTab);
	}
}

void CFlatTabCtrl::OnLButtonDown(UINT nFlags, CPoint pt)
{
	if (m_bCapture) return;

	// フォーカスを持ってなければ取得する
	if (!m_bFocus) {
		SetFocus();
		OnChangeFocus(TRUE);
	}

	CRect rc;
	GetClientRect(&rc);
	if (GetStyle() & FBS_DROPDOWN) {
		CClientDC dc(m_hWnd);
		m_bClicked = true;
		DrawTab(dc);
		SendToParent();
		m_bClicked = false;
		DrawTab(dc);
	}
	else if (OnTrackMouse(rc)) {
		SendToParent();
	}
}

bool CFlatTabCtrl::OnTrackMouse(const CRect& rcr)
{
	SetCapture();
	m_bCapture = true;
	m_bClicked = true;

	CClientDC dc(m_hWnd);
	DrawTab(dc);

	bool bCont = true;
	while (bCont) {
		MSG msg;
		while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			if (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST) {
				CPoint pt(LOWORD(msg.lParam), HIWORD(msg.lParam));
				m_bIn = (rcr.PtInRect(pt) != FALSE);

				if (msg.message == WM_LBUTTONUP || msg.message == WM_RBUTTONUP) {
					bCont = false;
					break;
				}
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	m_bClicked = false;
	DrawTab(dc);

	m_bCapture = false;
	ReleaseCapture();

	return m_bIn;
}

void CFlatTabCtrl::OnMouseMove(UINT nFlags, CPoint pt)
{
	if (m_bCapture) return;

	if (!m_bTrackMouseEvent) {
		CRect rc;
		GetClientRect(&rc);
		m_bIn = (rc.PtInRect(pt) != FALSE);

		TRACKMOUSEEVENT te;
		te.cbSize = sizeof(te);
		te.dwFlags = TME_LEAVE;
		te.hwndTrack = m_hWnd;
		te.dwHoverTime = HOVER_DEFAULT;
		m_bTrackMouseEvent = (_TrackMouseEvent(&te) != FALSE);

		if (m_bTrackMouseEvent) {
			CClientDC dc(m_hWnd);
			DrawTab(dc);
		}
	}
}

void CFlatTabCtrl::OnMouseLeave()
{
	if (m_bTrackMouseEvent) {
		m_bIn = false;
		CClientDC dc(m_hWnd);
		DrawTab(dc);
	}
	m_bTrackMouseEvent = false;
}

void CFlatTabCtrl::OnTabClick(WPARAM, LPARAM)
{
	OnKeyDown(VK_SPACE, 0, 0);
}

void CFlatTabCtrl::OnSetImageList(WPARAM, LPARAM lParam)
{
	HIMAGELIST him = (HIMAGELIST)lParam;
	if (m_hImage && !(GetStyle() & FBS_SHAREIMAGELIST))
		ImageList_Destroy(m_hImage);

	m_bAlphaImage = false;
	m_hImage = him;
	if (m_hImage)
		ImageList_GetIconSize(m_hImage, &m_cxBitmap, &m_cyBitmap);

	if (IsWindowVisible())
		Invalidate(TRUE);
}

void CFlatTabCtrl::OnSetBkColor(WPARAM, LPARAM lParam)
{
	COLORREF clr = (COLORREF)lParam;
	if (m_clrBack != clr) {
		m_clrBack = clr;
		if (IsWindowVisible()) Invalidate(TRUE);
	}
}

void CFlatTabCtrl::OnSetFont(HFONT hfont, BOOL bRedraw)
{
	m_hFont = hfont;
	if (bRedraw) Invalidate(TRUE);
}

void CFlatTabCtrl::OnDestroy()
{
	if (m_hImage && !(GetStyle() & FBS_SHAREIMAGELIST))
		ImageList_Destroy(m_hImage);
	if (m_hbm)
		DeleteObject(m_hbm);
	if (m_hFontDropDown)
		DeleteObject(m_hFontDropDown);
}

int CFlatTabCtrl::InsertItem(


int CFlatTabCtrl::ItemFromPoint(POINT point)
{
	int i, cnt = (int)m_array.GetSize();
	for (i = 0; i < cnt; i++) {
		CTabItem* pti = m_array[i];
		if (pti && pti->PtInRect(point))
			return i;
	}
	return -1;
}



////////////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK FlatTabWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CFlatTabCtrl* pWnd = new CFlatTabCtrl;
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}

