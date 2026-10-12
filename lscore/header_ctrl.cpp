/*
 *  Header control
 *
 *  TODO:
 *   - Imagelist support (partially).
 *   - Callback items (under construction).
 *   - Hottrack support (partially).
 *   - Custom draw support (including Notifications).
 *   - Drag and Drop support (including Notifications).
 *   - New messages.
 *   - Use notification format
 *   - Correct the order maintenance code to preserve valid order
 *
 */

#include "stdafx.h"
#include "lscore.h"
#include "common.h"

typedef struct tagHEADER_ITEM{
	int cxy;
	HBITMAP hbm;
	LPTSTR pszText;
	int fmt;
	LPARAM lParam;
	int iImage;
	int iOrder;		/* see documentation of HD_ITEM */
	BOOL bDown;		/* is item pressed? (used for drawing) */
	RECT rect;		/* bounding rectangle of the item */
	void SetText(LPCTSTR pszTextNew)
	{
		if (pszText) {
			if (pszText != LPSTR_TEXTCALLBACK)
				free(pszText);
			pszText = NULL;
		}
		if (pszTextNew) {
			if (pszTextNew == LPSTR_TEXTCALLBACK)
				pszText = LPSTR_TEXTCALLBACK;
			else if (*pszTextNew) {
				pszText = (LPTSTR)malloc((lstrlen(pszTextNew) + 1) * sizeof(TCHAR));
				lstrcpy(pszText, pszTextNew);
			}
		}
	}
} HEADER_ITEM;

typedef struct tagHEADER_INFO {
	HWND      hwndNotify;		/* Owner window to send notifications to */
	INT       numItem;			/* number of items (columns) */
	INT       nHeight;			/* height of the header (pixels) */
	HFONT     hFont;			/* handle to the current font */
	HCURSOR   hcurArrow;		/* handle to the arrow cursor */
	HCURSOR   hcurDivider;		/* handle to a cursor (used over dividers) <-|-> */
	HCURSOR   hcurDivopen;		/* handle to a cursor (used over dividers) <-||-> */
	int iMoveItem;				/* index of tracked item. (Tracking mode) */
	int xTrackOffset;			/* distance between the right side of the tracked item and the cursor */
	int xOldTrack;				/* track offset (see above) after the last WM_MOUSEMOVE */
	int nOldWidth;				/* width of a sizing item after the last WM_MOUSEMOVE */
	int iHotItem;				/* index of hot item (cursor is over this item) */
	int iMargin;				/* width of the margin that surrounds a bitmap */

	HIMAGELIST himl;			/* handle to an image list (may be 0) */
	SIZE sizeImage;				// イメージリストのアイコンサイズ

	CAtlArray<HEADER_ITEM*> item_array;	/* pointer to array of HEADER_ITEM's */
	CAtlArray<int> order_array;			/* array of item IDs indexed by order */

	bool bCaptured:1;			/* Is the mouse captured? */
	bool bPressed:1;			/* Is a header item pressed (down)? */
	bool bTracking:1;			/* Is in tracking mode? */
	bool bRectsValid:1;			/* validity flag for bounding rectangles */

	inline int OrderToIndex(int iorder)
	{
		return (0 <= iorder && iorder < numItem) ? order_array[iorder] : -1;
	}
	inline int IndexToOrder(int index)
	{
		return (0 <= index && index < numItem) ? item_array[index]->iOrder : -1;
	}
	inline HEADER_ITEM* GetItem(int index)
	{
		return (0 <= index && index < (int)item_array.GetCount()) ? item_array[index] : NULL;
	}

} HEADER_INFO;


#define VERT_BORDER				3
#define DIVIDER_WIDTH			4

#define HEADER_GetInfoPtr(hwnd) ((HEADER_INFO*)GetWindowLongPtrW(hwnd, 0))

static const WCHAR themeClass[] = L"Header";


inline static LRESULT HEADER_IndexToOrder(HWND hwnd, int index)
{
	HEADER_INFO* Ip = HEADER_GetInfoPtr(hwnd);
	return Ip->IndexToOrder(index);
}

inline static int HEADER_OrderToIndex(HWND hwnd, WPARAM wParam)
{
	HEADER_INFO* Ip = HEADER_GetInfoPtr(hwnd);
	return Ip->OrderToIndex((int)wParam);
}

static void HEADER_SetItemBounds(HWND hwnd)
{
	HEADER_INFO* Ip = HEADER_GetInfoPtr(hwnd);
	Ip->bRectsValid = true;

	if (Ip->numItem == 0)
		return;

	CRect rc;
	GetClientRect(hwnd, &rc);

	int x = rc.left;
	for (int i = 0; i < Ip->numItem; i++) {
		HEADER_ITEM* item = Ip->GetItem(Ip->OrderToIndex(i));
		item->rect.top = rc.top;
		item->rect.bottom = rc.bottom;
		item->rect.left = x;
		item->rect.right = item->rect.left + ((item->cxy > 0) ? item->cxy : 0);
		x = item->rect.right;
    }
}

static LRESULT HEADER_Size(HWND hwnd, WPARAM wParam)
{
	HEADER_INFO* Ip = HEADER_GetInfoPtr(hwnd);
	Ip->bRectsValid = false;
	return 0;
}

