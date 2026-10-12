#include "stdafx.h"
#include <limits.h>
#include "resource.h"
#include "lscore.h"
#include "fnc.h"
#include "common.h"
#include "stdio.h"
#include "colortable.h"
#include "spin_sup.h"
#include "picker.h"
#include "tooltip_sup.h"
#include "gdiplus_sup.h"

extern "C" BOOL WINAPI lccAlphaBlend(HDC, int, int, int, int, HDC, int, int, DWORD);

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CUSTOM_BOX_VALUE		-2
#define MAX_COLORS				256
#define CPN_PROGRESS			999

#define IDC_HUE					703
#define IDC_SAT					704
#define IDC_LUM					705
#define IDC_RED					706
#define IDC_GREEN				707
#define IDC_BLUE				708

#define BGR(b,g,r)				((DWORD)(((BYTE)(b)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(r))<<16)))
#define bgrBValue(v)			(LOBYTE(v))
#define bgrGValue(v)			(LOBYTE(((WORD)(v)) >> 8))
#define bgrRValue(v)			(LOBYTE((v)>>16))

////////////////////////////////////////////////////////////////////////
//

class CHexEdit : public CWindowImpl<CHexEdit, CEdit>
{
public:
	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

BEGIN_MSG_MAP(CHexEdit)
	MESSAGE_HANDLER(WM_CHAR, OnHandlerChar)
END_MSG_MAP()

	LRESULT OnHandlerChar(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		TCHAR ch = (TCHAR)wParam;
		if ((ch < ' ') ||
			('0' <= ch && ch <= '9') ||
			('a' <= ch && ch <= 'f') ||
			('A' <= ch && ch <= 'F')) {
			bHandled = FALSE;
			return 1;
		}
		MessageBeep(0);
		return 0;
	}
};

static void UpdateColorValue(HWND hdlg)
{
	if (GetWindowLongPtr(hdlg, DWLP_USER)) return;

	int r = GetDlgItemInt(hdlg, IDC_RED,   NULL, FALSE);
	int g = GetDlgItemInt(hdlg, IDC_GREEN, NULL, FALSE);
	int b = GetDlgItemInt(hdlg, IDC_BLUE,  NULL, FALSE);
	CString str; str.Format(_T("%06X"), BGR(b,g,r));
	SetDlgItemText(hdlg, IDC_RGBVALUE, str);
}

static UINT_PTR CALLBACK _cchook(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
	{
		SetWindowLongPtr(hdlg, DWLP_USER, 0);
		Spin_SetRange(GetDlgItem(hdlg, IDC_HSPIN), 0, 239);
		Spin_SetRange(GetDlgItem(hdlg, IDC_SSPIN), 0, 240);
		Spin_SetRange(GetDlgItem(hdlg, IDC_LSPIN), 0, 240);
		Spin_SetRange(GetDlgItem(hdlg, IDC_RSPIN), 0, 255);
		Spin_SetRange(GetDlgItem(hdlg, IDC_GSPIN), 0, 255);
		Spin_SetRange(GetDlgItem(hdlg, IDC_BSPIN), 0, 255);
		SendDlgItemMessage(hdlg, IDC_RGBVALUE, EM_LIMITTEXT, 6, 0);
		CHexEdit* pHexEdit = new CHexEdit;
		pHexEdit->SubclassWindow(GetDlgItem(hdlg, IDC_RGBVALUE));
		UpdateColorValue(hdlg);
		break;
	}
	case WM_COMMAND:
		if (HIWORD(wParam) == EN_CHANGE) {
			switch (LOWORD(wParam)) {
			case IDC_RED:
			case IDC_BLUE:
			case IDC_GREEN:
				UpdateColorValue(hdlg);
				break;
			case IDC_RGBVALUE:
				if (GetFocus() == GetDlgItem(hdlg, IDC_RGBVALUE) &&
					GetCapture() == NULL &&
					GetKeyState(VK_LBUTTON) >= 0) {
					CString str;
					LPTSTR p = str.GetBuffer(256);
					*p = 0;
					GetDlgItemText(hdlg, IDC_RGBVALUE, p, 256);
					str.ReleaseBuffer();
					int value = 0;
					if (str.GetLength() > 0) {
						str.Insert(0, _T("0x"));
						StrToIntEx(str, STIF_SUPPORT_HEX, &value);
					}
					SetWindowLongPtr(hdlg, DWLP_USER, 1);
					str.Format(_T("%d"), bgrRValue(value));	SetDlgItemText(hdlg, IDC_RED, str);
					str.Format(_T("%d"), bgrGValue(value));	SetDlgItemText(hdlg, IDC_GREEN, str);
					str.Format(_T("%d"), bgrBValue(value));	SetDlgItemText(hdlg, IDC_BLUE, str);
					SetWindowLongPtr(hdlg, DWLP_USER, 0);
				}
				break;
			}
		}
		break;
	}
	return FALSE;
}

