/*
 * Interface code to StatusWindow widget/control
 *
 * Copyright 1996 Bruce Milner
 * Copyright 1998, 1999 Eric Kohl
 * Copyright 2002 Dimitrie O. Paun
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * NOTE
 * 
 * This code was audited for completeness against the documented features
 * of Comctl32.dll version 6.0 on Sep. 24, 2002, by Dimitrie O. Paun.
 * 
 * Unless otherwise noted, we believe this code to be complete, as per
 * the specification mentioned above.
 * If you discover missing features, or bugs, please note them below.
 * 
 * TODO:
 * 	-- CCS_BOTTOM (default)
 * 	-- CCS_LEFT
 * 	-- CCS_NODEVIDER
 * 	-- CCS_NOMOVEX
 * 	-- CCS_NOMOVEY
 * 	-- CCS_NOPARENTALIGN
 * 	-- CCS_RIGHT
 * 	-- CCS_TOP
 * 	-- CCS_VERT (defaults to RIGHT)
 */

#include <stdarg.h>
#include <string.h>

#include "windef.h"
#include "winbase.h"
#include "wine/unicode.h"
#include "wingdi.h"
#include "winuser.h"
#include "winnls.h"
#include "commctrl.h"
#include "comctl32.h"
#include "uxtheme.h"
#include "tmschema.h"
#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(statusbar);

typedef struct {
    INT 	x;
    INT 	style;
    RECT	bound;
    LPWSTR	text;
    HICON       hIcon;
} STATUSWINDOWPART;

typedef struct {
    HWND              m_hWnd;
    HWND              m_hWndNotify;
    WORD              numParts;
    UINT              height;
    BOOL              simple;
    HWND              m_hWndTip;
    HFONT             hFont;
    HFONT             hDefaultFont;
    COLORREF          clrBk;		/* background color */
    BOOL              bUnicode;		/* unicode flag */
    BOOL              NtfUnicode;	/* notify format */
    STATUSWINDOWPART  part0;		/* simple window */
    STATUSWINDOWPART* parts;
    INT               horizontalBorder;
    INT               verticalBorder;
    INT               horizontalGap;
} STATUS_INFO;

/*
 * Run tests using Waite Group Windows95 API Bible Vol. 1&2
 * The second cdrom contains executables drawstat.exe, gettext.exe,
 * simple.exe, getparts.exe, setparts.exe, statwnd.exe
 */

#define HORZ_BORDER 0
#define VERT_BORDER 2
#define HORZ_GAP    2

static const WCHAR themeClass[] = { 'S','t','a','t','u','s',0 };

/* prototype */
static void
STATUSBAR_SetPartBounds (STATUS_INFO *infoPtr);

static inline LPCSTR debugstr_t(LPCWSTR text, BOOL isW)
{
  return isW ? debugstr_w(text) : debugstr_a((LPCSTR)text);
}

static void STATUSBAR_DrawSizeGrip(HTHEME theme, HDC hdc, LPRECT lpRect)
{
	CDCHandle dc = hdc;

	if (theme) {
		RECT gripperRect;
		SIZE gripperSize;
		gripperRect = *lpRect;
		if (SUCCEEDED (GetThemePartSize (theme, hdc, SP_GRIPPER, 0, lpRect, TS_DRAW, &gripperSize))) {
			gripperRect.left = gripperRect.right - gripperSize.cx;
			gripperRect.top = gripperRect.bottom - gripperSize.cy;
			if (SUCCEEDED (DrawThemeBackground(theme, hdc, SP_GRIPPER, 0, &gripperRect, NULL)))
				return;
		}
	}

	POINT pt;
	pt.x = lpRect->right - 1;
	pt.y = lpRect->bottom - 1;

	CPen pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DFACE));
	HPEN hOldPen = dc.SelectPen(pen);
	dc.MoveTo(pt.x - 12, pt.y);
	dc.LineTo(pt.x, pt.y);
	dc.LineTo(pt.x, pt.y - 13);

	pt.x--;
	pt.y--;

	pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
	dc.SelectPen(pen);
	int i;
	for (i = 1; i < 11; i += 4) {
		dc.MoveTo(pt.x - i, pt.y);
		dc.LineTo(pt.x + 1, pt.y - i - 1);

		dc.MoveTo(pt.x - i - 1, pt.y);
		dc.LineTo(pt.x + 1, pt.y - i - 2);
	}

	pen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DHIGHLIGHT));
	dc.SelectPen(pen);
	for (i = 3; i < 13; i += 4) {
		dc.MoveTo(pt.x - i, pt.y);
		dc.LineTo(pt.x + 1, pt.y - i - 1);
	}

	dc.SelectPen(hOldPen);
}


static void STATUSBAR_DrawPart (STATUS_INFO *infoPtr, HDC hdc, STATUSWINDOWPART *part, int itemID)
{
	CDCHandle dc = hdc;

	CRect rc = part->bound;
	UINT border = BDR_SUNKENOUTER;
	HTHEME theme = GetWindowTheme(infoPtr->m_hWnd);
	int themePart = SP_PANE;

	if (part->style & SBT_POPOUT)
		border = BDR_RAISEDOUTER;
	else if (part->style & SBT_NOBORDERS)
		border = 0;

	if (theme) {
		if ((GetWindowLongW (infoPtr->m_hWnd, GWL_STYLE) & SBARS_SIZEGRIP) && (infoPtr->simple || (itemID == (infoPtr->numParts - 1))))
			themePart = SP_GRIPPERPANE;
		DrawThemeBackground(theme, hdc, themePart, 0, &rc, NULL);
	}
	else
		dc.DrawEdge(&rc, border, BF_RECT|BF_ADJUST);

	if (part->style & SBT_OWNERDRAW) {
		DRAWITEMSTRUCT dis;
		dis.CtlID = GetWindowLongPtrW (infoPtr->m_hWnd, GWLP_ID);
		dis.itemID = itemID;
		dis.hwndItem = infoPtr->m_hWnd;
		dis.hDC = dc;
		dis.rcItem = rc;
		dis.itemData = (ULONG_PTR)part->text;
		SendMessageW(infoPtr->m_hWndNotify, WM_DRAWITEM, (WPARAM)dis.CtlID, (LPARAM)&dis);
	}
	else {
		if (part->hIcon) {
			int cy = rc.Height();
			rc.left += 2;
			dc.DrawIconEx(rc.left, rc.top, part->hIcon, cy, cy, 0, 0, DI_NORMAL);
			rc.left += cy;
		}
		DrawStatusText(dc, &rc, part->text, SBT_NOBORDERS);
    }
}

