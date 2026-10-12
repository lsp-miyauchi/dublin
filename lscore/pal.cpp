///////////////////////////////////////////////////////
// PALETTE control implementation

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "common.h"
#include "fnc.h"

typedef struct tagPALINFO {
	HPALETTE hpal;
	int entries;
	COLORREF rgb[256];
	DWORD stat[256];
	HFONT hfont;
	int dx;
	int dy;
	int ofx;
	int ofy;
	int sel;
} PALINFO, *LPPALINFO;

#define GETPAL(n)				((LPPALINFO)GetWindowLongPtr((n), 0))
#define SETPAL(n,x)				(SetWindowLongPtr((n), 0, (LONG_PTR)(x)))
#define PLST_SELECT				0x00000001

int __fastcall dib_numcolors(void* pv)
{
	int colors = ((BITMAPINFOHEADER*)pv)->biClrUsed;
	if (!colors)
	{
		int bits = ((BITMAPINFOHEADER*)pv)->biBitCount;
		colors = (bits <= 8) ? (1 << bits) : 0;
	}
	return colors;
}

//////////////////////////////////////////////////////////////////////
//

static void pl_OnSize(HWND hwnd, CSize csz, UINT nFlag);
static void pl_OnLButtonDown(HWND hwnd, UINT fKeys, CPoint pt);
static void pl_OnLButtonDblClk(HWND hwnd, UINT fKeys, CPoint pt);
static BOOL pl_GetItemRect(HWND hwnd, int index, CRect& rrc);
static int pl_GetItemIndex(HWND hwnd, CPoint pt);
static void pl_RedrawItem(HWND hwnd, int index);
static void pl_OnPaint(HWND hwnd);
static void pl_OnPaletteChanged(HWND hwnd, HWND hwndFocus);
static LRESULT pl_OnSetPalette(HWND hwnd, HPALETTE hpal, BOOL bRedraw);
static LRESULT pl_OnSetDibPalette(HWND hwnd, BITMAPINFO* pbi, BOOL bRedraw);
static BOOL pl_OnQueryNewPalette(HWND hwnd);
static void pl_OnDestroy(HWND hwnd);
static int pl_OnGetCount(HWND hwnd);
static int pl_OnGetSelCount(HWND hwnd);
static int pl_OnGetSelItems(HWND hwnd, int limits, int* pselarr);
static int pl_OnSetSelItems(HWND hwnd, int entries, int* pselarr);
static BOOL pl_OnGetColor(HWND hwnd, int index, COLORREF* pcr);
static COLORREF pl_OnSetColor(HWND hwnd, int index, COLORREF cr);
static int pl_OnGetCurSel(HWND hwnd);
static int pl_OnSetCurSel(HWND hwnd, int index);