static int HEADER_DrawItem(HWND hwnd, CDCHandle dc, int iItem, BOOL bHotTrack)
{
	HEADER_INFO* Ip = HEADER_GetInfoPtr(hwnd);
	HEADER_ITEM* item = Ip->GetItem(iItem);
	HTHEME theme = GetWindowTheme(hwnd);

	if (!Ip->bRectsValid)
		HEADER_SetItemBounds(hwnd);

	CRect r = item->rect;
	if (r.Width() == 0)
		return item->rect.right;

	int nOldBkMode = dc.GetBkMode();

	if (theme != NULL) {
		int state = (item->bDown) ? HIS_PRESSED : (bHotTrack ? HIS_HOT : HIS_NORMAL);
		DrawThemeBackground(theme, dc, HP_HEADERITEM, state, &r, NULL);
		GetThemeBackgroundContentRect(theme, dc, HP_HEADERITEM, state, &r, &r);
	}
	else {
		if (GetWindowLong(hwnd, GWL_STYLE) & HDS_BUTTONS) {
			if (item->bDown)
				dc.DrawEdge(&r, BDR_RAISEDOUTER, BF_RECT | BF_FLAT | BF_MIDDLE | BF_ADJUST);
			else
				dc.DrawEdge(&r, EDGE_RAISED, BF_RECT | BF_SOFT | BF_MIDDLE | BF_ADJUST);
		}
		else
			dc.DrawEdge(&r, EDGE_ETCHED, BF_BOTTOM | BF_RIGHT | BF_ADJUST);
	}
	if (item->bDown) {
		r.left += 2;
		r.top  += 2;
	}

	int cxEdge = GetSystemMetrics(SM_CXEDGE);
	r.InflateRect(cxEdge, 0);


	UINT rw, rh, /* width and height of r */
	*x = NULL, *w = NULL; /* x and width of the pic (bmp or img) which is part of cnt */
	/* cnt,txt,img,bmp */
	UINT cx, tx, ix, bx, cw, tw, iw, bw;
	BITMAP bmp;

	cw = tw = iw = bw = 0;
	rw = r.right - r.left;
	rh = r.bottom - r.top;

	if (item->fmt & HDF_STRING) {
		CRect rct;
		dc.DrawText(item->pszText, -1, &rct, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_CALCRECT);
		cw = rct.Width() + 2 * Ip->iMargin;
	}

	if ((item->fmt & HDF_IMAGE) && (Ip->himl)) {
		iw = Ip->sizeImage.cx + 2 * Ip->iMargin;
		x = &ix;
		w = &iw;
	}

	if ((item->fmt & HDF_BITMAP) && (item->hbm)) {
		GetObject(item->hbm, sizeof(BITMAP), (LPVOID)&bmp);
		bw = bmp.bmWidth + 2 * Ip->iMargin;
		if (!iw) {
			x = &bx;
			w = &bw;
		}
	}

	if (bw || iw)
		cw += *w;

	/* align cx using the unclipped cw */
	if ((item->fmt & HDF_JUSTIFYMASK) == HDF_LEFT)
		cx = r.left;
	else if ((item->fmt & HDF_JUSTIFYMASK) == HDF_CENTER)
		cx = r.left + rw / 2 - cw / 2;
	else /* HDF_RIGHT */
		cx = r.right - cw;

	/* clip cx & cw */
	if (cx < r.left)
		cx = r.left;
	if (cx + cw > r.right)
		cw = r.right - cx;

	tx = cx + Ip->iMargin;
	/* since cw might have changed we have to recalculate tw */
	tw = cw - Ip->iMargin * 2;

	if (iw || bw) {
		tw -= *w;
		if (item->fmt & HDF_BITMAP_ON_RIGHT) {
			/* put pic behind text */
			*x = cx + tw + Ip->iMargin * 3;
		}
		else {
			*x = cx + Ip->iMargin;
			/* move text behind pic */
			tx += *w;
		}
	}

	if (iw && bw) {
		/* since we're done with the layout we can
		 * now calculate the position of bmp which
		 * has no influence on alignment and layout
		 * because of img
		 */
		if ((item->fmt & HDF_JUSTIFYMASK) == HDF_RIGHT)
			bx = cx - bw + Ip->iMargin;
		else
			bx = cx + cw + Ip->iMargin;
	}

	if (iw || bw) {
		CClientDC dcc(hwnd);
		HRGN hClipRgn = CreateRectRgn(r.left, r.top, r.right, r.bottom);
		dcc.SelectClipRgn(hClipRgn);

		if (bw) {
			CDC dcb;
			dcb.CreateCompatibleDC(dcc);
			dcb.SelectBitmap(item->hbm);
			dcc.BitBlt(bx, r.top + ((int)rh - bmp.bmHeight) / 2, bmp.bmWidth, bmp.bmHeight, dcb, 0, 0, SRCCOPY);
		}

		if (iw) {
			ImageList_DrawEx(Ip->himl, item->iImage, dcc,
						ix, r.top + ((int)rh - Ip->sizeImage.cy) / 2,
						Ip->sizeImage.cx, Ip->sizeImage.cy, CLR_DEFAULT, CLR_DEFAULT, 0);
		}
		DeleteObject(hClipRgn);
	}

	if (((item->fmt & HDF_STRING) ||
		(!(item->fmt & (HDF_STRING|HDF_BITMAP|HDF_BITMAP_ON_RIGHT|HDF_IMAGE)))) &&
		(item->pszText)) {
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor((bHotTrack && !theme) ? COLOR_HIGHLIGHT : COLOR_BTNTEXT);
		r.left  = tx;
		r.right = tx + tw;
		dc.DrawText(item->pszText, -1, &r, DT_LEFT|DT_END_ELLIPSIS|DT_VCENTER|DT_SINGLELINE);
	}

	dc.SetBkMode(nOldBkMode);
	return item->rect.right;
}


static void HEADER_Refresh(HWND hwnd, CDCHandle dc)
{
	HEADER_INFO *Ip = HEADER_GetInfoPtr(hwnd);

	/* get rect for the bar, adjusted for the border */
	CRect rc;
	GetClientRect(hwnd, &rc);

	HFONT hFont = Ip->hFont ? Ip->hFont : (HFONT)GetStockObject(SYSTEM_FONT);
	HFONT hOldFont = dc.SelectFont(hFont);

	/* draw Background */
	HTHEME theme = GetWindowTheme(hwnd);
	if (theme == NULL) {
		dc.FillSolidRect(&rc, GetSysColor(COLOR_3DFACE));
	}

	int x = rc.left;
	for (int i = 0; x <= rc.right && i < Ip->numItem; i++) {
		BOOL bHotTrack = Ip->bTracking ? (Ip->iMoveItem == i) : (Ip->iHotItem == i);
		x = HEADER_DrawItem(hwnd, dc, Ip->OrderToIndex(i), bHotTrack);
	}

	if ((x <= rc.right) && (Ip->numItem > 0)) {
		rc.left = x;
		if (theme != NULL) {
			DrawThemeBackground(theme, dc, HP_HEADERITEM, HIS_NORMAL, &rc, NULL);
		}
		else {
			if (GetWindowLong(hwnd, GWL_STYLE) & HDS_BUTTONS)
				dc.DrawEdge(&rc, EDGE_RAISED, BF_TOP|BF_LEFT|BF_BOTTOM|BF_SOFT);
			else
				dc.DrawEdge(&rc, EDGE_ETCHED, BF_BOTTOM);
		}
	}
	dc.SelectFont(hOldFont);
}

static void HEADER_RefreshItem(HWND hwnd, HDC hdc, int index)
{
	HEADER_INFO* Ip = HEADER_GetInfoPtr(hwnd);

    HFONT hFont = Ip->hFont ? Ip->hFont : (HFONT)GetStockObject(SYSTEM_FONT);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

	BOOL bHotTrack = Ip->bTracking ? (Ip->iMoveItem == index) : (Ip->iHotItem == index);
	HEADER_DrawItem(hwnd, hdc, index, bHotTrack);
	SelectObject(hdc, hOldFont);
}

