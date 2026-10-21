/////////////////////////////////////////////////////////////////////////
// flatbuton.cpp

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"
#include "common.h"
#include "gdiplus_sup.h"
#include "tooltip_sup.h"

/////////////////////////////////////////////////////////////////////////
//

#define BKRGB		(RGB(255,0,255))

class CFlatButtonCtrl
		: public CWindowImpl<CFlatButtonCtrl>
		, public CThemeImpl<CFlatButtonCtrl>
		, public CToolTipUsing<CFlatButtonCtrl>
{
private:
	HIMAGELIST m_hImage;
	HIMAGELIST m_himSystem;
	int m_cxImage;
	int m_cyImage;
	CSize m_sizeSystemIcon;
	int m_numImages;
	HFONT m_hFont;
	HFONT m_hFontDropDown;
	HBITMAP m_hbm;
	COLORREF m_clrBack;
	COLORREF m_clrFocus;
	int m_nDropDownWidth;
	int m_shieldIndex;
	CString m_tipMessage;
	bool m_bIn:1;
	bool m_bFocus:1;
	bool m_bKeyPressed:1;
	bool m_bClicked:1;
	bool m_bTrackMouseEvent:1;
	bool m_bCapture:1;
	bool m_bAlphaImage:1;
	bool m_bShieldIcon:1;

public:
	CFlatButtonCtrl()
		: m_hImage(NULL)
		, m_cxImage(0)
		, m_cyImage(0)
		, m_numImages(0)
		, m_bIn(false)
		, m_bFocus(false)
		, m_bKeyPressed(false)
		, m_bClicked(false)
		, m_bTrackMouseEvent(false)
		, m_bCapture(false)
		, m_bAlphaImage(false)
		, m_bShieldIcon(false)
		, m_hFont(NULL)
		, m_hbm(NULL)
		, m_clrBack(INVALID_COLOR)
		, m_clrFocus(RGB(202, 202, 224))
		, m_hFontDropDown(NULL)
		, m_nDropDownWidth(-1)
		, m_shieldIndex(-1)
	{
		//@	SetThemeClassList(L"Toolbar");
		HIMAGELIST himL;
		Shell_GetImageLists(&himL, &m_himSystem);
		int ix, iy;
		ImageList_GetIconSize(m_himSystem, &ix, &iy);
		m_sizeSystemIcon = CSize(ix, iy);
	}

	~CFlatButtonCtrl()
	{
	}

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

BEGIN_MSG_MAP(CFlatButtonCtrl)
	CHAIN_MSG_MAP(CThemeImpl<CFlatButtonCtrl>)
	CHAIN_MSG_MAP(CToolTipUsing<CFlatButtonCtrl>)
	ON_WM_CREATE()
	ON_WM_GETDLGCODE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SETFONT()
	ON_MESSAGE_VOID(WM_ENABLE, OnEnable)
	ON_MESSAGE_VOID(BM_CLICK, OnButtonClick)
	ON_WM_SIZE()
	ON_WM_PRINTCLIENT()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_DESTROY()
	ON_NOTIFY_CODE(TTN_NEEDTEXT, OnNotifyToolTipText)
	ON_MESSAGE(FBM_LOADIMAGELIST, OnLoadImageList)
	ON_MESSAGE_VOID(FBM_SETIMAGELIST, OnSetImageList)
	ON_MESSAGE_VOID(FBM_SETBKCOLOR, OnSetBkColor)
	ON_MESSAGE(FBM_SETSHIELD, OnSetShield)
	ON_MESSAGE_VOID(FBM_SETTIPTEXT, OnSetTipText)
END_MSG_MAP()


protected:
	void set_default_flatbuttonimage(int type);
	void DrawButtonImage(HDC hdc, int x, int y, int iType);
	void DrawButton(HDC hdc);
	void DrawThemeButton(HDC hdc);
	__inline void SendToParent() { GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED), (LPARAM)m_hWnd); }
	int GetDropDownWidth();
	void DrawDropDown(HDC hdc, const CRect& rrc);

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
	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnPrintClient(HDC hdc, LPARAM flags);
	void OnPaint();
	void OnLButtonDown(UINT flags, CPoint point);
	void OnMouseMove(UINT flags, CPoint point);
	void OnMouseLeave();
	bool OnTrackMouse(const CRect& rcr);
	void OnDestroy();
	void OnNotifyToolTipText(int code, NMHDR* phdr, LRESULT* pResult);
	void OnButtonClick(WPARAM, LPARAM);
	LRESULT OnLoadImageList(WPARAM wParam, LPARAM lParam);
	void OnSetImageList(WPARAM, LPARAM lParam);
	void OnSetBkColor(WPARAM, LPARAM lParam);
	void OnSetTipText(WPARAM, LPARAM lParam);
	LRESULT OnSetShield(WPARAM, LPARAM lParam);
};


