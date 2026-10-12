// tasklink.cpp

#define TASKLINKCLASSNAMEW    L"TaskLink32"
#define TASKLINKCLASSNAMEA    "TaskLink32"

#ifdef  UNICODE
#define TASKLINKCLASSNAME     TASKLINKCLASSNAMEW
#else
#define TASKLINKCLASSNAME     TASKLINKCLASSNAMEA
#endif

#define WM_CALCDIMENSIONS     (WM_USER+0x1000)
#define WM_GETORIGDIMENSIONS  (WM_USER+0x1001)
#define WM_GETDLGITEM         (WM_USER+0x1002)


// TLS_NOAUTOSIZE
#define TLS_reserved          0x00000010L     // precedence over TLS_END_ELLIPSIS and TLS_WORDBREAK

#define TLS_TOOLTIP           0x00000001L
#define TLS_FOCUSRECT         0x00000002L
#define TLS_PLAYSOUND         0x00000004L
#define TLS_NOSETFOCUSONCLICK 0x00000008L
#define TLS_DONTRELAYLBDNUP   0x00000020L
#define TLS_BOLD              0x00000040L
#define TLS_NORMUNDERLINE     0x00000080L
#define TLS_HOTUNDERLINE      0x00000100L
#define TLS_END_ELLIPSIS      0x00000200L     // only used if !CHS_WORDBREAK
#define TLS_WORDBREAK         0x00000400L     
#define TLS_ICONRIGHT         0x00000800L     // place icon right

#define TLS_LEFT              0x00000000L
#define TLS_RIGHT             0x00001000L
#define TLS_CENTER            0x00002000L

#define TLS_MIDDLE            0x00000000L
#define TLS_TOP               0x00004000L
#define TLS_BOTTOM            0x00008000L

#define TLS_HALIGNMASK        0x00003000L
#define TLS_VALIGNMASK        0x0000C000L

#define TLM_SETFONTS          (WM_USER+0x0001)
#define TLM_SETCOLORS         (WM_USER+0x0002)
#define TLM_SETIMAGES         (WM_USER+0x0003)
#define TLM_GETTTCTRL         (WM_USER+0x0004)
#define TLM_SETCURSOR         (WM_USER+0x0006)
#define TLM_SETHSPACE         (WM_USER+0x0007)
#define TLM_GETHSPACE         (WM_USER+0x0008)
#define TLM_ACTIVATE          (WM_USER+0x0009)

#define TLN_GETDISPINFO       ((UINT)(-10000))


#define TLS_NOAUTOSIZE        TLS_reserved


typedef struct tagTLSTRUCT {
	int                       cbSize;
	RECT                      rcText;
	RECT                      rcIcon;
//  SIZE                      siText;

} TLSTRUCT, *PTLSTRUCT;


////////////////////////////////////////////////////////////////////////
//

// e.g. "Explorer\\Navigating"
// ".Default\\Minimize"
// ".Default\\Maximize"
static void PlaySoundThemeSound(LPCTSTR pcszRegAppSound)
{
	HKEY        hKey = NULL;
	TCHAR       szString[256];
	TCHAR       szWaveFile[MAX_PATH];
	TCHAR       szWaveFileExpanded[MAX_PATH];
	DWORD       dwType,dwSize = MAX_PATH;

	szWaveFile[0] = 0;
	wsprintf(szString, _T("AppEvents\\Schemes\\Apps\\%s\\.Current"), pcszRegAppSound);
	if (ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, szString, &hKey)) {
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, NULL, NULL, &dwType, (LPBYTE)szWaveFile, &dwSize)) {
			if (dwType == REG_EXPAND_SZ) {
				if (ExpandEnvironmentStrings(szWaveFile, szWaveFileExpanded, MAX_PATH))
					PlaySound(szWaveFileExpanded, NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT | SND_NOSTOP | SND_NOWAIT);
			}
			else if (dwType == REG_SZ) {
				PlaySound(szWaveFile, NULL, SND_ASYNC | SND_FILENAME | SND_NODEFAULT | SND_NOSTOP | SND_NOWAIT);
			}
		}
		RegCloseKey(hKey);
	}
}

////////////////////////////////////////////////////////////////////////
//

class CTaskLinkCtrl : public CWindowImpl<CTaskLinkCtrl>
{
protected:
	bool m_bLButtonDown:1;
	bool m_bTrackMouse:1;
	bool m_bKeyDown:1;
	bool m_bHasFocus:1;
	bool m_bEnabled:1;
	bool m_bActive:1;
	bool m_bMultiLine:1;
	bool m_bColorSet:1;

	COLORREF m_clrNormal;
	COLORREF m_clrHot;
	COLORREF m_clrPressed;
	COLORREF m_clrDisabled;
	HBRUSH m_hbrBkgnd;
	HFONT m_hfontNormal;
	HFONT m_hfontHot;
	HCURSOR m_hCursor;

	int m_iItemState;

	int m_iOrigWidth;
	int m_iOrigHeight;
	int m_iHorzSpace;
	CPoint m_ptMouse;

	HIMAGELIST m_himNormal;
	HIMAGELIST m_himHot;
	HIMAGELIST m_himDisabled;
	int        m_iImageIndex;

	CWindow m_tip;
	TLSTRUCT m_tls;

public:
	CTaskLinkCtrl()
	{
	}