extern "C"
BOOL WINAPI lccSelectCustomColor(HWND hOwner, COLORREF rgb, COLORREF* rgbNew)
{
	CHOOSECOLOR cc;
	DWORD ccols[16];

	memset(&cc, 0, sizeof(cc));
	for (int i = 0; i < 16; i++)
		ccols[i] = RGB(255, 255, 255);

	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = hOwner;
	cc.rgbResult = rgb;
	cc.lpCustColors = ccols;
	cc.Flags = CC_FULLOPEN | CC_ANYCOLOR | CC_RGBINIT |
			   CC_ENABLETEMPLATE | CC_ENABLEHOOK;
	cc.hInstance = (HWND)MODINST;
	cc.lpTemplateName = _MKIR(IDD_COMM_COLOR);
	cc.lpfnHook = (LPCCHOOKPROC)_cchook;
	if (ChooseColor(&cc)) {
		*rgbNew = cc.rgbResult;
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////
// CColorPaletteWindow

static TCHAR szCustomText[] = _T("その他の色...");

class CColorPaletteWindow
		: public CWindowImpl<CColorPaletteWindow>
		, public CToolTipUsing<CColorPaletteWindow>
{
// ※※このウィンドウクラスはオートデリートしない
public:
	UINT m_nResult;
	COLORREF m_clrSelect;

	DECLARE_WND_CLASS_EX(_T("ColorPalette"), CS_SAVEBITS, COLOR_WINDOW)

private:
	int	m_colors;
	int m_numCol;
	int m_numRow;
	int m_boxSize;
	int m_hotIndex;
	int m_selIndex;
	int m_nMargin;
	int m_gap;
	CRect m_rcCus;
	CRect m_rcLis;
	CFont m_font;
	HPALETTE m_hpal;
	COLORREF m_clrOriginal;
	COLORREF* m_pcrTable;
	bool m_bClicked:1;
	gdiplus_using m_gu;

public:
	CColorPaletteWindow()
		: m_hpal(NULL)
		, m_gap(4)
		, m_bClicked(false)
	{
	}

	~CColorPaletteWindow()
	{
		if (m_hpal)		DeleteObject(m_hpal);
	}

	HWND Create(int x, int y)
	{
		CRect rc(x, y, x + 10, y + 10);
		HWND hwnd = CWindowImpl::Create(NULL, rc, NULL, WS_POPUP | WS_VISIBLE, WS_EX_TOOLWINDOW | WS_EX_TOPMOST);
		ATLASSERT(hwnd != NULL);
		return hwnd;
	}

	void Initialize(CUSTPAL* pTable, COLORREF crInit)
	{
		m_nMargin = GetSystemMetrics(SM_CXEDGE);
		m_nResult = CPN_SELENDCANCEL;
		if (!pTable)
			pTable = &cpNorm;

		m_pcrTable = pTable->table;
		m_colors = pTable->colors;
		if (m_colors > MAX_COLORS)
			m_colors = MAX_COLORS;
		m_numCol = pTable->column;

		m_numRow = 0;
		m_boxSize = 16;
		m_hotIndex = INVALID_COLOR;
		m_selIndex = INVALID_COLOR;
		m_clrSelect = m_clrOriginal = crInit;

		// Idiot check: Make sure the colour square is at least 5 x 5;
		if (m_boxSize - 2 * m_nMargin - 2 < 5)
			m_boxSize = 5 + 2 * m_nMargin + 2;

		// Create the font
		NONCLIENTMETRICS ncm;
		ncm.cbSize = RunTimeHelper::SizeOf_NONCLIENTMETRICS();
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
		m_font.CreateFontIndirect(&ncm.lfMessageFont);

		// Create the palette
	    struct {
			LOGPALETTE    LogPalette;
			PALETTEENTRY  PalEntry[MAX_COLORS];
	    } pal;

		LOGPALETTE* plp = (LOGPALETTE*)&pal;
		plp->palVersion = 0x300;
		plp->palNumEntries = (WORD)m_colors;

		for (int i = 0; i < m_colors; i++) {
			plp->palPalEntry[i].peRed   = GetRValue(m_pcrTable[i]);
			plp->palPalEntry[i].peGreen = GetGValue(m_pcrTable[i]);
			plp->palPalEntry[i].peBlue  = GetBValue(m_pcrTable[i]);
			plp->palPalEntry[i].peFlags = 0;
	    }
		m_hpal = CreatePalette(plp);
		m_nResult = CPN_PROGRESS;
	}

BEGIN_MSG_MAP(CColorPaletteWindow)
	CHAIN_MSG_MAP(CToolTipUsing<CColorPaletteWindow>)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_PRINTCLIENT()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_KILLFOCUS()
	ON_WM_ERASEBKGND()
	ON_NOTIFY_CODE(TTN_NEEDTEXT, OnToolTipNeedText)
	ON_WM_DESTROY()
END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT pcs)
	{
		SetWindowSize();
		FindCellFromColor(m_clrOriginal);

		TTEnable(true);

		SetCapture();
		return 0;
	}

	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		int index = (m_hotIndex != INVALID_COLOR) ? m_hotIndex : m_selIndex;
		int row = GetRow(index);
		int col = GetColumn(index);
		switch (nChar) {
		case VK_DOWN:
			if (row == CUSTOM_BOX_VALUE)
				row = col = 0;
    	    else {
    	        row++;
				if (GetIndex(row, col) < 0)
					row = col = CUSTOM_BOX_VALUE;
	        }
			SetHotItem(GetIndex(row, col));
			break;

		case VK_UP:
			if (row == CUSTOM_BOX_VALUE) {
        	    row = GetRow(m_colors - 1);
				col = GetColumn(m_colors - 1);
	        }
			else if (row > 0)
				row--;
	        else /* row == 0 */
				row = col = CUSTOM_BOX_VALUE;
			SetHotItem(GetIndex(row, col));
			break;

		case VK_RIGHT:
			if (row == CUSTOM_BOX_VALUE)
				row = col = 0;
			else if (col < m_numCol - 1) 
				col++;
			else {
				col = 0; row++;
			}

			if (GetIndex(row, col) == INVALID_COLOR)
				row = col = CUSTOM_BOX_VALUE;

			SetHotItem(GetIndex(row, col));
			break;

		case VK_HOME:
			SetHotItem(GetIndex(0, 0));
			break;

		case VK_END:
			SetHotItem(GetIndex(m_numRow - 1, m_numCol - 1));
			break;

		case VK_LEFT:
			if (row == CUSTOM_BOX_VALUE) {
				row = GetRow(m_colors - 1);
				col = GetColumn(m_colors - 1);
			}
	        else if (col > 0)
				col--;
			else { /* col == 0 */
				if (row > 0) {
					row--;
					col = m_numCol - 1;
				}
				else
					row = col = CUSTOM_BOX_VALUE;
			}
			SetHotItem(GetIndex(row, col));
			break;

		case VK_ESCAPE:
			m_clrSelect = m_clrOriginal;
			EndSelection(CPN_SELENDCANCEL);
			break;

		case VK_RETURN:
		case VK_SPACE:
			EndSelection(CPN_SELENDOK);
			break;
	    }
	}

	void OnPrintClient(HDC hdc, LPARAM flags)
	{
		if (flags & PRF_ERASEBKGND)
			DefWindowProc(WM_ERASEBKGND, (WPARAM)hdc, 0);

		if (flags & (PRF_CHECKVISIBLE | PRF_CLIENT | PRF_OWNED))
			OnDraw(hdc);
	}

	void OnPaint()
	{
		CPaintDC dc(m_hWnd);
		OnDraw(dc);
	}

	void OnDraw(HDC hdc)
	{
	    // Draw colour cells
		for (int i = 0; i < m_colors; i++)
			DrawCell(hdc, i);
		// Draw custom text
		DrawCell(hdc, CUSTOM_BOX_VALUE);
	}

	void OnMouseMove(UINT nFlags, CPoint pt)
	{
		int sel = HitTest(pt);
		if (sel != m_hotIndex) {
			TTActivate(FALSE);
			SetHotItem(sel);
			TTActivate(TRUE);
		}
	}

	void OnLButtonDown(UINT nFlags, CPoint pt)
	{
		int sel = HitTest(pt);
		if (sel != INVALID_COLOR)
			m_bClicked = true;
		else
			EndSelection(CPN_SELENDCANCEL);
	}

	void OnLButtonUp(UINT nFlags, CPoint pt)
	{
		if (m_bClicked) {
			int sel = HitTest(pt);
			if (sel != INVALID_COLOR)
				EndSelection(CPN_SELENDOK);
		    else
				EndSelection(CPN_SELENDCANCEL);
		}
	}

	void OnKillFocus(HWND hNew)
	{
		EndSelection(CPN_SELENDCANCEL);
	}

	void drawshadow(HDC hdc, CRect& rc)
	{
		CDCHandle dc = hdc;
		HBITMAP hbm = CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
		CDC dcw;
		dcw.CreateCompatibleDC(dc);
		hbm = dcw.SelectBitmap(hbm);

		CRect rcc(0, 0, rc.Width(), rc.Height());
		dcw.FillSolidRect(&rcc, RGB(192, 192, 192));
		rcc.DeflateRect(1, 1);
		dcw.FillSolidRect(&rcc, RGB(128, 128, 128));
		rcc.DeflateRect(1, 1);
		dcw.FillSolidRect(&rcc, RGB(64, 64, 64));
		rcc.DeflateRect(1, 1);
		dcw.FillSolidRect(&rcc, RGB(0, 0, 0));
		lccAlphaBlend(dc, rc.left, rc.top, rc.Width(), rc.Height(), dcw, 0, 0, 64);
		DeleteObject(dcw.SelectBitmap(hbm));
	}

	BOOL OnEraseBkgnd(HDC hdc)
	{
		CRect rc;
		GetClientRect(&rc);
		CDCHandle dc = hdc;

		rc.left += m_gap;
		rc.top += m_gap;
		drawshadow(hdc, rc);

		rc.OffsetRect(-m_gap, -m_gap);

		// Draw raised window edge (ex-window style WS_EX_WINDOWEDGE is sposed to do
		// this, but for some reason isn't
		dc.DrawEdge(&rc, BDR_RAISEDOUTER, BF_RECT | BF_FLAT | BF_ADJUST);
		dc.FillSolidRect(&rc, COLOR_VISTA_MENUBKGND);
		return TRUE;
	}

	void OnDestroy()
	{
		ReleaseCapture();
	}

	int HitTest(CPoint pt)
	{
		int sel = INVALID_COLOR;

		// Translate points to be relative raised window edge
		pt.x -= m_nMargin;
		pt.y -= m_nMargin;

		// First check we aren't in text box
		if (m_rcCus.PtInRect(pt))
			sel = CUSTOM_BOX_VALUE;
		else if (m_rcLis.PtInRect(pt)) {
			// Get the row and column
			sel = GetIndex(pt.y / m_boxSize, pt.x / m_boxSize);

			// In range? If not, default and exit
	        if (sel < 0 || sel >= m_colors)
				sel = INVALID_COLOR;
		}
		return sel;
	}


	int GetIndex(int row, int col)
	{
	 	if ((row == CUSTOM_BOX_VALUE || col == CUSTOM_BOX_VALUE))
			return CUSTOM_BOX_VALUE;
		else if (row < 0 || col < 0 || row >= m_numRow || col >= m_numCol)
			return INVALID_COLOR;
		else {
			if (row * m_numCol + col >= m_colors)
				return INVALID_COLOR;
			else
				return row * m_numCol + col;
		}
	}

	int GetRow(int index)
	{
		if (index == CUSTOM_BOX_VALUE)				return CUSTOM_BOX_VALUE;
		else if (index < 0 || index >= m_colors)	return INVALID_COLOR;
		else										return index / m_numCol;
	}

	int GetColumn(int index)
	{
		if (index == CUSTOM_BOX_VALUE)				return CUSTOM_BOX_VALUE;
		else if (index < 0 || index >= m_colors)	return INVALID_COLOR;
		else										return index % m_numCol;
	}

	void FindCellFromColor(COLORREF crColor)
	{
	    for (int i = 0; i < m_colors; i++) {
			if (m_pcrTable[i] == crColor) {
	            m_selIndex = i;
				return;
	        }
	    }
		m_selIndex = CUSTOM_BOX_VALUE;
	}

	BOOL GetCellRect(int index, CRect& rc)
	{
		// Gets the dimensions of the colour cell given by (row,col)
		if (index == CUSTOM_BOX_VALUE) {
			rc = m_rcCus;
	        return TRUE;
		}

		if (index < 0 || index >= m_colors)
	        return FALSE;

		rc.SetRect(0, 0, m_boxSize, m_boxSize);
		rc.OffsetRect(GetColumn(index) * m_boxSize + m_nMargin, GetRow(index) * m_boxSize + m_nMargin);
		return TRUE;
	}

	void SetWindowSize()
	{
	    CSize csz(0, 0);
		CClientDC dc(m_hWnd);
		HFONT hfOld = dc.SelectFont(m_font);
		GetTextExtentPoint32(dc, szCustomText, lstrlen(szCustomText), &csz);
		dc.SelectFont(hfOld);

		csz += CSize(2 * m_nMargin, 2 * m_nMargin);

		// Add even more space to draw the horizontal line
		csz.cy += 2 * m_nMargin + 2;

		m_numRow = m_colors / m_numCol;
		if (m_colors % m_numCol)
			m_numRow++;

	    // Get the current window position, and set the new size
	    CRect rc;
		GetWindowRect(&rc);

		rc.right = rc.left + m_numCol * m_boxSize + 2 * m_nMargin;
		rc.bottom = rc.top  + m_numRow * m_boxSize + 2 * m_nMargin;

	    // expand window if necessary, and set text width as
	    // window width
		if (csz.cx > rc.Width())
			rc.right = rc.left + csz.cx;
		csz.cx = rc.Width() - 2 * m_nMargin;

		// List area
		m_rcLis = rc;
		m_rcLis.OffsetRect(-rc.left, -rc.top);
		m_rcLis.InflateRect(-m_nMargin, -m_nMargin);

		// Work out the custom text area
		m_rcCus.SetRect(m_nMargin, rc.Height(), 
						m_nMargin + csz.cx, 
						rc.Height() + m_nMargin + csz.cy);
		rc.bottom += m_rcCus.Height() + 2 * m_nMargin;

		// 影
		rc.right += m_gap;
		rc.bottom += m_gap;

		// Need to check it'll fit on screen: Too far right?
		csz = CSize(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		if (rc.right > csz.cx)
			rc.OffsetRect(-(rc.right - csz.cx), 0);

	    // Too far left?
	    if (rc.left < 0)
			rc.OffsetRect(-rc.left, 0);

	    // Bottom falling out of screen?
		if (rc.bottom > csz.cy)
			rc.OffsetRect(0, -(rc.bottom - csz.cy));

		// Set the window size and position
		MoveWindow(&rc, TRUE);
	}

	void SetHotItem(int index)
	{
		if (index > m_colors)
			index = CUSTOM_BOX_VALUE;

		if (m_hotIndex != index) {
			int tmp = m_hotIndex;
			m_hotIndex = index;
			if (tmp != INVALID_COLOR)
				RedrawItem(tmp);
			if (m_hotIndex != INVALID_COLOR) {
				RedrawItem(m_hotIndex);
				if (m_hotIndex != CUSTOM_BOX_VALUE)
					m_clrSelect = m_pcrTable[m_hotIndex];
			}
			else
				m_clrSelect = INVALID_COLOR;
		}
	}

	void EndSelection(int nResult)
	{
		if (m_nResult == CPN_PROGRESS) {
			if (nResult == CPN_SELENDOK && m_hotIndex == CUSTOM_BOX_VALUE)
				nResult = CPN_SELENDCUSTOM;
			m_nResult = nResult;
		}
		DestroyWindow();
	}

	void OnToolTipNeedText(UINT id, NMHDR* phdr, LRESULT* pResult)
	{
		*pResult = FALSE;

		CPoint pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);

		CRect rc;
		GetClientRect(rc);
		if (rc.PtInRect(pt)) {
			TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)phdr;
			pTTT->szText[0] = 0;
			int sel = HitTest(pt);
			if (sel != INVALID_COLOR) {
				COLORREF cr = INVALID_COLOR;
				if (sel == CUSTOM_BOX_VALUE) {
					if (m_selIndex == CUSTOM_BOX_VALUE)
						cr = m_clrOriginal;
				}
				else
					cr = m_pcrTable[sel];

				if (cr != INVALID_COLOR)
					wsprintf(pTTT->szText, _T("#%02X%02X%02X"), GetRValue(cr), GetGValue(cr), GetBValue(cr));
			}
			*pResult = TRUE;
		}
	}

	void DrawItemRect(HDC hdc, RECT* prc, COLORREF clrPane, COLORREF clrBorder)
	{
		CDCHandle dc = hdc;
		HBRUSH hbr = CreateSolidBrush(clrPane);
		HPEN hpen = CreatePen(PS_SOLID, 1, clrBorder);

		HBRUSH hbrOld = dc.SelectBrush(hbr);
		HPEN hpenOld = dc.SelectPen(hpen);

		// Draw the cell colour
		CRect rc = *prc;
		rc.DeflateRect(m_nMargin, m_nMargin);
		dc.Rectangle(rc);

		// restore DC and cleanup
		dc.SelectBrush(hbrOld);
		dc.SelectPen(hpenOld);
		DeleteObject(hbr);
		DeleteObject(hpen);
	}

	void DrawCellFrame(CDCHandle& dc, CRect& rc, int index)
	{
		// fill background
		if (m_hotIndex == index || m_selIndex == index) {
			lccDrawSelectionFrame(dc, &rc, TRUE, (m_selIndex == index) ? -1 : 128);
		}
		else {
			dc.FillSolidRect(&rc, COLOR_VISTA_MENUBKGND);
		}
	}

	void DrawCell(HDC hdc, int index)
	{
		CDCHandle dc = hdc;
		CRect rc;

		COLORREF clrFace = COLOR_VISTA_MENUBKGND;
		COLORREF clrShadow = GetSysColor(COLOR_3DSHADOW);
		COLORREF clrHilight = GetSysColor(COLOR_3DHILIGHT);

		// For the Custom Text area
		if (index == CUSTOM_BOX_VALUE) {
			// The extent of the actual text button
			rc = m_rcCus;
			rc.top += 2 * m_nMargin;

			// Fill background
			dc.FillSolidRect(&rc, clrFace);

			// 水平セパレータ
			CRect rcw = m_rcCus;
			rcw.bottom = rcw.top + 1;
			dc.FillSolidRect(&rcw, RGB(0xe0, 0xe0, 0xe0));
			rcw.OffsetRect(0, 1);
			dc.FillSolidRect(&rcw, RGB(0xff, 0xff, 0xff));
			rc.DeflateRect(1, 1);

			// セルフレーム
			DrawCellFrame(dc, rc, index);

			// Draw custom text
			dc.SetBkMode(TRANSPARENT);
			HFONT hfOld = dc.SelectFont(m_font);

			CSize size;
			dc.GetTextExtent(szCustomText, lstrlen(szCustomText), &size);

			if (m_selIndex == CUSTOM_BOX_VALUE) {
				int dx = m_boxSize * 2 + 4 + size.cx;
				rc.left = rc.left + (rc.Width() - dx) / 2;

				// カラーパレット描画
				rcw = rc;
				rcw.right = rcw.left + m_boxSize * 2;
				rcw.top += (rcw.Height() - m_boxSize) / 2;
				rcw.bottom = rcw.top + m_boxSize;
				DrawItemRect(dc, &rcw, m_clrOriginal, clrShadow);
				rc.left = rcw.right + 4;
			}
			dc.DrawText(szCustomText, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			dc.SelectFont(hfOld);
			return;
		}

		if (!GetCellRect(index, rc))
			return;

		// Select and realize the palette
		HPALETTE hpalOld = NULL;
		if (dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE) {
			hpalOld = dc.SelectPalette(m_hpal, FALSE);
			dc.RealizePalette();
	    }

		// セルフレーム
		DrawCellFrame(dc, rc, index);

		// カラーパレット描画
		DrawItemRect(dc, &rc, m_pcrTable[index], clrShadow);

		if (dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE)
			dc.SelectPalette(hpalOld, FALSE);
	}

	void RedrawItem(int index)
	{
		CRect rc;
		if (GetCellRect(index, rc))
			RedrawWindow(&rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
};

////////////////////////////////////////////////////////////////
//

static CUSTPAL* _palload(LPCTSTR pszFile)
{
	CUSTPAL* pTable = NULL;
	FILE* fp;
	if (!_tfopen_s(&fp, pszFile, _T("rt"))) {
		pTable = (CUSTPAL*)malloc(sizeof(CUSTPAL) + sizeof(COLORREF) * 256);
		pTable->colors = 0;
		pTable->column = 0;
		pTable->table = (COLORREF*)((BYTE*)pTable + sizeof(CUSTPAL));

		// カラム数読み込み
		TCHAR buf[256];
		while (_fgetts(buf, ARRAYSIZE(buf), fp)) {
			if (buf[0] != '#')
				break;
		}
		pTable->column = _ttoi(buf);

		// カラーデータ読み込み
		int r, g, b;
		while (_fgetts(buf, ARRAYSIZE(buf), fp) && pTable->colors < MAX_COLORS) {
			if (buf[0] != '#') {
				_stscanf_s(buf, _T("%d,%d,%d"), &r, &g, &b);
				pTable->table[pTable->colors] = RGB(r, g, b);
				pTable->colors++;
			}
		}
		fclose(fp);
	}
	return pTable;
}

extern "C"
BOOL WINAPI lccColorPickerEx(HWND hParent, POINT p, COLORREF* pcrColor, LPCTSTR pszPalFile)
{
	CColorPaletteWindow palette;
	CUSTPAL* pTable;
	if (HIWORD(pszPalFile) == NULL) {
		switch (LOWORD(pszPalFile)) {
			case 1: pTable = &cpGrad; break;
			case 2: pTable = &cpWeb;  break;
			case 3: pTable = &cpIe4;  break;
			case 4: pTable = &cp256;  break;
			case 0:
			default: pTable = &cpNorm; break;
		}
		palette.Initialize(pTable, *pcrColor);
	}
	else {
		pTable = _palload(pszPalFile);
		if (!pTable)
			pTable = &cpNorm;
		palette.Initialize(pTable, *pcrColor);
		if (pTable != &cpNorm)
			free(pTable);
	}

	palette.Create(p.x, p.y);
	while (palette.IsWindow()) {
		MSG msg;
		while (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
			BOOL bUnicode = ::IsWindowUnicode(msg.hwnd);
			BOOL bOK;
			if (bUnicode)
				bOK = ::GetMessageW(&msg, NULL, 0, 0);
			else
				bOK = ::GetMessageA(&msg, NULL, 0, 0);

			if (bOK) {
				::TranslateMessage(&msg);
				if (bUnicode)
					::DispatchMessageW(&msg);
				else
					::DispatchMessageA(&msg);
			}
		}
	}

	BOOL bOK = FALSE;
	if (palette.m_nResult == CPN_SELENDOK) {
		bOK = TRUE;
		*pcrColor = palette.m_clrSelect;
	}
	else if (palette.m_nResult == CPN_SELENDCUSTOM) {
		COLORREF rgb;
		if (lccSelectCustomColor(hParent, *pcrColor, &rgb)) {
			*pcrColor = rgb;
			bOK = TRUE;
		}
	}
	return bOK;
}

extern "C"
BOOL WINAPI lccColorPicker(HWND hParent, POINT p, COLORREF* pcrColor, BOOL bGrad)
{
	return lccColorPickerEx(hParent, p, pcrColor, bGrad ? LPCTSTR(1) : LPCTSTR(0));
}

