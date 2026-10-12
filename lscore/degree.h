//////////////////////////////////////////////////////////////////////
// degree.h degree control
//

#ifndef __DEGREE_INCLUDE

#define THUMBWIDTH				10

class CDegreeCtrl : public CWindowImpl<CDegreeCtrl>,
					public CThemeImpl<CDegreeCtrl>
{
private:
	int m_min;
	int m_max;
	int m_pos;
	int m_xofs;
	int m_step;
	int m_page;
	BOOL m_bCaptured;
	CRect m_area;
	CSimpleArray<COLORREF> m_arrColor;
	CWindow m_buddy;
	int m_buddyBase;

public:
	CDegreeCtrl();
	~CDegreeCtrl() { }

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

BEGIN_MSG_MAP(CDegreeCtrl)
	CHAIN_MSG_MAP(CThemeImpl<CDegreeCtrl>)
	ON_WM_GETDLGCODE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_MESSAGE_VOID(WM_ENABLE, OnEnable)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_PRINTCLIENT()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CANCELMODE()
	ON_MESSAGE_VOID(TBM_SETRANGE, OnHandlerSetRange)
	ON_MESSAGE_VOID(TBM_SETPOS, OnHandlerSetPos)
	ON_MESSAGE_VOID(TBM_SETPAGESIZE, OnHandlerSetPageSize)
	ON_MESSAGE_VOID(TBM_SETBUDDYBASE, OnHandlerSetBuddyBase)
	ON_MESSAGE(TBM_SETBUDDY, OnHandlerSetBuddy)
	ON_MESSAGE(TBM_GETPOS, OnHandlerGetPos)
	ON_MESSAGE_VOID(TBM_SETBKCOLORS, OnHandlerSetBkColors)
	ON_MESSAGE_VOID(TBM_SETBKCOLORSEX, OnHandlerSetBkColorsEx)
END_MSG_MAP()


protected:
	void OnDraw(HDC hdc);      // pass on pure virtual
	void DrawThumb(HDC hdc);
	void SetValueToBuddy();

protected:
	// メッセージマップ
	int OnCreate(LPCREATESTRUCT lpcs);
	UINT OnGetDlgCode() { return DLGC_WANTARROWS; }
	void OnSize(UINT nType, int cx, int cy);
	BOOL OnEraseBkgnd(HDC hdc);
	void OnEnable(WPARAM wParam, LPARAM lParam);
	void OnSetFocus(HWND hOldWnd);
	void OnKillFocus(HWND hOldWnd);
	void OnPrintClient(HDC hdc, LPARAM flags);
	void OnPaint();
	void OnLButtonDown(UINT flags, CPoint point);
	void OnLButtonUp(UINT flags, CPoint point);
	void OnMouseMove(UINT flags, CPoint point);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnCancelMode();
	void OnHandlerSetRange(WPARAM wParam, LPARAM lParam);
	void OnHandlerSetPos(WPARAM wParam, LPARAM lParam);
	void OnHandlerSetPageSize(WPARAM wParam, LPARAM lParam);
	void OnHandlerSetBuddyBase(WPARAM wParam, LPARAM lParam);
	LRESULT OnHandlerSetBuddy(WPARAM wParam, LPARAM lParam);
	LRESULT OnHandlerGetPos(WPARAM wParam, LPARAM lParam);
	void OnHandlerSetBkColors(WPARAM wParam, LPARAM lParam);
	void OnHandlerSetBkColorsEx(WPARAM wParam, LPARAM lParam);

	void ChangeFocus(BOOL bHaveFocus);
	void SetRange(int nMin, int nMax, BOOL bRedraw);
	void SetPos(int pos, BOOL bRedraw);
	int SetPageSize(int page);
	void SetBuddyBase(int buddyBase);
	HWND SetBuddy(HWND hBuddy);
	void SetBkColors(COLORREF rgb1, COLORREF rgb2);
	void SetBkColorsEx(COLORREF* crs, int numcolors);
};

_inline void CDegreeCtrl::OnSetFocus(HWND hOldWnd)
	{ ChangeFocus(TRUE); }
_inline void CDegreeCtrl::OnKillFocus(HWND hNewWnd)
	{ ChangeFocus(FALSE); }
_inline void CDegreeCtrl::OnEnable(WPARAM wParam, LPARAM lParam)
	{ Invalidate(); }
_inline void CDegreeCtrl::OnHandlerSetRange(WPARAM wParam, LPARAM lParam)
	{ SetRange((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (BOOL)wParam); }
_inline void CDegreeCtrl::OnHandlerSetPos(WPARAM wParam, LPARAM lParam)
	{ SetPos((int)lParam, (BOOL)wParam); }
_inline void CDegreeCtrl::OnHandlerSetPageSize(WPARAM wParam, LPARAM lParam)
	{ SetPageSize((int)lParam); }
_inline LRESULT CDegreeCtrl::OnHandlerGetPos(WPARAM wParam, LPARAM lParam)
	{ return (LRESULT)m_pos; }
_inline void CDegreeCtrl::OnHandlerSetBuddyBase(WPARAM wParam, LPARAM lParam)
	{ SetBuddyBase((int)wParam); }
_inline LRESULT CDegreeCtrl::OnHandlerSetBuddy(WPARAM wParam, LPARAM lParam)
	{ return (LRESULT)SetBuddy((HWND)lParam); }
_inline void CDegreeCtrl::OnHandlerSetBkColors(WPARAM wParam, LPARAM lParam)
	{ SetBkColors((COLORREF)lParam, (COLORREF)wParam); }
_inline void CDegreeCtrl::OnHandlerSetBkColorsEx(WPARAM wParam, LPARAM lParam)
	{ SetBkColorsEx((COLORREF*)lParam, (int)wParam); }


#endif		// __DEGREE_INCLUDEE