///////////////////////////////////////////////////////
// FLATBUTTON control implementation

#define CHAR_DOWNARROW		(_T('6'))
#define CHAR_SPACING_GAP	(-2)

void CFlatButtonCtrl::set_default_flatbuttonimage(int type)
{
	UINT id = 0;
	switch (type) {
		case FBITYPE_FOLDER:	id = IDB_FBI_FOLDER; break;
		case FBITYPE_HELP:		id = IDB_FBI_HELP; break;
		case FBITYPE_COLOR:		id = IDB_FBI_COLOR; break;
		case FBITYPE_HINT:		id = IDB_FBI_HINT; break;
		case FBITYPE_BROWSE:	id = IDB_FBI_BROWSE; break;
	}
	if (id && IsWindow()) OnLoadImageList((WPARAM)RESINST, (LPARAM)id);
}

int CFlatButtonCtrl::OnCreate(LPCREATESTRUCT pcs)
{
	// Theme
	OpenThemeData(pcs->style & FBS_PUSHBUTTON ? L"Button" : L"Toolbar");

	set_default_flatbuttonimage(pcs->style & FBITYPE_MASK);
	return 0;
}

void CFlatButtonCtrl::OnSize(UINT nType, int cx, int cy)
{
	if (nType == SIZE_MINIMIZED) return;

	if (m_hbm) DeleteObject(m_hbm);
	CWindowDC dc(m_hWnd);
	m_hbm = CreateCompatibleBitmap(dc, cx, cy);
}

void CFlatButtonCtrl::OnPrintClient(HDC hdc, LPARAM flags)
{
	if (flags & PRF_ERASEBKGND)
		DefWindowProc(WM_ERASEBKGND, (WPARAM)hdc, 0);

	if (flags & (PRF_CHECKVISIBLE | PRF_CLIENT | PRF_OWNED))
		DrawButton(hdc);
}

void CFlatButtonCtrl::OnPaint()
{
	CPaintDC dc(m_hWnd);
	DrawButton(dc);
}

void CFlatButtonCtrl::DrawButtonImage(HDC hdc, int index, int x, int y)
{
	if (index == 2) {
		// disabled
		if (m_numImages < 3) {
			lccDrawEmbossedImage(hdc, m_hImage, 0, CPoint(x, y));
			return;
		}
	}
	else if (index == 1) {
		// hot
		if (m_numImages < 2)
			index = 0;
	}
	else
		index = 0;

	DrawAlphaImageList(m_hImage, index, hdc, x, y, m_bAlphaImage);
}

