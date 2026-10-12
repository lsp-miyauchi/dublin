/*
 * Combo controls
 *
 * Copyright 1997 Alex Korobka
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
 * NOTES
 *
 * This code was audited for completeness against the documented features
 * of Comctl32.dll version 6.0 on Oct. 4, 2004, by Dimitrie O. Paun.
 * 
 * Unless otherwise noted, we believe this code to be complete, as per
 * the specification mentioned above.
 * If you discover missing features, or bugs, please note them below.
 * 
 * TODO:
 *   - ComboBox_[GS]etMinVisible()
 *   - CB_GETMINVISIBLE, CB_SETMINVISIBLE
 *   - CB_SETTOPINDEX
 */

#include <stdarg.h>
#include <string.h>

#define OEMRESOURCE

#include "windef.h"
#include "winbase.h"
#include "wingdi.h"
#include "winuser.h"
#include "wine/winuser16.h"
#include "user_private.h"
#include "win.h"
#include "controls.h"
#include "winreg.h"
#include "winternl.h"
#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(combo);

////////////////////////////////////////////////////////////////////////
// listbox's undocumented message

#define LB_CARETON             0x01a3
#define LB_CARETOFF            0x01a4

////////////////////////////////////////////////////////////////////////
// combo box

#define ID_CB_LISTBOX           1000
#define ID_CB_EDIT              1001

////////////////////////////////////////////////////////////////////////
// internal flags

#define CBF_NOROLLUP            0x0004
#define CBF_NORESIZE            0x0080
#define CBF_NOTIFY              0x0100
#define CBF_SELCHANGE           0x0400
#define CBF_EUI                 0x8000

////////////////////////////////////////////////////////////////////////
// combo state struct

typedef struct {
	HWND m_hWnd;
	HWND owner;
	UINT dwStyle;
	HWND m_edit;
	HWND m_lbox;
	UINT wState;
	HFONT hFont;
	CRect rcText;
	CRect rcButton;
	CRect rcDropped;
	int indexDropped;
	INT fixedOwnerDrawHeight;
	INT droppedWidth;				// last two are not used unless set
	INT editHeight;					// explicitly

	bool m_bDropped:1;
	bool m_bCaptured:1;
	bool m_bPressed:1;
	bool m_bFocus:1;
	bool m_bHasEditCtrl:1;
	bool m_bNoRedraw:1;
	bool m_bNoLBSelect:1;
	bool m_bNoEditNotify:1;
	bool m_bNeedMeasureItem:1;

	inline UINT GetType() { return (dwStyle & CBS_TYPEMASK); }
	inline bool IsSimple() { return ((dwStyle & CBS_TYPEMASK)==CBS_SIMPLE); }
	inline bool IsDropDown() { return ((dwStyle & CBS_TYPEMASK)==CBS_DROPDOWN); }
	inline bool IsDropDownList() { return ((dwStyle & CBS_TYPEMASK)==CBS_DROPDOWNLIST); }
	inline bool IsOwnerDraw() { return ((dwStyle & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE)) != 0); }
	inline bool HasStrings() { return ((dwStyle & CBS_HASSTRINGS) != 0); }
} HEADCOMBO,*LPHEADCOMBO;

////////////////////////////////////////////////////////////////////////
// bits in the dwKeyData

#define KEYDATA_ALT             0x2000
#define KEYDATA_PREVSTATE       0x4000

////////////////////////////////////////////////////////////////////////
// Additional combo box definitions

#define CB_NOTIFY( lphc, code ) \
    (SendMessageW((lphc)->owner, WM_COMMAND, \
                  MAKEWPARAM(GetWindowLongPtrW((lphc)->m_hWnd,GWLP_ID), (code)), (LPARAM)(lphc)->m_hWnd))

#define CBS_TYPEMASK			0x0003L

#define CB_DISABLED(lphc)			(!IsWindowEnabled((lphc)->m_hWnd))
#define CB_HWND(lphc)				((lphc)->m_hWnd)

#define ISWIN31 (LOWORD(GetVersion()) == 0x0a03)


////////////////////////////////////////////////////////////////////////
// Drawing globals

static HBITMAP 	hComboBmp = 0;
static UINT	CBitHeight, CBitWidth;

////////////////////////////////////////////////////////////////////////
// Look and feel dependent "constants"

#define COMBO_YBORDERGAP         5
#define COMBO_XBORDERSIZE()      2
#define COMBO_YBORDERSIZE()      2
#define COMBO_EDITBUTTONSPACE()  0
#define EDIT_CONTROL_PADDING()   1

static LRESULT WINAPI ComboWndProcA( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
static LRESULT WINAPI ComboWndProcW( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );

////////////////////////////////////////////////////////////////////////
// combo class descriptor

const struct builtin_class_descr COMBO_builtin_class =
{
    "ComboBox",           /* name */
    CS_PARENTDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW, /* style  */
    ComboWndProcA,        /* procA */
    ComboWndProcW,        /* procW */
    sizeof(HEADCOMBO *),  /* extra */
    IDC_ARROW,            /* cursor */
    0                     /* brush */
};


////////////////////////////////////////////////////////////////////////
// COMBO_Init ---- Load combo button bitmap.

static BOOL COMBO_Init(void)
{
	if (hComboBmp) return TRUE;

	BOOL bOK = FALSE;
	CDC dc;
	if (dc.CreateCompatibleDC(NULL)) {
		hComboBmp = ::LoadBitmap(NULL, MAKEINTRESOURCEW(OBM_COMBO));
		if (hComboBmp) {
			BITMAP bm;
			GetObject(hComboBmp, sizeof(bm), &bm);
			CBitHeight = bm.bmHeight;
			CBitWidth  = bm.bmWidth;
			HBITMAP htemp = dc.SelectBitmap(hComboBmp);

			CRect r(0, 0, CBitWidth, CBitHeight);
			dc.InvertRect(&r);
			dc.SelectBitmap(htemp);
			bOK = TRUE;
		}
	}
	return bOK;
}

////////////////////////////////////////////////////////////////////////
// COMBO_NCCreate

BOOL COMBO_OnNcCreate(LPCREATESTRUCT pcs)
{
	LPHEADCOMBO lphc;
	if (COMBO_Init()) {
		LPHEADCOMBO lphc = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(HEADCOMBO));
		if (lphc) {
			lphc->m_hWnd = hwnd;
			lphc->m_bDropped = false;
			lphc->m_bCaptured = false;
			lphc->m_bPressed = false;
			lphc->m_bFocus = false;
			lphc->m_bHasEditCtrl = false;
			lphc->m_bNoRedraw = false;
			lphc->m_bNoLBSelect = false;
			lphc->m_bNoEditNotify = false;
			lphc->m_bNeedMeasureItem = false;

			SetWindowLongPtr(hwnd, 0, (LONG_PTR)lphc);
			// some braindead apps do try to use scrollbar/border flags
			lphc->dwStyle = style & ~(WS_BORDER | WS_HSCROLL | WS_VSCROLL);
			SetWindowLong(hwnd, GWL_STYLE, pcs->style & ~(WS_BORDER | WS_HSCROLL | WS_VSCROLL));

			// We also have to remove the client edge style to make sure
			// we don't end-up with a non client area.
			SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_CLIENTEDGE);

			if (!(pcs->style & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE)))
				lphc->dwStyle |= CBS_HASSTRINGS;
			if (!(GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_NOPARENTNOTIFY))
				lphc->wState |= CBF_NOTIFY;
			return TRUE;
		}
	}
	return FALSE;
}

/***********************************************************************
 *           COMBO_NCDestroy
 */
static LRESULT COMBO_NCDestroy( LPHEADCOMBO lphc )
{

	if (lphc) {
		if (!lphc->IsSimple() && lphc->m_lbox.IsWindow())
			lphc->m_lbox.DestroyWindow();

		SetWindowLongPtrW( lphc->m_hWnd, 0, 0 );
		HeapFree( GetProcessHeap(), 0, lphc );
	}
	return 0;
}

/***********************************************************************
 *           CBGetTextAreaHeight
 *
 * This method will calculate the height of the text area of the
 * combobox.
 * The height of the text area is set in two ways.
 * It can be set explicitly through a combobox message or through a
 * WM_MEASUREITEM callback.
 * If this is not the case, the height is set to 13 dialog units.
 * This height was determined through experimentation.
 */
