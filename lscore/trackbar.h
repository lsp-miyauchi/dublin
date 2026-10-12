//////////////////////////////////////////////////////////////////////
// trackbar.h degree control
//

#ifndef __DEGREE_INCLUDE

//#define THUMBWIDTH				15
//#define THUMBHEIGHT				8
#define THUMBWIDTH				11
#define THUMBHEIGHT				12

class CTrackCtrl : public CWindowImpl<CTrackCtrl>
{
private:
	int m_min;
	int m_max;
	int m_pos;
	int m_step;
	int m_page;
	int m_selmin;
	int m_selmax;
	int m_freq;
	BOOL m_bCaptured;
	BOOL m_bKeyPressed;
	int m_buddyBase;
	HIMAGELIST m_him;
	HIMAGELIST m_himUser;
	HFONT m_hfont;
	CSize m_sThumb;
	TCHAR* m_suffix;

public:
	CTrackCtrl();
	~CTrackCtrl();

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

BEGIN_MSG_MAP(CTrackCtrl)
	ON_WM_CREATE()
	ON_WM_GETDLGCODE()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SETFONT()
	ON_MESSAGE_VOID(WM_ENABLE, OnEnable)
	ON_WM_SETTEXT()
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
	ON_MESSAGE_VOID(TBM_SETSEL, OnHandlerSetSel)
	ON_MESSAGE_VOID(TBM_SETBUDDYBASE, OnHandlerSetBuddyBase)
	ON_MESSAGE_VOID(TBM_SETTICFREQ, OnHandlerSetTicFreq)
	ON_MESSAGE_VOID(TBM_SETSUFFIX, OnHandlerSetSuffix)
	ON_MESSAGE(TBM_GETPOS, OnHandlerGetPos)
	ON_MESSAGE(TBM_SETTHUMBIMAGE, OnHandlerSetThumbImage)
	ON_MESSAGE(TBM_GETTHUMBIMAGE, OnHandlerGetThumbImage)
END_MSG_MAP()

	// メッセージマップ
	int OnCreate(LPCREATESTRUCT pcs);
	void OnSize(UINT nType, int cx, int cy);
	BOOL OnEraseBkgnd(HDC hdc);
	UINT OnGetDlgCode() { return DLGC_WANTARROWS; }
	void OnSetFocus(HWND hOldWnd);
	void OnSetText(LPCTSTR pszText, BOOL& bHandled);
	void OnKillFocus(HWND hNewWnd);
	void OnPrintClient(HDC hdc, LPARAM flags);
	void OnPaint();
	void OnSetFont(HFONT hFont, BOOL bRedraw);
	void OnEnable(WPARAM wParam, LPARAM lParam);
	void OnLButtonDown(UINT flags, CPoint point);
	void OnLButtonUp(UINT flags, CPoint point);
	void OnMouseMove(UINT flags, CPoint point);
	BOOL OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	BOOL OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnCancelMode();
	void OnHandlerSetRange(WPARAM wParam, LPARAM lParam);
	void OnHandlerSetPos(WPARAM wParam, LPARAM lParam);
	void OnHandlerSetPageSize(WPARAM wParam, LPARAM lParam);
	void OnHandlerSetSel(WPARAM wParam, LPARAM lParam);
	void OnHandlerSetBuddyBase(WPARAM wParam, LPARAM lParam);
	void OnHandlerSetTicFreq(WPARAM wParam, LPARAM lParam);
	LRESULT OnHandlerGetPos(WPARAM wParam, LPARAM lParam);
	LRESULT OnHandlerSetThumbImage(WPARAM wParam, LPARAM lParam);
	LRESULT OnHandlerGetThumbImage(WPARAM wParam, LPARAM lParam);
	void OnHandlerSetSuffix(WPARAM wParam, LPARAM lParam);

	void OnDraw(HDC hdc);
	void DrawThumb(HDC hdc);
	void ChangeFocus(BOOL bHaveFocus);
	void SetRange(int nMin, int nMax, BOOL bRedraw);
	void SetSelection(int nMin, int nMax, BOOL bRedraw);
	void SetPos(int pos, BOOL bRedraw);
	int SetPageSize(int page);
	void SetBuddyBase(int buddyBase);
	void SetTicFreq(int nFreq);
	HIMAGELIST SetThumbImage(HIMAGELIST him);
	void SetSuffix(LPCTSTR pszSuffix);
};

_inline BOOL CTrackCtrl::OnEraseBkgnd(HDC hdc)
	{ return TRUE; }
_inline void CTrackCtrl::OnEnable(WPARAM wParam, LPARAM lParam)
	{ Invalidate(); }
_inline void CTrackCtrl::OnSetFocus(HWND hOldWnd)
	{ ChangeFocus(TRUE); }
_inline void CTrackCtrl::OnKillFocus(HWND hNewWnd)
	{ ChangeFocus(FALSE); }
_inline void CTrackCtrl::OnHandlerSetRange(WPARAM wParam, LPARAM lParam)
	{ SetRange((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (BOOL)wParam); }
_inline void CTrackCtrl::OnHandlerSetPos(WPARAM wParam, LPARAM lParam)
	{ SetPos((int)lParam, (BOOL)wParam); }
_inline void CTrackCtrl::OnHandlerSetPageSize(WPARAM wParam, LPARAM lParam)
	{ SetPageSize((int)lParam); }
_inline void CTrackCtrl::OnHandlerSetSel(WPARAM wParam, LPARAM lParam)
	{ SetSelection((int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam), (BOOL)wParam); }
_inline void CTrackCtrl::OnHandlerSetBuddyBase(WPARAM wParam, LPARAM lParam)
	{ SetBuddyBase((int)wParam); }
_inline void CTrackCtrl::OnHandlerSetTicFreq(WPARAM wParam, LPARAM lParam)
	{ SetTicFreq((int)wParam); }
_inline LRESULT CTrackCtrl::OnHandlerGetPos(WPARAM wParam, LPARAM lParam)
	{ return (LRESULT)m_pos; }
_inline LRESULT CTrackCtrl::OnHandlerSetThumbImage(WPARAM wParam, LPARAM lParam)
	{ return (LRESULT)SetThumbImage((HIMAGELIST)lParam); }
_inline LRESULT CTrackCtrl::OnHandlerGetThumbImage(WPARAM wParam, LPARAM lParam)
	{ return (LRESULT)m_himUser; }
_inline void CTrackCtrl::OnHandlerSetSuffix(WPARAM wParam, LPARAM lParam)
	{ SetSuffix((LPCTSTR)lParam); }


#endif		// __DEGREE_INCLUDEE