void CFlatButtonCtrl::DrawThemeButton(HDC hdc)
{
	CRect rcc;
	GetClientRect(&rcc);
	CRect rc = rcc;

	bool bEnabled = (IsWindowEnabled() != FALSE);
	DWORD style = GetStyle();

	COLORREF clrWindow = GetSysColor(COLOR_WINDOW);
	COLORREF clrHilight = GetSysColor(COLOR_HIGHLIGHT);
	COLORREF clr3dFace = GetSysColor(COLOR_3DFACE);
	COLORREF clr3dShadow = GetSysColor(COLOR_3DSHADOW);
	COLORREF clr3dHilight = GetSysColor(COLOR_3DHILIGHT);

	CDCHandle dc = hdc;
	CDC dcw;
	dcw.CreateCompatibleDC(dc);
	HBITMAP hbmo = dcw.SelectBitmap(m_hbm);

	// 背面を塗りつぶす
	HBRUSH hbr = (HBRUSH)GetParent().SendMessage(WM_CTLCOLORSTATIC, (WPARAM)dcw.m_hDC, (LPARAM)m_hWnd);
	if (hbr)
		dcw.FillRect(&rc, hbr);
	else {
		dcw.FillSolidRect(&rc, clr3dFace);
	}

	CPoint pt(0, 0);
	int iState;
	if (style & FBS_PUSHBUTTON) {
		// プッシュボタン
		if (!bEnabled)
			iState = PBS_DISABLED;
		else if (m_bClicked || m_bKeyPressed)
			iState = PBS_PRESSED;
		else
			iState = m_bIn ? PBS_HOT : (m_bFocus ? PBS_DEFAULTED : PBS_NORMAL);
		DrawThemeBackground(dcw, BP_PUSHBUTTON, iState, &rc, NULL);
	}
	else {
		if (style & FBS_FRAMEBORDER) {
			BOOL bActive = FALSE;
			int alpha = 255;
			if (bEnabled) {
				if (m_bFocus || m_bIn || m_bClicked || m_bKeyPressed) {
					bActive = TRUE;
					if (m_bClicked || m_bKeyPressed)
						alpha = 255;
					else if (m_bFocus)
						alpha = 160;
					else if (m_bIn)
						alpha = 96;
				}
			}
			else
				alpha = 96;
			lccDrawSelectionRoundFrame(dcw, &rc, bActive, alpha);
		}
		else {
			if (!bEnabled)
				iState = TS_DISABLED;
			else if (m_bClicked || m_bKeyPressed)
				iState = TS_PRESSED; //HOTCHECKED;
			else
				iState = (m_bFocus || m_bIn) ? TS_HOT : TS_NORMAL;
			DrawThemeBackground(dcw, TP_BUTTON, iState, &rc, NULL);
		}
	}

	int iImage = 0;
	if (!bEnabled)
		iImage = 2;
	else if (m_bIn || m_bKeyPressed || m_bFocus)
		iImage = 1;

	rc = rcc;
	rc.DeflateRect(1, 1);
	if (style & FBS_DROPDOWN) {
		rc.right -= GetDropDownWidth();
	}

	HFONT hf = NULL;
	if (m_hFont)
		hf = dcw.SelectFont(m_hFont);

	// ウィンドウテキスト
	CString str;
	if (!(style & FBS_NOTEXT))
		GetWindowText(str);

	// 行の長さを計算
	int hole_width = 0;							// 全体の長さ

	// シールドアイコンのアイコン幅を加える
	if (m_bShieldIcon) {
		hole_width += m_sizeSystemIcon.cx;
	}
	// イメージ幅を加える
	if (m_hImage) {
		if (hole_width)
			hole_width += 2;
		hole_width += m_cxImage;
	}
	// テキスト長
	int text_width = 0;
	CRect rcw(0, 0, 0, 0);
	if (!str.IsEmpty()) {
		dcw.DrawText(str, -1, &rcw, DT_CALCRECT | DT_SINGLELINE);
		text_width = rcw.Width();
		if (hole_width)
			hole_width += 2;
		hole_width += text_width;
	}

	rcw = rc;
	rcw.left += (rc.Width() - hole_width) / 2;
	rcw.right = rcw.left + hole_width;

	if (m_bShieldIcon) {
		// シールドアイコンを描画
		ImageList_Draw(m_himSystem, m_shieldIndex, dcw, pt.x + rcw.left, pt.y + rcw.top + (rcw.Height() - m_sizeSystemIcon.cy) / 2, ILD_TRANSPARENT);
		rcw.left += m_sizeSystemIcon.cx + 2;
	}

	if (m_hImage) {
		// イメージ位置を計算し描画
		DrawButtonImage(dcw, iImage, pt.x + rcw.left, pt.y + rcw.top + (rcw.Height() - m_cyImage) / 2);
		rcw.left += m_cxImage + 2;
	}

	if (!str.IsEmpty()) {
		// テキスト描画矩形の計算
		dcw.SetBkMode(TRANSPARENT);
		if (!bEnabled) {
			// Diasable状態の背景文字列
			rcw.OffsetRect(1, 1);
			dcw.SetTextColor(clr3dHilight);
			dcw.DrawText(str, -1, &rcw, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			rcw.OffsetRect(-1, -1);
			dcw.SetTextColor(clr3dShadow);
		}
		else
			dcw.SetTextColor(GetSysColor(COLOR_BTNTEXT));
		dcw.DrawText(str, -1, &rcw, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	}

	if (hf)
		dcw.SelectFont(hf);

	if (style & FBS_DROPDOWN) {
		rc.left = rc.right;
		rc.right = rcc.right - 1;
		DrawDropDown(dcw, rc);
	}

	dc.BitBlt(rcc.left, rcc.top, rcc.Width(), rcc.Height(), dcw, 0, 0, SRCCOPY);
	dcw.SelectBitmap(hbmo);
}

void CFlatButtonCtrl::DrawButton(HDC hdc)
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
		(m_bIn || m_bKeyPressed || m_bFocus))
		clr = m_clrFocus;

	clr = dcw.SetBkColor(clr);
	dcw.ExtTextOut(0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	dcw.SetBkColor(clr);

	// 枠描画
	CPoint pt(0, 0);
	if (m_bClicked || m_bKeyPressed) {
		draw_rect(dcw, rc, SEL_PUSH);
		pt.Offset(1, 1);
	}
	else {
		if (m_bFocus || m_bIn)									draw_rect(dcw, rc, SEL_POP);
		else if (style & (FBS_FRAMEBORDER | FBS_PUSHBUTTON))	draw_rect(dcw, rc, SEL_BORDER);
		else													draw_rect(dcw, rc, SEL_NORMAL);
	}

	BOOL bEnabled = IsWindowEnabled();
	int iImage = 0;
	if (!bEnabled)
		iImage = 2;
	else if (m_bIn || m_bKeyPressed || m_bFocus)
		iImage = 1;

	rc = rcc;
	rc.DeflateRect(1, 1);
	if (style & FBS_DROPDOWN) {
		rc.right -= GetDropDownWidth();
	}

	CString str;
	if (!(style & FBS_NOTEXT) && GetWindowText(str) > 0) {
		HFONT hf = NULL;
		if (m_hFont)
			hf = dcw.SelectFont(m_hFont);

		CRect rcw(0, 0, 0, 0);
		dcw.DrawText(str, -1, &rcw, DT_CALCRECT | DT_SINGLELINE);
		int text_width = rcw.Width() + 2;		// 行の長さを計算
		int hole_width = text_width;			// イメージを含めた全体の長さ

		if (m_hImage) {
			// イメージ幅を加える
			hole_width += m_cxImage + 2;
		}
		if (m_bShieldIcon) {
			// シールドアイコンのアイコン幅を加える
			hole_width += m_sizeSystemIcon.cx + 2;
		}

		rcw = rc;
		rcw.left += (rc.Width() - hole_width) / 2;
		rcw.right = rcw.left + hole_width;

		if (m_hImage) {
			DrawButtonImage(dcw, iImage, pt.x + rcw.left, pt.y + (rcw.top + rcw.bottom - m_cyImage) / 2);
			rcw.left += m_cxImage + 2;
		}
		if (m_bShieldIcon) {
			// シールドアイコンを描画
			ImageList_Draw(m_himSystem, m_shieldIndex, dcw, pt.x + rcw.left, pt.y + (rcw.top + rcw.bottom - m_sizeSystemIcon.cy) / 2, ILD_TRANSPARENT);
			rcw.left += m_sizeSystemIcon.cx + 2;
		}

		// テキスト描画矩形の計算
		if (m_bClicked || m_bKeyPressed)
			rcw.OffsetRect(1, 1);

		dcw.SetBkMode(TRANSPARENT);
		if (!bEnabled) {
			rcw.OffsetRect(1, 1);
			dcw.SetTextColor(GetSysColor(COLOR_BTNHIGHLIGHT));
			dcw.DrawText(str, -1, &rcw, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
			rcw.OffsetRect(-1, -1);
			dcw.SetTextColor(GetSysColor(COLOR_BTNSHADOW));
		}
		else
			dcw.SetTextColor(GetSysColor(COLOR_BTNTEXT));
		dcw.DrawText(str, -1, &rcw, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

		if (hf)
			dcw.SelectFont(hf);
	}
	else {
		if (m_hImage) {
			DrawButtonImage(dcw, iImage, pt.x + (rc.left + rc.right - m_cxImage) / 2, pt.y + (rc.top + rc.bottom - m_cyImage) / 2);
		}
	}

	if (style & FBS_DROPDOWN) {
		rc.left = rc.right;
		rc.right = rcc.right - 1;
		if (m_bClicked || m_bKeyPressed)
			rc.OffsetRect(1, 1);
		DrawDropDown(dcw, rc);
	}

	dc.BitBlt(rcc.left, rcc.top, rcc.Width(), rcc.Height(), dcw, 0, 0, SRCCOPY);
	dcw.SelectBitmap(hbmo);
}

void CFlatButtonCtrl::OnChangeFocus(bool bFocus)
{
	m_bFocus = bFocus;
	CClientDC dc(m_hWnd);
	DrawButton(dc);
}

void CFlatButtonCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar) {
	case VK_SPACE:
		if (!m_bKeyPressed) {
			CClientDC dc(m_hWnd);
			m_bKeyPressed = true;
			DrawButton(dc);
			if (GetStyle() & FBS_DROPDOWN) {
				SendToParent();
				m_bKeyPressed = false;
				DrawButton(dc);
			}
		}
		break;
	}
}

void CFlatButtonCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar) {
	case VK_SPACE:
		if (m_bKeyPressed) {
			m_bKeyPressed = false;
			CClientDC dc(m_hWnd);
			DrawButton(dc);
			SendToParent();
		}
		break;
	}
}