static int HEADER_InternalHitTest(HWND hwnd, POINT pt, UINT* pHit)
{
	HEADER_INFO* Ip = HEADER_GetInfoPtr(hwnd);

	CRect rc;
	GetClientRect(hwnd, &rc);

	*pHit = 0;
	BOOL bNoWidth = FALSE;
	if (rc.PtInRect(pt)) {
		if (Ip->numItem == 0) {
			*pHit |= HHT_NOWHERE;
			return 1;
		}
		else {
			/* somewhere inside */
			CRect rcTest;
			for (int i = 0; i < Ip->numItem; i++) {
				rc = Ip->GetItem(i)->rect;
				int width = rc.Width();
				if (width == 0) {
					bNoWidth = TRUE;
					continue;
				}
				if (rc.PtInRect(pt)) {
					if (width <= 2 * DIVIDER_WIDTH) {
						*pHit |= HHT_ONHEADER;
						return i;
					}
					if (i > 0) {
						rcTest = rc;
						rcTest.right = rcTest.left + DIVIDER_WIDTH;
						if (rcTest.PtInRect(pt)) {
							if (bNoWidth) {
								*pHit |= HHT_ONDIVOPEN;
								return (i - 1);
							}
							else {
								*pHit |= HHT_ONDIVIDER;
								return (i - 1);
							}
						}
					}
					rcTest = rc;
					rcTest.left = rcTest.right - DIVIDER_WIDTH;
					if (rcTest.PtInRect(pt)) {
						*pHit |= HHT_ONDIVIDER;
						return i;
					}
					*pHit |= HHT_ONHEADER;
					return i;
				}
			}

			/* check for last divider part (on nowhere) */
			rc = Ip->GetItem(Ip->numItem - 1)->rect;
			rc.left = rc.right;
			rc.right += DIVIDER_WIDTH;
			if (rc.PtInRect(pt)) {
				if (bNoWidth) {
					*pHit |= HHT_ONDIVOPEN;
					return (Ip->numItem - 1);
				}
				else {
					*pHit |= HHT_ONDIVIDER;
					return (Ip->numItem - 1);
				}
			}

			*pHit |= HHT_NOWHERE;
			return 1;
		}
	}
	else {
		if (pt.x < rc.left)
			*pHit |= HHT_TOLEFT;
		else if (pt.x > rc.right)
			*pHit |= HHT_TORIGHT;

		if (pt.y < rc.top)
			*pHit |= HHT_ABOVE;
		else if (pt.y > rc.bottom)
			*pHit |= HHT_BELOW;
	}
	return 1;
}


static void HEADER_DrawTrackLine(HWND hwnd, CDCHandle dc, int x)
{
	CRect rc;
	GetClientRect(hwnd, &rc);

	HPEN hpen = dc.SelectPen((HPEN)GetStockObject(BLACK_PEN));
	int rop = dc.SetROP2(R2_XORPEN);
	dc.MoveTo(x, rc.top);
	dc.LineTo(x, rc.bottom);
	dc.SetROP2(rop);
	dc.SelectPen(hpen);
}

static BOOL HEADER_SendSimpleNotify(HWND hwnd, UINT code)
{
	HEADER_INFO *Ip = HEADER_GetInfoPtr (hwnd);
	NMHDR nmhdr;
	nmhdr.hwndFrom = hwnd;
	nmhdr.idFrom = GetDlgCtrlID(hwnd);
	nmhdr.code = code;
	return (BOOL)SendMessage(Ip->hwndNotify, WM_NOTIFY, (WPARAM)nmhdr.idFrom, (LPARAM)&nmhdr);
}

static BOOL HEADER_SendHeaderNotify(HWND hwnd, UINT code, INT iItem, INT mask)
{
	HEADER_INFO* Ip = HEADER_GetInfoPtr(hwnd);

	HEADER_ITEM* item = Ip->GetItem(iItem);
	HDITEM hdItem;
	hdItem.mask = mask;
	hdItem.cxy = item->cxy;
	hdItem.hbm = item->hbm;
	hdItem.pszText = NULL;
	hdItem.cchTextMax = 0;
//	hdItem.pszText = item->pszText;
//	hdItem.cchTextMax = item->cchTextMax;
	hdItem.fmt = item->fmt;
	hdItem.lParam = item->lParam;
	hdItem.iOrder = item->iOrder;
	hdItem.iImage = item->iImage;

	NMHEADER nmhdr;
	nmhdr.hdr.hwndFrom = hwnd;
	nmhdr.hdr.idFrom = GetDlgCtrlID(hwnd);
	nmhdr.hdr.code = code;
	nmhdr.iItem = iItem;
	nmhdr.iButton = 0;
	nmhdr.pitem = &hdItem;
	return (BOOL)SendMessage(Ip->hwndNotify, WM_NOTIFY, (WPARAM)nmhdr.hdr.idFrom, (LPARAM)&nmhdr);
}

/**
 * Send Disp Info notification. 
 *   depends on NMHDDISPINFOW having same structure as NMHDDISPINFOA 
 *   (so we handle the two cases only doing a specific cast for pszText).
 *
 * @param hwnd : hwnd header container handler
 * @param mask : notification mask (usually HDI_TEXT or HDI_IMAGE)
 * @param pDispInfo : NMHDDISPINFO structure (can be unicode or ansi)
 * @param isW : TRUE if dispinfo is Unicode
 */
static BOOL HEADER_SendHeaderDispInfoNotify(HWND hwnd, int index, INT mask, HDITEM* hdItem, HEADER_ITEM* item)
{
	HEADER_INFO* Ip = HEADER_GetInfoPtr(hwnd);
	NMHDDISPINFO nhdi;

	memset(&nhdi, 0, sizeof(NMHDDISPINFO));
	nhdi.hdr.hwndFrom = hwnd;
	nhdi.hdr.idFrom = GetDlgCtrlID(hwnd);
	nhdi.hdr.code = HDN_GETDISPINFO;
	nhdi.iItem = index;
	nhdi.mask = mask;
//	nhdi.pszText      = malloc(sizeof(WCHAR) * 260);
//	nhdi.cchTextMax   = 260;
	BOOL ret = (BOOL)SendMessage(Ip->hwndNotify, WM_NOTIFY, (WPARAM)nhdi.hdr.idFrom, (LPARAM)&nhdi);

	if (nhdi.mask & HDI_DI_SETITEM) {
		if (nhdi.mask & HDI_IMAGE)
			item->iImage = nhdi.iImage;
		if (nhdi.mask & HDI_TEXT)
			item->SetText(nhdi.pszText);

//		FIXME("NMHDDISPINFO returns with flags HDI_DI_SETITEM\n");
	}

	if (hdItem) {
		if ((hdItem->mask & mask) & HDI_IMAGE)
			hdItem->iImage = nhdi.iImage;
		if ((hdItem->mask & mask) & HDI_TEXT)
			lstrcpyn(hdItem->pszText, nhdi.pszText, hdItem->cchTextMax);
	}
	return ret;
}


