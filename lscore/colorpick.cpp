#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"
#include "common.h"
#include "picker.h"

///////////////////////////////////////////////////////
// COLOR PICKER control implementation

class CColorPickCtrl : public CPickerCtrlImpl<CColorPickCtrl>
{
private:
	COLORREF m_clrText;
	COLORREF m_clrBack;

public:
	CColorPickCtrl();

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

BEGIN_MSG_MAP(CColorPickCtrl)
	CHAIN_MSG_MAP(CPickerCtrlImpl<CColorPickCtrl>)
	ON_MESSAGE(CPM_GETTEXTCOLOR, OnGetTextColor)
	ON_MESSAGE(CPM_GETBKCOLOR, OnGetBkColor)
	ON_MESSAGE(CPM_SETTEXTCOLOR, OnSetTextColor)
	ON_MESSAGE(CPM_SETBKCOLOR, OnSetBkColor)
END_MSG_MAP()

	LRESULT OnGetTextColor(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_clrText; }
	LRESULT OnGetBkColor(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_clrBack; }
	LRESULT OnSetTextColor(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetBkColor(WPARAM wParam, LPARAM lParam);

	void DrawTextFace(HDC hdc, RECT* prc);
	void Browse();
};

CColorPickCtrl::CColorPickCtrl()
	: m_clrText(GetSysColor(COLOR_WINDOWTEXT))
	, m_clrBack(GetSysColor(COLOR_WINDOW))
{
	HIMAGELIST him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_PICKER_COLOR), 16, RGB(255, 0, 255));
	SetButtonImageList(him);
}

LRESULT CColorPickCtrl::OnSetTextColor(WPARAM wParam, LPARAM lParam)
{
	COLORREF clr = (COLORREF)lParam;
	if (m_clrText != clr) {
		m_clrText = clr;
		InvalidateRect(&m_rcText, FALSE);
		UpdateWindow();
	}
	return 0;
}

LRESULT CColorPickCtrl::OnSetBkColor(WPARAM wParam, LPARAM lParam)
{
	COLORREF clr = (COLORREF)lParam;
	if (m_clrBack != clr) {
		m_clrBack = clr;
		InvalidateRect(&m_rcText, FALSE);
		UpdateWindow();
	}
	return 0;
}

void CColorPickCtrl::DrawTextFace(HDC hdc, RECT* prc)
{
	CDCHandle dc = hdc;
	CRect rc = *prc;

	BOOL bEnabled = IsWindowEnabled();
	COLORREF clrFace = GetSysColor(COLOR_3DFACE);
	COLORREF clrShadow = GetSysColor(COLOR_3DSHADOW);
	COLORREF clrBkgnd = (m_clrBack == COLOR_DEFAULT) ? GetSysColor(COLOR_WINDOW) : m_clrBack;
	dc.FillSolidRect(&rc, bEnabled ? clrBkgnd : mix_color(clrFace, clrShadow, 90));

	CString str;
	if (GetWindowText(str) > 0) {
		HFONT hfOld = NULL;
		if (m_hFont)
			hfOld = dc.SelectFont(m_hFont);

		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(bEnabled ? m_clrText : GetSysColor(COLOR_GRAYTEXT));
		dc.DrawText(str, -1, &rc, DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX | DT_END_ELLIPSIS);

		if (hfOld)
			dc.SelectFont(hfOld);
	}

	if (m_bHasFocus)
		dc.DrawFocusRect(&rc);
}

void CColorPickCtrl::Browse()
{
	CRect rc;
	GetWindowRect(&rc);
	COLORREF clr = m_clrBack;
	CPoint pt(rc.left, rc.bottom);

	// カラーテーブルインデクス
	int index = GetStyle() & CPS_STYLE_MASK;
	if (index > 1)
		index = 4;

	if (lccColorPickerEx(m_hWnd, pt, &clr, MAKEINTRESOURCE(index))) {
		m_clrBack = clr;
		RedrawWindow(&m_rcText, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		GetParent().SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), CBN_SELCHANGE), (LPARAM)m_hWnd);
	}
}

//////////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK ColorPickWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CColorPickCtrl* pWnd = new CColorPickCtrl();
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}