extern "C"
LRESULT CALLBACK PalWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CREATE:
			SETPAL(hwnd, 0);
			return 0;

		case WM_SIZE:
			pl_OnSize(hwnd, CSize(LOWORD(lParam), HIWORD(lParam)), (UINT)wParam);
			return 0;

		case WM_LBUTTONDOWN:
			pl_OnLButtonDown(hwnd, (UINT)wParam, CPoint((DWORD)lParam));
			return 0;

		case WM_LBUTTONDBLCLK:
			pl_OnLButtonDblClk(hwnd, (UINT)wParam, CPoint((DWORD)lParam));
			return 0;

		case WM_PAINT:
			pl_OnPaint(hwnd);
			return 0;

		case WM_PALETTECHANGED:
			pl_OnPaletteChanged(hwnd, (HWND)wParam);
			return 0;

		case WM_QUERYNEWPALETTE:
			return (LRESULT)pl_OnQueryNewPalette(hwnd);

		case WM_GETDLGCODE:
			return DLGC_STATIC;

		case WM_DESTROY:
			pl_OnDestroy(hwnd);
			break;

		case PLTM_SETPALETTE:
			return pl_OnSetPalette(hwnd, (HPALETTE)lParam, (BOOL)wParam);

		case PLTM_SETDIBPALETTE:
			return pl_OnSetDibPalette(hwnd, (BITMAPINFO*)lParam, (BOOL)wParam);

		case PLTM_GETCOUNT:
			return (LRESULT)pl_OnGetCount(hwnd);

		case PLTM_GETSELCOUNT:
			return (LRESULT)pl_OnGetSelCount(hwnd);

		case PLTM_GETSELITEMS:
			return (LRESULT)pl_OnGetSelItems(hwnd, (int)wParam, (int*)lParam);

		case PLTM_SETSELITEMS:
			return (LRESULT)pl_OnSetSelItems(hwnd, (int)wParam, (int*)lParam);

		case PLTM_GETCOLOR:
			return (LRESULT)pl_OnGetColor(hwnd, (int)wParam, (COLORREF*)lParam);

		case PLTM_SETCOLOR:
			return (LRESULT)pl_OnSetColor(hwnd, (int)wParam, (COLORREF)lParam);

		case PLTM_GETCURSEL:
			return (LRESULT)pl_OnGetCurSel(hwnd);

		case PLTM_SETCURSEL:
			return pl_OnSetCurSel(hwnd, (int)wParam);

		default:
			break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void pl_OnSize(HWND hwnd, CSize csz, UINT nFlag)
{
	LPPALINFO ppi = GETPAL(hwnd);
	if (ppi && nFlag != SIZE_MINIMIZED)
	{
		CRect rc;
		GetClientRect(hwnd, &rc);
		ppi->dx = rc.Width() >> 4;
		ppi->dy = rc.Height() >> 4;
		ppi->ofx = (rc.Width() - ((ppi->dx << 4) + 1)) >> 1;
		ppi->ofy = (rc.Height() - ((ppi->dy << 4) + 1)) >> 1;
		InvalidateRect(hwnd, NULL, TRUE);
	}
}

static BOOL pl_GetItemRect(HWND hwnd, int index, CRect& rrc)
{
	LPPALINFO ppi = GETPAL(hwnd);
	if (ppi && index >= 0 && index < ppi->entries)
	{
		int x = index & 0xF;
		int y = index >> 4;
		rrc.left = x * ppi->dx;
		rrc.top = y * ppi->dy;
		rrc.right = rrc.left + ppi->dx;
		rrc.bottom = rrc.top + ppi->dy;
		rrc.OffsetRect(ppi->ofx, ppi->ofy);
		return TRUE;
	}
	return FALSE;
}

static int pl_GetItemIndex(HWND hwnd, CPoint pt)
{
	LPPALINFO ppi = GETPAL(hwnd);
	if (ppi)
	{
		CRect rc;
		int x, y;
		for (int i = 0; i < ppi->entries; i++)
		{
			x = i & 0xF;
			y = i >> 4;
			rc.left = x * ppi->dx;
			rc.top = y * ppi->dy;
			rc.right = rc.left + ppi->dx;
			rc.bottom = rc.top + ppi->dy;
			rc.OffsetRect(ppi->ofx, ppi->ofy);
			if (rc.PtInRect(pt))
				return i;
		}
	}
	return -1;
}