	~CTaskLinkCtrl()
	{
	}

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

typedef CTaskLinkCtrlImpl< T, TBase, TWinTraits >	thisClass;
BEGIN_MSG_MAP(this)
	ON_WM_CREATE()
	ON_MESSAGE(WM_GETDLGCODE, OnGetDlgCode)
	MESSAGE_RANGE_HANDLER(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseMessages)
	ON_WM_DESTROY()
	ON_MESSAGE_VOID(WM_GETORIGDIMONSIONS, OnGetOrigDimensions)
	ON_MESSAGE_VOID(WM_CALCDIMONSIONS, OnCalcDimensions)
	ON_MESSAGE_VOID(WM_SETFONT, OnSetFont)
	ON_MESSAGE(WM_NCHITTEST, OnNcHitTest)
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_SYSCOLORCHANGE, OnSysColorChange)
	ON_WM_SIZE()
	ON_MESSAGE_VOID(WM_ENABLE, OnEnable)
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_MESSAGE_VOID(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_MOUSEMOVE()
	ON_MESSAGE_VOID(WM_SETFOCUS, OnSetFocus)
	ON_MESSAGE_VOID(WM_KILLFOCUS, OnKillFocus)
	ON_MESSAGE_VOID(WM_CAPTURECHANGED, OnCaptureChanged)
	ON_NOTIFY_CODE(TTN_NEEDTEXT, OnNotifyToolTipText)
	ON_WM_PAINT()
	ON_MESSAGE(TLM_SETFONTS, OnTlsSetFonts)
	ON_MESSAGE(TLM_SETCOLORS, OnTlsSetColors)
	ON_MESSAGE(TLM_SETIMAGES, OnTlsSetImages)
	ON_MESSAGE(TLM_GETTTCTRL, OnTlsGetTTCtrl)
	ON_MESSAGE(TLM_SETCURSOR, OnTlsSetCursor)
	ON_MESSAGE(TLM_SETHSPACE, OnTlsSetHorzSpace)
	ON_MESSAGE(TLM_GETHSPACE, OnTlsGetHorzSpace)
	ON_MESSAGE_VOID(TLM_ACTIVATE, OnTlsActivate)
END_MSG_MAP()

	int OnCreate(CREATESTRUCT* pcs);
	LRESULT OnGetDlgCode();
	LRESULT OnMouseMessages(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	void OnDestroy();
	void OnGetOrigDimensions(WPARAM wParam, LPARAM lParam);
	LRESULT OnCalcDimensions((WPARAM wParam, LPARAM lParam);
	void OnSetFont(WPARAM wParam, LPARAM lParam);
	LRESULT OnNcHitTest(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetCursor(WPARAM wParam, LPARAM lParam);
	BOOL OnEraseBkgnd(HDC hdc);
	void OnSysColorChange(WPARAM wParam, LPARAM lParam);
	void OnSize(UINT nType, int cx, int cy);
	void OnEnable(WPARAM wParam, LPARAM lParam);
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnLButtonDown(UINT nFlags, CPoint point)
	void OnLButtonUp(UINT nFlags, CPoint point)
	void OnMouseLeave(WPARAM wParam, LPARAM lParam);
	void OnMouseMove(UINT nFlags, CPoint point);
	void OnSetFocus(WPARAM wParam, LPARAM lParam);
	void OnKillFocus(WPARAM wParam, LPARAM lParam);
	void OnCaptureChanged(WPARAM wParam, LPARAM lParam);
	void OnNotifyToolTipText(int code, NMHDR* phdr, LRESULT* pResult);
	void OnPaint();
	LRESULT OnPaintClient(WPARAM wParam, LPARAM lParam);
	LRESULT OnTlsSetFonts(WPARAM wParam, LPARAM lParam);
	LRESULT OnTlsSetColors(WPARAM wParam, LPARAM lParam);
	LRESULT OnTlsSetImages(WPARAM wParam, LPARAM lParam);
	LRESULT OnTlsGetTTCtrl(WPARAM wParam, LPARAM lParam);
	LRESULT OnTlsSetCursor(WPARAM wParam, LPARAM lParam);
	LRESULT OnTlsSetHorzSpace(WPARAM wParam, LPARAM lParam);
	LRESULT OnTlsGetHorzSpace(WPARAM wParam, LPARAM lParam);
	void OnTlsActivate(WPARAM w2Param, LPARAM lParam);

	void GetHitTestRect(RECT* prc);
	void ResizeTTControl();
	void CheckNeedRepaint();
	BOOL GetItemSize(PSIZE pSI, int iRequiredWidth);
};

int CTaskLinkCtrl::OnCreate(CREATESTRUCT* pcs)
{
	LOGFONT lf;
	CSize si;
	memset(m_tls, 0, sizeof(TLSTRUCT));

	CRect rc;
	GetClientRect(&rc);
	m_iOrigWidth  = rc.right;
	m_iOrigHeight = rc.bottom;
	m_tls.cbSize  = sizeof(TLSTRUCT);
	m_iHorzSpace = 6;
	m_bActive = true;
	m_ptMouse.x = -1;
	m_ptMouse.y = -1;
	m_iItemState  = ITEMSTATE_COLD;

	LONG style = GetStyle();
	if (style & TLS_TOOLTIP) {
		m_tip = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, TOOLTIPS_CLASS,
											NULL, WS_POPUP | TTS_NOPREFIX,
											CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
											m_hWnd, NULL, NULL, NULL);
		if (!m_tip.IsWindow())
			return -1;

		m_tip.SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		ResizeTTControl();
	}

	m_bEnabled = (::IsWindowEnabled(m_hWnd) != FALSE);
	if (!m_bEnabled)
		m_iItemState = ITEMSTATE_DISABLED;
	if (m_tip.IsWindow())
		m_tip.SendMessage(TTM_ACTIVATE, (WPARAM)((m_bEnabled && m_bActive) ? TRUE : FALSE), 0L);

	m_hbrBkgnd  = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	m_clrNormal     = GetSysColor(COLOR_BTNTEXT);
	m_clrHot        = GetSysColor(COLOR_HOTLIGHT);
	m_clrPressed    = GetSysColor(COLOR_HOTLIGHT);
	m_clrDisabled   = GetSysColor(COLOR_GRAYTEXT);

	if (GetLogFont((HFONT)GetStockObject(DEFAULT_GUI_FONT),&lf)) {
		lf.lfWeight       = (style & TLS_BOLD) ? FW_BOLD : FW_NORMAL;
		lf.lfUnderline    = (style & TLS_NORMUNDERLINE) ? TRUE : FALSE;
		m_hfontNormal = CreateFontIndirect(&lf);

		lf.lfWeight       = (style & TLS_BOLD) ? FW_BOLD : FW_NORMAL;
		lf.lfUnderline    = (style & TLS_HOTUNDERLINE) ? TRUE : FALSE;
		m_hfontHot    = CreateFontIndirect(&lf);
	}

	if (GetItemSize(&si, -1)) {
		SetWindowPos(NULL, 0, 0, si.cx, si.cy, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
		ResizeTTControl();
	}
	return 0;
}

LRESULT CTaskLinkCtrl::OnGetDlgCode()
{
	return DLGC_BUTTON;
}

LRESULT CTaskLinkCtrl::OnMouseMessages(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LONG style = GetStyle();
	switch (uMsg) {
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		if (style & TLS_DONTRELAYLBDNUP)
			break;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		if ((style & TLS_TOOLTIP) && m_tip.IsWindow()) {
			MSG       msg;
			msg.hwnd    = m_hWnd;
			msg.message = uMsg;
			msg.wParam  = wParam;
			msg.lParam  = lParam;
			msg.pt.x    = ((int)(short)LOWORD(lParam));
			msg.pt.y    = ((int)(short)HIWORD(lParam));
			ClientToScreen(&msg.pt);
			m_tip.SendMessage(TTM_RELAYEVENT, (WPARAM)0, (LPARAM)&msg);
		}
		break;
	}
	bHandled = FALSE;
	return 1;
}

void CTaskLinkCtrl::OnDestroy()
{
	if (m_tip.IsWindow())
		m_tip.DestroyWindow();
	if (m_hbrBkgnd)
		DeleteObject(m_hbrBkgnd);
	if (m_hfontNormal)
		DeleteObject(m_hfontNormal);
	if (m_hfontHot)
		DeleteObject(m_hfontHot);
}

void CTaskLinkCtrl::OnSetFont(WPARAM wParam, LPARAM lParam)
{
	LOGFONT lf;
	CSize si;

	if (m_hfontNormal) {
		DeleteObject(m_hfontNormal);
		m_hfontNormal = NULL;
	}
	if (m_hfontHot) {
		DeleteObject(m_hfontHot);
		m_hfontHot = NULL;
	}

	LONG style = GetStyle();
	if (GetLogFont(wParam ? ((HFONT)wParam) : ((HFONT)GetStockObject(DEFAULT_GUI_FONT)), &lf) ) {
		lf.lfWeight       = (style & TLS_BOLD) ? FW_BOLD : FW_NORMAL;
		lf.lfUnderline    = (style & TLS_NORMUNDERLINE) ? TRUE : FALSE;
		m_hfontNormal = CreateFontIndirect(&lf);
		lf.lfWeight       = (style & TLS_BOLD) ? FW_BOLD : FW_NORMAL;
		lf.lfUnderline    = (style & TLS_HOTUNDERLINE) ? TRUE : FALSE;
		m_hfontHot    = CreateFontIndirect(&lf);

		if (GetItemSize(&si, -1)) {
			SetWindowPos(NULL, 0, 0, si.cx, si.cy, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOREDRAW);
			ResizeTTControl();
		}

		if (LOWORD(lParam))
			Invalidate();
	}
}

LRESULT CTaskLinkCtrl::OnNcHitTest(WPARAM wParam, LPARAM lParam)
{
	CPoint pt;
	CRect rc;

	GetHitTestRect(&rc);
	pt.x = (int)((short)LOWORD(lParam));
	pt.y = (int)((short)HIWORD(lParam));
	ScreenToClient(&pt);
	return (pt.PtInRect(&rc) ? HTCLIENT : HTNOWHERE);
}

LRESULT CTaskLinkCtrl::OnSetCursor(WPARAM wParam, LPARAM lParam)
{
	if (!m_bActive) {
		bHandled = FALSE;
		return 0;
	}

	if (LOWORD(lParam) == HTCLIENT)
		SetCursor(m_hCursor ? m_hCursor : LoadCursor(NULL, IDC_HAND));
	else
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	return TRUE;
}

BOOL CTaskLinkCtrk::OnEraseBkgnd(HDC hdc)
{
	return TRUE;
}

void CTaskLinkCtrl::OnGetOrigDimensions(WPARAM wParam, LPARAM lParam)
{
	*((int*)wParam) = m_iOrigWidth;
	*((int*)lParam) = m_iOrigHeight;
}

LRESULT CTaskLinkCtrl::OnCalcDimensions((WPARAM wParam, LPARAM lParam)
{
	if (IsBadWritePtr((VOID *)wParam, sizeof(SIZE)))
		return FALSE;

	BOOL              bReturn;
	pTLSSave = (PTLSTRUCT)malloc(sizeof(TLSTRUCT));
	if (!pTLSSave) return FALSE;
	memcpy(pTLSSave,pTLS,sizeof(TLSTRUCT));
	bReturn = GetItemSize(hwnd,pTLS,(PSIZE)wParam,(int)lParam);
	memcpy(pTLS,pTLSSave,sizeof(TLSTRUCT));
	return bReturn;
}

void CTaskLinkCtrl::OnSysColorChange(WPARAM wParam, LPARAM lParam)
{
	if (!m_bColorSet) {
		if (m_hbrBkgnd)
			DeleteObject(m_hbrBkgnd);
		m_hbrBkgnd  = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
		m_clrNormal     = GetSysColor(COLOR_BTNTEXT);
		m_clrHot        = GetSysColor(COLOR_HOTLIGHT);
		m_clrPressed    = GetSysColor(COLOR_HOTLIGHT);
		m_clrDisabled   = GetSysColor(COLOR_GRAYTEXT);
	}
}

void CTaskLinkCtrl::OnSize(UINT nType, int cx, int cy)
{
	CSize si;
	GetItemSize(&si, -1);
}

void CTaskLinkCtrl::OnEnable(WPARAM wParam, LPARAM lParam)
{
	m_bEnabled = (wParam ? true : false);
	CheckNeedRepaint();
	if ((GetStyle() & TLS_TOOLTIP) && m_tip.IsWindow())
		m_tip.SendMessage(TTM_ACTIVATE, (WPARAM)((m_bEnabled && m_bActive) ? TRUE : FALSE), 0L);
}

void CTaskLinkCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((nChar == VK_SPACE) &&
		m_bHasFocus &&
		m_bEnabled &&
		m_bActive &&
		(GetCapture() != m_hWnd)) {
		m_bKeyDown = true;
		CheckNeedRepaint();
	}
}

void CTaskLinkCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if ((nChar == VK_SPACE) &&
		m_bHasFocus &&
		m_bEnabled &&
		m_bActive &&
		m_bKeyDown) {
		m_bKeyDown = false;
		CheckNeedRepaint();

		if (m_bEnabled) {
			if (GetStyle() & TLS_PLAYSOUND)
				PlaySoundThemeSound(TEXT("Explorer\\Navigating"));
			GetParent().SendMessage(WM_COMMAND, (WPARAM)GetDlgCtrlID(), (LPARAM)m_hWnd);
		}
	}
}

void CTaskLinkCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!m_bActive || !m_bEnabled) return;

	m_ptMouse = point;
	m_bLButtonDown = true;
	if (!(GetStyle() & TLS_NOSETFOCUSONCLICK) && m_bEnabled)
		SetFocus();
	SetCapture();
	CheckNeedRepaint();
}

void CTaskLinkCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (!m_bActive || !m_bEnabled) return;

	m_ptMouse = point;
	if (m_bLButtonDown) {
		m_bLButtonDown = false;
		CheckNeedRepaint();

		CRect rc;
		GetClientRect(&rc);
		if (point.PtInRect(&rc)) {
			ReleaseCapture();
			if (m_bEnabled /*&& (m_iItemState==ITEMSTATE_HOT)*/) {
				if (GetStyle() & TLS_PLAYSOUND)
					PlaySoundThemeSound(TEXT("Explorer\\Navigating"));
				GetParent().SendMessage(WM_COMMAND, (WPARAM)GetDlgCtrlID(), (LPARAM)m_hWnd);
			}
		}
		else
			ReleaseCapture();
	}
}

void CTaskLinkCtrl::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	m_ptMosue.x = -1;
	m_ptMosue.y = -1;
	m_bTrackMouse = false;
	CheckNeedRepaint();
}

void CTaskLinkCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_bActive || !m_bEnabled) return;

	TRACKMOUSEEVENT tme;
	if (!m_bTrackMouse) {
		m_bTrackMouse = true;
		tme.cbSize        = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags       = TME_LEAVE;
		tme.dwHoverTime   = 0;
		tme.hwndTrack     = m_hWnd;
		TrackMouseEvent(&tme);
	}

	m_ptMouse = point;
	CheckNeedRepaint();
}

