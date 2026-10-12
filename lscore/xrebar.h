////////////////////////////////////////////////////////////////////////
// xrebar.h

#pragma once
#ifndef __XREBAR_H__
#define __XREBAR_H__

#include "osplatform.h"

class CXRebarCtrl :	public CWindowImpl<CXRebarCtrl, CReBarCtrl>
{
private:
	bool m_bUseLines:1;
	bool m_bRoundBorders:1;
	bool m_bIsDragging:1;
	HWND m_hwndMenubar;
	COSPlatform m_osp;

public:
	CXRebarCtrl()
		: m_bRoundBorders(true)
		, m_bUseLines(false)
		, m_bIsDragging(false)
		, m_hwndMenubar(NULL)
	{
	}

	void SetMenuBar(HWND hwndMenubar)
	{
		m_hwndMenubar = hwndMenubar;
	}

BEGIN_MSG_MAP(CXRebarCtrl)
	MESSAGE_HANDLER(WM_LBUTTONDOWN, OnHandlerLButtonDown)
	MESSAGE_HANDLER(WM_LBUTTONUP, OnHandlerLButtonUp)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnHandlerEraseBkgnd)
END_MSG_MAP()

	LRESULT OnHandlerLButtonDown(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_bIsDragging = true;
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnHandlerLButtonUp(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_bIsDragging = false;
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnHandlerEraseBkgnd(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (!m_osp.IsThemed()) {
			bHandled = FALSE;
			return 1;
		}

		CDCHandle dc = (HDC)wParam;

		// client size
		CRect rc;
		GetClientRect(&rc);
		int barWidth = rc.Width();
		int barHeight = rc.Height();

		// vertical
		bool bVertical = ((GetStyle() & CCS_VERT) != 0);

		// Create and set up our memory DC
		CDC dcm;
		dcm.CreateCompatibleDC(dc);
		CBitmap bm;
		bm.CreateCompatibleBitmap(dc, barWidth, barHeight);
		HBITMAP hbmw = dcm.SelectBitmap(bm);

		// Draw to Rebar background to the memory DC
		COLORREF clrBkgndT = GetSysColor(COLOR_3DFACE);
		COLORREF clrBkgndF = xrb_mix_color(clrBkgndT, GetSysColor(COLOR_WINDOW), 50);

		rc.right = 600;
		xrb_gradfill(dcm, rc, clrBkgndF, clrBkgndT, false);
		if (barWidth >= 600) {
			rc.left = 600;
			rc.right = barWidth;
			dcm.FillSolidRect(&rc, clrBkgndT);
		}

		// Draw the individual band backgrounds
		COLORREF clrBandF = xrb_mix_color(clrBkgndT, GetSysColor(COLOR_WINDOW), 30);
		COLORREF clrBandT = xrb_mix_color(clrBkgndT, GetSysColor(COLOR_3DSHADOW), 50);
		COLORREF clrBandS = xrb_mix_color(clrBkgndT, GetSysColor(COLOR_WINDOW), 20);
		COLORREF clrBandE = xrb_mix_color(clrBkgndT, GetSysColor(COLOR_3DSHADOW), 20);

		// menubarは対象から外す
		int indexMenubar = GetBandFromWindow(m_hwndMenubar);

		if (!bVertical) {

			CDC dcMask, dcShade;
			dcMask.CreateCompatibleDC(dc);
			dcShade.CreateCompatibleDC(dc);

			int numBands = GetBandCount();
			for (int i = 0 ; i < numBands; i++) {
				REBARBANDINFO rbbi = { RunTimeHelper::SizeOf_REBARBANDINFO(), RBBIM_STYLE | RBBIM_CHILD | RBBIM_IDEALSIZE };
				GetBandInfo(i, &rbbi);

				// バンドが非表示でなく且つメニューバーでない場合
				if (!(rbbi.fStyle & RBBS_HIDDEN) && (i != indexMenubar)) {
					// Determine the size of this band
					CRect rcBand;
					GetRect(i, &rcBand);

					CRect rcBorder;
					GetBandBorders(i, &rcBorder);

					int cxBand = rcBand.Width();
					int cyBand = rcBand.Height();

					// バンドの子ウィンドウがツールバーであれば、ツールバーエンド位置を計算して、
					// その位置でRoundRectが終了するように描画する。
					bool bChevronVisible = false;
					CRect rcChild;
					::GetWindowRect(rbbi.hwndChild, &rcChild);
					CRect rcw;
					int delta = 0;
					int nBtnCount = (int)::SendMessage(rbbi.hwndChild, TB_BUTTONCOUNT, 0, 0L);
					if (nBtnCount > 0) {
						if ((UINT)rcChild.Width() > rbbi.cxIdeal) {
							if (::SendMessage(rbbi.hwndChild, TB_GETITEMRECT, nBtnCount - 1, (LPARAM)&rcw)) {
								delta = cxBand - rcw.right - rcBorder.left;
							}
						}
						else if ((UINT)rcChild.Width() < rbbi.cxIdeal) {
							bChevronVisible = true;
						}
					}

					// RoundRectマスクの作成
					CBitmap bmMask;
					bmMask.CreateBitmap(cxBand, cyBand, 1, 1, NULL);
					HBITMAP hbmMaskOld = dcMask.SelectBitmap(bmMask);
					dcMask.PatBlt(0, 0, cxBand, cyBand, WHITENESS);
					HPEN hpenOld = dcMask.SelectPen((HPEN)::GetStockObject(BLACK_PEN));
					HBRUSH hbrOld = dcMask.SelectBrush((HBRUSH)::GetStockObject(BLACK_BRUSH));
					dcMask.RoundRect(0, 0, cxBand - delta, cyBand, 6, 6);			// 矩形幅のdelta値による補正
					dcMask.SelectBrush(hbrOld);
					dcMask.SelectPen(hpenOld);

					// グラデーションの作成
					CBitmap bmShade;
					bmShade.CreateCompatibleBitmap(dc, cxBand, cyBand);
					HBITMAP hbmShadeOld = dcShade.SelectBitmap(bmShade);
					rc.SetRect(0, 0, cxBand, cyBand / 3);
					dcShade.FillSolidRect(&rc, clrBandF);
					rc.top = rc.bottom;
					rc.bottom = cyBand;
					xrb_gradfill(dcShade, rc, clrBandF, clrBandT, true);
					// 上端のより薄いライン
					rcw.SetRect(0, 0, cxBand, 1);
					dcShade.FillSolidRect(&rcw, clrBandS);
					// 下端のより濃いライン
					rcw.SetRect(0, cyBand - 1, cxBand, cyBand);
					dcShade.FillSolidRect(&rcw, clrBandE);

					// 転送
					dcm.BitBlt(rcBand.left, rcBand.top, cxBand, cyBand, dcShade, 0, 0, SRCINVERT);
					dcm.BitBlt(rcBand.left, rcBand.top, cxBand, cyBand, dcMask, 0, 0, SRCAND);
					dcm.BitBlt(rcBand.left, rcBand.top, cxBand, cyBand, dcShade, 0, 0, SRCINVERT);

					dcShade.SelectBitmap(hbmShadeOld);
					bmShade.DeleteObject();
					dcMask.SelectBitmap(hbmMaskOld);
					bmMask.DeleteObject();

					// バンドドラッグ中のバンドエンド（右側）のジャギーを処理する
					if (m_bIsDragging) {
						CClientDC dcRebar(m_hWnd);
						dcRebar.BitBlt(rcBand.left + rcBorder.left, rcBand.top, cxBand - rcBorder.left, cyBand, dcm, rcBand.left + rcBorder.left, rcBand.top, SRCCOPY);
						if (bChevronVisible) {
							// シェブロン部分を強制描画
							rcw = rcBand;
							rcw.left += rcBorder.left + rcChild.Width();
							RedrawWindow(&rcw, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_NOCHILDREN);
						}
					}
				}
			}

			if (m_bUseLines) {
				// Draw lines between bands
				for (int j = 0; j < numBands - 1; j++) {
					GetRect(j, &rc);
					rc.left = max(0, rc.left - 4);
					rc.bottom += 2;
					dcm.DrawEdge(&rc, EDGE_ETCHED, BF_BOTTOM | BF_ADJUST);
				}
			}

		}	// !vertical

		// Copy the Memory DC to the window's DC
		dc.BitBlt(0, 0, barWidth, barHeight, dcm, 0, 0, SRCCOPY);

		// Cleanup
		dcm.SelectBitmap(hbmw);

		return TRUE;
	}

	int GetBandFromWindow(const HWND hWnd)
	{
		if (::IsWindow(hWnd)) {
			int numBands = GetBandCount();
			for (int i = 0; i < numBands; i++) {
				REBARBANDINFO rbbi = { RunTimeHelper::SizeOf_REBARBANDINFO(), RBBIM_CHILD };
				GetBandInfo(i, &rbbi);
				if (rbbi.hwndChild == hWnd)
					return i;
			}
		}
		return -1;
	}

	inline COLORREF xrb_mix_color(COLORREF c1, COLORREF c2, int t)
	{
		return RGB(	(t * GetRValue(c1) + (100 - t) * GetRValue(c2)) / 100,
					(t * GetGValue(c1) + (100 - t) * GetGValue(c2)) / 100,
					(t * GetBValue(c1) + (100 - t) * GetBValue(c2)) / 100	);
	}

	void xrb_gradfill(HDC hdc, CRect& rc, COLORREF crFrom, COLORREF crTo, bool bVert = true)
	{
		TRIVERTEX vert[2];
		GRADIENT_RECT grc;

		vert[0].x     = rc.left;
		vert[0].y     = rc.top;
		vert[0].Red   = GetRValue(crFrom) << 8;
		vert[0].Green = GetGValue(crFrom) << 8;
		vert[0].Blue  = GetBValue(crFrom) << 8;
		vert[0].Alpha = 0x0000;

		vert[1].x     = rc.right;
		vert[1].y     = rc.bottom;
		vert[1].Red   = GetRValue(crTo) << 8;
		vert[1].Green = GetGValue(crTo) << 8;
		vert[1].Blue  = GetBValue(crTo) << 8;
		vert[1].Alpha = 0x0000;

		grc.UpperLeft  = 0;
		grc.LowerRight = 1;
		GradientFill(hdc, vert, 2, &grc, 1, bVert ? GRADIENT_FILL_RECT_V : GRADIENT_FILL_RECT_H);
	}
};


#endif		// __XREBAR_H__