void CFlatButtonCtrl::OnLButtonDown(UINT nFlags, CPoint pt)
{
	if (m_bCapture || m_bKeyPressed) return;

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
		DrawButton(dc);
		SendToParent();
		m_bClicked = false;
		DrawButton(dc);
	}
	else if (OnTrackMouse(rc)) {
		SendToParent();
	}
}

bool CFlatButtonCtrl::OnTrackMouse(const CRect& rcr)
{
	SetCapture();
	m_bCapture = true;
	m_bClicked = true;

	CClientDC dc(m_hWnd);
	DrawButton(dc);

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
	DrawButton(dc);

	m_bCapture = false;
	ReleaseCapture();

	return m_bIn;
}

void CFlatButtonCtrl::OnMouseMove(UINT nFlags, CPoint pt)
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
			DrawButton(dc);
		}
	}
}

void CFlatButtonCtrl::OnMouseLeave()
{
	if (m_bTrackMouseEvent) {
		m_bIn = false;
		CClientDC dc(m_hWnd);
		DrawButton(dc);
	}
	m_bTrackMouseEvent = false;
}

void CFlatButtonCtrl::OnButtonClick(WPARAM, LPARAM)
{
	OnKeyDown(VK_SPACE, 0, 0);
	OnKeyUp(VK_SPACE, 0, 0);
}