static void STATUSBAR_RefreshPart (STATUS_INFO *infoPtr, HDC hdc, STATUSWINDOWPART *part, int itemID)
{
    HBRUSH hbrBk;
    HFONT  hOldFont;
    HTHEME theme;

    TRACE("item %d\n", itemID);
    if (!IsWindowVisible (infoPtr->m_hWnd))
        return;

    if (part->bound.right < part->bound.left) return;

	if ((theme = GetWindowTheme (infoPtr->m_hWnd))) {
        RECT cr;
        GetClientRect (infoPtr->m_hWnd, &cr);
        DrawThemeBackground(theme, hdc, 0, 0, &cr, &part->bound);
    }
	else {
        if (infoPtr->clrBk != CLR_DEFAULT)
                hbrBk = CreateSolidBrush (infoPtr->clrBk);
        else
                hbrBk = GetSysColorBrush (COLOR_3DFACE);
        FillRect(hdc, &part->bound, hbrBk);
        if (infoPtr->clrBk != CLR_DEFAULT)
                DeleteObject (hbrBk);
    }

    hOldFont = SelectObject (hdc, infoPtr->hFont ? infoPtr->hFont : infoPtr->hDefaultFont);

	STATUSBAR_DrawPart (infoPtr, hdc, part, itemID);

    SelectObject (hdc, hOldFont);

    if (GetWindowLongW (infoPtr->m_hWnd, GWL_STYLE) & SBARS_SIZEGRIP) {
        RECT rect;

	GetClientRect (infoPtr->m_hWnd, &rect);
	STATUSBAR_DrawSizeGrip (theme, hdc, &rect);
    }
}


static LRESULT
STATUSBAR_Refresh (STATUS_INFO *infoPtr, HDC hdc)
{
    if (!IsWindowVisible(infoPtr->m_hWnd))
        return 0;

	STATUSBAR_SetPartBounds(infoPtr);

	CRect rc;
	GetClientRect(infoPtr->m_hWnd, &rc);

    HTHEME theme = GetWindowTheme(infoPtr->m_hWnd);
	if (theme)
		DrawThemeBackground(theme, hdc, 0, 0, &rc, NULL);
    else {
		HBRUSH hbr;
		if (infoPtr->clrBk != CLR_DEFAULT)
			hbr = CreateSolidBrush(infoPtr->clrBk);
        else
			hbr = GetSysColorBrush(COLOR_3DFACE);
		FillRect(hdc, &rc, hbr);
		if (infoPtr->clrBk != CLR_DEFAULT)
			DeleteObject(hbr);
    }

	HFONT hOldFont = SelectObject(hdc, infoPtr->hFont ? infoPtr->hFont : infoPtr->hDefaultFont);

	if (infoPtr->simple)
		STATUSBAR_RefreshPart (infoPtr, hdc, &infoPtr->part0, 0);
	else {
		for (int i = 0; i < infoPtr->numParts; i++)
			STATUSBAR_RefreshPart (infoPtr, hdc, &infoPtr->parts[i], i);
	}

	SelectObject(hdc, hOldFont);

	if (GetWindowLongW(infoPtr->m_hWnd, GWL_STYLE) & SBARS_SIZEGRIP)
		STATUSBAR_DrawSizeGrip(theme, hdc, &rc);

	return 0;
}


static void
STATUSBAR_SetPartBounds (STATUS_INFO *infoPtr)
{
    STATUSWINDOWPART *part;
    RECT rect, *r;
    int	i;

    /* get our window size */
    GetClientRect (infoPtr->m_hWnd, &rect);
    TRACE("client wnd size is %d,%d - %d,%d\n", rect.left, rect.top, rect.right, rect.bottom);

    rect.left += infoPtr->horizontalBorder;
    rect.top += infoPtr->verticalBorder;

    /* set bounds for simple rectangle */
    infoPtr->part0.bound = rect;

    /* set bounds for non-simple rectangles */
    for (i = 0; i < infoPtr->numParts; i++) {
	part = &infoPtr->parts[i];
	r = &infoPtr->parts[i].bound;
	r->top = rect.top;
	r->bottom = rect.bottom;
	if (i == 0)
	    r->left = 0;
	else
	    r->left = infoPtr->parts[i-1].bound.right + infoPtr->horizontalGap;
	if (part->x == -1)
	    r->right = rect.right;
	else
	    r->right = part->x;

	if (infoPtr->m_hWndTip) {
	    TTTOOLINFOW ti;

	    ti.cbSize = sizeof(TTTOOLINFOW);
	    ti.hwnd = infoPtr->m_hWnd;
	    ti.uId = i;
	    ti.rect = *r;
	    SendMessageW (infoPtr->m_hWndTip, TTM_NEWTOOLRECTW,
			    0, (LPARAM)&ti);
	}
    }
}


static LRESULT
STATUSBAR_Relay2Tip (STATUS_INFO *infoPtr, UINT uMsg,
		     WPARAM wParam, LPARAM lParam)
{
    MSG msg;

    msg.hwnd = infoPtr->m_hWnd;
    msg.message = uMsg;
    msg.wParam = wParam;
    msg.lParam = lParam;
    msg.time = GetMessageTime ();
    msg.pt.x = (short)LOWORD(GetMessagePos ());
    msg.pt.y = (short)HIWORD(GetMessagePos ());

    return SendMessageW (infoPtr->m_hWndTip, TTM_RELAYEVENT, 0, (LPARAM)&msg);
}


static BOOL
STATUSBAR_GetBorders (STATUS_INFO *infoPtr, INT out[])
{
    TRACE("\n");
    out[0] = infoPtr->horizontalBorder;
    out[1] = infoPtr->verticalBorder;
    out[2] = infoPtr->horizontalGap;

    return TRUE;
}