void CTaskLinkCtrl::OnSetFocus(WPARAM wParam, LPARAM lParam)
{
	m_bHasFocus = true;
	if (GetStyle() & TLS_FOCUSRECT)
		Invalidate();
}

void CTaskLinkCtrl::OnKillFocus(WPARAM wParam, LPARAM lParam)
{
	m_bHasFocus = false;
	m_bKeyDown = false;
	if (GetStyle() & TLS_FOCUSRECT) {
		Invalidate();
	}
}

void CTaskLinkCtrl::OnCaptureChanged(WPARAM wParam, LPARAM lParam)
{
	m_bLButtonDown = false;
	m_bTrackMouse  = false;
	m_ptMouse.x = -1;
	m_ptMouse.y = -1;
	CheckNeedRepaint();
}

void CTaskLinkCtrl::OnNotifyToolTipText(int id, NMHDR* phdr, LRESULT* pResult)
{
	NMTTDISPINFO* ptt = (NMTTDISPINFO*)phdr;
	HWND hwndSave = phdr->hwndFrom;
	UINT idSave = phdr->idFrom;
	phdr->hwndFrom = m_hWnd;
	phdr->idFrom   = GetDlgCtrlID();
	phdr->code     = TLN_GETDISPINFO;
	GetParent().SendMessage(WM_NOTIFY, (WPARAM)m_hWnd, (LPARAM)phdr);
	phdr->hwndFrom = hwndSave;
    phdr->idFrom   = idSave;
    phdr->code     = TTN_GETDISPINFO;
}

void CTaskLinkCtrl::OnPaint()
{
	CPaintDC dc(m_hWnd);
	CPoint pt(0, 0);
	OnDraw(dc, pt, TRUE, -1);
}

LRESULT CTaskLinkCtrl::OnPaintClient(WPARAM wParam, LPARAM lParam)
{
	if (wParam) {
		CDCHandle dc = (HDC)wParam;
		UINT uFlags = (UINT)lParam;
		CPoint pt(0, 0);

		if (!(!::IsWindowVisible(m_hWnd) && (uFlags & PRF_CHECKVISIBLE)))
			OnDraw(dc pt, uFlags & PRF_ERASEBKGND ? TRUE : FALSE, -1);
		return TRUE;
	}
	return FALSE;
}