static BOOL HEADER_SendClickNotify(HWND hwnd, UINT code, INT iItem)
{
	HEADER_INFO *Ip = HEADER_GetInfoPtr (hwnd);
	NMHEADERA nmhdr;
	nmhdr.hdr.hwndFrom = hwnd;
	nmhdr.hdr.idFrom = GetDlgCtrlID(hwnd);
	nmhdr.hdr.code = code;
	nmhdr.iItem = iItem;
	nmhdr.iButton = 0;
	nmhdr.pitem = NULL;
	return (BOOL)SendMessage(Ip->hwndNotify, WM_NOTIFY, (WPARAM)nmhdr.hdr.idFrom, (LPARAM)&nmhdr);
}


static LRESULT HEADER_CreateDragImage(HWND hwnd, WPARAM wParam)
{
//	FIXME("empty stub!\n");
	return 0;
}

static LRESULT HEADER_DeleteItem(HWND hwnd, WPARAM wParam)
{
	HEADER_INFO *Ip = HEADER_GetInfoPtr(hwnd);
	INT iItem = (INT)wParam;

	if ((iItem < 0) || (iItem >= (INT)Ip->numItem))
		return FALSE;

	HEADER_ITEM* item;
	if (Ip->numItem == 1) {
		item = Ip->GetItem(0);
		if (item->pszText)
			free(item->pszText);
		free(item);
		Ip->item_array.RemoveAll();
		Ip->order_array.RemoveAll();
		Ip->numItem = 0;
	}
	else {
		item = Ip->GetItem(iItem);
		if (item->pszText)
			free(item->pszText);
		int iOrder = item->iOrder;
		free(item);
		Ip->item_array.RemoveAt(iItem);
		Ip->numItem--;

		/* Correct the orders */
		if (iOrder < Ip->numItem) {
			Ip->order_array.RemoveAt(iOrder);
			for (int i = 0; i < Ip->numItem; i++) {
				if (Ip->order_array[i] > iItem)
					Ip->order_array[i]--;
				if (i >= iOrder)
					Ip->GetItem(Ip->order_array[i])->iOrder = Ip->order_array[i];
			}
		}
	}
	HEADER_SetItemBounds(hwnd);
	InvalidateRect(hwnd, NULL, FALSE);
	return TRUE;
}


static LRESULT HEADER_GetImageList(HWND hwnd)
{
	HEADER_INFO *Ip = HEADER_GetInfoPtr (hwnd);
	return (LRESULT)Ip->himl;
}

static LRESULT HEADER_GetItem(HWND hwnd, int index, HDITEM* hdItem)
{
	HEADER_INFO*Ip = HEADER_GetInfoPtr(hwnd);

	if (!hdItem || (index < 0) || (index >= Ip->numItem))
		return FALSE;

	if (hdItem->mask == 0)
		return TRUE;

	HEADER_ITEM* item = Ip->GetItem(index);

	if (hdItem->mask & HDI_BITMAP)
		hdItem->hbm = item ? item->hbm : 0;

	if (hdItem->mask & HDI_FORMAT)
		hdItem->fmt = item ? item->fmt : 0;

	if (hdItem->mask & HDI_WIDTH)
		hdItem->cxy = item ? item->cxy : 0;

	if (hdItem->mask & HDI_LPARAM)
		hdItem->lParam = item ? item->lParam : 0;

	if (hdItem->mask & HDI_IMAGE) {
		hdItem->iImage = item ? item->iImage : 0;
		if (item->iImage == I_IMAGECALLBACK) {
			HEADER_SendHeaderDispInfoNotify(hwnd, index, HDI_IMAGE, hdItem, item);
		}
	}

	if (hdItem->mask & HDI_ORDER)
		hdItem->iOrder = item ? item->iOrder : 0;

	if (hdItem->mask & HDI_TEXT) {
		if (!item)
			*hdItem->pszText = 0;  /* null pointer check */
		else if (item->pszText == LPSTR_TEXTCALLBACK)
			HEADER_SendHeaderDispInfoNotify(hwnd, index, HDI_TEXT, hdItem, item);
		else
			lstrcpyn(hdItem->pszText, item->pszText, hdItem->cchTextMax);
	}
	return TRUE;
}

inline static LRESULT HEADER_GetItemCount(HWND hwnd)
{
	HEADER_INFO *Ip = HEADER_GetInfoPtr(hwnd);
	return Ip->numItem;
}

static LRESULT HEADER_GetItemRect(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	HEADER_INFO *Ip = HEADER_GetInfoPtr (hwnd);

	int index = (int)wParam;
	if (index < 0 || index >= Ip->numItem)
		return FALSE;

	RECT* prc = (RECT*)lParam;
	HEADER_ITEM* item = Ip->GetItem(index);
	*prc = item->rect;
	return TRUE;
}

static LRESULT HEADER_GetOrderArray(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	HEADER_INFO* Ip = HEADER_GetInfoPtr(hwnd);

	if ((int)wParam < Ip->numItem)
		return FALSE;

	int* order = (int*)lParam;
	for (int i = 0; i < Ip->numItem; i++)
		*order++ = Ip->order_array[i];
	return TRUE;
}

static LRESULT HEADER_SetOrderArray(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	HEADER_INFO* Ip = HEADER_GetInfoPtr(hwnd);

	if ((int)wParam < Ip->numItem)
		return FALSE;

	int i;
	int* order = (int*)lParam;
	for (i = 0; i < Ip->numItem; i++)
		Ip->order_array[i] = order[i];

	for (i = 0; i < (int)wParam; i++) {
		HEADER_ITEM* item = Ip->GetItem(order[i]);
		item->iOrder = i;
	}
	Ip->bRectsValid = false;
	InvalidateRect(hwnd, NULL, FALSE);
	return TRUE;
}

inline static LRESULT HEADER_GetUnicodeFormat(HWND hwnd)
{
	return TRUE;
}

static LRESULT HEADER_HitTest(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	HDHITTESTINFO* phti = (HDHITTESTINFO*)lParam;
	phti->iItem = HEADER_InternalHitTest(hwnd, phti->pt, &phti->flags);
	if (phti->flags == HHT_NOWHERE)
		return -1;
	else
		return phti->iItem;
}

