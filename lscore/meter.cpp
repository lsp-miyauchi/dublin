///////////////////////////////////////////////////////
// meter.cpp

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"
#include "common.h"

#include "staticctrl.h"


///////////////////////////////////////////////////////
// METER control implementation

class CMeterCtrl : public CDefStaticCtrlImpl<CMeterCtrl>,
					public CThemeImpl<CMeterCtrl>
{
private:
	COLORREF m_clrFore;
	COLORREF m_clrBack;
	UINT m_nComplete;
	UINT m_nTotalSize;

public:
	CMeterCtrl();
	~CMeterCtrl() { }

BEGIN_MSG_MAP(CMeterCtrl)
	CHAIN_MSG_MAP(CThemeImpl<CMeterCtrl>)
	CHAIN_MSG_MAP(CDefStaticCtrlImpl<CMeterCtrl>)
	ON_MESSAGE(MM_SETSIZE, OnSetSize)
	ON_MESSAGE(MM_GETSIZE, OnGetSize)
	ON_MESSAGE(MM_SETCOMPLETE, OnSetComplete)
	ON_MESSAGE(MM_GETCOMPLETE, OnGetComplete)
	ON_MESSAGE(MM_SETTEXTCOLOR, OnSetTextColor)
	ON_MESSAGE(MM_SETBKCOLOR, OnSetBkColor)
	ON_MESSAGE(MM_GETTEXTCOLOR, OnGetTextColor)
	ON_MESSAGE(MM_GETBKCOLOR, OnGetBkColor)
END_MSG_MAP()

	__inline LRESULT OnGetSize(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_nTotalSize; }
	LRESULT OnSetSize(WPARAM wParam, LPARAM lParam);
	__inline LRESULT OnGetComplete(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_nComplete; }
	LRESULT OnSetComplete(WPARAM wParam, LPARAM lParam);
	__inline LRESULT OnGetTextColor(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_clrFore; }
	LRESULT OnSetTextColor(WPARAM wParam, LPARAM lParam);
	__inline LRESULT OnGetBkColor(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_clrBack; }
	LRESULT OnSetBkColor(WPARAM wParam, LPARAM lParam);

	void OnDraw(HDC hdc);
	void DrawMeter(HDC hdc);
	void DrawBlockBar(HDC hdc);
};

CMeterCtrl::CMeterCtrl()
{
	m_nTotalSize = 100;
	m_nComplete = 0;
	m_clrFore = RGB(0, 0, 255);
	m_clrBack = RGB(255, 255, 255);

	// Theme
	SetThemeClassList(L"Progress");
}

void CMeterCtrl::OnDraw(HDC hdc)
{
	if (GetStyle() & MS_BLOCKBAR)
		DrawBlockBar(hdc);
	else
		DrawMeter(hdc);
}

LRESULT CMeterCtrl::OnSetSize(WPARAM wParam, LPARAM lParam)
{
	m_nTotalSize = (UINT)wParam;
	Invalidate(FALSE);
	UpdateWindow();
	return 0;
}

LRESULT CMeterCtrl::OnSetComplete(WPARAM wParam, LPARAM lParam)
{
	m_nComplete = (UINT)wParam;
	Invalidate(FALSE);
	UpdateWindow();
	return 0;
}

LRESULT CMeterCtrl::OnSetTextColor(WPARAM wParam, LPARAM lParam)
{
	if (m_clrFore != (COLORREF)lParam) {
		m_clrFore = (COLORREF)lParam;
		Invalidate(TRUE);
	}
	return 0;
}

LRESULT CMeterCtrl::OnSetBkColor(WPARAM wParam, LPARAM lParam)
{
	if (m_clrBack != (COLORREF)lParam) {
		m_clrBack = (COLORREF)lParam;
		Invalidate(TRUE);
	}
	return 0;
}

void CMeterCtrl::DrawMeter(HDC hdc)
{
	CDCHandle dc = hdc;
	UINT nSize = m_nTotalSize;
	UINT nComplete = m_nComplete;
	if (!nSize) {
		nSize = 1;
		nComplete = 0;
	}
	TCHAR buf[16];
	wsprintf(buf, _T("%d%%"), (100 * nComplete) / nSize);

	HFONT hfont = dc.SelectFont(m_hFont);
	dc.SetTextColor(m_clrFore);
	dc.SetBkColor(m_clrBack);

	// 前景色と背景色を反転させる
	COLORREF rgb = dc.GetBkColor();
	dc.SetBkColor(dc.SetTextColor(rgb));

	// 完了済み領域の矩形座標を設定
	CRect rc, rcClient;
	GetClientRect(&rcClient);
	rc.SetRect(0, 0, (rcClient.right * nComplete) / nSize, rcClient.bottom);

	// 文字高を取得
	CSize sz;
	dc.GetTextExtent(_T("X"), 1, &sz);

	// 完了率をウィンドウへ出力
	dc.SetTextAlign(TA_CENTER | TA_TOP);
	dc.ExtTextOut(rcClient.right / 2, (rcClient.bottom - sz.cy) / 2,
				ETO_OPAQUE | ETO_CLIPPED, &rc, buf, lstrlen(buf), NULL);

	// 残量率矩形を設定
    rc.left = rc.right;
    rc.right = rcClient.right;

	// 前景色と背景色を反転させる
	rgb = dc.GetBkColor();
	dc.SetBkColor(dc.SetTextColor(rgb));

	// 残量率をウィンドウへ出力
	dc.ExtTextOut(rcClient.right / 2, (rcClient.bottom - sz.cy) / 2,
				ETO_OPAQUE | ETO_CLIPPED, &rc, buf, lstrlen(buf), NULL);

	dc.SelectFont(hfont);
}

void CMeterCtrl::DrawBlockBar(HDC hdc)
{
	CDCHandle dc = hdc;
	CRect rc;
	GetClientRect(&rc);
	DWORD style = GetStyle();
	if (style & MS_FRAMERECT) {
		if (!IsThemeNull())
			DrawThemeBackground(dc, PP_BAR, 0, &rc, NULL);
		else
			dc.DrawEdge(&rc, BDR_SUNKENOUTER, BF_TOPLEFT | BF_BOTTOMRIGHT);
		rc.InflateRect(-3, -3);
	}
	int xDelta = (style & MS_LARGEBLOCK) ? 8 : 5;
	int xBlock = rc.Width() / xDelta;

	UINT nSize = m_nTotalSize;
	UINT nComplete = m_nComplete;
	if (!nSize) {
		nSize = 1;
		nComplete = 0;
	}

	CRect rcBlock = rc;
	rcBlock.right = rcBlock.left + xDelta - 1;
	int xPar = rc.Width() * nComplete / nSize;
	for (int i = 0; i < xBlock; i++) {
		if (xPar > rcBlock.left)
			_FillG(dc, rcBlock, m_clrFore);
		else
			dc.FillSolidRect(&rcBlock, m_clrBack);
		rcBlock.OffsetRect(xDelta, 0);
	}
}

///////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK MeterWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CMeterCtrl* pWnd = new CMeterCtrl();
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}