static BOOL
STATUSBAR_SetBorders (STATUS_INFO *infoPtr, INT in[])
{
    TRACE("\n");
    infoPtr->horizontalBorder = in[0];
    infoPtr->verticalBorder = in[1];
    infoPtr->horizontalGap = in[2];
    InvalidateRect(infoPtr->m_hWnd, NULL, FALSE);

    return TRUE;
}


static HICON
STATUSBAR_GetIcon (STATUS_INFO *infoPtr, INT nPart)
{
    TRACE("%d\n", nPart);
    /* MSDN says: "simple parts are indexed with -1" */
    if ((nPart < -1) || (nPart >= infoPtr->numParts))
	return 0;

    if (nPart == -1)
        return (infoPtr->part0.hIcon);
    else
        return (infoPtr->parts[nPart].hIcon);
}


static INT
STATUSBAR_GetParts (STATUS_INFO *infoPtr, INT num_parts, INT parts[])
{
    INT   i;

    TRACE("(%d)\n", num_parts);
    if (parts) {
	for (i = 0; i < num_parts; i++) {
	    parts[i] = infoPtr->parts[i].x;
	}
    }
    return infoPtr->numParts;
}


static BOOL
STATUSBAR_GetRect (STATUS_INFO *infoPtr, INT nPart, LPRECT rect)
{
    TRACE("part %d\n", nPart);
    if(nPart >= infoPtr->numParts || nPart < 0)
      return FALSE;
    if (infoPtr->simple)
	*rect = infoPtr->part0.bound;
    else
	*rect = infoPtr->parts[nPart].bound;
    return TRUE;
}


static LRESULT
STATUSBAR_GetTextA (STATUS_INFO *infoPtr, INT nPart, LPSTR buf)
{
    STATUSWINDOWPART *part;
    LRESULT result;

    TRACE("part %d\n", nPart);

    /* MSDN says: "simple parts use index of 0", so this check is ok. */
    if (nPart < 0 || nPart >= infoPtr->numParts) return 0;

    if (infoPtr->simple)
	part = &infoPtr->part0;
    else
	part = &infoPtr->parts[nPart];

    if (part->style & SBT_OWNERDRAW)
	result = (LRESULT)part->text;
    else {
        DWORD len = part->text ? WideCharToMultiByte( CP_ACP, 0, part->text, -1,
                                                      NULL, 0, NULL, NULL ) - 1 : 0;
        result = MAKELONG( len, part->style );
        if (part->text && buf)
            WideCharToMultiByte( CP_ACP, 0, part->text, -1, buf, len+1, NULL, NULL );
    }
    return result;
}


static LRESULT
STATUSBAR_GetTextW (STATUS_INFO *infoPtr, INT nPart, LPWSTR buf)
{
    STATUSWINDOWPART *part;
    LRESULT result;

    TRACE("part %d\n", nPart);
    if (nPart < 0 || nPart >= infoPtr->numParts) return 0;

    if (infoPtr->simple)
	part = &infoPtr->part0;
    else
	part = &infoPtr->parts[nPart];

    if (part->style & SBT_OWNERDRAW)
	result = (LRESULT)part->text;
    else {
	result = part->text ? strlenW (part->text) : 0;
	result |= (part->style << 16);
	if (part->text && buf)
	    strcpyW (buf, part->text);
    }
    return result;
}


static LRESULT
STATUSBAR_GetTextLength (STATUS_INFO *infoPtr, INT nPart)
{
    STATUSWINDOWPART *part;
    DWORD result;

    TRACE("part %d\n", nPart);

    /* MSDN says: "simple parts use index of 0", so this check is ok. */
    if (nPart < 0 || nPart >= infoPtr->numParts) return 0;

    if (infoPtr->simple)
	part = &infoPtr->part0;
    else
	part = &infoPtr->parts[nPart];

    if ((~part->style & SBT_OWNERDRAW) && part->text)
	result = strlenW(part->text);
    else
	result = 0;

    result |= (part->style << 16);
    return result;
}

static LRESULT
STATUSBAR_GetTipTextA (STATUS_INFO *infoPtr, INT id, LPSTR tip, INT size)
{
    TRACE("\n");
    if (tip) {
        CHAR buf[INFOTIPSIZE];
        buf[0]='\0';

        if (infoPtr->m_hWndTip) {
            TTTOOLINFOA ti;
            ti.cbSize = sizeof(TTTOOLINFOA);
            ti.hwnd = infoPtr->m_hWnd;
            ti.uId = id;
            ti.lpszText = buf;
            SendMessageA (infoPtr->m_hWndTip, TTM_GETTEXTA, 0, (LPARAM)&ti);
        }
        lstrcpynA (tip, buf, size);
    }
    return 0;
}


static LRESULT
STATUSBAR_GetTipTextW (STATUS_INFO *infoPtr, INT id, LPWSTR tip, INT size)
{
    TRACE("\n");
    if (tip) {
        WCHAR buf[INFOTIPSIZE];
        buf[0]=0;

	if (infoPtr->m_hWndTip) {
	    TTTOOLINFOW ti;
	    ti.cbSize = sizeof(TTTOOLINFOW);
	    ti.hwnd = infoPtr->m_hWnd;
	    ti.uId = id;
            ti.lpszText = buf;
	    SendMessageW(infoPtr->m_hWndTip, TTM_GETTEXTW, 0, (LPARAM)&ti);
	}
	lstrcpynW(tip, buf, size);
    }

    return 0;
}


static COLORREF
STATUSBAR_SetBkColor (STATUS_INFO *infoPtr, COLORREF color)
{
    COLORREF oldBkColor;

    oldBkColor = infoPtr->clrBk;
    infoPtr->clrBk = color;
    InvalidateRect(infoPtr->m_hWnd, NULL, FALSE);

    TRACE("CREF: %08x -> %08x\n", oldBkColor, infoPtr->clrBk);
    return oldBkColor;
}