static LRESULT HEADER_InsertItem(HWND hwnd, int index, HDITEM* hdItem)
{
	HEADER_INFO* Ip = HEADER_GetInfoPtr(hwnd);

	if (hdItem == NULL || index < 0)
		return -1;

	if (index > Ip->numItem)
		index = Ip->numItem;

	int iOrder = (hdItem->mask & HDI_ORDER) ? hdItem->iOrder : index;
	if (iOrder < 0)
		iOrder = 0;
	else if (Ip->numItem < iOrder)
		iOrder = Ip->numItem;

	HEADER_ITEM* item = (HEADER_ITEM*)malloc(sizeof(HEADER_ITEM));
	memset(item, 0, sizeof(HEADER_ITEM));

	if (Ip->numItem == 0) {
		Ip->item_array.InsertAt(0, item);
		Ip->order_array.InsertAt(0, 0);
	}
	else {
		Ip->item_array.InsertAt(index, item);
		Ip->order_array.InsertAt(index, iOrder);
	}
	Ip->numItem++;

	int i;
	for (i = 0; i < Ip->numItem; i++) {
		if (i != iOrder && Ip->order_array[i] >= index)
			Ip->order_array[i]++;
		Ip->GetItem(Ip->order_array[i])->iOrder = Ip->order_array[i];
	}

	item->bDown = FALSE;

	if (hdItem->mask & HDI_WIDTH)
		item->cxy = hdItem->cxy;

	if (hdItem->mask & HDI_FORMAT)
		item->fmt = hdItem->fmt;

	if (item->fmt == 0)
		item->fmt = HDF_LEFT;

	if (hdItem->mask & HDI_BITMAP)
		item->hbm = hdItem->hbm;

	if (hdItem->mask & HDI_LPARAM)
		item->lParam = hdItem->lParam;

	if (hdItem->mask & HDI_IMAGE) {
		if (hdItem->iImage != I_IMAGECALLBACK)
			item->iImage = hdItem->iImage;
		else {
			item->iImage = hdItem->iImage;
			HEADER_SendHeaderDispInfoNotify(hwnd, index, HDI_IMAGE, NULL, item);
		}
	}

	if (hdItem->mask & HDI_TEXT) {
		if (hdItem->pszText != LPSTR_TEXTCALLBACK)
			item->SetText(hdItem->pszText);
		else {
			item->pszText = LPSTR_TEXTCALLBACK;
			HEADER_SendHeaderDispInfoNotify(hwnd, index, HDI_TEXT, NULL, item);
		}
		item->fmt |= HDF_STRING;
	}

	item->iOrder = iOrder;

	HEADER_SetItemBounds(hwnd);
	InvalidateRect(hwnd, NULL, FALSE);
	return index;
}

static LRESULT HEADER_Layout(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	HEADER_INFO *Ip = HEADER_GetInfoPtr (hwnd);
	LPHDLAYOUT lpLayout = (LPHDLAYOUT)lParam;

	lpLayout->pwpos->hwnd = hwnd;
	lpLayout->pwpos->hwndInsertAfter = 0;
	lpLayout->pwpos->x = lpLayout->prc->left;
	lpLayout->pwpos->y = lpLayout->prc->top;
	lpLayout->pwpos->cx = lpLayout->prc->right - lpLayout->prc->left;
	if (GetWindowLong(hwnd, GWL_STYLE) & HDS_HIDDEN)
		lpLayout->pwpos->cy = 0;
	else {
		lpLayout->pwpos->cy = Ip->nHeight;
		lpLayout->prc->top += Ip->nHeight;
	}
	lpLayout->pwpos->flags = SWP_NOZORDER;
	Ip->bRectsValid = false;
	return TRUE;
}

static LRESULT HEADER_SetImageList(HWND hwnd, HIMAGELIST himl)
{
	HEADER_INFO *Ip = HEADER_GetInfoPtr (hwnd);
	HIMAGELIST himlOld;

	himlOld = Ip->himl;
	Ip->himl = himl;
	if (Ip->himl) {
		int cx, cy;
		ImageList_GetIconSize(Ip->himl, &cx, &cy);
		Ip->sizeImage.cx = cx;
		Ip->sizeImage.cy = cy;
	}
	else {
		Ip->sizeImage.cx = 0;
		Ip->sizeImage.cy = 0;
	}

//	FIXME: Refresh needed???

	return (LRESULT)himlOld;
}

static LRESULT HEADER_GetBitmapMargin(HWND hwnd)
{
	HEADER_INFO *Ip = HEADER_GetInfoPtr(hwnd);
	return Ip->iMargin;
}

static LRESULT HEADER_SetBitmapMargin(HWND hwnd, WPARAM wParam)
{
	HEADER_INFO *Ip = HEADER_GetInfoPtr (hwnd);
	INT oldMargin = Ip->iMargin;
	Ip->iMargin = (INT)wParam;
	return oldMargin;
}

static LRESULT HEADER_SetItem(HWND hwnd, int index, HDITEM* hdItem)
{
	HEADER_INFO *Ip = HEADER_GetInfoPtr (hwnd);

	if (hdItem == NULL || index < 0 || index >= Ip->numItem)
		return FALSE;

	if (HEADER_SendHeaderNotify(hwnd, HDN_ITEMCHANGING, index, hdItem->mask))
		return FALSE;

	HEADER_ITEM* item = Ip->GetItem(index);
	if (hdItem->mask & HDI_BITMAP)
		item->hbm = hdItem->hbm;

	if (hdItem->mask & HDI_FORMAT)
		item->fmt = hdItem->fmt;

	if (hdItem->mask & HDI_LPARAM)
		item->lParam = hdItem->lParam;

	if (hdItem->mask & HDI_WIDTH)
		item->cxy = hdItem->cxy;

	if (hdItem->mask & HDI_IMAGE) {
		if (hdItem->iImage != I_IMAGECALLBACK)
			item->iImage = hdItem->iImage;
		else {
			item->iImage = hdItem->iImage;
			HEADER_SendHeaderDispInfoNotify(hwnd, index, HDI_IMAGE, NULL, item);
		}
	}

	if (hdItem->mask & HDI_TEXT) {
		if (hdItem->pszText != LPSTR_TEXTCALLBACK)		/* covers != TEXTCALLBACKA too */
			item->SetText(hdItem->pszText);
		else {
			item->pszText = LPSTR_TEXTCALLBACK;
			HEADER_SendHeaderDispInfoNotify(hwnd, index, HDI_TEXT, NULL, item);
		}
	}

	if (hdItem->mask & HDI_ORDER) {
		if (item->iOrder < hdItem->iOrder)
			memmove(&Ip->order_array[item->iOrder], &Ip->order_array[item->iOrder + 1], (hdItem->iOrder - item->iOrder) * sizeof(INT));
		if (hdItem->iOrder < item->iOrder)
			memmove(&Ip->order_array[hdItem->iOrder + 1], &Ip->order_array[hdItem->iOrder], (item->iOrder - hdItem->iOrder) * sizeof(INT));

		Ip->order_array[hdItem->iOrder] = index;

		int nMin = min(item->iOrder, hdItem->iOrder);
		int nMax = max(item->iOrder, hdItem->iOrder);
		for (int i = nMin; i <= nMax; i++)
			Ip->GetItem(Ip->order_array[i])->iOrder = Ip->order_array[i];
	}

	HEADER_SendHeaderNotify(hwnd, HDN_ITEMCHANGED, index, hdItem->mask);
	HEADER_SetItemBounds(hwnd);
	InvalidateRect(hwnd, NULL, FALSE);

	return TRUE;
}