LRESULT CTaskLinkCtrl::OnTlsSetFonts(WPARAM w2Param, LPARAM lParam)
{
	if (m_hfontNormal) {
		DeleteObject(m_hfontNormal);
		m_hfontNormal = NULL;
	}
	if (m_hfontHot) {
		DeleteObject(m_hfontHot);
		m_hfontHot = NULL;
	}

	LOGFONT lf;
	LONG style = GetStyle();
	if (GetLogFont(wParam ? ((HFONT)wParam) : ((HFONT)GetStockObject(DEFAULT_GUI_FONT)), &lf)) {
		lf.lfWeight       = (style & TLS_BOLD) ? FW_BOLD : FW_NORMAL;
		lf.lfUnderline    = (style & TLS_NORMUNDERLINE) ? TRUE : FALSE;
		m_hfontNormal = CreateFontIndirect(&lf);
		lf.lfWeight       = (style & TLS_BOLD) ? FW_BOLD : FW_NORMAL;
		lf.lfUnderline    = (style & TLS_HOTUNDERLINE) ? TRUE : FALSE;
		m_hfontHot    = CreateFontIndirect(&lf);

		CSize si;
		if (GetItemSize(&si,-1)) {
			SetWindowPos(NULL,0,0,si.cx, si.cy, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER|SWP_NOREDRAW);
			ResizeTTControl();
		}
	}
	return TRUE;
}

LRESULT CTaskLinkCtrl::OnTlsSetColors(WPARAM w2Param, LPARAM lParam)
{
	if (IsBadReadPtr((CONST VOID *)wParam, sizeof(COLORREF) * 5)) {		// return to original colors
		if (m_hbrBkgnd) {
			DeleteObject(m_hbrBkgnd);
			m_hbrBkgnd = NULL;
		}
		m_hbrBkgnd  = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
		m_clrNormal   = GetSysColor(COLOR_BTNTEXT);
		m_clrHot      = GetSysColor(COLOR_HOTLIGHT);
		m_clrPressed  = GetSysColor(COLOR_HOTLIGHT);
		m_clrDisabled = GetSysColor(COLOR_GRAYTEXT);
		m_bColorSet   = false;
		return TRUE;
	}

	if (m_hbrBkgnd) {
		DeleteObject(m_hbrBkgnd);
		m_hbrBkgnd = NULL;
	}

	m_bColorSet   = true;
	m_hbrBkgnd    = CreateSolidBrush( ((COLORREF*)wParam)[0] );
	m_clrNormal   = ((COLORREF*)wParam)[1];
	m_clrHot      = ((COLORREF*)wParam)[2];
	m_clrPressed  = ((COLORREF*)wParam)[3];
	m_clrDisabled = ((COLORREF*)wParam)[4];

	CSize si;
	if (GetItemSize(&si, -1)) {
		SetWindowPos(NULL, 0, 0, si.cx, si.cy, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOREDRAW);
		ResizeTTControl();
	}
	return TRUE;
}

LRESULT CTaskLinkCtrl::OnTlsSetImages(WPARAM w2Param, LPARAM lParam)
{
	if (IsBadReadPtr((CONST VOID *)lParam, sizeof(HIMAGELIST) * 3))
		return FALSE;

	m_iImageIndex = (int)wParam;
	m_himNormal   = ((HIMAGELIST*)lParam)[0];
	m_himHot      = ((HIMAGELIST*)lParam)[1];
	m_himDisabled = ((HIMAGELIST*)lParam)[2];

	CSize si;
	if (GetItemSize(&si, -1)) {
		SetWindowPos(NULL, 0, 0, si.cx, si.cy, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOREDRAW);
		ResizeTTControl();
	}
	return TRUE;
}

LRESULT CTaskLinkCtrl::OnTlsGetTTCtrl(WPARAM w2Param, LPARAM lParam)
{
	return (LPARAM)m_tip.m_hWnd;
}

LRESULT CTaskLinkCtrl::OnTlsSetCursor(WPARAM w2Param, LPARAM lParam)
{
	m_hCursor = (HCURSOR)wParam;
	if (!m_hCursor)
		m_hCursor = LoadCursor(NULL,�@IDC_HAND);
	return TRUE;
}

LRESULT CTaskLinkCtrl::OnTlsSetHorzSpace(WPARAM w2Param, LPARAM lParam)
{
	m_iHorzSpace = (int)wParam;
	CSize si;
	if (GetItemSize(&si, -1)) {
		SetWindowPos(NULL, 0, 0, si.cx, si.cy, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOREDRAW);
		ResizeTTControl();
	}
	return TRUE;
}

LRESULT CTaskLinkCtrl::OnTlsGetHorzSpace(WPARAM w2Param, LPARAM lParam)
{
	return (LRESULT)m_iHorzSpace;
}

void CTaskLinkCtrl::OnTlsActivate(WPARAM w2Param, LPARAM lParam)
{
	m_bActive = (wParam != FALSE);
	if (!m_bActive) {
		if (m_bEnabled) {
			m_iItemState        = ITEMSTATE_COLD;
			m_bLButtonDown  = false;
			m_ptMouse.x = -1;
			m_ptMouse.y = -1;
			Invalidate();
		}
	}

	if ((GetStyle() & TLS_TOOLTIP) && m_tip.IsWindow())
		m_tip.SendMessage(TTM_ACTIVATE, (WPARAM)((m_bEnabled && m_bActive) ? TRUE : FALSE), 0L);
}

