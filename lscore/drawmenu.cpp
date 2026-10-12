//////////////////////////////////////////////////////////////////////
// drawmenu.cpp

#include "stdafx.h"
#include "sys\stat.h"
#include "lscore.h"
#include "common.h"
#include "fnc.h"
#include "findfile.h"
#include "resource.h"
#include "gdiplus_sup.h"

////////////////////////////////////////////////////////////////////////
// constants used for drawing

const int CX_TEXT_GAP = 4;					// num pixels between button and text
const int CX_TEXT_MARGIN = 2;				// num pixels after hilite to start text
const int CX_BUTTON_MARGIN = 2;				// num pixels wider button is than bitmap
const int CX_ICON_MARGIN_LEFT = 3;
const int CX_ICON_MARGIN_RIGHT = 5;
const DWORD MAGICROP = 0xb8074a;


////////////////////////////////////////////////////////////////////////
// DrawText flags

#define DT_MYSTANDARD		(DT_SINGLELINE | DT_LEFT | DT_VCENTER)


////////////////////////////////////////////////////////////////////////
//

COLORREF __fastcall calc_innercolor(COLORREF base)
{
/* @@
	WORD h, l, s;
	ColorRGBToHLS(base, &h, & l, &s);
	l = 192;
	s = 80;
	return ColorHLSToRGB(h, l, s);
@@ */
	return mix_color(GetSysColor(COLOR_WINDOW), base, 70);
}

static void draw_frame(HDC hdc, CRect& rc, COLORREF clrBorder, COLORREF clrPane)
{
	CDCHandle dc = hdc;
	dc.FillSolidRect(rc, clrPane);
	dc.Draw3dRect(rc, clrBorder, clrBorder);
}

/////////////////////////////////////////////////////////////////////////
//

void DrawCoolFrame(HDC hdc, CRect& rc)
{
	COLORREF clrBorder = GetSysColor(COLOR_HIGHLIGHT);
	COLORREF clrPane = calc_innercolor(clrBorder);
	draw_frame(hdc, rc, clrBorder, clrPane);
}

/////////////////////////////////////////////////////////////////////////
//

// ---------------------------
// 透過せずに描画する場合

#define xCLR_FRAMEOUTER		RGB(169, 200, 245)
#define xCLR_FRAMEINNER		RGB(240, 250, 253)
#define xCLR_PANETOP			RGB(225, 245, 251)
#define xCLR_PAINBOTTOM		RGB(190, 218, 251)

static void DrawVistaSelectionFrame(HDC hdc, RECT* prc)
{
	CDCHandle dc = hdc;
	CRect rc = *prc;
	dc.FillSolidRect(&rc, RGB(169, 200, 245));
	rc.InflateRect(-1, -1);
	dc.FillSolidRect(&rc, RGB(240, 250, 253));
	rc.InflateRect(-1, -1);

	int dy = rc.Height() / 4;
	CRect rcw = rc;
	rcw.bottom = rcw.top + dy;
	dc.FillSolidRect(&rcw, RGB(225, 245, 251));

	rcw = rc;
	rcw.top = rcw.bottom - dy;
	dc.FillSolidRect(&rcw, RGB(190, 218, 251));

	rc.top += dy;
	rc.bottom -= dy;

	FillShadeRect(dc, rc, RGB(225, 245, 251), RGB(190, 218, 251), TRUE);
}

// ---------------------------
// 透過して描画する場合

#define MKARGB(a,r,g,b)				((ARGB)(((DWORD)(BYTE)(b))|(((DWORD)(BYTE)(g))<<8)|(((DWORD)(BYTE)(r))<<16)|(((DWORD)(BYTE)(a))<<24)))
#define RGB2ARGB(a,clr)				(MKARGB((a),GetRValue((clr)),GetGValue((clr)),GetBValue((clr))))

#define RGB_ACTIVE_FRAMEOUTER		RGB(114, 165, 239)
#define RGB_ACTIVE_FRAMEINNER		RGB(230, 247, 252)
#define RGB_ACTIVE_PANETOP			RGB(206, 239, 248)
#define RGB_ACTIVE_PAINBOTTOM		RGB(148, 194, 248)

#define RGB_INACTIVE_FRAMEOUTER		RGB(185, 185, 185)
#define RGB_INACTIVE_FRAMEINNER		RGB(248, 248, 248)
#define RGB_INACTIVE_PANETOP		RGB(245, 245, 245)
#define RGB_INACTIVE_PAINBOTTOM		RGB(217, 217, 217)

static COLORREF ClrActive[4] = {
	RGB_ACTIVE_FRAMEOUTER,
	RGB_ACTIVE_FRAMEINNER,
	RGB_ACTIVE_PANETOP,
	RGB_ACTIVE_PAINBOTTOM,
};

static COLORREF ClrInactive[4] = {
	RGB_INACTIVE_FRAMEOUTER,
	RGB_INACTIVE_FRAMEINNER,
	RGB_INACTIVE_PANETOP,
	RGB_INACTIVE_PAINBOTTOM,
};