static INT CBGetTextAreaHeight(HWND hwnd, LPHEADCOMBO lphc)
{
	int iTextItemHeight;

	if (lphc->editHeight) {
		/* explicitly set height */
		iTextItemHeight = lphc->editHeight;
	}
	else {
		CClientDC dc(hwnd);
		HFONT hPrevFont = 0;
		if (lphc->hFont)
			hPrevFont = dc.SelectFont(lphc->hFont);

		TEXTMETRIC tm;
		dc.GetTextMetricsW(&tm);
		int baseUnitY = tm.tmHeight;

		if (hPrevFont)
			dc.SelectFont(hPrevFont);

		iTextItemHeight = ((13 * baseUnitY) / 8);

		/*
		 * This "formula" calculates the height of the complete control.
		 * To calculate the height of the text area, we have to remove the
		 * borders.
		 */
		iTextItemHeight -= 2*COMBO_YBORDERSIZE();
	}

	/*
	 * Check the ownerdraw case if we haven't asked the parent the size
	 * of the item yet.
	 */
	if ( lphc->IsOwnerDraw() && lphc->m_bNeedMeasureItem) {
		INT               originalItemHeight = iTextItemHeight;
		UINT id = (UINT)GetWindowLongPtrW( lphc->m_hWnd, GWLP_ID );

		/*
		 * We use the client rect for the width of the item.
		 */
		CRect rc;
		GetClientRect(hwnd, &rc);

		lphc->m_bNeedMeasureItem = false;

		/*
		 * Send a first one to measure the size of the text area
		 */
		MEASUREITEMSTRUCT mi;
		mi.CtlType    = ODT_COMBOBOX;
		mi.CtlID      = id;
		mi.itemID     = -1;
		mi.itemWidth  = rc.Width();
		mi.itemHeight = iTextItemHeight - 6; /* ownerdrawn cb is taller */
		mi.itemData   = 0;
		SendMessage(lphc->owner, WM_MEASUREITEM, id, (LPARAM)&mi);
		iTextItemHeight = 6 + mi.itemHeight;

		/*
		 * Send a second one in the case of a fixed ownerdraw list to calculate the
		 * size of the list items. (we basically do this on behalf of the listbox)
		 */
		if (lphc->dwStyle & CBS_OWNERDRAWFIXED) {
			mi.CtlType    = ODT_COMBOBOX;
			mi.CtlID      = id;
			mi.itemID     = 0;
			mi.itemWidth  = rc.Width();
			mi.itemHeight = originalItemHeight;
			mi.itemData   = 0;
			SendMessage(lphc->owner, WM_MEASUREITEM, id, (LPARAM)&mi);
			lphc->fixedOwnerDrawHeight = mi.itemHeight;
		}

		/*
		 * Keep the size for the next time
		 */
		lphc->editHeight = iTextItemHeight;
	}
	return iTextItemHeight;
}

/***********************************************************************
 *           CBForceDummyResize
 *
 * The dummy resize is used for listboxes that have a popup to trigger
 * a re-arranging of the contents of the combobox and the recalculation
 * of the size of the "real" control window.
 */