static BOOL
STATUSBAR_SetIcon (STATUS_INFO *infoPtr, INT nPart, HICON hIcon)
{
    if ((nPart < -1) || (nPart >= infoPtr->numParts))
	return FALSE;

    TRACE("setting part %d\n", nPart);

    /* FIXME: MSDN says "if nPart is -1, the status bar is assumed simple" */
    if (nPart == -1) {
	if (infoPtr->part0.hIcon == hIcon) /* same as - no redraw */
	    return TRUE;
	infoPtr->part0.hIcon = hIcon;
	if (infoPtr->simple)
            InvalidateRect(infoPtr->m_hWnd, &infoPtr->part0.bound, FALSE);
    } else {
	if (infoPtr->parts[nPart].hIcon == hIcon) /* same as - no redraw */
	    return TRUE;

	infoPtr->parts[nPart].hIcon = hIcon;
	if (!(infoPtr->simple))
            InvalidateRect(infoPtr->m_hWnd, &infoPtr->parts[nPart].bound, FALSE);
    }
    return TRUE;
}


static BOOL
STATUSBAR_SetMinHeight (STATUS_INFO *infoPtr, INT height)
{

    TRACE("(height=%d)\n", height);
    if (IsWindowVisible (infoPtr->m_hWnd)) {
	INT  width, x, y;
	RECT parent_rect;
        HTHEME theme;

	GetClientRect (infoPtr->m_hWndNotify, &parent_rect);
	infoPtr->height = height + infoPtr->verticalBorder;
        
        if ((theme = GetWindowTheme (infoPtr->m_hWnd)))
        {
            /* Determine bar height from theme such that the content area is
             * 'height' pixels large */
            HDC hdc = GetDC (infoPtr->m_hWnd);
            RECT r;
            memset (&r, 0, sizeof (r));
            r.bottom = height;
            if (SUCCEEDED(GetThemeBackgroundExtent (theme, hdc, SP_PANE, 0, &r, &r)))
            {
                infoPtr->height = r.bottom - r.top;
            }
            ReleaseDC (infoPtr->m_hWnd, hdc);
        }
        
	width = parent_rect.right - parent_rect.left;
	x = parent_rect.left;
	y = parent_rect.bottom - infoPtr->height;
	MoveWindow (infoPtr->m_hWnd, parent_rect.left,
		    parent_rect.bottom - infoPtr->height,
		    width, infoPtr->height, TRUE);
	STATUSBAR_SetPartBounds (infoPtr);
    }

    return TRUE;
}


static BOOL
STATUSBAR_SetParts (STATUS_INFO *infoPtr, INT count, LPINT parts)
{
    STATUSWINDOWPART *tmp;
    int	i, oldNumParts;

    TRACE("(%d,%p)\n", count, parts);

    oldNumParts = infoPtr->numParts;
    infoPtr->numParts = count;
    if (oldNumParts > infoPtr->numParts) {
	for (i = infoPtr->numParts ; i < oldNumParts; i++) {
	    if (infoPtr->parts[i].text && !(infoPtr->parts[i].style & SBT_OWNERDRAW))
		Free (infoPtr->parts[i].text);
	}
    } else if (oldNumParts < infoPtr->numParts) {
	tmp = Alloc (sizeof(STATUSWINDOWPART) * infoPtr->numParts);
	if (!tmp) return FALSE;
	for (i = 0; i < oldNumParts; i++) {
	    tmp[i] = infoPtr->parts[i];
	}
	if (infoPtr->parts)
	    Free (infoPtr->parts);
	infoPtr->parts = tmp;
    }
    if (oldNumParts == infoPtr->numParts) {
	for (i=0; i < oldNumParts; i++)
	    if (infoPtr->parts[i].x != parts[i])
		break;
	if (i==oldNumParts) /* Unchanged? no need to redraw! */
	    return TRUE;
    }

    for (i = 0; i < infoPtr->numParts; i++)
	infoPtr->parts[i].x = parts[i];

    if (infoPtr->m_hWndTip) {
	INT nTipCount, i;
	TTTOOLINFOW ti;

	ZeroMemory (&ti, sizeof(TTTOOLINFOW));
	ti.cbSize = sizeof(TTTOOLINFOW);
	ti.hwnd = infoPtr->m_hWnd;

	nTipCount = SendMessageW (infoPtr->m_hWndTip, TTM_GETTOOLCOUNT, 0, 0);
	if (nTipCount < infoPtr->numParts) {
	    /* add tools */
	    for (i = nTipCount; i < infoPtr->numParts; i++) {
		TRACE("add tool %d\n", i);
		ti.uId = i;
		SendMessageW (infoPtr->m_hWndTip, TTM_ADDTOOLW,
				0, (LPARAM)&ti);
	    }
	}
	else if (nTipCount > infoPtr->numParts) {
	    /* delete tools */
	    for (i = nTipCount - 1; i >= infoPtr->numParts; i--) {
		TRACE("delete tool %d\n", i);
		ti.uId = i;
		SendMessageW (infoPtr->m_hWndTip, TTM_DELTOOLW,
				0, (LPARAM)&ti);
	    }
	}
    }
    STATUSBAR_SetPartBounds (infoPtr);
    InvalidateRect(infoPtr->m_hWnd, NULL, FALSE);
    return TRUE;
}


static BOOL
STATUSBAR_SetTextT (STATUS_INFO *infoPtr, INT nPart, WORD style,
		    LPCWSTR text, BOOL isW)
{
    STATUSWINDOWPART *part=NULL;
    BOOL changed = FALSE;
    INT  oldStyle;

    if (style & SBT_OWNERDRAW) {
         TRACE("part %d, text %p\n",nPart,text);
    }
    else TRACE("part %d, text %s\n", nPart, debugstr_t(text, isW));

    /* MSDN says: "If the parameter is set to SB_SIMPLEID (255), the status
     * window is assumed to be a simple window */

    if (nPart == 0x00ff) {
	part = &infoPtr->part0;
    } else {
	if (infoPtr->parts && nPart >= 0 && nPart < infoPtr->numParts) {
	    part = &infoPtr->parts[nPart];
	}
    }
    if (!part) return FALSE;

    if (part->style != style)
	changed = TRUE;

    oldStyle = part->style;
    part->style = style;
    if (style & SBT_OWNERDRAW) {
        if (!(oldStyle & SBT_OWNERDRAW)) {
            if (part->text)
                Free (part->text);
        } else if (part->text == text)
            return TRUE;
        part->text = (LPWSTR)text;
    } else {
	LPWSTR ntext;

	if (text && !isW) {
	    LPCSTR atxt = (LPCSTR)text;
            DWORD len = MultiByteToWideChar( CP_ACP, 0, atxt, -1, NULL, 0 );
	    ntext = Alloc( (len + 1)*sizeof(WCHAR) );
	    if (!ntext) return FALSE;
            MultiByteToWideChar( CP_ACP, 0, atxt, -1, ntext, len );
	} else if (text) {
	    ntext = Alloc( (strlenW(text) + 1)*sizeof(WCHAR) );
	    if (!ntext) return FALSE;
	    strcpyW (ntext, text);
	} else ntext = 0;

	/* check if text is unchanged -> no need to redraw */
	if (text) {
	    if (!changed && part->text && !lstrcmpW(ntext, part->text)) {
		Free(ntext);
		return TRUE;
	    }
	} else {
	    if (!changed && !part->text)
		return TRUE;
	}

	if (part->text && !(oldStyle & SBT_OWNERDRAW))
	    Free (part->text);
	part->text = ntext;
    }
    InvalidateRect(infoPtr->m_hWnd, &part->bound, FALSE);
    UpdateWindow(infoPtr->m_hWnd);

    return TRUE;
}