inline static LRESULT HEADER_SetUnicodeFormat(HWND hwnd, WPARAM wParam)
{
	return TRUE;
}

static LRESULT HEADER_Create(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	HEADER_INFO* Ip = (HEADER_INFO*)malloc(sizeof(HEADER_INFO));
	SetWindowLongPtr(hwnd, 0, (DWORD_PTR)Ip);

	HMODULE hmod = GetModuleHandle(_T("comctl32"));
	Ip->hwndNotify = ((LPCREATESTRUCT)lParam)->hwndParent;
	Ip->numItem = 0;
	Ip->hFont = 0;
	Ip->bRectsValid = false;
	Ip->hcurArrow = LoadCursor(0, IDC_ARROW);
	Ip->hcurDivider = LoadCursor(hmod, MAKEINTRESOURCE(106));
	Ip->hcurDivopen = LoadCursor(hmod, MAKEINTRESOURCE(107));
	Ip->bPressed  = false;
	Ip->bTracking = false;
	Ip->iMoveItem = 0;
	Ip->himl = 0;
	Ip->iHotItem = -1;
	Ip->iMargin = 3 * GetSystemMetrics(SM_CXEDGE);
	Ip->sizeImage.cx = Ip->sizeImage.cy = 0;

	CClientDC dc(NULL);
	HFONT hOldFont = dc.SelectFont((HFONT)GetStockObject(SYSTEM_FONT));
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	Ip->nHeight = tm.tmHeight + VERT_BORDER;
	dc.SelectFont(hOldFont);

	OpenThemeData(hwnd, themeClass);

	return 0;
}

static LRESULT HEADER_Destroy(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	HEADER_INFO *Ip = HEADER_GetInfoPtr (hwnd);

	int cnt = (int)Ip->item_array.GetCount();
	for (int i = 0; i < cnt; i++) {
		HEADER_ITEM* item = Ip->GetItem(i);
		if (item->pszText && item->pszText != LPSTR_TEXTCALLBACK)
			free(item->pszText);
		free(item);
	}
	Ip->item_array.RemoveAll();
	Ip->order_array.RemoveAll();

	if (Ip->himl)
		ImageList_Destroy(Ip->himl);

	SetWindowLongPtr(hwnd, 0, 0);
	free(Ip);

	HTHEME theme = GetWindowTheme(hwnd);
	CloseThemeData(theme);
	return 0;
}

static inline LRESULT HEADER_GetFont(HWND hwnd)
{
	HEADER_INFO *Ip = HEADER_GetInfoPtr (hwnd);
	return (LRESULT)Ip->hFont;
}

static LRESULT HEADER_OnLButtonDblClk(HWND hwnd, UINT fwKeys, CPoint pt)
{
	UINT hit;
	int index = HEADER_InternalHitTest(hwnd, pt, &hit);

	if ((GetWindowLong(hwnd, GWL_STYLE) & HDS_BUTTONS) && (hit == HHT_ONHEADER))
		HEADER_SendHeaderNotify(hwnd, HDN_ITEMDBLCLICK, index, 0);
	else if ((hit == HHT_ONDIVIDER) || (hit == HHT_ONDIVOPEN))
		HEADER_SendHeaderNotify(hwnd, HDN_DIVIDERDBLCLICK, index, 0);
	return 0;
}

static LRESULT HEADER_OnLButtonDown(HWND hwnd, UINT fwKeys, CPoint pt)
{
	HEADER_INFO* Ip = HEADER_GetInfoPtr(hwnd);
	DWORD style = GetWindowLongW(hwnd, GWL_STYLE);
	HDC hdc;

	UINT hit;
	int index = HEADER_InternalHitTest(hwnd, pt, &hit);

	HEADER_ITEM* item = Ip->GetItem(index);
	if ((style & HDS_BUTTONS) && (hit == HHT_ONHEADER)) {
		SetCapture(hwnd);
		Ip->bCaptured = true;
		Ip->bPressed  = true;
		Ip->iMoveItem = index;

		item->bDown = TRUE;

		hdc = GetDC(hwnd);
		HEADER_RefreshItem(hwnd, hdc, index);
		ReleaseDC(hwnd, hdc);
	}
	else if ((hit == HHT_ONDIVIDER) || (hit == HHT_ONDIVOPEN)) {
		if (!(HEADER_SendHeaderNotify(hwnd, HDN_BEGINTRACK, index, 0))) {
			SetCapture(hwnd);
			Ip->bCaptured = true;
			Ip->bTracking = true;
			Ip->iMoveItem = index;
			Ip->nOldWidth = item->cxy;
			Ip->xTrackOffset = item->rect.right - pt.x;

			if (!(style & HDS_FULLDRAG)) {
				Ip->xOldTrack = item->rect.right;
				hdc = GetDC(hwnd);
				HEADER_DrawTrackLine(hwnd, hdc, Ip->xOldTrack);
				ReleaseDC(hwnd, hdc);
			}
		}
	}
	return 0;
}