extern "C"
void WINAPI lccDrawSelectionFrameEx(HDC hdc, RECT* prc, COLORREF* pclrTable, int alpha)
{
	Graphics g(hdc);

	Color clrTable[4];
	for (int i = 0; i < 4; i++) {
		COLORREF clr = pclrTable[i];
		clrTable[i].SetValue(MKARGB(alpha, GetRValue(clr), GetGValue(clr), GetBValue(clr)));
	}

	// 枠描画
	CRect rc = *prc;
	Pen pen(clrTable[0], 1);
	pen.SetAlignment(PenAlignmentInset);
	g.DrawRectangle(&pen, rc.left, rc.top, rc.Width() - 1, rc.Height() - 1);

	rc.InflateRect(-1, -1);
	pen.SetColor(clrTable[1]);
	g.DrawRectangle(&pen, rc.left, rc.top, rc.Width() - 1, rc.Height() - 1);

	// 塗り潰し
	rc.InflateRect(-1, -1);
	LinearGradientBrush lbr(Rect(rc.left, rc.top - 1, rc.Width(), rc.Height() + 1), clrTable[2], clrTable[3], LinearGradientModeVertical);

	REAL fact[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	REAL pos[4] = { 0.0f, 0.25f, 0.5f, 0.75f };
	lbr.SetBlend(fact, pos, 4);
	g.FillRectangle(&lbr, rc.left, rc.top, rc.Width(), rc.Height());
}

extern "C"
void WINAPI lccDrawSelectionFrame(HDC hdc, RECT* prc, BOOL bActive, int alpha)
{
	lccDrawSelectionFrameEx(hdc, prc, bActive ? ClrActive : ClrInactive, alpha < 0 ? 192 : alpha);
}

/////////////////////////////////////////////////////////////////////////
//

extern "C"
void WINAPI lccDrawSelectionRoundFrameEx(HDC hdc, RECT* prc, COLORREF* pclrTable, int alpha)
{
	Graphics g(hdc);

	Color clrTable[4];
	for (int i = 0; i < 4; i++) {
		COLORREF clr = pclrTable[i];
		clrTable[i].SetValue(MKARGB(alpha, GetRValue(clr), GetGValue(clr), GetBValue(clr)));
	}

	CRect rc = *prc;

	// 中枠描画
	rc.InflateRect(-1, -1);
	Pen pen(clrTable[1], 1);
	pen.SetAlignment(PenAlignmentInset);
	g.DrawRectangle(&pen, rc.left, rc.top, rc.Width() - 1, rc.Height() - 1);

	// 塗り潰し
	rc.InflateRect(-1, -1);
	LinearGradientBrush lbr(Rect(rc.left, rc.top - 1, rc.Width(), rc.Height() + 1), clrTable[2], clrTable[3], LinearGradientModeVertical);

	REAL fact[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	REAL pos[4] = { 0.0f, 0.25f, 0.5f, 0.75f };
	lbr.SetBlend(fact, pos, 4);
	g.FillRectangle(&lbr, rc.left, rc.top, rc.Width(), rc.Height());

	// 枠描画
	CDCHandle dc = hdc;
	rc = *prc;
	HPEN hpen = dc.SelectPen(CreatePen(PS_SOLID, 0, pclrTable[0]));
	HBRUSH hbr = dc.SelectBrush((HBRUSH)GetStockObject(NULL_BRUSH));
	dc.RoundRect(&rc, CPoint(5, 5));
	dc.SelectBrush(hbr);
	DeleteObject(dc.SelectPen(hpen));
}

extern "C"
void WINAPI lccDrawSelectionRoundFrame(HDC hdc, RECT* prc, BOOL bActive, int alpha)
{
	lccDrawSelectionRoundFrameEx(hdc, prc, bActive ? ClrActive : ClrInactive, alpha < 0 ? 192 : alpha);
}

/////////////////////////////////////////////////////////////////////////
//

/* @@

extern "C"
DWORD WINAPI lccGetMenuColor(UINT index)
{
	switch (index) {
	case MENUCOLOR_BACKGROUND:				return RGB(0xf0, 0xf0, 0xf0);
	case MENUCOLOR_SEPARATOR_SHADOW:		return RGB(0xe0, 0xe0, 0xe0);
	case MENUCOLOR_SEPARATOR_HILIGHT:		return RGB(0xff, 0xff, 0xff);
	case MENUCOLOR_TEXT:					return RGB(0x00, 0x00, 0x00);
	case MENUCOLOR_HIGHLIGHTTEXT:			return RGB(0x00, 0x00, 0x00);
	case MENUCOLOR_GRAYEDTEXT:				return RGB(0x80, 0x80, 0x80);
	default: break;
	}
	return RGB(0x00, 0x00, 0x00);
}

@@ */

/////////////////////////////////////////////////////////////////////////
//

static HBITMAP makecheckmark(HDC hdc, CRect& rc, BOOL bRadio)
{
	HBITMAP hbm = CreateBitmap(rc.Width(), rc.Height(), 1, 1, NULL);
	CDC dcw;
	dcw.CreateCompatibleDC(hdc);
	hbm = dcw.SelectBitmap(hbm);
	dcw.PatBlt(0, 0, rc.Width(), rc.Height(), WHITENESS);
	dcw.DrawFrameControl(&rc, DFC_MENU, bRadio ? DFCS_MENUBULLET : DFCS_MENUCHECK);
	return dcw.SelectBitmap(hbm);
}

////////////////////////////////////////////////////////////////////////
// Draw 3D checkmark
//
//		dc				device context to draw in
//		rc				rectangle to center bitmap in
//		bSelected		TRUE if button is also selected
//		hbmCheck		Checkmark bitmap to use, or NULL for default
//

#ifndef OBM_CHECK
#define OBM_CHECK 32760 // from winuser.h
#endif

static BOOL draw_check_3d(HDC hdc, CRect& rc, BOOL bSelected, HBITMAP hbmCheck)
{
	CDCHandle dc = hdc;

	// get checkmark bitmap if none, use Windows standard
	if (!hbmCheck) hbmCheck = LoadBitmap(NULL, _MKIR(OBM_CHECK));

	// center bitmap in caller's rectangle
	BITMAP bm;
	GetObject(hbmCheck, sizeof(bm), &bm);
	int cx = bm.bmWidth;
	int cy = bm.bmHeight;
	CRect rcDest = rc;
	CPoint p(0, 0);
	CSize delta(CPoint((rc.Width() - cx) / 2, (rc.Height() - cy) / 2));
	if (rc.Width() > cx)
		rcDest = CRect(rc.TopLeft() + delta, CSize(cx, cy));
	else
		p -= delta;

	// select checkmark into memory DC
	CDC dcw;
	dcw.CreateCompatibleDC(dc);
	HBITMAP hOldBM = dcw.SelectBitmap(hbmCheck);

	// set BG color based on selected state
	COLORREF colorOld = dc.SetBkColor(GetSysColor(bSelected ? COLOR_MENU : COLOR_3DLIGHT));
	dc.BitBlt(rcDest.left, rcDest.top, rcDest.Width(), rcDest.Height(), dcw, p.x, p.y, SRCCOPY);
	dc.SetBkColor(colorOld);

	dcw.SelectBitmap(hOldBM);			// restore

	// draw pushed-in hilight.
	if (rc.Width() > cx)				// if room:
		rcDest.InflateRect(1, 1);		// inflate checkmark by one pixel all around
	dc.DrawEdge(&rcDest, BDR_SUNKENOUTER, BF_RECT);

	return TRUE;
}

static BOOL draw_check_xp(HDC hdc, CRect& rc, BOOL bRadio, HBITMAP hbmCheck)
{
	if (IsComm6()) {
		int cx, cy;
		HIMAGELIST him = GetCommonImageList(IMAGERESOURCE_CHECKMARK);
		ImageList_GetIconSize(him, &cx, &cy);
		ImageList_Draw(him, 0, hdc,
				(rc.left + rc.right - cx) / 2,
				(rc.top + rc.bottom - cy) / 2, ILD_TRANSPARENT);
		return TRUE;
	}

	CDCHandle dc = hdc;
	// get checkmark bitmap if none, use Windows standard
	bool makebmp = false;
	if (!hbmCheck) {
		hbmCheck = makecheckmark(hdc, rc, bRadio);
		makebmp = true;
	}

	// center bitmap in caller's rectangle
	BITMAP bm;
	GetObject(hbmCheck, sizeof(bm), &bm);
	int cx = bm.bmWidth;
	int cy = bm.bmHeight;
	CRect rcd = rc;
	CPoint p(0, 0);
	CSize delta(CPoint((rc.Width() - cx) / 2, (rc.Height() - cy) / 2));
	if (rc.Width() > cx)
		rcd = CRect(rc.TopLeft() + delta, CSize(cx, cy));
	else
		p -= delta;

	// select checkmark into memory DC
	CDC dcw;
	dcw.CreateCompatibleDC(dc);
	HBITMAP hOldBM = dcw.SelectBitmap(hbmCheck);

	// set BG color based on selected state
	COLORREF colorOld = dc.SetBkColor(RGB(255, 255, 255));
	dc.BitBlt(rcd.left, rcd.top, rcd.Width(), rcd.Height(), dcw, p.x, p.y, SRCAND);
	dc.SetBkColor(RGB(0, 0, 0));
	dc.BitBlt(rcd.left, rcd.top, rcd.Width(), rcd.Height(), dcw, p.x, p.y, SRCINVERT);
	dcw.SelectBitmap(hOldBM);			// restore
	dc.SetBkColor(colorOld);

	if (makebmp) DeleteObject(hbmCheck);

	return TRUE;
}

static BOOL draw_check(HDC hdc, CRect& rc, BOOL bSelected, HBITMAP hbmCheck)
{
	CDCHandle dc = hdc;
	// get checkmark bitmap if none, use Windows standard
	if (!hbmCheck) hbmCheck = LoadBitmap(NULL, _MKIR(OBM_CHECK));

	// center bitmap in caller's rectangle
	BITMAP bm;
	GetObject(hbmCheck, sizeof(bm), &bm);
	int cx = bm.bmWidth;
	int cy = bm.bmHeight;
	CRect rcd = rc;
	CPoint p(0, 0);
	CSize delta(CPoint((rc.Width() - cx) / 2, (rc.Height() - cy) / 2));
	if (rc.Width() > cx)
		rcd = CRect(rc.TopLeft() + delta, CSize(cx, cy));
	else
		p -= delta;

	// select checkmark into memory DC
	CDC dcw;
	dcw.CreateCompatibleDC(dc);
	HBITMAP hOldBM = dcw.SelectBitmap(hbmCheck);

	// set BG color based on selected state
	COLORREF colFore = dc.GetTextColor();
	COLORREF colBack = dc.SetBkColor(RGB(255, 255, 255));
	if (bSelected)
		dc.SetTextColor(RGB(0, 0, 0));
	dc.BitBlt(rcd.left, rcd.top, rcd.Width(), rcd.Height(), dcw, p.x, p.y, SRCAND);
	if (bSelected)
		dc.SetTextColor(RGB(255, 255, 255));
	dc.SetBkColor(RGB(0, 0, 0));
	dc.BitBlt(rcd.left, rcd.top, rcd.Width(), rcd.Height(), dcw, p.x, p.y, SRCINVERT);

	dcw.SelectBitmap(hOldBM);			// restore
	dc.SetBkColor(colBack);
	dc.SetTextColor(colFore);

	return TRUE;
}

/* @@@@ もう使わない？
void DitherBlt(HDC hdcDest, int nXDest, int nYDest, int nWidth, int nHeight, HBITMAP hbm, int nXSrc, int nYSrc)
{
	// Create a generic DC for all BitBlts
	HDC hDC = CreateCompatibleDC(hdcDest);
	if (hDC)
	{
		// Create a DC for the monochrome DIB section
		HDC bwDC = CreateCompatibleDC(hDC);
		if (bwDC)
		{
			// Create the monochrome DIB section with a black and white palette
			struct {
			   BITMAPINFOHEADER bmiHeader; 
			   RGBQUAD          bmiColors[2]; 
			} RGBBWBITMAPINFO = {
				
				{	// a BITMAPINFOHEADER
					sizeof(BITMAPINFOHEADER),	// biSize 
					nWidth,		// biWidth; 
					nHeight,	// biHeight; 
					1,			// biPlanes; 
					1,			// biBitCount 
					BI_RGB,		// biCompression; 
					0,			// biSizeImage; 
					0,			// biXPelsPerMeter; 
					0,			// biYPelsPerMeter; 
					0,			// biClrUsed; 
					0			// biClrImportant; 
				},

				{
					{ 0x00, 0x00, 0x00, 0x00 }, { 0xFF, 0xFF, 0xFF, 0x00 }
				} 
			};
			VOID *pbitsBW;
			HBITMAP hbmBW = CreateDIBSection(bwDC, (LPBITMAPINFO)&RGBBWBITMAPINFO, DIB_RGB_COLORS, &pbitsBW, NULL, 0);
			
			if (hbmBW)
			{
				// Attach the monochrome DIB section and the bitmap to the DCs
				SelectObject(bwDC, hbmBW);
				SelectObject(hDC, hbm);

				// BitBlt the bitmap into the monochrome DIB section
				BitBlt(bwDC, 0, 0, nWidth, nHeight, hDC, nXSrc, nYSrc, SRCCOPY);

				// Paint the destination rectangle in gray
				FillRect(hdcDest, CRect(nXDest, nYDest, nXDest + nWidth, nYDest + nHeight), GetSysColorBrush(COLOR_3DFACE));
				
				// BitBlt the black bits in the monochrome bitmap into COLOR_3DHILIGHT bits in the destination DC
				// The magic ROP comes from the Charles Petzold's book
				HBRUSH hb = CreateSolidBrush(GetSysColor(COLOR_3DHILIGHT));
				HBRUSH oldBrush = (HBRUSH)SelectObject(hdcDest, hb);
				BitBlt(hdcDest, nXDest + 1, nYDest + 1, nWidth, nHeight, bwDC, 0, 0, 0xB8074A);

				// BitBlt the black bits in the monochrome bitmap into COLOR_3DSHADOW bits in the destination DC
				hb = CreateSolidBrush(GetSysColor(COLOR_3DSHADOW));
				DeleteObject(SelectObject(hdcDest, hb));
				BitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, bwDC, 0, 0, 0xB8074A);
				DeleteObject(SelectObject(hdcDest, oldBrush));
			}
			DeleteDC(bwDC);
		}
		DeleteDC(hDC);
	}
}
@@@@ */

static void DrawEmbossedImage(HDC hdc, HIMAGELIST hImage, int iImage, POINT pt)
{
	IMAGEINFO info;
	if (!ImageList_GetImageInfo(hImage, iImage, &info))
		return;

	BITMAP bm;
	GetObject(info.hbmImage, sizeof(bm), &bm);
	bool bAlphaImage = (IsRGBA() && (bm.bmBitsPixel == 32));

	if (bAlphaImage) {
		IMAGELISTDRAWPARAMS ildp;
		memset(&ildp, 0, sizeof(ildp));
		ildp.cbSize = sizeof(IMAGELISTDRAWPARAMS);
		ildp.himl = hImage;
		ildp.i = iImage;
		ildp.hdcDst = hdc;
		ildp.x = pt.x;
		ildp.y = pt.y;
		ildp.cx = 0;
		ildp.cy = 0;
		ildp.xBitmap = 0;
		ildp.yBitmap = 0;
		ildp.fStyle = ILD_TRANSPARENT;
		ildp.fState = ILS_SATURATE;
		ildp.Frame = 0;
		::ImageList_DrawIndirect(&ildp);
	}
	else {
		// カラービットマップの作成
		int cx = info.rcImage.right - info.rcImage.left;
		int cy = info.rcImage.bottom - info.rcImage.top;
		HBITMAP hbm = CreateBitmap(cx, cy, 1, 1, NULL);

		// メモリDCへ描画 -- 背景は先に白で埋める
		HDC hmemdc = CreateCompatibleDC(hdc);
		HGDIOBJ hbmOld = SelectObject(hmemdc, hbm);
		PatBlt(hmemdc, 0, 0, cx, cy, WHITENESS);
		ImageList_Draw(hImage, iImage, hmemdc, 0, 0, ILD_TRANSPARENT);

		// これを先にやんなきゃダメ。 でもなぜ？
		COLORREF rgb = SetBkColor(hdc, RGB(255, 255, 255));

		// Draw using hilite offset by (1,1), then shadow
		HBRUSH hbrShadow = CreateSolidBrush(GetSysColor(COLOR_3DSHADOW));
		HBRUSH hbrHilite = CreateSolidBrush(GetSysColor(COLOR_3DHIGHLIGHT));
		HGDIOBJ hbrOld = SelectObject(hdc, hbrHilite);
		BitBlt(hdc, pt.x + 1, pt.y + 1, cx, cy, hmemdc, 0, 0, MAGICROP);
		SelectObject(hdc, hbrShadow);
		BitBlt(hdc, pt.x, pt.y, cx, cy, hmemdc, 0, 0, MAGICROP);
		SelectObject(hdc, hbrOld);

		DeleteObject(hbrShadow);
		DeleteObject(hbrHilite);

		SetBkColor(hdc, rgb);					// restore
		SelectObject(hmemdc, hbmOld);			// ...
		DeleteObject(hbm);
		DeleteDC(hmemdc);
	}
}

extern "C"
void WINAPI lccDrawEmbossedImage(HDC hdc, HIMAGELIST hImage, int iImage, POINT pt)
{
	DrawEmbossedImage(hdc, hImage, iImage, pt);
}

static void draw_menutext(HDC hdc, CRect& rc, LPCTSTR pszText, COLORREF clrText, UINT txflg)
{
	if (!pszText || *pszText == 0) return;

	TCHAR buf[MAX_PATH];
	lstrcpy(buf, pszText);

	TCHAR* p = StrChr(buf, '\t');
	if (p)
		*p++ = '\0';

	COLORREF clr = SetTextColor(hdc, clrText);
	DrawText(hdc, buf, -1, rc, DT_MYSTANDARD | txflg);
	if (p) {
		CRect rcr = rc;
		rcr.right -= GetSystemMetrics(SM_CXMENUCHECK);
		DrawText(hdc, p, -1, rcr, DT_MYSTANDARD | DT_RIGHT | txflg);
	}
	SetTextColor(hdc, clr);
}

static void _drawmenu(DRAWITEMSTRUCT* pdi, LPCTSTR pText, HIMAGELIST hImage, int iImage, CSize sizeButton, UINT txflg, BOOL bAlphaImage)
{
	CDCHandle dc = pdi->hDC;

	CRect rcItem(&pdi->rcItem);
	bool bGrayed = ((pdi->itemState & (ODS_GRAYED | ODS_DISABLED)) != 0);
	bool bSelect = ((pdi->itemState & ODS_SELECTED) != 0);
	bool bChecked  = ((pdi->itemState & ODS_CHECKED) != 0);
	BOOL bHaveBtn = FALSE;

	COLORREF clrMenu = GetSysColor(COLOR_MENU);
	COLORREF clrText = GetSysColor(bGrayed ? COLOR_GRAYTEXT : COLOR_MENUTEXT);
	COLORREF clr3DLight = GetSysColor(COLOR_3DLIGHT);
	COLORREF clr3DHilight = GetSysColor(COLOR_3DHILIGHT);
	COLORREF clr3DShadow = GetSysColor(COLOR_3DSHADOW);
	COLORREF clrHilite = GetSysColor(COLOR_HIGHLIGHT);
	COLORREF clrHiliteText = GetSysColor(COLOR_HIGHLIGHTTEXT);

	dc.FillSolidRect(&rcItem, bSelect ? clrHilite : clrMenu);

	CRect rcBtn = rcItem;
	rcBtn.left += CX_BUTTON_MARGIN;
	rcBtn.right = rcBtn.left + sizeButton.cx;
	rcBtn.top += (rcBtn.Height() - sizeButton.cy) / 2;
	rcBtn.bottom = rcBtn.top + sizeButton.cy;
	if (hImage && iImage >= 0) {
		bHaveBtn = TRUE;

		CRect rc = rcItem;
		rc.right = rc.left + sizeButton.cx + CX_BUTTON_MARGIN * 2 + CX_TEXT_GAP;

		CRect rcb = rc;
		rcb.right = rcb.left + sizeButton.cx + CX_BUTTON_MARGIN * 2;
		if (bChecked) {
			dc.FillSolidRect(rcb, bSelect ? clrMenu : clr3DLight);
			dc.DrawEdge(rcb, BDR_SUNKENOUTER, BF_RECT);
		}

		if (!bGrayed)
			ImageList_Draw(hImage, iImage, dc, rcBtn.left, rcBtn.top, ILD_TRANSPARENT);
		else
			DrawEmbossedImage(dc, hImage, iImage, CPoint(rcBtn.TopLeft()));
	}
	else {
		CMenuItemInfo info;
		info.fMask = MIIM_CHECKMARKS;
		GetMenuItemInfo((HMENU)pdi->hwndItem, pdi->itemID, FALSE, &info);
		if (bChecked || info.hbmpUnchecked) {
			bHaveBtn = draw_check_3d(dc, rcBtn, bSelect, bChecked ? info.hbmpChecked : info.hbmpUnchecked);
		}
	}

	CRect rcBack = rcItem;
	if (bHaveBtn)
		rcBack.left += sizeButton.cx + CX_BUTTON_MARGIN * 2 + CX_TEXT_GAP;

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	CRect rcText = rcItem;
	if (bHaveBtn)
		rcText.left += sizeButton.cx + CX_BUTTON_MARGIN * 2 + CX_TEXT_GAP;

	dc.SetBkMode(TRANSPARENT);
	rcText.left = rcItem.left + sizeButton.cx + CX_BUTTON_MARGIN * 2 + CX_TEXT_GAP + CX_TEXT_MARGIN;
//@	rcText.right -= GetSystemMetrics(SM_CXMENUCHECK);

	if (pdi->itemID == 0) {			// セパレータ
		CRect rc = rcItem;
		rc.top += rcText.Height() / 2 - 1;
		rc.bottom = rc.top + 1;
		dc.FillSolidRect(&rc, clr3DShadow);
		if (!IsThemed()) {
			rc.OffsetRect(0, 1);
			dc.FillSolidRect(&rc, clr3DHilight);
		}
	}
	else {
		if (bGrayed && !bSelect) {
			CRect rcw = rcText + CPoint(1, 1);
			draw_menutext(dc, rcw, pText, clr3DHilight, txflg);
		}
		draw_menutext(dc, rcText, pText, bSelect ? clrHiliteText : clrText, txflg);
	}
}

static void _drawmenu_xp(DRAWITEMSTRUCT* pdi, LPCTSTR pText, HIMAGELIST hImage, int iImage, CSize sizeButton, UINT txflg, BOOL bAlphaImage)
{
	CDCHandle dc = pdi->hDC;

	CRect rcItem(pdi->rcItem);

	bool bGrayed = ((pdi->itemState & (ODS_GRAYED | ODS_DISABLED)) != 0);
	bool bSelected = ((pdi->itemState & ODS_SELECTED) != 0);
	bool bChecked  = ((pdi->itemState & ODS_CHECKED) != 0);

	COLORREF clrMenu = GetSysColor(COLOR_MENU);
	COLORREF clrShadow = GetSysColor(COLOR_3DSHADOW);
	COLORREF clrHilite = GetSysColor(COLOR_3DHILIGHT);
	COLORREF clrText = GetSysColor(bGrayed ? COLOR_GRAYTEXT : COLOR_MENUTEXT);
	COLORREF clrBorder = GetSysColor(bGrayed ? COLOR_3DSHADOW : COLOR_HIGHLIGHT);

	// 背景
	CRect rc;
	COLORREF clrPane = calc_innercolor(clrBorder);
	if (bSelected) {
		draw_frame(dc, rcItem, clrBorder, clrPane);
	}
	else {
		dc.FillSolidRect(&rcItem, clrMenu);
		rc = rcItem;
		rc.right = rc.left + sizeButton.cx + CX_BUTTON_MARGIN * 2;
		if (IsWindowThemed())
			dc.FillSolidRect(&rc, mix_color(clrMenu, clrShadow, 80));
		else
			FillShadeRect(dc, rc, clrMenu, mix_color(clrMenu, clrShadow, 50), FALSE);
	}

	dc.SetBkMode(TRANSPARENT);
	CRect rcText = rcItem;
	rcText.left += sizeButton.cx + CX_BUTTON_MARGIN * 2 + CX_TEXT_GAP + CX_TEXT_MARGIN;

	if (pdi->itemID == 0) {
		// メニューセパレータ
		rc = rcText;
		rc.top += rcText.Height() / 2 - 1;
		rc.bottom = rc.top + 1;

		if (IsThemed()) {
			COLORREF cra[5];
			cra[0] = cra[4] = clrMenu;
			cra[1] = cra[2] = cra[3] = clrShadow;
			FillShadeRectEx(dc, rc, cra, 5, FALSE);
		}
		else {
			dc.FillSolidRect(&rc, clrShadow);
			rc.OffsetRect(0, 1);
			dc.FillSolidRect(&rc, clrHilite);
		}
	}
	else {
		// メニュー文字列
		if (bGrayed && !bSelected) {
			CRect rcw = rcText + CPoint(1, 1);
			draw_menutext(dc, rcw, pText, clrHilite, txflg);
		}
		draw_menutext(dc, rcText, pText, clrText, txflg);
	}

	// ボタン描画
	CRect rcBtn = rcItem;
	rcBtn.left += CX_BUTTON_MARGIN;
	rcBtn.right = rcBtn.left + sizeButton.cx;
	rcBtn.top += (rcBtn.Height() - sizeButton.cy) / 2;
	rcBtn.bottom = rcBtn.top + sizeButton.cy;
	if (hImage && iImage >= 0) {
		if (!bGrayed) {
			// ボタン枠
			if (bChecked && !bSelected) {
				CRect rc = rcItem;
				rc.right = rc.left + sizeButton.cx + CX_BUTTON_MARGIN * 2;
				draw_frame(dc, rc, clrBorder, clrPane);
			}
			// ボタン
			ImageList_Draw(hImage, iImage, dc, rcBtn.left, rcBtn.top, ILD_TRANSPARENT);
		}
		else { // use DrawEmbossed to draw disabeld button, w/color flag
			DrawEmbossedImage(dc, hImage, iImage, CPoint(rcBtn.TopLeft()));
		}
	} else {
		// no button: look for custom checked/unchecked bitmaps
		CMenuItemInfo info;
		info.fMask = MIIM_CHECKMARKS | MIIM_TYPE;
		::GetMenuItemInfo((HMENU)pdi->hwndItem, pdi->itemID, MF_BYCOMMAND, &info);
		if (bChecked || info.hbmpUnchecked != NULL) {
			// draw button hole
			if (!bSelected && !IsComm6())
				draw_frame(dc, rcBtn, clrBorder, clrPane);
			BOOL bRadio = ((info.fType & MFT_RADIOCHECK) != 0);
			draw_check_xp(dc, rcBtn, bRadio, bChecked ? info.hbmpChecked : info.hbmpUnchecked);
		}
	}
}

static void _drawmenu_win(DRAWITEMSTRUCT* pdi, LPCTSTR pText, HIMAGELIST hImage, int iImage, CSize sizeButton, UINT txflg, BOOL bAlphaImage)
{
	CDCHandle dc = pdi->hDC;

	CRect rcItem(&pdi->rcItem);
	bool bGrayed = ((pdi->itemState & (ODS_GRAYED | ODS_DISABLED)) != 0);
	bool bSelected = ((pdi->itemState & ODS_SELECTED) != 0);
	bool bChecked  = ((pdi->itemState & ODS_CHECKED) != 0);

	COLORREF clrMenu = GetSysColor(COLOR_MENU);
	COLORREF clrText = GetSysColor(bGrayed ? COLOR_GRAYTEXT : COLOR_MENUTEXT);
	COLORREF clrHilite = GetSysColor(COLOR_HIGHLIGHT);
	COLORREF clrHiliteText = GetSysColor(COLOR_HIGHLIGHTTEXT);

	dc.FillSolidRect(&rcItem, bSelected ? clrHilite : clrMenu);

	dc.SetBkMode(TRANSPARENT);
	CRect rcText = rcItem;
	rcText.left += sizeButton.cx + CX_BUTTON_MARGIN * 2 + CX_TEXT_GAP + CX_TEXT_MARGIN;
//@	rcText.right -= GetSystemMetrics(SM_CXMENUCHECK);

	if (pdi->itemID == 0) {		// セパレータ
		CRect rc = rcText;
		rc.top += rcText.Height() / 2 - 1;
		rc.bottom = rc.top + 1;
		dc.FillSolidRect(&rc, GetSysColor(COLOR_3DSHADOW));
		if (!IsThemed()) {
			rc.OffsetRect(0, 1);
			dc.FillSolidRect(&rc, GetSysColor(COLOR_3DHILIGHT));
		}
	} else {					// メニュー文字列
		if (bGrayed) {
			if (!bSelected) {
				CRect rcw = rcText + CPoint(1, 1);
				draw_menutext(dc, rcw, pText, clrHiliteText, txflg);
			}
		}
		draw_menutext(dc, rcText, pText, bSelected ? clrHiliteText : clrText, txflg);
	}

	CRect rcBtn = rcItem;
	rcBtn.left += CX_BUTTON_MARGIN;
	rcBtn.right = rcBtn.left + sizeButton.cx;
	rcBtn.top += (rcBtn.Height() - sizeButton.cy) / 2;
	rcBtn.bottom = rcBtn.top + sizeButton.cy;
	if (hImage && iImage >= 0) {
		if (!bGrayed)
			ImageList_Draw(hImage, iImage, dc, rcBtn.left, rcBtn.top, ILD_TRANSPARENT);
		else
			DrawEmbossedImage(dc, hImage, iImage, CPoint(rcBtn.TopLeft()));
	} else {
		// no button: look for custom checked/unchecked bitmaps
		CMenuItemInfo info;
		info.fMask = MIIM_CHECKMARKS;
		::GetMenuItemInfo((HMENU)pdi->hwndItem, pdi->itemID, FALSE, &info);
		if (bChecked || info.hbmpUnchecked) {
			draw_check(dc, rcBtn, bSelected, bChecked ? info.hbmpChecked : info.hbmpUnchecked);
		}
	}
}

extern "C"
void WINAPI lccDrawButtonMenuEx(int menuStyle, DRAWITEMSTRUCT* pdi, LPCTSTR pText, HIMAGELIST hImage, int iImage, SIZE sizeButton, UINT txflg, BOOL bAlphaImage)
{
	// フリッカーフリー描画
	DRAWITEMSTRUCT di;
	memcpy(&di, pdi, sizeof(di));
	OffsetRect(&di.rcItem, -pdi->rcItem.left, -pdi->rcItem.top);

	CRect rc(pdi->rcItem);
	di.hDC = CreateCompatibleDC(pdi->hDC);
	HBITMAP hbm = CreateCompatibleBitmap(pdi->hDC, rc.Width(), rc.Height());
	hbm = (HBITMAP)SelectObject(di.hDC, hbm);
	HFONT hfont = (HFONT)SelectObject(pdi->hDC, GetStockFont(DEFAULT_GUI_FONT));
	hfont = (HFONT)SelectObject(di.hDC, hfont);
	BitBlt(di.hDC, 0, 0, rc.Width(), rc.Height(), pdi->hDC, pdi->rcItem.left, pdi->rcItem.top, SRCCOPY);

	CSize sizeImage(0, 0);
	if (hImage) {
		int cx, cy;
		if (ImageList_GetIconSize(hImage, &cx, &cy)) {
			sizeImage.cx = cx;
			sizeImage.cy = cy;
		}
	}

	if (menuStyle == bmenuDefault)
		_drawmenu(&di, pText, hImage, iImage, sizeImage, txflg, bAlphaImage);
	else if (menuStyle == bmenuXP)
		_drawmenu_xp(&di, pText, hImage, iImage, sizeImage, txflg, bAlphaImage);
	else
		_drawmenu_win(&di, pText, hImage, iImage, sizeImage, txflg, bAlphaImage);

	BitBlt(pdi->hDC, rc.left, rc.top, rc.Width(), rc.Height(), di.hDC, 0, 0, SRCCOPY);

	SelectObject(pdi->hDC, SelectObject(di.hDC, hfont));
	DeleteObject(SelectObject(di.hDC, hbm));
	DeleteDC(di.hDC);
}

extern "C"
void WINAPI lccDrawButtonMenu(int menuStyle, DRAWITEMSTRUCT* pdi, LPCTSTR pText, HIMAGELIST hImage, int iImage, SIZE sizeButton, UINT txflg)
{
	lccDrawButtonMenuEx(menuStyle, pdi, pText, hImage, iImage, sizeButton, txflg, FALSE);
}

////////////////////////////////////////////////////////////////////////
//

static void _drawmenu_vista(DRAWITEMSTRUCT* pdi, LPCTSTR pText, HIMAGELIST hImage, int iImage, UINT txflg)
{
	CSize sizeIcon(16, 16);
	if (hImage) {
		int cx, cy;
		if (ImageList_GetIconSize(hImage, &cx, &cy)) {
			sizeIcon.cx = cx;
			sizeIcon.cy = cy;
		}
	}

	CDCHandle dc = pdi->hDC;
	CRect rcItem(pdi->rcItem);

	bool bGrayed = ((pdi->itemState & (ODS_GRAYED | ODS_DISABLED)) != 0);
	bool bSelected = ((pdi->itemState & ODS_SELECTED) != 0);
	bool bChecked  = ((pdi->itemState & ODS_CHECKED) != 0);

	COLORREF clrMenu = GetSysColor(COLOR_MENU);
	COLORREF clrShadow = GetSysColor(COLOR_3DSHADOW);
	COLORREF clrHilite = GetSysColor(COLOR_3DHILIGHT);
	COLORREF clrText = GetSysColor(bGrayed ? COLOR_GRAYTEXT : COLOR_MENUTEXT);
	COLORREF clrBorder = GetSysColor(bGrayed ? COLOR_3DSHADOW : COLOR_HIGHLIGHT);

	COLORREF clrPane = calc_innercolor(clrBorder);
	Graphics g(dc);

	// 背景
	HBRUSH hbrBack = ::GetSysColorBrush(COLOR_MENU);
	if (pdi->CtlType == ODT_MENU) {
		MENUINFO mi;
		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.fMask = MIM_BACKGROUND;
		if (::GetMenuInfo((HMENU)pdi->hwndItem, &mi))
			hbrBack = mi.hbrBack;
	}
	dc.FillRect(&rcItem, hbrBack);

	// イメージ矩形
	CRect rcIcon = rcItem;
	rcIcon.right = rcIcon.left + sizeIcon.cx + CX_ICON_MARGIN_LEFT + CX_ICON_MARGIN_RIGHT;

	CRect rc = rcIcon;
	rc.left = rc.right - 1;
	dc.FillSolidRect(&rc, RGB(0xe0, 0xe0, 0xe0));
	rc.OffsetRect(1, 0);
	dc.FillSolidRect(&rc, RGB(0xff, 0xff, 0xff));

	if (bSelected) {
		// Vistaメニュー
		lccDrawSelectionFrame(dc, &rcItem, !bGrayed, -1);
	} else if (bChecked) {
		// さらに薄い色で
		// 塗り潰し
		SolidBrush sbr(Color(32, GetRValue(clrBorder), GetGValue(clrBorder), GetBValue(clrBorder)));
		g.FillRectangle(&sbr, rcItem.left, rcItem.top, rcItem.Width(), rcItem.Height());
	}

	dc.SetBkMode(TRANSPARENT);
	CRect rcText = rcItem;
	rcText.left = rcIcon.right + CX_TEXT_GAP + CX_TEXT_MARGIN;

	// セパレータか否かを検証
	bool bSeparator = (pdi->itemID == 0);
	if (pdi->CtlType == ODT_MENU) {
		CMenuItemInfo mii;
		mii.fMask = MIIM_TYPE;
		if (GetMenuItemInfo((HMENU)pdi->hwndItem, pdi->itemID, FALSE, &mii))
			bSeparator = ((mii.fType & MFT_SEPARATOR) != 0);
	}

	if (bSeparator) {
		// メニューセパレータ
		rc = rcItem;
		rc.left = rcIcon.right + 1;
		rc.top += rcText.Height() / 2 - 1;
		rc.bottom = rc.top + 1;

		dc.FillSolidRect(&rc, RGB(0xe0, 0xe0, 0xe0));
		rc.OffsetRect(0, 1);
		dc.FillSolidRect(&rc, RGB(0xff, 0xff, 0xff));
		return;
	}

	// メニュー文字列
	if (bGrayed && !bSelected) {
		rc = rcText + CPoint(1, 1);
		draw_menutext(dc, rc, pText, clrHilite, txflg);
	}
	draw_menutext(dc, rcText, pText, clrText, txflg);

	// ボタン描画
	CPoint ptIcon(rcIcon.left + CX_ICON_MARGIN_LEFT, rcIcon.top + (rcIcon.Height() - sizeIcon.cy) / 2);
	if (hImage && iImage >= 0) {
		if (!bGrayed) {
			// ボタン枠
			if (bChecked && !bSelected)
				lccDrawSelectionFrame(dc, &rcIcon, !bGrayed, -1);
			// ボタン
			ImageList_Draw(hImage, iImage, dc, ptIcon.x, ptIcon.y, ILD_TRANSPARENT);
		} else {
			// use DrawEmbossed to draw disabeld button, w/color flag
			DrawEmbossedImage(dc, hImage, iImage, ptIcon);
		}
	} else {
		// no button: look for custom checked/unchecked bitmaps
		CMenuItemInfo info;
		info.fMask = MIIM_CHECKMARKS | MIIM_TYPE;
		::GetMenuItemInfo((HMENU)pdi->hwndItem, pdi->itemID, MF_BYCOMMAND, &info);
		if (bChecked || info.hbmpUnchecked != NULL) {
			// draw button hole
			rc.SetRect(ptIcon.x, ptIcon.y, ptIcon.x + sizeIcon.cx, ptIcon.y + sizeIcon.cy);
			if (!bSelected && !IsComm6())
				draw_frame(dc, rc, clrBorder, clrPane);
			BOOL bRadio = ((info.fType & MFT_RADIOCHECK) != 0);
			draw_check_xp(dc, rc, bRadio, bChecked ? info.hbmpChecked : info.hbmpUnchecked);
		}
	}
}

extern "C"
void WINAPI lccDrawCustomMenu(DRAWITEMSTRUCT* pdi, LPCTSTR pText, HIMAGELIST hImage, int iImage, UINT txflg)
{
	// フリッカーフリー描画
	DRAWITEMSTRUCT di;
	memcpy(&di, pdi, sizeof(di));
	OffsetRect(&di.rcItem, -pdi->rcItem.left, -pdi->rcItem.top);

	CRect rc(pdi->rcItem);
	di.hDC = CreateCompatibleDC(pdi->hDC);
	HBITMAP hbm = CreateCompatibleBitmap(pdi->hDC, rc.Width(), rc.Height());
	hbm = (HBITMAP)SelectObject(di.hDC, hbm);
	HFONT hfont = (HFONT)SelectObject(pdi->hDC, GetStockFont(DEFAULT_GUI_FONT));
	hfont = (HFONT)SelectObject(di.hDC, hfont);
	BitBlt(di.hDC, 0, 0, rc.Width(), rc.Height(), pdi->hDC, pdi->rcItem.left, pdi->rcItem.top, SRCCOPY);

	_drawmenu_vista(&di, pText, hImage, iImage, txflg);

	BitBlt(pdi->hDC, rc.left, rc.top, rc.Width(), rc.Height(), di.hDC, 0, 0, SRCCOPY);

	SelectObject(pdi->hDC, SelectObject(di.hDC, hfont));
	DeleteObject(SelectObject(di.hDC, hbm));
	DeleteDC(di.hDC);
}

extern "C"
BOOL WINAPI lccGetBitmapResourceInfo(HINSTANCE hInst, LPCTSTR pszResource, BITMAPINFOHEADER* pBitmapInfoHeader)
{
	HRSRC hres = FindResource(hInst, pszResource, RT_BITMAP);
	if (hres) {
		HGLOBAL hmem = LoadResource(hInst, hres);
		if (hmem) {
			LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)LockResource(hmem);
			if (pbh) {
				memcpy(pBitmapInfoHeader, pbh, sizeof(BITMAPINFOHEADER));
				return TRUE;
			}
		}
	}
	return FALSE;
}

extern "C"
BOOL WINAPI lccIsAlphaBitmapResource(HINSTANCE hInst, LPCTSTR pszResource)
{
	BITMAPINFOHEADER bh;
	return (lccGetBitmapResourceInfo(hInst, pszResource, &bh) && bh.biBitCount == 32);
}

extern "C"
HIMAGELIST WINAPI lccCreateImageList(HINSTANCE hInst, UINT idResource, int cxIcon, COLORREF crMask)
{
	//
	// もう使わない
	//

	BOOL alpha = lccIsAlphaBitmapResource(hInst, _MKIR(idResource));
	HBITMAP hbm = (HBITMAP)LoadImage(hInst, _MKIR(idResource), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
	if (!hbm) return NULL;

	BITMAP bm;
	GetObject(hbm, sizeof(bm), &bm);

	UINT flags = (alpha ? ILC_COLOR32 : ILC_COLORDDB) | ILC_MASK;
	HIMAGELIST hImage = ImageList_Create(cxIcon, bm.bmHeight, flags, bm.bmWidth / cxIcon, 1);
	if (hImage) ImageList_AddMasked(hImage, hbm, crMask);

	DeleteObject(hbm);
	return hImage;
}

void DrawAlphaImageList(HIMAGELIST him, int index, HDC hdc, int x, int y, BOOL bAlpha)
{
	if (!bAlpha)
		ImageList_Draw(him, index, hdc, x, y, ILD_TRANSPARENT);
	else {
		IMAGELISTDRAWPARAMS ildp;
		memset(&ildp, 0, sizeof(ildp));
		ildp.cbSize = sizeof(IMAGELISTDRAWPARAMS);
		ildp.himl = him;
		ildp.i = 0;
		ildp.hdcDst = hdc;
		ildp.x = x;
		ildp.y = y;
		ildp.cx = 0;
		ildp.cy = 0;
		ildp.xBitmap = 0;
		ildp.yBitmap = 0;
		ildp.rgbBk = CLR_NONE;
		ildp.rgbFg = CLR_DEFAULT;
		ildp.crEffect = 0;
		ildp.fStyle = ILD_TRANSPARENT;
		if (index == 2)
			ildp.fState = ILS_SATURATE;
		else
			ildp.fState = ILS_NORMAL;
		ImageList_DrawIndirect(&ildp);
	}
}

////////////////////////////////////////////////////////////////////////
//

extern TCHAR gszCommonSetting[];
static const TCHAR sszSec[] = _T("Menu");
static const TCHAR sszKey[] = _T("CoolStyle");

extern "C"
void WINAPI lccSetCoolStyle(BOOL bCool)
{
	lccRegSetInt(gszCommonSetting, sszSec, sszKey, bCool);
}

extern "C"
BOOL WINAPI lccGetCoolStyle()
{
	BOOL bXP;
	if (!lccRegGetInt(gszCommonSetting, sszSec, sszKey, &bXP)) bXP = FALSE;
	return bXP;
}