static LRESULT
STATUSBAR_SetTipTextA (STATUS_INFO *infoPtr, INT id, LPSTR text)
{
    TRACE("part %d: \"%s\"\n", id, text);
    if (infoPtr->m_hWndTip) {
	TTTOOLINFOA ti;
	ti.cbSize = sizeof(TTTOOLINFOA);
	ti.hwnd = infoPtr->m_hWnd;
	ti.uId = id;
	ti.hinst = 0;
	ti.lpszText = text;
	SendMessageA (infoPtr->m_hWndTip, TTM_UPDATETIPTEXTA, 0, (LPARAM)&ti);
    }

    return 0;
}


static LRESULT
STATUSBAR_SetTipTextW (STATUS_INFO *infoPtr, INT id, LPWSTR text)
{
    TRACE("part %d: \"%s\"\n", id, debugstr_w(text));
    if (infoPtr->m_hWndTip) {
	TTTOOLINFOW ti;
	ti.cbSize = sizeof(TTTOOLINFOW);
	ti.hwnd = infoPtr->m_hWnd;
	ti.uId = id;
	ti.hinst = 0;
	ti.lpszText = text;
	SendMessageW (infoPtr->m_hWndTip, TTM_UPDATETIPTEXTW, 0, (LPARAM)&ti);
    }

    return 0;
}


inline static LRESULT
STATUSBAR_SetUnicodeFormat (STATUS_INFO *infoPtr, BOOL bUnicode)
{
    BOOL bOld = infoPtr->bUnicode;

    TRACE("(0x%x)\n", bUnicode);
    infoPtr->bUnicode = bUnicode;

    return bOld;
}


static BOOL
STATUSBAR_Simple (STATUS_INFO *infoPtr, BOOL simple)
{
    NMHDR  nmhdr;

    TRACE("(simple=%d)\n", simple);
    if (infoPtr->simple == simple) /* no need to change */
	return TRUE;

    infoPtr->simple = simple;

    /* send notification */
    nmhdr.hwndFrom = infoPtr->m_hWnd;
    nmhdr.idFrom = GetWindowLongPtrW (infoPtr->m_hWnd, GWLP_ID);
    nmhdr.code = SBN_SIMPLEMODECHANGE;
    SendMessageW (infoPtr->m_hWndNotify, WM_NOTIFY, 0, (LPARAM)&nmhdr);
    InvalidateRect(infoPtr->m_hWnd, NULL, FALSE);
    return TRUE;
}


static LRESULT
STATUSBAR_WMDestroy (STATUS_INFO *infoPtr)
{
    int	i;

    TRACE("\n");
    for (i = 0; i < infoPtr->numParts; i++) {
	if (infoPtr->parts[i].text && !(infoPtr->parts[i].style & SBT_OWNERDRAW))
	    Free (infoPtr->parts[i].text);
    }
    if (infoPtr->part0.text && !(infoPtr->part0.style & SBT_OWNERDRAW))
	Free (infoPtr->part0.text);
    Free (infoPtr->parts);

    /* delete default font */
    if (infoPtr->hDefaultFont)
	DeleteObject (infoPtr->hDefaultFont);

    /* delete tool tip control */
    if (infoPtr->m_hWndTip)
	DestroyWindow (infoPtr->m_hWndTip);

    CloseThemeData (GetWindowTheme (infoPtr->m_hWnd));

    SetWindowLongPtrW(infoPtr->m_hWnd, 0, 0);
    Free (infoPtr);
    return 0;
}


