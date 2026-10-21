///////////////////////////////////////////////////////
// ColoredProgressBar control implementation

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "common.h"
#include "fnc.h"

#include "staticctrl.h"

class CColoredProgressBarCtrl : public CDefStaticCtrlImpl<CColoredProgressBarCtrl>,
								public CThemeImpl<CColoredProgressBarCtrl>
{
private:
	COLORREF m_crFrom;
	COLORREF m_crTo;
	INT_PTR m_vmin;						// 最小値
	INT_PTR m_vmax;						// 最大値
	INT_PTR m_step;						// 増分値
	INT_PTR m_curpos;					// 現在値
	int m_curper;						// 0 - 100 %値
	int m_numbits;						// デバコンのBITSPIXEL

public:
	CColoredProgressBarCtrl();

BEGIN_MSG_MAP(CColoredProgressBarCtrl)
	CHAIN_MSG_MAP(CThemeImpl<CColoredProgressBarCtrl>)
	CHAIN_MSG_MAP(CDefStaticCtrlImpl<CColoredProgressBarCtrl>)
	ON_MESSAGE(PBM_SETRANGE, OnHandlerSetRange)
	ON_MESSAGE(PBM_SETRANGE32, OnHandlerSetRange32)
	ON_MESSAGE(PBM_GETRANGE, OnHandlerGetRange)
	ON_MESSAGE(PBM_SETPOS, OnHandlerSetPos)
	ON_MESSAGE(PBM_GETPOS, OnHandlerGetPos)
	ON_MESSAGE(PBM_SETSTEP, OnHandlerSetStep)
	ON_MESSAGE(PBM_DELTAPOS, OnHandlerDeltaPos)
	ON_MESSAGE(PBM_STEPIT, OnHandlerStepIt)
	ON_MESSAGE(PBM_SETBARCOLOR, OnHandlerSetBarColor)
END_MSG_MAP()

	__inline LRESULT OnHandlerSetRange(WPARAM wParam, LPARAM lParam) { return SetRange((INT_PTR)(short)LOWORD(lParam), (INT_PTR)(short)HIWORD(lParam)); }
	__inline LRESULT OnHandlerSetRange32(WPARAM wParam, LPARAM lParam) { return SetRange((INT_PTR)wParam, (INT_PTR)lParam); }
	__inline LRESULT OnHandlerGetRange(WPARAM wParam, LPARAM lParam) { return GetRange((BOOL)wParam, (PBRANGE*)lParam); }
	__inline LRESULT OnHandlerSetPos(WPARAM wParam, LPARAM lParam) { return SetPos((INT_PTR)wParam); }
	__inline LRESULT OnHandlerGetPos(WPARAM wParam, LPARAM lParam) { return m_curpos; }
	__inline LRESULT OnHandlerSetStep(WPARAM wParam, LPARAM lParam) { return SetStep((INT_PTR)wParam); }
	__inline LRESULT OnHandlerDeltaPos(WPARAM wParam, LPARAM lParam) { return DeltaPos((INT_PTR)wParam); }
	__inline LRESULT OnHandlerStepIt(WPARAM wParam, LPARAM lParam) { return StepIt(); }
	__inline LRESULT OnHandlerSetBarColor(WPARAM wParam, LPARAM lParam) { return SetBarColor((COLORREF)lParam, (COLORREF)wParam); }

	void OnDraw(HDC hdc);
	LRESULT SetRange(INT_PTR vmin, INT_PTR vmax);
	LRESULT SetPos(INT_PTR pos);
	LRESULT GetRange(BOOL fWhichLimit, PBRANGE* ppbr);
	LRESULT SetStep(INT_PTR step);
	LRESULT DeltaPos(INT_PTR deltapos);
	LRESULT StepIt();
	LRESULT SetBarColor(COLORREF crFrom, COLORREF crTo);
};


CColoredProgressBarCtrl::CColoredProgressBarCtrl()
{
	m_vmin = 0;
	m_vmax = 100;
	m_step = 1;
	m_curpos = 0;
	m_curper = 0;

	CClientDC dc(nullptr);
	m_numbits = dc.GetDeviceCaps(BITSPIXEL);
	m_crFrom = RGB(0, 192, 255);
	m_crTo = RGB(64, 0, 128);
	if (m_numbits <= 8)
		m_crTo = m_crFrom = RGB(0, 0, 128);

	// Theme
	SetThemeClassList(L"Progress");
}