void CFlatButtonCtrl::OnSetImageList(WPARAM, LPARAM lParam)
{
	HIMAGELIST him = (HIMAGELIST)lParam;
	if (m_hImage && !(GetStyle() & FBS_SHAREIMAGELIST))
		ImageList_Destroy(m_hImage);

	m_bAlphaImage = false;
	m_hImage = him;
	if (m_hImage) {
		m_numImages = ImageList_GetImageCount(m_hImage);
		ImageList_GetIconSize(m_hImage, &m_cxImage, &m_cyImage);

		IMAGEINFO info;
		if (ImageList_GetImageInfo(m_hImage, 0, &info)) {
			BITMAP bm;
			GetObject(info.hbmImage, sizeof(bm), &bm);
			m_bAlphaImage = (IsRGBA() && (bm.bmBitsPixel == 32));
		}
	}

	if (IsWindowVisible())
		Invalidate(TRUE);
}

LRESULT CFlatButtonCtrl::OnLoadImageList(WPARAM wParam, LPARAM lParam)
{
	HINSTANCE hInst = (HINSTANCE)wParam;
	LPCTSTR pszResource = (LPCTSTR)lParam;
	BITMAPINFOHEADER bh;
	if (!lccGetBitmapResourceInfo(hInst, pszResource, &bh)) return FALSE;

	if (m_hImage && !(GetStyle() & FBS_SHAREIMAGELIST))
		ImageList_Destroy(m_hImage);

	if (!IsRGBA() || bh.biBitCount != 32) {
		m_bAlphaImage = false;
		m_hImage = ImageList_LoadImage(hInst, pszResource,
							bh.biWidth / 3, 0, RGB(255, 0, 255),
							IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_DEFAULTCOLOR);
	}
	else {
		m_bAlphaImage = true;
		m_hImage = ImageList_LoadImage(hInst, pszResource,
							bh.biWidth, 0, RGB(255, 0, 255),
							IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_DEFAULTCOLOR);
	}
	if (m_hImage) {
		m_numImages = ImageList_GetImageCount(m_hImage);
		ImageList_GetIconSize(m_hImage, &m_cxImage, &m_cyImage);
	}
	if (IsWindowVisible())
		Invalidate(TRUE);
	return TRUE;
}