static LRESULT
STATUSBAR_WMCreate (HWND hwnd, LPCREATESTRUCTA lpCreate)
{
    STATUS_INFO *infoPtr;
    NONCLIENTMETRICSW nclm;
    DWORD dwStyle;
    RECT rect;
    int	i, width, len, textHeight = 0;
    HDC	hdc;

    TRACE("\n");
    infoPtr = (STATUS_INFO*)Alloc (sizeof(STATUS_INFO));
    if (!infoPtr) goto create_fail;
    SetWindowLongPtrW (hwnd, 0, (DWORD_PTR)infoPtr);

    infoPtr->m_hWnd = hwnd;
    infoPtr->m_hWndNotify = lpCreate->hwndParent;
    infoPtr->numParts = 1;
    infoPtr->parts = 0;
    infoPtr->simple = FALSE;
    infoPtr->clrBk = CLR_DEFAULT;
    infoPtr->hFont = 0;
    infoPtr->horizontalBorder = HORZ_BORDER;
    infoPtr->verticalBorder = VERT_BORDER;
    infoPtr->horizontalGap = HORZ_GAP;

    i = SendMessageW(infoPtr->m_hWndNotify, WM_NOTIFYFORMAT, (WPARAM)hwnd, NF_QUERY);
    infoPtr->NtfUnicode = (i == NFR_UNICODE);

    GetClientRect (hwnd, &rect);
    InvalidateRect (hwnd, &rect, 0);
    UpdateWindow(hwnd);

    ZeroMemory (&nclm, sizeof(nclm));
    nclm.cbSize = sizeof(nclm);
    SystemParametersInfoW (SPI_GETNONCLIENTMETRICS, nclm.cbSize, &nclm, 0);
    infoPtr->hDefaultFont = CreateFontIndirectW (&nclm.lfStatusFont);

    /* initialize simple case */
    infoPtr->part0.bound = rect;
    infoPtr->part0.text = 0;
    infoPtr->part0.x = 0;
    infoPtr->part0.style = 0;
    infoPtr->part0.hIcon = 0;

    /* initialize first part */
    infoPtr->parts = Alloc (sizeof(STATUSWINDOWPART));
    if (!infoPtr->parts) goto create_fail;
    infoPtr->parts[0].bound = rect;
    infoPtr->parts[0].text = 0;
    infoPtr->parts[0].x = -1;
    infoPtr->parts[0].style = 0;
    infoPtr->parts[0].hIcon = 0;
    
    OpenThemeData (hwnd, themeClass);

    if (IsWindowUnicode (hwnd)) {
	infoPtr->bUnicode = TRUE;
	if (lpCreate->lpszName &&
	    (len = strlenW ((LPCWSTR)lpCreate->lpszName))) {
	    infoPtr->parts[0].text = Alloc ((len + 1)*sizeof(WCHAR));
	    if (!infoPtr->parts[0].text) goto create_fail;
	    strcpyW (infoPtr->parts[0].text, (LPCWSTR)lpCreate->lpszName);
	}
    }
    else {
	if (lpCreate->lpszName &&
	    (len = strlen((LPCSTR)lpCreate->lpszName))) {
            DWORD lenW = MultiByteToWideChar( CP_ACP, 0, (LPCSTR)lpCreate->lpszName, -1, NULL, 0 );
	    infoPtr->parts[0].text = Alloc (lenW*sizeof(WCHAR));
	    if (!infoPtr->parts[0].text) goto create_fail;
            MultiByteToWideChar( CP_ACP, 0, (LPCSTR)lpCreate->lpszName, -1,
                                 infoPtr->parts[0].text, lenW );
	}
    }

    dwStyle = GetWindowLongW (hwnd, GWL_STYLE);
    /* native seems to clear WS_BORDER, too */
    dwStyle &= ~WS_BORDER;

    /* statusbars on managed windows should not have SIZEGRIP style */
    if ((dwStyle & SBARS_SIZEGRIP) && lpCreate->hwndParent &&
        GetPropA( lpCreate->hwndParent, "__wine_x11_managed" ))
        dwStyle &= ~SBARS_SIZEGRIP;

    SetWindowLongW (hwnd, GWL_STYLE, dwStyle);

    if ((hdc = GetDC (hwnd))) {
	TEXTMETRICW tm;
	HFONT hOldFont;

	hOldFont = SelectObject (hdc, infoPtr->hDefaultFont);
	GetTextMetricsW (hdc, &tm);
	textHeight = tm.tmHeight;
	SelectObject (hdc, hOldFont);
	ReleaseDC (hwnd, hdc);
    }
    TRACE("    textHeight=%d\n", textHeight);

    if (dwStyle & SBT_TOOLTIPS) {
	infoPtr->m_hWndTip =
	    CreateWindowExW (0, TOOLTIPS_CLASSW, NULL, WS_POPUP | TTS_ALWAYSTIP,
			     CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			     CW_USEDEFAULT, hwnd, 0,
			     (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE), NULL);

	if (infoPtr->m_hWndTip) {
	    NMTOOLTIPSCREATED nmttc;

	    nmttc.hdr.hwndFrom = hwnd;
	    nmttc.hdr.idFrom = GetWindowLongPtrW (hwnd, GWLP_ID);
	    nmttc.hdr.code = NM_TOOLTIPSCREATED;
	    nmttc.hwndToolTips = infoPtr->m_hWndTip;

	    SendMessageW (lpCreate->hwndParent, WM_NOTIFY,
			    (WPARAM)nmttc.hdr.idFrom, (LPARAM)&nmttc);
	}
    }

    if (!(dwStyle & CCS_NORESIZE)) { /* don't resize wnd if it doesn't want it ! */
        HTHEME theme;
        GetClientRect (infoPtr->m_hWndNotify, &rect);
        width = rect.right - rect.left;
        infoPtr->height = textHeight + 4 + infoPtr->verticalBorder;
        
        if ((theme = GetWindowTheme (hwnd)))
        {
            /* Determine bar height from theme such that the content area is
             * textHeight pixels large */
            HDC hdc = GetDC (hwnd);
            RECT r;
            memset (&r, 0, sizeof (r));
            r.bottom = textHeight;
            if (SUCCEEDED(GetThemeBackgroundExtent (theme, hdc, SP_PANE, 0, &r, &r)))
            {
                infoPtr->height = r.bottom - r.top;
            }
            ReleaseDC (hwnd, hdc);
        }
    
        SetWindowPos(hwnd, 0, lpCreate->x, lpCreate->y - 1,
			width, infoPtr->height, SWP_NOZORDER);
        STATUSBAR_SetPartBounds (infoPtr);
    }

    return 0;

create_fail:
    TRACE("    failed!\n");
    if (infoPtr) STATUSBAR_WMDestroy(infoPtr);
    return -1;
}


/* in contrast to SB_GETTEXT*, WM_GETTEXT handles the text
 * of the first part only (usual behaviour) */
static INT
STATUSBAR_WMGetText (STATUS_INFO *infoPtr, INT size, LPWSTR buf)
{
    INT len;

    TRACE("\n");
    if (!(infoPtr->parts[0].text))
        return 0;
    if (infoPtr->bUnicode)
        len = strlenW (infoPtr->parts[0].text);
    else
        len = WideCharToMultiByte( CP_ACP, 0, infoPtr->parts[0].text, -1, NULL, 0, NULL, NULL )-1;

    if (size > len) {
	if (infoPtr->bUnicode)
	    strcpyW (buf, infoPtr->parts[0].text);
	else
            WideCharToMultiByte( CP_ACP, 0, infoPtr->parts[0].text, -1,
                                 (LPSTR)buf, len+1, NULL, NULL );
	return len;
    }

    return -1;
}


