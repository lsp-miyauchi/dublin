///////////////////////////////////////////////////////
// INFOBAR control implementation

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"
#include "common.h"

#include "staticctrl.h"

class CTcString
{
public:
	CString m_str;
	COLORREF m_cr;

	CTcString()
	{
		m_cr = GetSysColor(COLOR_WINDOWTEXT);
		m_str = _T("");
	}
};

///////////////////////////////////////////////////////
//

class CInfoBarCtrl : public CDefStaticCtrlImpl<CInfoBarCtrl>
{
private:
	CTcString m_tcLeft;
	CTcString m_tcCenter;
	CTcString m_tcRight;
	COLORREF m_crBack1;
	COLORREF m_crBack2;

public:
	CInfoBarCtrl();
	~CInfoBarCtrl() { }

BEGIN_MSG_MAP(CInfoBarCtrl)
	CHAIN_MSG_MAP(CDefStaticCtrlImpl<CInfoBarCtrl>)
	ON_MESSAGE(IBM_SETBKCOLOR, OnSetBkColor)
	ON_MESSAGE(IBM_SETLEFTTEXT, OnSetLeftText)
	ON_MESSAGE(IBM_SETCENTERTEXT, OnSetCenterText)
	ON_MESSAGE(IBM_SETRIGHTTEXT, OnSetRightText)
END_MSG_MAP()

	LRESULT OnSetBkColor(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetLeftText(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetCenterText(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetRightText(WPARAM wParam, LPARAM lParam);

	BOOL OnEraseBkgnd(HDC hdc);
	void OnDraw(HDC hdc);
	void ResetText(CTcString& tc, COLORREF cr, LPCTSTR psz);
};


CInfoBarCtrl::CInfoBarCtrl()
{
	m_crBack1 = GetSysColor(COLOR_WINDOW);
	m_crBack2 = GetSysColor(COLOR_WINDOW);
}

_inline BOOL CInfoBarCtrl::OnEraseBkgnd(HDC hdc)
	{ return TRUE; }


LRESULT CInfoBarCtrl::OnSetBkColor(WPARAM wParam, LPARAM lParam)
{
	if ((m_crBack1 != (COLORREF)lParam) || (m_crBack2 != (COLORREF)wParam)) {
		m_crBack1 = (COLORREF)lParam;
		m_crBack2 = (COLORREF)wParam;
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	return 0;
}

LRESULT CInfoBarCtrl::OnSetLeftText(WPARAM wParam, LPARAM lParam)
{
	ResetText(m_tcLeft, (COLORREF)wParam, (LPCTSTR)lParam);
	return 0;
}

LRESULT CInfoBarCtrl::OnSetCenterText(WPARAM wParam, LPARAM lParam)
{
	ResetText(m_tcCenter, (COLORREF)wParam, (LPCTSTR)lParam);
	return 0;
}

LRESULT CInfoBarCtrl::OnSetRightText(WPARAM wParam, LPARAM lParam)
{
	ResetText(m_tcRight, (COLORREF)wParam, (LPCTSTR)lParam);
	return 0;
}

void CInfoBarCtrl::ResetText(CTcString& tc, COLORREF cr, LPCTSTR psz)
{
	tc.m_cr = cr;
	if (psz && *psz)
		tc.m_str = psz;
	else
		tc.m_str = _T("");
	RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
}

void CInfoBarCtrl::OnDraw(HDC hdc)
{
	CRect rc;
	GetClientRect(&rc);

	CDCHandle dc = hdc;
	CDC dcw;
	dcw.CreateCompatibleDC(dc);

	HBITMAP hbm = CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
	HBITMAP hbmo = dcw.SelectBitmap(hbm);
	FillShadeRect(dcw, rc, m_crBack1, m_crBack2, FALSE);
	HFONT hfonto = dcw.SelectFont(m_hFont);

	// 左テキスト
	CRect rcw = rc;
	rcw.InflateRect(-4, 0);
	dcw.SetBkMode(TRANSPARENT);
	if (!m_tcLeft.m_str.IsEmpty()) {
		dcw.SetTextColor(m_tcLeft.m_cr);
		dcw.DrawText(m_tcLeft.m_str, -1, &rcw, DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOPREFIX);
	}
	// 右テキスト
	if (!m_tcRight.m_str.IsEmpty()) {
		dcw.SetTextColor(m_tcRight.m_cr);
		dcw.DrawText(m_tcRight.m_str, -1, &rcw, DT_SINGLELINE | DT_VCENTER | DT_RIGHT | DT_NOPREFIX);
	}
	// 中央テキスト
	if (!m_tcCenter.m_str.IsEmpty()) {
		dcw.SetTextColor(m_tcCenter.m_cr);
		dcw.DrawText(m_tcCenter.m_str, -1, &rcw, DT_SINGLELINE | DT_VCENTER | DT_RIGHT | DT_NOPREFIX);
	}

	// 実画面へ転送
	dc.BitBlt(0, 0, rc.Width(), rc.Height(), dcw, 0, 0, SRCCOPY);

	dcw.SelectFont(hfonto);
	DeleteObject(dcw.SelectBitmap(hbmo));
}

extern "C"
LRESULT CALLBACK InfoBarWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CInfoBarCtrl* pWnd = new CInfoBarCtrl;
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}