void CFlatButtonCtrl::OnSetBkColor(WPARAM, LPARAM lParam)
{
	COLORREF clr = (COLORREF)lParam;
	if (m_clrBack != clr) {
		m_clrBack = clr;
		if (IsWindowVisible()) Invalidate(TRUE);
	}
}

void CFlatButtonCtrl::OnSetTipText(WPARAM, LPARAM lParam)
{
	LPCTSTR psz = (LPCTSTR)lParam;
	if (!psz || *psz == 0) {
		m_tipMessage.Empty();
		if (TTIsEnabled()) {
			TTActivate(FALSE);
		}
	}
	else {
		m_tipMessage = psz;
		if (!TTIsEnabled()) {
			TTEnable(true);
			TTActivate(TRUE);
		}
	}
}

LRESULT CFlatButtonCtrl::OnSetShield(WPARAM, LPARAM lParam)
{
	if (!OsVISTA()) return 0;

	bool fShieldIconRequired = (lParam != 0);
	if (fShieldIconRequired != m_bShieldIcon) {
		m_bShieldIcon = fShieldIconRequired;
		if (m_bShieldIcon && m_shieldIndex < 0) {
			SHSTOCKICONINFO sii;
			memset(&sii, 0, sizeof(sii));
			sii.cbSize = sizeof(sii);
			if (SUCCEEDED(SHGetStockIconInfo(SIID_SHIELD, SHGSI_SYSICONINDEX, &sii)))
				m_shieldIndex = sii.iSysImageIndex;
		}
	}
	return 1;
}

void CFlatButtonCtrl::OnSetFont(HFONT hfont, BOOL bRedraw)
{
	m_hFont = hfont;
	if (bRedraw) Invalidate(TRUE);
}

void CFlatButtonCtrl::OnDestroy()
{
	if (m_hImage && !(GetStyle() & FBS_SHAREIMAGELIST))
		ImageList_Destroy(m_hImage);
	if (m_hbm)
		DeleteObject(m_hbm);
	if (m_hFontDropDown)
		DeleteObject(m_hFontDropDown);
}

void CFlatButtonCtrl::OnNotifyToolTipText(int id, NMHDR* phdr, LRESULT* pResult)
{
	if (TTIsEnabled()) {
		NMTTDISPINFO* ptt = (NMTTDISPINFO*)phdr;
		// ...
		if (!m_tipMessage.IsEmpty())
			ptt->lpszText = (LPTSTR)(LPCTSTR)m_tipMessage;
	}
}

int CFlatButtonCtrl::GetDropDownWidth()
{
	// return cached version if already determined...
	if (m_nDropDownWidth != -1) return m_nDropDownWidth;

	// otherwise calculate it...
	if (m_hFontDropDown == NULL) {
		CClientDC dc(nullptr);

//@		int check_height = GetSystemMetrics(SM_CYMENUCHECK);
		int check_height = 11;
		HFONT hFont = CreateFont(check_height, 0, 0, 0, FW_NORMAL, 0, 0, 0, SYMBOL_CHARSET, 0, 0, 0, 0, _T("Marlett"));
		HFONT hFontOld = nullptr;
		if (hFont != nullptr)
			hFontOld = dc.SelectFont(hFont);
		dc.GetCharWidth(CHAR_DOWNARROW, CHAR_DOWNARROW, &m_nDropDownWidth);
		if (hFont != nullptr) {
			dc.SelectFont(hFontOld);
			m_hFontDropDown = hFont;
		}
	}
	ATLASSERT(m_nDropDownWidth != -1);
	return m_nDropDownWidth;
}

void CFlatButtonCtrl::DrawDropDown(HDC hdc, const CRect& rrc)
{
	CDCHandle dc(hdc);

	int delta = GetDropDownWidth();
	if (delta > 0) {
		CRect rc = rrc;
		rc.left = rc.right - delta;
		HFONT hFontOld = dc.SelectFont(m_hFontDropDown);
		COLORREF clrt = dc.SetTextColor(RGB(0, 0, 0));
		int mode = dc.SetBkMode(TRANSPARENT);
		const TCHAR ch = CHAR_DOWNARROW;
		dc.DrawText(&ch, 1, &rc, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);
		dc.SetBkMode(mode);
		dc.SetTextColor(clrt);
		dc.SelectFont(hFontOld);
	}
}

////////////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK FlatButtonWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CFlatButtonCtrl* pWnd = new CFlatButtonCtrl;
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}