static void CBForceDummyResize(LPHEADCOMBO lphc)
{
	int newComboHeight = CBGetTextAreaHeight(lphc->m_hWnd,lphc) + 2 * COMBO_YBORDERSIZE();

	CRect rc;
	GetWindowRect(lphc->m_hWnd, &rc);

	/*
	 * We have to be careful, resizing a combobox also has the meaning that the
	 * dropped rect will be resized. In this case, we want to trigger a resize
	 * to recalculate layout but we don't want to change the dropped rectangle
	 * So, we pass the height of text area of control as the height.
	 * this will cancel-out in the processing of the WM_WINDOWPOSCHANGING
	 * message.
	 */
	SetWindowPos(lphc->m_hWnd, NULL, 0, 0, rc.Width(), newComboHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

/***********************************************************************
 *           CBCalcPlacement
 *
 * Set up component coordinates given valid lphc->RectCombo.
 */
static void CBCalcPlacement(
  HWND        hwnd,
  LPHEADCOMBO lphc,
  LPRECT      lprEdit,
  LPRECT      lprButton,
  LPRECT      lprLB)
{
	// Again, start with the client rectangle.
	GetClientRect(hwnd, lprEdit);

	// Remove the borders
	InflateRect(lprEdit, -COMBO_XBORDERSIZE(), -COMBO_YBORDERSIZE());

	// Chop off the bottom part to fit with the height of the text area.
	lprEdit->bottom = lprEdit->top + CBGetTextAreaHeight(hwnd, lphc);

	// The button starts the same vertical position as the text area.
	CopyRect(lprButton, lprEdit);

	// If the combobox is "simple" there is no button.
	if (lphc->IsSimple())
		lprButton->left = lprButton->right = lprButton->bottom = 0;
	else {
		// Let's assume the combobox button is the same width as the scrollbar button.
		// size the button horizontally and cut-off the text area.
		lprButton->left = lprButton->right - GetSystemMetrics(SM_CXVSCROLL);
		lprEdit->right  = lprButton->left;
	}

	// In the case of a dropdown, there is an additional spacing between the
	// text area and the button.
	if (lphc->IsDropDown()) {
		lprEdit->right -= COMBO_EDITBUTTONSPACE();
	}

	// If we have an edit control, we space it away from the borders slightly.
	if (!lphc->IsDropDownList()) {
		InflateRect(lprEdit, -EDIT_CONTROL_PADDING(), -EDIT_CONTROL_PADDING());
	}

	// Adjust the size of the listbox popup.
	if (lphc->IsSimple()) {
		// Use the client rectangle to initialize the listbox rectangle
		GetClientRect(hwnd, lprLB);

		// Then, chop-off the top part.
		lprLB->top = lprEdit->bottom + COMBO_YBORDERSIZE();
	}
	else {
		// Make sure the dropped width is as large as the combobox itself.
		if (lphc->droppedWidth < (lprButton->right + COMBO_XBORDERSIZE())) {
			lprLB->right  = lprLB->left + (lprButton->right + COMBO_XBORDERSIZE());

		// In the case of a dropdown, the popup listbox is offset to the right.
		// so, we want to make sure it's flush with the right side of the combobox
		if (lphc->IsDropDown())
			lprLB->right -= COMBO_EDITBUTTONSPACE();
		}
		else
			lprLB->right = lprLB->left + lphc->droppedWidth;
	}

	// don't allow negative window width
	if (lprEdit->right < lprEdit->left)
		lprEdit->right = lprEdit->left;
}

/***********************************************************************
 *           CBGetDroppedControlRect
 */
static void CBGetDroppedControlRect( LPHEADCOMBO lphc, LPRECT prc)
{
	// In windows, CB_GETDROPPEDCONTROLRECT returns the upper left corner
	// of the combo box and the lower right corner of the listbox
	GetWindowRect(lphc->m_hWnd, prc);

	prc->right =  prc->left + lphc->rcDropped.Width();
	prc->bottom = prc->top + lphc->rcDropped.Height();
}

/***********************************************************************
 *           COMBO_WindowPosChanging
 */
static LRESULT COMBO_WindowPosChanging(
  HWND        hwnd,
  LPHEADCOMBO lphc,
  WINDOWPOS*  posChanging)
{
  /*
   * We need to override the WM_WINDOWPOSCHANGING method to handle all
   * the non-simple comboboxes. The problem is that those controls are
   * always the same height. We have to make sure they are not resized
   * to another value.
   */
	if (!lphc->IsSimple() && ((posChanging->flags & SWP_NOSIZE) == 0) ) {
		int newComboHeight = CBGetTextAreaHeight(hwnd,lphc) + 2 * COMBO_YBORDERSIZE();

    /*
     * Resizing a combobox has another side effect, it resizes the dropped
     * rectangle as well. However, it does it only if the new height for the
     * combobox is different from the height it should have. In other words,
     * if the application resizing the combobox only had the intention to resize
     * the actual control, for example, to do the layout of a dialog that is
     * resized, the height of the dropdown is not changed.
     */
		if (posChanging->cy != newComboHeight) {
			lphc->rcDropped.bottom = lphc->rcDropped.top + posChanging->cy - newComboHeight;
			posChanging->cy = newComboHeight;
		}
	}
	return 0;
}

/***********************************************************************
 *           COMBO_Create
 */
static LRESULT COMBO_Create( HWND hwnd, LPHEADCOMBO lphc, HWND hwndParent, LONG style )
{
  static const WCHAR clbName[] = {'C','o','m','b','o','L','B','o','x',0};
  static const WCHAR editName[] = {'E','d','i','t',0};

	if (!lphc->GetType())
		lphc->dwStyle |= CBS_SIMPLE;
	if (!lphc->IsDropDownList())
		lphc->m_bHasEditCtrl = true;

	lphc->owner = hwndParent;

	/*
	 * The item height and dropped width are not set when the control
	 * is created.
	 */
	lphc->droppedWidth = lphc->editHeight = 0;

	/*
	 * The first time we go through, we want to measure the ownerdraw item
	 */
	lphc->m_bNeedMeasureItem = true;

	/* M$ IE 3.01 actually creates (and rapidly destroys) an ownerless combobox */
	if (lphc->owner || !(style & WS_VISIBLE)) {
		UINT lbeStyle   = 0;
		UINT lbeExStyle = 0;

		/*
		 * Initialize the dropped rect to the size of the client area of the
		 * control and then, force all the areas of the combobox to be
		 * recalculated.
		 */
		GetClientRect( hwnd, &lphc->rcDropped );
		CBCalcPlacement(hwnd, lphc, &lphc->rcText, &lphc->rcButton, &lphc->rcDropped );

		/*
		 * Adjust the position of the popup listbox if it's necessary
		 */
		if (!lphc->IsSimple()) {
			lphc->rcDropped.top   = lphc->rcText.bottom + COMBO_YBORDERSIZE();

			/*
			 * If it's a dropdown, the listbox is offset
			 */
			if (lphc->IsDropDown())
				lphc->rcDropped.left += COMBO_EDITBUTTONSPACE();

			if (lphc->rcDropped.bottom < lphc->rcDropped.top)
				lphc->rcDropped.bottom = lphc->rcDropped.top;
			if (lphc->rcDropped.right < lphc->rcDropped.left)
				lphc->rcDropped.right = lphc->rcDropped.left;
			MapWindowPoints( hwnd, 0, (LPPOINT)&lphc->rcDropped, 2 );
		}

		/* create listbox popup */
		lbeStyle = (LBS_NOTIFY | LBS_COMBOBOX | WS_BORDER | WS_CLIPSIBLINGS | WS_CHILD) |
                 (style & (WS_VSCROLL | CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE));

		if (lphc->dwStyle & CBS_SORT)
			lbeStyle |= LBS_SORT;
		if( lphc->dwStyle & CBS_HASSTRINGS )
			lbeStyle |= LBS_HASSTRINGS;
		if( lphc->dwStyle & CBS_NOINTEGRALHEIGHT )
			lbeStyle |= LBS_NOINTEGRALHEIGHT;
		if( lphc->dwStyle & CBS_DISABLENOSCROLL )
			lbeStyle |= LBS_DISABLENOSCROLL;

		if (lphc->IsSimple()) {			 	/* child listbox */
			lbeStyle |= WS_VISIBLE;

			/*
			 * In win 95 look n feel, the listbox in the simple combobox has
			 * the WS_EXCLIENTEDGE style instead of the WS_BORDER style.
			 */
			lbeStyle   &= ~WS_BORDER;
			lbeExStyle |= WS_EX_CLIENTEDGE;
		}
		else
			lbeExStyle |= (WS_EX_TOPMOST | WS_EX_TOOLWINDOW);

		lphc->m_lbox = CreateWindowEx(lbeExStyle, clbName, NULL, lbeStyle,
                                           lphc->rcDropped.left,
                                           lphc->rcDropped.top,
                                           lphc->rcDropped.Width(),
                                           lphc->rcDropped.Height(),
                                           hwnd, (HMENU)ID_CB_LISTBOX,
                                           (HINSTANCE)GetWindowLongPtrW( hwnd, GWLP_HINSTANCE ), lphc );

		if (lphc->m_lbox.IsWindow()) {
			BOOL bEdit = TRUE;
			lbeStyle = WS_CHILD | WS_VISIBLE | ES_NOHIDESEL | ES_LEFT | ES_COMBO;

			if (lphc->m_bHasEditCtrl) {
				if (lphc->dwStyle & CBS_OEMCONVERT)		lbeStyle |= ES_OEMCONVERT;
				if (lphc->dwStyle & CBS_AUTOHSCROLL)	lbeStyle |= ES_AUTOHSCROLL;
				if (lphc->dwStyle & CBS_LOWERCASE)		lbeStyle |= ES_LOWERCASE;
				else if (lphc->dwStyle & CBS_UPPERCASE)	lbeStyle |= ES_UPPERCASE;
				if (!IsWindowEnabled(hwnd))				lbeStyle |= WS_DISABLED;

				lphc->m_edit = CreateWindowExW(0, editName, NULL, lbeStyle,
                                                   lphc->rcText.left, lphc->rcText.top,
                                                   lphc->rcText.Width(),
                                                   lphc->rcText.Height(),
                                                   hwnd, (HMENU)ID_CB_EDIT,
                                                   (HINSTANCE)GetWindowLongPtrW( hwnd, GWLP_HINSTANCE ), NULL );

				if (!lphc->m_edit.IsWindow())
					bEdit = FALSE;
			}

			if (bEdit) {
				if (!lphc->IsSimple()) {
					/* Now do the trick with parent */
					lphc->m_lbox.SetParent(HWND_DESKTOP);

					/*
					 * If the combo is a dropdown, we must resize the control
					 * to fit only the text area and button. To do this,
					 * we send a dummy resize and the WM_WINDOWPOSCHANGING message
					 * will take care of setting the height for us.
					 */
					CBForceDummyResize(lphc);
				}
				return 0;
			}
		}
	}

	/* CreateWindow() will send WM_NCDESTROY to cleanup */
	return -1;
}

/***********************************************************************
 *           CBPaintButton
 *
 * Paint combo button (normal, pressed, and disabled states).
 */
static void CBPaintButton( LPHEADCOMBO lphc, HDC hdc, RECT rectButton)
{
    UINT buttonState = DFCS_SCROLLCOMBOBOX;

	if (lphc->m_bNoRedraw) return;


    if (lphc->m_bPressed)
		buttonState |= DFCS_PUSHED;

    if (CB_DISABLED(lphc))
		buttonState |= DFCS_INACTIVE;

    DrawFrameControl(hdc, &rectButton, DFC_SCROLL, buttonState);
}

/***********************************************************************
 *           CBPaintText
 *
 * Paint CBS_DROPDOWNLIST text field / update edit control contents.
 */
static void CBPaintText(
  LPHEADCOMBO lphc,
  HDC         hdc,
  RECT        rectEdit)
{
   INT	id, size = 0;
   LPWSTR pText = NULL;

	if (lphc->m_bNoRedraw) return;

   /* follow Windows combobox that sends a bunch of text
    * inquiries to its listbox while processing WM_PAINT. */

   if( (id = lphc->m_lbox.GetCurSel() ) != LB_ERR )
   {
        size = lphc->m_lbox.GetTextLen(id);
	if (size == LB_ERR)
	  FIXME("LB_ERR probably not handled yet\n");
        if( (pText = HeapAlloc( GetProcessHeap(), 0, (size + 1) * sizeof(WCHAR))) )
	{
            /* size from LB_GETTEXTLEN may be too large, from LB_GETTEXT is accurate */
	    size = lphc->m_lbox.GetText(id, pText);
	    pText[size] = '\0';	/* just in case */
	} else return;
   }
   else
       if( !lphc->IsOwnerDraw() )
	   return;

   if (lphc->m_bHasEditCtrl) {
        static const WCHAR empty_stringW[] = { 0 };
	if( lphc->HasStrings() )
		lphc->m_edit.SetWindowText(pText ? pText : empty_stringW);
	if (lphc->m_bFocus)
		lphc->m_edit.SetSel(0, -1);
   }
   else /* paint text field ourselves */
   {
     UINT	itemState = ODS_COMBOBOXEDIT;
     HFONT	hPrevFont = (lphc->hFont) ? SelectObject(hdc, lphc->hFont) : 0;

     /*
      * Give ourselves some space.
      */
     InflateRect( &rectEdit, -1, -1 );

     if( lphc->IsOwnerDraw() )
     {
       DRAWITEMSTRUCT dis;
       HRGN           clipRegion;
       UINT ctlid = (UINT)GetWindowLongPtrW( lphc->m_hWnd, GWLP_ID );

       /* setup state for DRAWITEM message. Owner will highlight */
       if (lphc->m_bFocus && !lphc->m_bDropped)
			itemState |= ODS_SELECTED | ODS_FOCUS;

       /*
	* Save the current clip region.
	* To retrieve the clip region, we need to create one "dummy"
	* clip region.
	*/
       clipRegion = CreateRectRgnIndirect(&rectEdit);

       if (GetClipRgn(hdc, clipRegion)!=1)
       {
	 DeleteObject(clipRegion);
	 clipRegion=NULL;
       }

       if (!IsWindowEnabled(lphc->m_hWnd) & WS_DISABLED) itemState |= ODS_DISABLED;

       dis.CtlType	= ODT_COMBOBOX;
       dis.CtlID	= ctlid;
       dis.hwndItem	= lphc->m_hWnd;
       dis.itemAction	= ODA_DRAWENTIRE;
       dis.itemID	= id;
       dis.itemState	= itemState;
       dis.hDC		= hdc;
       dis.rcItem	= rectEdit;
       dis.itemData	= lphc->m_lbox.GetItemData(id);

       /*
	* Clip the DC and have the parent draw the item.
	*/
       IntersectClipRect(hdc,
			 rectEdit.left,  rectEdit.top,
			 rectEdit.right, rectEdit.bottom);

       SendMessageW(lphc->owner, WM_DRAWITEM, ctlid, (LPARAM)&dis );

       /*
	* Reset the clipping region.
	*/
       SelectClipRgn(hdc, clipRegion);
     }
     else
     {
       static const WCHAR empty_stringW[] = { 0 };

       if ( lphc->m_bFocus && !lphc->m_bDropped ) {

	   /* highlight */
	   FillRect( hdc, &rectEdit, GetSysColorBrush(COLOR_HIGHLIGHT) );
	   SetBkColor( hdc, GetSysColor( COLOR_HIGHLIGHT ) );
	   SetTextColor( hdc, GetSysColor( COLOR_HIGHLIGHTTEXT ) );
       }

       ExtTextOutW( hdc,
		    rectEdit.left + 1,
		    rectEdit.top + 1,
		    ETO_OPAQUE | ETO_CLIPPED,
		    &rectEdit,
		    pText ? pText : empty_stringW , size, NULL );

       if (lphc->m_bFocus && !lphc->m_bDropped)
	 DrawFocusRect( hdc, &rectEdit );
     }

     if( hPrevFont )
       SelectObject(hdc, hPrevFont );
   }
   HeapFree( GetProcessHeap(), 0, pText );
}

/***********************************************************************
 *           CBPaintBorder
 */
static void CBPaintBorder(HWND hwnd, LPHEADCOMBO lphc, HDC hdc)
{
	CRect rc;
	if (!lphc->IsSimple())
		GetClientRect(hwnd, &rc);
	else {
		rc = lphc->rcText;
		rc.InflateRect(EDIT_CONTROL_PADDING(), EDIT_CONTROL_PADDING());
		rc.InflateRect(COMBO_XBORDERSIZE(), COMBO_YBORDERSIZE());
	}
	DrawEdge(hdc, &rc, EDGE_SUNKEN, BF_RECT);
}

/***********************************************************************
 *           COMBO_PrepareColors
 *
 * This method will sent the appropriate WM_CTLCOLOR message to
 * prepare and setup the colors for the combo's DC.
 *
 * It also returns the brush to use for the background.
 */
static HBRUSH COMBO_PrepareColors(LPHEADCOMBO lphc, HDC hDC)
{
	HBRUSH  hbr;
	// Get the background brush for this control.
	if (CB_DISABLED(lphc)) {
		hbr = (HBRUSH)SendMessage(lphc->owner, WM_CTLCOLORSTATIC, (WPARAM)hDC, (LPARAM)lphc->m_hWnd);

		// We have to change the text color since WM_CTLCOLORSTATIC will
		// set it to the "enabled" color. This is the same behavior as the edit control
		SetTextColor(hDC, GetSysColor(COLOR_GRAYTEXT));
	}
	else {
		if (lphc->m_bHasEditCtrl)
			hbr = (HBRUSH)SendMessage(lphc->owner, WM_CTLCOLOREDIT, (WPARAM)hDC, (LPARAM)lphc->m_hWnd);
		else
			hbr = (HBRUSH)SendMessage(lphc->owner, WM_CTLCOLORLISTBOX, (WPARAM)hDC, (LPARAM)lphc->m_hWnd);
	}

	// Catch errors.
	if (!hbr)
		hbr = GetSysColorBrush(COLOR_WINDOW);
	return hbr;
}


/***********************************************************************
 *           COMBO_Paint
 */
static LRESULT COMBO_Paint(LPHEADCOMBO lphc, HDC hParamDC)
{
	PAINTSTRUCT ps;
	HDC hDC = (hParamDC) ? hParamDC : BeginPaint( lphc->m_hWnd, &ps);

	if (hDC && !lphc->m_bNoRedraw) {
		HBRUSH	hPrevBrush, hBkgBrush;

		// Retrieve the background brush and select it in the DC.
		hBkgBrush = COMBO_PrepareColors(lphc, hDC);
		hPrevBrush = SelectObject( hDC, hBkgBrush );
		if (!lphc->m_bHasEditCtrl)
			FillRect(hDC, &lphc->rcText, hBkgBrush);

		// In non 3.1 look, there is a sunken border on the combobox
		CBPaintBorder(lphc->m_hWnd, lphc, hDC);

		if (!IsRectEmpty(&lphc->rcButton))
			CBPaintButton(lphc, hDC, lphc->rcButton);

		// paint the edit control padding area
		if (!lphc->IsDropDownList()) {
			CRect rPadEdit = lphc->rcText;
			rPadEdit.InflateRect(EDIT_CONTROL_PADDING(), EDIT_CONTROL_PADDING());
			FrameRect(hDC, &rPadEdit, GetSysColorBrush(COLOR_WINDOW));
		}

		if (!lphc->m_bHasEditCtrl)
			CBPaintText(lphc, hDC, lphc->rcText);

		if (hPrevBrush)
			SelectObject(hDC, hPrevBrush);
	}

	if (!hParamDC)
		EndPaint(lphc->m_hWnd, &ps);
	return 0;
}

/***********************************************************************
 *           CBUpdateLBox
 *
 * Select listbox entry according to the contents of the edit control.
 */
static INT CBUpdateLBox( LPHEADCOMBO lphc, BOOL bSelect )
{
   LPWSTR pText = NULL;

	int idx = LB_ERR;
	int length = lphc->m_edit.GetTextLength();

	if (length > 0)
		pText = HeapAlloc(GetProcessHeap(), 0, (length + 1) * sizeof(WCHAR));

	if( pText ) {
		lphc->m_edit.GetWindowText(pText, length + 1);
       idx = lphc->m_lbox.FindString(-1, pText);
       HeapFree( GetProcessHeap(), 0, pText );
   }

	lphc->m_lbox.SetCurSel(bSelect ? idx : -1);

	// probably superfluous but Windows sends this too
	lphc->m_lbox.SetCaretIndex(idx < 0 ? 0 : idx);
	lphc->m_lbox.SetTopIndex(idx < 0 ? 0 : idx);

	return idx;
}

/***********************************************************************
 *           CBUpdateEdit
 *
 * Copy a listbox entry to the edit control.
 */
static void CBUpdateEdit( LPHEADCOMBO lphc , INT index )
{
   INT	length;
   LPWSTR pText = NULL;
   static const WCHAR empty_stringW[] = { 0 };

   TRACE("\t %i\n", index );

   if( index >= 0 ) /* got an entry */
   {
		length = lphc->m_lbox.GetTextLen(index);
		if (length != LB_ERR) {
			if ( (pText = HeapAlloc(GetProcessHeap(), 0, (length + 1) * sizeof(WCHAR))) ) {
				lphc->m_lbox.GetText(index, pText);
			}
		}
	}

	lphc->m_bNoLBSelect = true;
	lphc->m_bNoEditNotify = true;
	lphc->m_edit.SetWindowText(pText ? (LPARAM)pText : (LPARAM)empty_stringW);
	lphc->m_bNoLBSelect = false;
	lphc->m_bNoEditNotify = false;

	if (lphc->m_bFocus)
		lphc->m_edit.SetSel(0, -1);

	HeapFree( GetProcessHeap(), 0, pText );
}

/***********************************************************************
 *           CBDropDown
 *
 * Show listbox popup.
 */
static void CBDropDown( LPHEADCOMBO lphc )
{
	HMONITOR monitor;
	MONITORINFO mon_info;
	CRect rect,r;
	int nItems = 0;
	int nDroppedHeight;

	CB_NOTIFY( lphc, CBN_DROPDOWN );

	/* set selection */

	lphc->m_bDropped = true;
	if (lphc->IsDropDown()) {
		lphc->indexDropped = CBUpdateLBox( lphc, TRUE );

		/* Update edit only if item is in the list */
		if (!lphc->m_bCaptured && lphc->indexDropped >= 0)
			CBUpdateEdit( lphc, lphc->indexDropped );
	}
	else {
		lphc->indexDropped = lphc->m_lbox.GetCurSel();
		lphc->m_lbox.SetTopIndex(lphc->indexDropped == LB_ERR ? 0 : lphc->indexDropped);
		lphc->m_lbox.SendMessage(LB_CARETON, 0, 0);
	}

	/* now set popup position */
	GetWindowRect(lphc->m_hWnd, &rect);

	/*
	 * If it's a dropdown, the listbox is offset
	 */
	if (lphc->IsDropDown())
		rect.left += COMBO_EDITBUTTONSPACE();

	/* if the dropped height is greater than the total height of the dropped
	 * items list, then force the drop down list height to be the total height
	 * of the items in the dropped list */

	/* And Remove any extra space (Best Fit) */
	nDroppedHeight = lphc->rcDropped.Height();

	/* if listbox length has been set directly by its handle */
	lphc->m_lbox.GetWindowRect(&r);
	if (nDroppedHeight < r.Height())
		nDroppedHeight = r.Height();
	nItems = lphc->m_lbox.GetCount();

	if (nItems > 0) {
		int nIHeight = lphc->m_lbox.GetItemHeight(0);
		int nHeight = nIHeight * nItems;

		if (nHeight < nDroppedHeight - COMBO_YBORDERSIZE())
			nDroppedHeight = nHeight + COMBO_YBORDERSIZE();

		if (nDroppedHeight < nHeight) {
            if (nItems < 5)
                nDroppedHeight = (nItems+1) * nIHeight;
            else
                nDroppedHeight = 6 * nIHeight;
      }
   }

	/*If height of dropped rectangle gets beyond a screen size it should go up, otherwise down.*/
	monitor = MonitorFromRect( &rect, MONITOR_DEFAULTTOPRIMARY );
	mon_info.cbSize = sizeof(mon_info);
	GetMonitorInfoW( monitor, &mon_info );

	if ((rect.bottom + nDroppedHeight) >= mon_info.rcWork.bottom)
		rect.bottom = rect.top - nDroppedHeight;

	lphc->m_lbox.SetWindowPos(HWND_TOP, rect.left, rect.bottom,
				lphc->rcDropped.Width(), nDroppedHeight, SWP_NOACTIVATE | SWP_SHOWWINDOW);

	if (!lphc->m_bNoRedraw)
		RedrawWindow( lphc->m_hWnd, NULL, 0, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_NOCHILDREN );

	lphc->m_lbox.EnableWindow(TRUE);
	if (GetCapture() != lphc->m_hWnd)
		lphc->m_lbox.SetCapture();
}

/***********************************************************************
 *           CBRollUp
 *
 * Hide listbox popup.
 */
static void CBRollUp( LPHEADCOMBO lphc, BOOL ok, BOOL bButton )
{
	HWND hWnd = lphc->m_hWnd;

	CB_NOTIFY( lphc, (ok) ? CBN_SELENDOK : CBN_SELENDCANCEL );

	if (IsWindow( hWnd ) && !lphc->IsSimple()) {
		if (lphc->m_bDropped) {
			RECT	rect;

			lphc->m_bDropped = false;
			lphc->m_lbox.ShowWindow(SW_HIDE);

			if (GetCapture() == lphc->m_lbox) {
				ReleaseCapture();
			}

			if (lphc->IsDropDown()) {
				rect = lphc->rcButton;
			}
			else {
				if (bButton) {
					UnionRect(&rect, &lphc->rcButton, &lphc->rcText);
				}
				else
					rect = lphc->rcText;
				bButton = TRUE;
			}

			if (bButton && !lphc->m_bNoRedraw)
				RedrawWindow(hWnd, &rect, 0, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_NOCHILDREN);
			CB_NOTIFY( lphc, CBN_CLOSEUP );
		}
	}
}

/***********************************************************************
 *           COMBO_FlipListbox
 *
 * Used by the ComboLBox to show/hide itself in response to VK_F4, etc...
 */
BOOL COMBO_FlipListbox( LPHEADCOMBO lphc, BOOL ok, BOOL bRedrawButton )
{
   if (lphc->m_bDropped) {
       CBRollUp( lphc, ok, bRedrawButton );
       return FALSE;
   }

   CBDropDown( lphc );
   return TRUE;
}

/***********************************************************************
 *           CBRepaintButton
 */
static void CBRepaintButton( LPHEADCOMBO lphc )
   {
  InvalidateRect(lphc->m_hWnd, &lphc->rcButton, TRUE);
  UpdateWindow(lphc->m_hWnd);
}

/***********************************************************************
 *           COMBO_SetFocus
 */
static void COMBO_SetFocus( LPHEADCOMBO lphc )
{
	if (!lphc->m_bFocus) {
		if (lphc->IsDropDownList())
			lphc->m_lbox.SendMessage(LB_CARETON, 0, 0);

       /* This is wrong. Message sequences seem to indicate that this
          is set *after* the notify. */
       /* lphc->m_bFocus = true;  */

		if (!lphc->m_bHasEditCtrl)
			InvalidateRect(lphc->m_hWnd, &lphc->rcText, TRUE);

		CB_NOTIFY( lphc, CBN_SETFOCUS );
		lphc->m_bFocus = true;
	}
}

/***********************************************************************
 *           COMBO_KillFocus
 */
static void COMBO_KillFocus( LPHEADCOMBO lphc )
{
   HWND	hWnd = lphc->m_hWnd;

   if (lphc->m_bFocus) {
       CBRollUp( lphc, FALSE, TRUE );
       if (IsWindow(hWnd)) {
           if (lphc->IsDropDownList())
               lphc->m_lbox.SendMessage(LB_CARETOFF, 0, 0);

			lphc->m_bFocus = false;

           /* redraw text */
	   if (!lphc->m_bHasEditCtrl)
	     InvalidateRect(lphc->m_hWnd, &lphc->rcText, TRUE);

           CB_NOTIFY( lphc, CBN_KILLFOCUS );
       }
   }
}

/***********************************************************************
 *           COMBO_Command
 */
static LRESULT COMBO_Command( LPHEADCOMBO lphc, WPARAM wParam, HWND hWnd )
{
   if (lphc->m_bHasEditCtrl && lphc->m_edit == hWnd) {
       /* ">> 8" makes gcc generate jump-table instead of cmp ladder */

       switch( HIWORD(wParam) >> 8 ) {
	   case (EN_SETFOCUS >> 8):
			COMBO_SetFocus( lphc );
	        break;

	   case (EN_KILLFOCUS >> 8):
		/* NOTE: it seems that Windows' edit control sends an
		 * undocumented message WM_USER + 0x1B instead of this
		 * notification (only when it happens to be a part of
		 * the combo). ?? - AK.
		 */

		COMBO_KillFocus( lphc );
		break;


	   case (EN_CHANGE >> 8):
	       /*
	        * In some circumstances (when the selection of the combobox
		* is changed for example) we don't want the EN_CHANGE notification
		* to be forwarded to the parent of the combobox. This code
		* checks a flag that is set in these occasions and ignores the
		* notification.
	        */
		if (lphc->m_bNoLBSelect)
			lphc->wState &= ~CBF_NOLBSELECT;
		else
		  CBUpdateLBox( lphc, lphc->m_bDropped);

			if (!lphc->m_bNoEditNotify)
				CB_NOTIFY( lphc, CBN_EDITCHANGE );
		break;

	   case (EN_UPDATE >> 8):
	        if (!lphc->m_bNoEditNotify)
		  CB_NOTIFY( lphc, CBN_EDITUPDATE );
		break;

	   case (EN_ERRSPACE >> 8):
		CB_NOTIFY( lphc, CBN_ERRSPACE );
       }
   }
   else if (lphc->m_lbox == hWnd )
   {
       switch( (short)HIWORD(wParam) )
       {
	   case LBN_ERRSPACE:
		CB_NOTIFY( lphc, CBN_ERRSPACE );
		break;

	   case LBN_DBLCLK:
		CB_NOTIFY( lphc, CBN_DBLCLK );
		break;

	   case LBN_SELCHANGE:
	   case LBN_SELCANCEL:

               TRACE("[%p]: lbox selection change [%x]\n", lphc->m_hWnd, lphc->wState );

		if( HIWORD(wParam) == LBN_SELCHANGE)
		{
		   if (lphc->m_bHasEditCtrl) {
		       INT index = lphc->m_lbox.GetCurSel();
		       lphc->m_bNoLBSelect = true;
		       CBUpdateEdit( lphc, index );
		       /* select text in edit, as Windows does */
				lphc->m_edit.SetSel(0, -1);
		   }
		   else
		       InvalidateRect(lphc->m_hWnd, &lphc->rcText, TRUE);
		}

		/* do not roll up if selection is being tracked
		 * by arrowkeys in the dropdown listbox */
                if( (lphc->m_bDropped && !(lphc->wState & CBF_NOROLLUP)) )
                {
                   CBRollUp( lphc, (HIWORD(wParam) == LBN_SELCHANGE), TRUE );
                }
		else lphc->wState &= ~CBF_NOROLLUP;

		CB_NOTIFY( lphc, CBN_SELCHANGE );

		/* fall through */

	   case LBN_SETFOCUS:
	   case LBN_KILLFOCUS:
		/* nothing to do here since ComboLBox always resets the focus to its
		 * combo/edit counterpart */
		 break;
       }
   }
   return 0;
}

/***********************************************************************
 *           COMBO_ItemOp
 *
 * Fixup an ownerdrawn item operation and pass it up to the combobox owner.
 */
static LRESULT COMBO_ItemOp( LPHEADCOMBO lphc, UINT msg, LPARAM lParam )
{
   HWND hWnd = lphc->m_hWnd;
   UINT id = (UINT)GetWindowLongPtrW( hWnd, GWLP_ID );

   TRACE("[%p]: ownerdraw op %04x\n", lphc->m_hWnd, msg );

   switch( msg )
   {
   case WM_DELETEITEM:
       {
           DELETEITEMSTRUCT *lpIS = (DELETEITEMSTRUCT *)lParam;
           lpIS->CtlType  = ODT_COMBOBOX;
           lpIS->CtlID    = id;
           lpIS->hwndItem = hWnd;
           break;
       }
   case WM_DRAWITEM:
       {
           DRAWITEMSTRUCT *lpIS = (DRAWITEMSTRUCT *)lParam;
           lpIS->CtlType  = ODT_COMBOBOX;
           lpIS->CtlID    = id;
           lpIS->hwndItem = hWnd;
           break;
       }
   case WM_COMPAREITEM:
       {
           COMPAREITEMSTRUCT *lpIS = (COMPAREITEMSTRUCT *)lParam;
           lpIS->CtlType  = ODT_COMBOBOX;
           lpIS->CtlID    = id;
           lpIS->hwndItem = hWnd;
           break;
       }
   case WM_MEASUREITEM:
       {
           MEASUREITEMSTRUCT *lpIS = (MEASUREITEMSTRUCT *)lParam;
           lpIS->CtlType  = ODT_COMBOBOX;
           lpIS->CtlID    = id;
           break;
       }
   }
   return SendMessageW(lphc->owner, msg, id, lParam);
}


/***********************************************************************
 *           COMBO_GetTextW
 */
static LRESULT COMBO_GetTextW( LPHEADCOMBO lphc, INT count, LPWSTR buf )
{
    INT length;

	if (lphc->m_bHasEditCtrl)
		return lphc->m_edit.GetWindowText(buf, count);

    /* get it from the listbox */

    if (!count || !buf) return 0;
    if (lphc->m_lbox.IsWindow()) {
		int idx = lphc->m_lbox.GetCurSel();
		if (idx == LB_ERR) goto error;
		length = lphc->m_lbox.GetTextLen(idx);
        if (length == LB_ERR) goto error;

        /* 'length' is without the terminating character */
        if (length >= count)
        {
            LPWSTR lpBuffer = HeapAlloc(GetProcessHeap(), 0, (length + 1) * sizeof(WCHAR));
            if (!lpBuffer) goto error;
            length = lphc->m_lbox.GetText(idx, lpBuffer);

            /* truncate if buffer is too short */
            if (length != LB_ERR)
            {
                lstrcpynW( buf, lpBuffer, count );
                length = count;
            }
            HeapFree( GetProcessHeap(), 0, lpBuffer );
        }
        else
        	length = lphc->m_lbox.GetText(idx, buf);

        if (length == LB_ERR) return 0;
        return length;
    }

 error:  /* error - truncate string, return zero */
    buf[0] = 0;
    return 0;
}

/***********************************************************************
 *           CBResetPos
 *
 * This function sets window positions according to the updated
 * component placement struct.
 */
static void CBResetPos(
  LPHEADCOMBO lphc,
  LPRECT      rectEdit,
  LPRECT      rectLB,
  BOOL        bRedraw)
{
	BOOL bDrop = !lphc->IsSimple();

	// NOTE: logs sometimes have WM_LBUTTONUP before a cascade of sizing messages

	if (lphc->m_bHasEditCtrl)
		lphc->m_edit.SetWindowPos(NULL,
								rectEdit->left, rectEdit->top,
								rectEdit->right - rectEdit->left,
								rectEdit->bottom - rectEdit->top,
								SWP_NOZORDER | SWP_NOACTIVATE | ((bDrop) ? SWP_NOREDRAW : 0) );

		lphc->m_lbox.SetWindowPos(0, rectLB->left, rectLB->top,
								rectLB->right - rectLB->left, rectLB->bottom - rectLB->top,
								SWP_NOACTIVATE | SWP_NOZORDER | ((bDrop) ? SWP_NOREDRAW : 0) );

	if (bDrop) {
		if (lphc->m_bDropped) {
           lphc->m_bDropped = FALSE;
			lphc->m_lbox.ShowWindow(SW_HIDE);
		}

       if (bRedraw && !lphc->m_bNoRedraw)
           RedrawWindow( lphc->m_hWnd, NULL, 0, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW );
   }
}


/***********************************************************************
 *           COMBO_Size
 */
static void COMBO_Size( LPHEADCOMBO lphc, BOOL bRedraw )
  {
  CBCalcPlacement(lphc->m_hWnd,
		  lphc,
		  &lphc->rcText,
		  &lphc->rcButton,
		  &lphc->rcDropped);

  CBResetPos( lphc, &lphc->rcText, &lphc->rcDropped, bRedraw );
}


/***********************************************************************
 *           COMBO_Font
 */
static void COMBO_Font( LPHEADCOMBO lphc, HFONT hFont, BOOL bRedraw )
{
  /*
   * Set the font
   */
  lphc->hFont = hFont;

  /*
   * Propagate to owned windows.
   */
  if (lphc->m_bHasEditCtrl)
		lphc->m_edit.SetFont(hFont, bRedraw);
	lphc->m_lbox.SetFont(hFont, bRedraw);

  /*
   * Redo the layout of the control.
   */
	if (lphc->IsSimple()) {
    CBCalcPlacement(lphc->m_hWnd,
		    lphc,
		    &lphc->rcText,
		    &lphc->rcButton,
		    &lphc->rcDropped);

    CBResetPos( lphc, &lphc->rcText, &lphc->rcDropped, TRUE );
  }
  else
  {
    CBForceDummyResize(lphc);
  }
}


/***********************************************************************
 *           COMBO_SetItemHeight
 */
static LRESULT COMBO_SetItemHeight( LPHEADCOMBO lphc, INT index, INT height )
{
	LRESULT	lRet = CB_ERR;

	if( index == -1 ) /* set text field height */
	{
		if (height < 32768) {
			lphc->editHeight = height;

			// Redo the layout of the control.
			if (lphc->IsSimple()) {
				CBCalcPlacement(lphc->m_hWnd, lphc, &lphc->rcText, &lphc->rcButton, &lphc->rcDropped);
				CBResetPos( lphc, &lphc->rcText, &lphc->rcDropped, TRUE );
			}
			else
				CBForceDummyResize(lphc);
			lRet = height;
		}
	}
	else if ( lphc->IsOwnerDraw() )	/* set listbox item height */
		lRet = lphc->m_lbox.SetItemHeight(index, height);
	return lRet;
}

/***********************************************************************
 *           COMBO_SelectString
 */
static LRESULT COMBO_SelectString( LPHEADCOMBO lphc, INT start, LPARAM pText)
{
   INT index = lphc->m_lbox.SelectString(start, pText);
   if( index >= 0 )
   {
     if (lphc->m_bHasEditCtrl)
       CBUpdateEdit( lphc, index );
     else
     {
       InvalidateRect(lphc->m_hWnd, &lphc->rcText, TRUE);
     }
   }
   return (LRESULT)index;
}

/***********************************************************************
 *           COMBO_LButtonDown
 */
static void COMBO_LButtonDown( LPHEADCOMBO lphc, LPARAM lParam )
{
   POINT     pt;
   BOOL      bButton;
   HWND      hWnd = lphc->m_hWnd;

   pt.x = (short)LOWORD(lParam);
   pt.y = (short)HIWORD(lParam);
   bButton = PtInRect(&lphc->rcButton, pt);

	if (lphc->IsDropDownList() || (bButton && lphc->IsDropDown())) {
		lphc->m_bPressed = true;
		if (lphc->m_bDropped) {
			// got a click to cancel selection

           lphc->m_bPressed = false;
           CBRollUp( lphc, TRUE, FALSE );
	   if( !IsWindow( hWnd ) ) return;

           if (lphc->m_bCaptured) {
				lphc->m_bCaptured = false;
				ReleaseCapture();
			}
		}
       else
       {
	   /* drop down the listbox and start tracking */

           lphc->m_bCaptured = true;
           SetCapture( hWnd );
           CBDropDown( lphc );
       }
       if( bButton ) CBRepaintButton( lphc );
   }
}

/***********************************************************************
 *           COMBO_LButtonUp
 *
 * Release capture and stop tracking if needed.
 */
static void COMBO_LButtonUp( LPHEADCOMBO lphc )
{
	if (lphc->m_bCaptured) {
		lphc->m_bCaptured = false;
		if (lphc->IsDropDown()) {
			INT index = CBUpdateLBox(lphc, TRUE);
			// Update edit only if item is in the list
			if (index >= 0) {
				lphc->m_bNoLBSelect = true;
				CBUpdateEdit(lphc, index);
				lphc->m_bNoLBSelect = false;
			}
		}
		ReleaseCapture();
		lphc->m_lbox.SetCapture();
	}

	if (lphc->m_bPressed) {
		lphc->m_bPressed = false;
		CBRepaintButton( lphc );
	}
}

/***********************************************************************
 *           COMBO_MouseMove
 *
 * Two things to do - track combo button and release capture when
 * pointer goes into the listbox.
 */
static void COMBO_MouseMove( LPHEADCOMBO lphc, WPARAM wParam, LPARAM lParam )
{
   POINT  pt;
   RECT   lbRect;

   pt.x = (short)LOWORD(lParam);
   pt.y = (short)HIWORD(lParam);

   if (lphc->m_bPressed) {
     BOOL bButton;

     bButton = PtInRect(&lphc->rcButton, pt);

     if( !bButton )
     {
       lphc->m_bPressed = false;
       CBRepaintButton( lphc );
     }
   }

   lphc->m_lbox.GetClientRect(&lbRect);
   MapWindowPoints( lphc->m_hWnd, lphc->hWndLBox, &pt, 1 );
	if (PtInRect(&lbRect, pt)) {
       lphc->m_bCaptured = false;
       ReleaseCapture();
       if (lphc->IsDropDown())
       		CBUpdateLBox( lphc, TRUE );

       /* hand over pointer tracking */
       lphc->m_lbox.SendMessage(WM_LBUTTONDOWN, wParam, lParam);
   }
}

static LRESULT COMBO_GetComboBoxInfo(LPHEADCOMBO lphc, COMBOBOXINFO *pcbi)
{
    if (!pcbi || (pcbi->cbSize < sizeof(COMBOBOXINFO)))
        return FALSE;

    pcbi->rcItem = lphc->rcText;
    pcbi->rcButton = lphc->rcButton;
    pcbi->stateButton = 0;
    if (lphc->m_bPressed)
        pcbi->stateButton |= STATE_SYSTEM_PRESSED;
    if (IsRectEmpty(&lphc->rcButton))
        pcbi->stateButton |= STATE_SYSTEM_INVISIBLE;
    pcbi->hwndCombo = lphc->m_hWnd;
    pcbi->hwndItem = lphc->m_edit;
    pcbi->hwndList = lphc->m_lbox;
    return TRUE;
}

static char *strdupA(LPCSTR str)
{
    char *ret;
    DWORD len;

    if(!str) return NULL;

    len = strlen(str);
    ret = HeapAlloc(GetProcessHeap(), 0, len + 1);
    memcpy(ret, str, len + 1);
    return ret;
}

/***********************************************************************
 *           ComboWndProc_common
 *
 * http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/commctls/comboboxes/comboboxes.asp
 */
static LRESULT ComboWndProc_common( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
      LPHEADCOMBO lphc = (LPHEADCOMBO)GetWindowLongPtrW( hwnd, 0 );

      TRACE("[%p]: msg %s wp %08x lp %08lx\n",
            hwnd, SPY_GetMsgName(message, hwnd), wParam, lParam );

      if( lphc || message == WM_NCCREATE )
      switch(message)
      {

	/* System messages */

     	case WM_NCCREATE:
	{
		LONG style = ((LPCREATESTRUCTW)lParam)->style;
		return COMBO_NCCreate(hwnd, style);
	}
     	case WM_NCDESTROY:
		COMBO_NCDestroy(lphc);
		break;/* -> DefWindowProc */

     	case WM_CREATE:
	{
		HWND hwndParent = ((LPCREATESTRUCTW)lParam)->hwndParent;
		LONG style = ((LPCREATESTRUCTW)lParam)->style;
        return COMBO_Create(hwnd, lphc, hwndParent, style);
	}

        case WM_PRINTCLIENT:
		/* Fallthrough */
     	case WM_PAINT:
		/* wParam may contain a valid HDC! */
		return  COMBO_Paint(lphc, (HDC)wParam);

	case WM_ERASEBKGND:
                /* do all painting in WM_PAINT like Windows does */
                return 1;

	case WM_GETDLGCODE:
	{
		LRESULT result = DLGC_WANTARROWS | DLGC_WANTCHARS;
		if (lParam && (((LPMSG)lParam)->message == WM_KEYDOWN))
		{
		   int vk = (int)((LPMSG)lParam)->wParam;

		   if ((vk == VK_RETURN || vk == VK_ESCAPE) && lphc->m_bDropped)
		       result |= DLGC_WANTMESSAGE;
		}
		return  result;
	}
	case WM_WINDOWPOSCHANGING:
	        return  COMBO_WindowPosChanging(hwnd, lphc, (LPWINDOWPOS)lParam);
    case WM_WINDOWPOSCHANGED:
        /* SetWindowPos can be called on a Combobox to resize its Listbox.
         * In that case, the Combobox itself will not be resized, so we won't
         * get a WM_SIZE. Since we still want to update the Listbox, we have to
         * do it here.
         */
        /* we should not force repainting on WM_WINDOWPOSCHANGED, it breaks
         * Z-order based painting.
         */
        /* fall through */
	case WM_SIZE:
	        if( lphc->m_lbox.IsWindow() &&
		  !(lphc->wState & CBF_NORESIZE) ) COMBO_Size( lphc, message == WM_SIZE );
		return  TRUE;
	case WM_SETFONT:
		COMBO_Font( lphc, (HFONT)wParam, (BOOL)lParam );
		return  TRUE;
	case WM_GETFONT:
		return  (LRESULT)lphc->hFont;
	case WM_SETFOCUS:
		if (lphc->m_bHasEditCtrl)
		    lphc->m_edit.SetFocus();
		else
		    COMBO_SetFocus( lphc );
		return  TRUE;
	case WM_KILLFOCUS:
            {
                HWND hwndFocus = WIN_GetFullHandle( (HWND)wParam );
		if( !hwndFocus ||
		    (hwndFocus != lphc->m_edit && hwndFocus != lphc->m_lbox ))
		    COMBO_KillFocus( lphc );
		return  TRUE;
            }
	case WM_COMMAND:
		return  COMBO_Command( lphc, wParam, WIN_GetFullHandle( (HWND)lParam ) );
	case WM_GETTEXT:
            return COMBO_GetTextW( lphc, wParam, (LPWSTR)lParam );
	case WM_SETTEXT:
	case WM_GETTEXTLENGTH:
	case WM_CLEAR:
                if ((message == WM_GETTEXTLENGTH) && !ISWIN31 && !lphc->m_bHasEditCtrl) {
                    int j = lphc->m_lbox.GetCurSel();
                    if (j == -1) return 0;
                    return lphc->m_lbox.GetTextLen(j);
                }
		else if (lphc->m_bHasEditCtrl) {
		    LRESULT ret;
		    lphc->m_bNoEditNotify = true;
		    ret = lphc->m_edit.SendMessage(message, wParam, lParam);
		    lphc->m_bNoEditNotify = false;
		    return ret;
		}
		else return CB_ERR;
	case WM_CUT:
        case WM_PASTE:
	case WM_COPY:
		if (lphc->m_bHasEditCtrl)
		    return lphc->m_edit.SendMessage(message, wParam, lParam);
		else
			return CB_ERR;

	case WM_DRAWITEM:
	case WM_DELETEITEM:
	case WM_COMPAREITEM:
	case WM_MEASUREITEM:
		return COMBO_ItemOp(lphc, message, lParam);
	case WM_ENABLE:
		if (lphc->m_bHasEditCtrl)
		    lphc->m_edit.EnableWindow((BOOL)wParam);
		lphc->m_lbox.EnableWindow((BOOL)wParam);

		/* Force the control to repaint when the enabled state changes. */
		InvalidateRect(lphc->m_hWnd, NULL, TRUE);
		return  TRUE;
	case WM_SETREDRAW:
		if( wParam )
		    lphc->m_bNoRedraw = false;
		else
		    lphc->m_bNoRedraw = true;

		if (lphc->m_bHasEditCtrl)
			lphc->m_edit.SendMessage(message, wParam, lParam);
		lphc->m_lbox.SendMessage(message, wParam, lParam);
		return  0;
	case WM_SYSKEYDOWN:
		if( KEYDATA_ALT & HIWORD(lParam) )
		    if( wParam == VK_UP || wParam == VK_DOWN )
			COMBO_FlipListbox( lphc, FALSE, FALSE );
                return  0;

	case WM_CHAR:
	case WM_IME_CHAR:
	case WM_KEYDOWN:
	{
		HWND hwndTarget;

		if ((wParam == VK_RETURN || wParam == VK_ESCAPE) &&
		     lphc->m_bDropped)
		{
		   CBRollUp( lphc, wParam == VK_RETURN, FALSE );
		   return TRUE;
		}
               else if ((wParam == VK_F4) && !(lphc->wState & CBF_EUI))
               {
                  COMBO_FlipListbox( lphc, FALSE, FALSE );
                  return TRUE;
               }

		if (lphc->m_bHasEditCtrl)
		    hwndTarget = lphc->m_edit;
		else
		    hwndTarget = lphc->m_lbox;

		return SendMessage(hwndTarget, message, wParam, lParam);
	}
	case WM_LBUTTONDOWN:
		if (!lphc->m_bFocus) SetFocus( lphc->m_hWnd );
		if (lphc->m_bFocus) COMBO_LButtonDown( lphc, lParam );
		return  TRUE;
	case WM_LBUTTONUP:
		COMBO_LButtonUp( lphc );
		return  TRUE;
	case WM_MOUSEMOVE:
		if (lphc->m_bCaptured)
		    COMBO_MouseMove( lphc, wParam, lParam );
		return  TRUE;

        case WM_MOUSEWHEEL:
                if (wParam & (MK_SHIFT | MK_CONTROL))
                    return DefWindowProcW(hwnd, message, wParam, lParam);

                if (GET_WHEEL_DELTA_WPARAM(wParam) > 0) return SendMessageW(hwnd, WM_KEYDOWN, VK_UP, 0);
                if (GET_WHEEL_DELTA_WPARAM(wParam) < 0) return SendMessageW(hwnd, WM_KEYDOWN, VK_DOWN, 0);
                return TRUE;

	/* Combo messages */

	case CB_ADDSTRING16:
		if( lphc->HasStrings() ) lParam = (LPARAM)MapSL(lParam);
		/* fall through */
	case CB_ADDSTRING:
		if( lphc->dwStyle & CBS_LOWERCASE )
			CharLowerW((LPWSTR)lParam);
		else if( lphc->dwStyle & CBS_UPPERCASE )
			CharUpperW((LPWSTR)lParam);
		return lphc->m_lbox.AddString((LPCTSTR)lParam);
	case CB_INSERTSTRING16:
		wParam = (INT)(INT16)wParam;
		if( lphc->HasStrings() ) lParam = (LPARAM)MapSL(lParam);
		/* fall through */
	case CB_INSERTSTRING:
		if( lphc->dwStyle & CBS_LOWERCASE )
			CharLowerW((LPWSTR)lParam);
		else if( lphc->dwStyle & CBS_UPPERCASE )
			CharUpperW((LPWSTR)lParam);
		return lphc->m_lbox.InsertString((int)wParam, (LPCTSTR)lParam);
	case CB_DELETESTRING16:
	case CB_DELETESTRING:
		return lphc->m_lbox.DeleteString((int)wParam);
	case CB_SELECTSTRING16:
		wParam = (INT)(INT16)wParam;
		if( lphc->HasStrings() ) lParam = (LPARAM)MapSL(lParam);
		/* fall through */
	case CB_SELECTSTRING:
		return COMBO_SelectString(lphc, (INT)wParam, lParam);
	case CB_FINDSTRING16:
		wParam = (INT)(INT16)wParam;
		if( lphc->HasStrings() ) lParam = (LPARAM)MapSL(lParam);
		/* fall through */
	case CB_FINDSTRING:
		return lphc->m_lbox.FindString((int)wParam, (LPCTSTR)lParam);
	case CB_FINDSTRINGEXACT16:
		wParam = (INT)(INT16)wParam;
		if( lphc->HasStrings() ) lParam = (LPARAM)MapSL(lParam);
		/* fall through */
	case CB_FINDSTRINGEXACT:
		return lphc->m_lbox.FindStringExact((int)wParam, (LPCTSTR)lParam);
	case CB_SETITEMHEIGHT16:
		wParam = (INT)(INT16)wParam;	/* signed integer */
		/* fall through */
	case CB_SETITEMHEIGHT:
		return  COMBO_SetItemHeight( lphc, (INT)wParam, (INT)lParam);
	case CB_GETITEMHEIGHT16:
		wParam = (INT)(INT16)wParam;
		/* fall through */
	case CB_GETITEMHEIGHT:
		if( (INT)wParam >= 0 )	/* listbox item */
			return lphc->m_lbox.GetItemHeight((int)wParam);
		return  CBGetTextAreaHeight(hwnd, lphc);
	case CB_RESETCONTENT16:
	case CB_RESETCONTENT:
		lphc->m_lbox.ResetContent();
		if (lphc->m_bHasEditCtrl && lphc->HasStrings() ) {
		    static const WCHAR empty_stringW[] = { 0 };
			lphc->m_edit.SetWindowText(empty_stringW);
		}
		else
			InvalidateRect(lphc->m_hWnd, NULL, TRUE);
		return  TRUE;
	case CB_INITSTORAGE:
		return lphc->m_lbox.InitStorage((int)wParam, (UINT)lParam);
	case CB_GETHORIZONTALEXTENT:
		return lphc->m_lbox.GetHorizontalExtent();
	case CB_SETHORIZONTALEXTENT:
		return lphc->m_lbox.SetHorizontalExtent((int)wParam);
	case CB_GETTOPINDEX:
		return lphc->m_lbox.GetTopIndex();
	case CB_GETLOCALE:
		return lphc->m_lbox.GetLocale();
	case CB_SETLOCALE:
		return lphc->m_lbox.SetLocal(wParam);
	case CB_GETDROPPEDWIDTH:
		if ( lphc->droppedWidth )
			return  lphc->droppedWidth;
		return  lphc->rcDropped.Wdith();
	case CB_SETDROPPEDWIDTH:
		if (!lphc->IsSimple() && (int)wParam < 32768)
			lphc->droppedWidth = (INT)wParam;
		return  CB_ERR;
	case CB_GETDROPPEDCONTROLRECT16:
		lParam = (LPARAM)MapSL(lParam);
		if( lParam )
		{
           RECT r;
           RECT16 *r16 = (RECT16 *)lParam;
           CBGetDroppedControlRect( lphc, &r );
           r16->left   = r.left;
           r16->top    = r.top;
           r16->right  = r.right;
           r16->bottom = r.bottom;
		}
		return  CB_OKAY;
	case CB_GETDROPPEDCONTROLRECT:
		if( lParam ) CBGetDroppedControlRect(lphc, (LPRECT)lParam );
		return  CB_OKAY;
	case CB_GETDROPPEDSTATE16:
	case CB_GETDROPPEDSTATE:
		return lphc->m_bDropped;
	case CB_DIR16:
		return SendMessageA(lphc->hWndLBox, LB_DIR16, wParam, lParam);
	case CB_DIR:
		return SendMessageW(lphc->hWndLBox, LB_DIR, wParam, lParam);

	case CB_SHOWDROPDOWN16:
	case CB_SHOWDROPDOWN:
		if (!lphc->IsSimple()) {
		    if( wParam )
		    {
			if (!lphc->m_bDropped)
			    CBDropDown( lphc );
		    }
		    else if (lphc->m_bDropped)
		            CBRollUp( lphc, FALSE, TRUE );
		}
		return  TRUE;
	case CB_GETCOUNT16:
	case CB_GETCOUNT:
		return SendMessageW(lphc->hWndLBox, LB_GETCOUNT, 0, 0);
	case CB_GETCURSEL16:
	case CB_GETCURSEL:
		return SendMessageW(lphc->hWndLBox, LB_GETCURSEL, 0, 0);
	case CB_SETCURSEL16:
		wParam = (INT)(INT16)wParam;
		/* fall through */
	case CB_SETCURSEL:
		lParam = SendMessageW(lphc->hWndLBox, LB_SETCURSEL, wParam, 0);
	        if( lParam >= 0 )
	            SendMessageW(lphc->hWndLBox, LB_SETTOPINDEX, wParam, 0);

		/* no LBN_SELCHANGE in this case, update manually */
		if (lphc->m_bHasEditCtrl)
		    CBUpdateEdit( lphc, (INT)wParam );
		else
		    InvalidateRect(lphc->m_hWnd, &lphc->rcText, TRUE);
		lphc->wState &= ~CBF_SELCHANGE;
	        return  lParam;
	case CB_GETLBTEXT16:
		wParam = (INT)(INT16)wParam;
		lParam = (LPARAM)MapSL(lParam);
		/* fall through */
	case CB_GETLBTEXT:
		return SendMessageW(lphc->hWndLBox, LB_GETTEXT, wParam, lParam);
	case CB_GETLBTEXTLEN16:
		wParam = (INT)(INT16)wParam;
		/* fall through */
	case CB_GETLBTEXTLEN:
                return SendMessageW(lphc->hWndLBox, LB_GETTEXTLEN, wParam, 0);
	case CB_GETITEMDATA16:
		wParam = (INT)(INT16)wParam;
		/* fall through */
	case CB_GETITEMDATA:
		return SendMessageW(lphc->hWndLBox, LB_GETITEMDATA, wParam, 0);
	case CB_SETITEMDATA16:
		wParam = (INT)(INT16)wParam;
		/* fall through */
	case CB_SETITEMDATA:
		return SendMessageW(lphc->hWndLBox, LB_SETITEMDATA, wParam, lParam);
	case CB_GETEDITSEL16:
		wParam = lParam = 0;   /* just in case */
		/* fall through */
	case CB_GETEDITSEL:
		/* Edit checks passed parameters itself */
		if (lphc->m_bHasEditCtrl)
		    return lphc->m_edit.SendMessage(EM_GETSEL, wParam, lParam);
		return  CB_ERR;
	case CB_SETEDITSEL16:
	case CB_SETEDITSEL:
		if (lphc->m_bHasEditCtrl)
			return lphc->m_edit.SetSel((INT)(INT16)LOWORD(lParam), (INT)(INT16)HIWORD(lParam));
		return  CB_ERR;
	case CB_SETEXTENDEDUI16:
	case CB_SETEXTENDEDUI:
                if (lphc->IsSimple())
                    return  CB_ERR;
		if( wParam )
		    lphc->wState |= CBF_EUI;
		else lphc->wState &= ~CBF_EUI;
		return  CB_OKAY;
	case CB_GETEXTENDEDUI16:
	case CB_GETEXTENDEDUI:
		return  (lphc->wState & CBF_EUI) ? TRUE : FALSE;
	case CB_GETCOMBOBOXINFO:
		return COMBO_GetComboBoxInfo(lphc, (COMBOBOXINFO *)lParam);
	case CB_LIMITTEXT:
		if (lphc->m_bHasEditCtrl)
			return lphc->m_edit.LimitText(wParam);
	default:
		if (message >= WM_USER)
		    WARN("unknown msg WM_USER+%04x wp=%04x lp=%08lx\n",
			message - WM_USER, wParam, lParam );
		break;
      }
      return DefWindowProcW(hwnd, message, wParam, lParam);
}

/***********************************************************************
 *           ComboWndProcW
 */
static LRESULT WINAPI ComboWndProcW( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    if (!IsWindow(hwnd)) return 0;
    return ComboWndProc_common( hwnd, message, wParam, lParam);
}

/*************************************************************************
 *           GetComboBoxInfo   (USER32.@)
 */
BOOL WINAPI GetComboBoxInfo(HWND hwndCombo,      /* [in] handle to combo box */
			    PCOMBOBOXINFO pcbi   /* [in/out] combo box information */)
{
    return SendMessageW(hwndCombo, CB_GETCOMBOBOXINFO, 0, (LPARAM)pcbi);
}