void CTaskLinkCtrl::OnDraw(HDC hdc, CPoint ptOffset, BOOL bPaintBackground, int iCtrlWidth)
{
  TLSTRUCT      TLSSave;
  TCHAR         szText[256];
  RECT          rcText,rc;
  UINT          uDTStyle = 0;
  HBITMAP       hOldBitmap;

	if (iCtrlWidth != -1) {
		CSize si;
		memcpy(&TLSSave,pTLS,sizeof(TLSTRUCT));
		GetItemSize(&si, iCtrlWidth);
    	rc.left   = rc.top = 0;
		rc.right  = si.cx;
		rc.bottom = si.cy;
	}
	else
		GetClientRect(&rc);

	CDCHandle dc = hdc;
	CDC dcw;
	dcw.CreateCompatibleDC(dc);
	HBITMAP hbm = CreateCompatibleBitmap(dc, rc.right, rc.bottom);

	if (hbm) {
		HBITMAP hbmw = dcw.SelectBitmap(hbm);

		LONG style   = GetStyle();
		LONG exStyle = GetExStyle();
		if (exStyle & WS_EX_RIGHT) {
			style |= TLS_ICONRIGHT;
			if ((style & TLS_HALIGNMASK) == TLS_LEFT)
				style = (style & ~TLS_HALIGNMASK) | TLS_RIGHT;
			else if ((style & TLS_HALIGNMASK) == TLS_RIGHT)
				style = (style & ~TLS_HALIGNMASK) | TLS_LEFT;
		}

		if (bPaintBackground) {
			HBRUSH hbr = (HBRUSH)GetParent().SendMessage(WM_CTLCOLORSTATIC, (WPARAM)(HDC)dcw, (LPARAM)m_hWnd);
			if (((ULONG)(hbr)) > 1)
				dcw.FillRect(&rc, hbr);
		}
		else
			dcw.BitBlt(0, 0, rc.right, rc.bottom, dc, ptOffset.x, ptOffset.y, SRCCOPY);

		HFONT hfont;
		COLORREF clrText;
		switch (m_iItemState) {
		case ITEMSTATE_COLD:
			hfont  = dcw.SelectFont(m_hfontNormal);
			clrText = dcw.SetTextColor(m_clrNormal);
			break;
		case ITEMSTATE_PRESSED:
			hfont  = dcw.SelectFont(m_hfontHot);
			clrText = dcw.SetTextColor(m_clrPressed);
			break;
		case ITEMSTATE_HOT:
			hfont  = dcw.SelectFont(m_hfontHot);
			clrText = dcw.SetTextColor(m_clrHot);
			break;
		default:
			hfont  = dcw.SelectFont(m_hfontNormal);
			clrText = dcw.SetTextColor(m_clrDisabled);
			break;
		}
		int iOldBkMode = dcw.SetBkMode(TRANSPARENT);

		if (m_himNormal) {
			HIMAGELIST him;
			switch (m_iItemState) {
			case ITEMSTATE_COLD:	him = m_himNormal; break;
			case ITEMSTATE_PRESSED:
			case ITEMSTATE_HOT:		him = m_himHot; break;
			default:				him = m_himDisabled; break;
			}
			if (!him) him = m_himNormal;

			if (him)
				ImageList_Draw(him, m_iImageIndex, dcw, m_tls.rcIcon.left, m_tls.rcIcon.top, ILD_TRANSPARENT);
		}

		szText[0] = 0;
		GetWindowText(szText, 256);
		memcpy(&rcText, &m_tls.rcText, sizeof(RECT));

		if (style & TLS_FOCUSRECT) {
			if (m_bHasFocus)
				dcw.DrawFocusRect(&rcText);
			rcText.left++;
			rcText.right--;
			rcText.top++;
			rcText.bottom--;
		}

		if (style & TLS_WORDBREAK) {		// multiline, use current control width
			if (m_bMultiLine)
				uDTStyle = DT_EXTERNALLEADING|DT_WORDBREAK;
			else
				uDTStyle = DT_SINGLELINE|DT_NOCLIP;
		}
		else {
			if (style & TLS_END_ELLIPSIS) {	// singleline, use current control width
				uDTStyle = DT_END_ELLIPSIS | DT_SINGLELINE;
			}
			else {							// singleline, ignore current control width
				uDTStyle = DT_SINGLELINE | DT_NOCLIP;
			}
		}

		if ((style & TLS_HALIGNMASK) == TLS_LEFT)
			uDTStyle |= DT_LEFT;
		else if ((style & TLS_HALIGNMASK) == TLS_RIGHT)
			uDTStyle |= DT_RIGHT;
		else if ((style & TLS_HALIGNMASK) == TLS_CENTER)
			uDTStyle |= DT_CENTER;

		if ((style & TLS_VALIGNMASK) == TLS_TOP)
			uDTStyle |= DT_TOP;
		else if ((style & TLS_VALIGNMASK) == TLS_BOTTOM)
			uDTStyle |= DT_BOTTOM;
		else if ((style & TLS_VALIGNMASK) == TLS_MIDDLE)
			uDTStyle |= DT_VCENTER;

		if (exStyle & WS_EX_RTLREADING)
			uDTStyle |= DT_RTLREADING;

		dcw.DrawTextEx(szText, -1, &rcText, uDTStyle, NULL);

		dcw.SetBkMode(iOldBkMode);
		dcw.SetTextColor(clrText);
		dcw.SelectFont(hfont);

		dc.BitBlt(ptOffset.x, ptOffset.y, rc. right,rc.bottom, dcw, 0, 0, SRCCOPY);
		dcw.SelectBitmap(hbmw);
	}
	if (hbm)
		DeleteObject(hbm);
	if (iCtrlWidth != -1)
		memcpy(pTLS,&TLSSave,sizeof(TLSTRUCT));
}

void CTaskLinkCtrl::GetHitTestRect(RECT* prc)
{
	if (GetStyle() & TLS_NOAUTOSIZE)
		GetClientRect(prc);
	else {
		*prc = m_tls.rcText;
		if ((m_tls.rcIcon.Width() > 0) && (m_tls.rcIcon.Height() > 0)) {
			if (m_tls.rcIcon.left < prc->left)
				prc->left = m_tls.rcIcon.left;
			if (m_tls.rcIcon.right > prc->right)
				prc->right = m_tls.rcIcon.right;
			if (m_tls.rcIcon.top < prc->top)
				prc->top = m_tls.rcIcon.top;
			if (m_tls.rcIcon.bottom > prc->bottom)
				prc->bottom = m_tls.rcIcon.bottom;
		}
	}
}