static LRESULT HEADER_OnLButtonUp(HWND hwnd, UINT fwKeys, CPoint pt)
{
	HEADER_INFO* Ip = HEADER_GetInfoPtr(hwnd);
	HDC   hdc;

	UINT hit;
	int index = HEADER_InternalHitTest(hwnd, pt, &hit);

	HEADER_ITEM* item;
	if (Ip->bPressed) {
		if ((index == Ip->iMoveItem) && (hit == HHT_ONHEADER)) {
			Ip->GetItem(Ip->iMoveItem)->bDown = FALSE;
			hdc = GetDC(hwnd);
			HEADER_RefreshItem(hwnd, hdc, Ip->iMoveItem);
			ReleaseDC(hwnd, hdc);
			HEADER_SendClickNotify(hwnd, HDN_ITEMCLICK, Ip->iMoveItem);
		}
		else if (hit == HHT_ONHEADER) {
			INT newindex = Ip->IndexToOrder(index);
			INT oldindex = Ip->IndexToOrder(Ip->iMoveItem);
			item= Ip->GetItem(index);
			item->iOrder=oldindex;

			item= Ip->GetItem(Ip->iMoveItem);
			item->iOrder = newindex;

			Ip->order_array[oldindex] = index;
			Ip->order_array[newindex] = Ip->iMoveItem;

			Ip->bRectsValid = false;
			InvalidateRect(hwnd, NULL, FALSE);
			// FIXME: Should some WM_NOTIFY be sent
		}

		Ip->bPressed = false;
	}
	else if (Ip->bTracking) {
		Ip->bTracking = false;
		HEADER_SendHeaderNotify(hwnd, HDN_ENDTRACK, Ip->iMoveItem, HDI_WIDTH);

		/*
		 * we want to do this even for HDS_FULLDRAG because this is where
		 * we send the HDN_ITEMCHANGING and HDN_ITEMCHANGED notifications
		 *
		 * if (!(dwStyle & HDS_FULLDRAG)) {
		 */
			hdc = GetDC(hwnd);
			HEADER_DrawTrackLine(hwnd, hdc, Ip->xOldTrack);
			ReleaseDC(hwnd, hdc);

			item = Ip->GetItem(Ip->iMoveItem);
			if (HEADER_SendHeaderNotify(hwnd, HDN_ITEMCHANGING, Ip->iMoveItem, HDI_WIDTH))
				item->cxy = Ip->nOldWidth;
			else {
				int nWidth = pt.x - item->rect.left + Ip->xTrackOffset;
				if (nWidth < 0)
					nWidth = 0;
				item->cxy = nWidth;
			}

			HEADER_SetItemBounds(hwnd);
			InvalidateRect(hwnd, NULL, TRUE);
			HEADER_SendHeaderNotify(hwnd, HDN_ITEMCHANGED, Ip->iMoveItem, HDI_WIDTH);
		/*
		 * }
		 */
	}
	if (Ip->bCaptured) {
		Ip->bCaptured = false;
		ReleaseCapture();
		HEADER_SendSimpleNotify(hwnd, NM_RELEASEDCAPTURE);
	}
	return 0;
}


static LRESULT HEADER_NotifyFormat(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	switch (lParam) {
	case NF_QUERY:
	case NF_REQUERY:
		return NFR_UNICODE;
	}
	return 0;
}

static LRESULT HEADER_OnMouseLeave(HWND hwnd)
{
	HEADER_INFO *Ip = HEADER_GetInfoPtr(hwnd);

	/* Reset hot-tracked item when mouse leaves control. */
	int oldHotItem = Ip->iHotItem;

	Ip->iHotItem = -1;
	if (oldHotItem != -1) {
		HDC hdc = GetDC(hwnd);
		HEADER_RefreshItem(hwnd, hdc, oldHotItem);
		ReleaseDC (hwnd, hdc);
	}
    return 0;
}


static LRESULT HEADER_OnMouseMove(HWND hwnd, UINT fwKeys, CPoint pt)
{
	HEADER_INFO* Ip = HEADER_GetInfoPtr(hwnd);
	INT   nWidth;
	HDC   hdc;

	/* With theming, hottracking is always enabled */
	DWORD style = GetWindowLong(hwnd, GWL_STYLE);
	BOOL bHotTrackEnabled =
			((style & HDS_BUTTONS) && (style & HDS_HOTTRACK)) || (GetWindowTheme(hwnd) != NULL);

	int oldHotItem = Ip->iHotItem;

	UINT hit;
	int index = HEADER_InternalHitTest(hwnd, pt, &hit);

	if (bHotTrackEnabled) {
		if (hit & (HHT_ONHEADER | HHT_ONDIVIDER | HHT_ONDIVOPEN))
			Ip->iHotItem = index;
		else
			Ip->iHotItem = -1;
	}

	if (Ip->bCaptured) {
		HEADER_ITEM* item = Ip->GetItem(Ip->iMoveItem);

		if (Ip->bPressed) {
			BOOL oldState = item->bDown;
			if ((index == Ip->iMoveItem) && (hit == HHT_ONHEADER))
				item->bDown = TRUE;
			else
				item->bDown = FALSE;
			if (oldState != item->bDown) {
				hdc = GetDC(hwnd);
				HEADER_RefreshItem(hwnd, hdc, Ip->iMoveItem);
				ReleaseDC(hwnd, hdc);
			}
		}
		else if (Ip->bTracking) {
			if (style & HDS_FULLDRAG) {
				if (HEADER_SendHeaderNotify(hwnd, HDN_TRACK, Ip->iMoveItem, HDI_WIDTH)) {
					nWidth = pt.x - item->rect.left + Ip->xTrackOffset;
					if (nWidth < 0)
						nWidth = 0;
					item->cxy = nWidth;
					HEADER_SendHeaderNotify(hwnd, HDN_ITEMCHANGED, Ip->iMoveItem, HDI_WIDTH);
				}
				HEADER_SetItemBounds(hwnd);
			}
			else {
				hdc = GetDC(hwnd);
				HEADER_DrawTrackLine(hwnd, hdc, Ip->xOldTrack);
				Ip->xOldTrack = pt.x + Ip->xTrackOffset;
				if (Ip->xOldTrack < item->rect.left)
					Ip->xOldTrack = item->rect.left;
				item->cxy = Ip->xOldTrack - item->rect.left;
				HEADER_DrawTrackLine(hwnd, hdc, Ip->xOldTrack);
				ReleaseDC(hwnd, hdc);
				HEADER_SendHeaderNotify(hwnd, HDN_TRACK, Ip->iMoveItem, HDI_WIDTH);
			}
		}
	}

	if (bHotTrackEnabled) {
		TRACKMOUSEEVENT tme;
		if (oldHotItem != Ip->iHotItem) {
			hdc = GetDC(hwnd);
			if (oldHotItem != -1)
				HEADER_RefreshItem(hwnd, hdc, oldHotItem);
			if (Ip->iHotItem != -1)
				HEADER_RefreshItem(hwnd, hdc, Ip->iHotItem);
			ReleaseDC(hwnd, hdc);
		}
		tme.cbSize = sizeof( tme );
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = hwnd;
		TrackMouseEvent( &tme );
	}
	return 0;
}


static LRESULT HEADER_Paint(HWND hwnd, WPARAM wParam)
{
    PAINTSTRUCT ps;

	HDC hdc = (wParam == 0) ? BeginPaint(hwnd, &ps) : (HDC)wParam;
	HEADER_Refresh(hwnd, hdc);
	if (!wParam)
		EndPaint(hwnd, &ps);
	return 0;
}


