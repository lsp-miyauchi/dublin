//////////////////////////////////////////////////////////////////////
// preview.h
//

#ifndef __PREVIEW_H_INCLUDE
#define __PREVIEW_H_INCLUDE

class CPreView
{
public:
	CPreView();
	virtual ~CPreView();

	static const SIZE sizeDefault;
		// used to specify default calculated page and line sizes

	// 描画
	void SetRedraw(BOOL bRedraw = TRUE);
	void Invalidate(BOOL bErase = TRUE);

	LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
// CWnd インプリメンテーション
	HWND GetSafeHwnd();
	DWORD GetStyle();
	DWORD GetExStyle();
	int GetScrollPos(int nBar) const;
	int SetScrollPos(int nBar, int nPos);
	BOOL GetScrollInfo(int nBar, SCROLLINFO* pInfo, UINT nMask = SIF_ALL);
	BOOL SetScrollInfo(int nBar, SCROLLINFO* pInfo);
	BOOL GetScrollRange(int fnBar, int& nMin, int& nMax);
	BOOL SetScrollRange(int fnBar, int nMin, int nMax);
	LRESULT Default();

// CScrollView インプリメンテーション
	// in logical units - call one of the following Set routines
	void SetZoomSizes(SIZE sizeTotal, const SIZE& sizePage = sizeDefault, const SIZE& sizeLine = sizeDefault);
	int GetScrollLimit(int nBar);


// Attributes
public:
	CSize GetTotalSize() const;             // logical size

	// for device units
	CPoint GetDeviceScrollPosition() const;
	void CenterOnPoint(CPoint ptCenter);

// Operations
public:
	void ScrollToPosition(POINT pt);    // set upper left position
	void CenterOnLogicalPoint(CPoint pt);
	CPoint GetLogicalCenterPoint(void);

// Implementation
protected:
	int m_nMapMode;
	CSize m_totalLog;           // total size in logical units (no rounding)
	CSize m_totalDev;           // total size in device units
	CSize m_pageDev;            // per page scroll size in device units
	CSize m_lineDev;            // per line scroll size in device units
	CSize m_origTotalDev;           // Origional total size in device units
	CSize m_origPageDev;            // Origional per page scroll size in device units
	CSize m_origLineDev;            // Origional per line scroll size in device units
	BOOL m_bCaptured;
	HCURSOR    m_hcHandOpen;
	HCURSOR    m_hcHandClose;
	BOOL m_bCenter;             // Center output if larger than total size
	BOOL m_bInsideUpdate;       // internal state for OnSize callback
	MSG m_msg;
	CPoint m_ptPrev;
	CPoint m_ptTrack;
	COLORREF m_crBkgnd;
	SCROLLINFO m_vscroll;
	SCROLLINFO m_hscroll;
	mif* m_image;
	mif* m_image_s;

	HBITMAP m_hbm;
	HBITMAP m_hbmEffect;
	HBITMAP m_hbmBack;
	HPALETTE m_hpal;
	HPALETTE m_hpalEffect;

	BOOL m_bHScroll;
	BOOL m_bVScroll;
	BOOL m_bWhole;

public:
	HWND m_hWnd;
	HWND m_hParent;

protected:
	virtual void OnDraw(HDC hdc);      // pass on pure virtual

	void UpdateBars();          // adjust scrollbars etc
	BOOL GetTrueClientSize(CSize& size, CSize& sizeSb);
		// size with no bars
	void GetScrollBarSizes(CSize& sizeSb);
	void GetScrollBarState(CSize sizeClient, CSize& needSb,
		CSize& sizeRange, CPoint& ptMove, BOOL bInsideClient);
	void ScrollToDevicePosition(POINT ptDev); // explicit scrolling no checking


private:
	void CalcBars(void);
	BOOL ViewDPtoLP (LPPOINT lpPoints, int nCount = 1);
	BOOL ViewLPtoDP (LPPOINT lpPoints, int nCount = 1);
	void ClientToDevice(CPoint &point);
	void NotifyUpdateEffect();

public:
	// Advanced: virtual AdjustWindowRect
	virtual void OnPrepareDC(HDC hdc);
	virtual void OnInitialUpdate(); // called first time after construct

protected:
	// メッセージマップ
	int OnCreate(LPCREATESTRUCT lpcs);
	void OnSize(UINT nType, int cx, int cy);
	void OnPaletteChanged(HWND hwndFocus);
	BOOL OnQueryNewPalette();
	BOOL OnEraseBkgnd(HDC hdc);
	void OnPaint();
	BOOL OnSetCursor(HWND hwnd, UINT nHitTest, UINT message);
	BOOL OnSetDIB(HDIB hdib);
	void OnSetWholeView(BOOL bWhole);
	void OnLButtonDown(UINT flags, CPoint point);
	void OnLButtonUp(UINT flags, CPoint point);
	void OnMouseMove(UINT flags, CPoint point);
};

__forceinline HWND CPreView::GetSafeHwnd()
	{ return ::IsWindow(m_hWnd) ? m_hWnd : NULL; }
__forceinline void CPreView::Invalidate(BOOL bErase)
	{ if (::IsWindow(m_hWnd)) InvalidateRect(m_hWnd, NULL, bErase); }
__forceinline DWORD CPreView::GetStyle()
	{ return (DWORD)GetWindowLong(m_hWnd, GWL_STYLE); }
__forceinline DWORD CPreView::GetExStyle()
	{ return (DWORD)GetWindowLong(m_hWnd, GWL_EXSTYLE); }
__forceinline void CPreView::SetRedraw(BOOL bRedraw)
	{ SendMessage(m_hWnd, WM_SETREDRAW, (WPARAM)bRedraw, 0); }

#endif				// __PREVIEW_H_INCLUDE