void CTaskLinkCtrl::ResizeTTControl()
{
	TOOLINFO ti;
	if ((GetStyle() & TLS_TOOLTIP) && m_tip.IsWindow()) {
		memset(&ti, 0, sizeof(ti));
		ti.cbSize   = sizeof(TOOLINFO);
		ti.uFlags   = TTF_TRANSPARENT;
		ti.hwnd     = m_hWnd;
		ti.lpszText = LPSTR_TEXTCALLBACK;
		ti.uId      = 1;
		GetHitTestRect(&ti.rect);
		m_tip.SendMessage(TTM_DELTOOL, 0, (LPARAM)&ti);
		m_tip.SendMessage(TTM_ADDTOOL, 0, (LPARAM)&ti);
	}
}

void CTaskLinkCtrl::CheckNeedRepaint()
{
	int iState = ITEMSTATE_COLD;
	CRect rc;
	if (!m_bEnabled)
		iState = ITEMSTATE_DISABLED;
	else {
		GetHitTestRect(&rc);

		if (m_bKeyDown)
			iState = ITEMSTATE_PRESSED;
		else {
			if (m_ptMouse.PtInRect(&rc)) {
				if (m_bLButtonDown)
					iState = ITEMSTATE_PRESSED;
				else
					iState = ITEMSTATE_HOT;
			}
			else if (GetCapture() == m_hWnd)
				iState = ITEMSTATE_HOT;
		}
	}

	if (m_iItemState != iState) {
		m_iItemState = iState;
		Invalidate();
	}
}