static BOOL
STATUSBAR_WMNCHitTest (STATUS_INFO *infoPtr, INT x, INT y)
{
    if (GetWindowLongW (infoPtr->m_hWnd, GWL_STYLE) & SBARS_SIZEGRIP) {
	RECT  rect;
	POINT pt;

	GetClientRect (infoPtr->m_hWnd, &rect);

	pt.x = x;
	pt.y = y;
	ScreenToClient (infoPtr->m_hWnd, &pt);

	rect.left = rect.right - 13;
	rect.top += 2;

	if (PtInRect (&rect, pt))
	    return HTBOTTOMRIGHT;
    }

    return HTERROR;
}


static LRESULT
STATUSBAR_WMPaint (STATUS_INFO *infoPtr, HDC hdc)
{
    PAINTSTRUCT ps;

    TRACE("\n");
    if (hdc) return STATUSBAR_Refresh (infoPtr, hdc);
    hdc = BeginPaint (infoPtr->m_hWnd, &ps);
    STATUSBAR_Refresh (infoPtr, hdc);
    EndPaint (infoPtr->m_hWnd, &ps);

    return 0;
}


static LRESULT
STATUSBAR_WMSetFont (STATUS_INFO *infoPtr, HFONT font, BOOL redraw)
{
    infoPtr->hFont = font;
    TRACE("%p\n", infoPtr->hFont);
    if (redraw)
        InvalidateRect(infoPtr->m_hWnd, NULL, FALSE);

    return 0;
}


static BOOL
STATUSBAR_WMSetText (STATUS_INFO *infoPtr, LPCSTR text)
{
    STATUSWINDOWPART *part;
    int len;

    TRACE("\n");
    if (infoPtr->numParts == 0)
	return FALSE;

    part = &infoPtr->parts[0];
    /* duplicate string */
    if (part->text)
        Free (part->text);
    part->text = 0;
    if (infoPtr->bUnicode) {
	if (text && (len = strlenW((LPCWSTR)text))) {
	    part->text = Alloc ((len+1)*sizeof(WCHAR));
	    if (!part->text) return FALSE;
	    strcpyW (part->text, (LPCWSTR)text);
	}
    }
    else {
	if (text && (len = lstrlenA(text))) {
            DWORD lenW = MultiByteToWideChar( CP_ACP, 0, text, -1, NULL, 0 );
            part->text = Alloc (lenW*sizeof(WCHAR));
	    if (!part->text) return FALSE;
            MultiByteToWideChar( CP_ACP, 0, text, -1, part->text, lenW );
	}
    }

    InvalidateRect(infoPtr->m_hWnd, &part->bound, FALSE);

    return TRUE;
}


static BOOL
STATUSBAR_WMSize (STATUS_INFO *infoPtr, WORD flags)
{
    INT  width, x, y;
    RECT parent_rect;

    /* Need to resize width to match parent */
    TRACE("flags %04x\n", flags);

    if (flags != SIZE_RESTORED && flags != SIZE_MAXIMIZED) {
	WARN("flags MUST be SIZE_RESTORED or SIZE_MAXIMIZED\n");
	return FALSE;
    }

    if (GetWindowLongW(infoPtr->m_hWnd, GWL_STYLE) & CCS_NORESIZE) return FALSE;

    /* width and height don't apply */
    GetClientRect (infoPtr->m_hWndNotify, &parent_rect);
    width = parent_rect.right - parent_rect.left;
    x = parent_rect.left;
    y = parent_rect.bottom - infoPtr->height;
    MoveWindow (infoPtr->m_hWnd, parent_rect.left,
		parent_rect.bottom - infoPtr->height,
		width, infoPtr->height, TRUE);
    STATUSBAR_SetPartBounds (infoPtr);
    return TRUE;
}


/* update theme after a WM_THEMECHANGED message */
static LRESULT theme_changed (STATUS_INFO* infoPtr)
{
    HTHEME theme = GetWindowTheme (infoPtr->m_hWnd);
    CloseThemeData (theme);
    OpenThemeData (infoPtr->m_hWnd, themeClass);
    return 0;
}


static LRESULT
STATUSBAR_NotifyFormat (STATUS_INFO *infoPtr, HWND from, INT cmd)
{
    if (cmd == NF_REQUERY) {
	INT i = SendMessageW(from, WM_NOTIFYFORMAT, (WPARAM)infoPtr->m_hWnd, NF_QUERY);
	infoPtr->NtfUnicode = (i == NFR_UNICODE);
    }
    return infoPtr->NtfUnicode ? NFR_UNICODE : NFR_ANSI;
}


static LRESULT
STATUSBAR_SendNotify (STATUS_INFO *infoPtr, UINT code)
{
    NMHDR  nmhdr;

    TRACE("code %04x\n", code);
    nmhdr.hwndFrom = infoPtr->m_hWnd;
    nmhdr.idFrom = GetWindowLongPtrW (infoPtr->m_hWnd, GWLP_ID);
    nmhdr.code = code;
    SendMessageW (infoPtr->m_hWndNotify, WM_NOTIFY, 0, (LPARAM)&nmhdr);
    return 0;
}