static void pl_RedrawItem(HWND hwnd, int index)
{
	CRect rc;
	if (pl_GetItemRect(hwnd, index, rc))
		RedrawWindow(hwnd, &rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

static void pl_OnLButtonDown(HWND hwnd, UINT fKeys, CPoint pt)
{
	int index = pl_GetItemIndex(hwnd, pt);
	if (index != -1)
	{
		LPPALINFO ppi = GETPAL(hwnd);
		PALNOTIFY pn;
		pn.hdr.hwndFrom = hwnd;
		pn.hdr.idFrom = GetDlgCtrlID(hwnd);
		pn.hdr.code = PLTN_CLICK;
		pn.nIndex = index;
		pn.rgb = ppi->rgb[index];
		SendMessage(GetParent(hwnd), WM_NOTIFY,
					(WPARAM)GetDlgCtrlID(hwnd), (LPARAM)&pn);

		if (GetWindowStyle(hwnd) & PLTS_SELMULTIPLE)
		{
			if (ppi->stat[index] & PLST_SELECT)
				ppi->stat[index] &= ~PLST_SELECT;
			else
				ppi->stat[index] |= PLST_SELECT;
		}
		else
		{
			int oldsel = ppi->sel;
			ppi->sel = index;
			if (oldsel != -1)
				pl_RedrawItem(hwnd, oldsel);
		}
		pl_RedrawItem(hwnd, index);
		pn.hdr.code = PLTN_SELCHANGED;
		pn.nIndex = index;
		pn.rgb = ppi->rgb[index];
		SendMessage(GetParent(hwnd), WM_NOTIFY,
					(WPARAM)GetDlgCtrlID(hwnd), (LPARAM)&pn);
	}
}

static void pl_OnLButtonDblClk(HWND hwnd, UINT fKeys, CPoint pt)
{
	int index = pl_GetItemIndex(hwnd, pt);
	if (index != -1)
	{
		PALNOTIFY pn;
		pn.hdr.code = PLTN_DBLCLICK;

		LPPALINFO ppi = GETPAL(hwnd);
		if (GetWindowStyle(hwnd) & PLTS_ALLOWEDIT)
		{
			COLORREF rgb;
			if (lccSelectCustomColor(hwnd, ppi->rgb[index], &rgb))
			{
				ppi->rgb[index] = rgb;
				pl_RedrawItem(hwnd, index);
				pn.hdr.code = PLTN_PALETTECHANGED;
			}
			else
				return;
		}

		pn.hdr.hwndFrom = hwnd;
		pn.hdr.idFrom = GetDlgCtrlID(hwnd);
		pn.nIndex = index;
		pn.rgb = ppi->rgb[index];
		SendMessage(GetParent(hwnd), WM_NOTIFY,
					(WPARAM)GetDlgCtrlID(hwnd), (LPARAM)&pn);
	}
}

static void pl_OnPaint(HWND hwnd)
{
	LPPALINFO ppi = GETPAL(hwnd);

	CPaintDC dc(hwnd);
	if (ppi && ppi->hpal)
	{
		HPALETTE hpalOld = SelectPalette(dc, ppi->hpal, TRUE);
		RealizePalette(dc);

		BOOL bMulti = (GetWindowStyle(hwnd) & PLTS_SELMULTIPLE);
		int x, y;
		CRect rc;
		for (int i = 0; i < ppi->entries; i++)
		{
			x = i & 0xF;
			y = i >> 4;
			rc.left = x * ppi->dx;
			rc.top = y * ppi->dy;
			rc.right = rc.left + ppi->dx;
			rc.bottom = rc.top + ppi->dy;
			rc.OffsetRect(ppi->ofx, ppi->ofy);

			rc.right++;
			rc.bottom++;

			if ((bMulti && ppi->stat[i] & PLST_SELECT) || (ppi->sel == i))
			{
				draw_rect(dc, &rc, RGB(0, 0, 0), (COLORREF)-1);
				rc.InflateRect(-1, -1);
				draw_rect(dc, &rc, RGB(255, 255, 255), (COLORREF)-1);
				rc.InflateRect(-1, -1);
				draw_rect(dc, &rc, RGB(0, 0, 0), ppi->rgb[i]);
			}
			else
				draw_rect(dc, &rc, RGB(0, 0, 0), ppi->rgb[i]);
		}
		SelectPalette(dc, hpalOld, FALSE);
	}
}

static void pl_OnPaletteChanged(HWND hwnd, HWND hwndFocus)
{
	LPPALINFO ppi = GETPAL(hwnd);
	if (ppi && ppi->hpal)
	{
		if (hwnd != hwndFocus)
		{
			CClientDC dc(hwnd);
			HPALETTE hpalOld = SelectPalette(dc, ppi->hpal, FALSE);
			RealizePalette(dc);
			UpdateColors(dc);
			SelectPalette(dc, hpalOld, FALSE);
		}
	}
}

static BOOL pl_OnQueryNewPalette(HWND hwnd)
{
	LPPALINFO ppi = GETPAL(hwnd);
	if (ppi && ppi->hpal)
	{
		CClientDC dc(hwnd);
		HPALETTE hpalOld = SelectPalette(dc, ppi->hpal, FALSE);
		RealizePalette(dc);
		InvalidateRect(hwnd, NULL, TRUE);
		SelectPalette(dc, hpalOld, FALSE);
		return TRUE;
	}
	return FALSE;
}

static void pl_OnDestroy(HWND hwnd)
{
	LPPALINFO ppi = GETPAL(hwnd);
	if (ppi) free(ppi);
}

static LRESULT pl_OnSetPalette(HWND hwnd, HPALETTE hpal, BOOL bRedraw)
{
	LPPALINFO ppi = GETPAL(hwnd);
	if (ppi) {
		free(ppi);
		ppi = NULL;
	}

	if (hpal) {
		ppi = (LPPALINFO)malloc(sizeof(PALINFO));
		memset(ppi, 0, sizeof(PALINFO));
		ppi->sel = -1;

		CRect rc;
		GetClientRect(hwnd, &rc);
		ppi->dx = rc.Width() >> 4;
		ppi->dy = rc.Height() >> 4;
		ppi->ofx = (rc.Width() - ((ppi->dx << 4) + 1)) >> 1;
		ppi->ofy = (rc.Height() - ((ppi->dy << 4) + 1)) >> 1;

		WORD cnt;
		GetObject(hpal, sizeof(WORD), &cnt);
		LPLOGPALETTE plp = (LPLOGPALETTE)malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * cnt);
		if (plp) {
			PALETTEENTRY* ppe = (PALETTEENTRY*)((BYTE*)plp + sizeof(LOGPALETTE));
			GetPaletteEntries(hpal, 0, cnt, ppe);
			for (int i = 0; i < (int)cnt; i++)
				ppi->rgb[i] = RGB(ppe[i].peRed, ppe[i].peGreen, ppe[i].peBlue);
			free(plp);
		}
		ppi->entries = (int)cnt;
		ppi->hpal = hpal;
	}

	SETPAL(hwnd, ppi);
	if (bRedraw)
		InvalidateRect(hwnd, NULL, TRUE);
	return 0;
}

static LRESULT pl_OnSetDibPalette(HWND hwnd, BITMAPINFO* pbi, BOOL bRedraw)
{
	int cnt;
	if (!pbi || (cnt = dib_numcolors(pbi)) < 1)
		return 0;

	LPPALINFO ppi = GETPAL(hwnd);
	if (ppi) {
		free(ppi);
		ppi = NULL;
	}

	ppi = (LPPALINFO)malloc(sizeof(PALINFO));
	memset(ppi, 0, sizeof(PALINFO));
	ppi->sel = -1;

	CRect rc;
	GetClientRect(hwnd, &rc);
	ppi->dx = rc.Width() >> 4;
	ppi->dy = rc.Height() >> 4;
	ppi->ofx = (rc.Width() - ((ppi->dx << 4) + 1)) >> 1;
	ppi->ofy = (rc.Height() - ((ppi->dy << 4) + 1)) >> 1;

	LOGPALETTE* plp = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * cnt);
	PALETTEENTRY* ppe = (PALETTEENTRY*)((BYTE*)plp + sizeof(LOGPALETTE));
	RGBQUAD* pq = pbi->bmiColors;
	for (int i = 0; i < cnt; i++) {
		ppe[i].peRed = pq[i].rgbRed;
		ppe[i].peGreen = pq[i].rgbGreen;
		ppe[i].peBlue = pq[i].rgbBlue;
		ppe[i].peFlags = 0;
		ppi->rgb[i] = RGB(pq[i].rgbRed, pq[i].rgbGreen, pq[i].rgbBlue);
	}
	ppi->entries = (int)cnt;
	ppi->hpal = CreatePalette(plp);
	free(plp);

	SETPAL(hwnd, ppi);
	if (bRedraw)
		InvalidateRect(hwnd, NULL, TRUE);
	return 0;
}

