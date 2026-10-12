///////////////////////////////////////////////////////////////////////
// view_histogram.h

#pragma once
#ifndef __VIEW_HISTOGRAM_H__
#define __VIEW_HISTOGRAM_H__

#include "container.h"
#include "histogram_calc.h"
#include "threadhandle.h"


///////////////////////////////////////////////////////////////////////
//

class CHistogramView : public CContainerImpl<CHistogramView>
{
private:
	CBitmap m_bmpHistogram;
	CHistogramInfo m_info;
	int m_mode;
	CFont m_font;
	UINT m_wm_histogram_calcdone;
	CThreadHandle m_thread;

public:
	CHistogramView()
		: m_mode(HISTOGRAM_COLOR)
		, m_wm_histogram_calcdone(::RegisterWindowMessage(_T("vwh_calcdone")))
	{
		SetTitle(_T("Histogram"));
		SetBkColor(LCOLOR_IMAGEPANE);
		m_info.SetBkColor(LCOLOR_IMAGEPANE);
	}

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
	}

	void ThreadExit()
	{
		if (m_thread.IsRunning()) {
			// スレッド終了を待つ
			m_thread.WaitForExit(INFINITE);
		}
		m_thread.Close();
	}

BEGIN_MSG_MAP(CHistogramView)
	CHAIN_MSG_MAP(CContainerImpl<CHistogramView>)
	ON_WM_DESTROY()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CONTEXTMENU()
	ON_REGISTERD_MESSAGE(m_wm_histogram_calcdone, OnHandlerCalcDone)
END_MSG_MAP()

	void OnDestroy()
	{
		ThreadExit();
	}

	void OnDraw(CDCHandle dc, CRect& rc)
	{
		if (!m_bmpHistogram.IsNull()) {
			BITMAP bm; m_bmpHistogram.GetBitmap(&bm);
			CDC dcw; dcw.CreateCompatibleDC(dc);

			HBITMAP hbm = dcw.SelectBitmap(m_bmpHistogram);
			dc.SetStretchBltMode(HALFTONE);
			dc.StretchBlt(rc.left, rc.top, rc.Width(), rc.Height(), dcw, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

			dcw.SelectBitmap(hbm);

			CPen pen; pen.CreatePen(PS_DOT, 0, RGB(128, 128, 128));
			HPEN hpen = dc.SelectPen(pen);
			float xDelta = rc.Width() / 4.0F;
			for (int i = 0; i < 3; i++) {
				dc.MoveTo((int)(xDelta + xDelta * i), rc.top);
				dc.LineTo((int)(xDelta + xDelta * i), rc.bottom);
			}
			dc.SelectPen(hpen);
		}
		else
			dc.FillSolidRect(&rc, LCOLOR_IMAGEPANE);
	}

	void OnLButtonDblClk(UINT fwKeys, CPoint point)
	{
		CRect rc;
		GetPaneRect(&rc);
		if (rc.PtInRect(point)) {
			switch (m_mode) {
			case HISTOGRAM_VALUE: m_mode = HISTOGRAM_COLOR; break;
			case HISTOGRAM_COLOR: m_mode = HISTOGRAM_MULTI; break;
			default:              m_mode = HISTOGRAM_VALUE; break;
			}
			UpdateHitogramBitmap();
		}
	}

	void OnContextMenu(HWND hWnd, CPoint pt)
	{
		CMenu pop; pop.CreatePopupMenu();
		pop.AppendMenu(MF_STRING, HISTOGRAM_COLOR, _T("RGB混合グラフ(&A)"));
		pop.AppendMenu(MF_STRING, HISTOGRAM_MULTI, _T("RGB個別グラフ(&M)"));
		pop.AppendMenu(MF_STRING, HISTOGRAM_VALUE, _T("輝度グラフ(&V)"));
		pop.CheckMenuRadioItem(HISTOGRAM_VALUE, HISTOGRAM_MULTI, m_mode, MF_BYCOMMAND);
		int id = pop.TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
		if (id > 0) {
			m_mode = id;
			UpdateHitogramBitmap();
		}
	}

	void UpdateHitogramBitmap()
	{
		m_bmpHistogram.Attach(m_info.MakeHistogramBitmap(m_mode));
		Invalidate();
	}

	void Calc()
	{
		BOOL ok = FALSE;
		Bitmap* bitmap = (Bitmap*)m_info.m_userdata;
		if (bitmap) {
			ok = m_info.Initialize(bitmap);
			delete bitmap;
		}
		::PostMessage(m_hWnd, m_wm_histogram_calcdone, (WPARAM)ok, 0);
	}

	static unsigned int __stdcall _ThreadProcEntry(void* param)
	{
		CHistogramView* view = (CHistogramView*)param;
		if (view)
			view->Calc();
		_endthreadex(0);
		return 0;
	}

	LRESULT OnHandlerCalcDone(WPARAM wParam, LPARAM lParam)
	{
		m_thread.WaitForExit(INFINITE);
		if (wParam)
			UpdateHitogramBitmap();
		return 0;
	}

	bool CalcHistogram(HDIB hdib)
	{
		if (hdib) {
			// ヒストグラム計算スレッド起動
			if (m_thread.IsRunning())
				return false;

			m_info.m_userdata = (LPARAM)GdipGetBitmapFromDIB(hdib);
			m_thread.Begin(_ThreadProcEntry, this, CREATE_SUSPENDED);
			m_thread.Resume();
		}
		else {
			m_bmpHistogram.DeleteObject();
			Invalidate();
		}
		return true;
	}
};


#endif		// __VIEW_HISTOGRAM_H__