static LRESULT WINAPI
StatusWindowProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    STATUS_INFO *infoPtr = (STATUS_INFO *)GetWindowLongPtrW (hwnd, 0);
    INT nPart = ((INT) wParam) & 0x00ff;
    LRESULT res;

    TRACE("hwnd=%p msg=%x wparam=%x lparam=%lx\n", hwnd, msg, wParam, lParam);
    if (!infoPtr && msg != WM_CREATE)
        return DefWindowProcW (hwnd, msg, wParam, lParam);

    switch (msg) {
	case SB_GETBORDERS:
	    return STATUSBAR_GetBorders (infoPtr, (INT *)lParam);

	case SB_GETICON:
	    return (LRESULT)STATUSBAR_GetIcon (infoPtr, nPart);

	case SB_GETPARTS:
	    return STATUSBAR_GetParts (infoPtr, (INT)wParam, (INT *)lParam);

	case SB_GETRECT:
	    return STATUSBAR_GetRect (infoPtr, nPart, (LPRECT)lParam);

	case SB_GETTEXTA:
	    return STATUSBAR_GetTextA (infoPtr, nPart, (LPSTR)lParam);

	case SB_GETTEXTW:
	    return STATUSBAR_GetTextW (infoPtr, nPart, (LPWSTR)lParam);

	case SB_GETTEXTLENGTHA:
	case SB_GETTEXTLENGTHW:
	    return STATUSBAR_GetTextLength (infoPtr, nPart);

	case SB_GETTIPTEXTA:
	    return STATUSBAR_GetTipTextA (infoPtr,  LOWORD(wParam), (LPSTR)lParam,  HIWORD(wParam));

	case SB_GETTIPTEXTW:
	    return STATUSBAR_GetTipTextW (infoPtr,  LOWORD(wParam), (LPWSTR)lParam,  HIWORD(wParam));

	case SB_GETUNICODEFORMAT:
	    return infoPtr->bUnicode;

	case SB_ISSIMPLE:
	    return infoPtr->simple;

	case SB_SETBORDERS:
	    return STATUSBAR_SetBorders (infoPtr, (INT *)lParam);

	case SB_SETBKCOLOR:
	    return STATUSBAR_SetBkColor (infoPtr, (COLORREF)lParam);

	case SB_SETICON:
	    return STATUSBAR_SetIcon (infoPtr, nPart, (HICON)lParam);

	case SB_SETMINHEIGHT:
	    return STATUSBAR_SetMinHeight (infoPtr, (INT)wParam);

	case SB_SETPARTS:
	    return STATUSBAR_SetParts (infoPtr, (INT)wParam, (LPINT)lParam);

	case SB_SETTEXTA:
	    return STATUSBAR_SetTextT (infoPtr, nPart, wParam & 0xff00, (LPCWSTR)lParam, FALSE);

	case SB_SETTEXTW:
	    return STATUSBAR_SetTextT (infoPtr, nPart, wParam & 0xff00, (LPCWSTR)lParam, TRUE);

	case SB_SETTIPTEXTA:
	    return STATUSBAR_SetTipTextA (infoPtr, (INT)wParam, (LPSTR)lParam);

	case SB_SETTIPTEXTW:
	    return STATUSBAR_SetTipTextW (infoPtr, (INT)wParam, (LPWSTR)lParam);

	case SB_SETUNICODEFORMAT:
	    return STATUSBAR_SetUnicodeFormat (infoPtr, (BOOL)wParam);

	case SB_SIMPLE:
	    return STATUSBAR_Simple (infoPtr, (BOOL)wParam);

	case WM_CREATE:
	    return STATUSBAR_WMCreate (hwnd, (LPCREATESTRUCTA)lParam);

	case WM_DESTROY:
	    return STATUSBAR_WMDestroy (infoPtr);

	case WM_GETFONT:
	    return (LRESULT)(infoPtr->hFont? infoPtr->hFont : infoPtr->hDefaultFont);

	case WM_GETTEXT:
            return STATUSBAR_WMGetText (infoPtr, (INT)wParam, (LPWSTR)lParam);

	case WM_GETTEXTLENGTH:
	    return STATUSBAR_GetTextLength (infoPtr, 0);

	case WM_LBUTTONDBLCLK:
            return STATUSBAR_SendNotify (infoPtr, NM_DBLCLK);

	case WM_LBUTTONUP:
	    return STATUSBAR_SendNotify (infoPtr, NM_CLICK);

	case WM_MOUSEMOVE:
	    return STATUSBAR_Relay2Tip (infoPtr, msg, wParam, lParam);

	case WM_NCHITTEST:
	    res = STATUSBAR_WMNCHitTest(infoPtr, (short)LOWORD(lParam),
                                        (short)HIWORD(lParam));
	    if (res != HTERROR) return res;
	    return DefWindowProcW (hwnd, msg, wParam, lParam);

	case WM_NCLBUTTONUP:
	case WM_NCLBUTTONDOWN:
    	    PostMessageW (infoPtr->m_hWndNotify, msg, wParam, lParam);
	    return 0;

	case WM_NOTIFYFORMAT:
	    return STATUSBAR_NotifyFormat(infoPtr, (HWND)wParam, (INT)lParam);

	case WM_PRINTCLIENT:
	case WM_PAINT:
	    return STATUSBAR_WMPaint (infoPtr, (HDC)wParam);

	case WM_RBUTTONDBLCLK:
	    return STATUSBAR_SendNotify (infoPtr, NM_RDBLCLK);

	case WM_RBUTTONUP:
	    return STATUSBAR_SendNotify (infoPtr, NM_RCLICK);

	case WM_SETFONT:
	    return STATUSBAR_WMSetFont (infoPtr, (HFONT)wParam, LOWORD(lParam));

	case WM_SETTEXT:
	    return STATUSBAR_WMSetText (infoPtr, (LPCSTR)lParam);

	case WM_SIZE:
	    if (STATUSBAR_WMSize (infoPtr, (WORD)wParam)) return 0;
            return DefWindowProcW (hwnd, msg, wParam, lParam);

        case WM_THEMECHANGED:
            return theme_changed (infoPtr);

	default:
	    if ((msg >= WM_USER) && (msg < WM_APP))
		ERR("unknown msg %04x wp=%04x lp=%08lx\n",
		     msg, wParam, lParam);
	    return DefWindowProcW (hwnd, msg, wParam, lParam);
    }
}


/***********************************************************************
 * STATUS_Register [Internal]
 *
 * Registers the status window class.
 */

void
STATUS_Register (void)
{
    WNDCLASSW wndClass;

    ZeroMemory (&wndClass, sizeof(WNDCLASSW));
    wndClass.style         = CS_GLOBALCLASS | CS_DBLCLKS | CS_VREDRAW;
    wndClass.lpfnWndProc   = StatusWindowProc;
    wndClass.cbClsExtra    = 0;
    wndClass.cbWndExtra    = sizeof(STATUS_INFO *);
    wndClass.hCursor       = LoadCursorW (0, (LPWSTR)IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wndClass.lpszClassName = STATUSCLASSNAMEW;

    RegisterClassW (&wndClass);
}


/***********************************************************************
 * STATUS_Unregister [Internal]
 *
 * Unregisters the status window class.
 */

void
STATUS_Unregister (void)
{
    UnregisterClassW (STATUSCLASSNAMEW, NULL);
}