static int pl_OnGetCount(HWND hwnd)
{
	LPPALINFO ppi = GETPAL(hwnd);
	if (ppi)
		return ppi->entries;
	return -1;
}

static int pl_OnGetSelCount(HWND hwnd)
{
	LPPALINFO ppi = GETPAL(hwnd);
	if (ppi && GetWindowStyle(hwnd) & PLTS_SELMULTIPLE)
	{
		int sel = 0;
		for (int i = 0; i < ppi->entries; i++)
		{
			if (ppi->stat[i] & PLST_SELECT)
				sel++;
		}
		return sel;
	}
	return -1;
}

static int pl_OnGetSelItems(HWND hwnd, int limits, int* pselarr)
{
	LPPALINFO ppi = GETPAL(hwnd);
	if (ppi && GetWindowStyle(hwnd) & PLTS_SELMULTIPLE)
	{
		limits = min(ppi->entries, limits);
		int sel = 0;
		for (int i = 0; i < ppi->entries; i++)
		{
			if (ppi->stat[i] & PLST_SELECT)
			{
				if (pselarr)
					pselarr[sel] = i;
				sel++;
			}
		}
		return sel;
	}
	return -1;
}

static int pl_OnSetSelItems(HWND hwnd, int entries, int* pselarr)
{
	LPPALINFO ppi = GETPAL(hwnd);
	if (ppi &&
		GetWindowStyle(hwnd) & PLTS_SELMULTIPLE &&
		entries > 0 &&
		pselarr)
	{
		int changeditem = 0;
		for (int i = 0; i < entries; i++)
		{
			int index = pselarr[i];
			if (index >= 0 &&
				index < ppi->entries)
			{
				if (!(ppi->stat[index] & PLST_SELECT))
				{
					ppi->stat[index] |= PLST_SELECT;
					pl_RedrawItem(hwnd, index);
				}
				changeditem++;
			}
		}
		return changeditem;
	}
	return -1;
}