void CColoredProgressBarCtrl::OnDraw(HDC hdc)
{
	CRect rcc;
	GetClientRect(&rcc);

	CRect rc = rcc;
	CDCHandle dc = hdc;
	CDC dcw;
	dcw.CreateCompatibleDC(dc);
	HBITMAP hbm = CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
	hbm = dcw.SelectBitmap(hbm);

	// 背面を塗りつぶす
	HBRUSH hbr = CtlColor_Static(GetParent(), hdc, m_hWnd);
	if (hbr) dcw.FillRect(&rc, hbr);

	if (!IsThemeNull()) {
		if (Os7()) {
			dcw.DrawEdge(&rc, BDR_SUNKENINNER | BDR_RAISEDOUTER, BF_RECT | BF_FLAT);
			rc.InflateRect(-2, -2);
		} else {
			DrawThemeBackground(dcw, PP_BAR, 0, &rc, NULL);
			rc.InflateRect(-4, -3);
		}
	} else {
		dcw.DrawEdge(&rc, BDR_SUNKENOUTER, BF_TOPLEFT | BF_BOTTOMRIGHT);
		rc.InflateRect(-2, -2);
	}

	int yb = rc.Height();
	int xb = (yb * 2) / 3;

	CRect rcb = rc;
	rcb.right = rcb.left + xb;

	int per;
	COLORREF cr, crOld = dcw.GetBkColor();
	BOOL bContinue = TRUE;
	while (bContinue && rcb.left < rc.right) {
		if (rcb.right > rc.right) {
			rcb.right = rc.right;
			bContinue = FALSE;
		}

		per = (rcb.right - rc.left) * 100 / rc.Width();
		if (per <= m_curper) {
			if (m_crFrom == m_crTo) {
				cr = m_crFrom;
			} else {
				int r1 = GetRValue(m_crFrom);
				int g1 = GetGValue(m_crFrom);
				int b1 = GetBValue(m_crFrom);
				int r2 = GetRValue(m_crTo);
				int g2 = GetGValue(m_crTo);
				int b2 = GetBValue(m_crTo);
				double dr = (double)((r2 - r1) / 100.0);
				double dg = (double)((g2 - g1) / 100.0);
				double db = (double)((b2 - b1) / 100.0);
				cr = RGB(r1 + (dr * (double)per), g1 + (dg * (double)per), b1 + (db * (double)per));
			}
			if (Os7()) {
				dcw.FillSolidRect(&rcb, cr);
			} else {
				_FillG(dcw, rcb, cr);
			}
		} else {
			if (IsThemeNull())
				dcw.FillSolidRect(&rcb, GetSysColor(COLOR_BTNFACE));
		}
		rcb.OffsetRect(xb + 2, 0);
	}
	dcw.SetBkColor(crOld);

	dc.BitBlt(rcc.left, rcc.top, rcc.Width(), rcc.Height(), dcw, 0, 0, SRCCOPY);
	DeleteObject(dcw.SelectBitmap(hbm));
}

LRESULT CColoredProgressBarCtrl::SetRange(INT_PTR vmin, INT_PTR vmax)
{
	INT_PTR vmin_old = 0;
	INT_PTR vmax_old = 0;
	if (vmin != vmax) {
		vmin_old = m_vmin;
		vmax_old = m_vmax;
		if (vmin_old != vmin || vmax_old != vmax) {
			m_vmin = min(vmin, vmax);
			m_vmax = max(vmin, vmax);
			m_curpos = max(m_vmin, min(m_curpos, m_vmax));
			m_curper = (int)(((__int64)m_curpos * 100) / (__int64)(m_vmax - m_vmin));
			RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
	}
	return MAKELONG(vmin_old, vmax_old);
}

LRESULT CColoredProgressBarCtrl::SetPos(INT_PTR pos)
{
	INT_PTR pos_old = m_curpos;
	if (pos != pos_old) {
		m_curpos = max(m_vmin, min(pos, m_vmax));
		m_curper = (int)(((__int64)m_curpos * 100) / (__int64)(m_vmax - m_vmin));
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	return pos_old;
}

LRESULT CColoredProgressBarCtrl::GetRange(BOOL fWhichLimit, PBRANGE* ppbr)
{
	ppbr->iLow = (int)m_vmin;
	ppbr->iHigh = (int)m_vmax;
	return fWhichLimit ? m_vmin : m_vmax;
}

LRESULT CColoredProgressBarCtrl::SetStep(INT_PTR step)
{
	INT_PTR ret = m_step;
	if (step > 0 && (step < (m_vmax - m_vmin)))
		m_step = step;
	return ret;
}

LRESULT CColoredProgressBarCtrl::DeltaPos(INT_PTR deltapos)
{
	return SetPos(m_curpos + deltapos);
}

LRESULT CColoredProgressBarCtrl::StepIt()
{
	return SetPos(m_curpos + m_step);
}

LRESULT CColoredProgressBarCtrl::SetBarColor(COLORREF crFrom, COLORREF crTo)
{
	COLORREF crOldFrom = m_crFrom;
	COLORREF crOldTo = m_crTo;
	m_crFrom = crFrom;
	if (m_numbits <= 8) {
		m_crTo = m_crFrom;
	} else {
		m_crTo = crTo;
	}
	if (crOldFrom != m_crFrom || crOldTo != m_crTo)
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	return crOldFrom;
}

////////////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK ColoredProgressBarWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CColoredProgressBarCtrl* pWnd = new CColoredProgressBarCtrl();
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}