// iRequiredWidth only used if !(lStyle & TLS_NOAUTOSIZE) !!!
BOOL CTaskLinkCtrl::GetItemSize(PSIZE pSI, int iRequiredWidth)
{
	int iTextWidthAvail, iTextHeightAvail, iTextHeight, iDelta;
	CSize si;

	LONG style   = GetStyle();
	LONG exStyle = GetExStyle();
	if (exStyle & WS_EX_RIGHT) {
		style |= TLS_ICONRIGHT;
		if ((style & TLS_HALIGNMASK) == TLS_LEFT)
			style = (style & ~TLS_HALIGNMASK) | TLS_RIGHT;
		else if ((style & TLS_HALIGNMASK) == TLS_RIGHT)
			style = (style & ~TLS_HALIGNMASK) | TLS_LEFT;
	}

	CRect rc;
	if (style & TLS_NOAUTOSIZE) {
		GetClientRect(&rc);
	}
	else {
		if (iRequiredWidth == -1)
			GetClientRect(&rc);
		else
			rc.right  = iRequiredWidth;
			rc.left   = rc.top = 0;
			rc.bottom = 32767;
	}
	CRect rcOrig = rc;

	CSize si(0, 0);
	if (m_himNormal) {
		if (ImageList_GetIconSize(m_himNormal, &si.cx, &si.cy))
			si.cx += m_iHorzSpace;
	}

	CClientDC dc(m_hWnd);
	HFONT hfont = dc.SelectFont(m_hfontHot);

	CString str;
	GetWindowText(str);

	CRect rct;
	UINT uDTStyle = 0L;
	if (style & TLS_WORDBREAK) {				// multiline, use current control width
		rct.left   = 0;
		rct.right  = rc.right - si.cx;
		if (style & TLS_FOCUSRECT)
			rct.right -= 2;
		rct.top    = 0;
		rct.bottom = 0;
		uDTStyle = DT_EXTERNALLEADING | DT_WORDBREAK;
	}
	else {
		if (style & TLS_END_ELLIPSIS) {			// singleline, use current control width
			rct.left   = 0;
			rct.right  = rc.right - si.cx;
			if (style & TLS_FOCUSRECT)
				rct.right -= 2;
			rct.top    = 0;
			rct.bottom = 0;
			uDTStyle = DT_END_ELLIPSIS | DT_SINGLELINE;
		}
		else {									// singleline, ignore current control width
			rct.left   = 0;
			rct.right  = 0;
			rct.top    = 0;
			rct.bottom = 0;
			uDTStyle = DT_SINGLELINE | DT_NOCLIP;
		}
	}

	uDTStyle |= DT_CALCRECT | DT_LEFT | DT_TOP;
	if (exStyle & WS_EX_RTLREADING)
		uDTStyle |= DT_RTLREADING;

	CSize sizeText(rct.right, dc.DrawTextEx(str, -1, &rct, uDTStyle, NULL));
	rct.left      = 0;
	rct.right     = 0;
	rct.top       = 0;
	rct.bottom    = 0;
	uDTStyle = DT_SINGLELINE | DT_NOCLIP | DT_CALCRECT | DT_LEFT | DT_TOP;
	if (exStyle & WS_EX_RTLREADING)
		uDTStyle |= DT_RTLREADING;

	CSize sizeSingleLine(rct.right, dc.DrawTextEx(str, -1, &rct, uDTStyle, NULL));

	m_bMultiLine = (sizeText.cy >= (sizeSingleLine.cy << 1)) ? true : false;

	if (si.cy) {
		si.cx -= m_iHorzSpace;
	}

	dc.SelectFont(hfont);

	if (style & TLS_FOCUSRECT) {
		sizeText.cx       += 2;
		sizeText.cy       += 2;
		sizeSingleLine.cx += 2;
		sizeSingleLine.cy += 2;
	}

	// place icon either on the left or on the right side

	if (si.cy) {		// we do have an icon
		if (style & TLS_ICONRIGHT) {
			m_tls.rcIcon.right   = rc.right;
			m_tls.rcIcon.left    = m_tls.rcIcon.right-si.cx;
			m_tls.rcIcon.top     = 0;
			m_tls.rcIcon.bottom  = si.cy;
			rc.right      -= si.cx+m_iHorzSpace;
		}
		else {
			m_tls.rcIcon.left    = rc.left;
			m_tls.rcIcon.right   = m_tls.rcIcon.left+si.cx;
			m_tls.rcIcon.top     = 0;
			m_tls.rcIcon.bottom  = si.cy;
			rc.left       += si.cx+m_iHorzSpace;
		}
	}
	else
		memset(&m_tls.rcIcon, 0, sizeof(RECT));

	iTextWidthAvail = rc.Width();
	if (iTextWidthAvail <= 0)
		memset(&m_tls.rcText, 0, sizeof(RECT));
	else {
		if (iTextWidthAvail < siText.cx)
			siText.cx = iTextWidthAvail;

		switch (style & TLS_HALIGNMASK) {
		case TLS_LEFT:
			m_tls.rcText.left  = rc.left;
			m_tls.rcText.right = m_tls.rcText.left + siText.cx;
			break;
		case TLS_RIGHT:
			m_tls.rcText.right = rc.right;
			m_tls.rcText.left  = m_tls.rcText.right - siText.cx;
			break;
		default:
			m_tls.rcText.left  = rc.left + (iTextWidthAvail - siText.cx) / 2;
			m_tls.rcText.right = m_tls.rcText.left + siText.cx;
			break;
		}

		if (m_bMultiLine)
			iTextHeight = sizeText.cy;
		else
			iTextHeight = sizeSingleLine.cy;

		switch (style & TLS_VALIGNMASK) {
		case TLS_TOP:
			m_tls.rcText.top    = rc.top;
			m_tls.rcText.bottom = m_tls.rcText.top + iTextHeight;
			break;
		case TLS_BOTTOM:
			if (style & TLS_NOAUTOSIZE) {
				m_tls.rcText.bottom = rc.bottom;
				m_tls.rcText.top    = m_tls.rcText.bottom - iTextHeight;
				if (m_tls.rcText.top < 0)
					m_tls.rcText.top = 0;
			}
			else {
				m_tls.rcText.top    = rc.top;
				m_tls.rcText.bottom = m_tls.rcText.top + iTextHeight;
			}
			break;
		default:
			if (style & TLS_NOAUTOSIZE) {
				iTextHeightAvail    = rc.Height();
				if (iTextHeight > iTextHeightAvail)
					iTextHeight = iTextHeightAvail;
				m_tls.rcText.top    = rc.top + (iTextHeightAvail - iTextHeight) / 2;
				m_tls.rcText.bottom = m_tls.rcText.top + iTextHeight;
			}
			else {
				m_tls.rcText.top    = rc.top;
				m_tls.rcText.bottom = m_tls.rcText.top + iTextHeight;
			}
			break;
		}
	}

	if (si.cy) { // WE DO HAVE AN ICON ////////////////////////////////////////////////////////////////
		m_tls.rcIcon.top    += m_tls.rcText.top;
		m_tls.rcIcon.bottom += m_tls.rcText.top;

		if (m_bMultiLine) {
			if (si.cy > sizeText.cy) {		// icon bigger than multiline text
				iDelta = (si.cy - sizeText.cy) >> 1;
				m_tls.rcText.top    += iDelta;
				m_tls.rcText.bottom += iDelta;
			}
			else if (si.cy < sizeText.cy) {
				if (si.cy < sizeSingleLine.cy) {		// icon smaller than single text
					iDelta = (sizeSingleLine.cy - si.cy) >> 1;
					m_tls.rcIcon.top    += iDelta;
					m_tls.rcIcon.bottom += iDelta;
				}
				else if (si.cy > sizeSingleLine.cy) {	// icon bigger than single text (smaller than multiline)
					iDelta = (sizeText.cy - si.cy) >> 1;
					m_tls.rcIcon.top    += iDelta;
					m_tls.rcIcon.bottom += iDelta;
				}
			}
		}
		else {
			if (si.cy < sizeSingleLine.cy) {		// icon smaller than text
				iDelta = (sizeSingleLine.cy - si.cy) >> 1;
				m_tls.rcIcon.top    += iDelta;
				m_tls.rcIcon.bottom += iDelta;
			}
			else if (si.cy > sizeSingleLine.cy) {	// icon bigger than text
				iDelta = (si.cy - sizeSingleLine.cy) >> 1;
				m_tls.rcText.top    += iDelta;
				m_tls.rcText.bottom += iDelta;
			}
		}
	}

	int iMinY = (m_tls.rcText.top < m_tls.rcIcon.top) ? m_tls.rcText.top : m_tls.rcIcon.top;
	if (iMinY < 0) {
		m_tls.rcText.top    -= iMinY;
		m_tls.rcText.bottom -= iMinY;
		m_tls.rcIcon.top    -= iMinY;
		m_tls.rcIcon.bottom -= iMinY;
	}

	int iMaxY = (m_tls.rcText.bottom > m_tls.rcIcon.bottom) ? m_tls.rcText.bottom : m_tls.rcIcon.bottom;
	pSI->cx = rcOrig.right;
	if (style & TLS_NOAUTOSIZE) {
		pSI->cy = rc.bottom;
		if (m_tls.rcText.bottom > rc.bottom) m_tls.rcText.bottom = rc.bottom;
		if (m_tls.rcIcon.bottom > rc.bottom) m_tls.rcIcon.bottom = rc.bottom;
	}
	else
		pSI->cy = iMaxY;

	ResizeTTControl();
	return TRUE;
}

////////////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK TaskLinkWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CTaskLinkCtrl* pWnd = new CTaskLinkCtrl();
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}