static BOOL pl_OnGetColor(HWND hwnd, int index, COLORREF* pcr)
{
	LPPALINFO ppi = GETPAL(hwnd);
	if (ppi && index >= 0 && index < ppi->entries)
	{
		*pcr = ppi->rgb[index];
		return TRUE;
	}
	return FALSE;
}

static COLORREF pl_OnSetColor(HWND hwnd, int index, COLORREF cr)
{
	LPPALINFO ppi = GETPAL(hwnd);
	if (ppi && index >= 0 && index < ppi->entries)
	{
		COLORREF crOld = ppi->rgb[index];
		ppi->rgb[index] = cr;
		pl_RedrawItem(hwnd, index);
		return crOld;
	}
	return CLR_NONE;
}

static int pl_OnGetCurSel(HWND hwnd)
{
	LPPALINFO ppi = GETPAL(hwnd);
	return ppi ? ppi->sel : -1;
}

static int pl_OnSetCurSel(HWND hwnd, int index)
{
	LPPALINFO ppi = GETPAL(hwnd);
	if (ppi && index >= 0 && index < ppi->entries)
	{
		if (ppi->sel != index)
		{
			int oldsel = ppi->sel;
			ppi->sel = index;
			if (oldsel != -1)
				pl_RedrawItem(hwnd, oldsel);
		}
		pl_RedrawItem(hwnd, ppi->sel);
		return index;
	}
	return -1;
}