static LRESULT HEADER_OnRButtonUp(HWND hwnd, UINT fwKeys, CPoint pt)
{
	/* Send a Notify message */
	BOOL bRet = HEADER_SendSimpleNotify(hwnd, NM_RCLICK);

	/* Change to screen coordinate for WM_CONTEXTMENU */
	ClientToScreen(hwnd, &pt);

	/* Send a WM_CONTEXTMENU message in response to the RBUTTONUP */
	SendMessage(hwnd, WM_CONTEXTMENU, (WPARAM)hwnd, MAKELPARAM(pt.x, pt.y));
	return bRet;
}


static LRESULT HEADER_SetCursor(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	HEADER_INFO *Ip = HEADER_GetInfoPtr (hwnd);

	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(hwnd, &pt);

	UINT hit;
	HEADER_InternalHitTest(hwnd, pt, &hit);
	if (hit == HHT_ONDIVIDER)		SetCursor(Ip->hcurDivider);
	else if (hit == HHT_ONDIVOPEN)	SetCursor(Ip->hcurDivopen);
	else							SetCursor(Ip->hcurArrow);
	return 0;
}

static LRESULT HEADER_SetFont(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	HEADER_INFO *Ip = HEADER_GetInfoPtr (hwnd);

	Ip->hFont = (HFONT)wParam;
	HFONT hFont = Ip->hFont ? Ip->hFont : (HFONT)GetStockObject(SYSTEM_FONT);

	CClientDC dc(NULL);
	HFONT hOldFont = dc.SelectFont(hFont);

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);

	Ip->nHeight = tm.tmHeight + VERT_BORDER;
	dc.SelectFont(hOldFont);

	Ip->bRectsValid = false;

	if (lParam) {
		InvalidateRect(hwnd, NULL, FALSE);
	}
	return 0;
}

/* Update the theme handle after a theme change */
static LRESULT HEADER_ThemeChanged(HWND hwnd)
{
	HTHEME theme = GetWindowTheme(hwnd);
	CloseThemeData(theme);
	OpenThemeData(hwnd, themeClass);
	InvalidateRect(hwnd, NULL, FALSE);
	return 0;
}

static LRESULT WINAPI HEADER_WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!HEADER_GetInfoPtr (hwnd) && (msg != WM_CREATE))
	return DefWindowProc(hwnd, msg, wParam, lParam);
    switch (msg) {
/*	case HDM_CLEARFILTER: */

	case HDM_CREATEDRAGIMAGE:
	    return HEADER_CreateDragImage (hwnd, wParam);

	case HDM_DELETEITEM:
	    return HEADER_DeleteItem (hwnd, wParam);

/*	case HDM_EDITFILTER: */

	case HDM_GETBITMAPMARGIN:
	    return HEADER_GetBitmapMargin(hwnd);

	case HDM_GETIMAGELIST:
	    return HEADER_GetImageList (hwnd);

	case HDM_GETITEM:
	    return HEADER_GetItem(hwnd, (INT)wParam, (HDITEM*)lParam);

	case HDM_GETITEMCOUNT:
	    return HEADER_GetItemCount (hwnd);

	case HDM_GETITEMRECT:
	    return HEADER_GetItemRect (hwnd, wParam, lParam);

	case HDM_GETORDERARRAY:
	    return HEADER_GetOrderArray(hwnd, wParam, lParam);

	case HDM_GETUNICODEFORMAT:
	    return HEADER_GetUnicodeFormat (hwnd);

	case HDM_HITTEST:
	    return HEADER_HitTest (hwnd, wParam, lParam);

	case HDM_INSERTITEM:
	    return HEADER_InsertItem(hwnd, (INT)wParam, (HDITEM*)lParam);

	case HDM_LAYOUT:
	    return HEADER_Layout (hwnd, wParam, lParam);

	case HDM_ORDERTOINDEX:
	    return HEADER_OrderToIndex(hwnd, wParam);

	case HDM_SETBITMAPMARGIN:
	    return HEADER_SetBitmapMargin(hwnd, wParam);

/*	case HDM_SETFILTERCHANGETIMEOUT: */

/*	case HDM_SETHOTDIVIDER: */

	case HDM_SETIMAGELIST:
	    return HEADER_SetImageList (hwnd, (HIMAGELIST)lParam);

	case HDM_SETITEM:
	    return HEADER_SetItem(hwnd, (INT)wParam, (HDITEM*)lParam);

	case HDM_SETORDERARRAY:
	    return HEADER_SetOrderArray(hwnd, wParam, lParam);

	case HDM_SETUNICODEFORMAT:
	    return HEADER_SetUnicodeFormat (hwnd, wParam);

        case WM_CREATE:
            return HEADER_Create (hwnd, wParam, lParam);

        case WM_DESTROY:
            return HEADER_Destroy (hwnd, wParam, lParam);

        case WM_ERASEBKGND:
            return 1;

        case WM_GETDLGCODE:
            return DLGC_WANTTAB | DLGC_WANTARROWS;

        case WM_GETFONT:
            return HEADER_GetFont (hwnd);

        case WM_LBUTTONDBLCLK:
            return HEADER_OnLButtonDblClk(hwnd, (UINT)wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));

        case WM_LBUTTONDOWN:
            return HEADER_OnLButtonDown(hwnd, (UINT)wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));

        case WM_LBUTTONUP:
            return HEADER_OnLButtonUp(hwnd, (UINT)wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));

        case WM_MOUSELEAVE:
            return HEADER_OnMouseLeave(hwnd);

        case WM_MOUSEMOVE:
            return HEADER_OnMouseMove(hwnd, (UINT)wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));

	case WM_NOTIFYFORMAT:
            return HEADER_NotifyFormat (hwnd, wParam, lParam);

	case WM_SIZE:
	    return HEADER_Size (hwnd, wParam);

        case WM_THEMECHANGED:
            return HEADER_ThemeChanged (hwnd);

        case WM_PRINTCLIENT:
        case WM_PAINT:
            return HEADER_Paint (hwnd, wParam);

        case WM_RBUTTONUP:
            return HEADER_OnRButtonUp(hwnd, (UINT)wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));

        case WM_SETCURSOR:
            return HEADER_SetCursor (hwnd, wParam, lParam);

        case WM_SETFONT:
            return HEADER_SetFont (hwnd, wParam, lParam);

        default:
	    return DefWindowProcA (hwnd, msg, wParam, lParam);
    }
}

ATOM Register_HeaderCtrl()
{
	WNDCLASS wc;
	memset(&wc, 0, sizeof(wc));
	wc.style         = CS_GLOBALCLASS | CS_DBLCLKS;
	wc.lpfnWndProc   = HEADER_WindowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(HEADER_INFO*);
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.lpszClassName = WC_LSC_HEADER;
	return RegisterClass(&wc);
}
