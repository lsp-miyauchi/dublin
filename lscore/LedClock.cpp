///////////////////////////////////////////////////////
// ledclock.cpp

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "gdiplus_sup.h"
#include "fnc.h"

typedef unsigned (WINAPI *PBEGINTHREADEX_START_ROUTINE)(void* pThreadParameter);
typedef PBEGINTHREADEX_START_ROUTINE LPBEGINTHREADEX_START_ROUTINE;


///////////////////////////////////////////////////////
// LEDCLOCK control implementation

#define SGX						7
#define SGY						12
#define SGLX					14
#define SGLY					17

#define LCS_STARTED				0x80000000

// forword definition
class CLedClockCtrl;

class CLedClock
{
public:
	CLedClock();
	~CLedClock();

	BOOL IsStarted() { return (m_dwStat & LCS_STARTED); }
	void Lock() { EnterCriticalSection(&m_crit); }
	void Unlock() { LeaveCriticalSection(&m_crit); }
	BOOL InitThread(LPBEGINTHREADEX_START_ROUTINE pfn);
	void TermThread()
	{
		if (IsThreadStillActive()) {
			SetEvent(m_kill);
			// スレッド終了を待つ
			WaitForSingleObject(m_thread, INFINITE);
		}
		CloseHandle(m_start);
		CloseHandle(m_stop);
		CloseHandle(m_kill);
		CloseHandle(m_thread);
	}
	bool IsThreadStillActive()
	{
		if (m_thread) {
			DWORD code;
			return (GetExitCodeThread(m_thread, &code) && code == STILL_ACTIVE);
		}
		return false;
	}
	void Suspend()
	{
		if (IsThreadStillActive())
			SetEvent(m_stop);
	}
	void Resume()
	{
		if (IsThreadStillActive())
			SetEvent(m_start);
	}
	HIMAGELIST m_hImage;
	DWORD m_dwStat;
	int m_nMode;
	int m_nLimValue;
	int m_nIniValue;
	CBitmap m_bmLed;
	CBitmap m_bmBkgnd;
	CLedClockCtrl* m_pWnd;

	HANDLE m_thread;
	HANDLE m_start;
	HANDLE m_stop;
	HANDLE m_kill;

	HDC m_hdc;
	CRITICAL_SECTION m_crit;
	int m_cx;
	int m_cy;
};

CLedClock::CLedClock()
{
	gdiplus_using gu;

	m_cx = SGX;
	m_cy = SGY;
	m_hImage = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_DIGITALNUMBER), m_cx, RGB(255, 0, 255));
	m_dwStat = 0;
	m_nLimValue = 0;
	m_nIniValue = 0;
	m_pWnd = NULL;

	m_thread = NULL;
	m_start = NULL;
	m_stop = NULL;
	m_kill = NULL;

	m_hdc = NULL;

	CClientDC dc(nullptr);
	m_bmLed.CreateCompatibleBitmap(dc, m_cx * 8, m_cy);

	InitializeCriticalSection(&m_crit);
}

CLedClock::~CLedClock()
{
	if (m_hImage)	ImageList_Destroy(m_hImage);

	DeleteCriticalSection(&m_crit);
}

BOOL CLedClock::InitThread(LPBEGINTHREADEX_START_ROUTINE pfnThread)
{
	unsigned id;
	m_thread = (HANDLE)_beginthreadex(NULL, 0, pfnThread, this, CREATE_SUSPENDED, &id);
	if (m_thread) {
		SetThreadPriority(m_thread, THREAD_PRIORITY_BELOW_NORMAL);
		m_start = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_stop = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_kill = CreateEvent(NULL, FALSE, FALSE, NULL);

		ResumeThread(m_thread);
		return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////


class CLedClockCtrl : public CWindowImpl<CLedClockCtrl>
{
private:
	CLedClock* m_pLed;

public:
	CLedClockCtrl();
	~CLedClockCtrl() { }

	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

BEGIN_MSG_MAP(CLedClockCtrl)
	ON_WM_CREATE()
	ON_MESSAGE(WM_GETDLGCODE, OnGetDlgCode)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(LCM_START, OnStart)
	ON_MESSAGE(LCM_STOP, OnStop)
	ON_MESSAGE(LCM_RESET, OnReset)
END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT pcs);
	void OnSize(UINT flag, int cx, int cy);
	void OnDestroy();
	LRESULT OnGetDlgCode(WPARAM wParam, LPARAM lParam);
	void OnPaint();
	BOOL OnEraseBkgnd(HDC hdc);
	LRESULT OnStart(WPARAM wParam, LPARAM lParam);
	LRESULT OnReset(WPARAM wParam, LPARAM lParam);
	LRESULT OnStop(WPARAM wParam, LPARAM lParam);

	static void Draw(CLedClock* led);
	static unsigned WINAPI ClockProc(CLedClock* led);
};

CLedClockCtrl::CLedClockCtrl()
{
	m_pLed = NULL;
}

int CLedClockCtrl::OnCreate(LPCREATESTRUCT pcs)
{
	DWORD style = GetStyle();

	m_pLed = new CLedClock();
	m_pLed->m_nMode = style & LCS_MODEMASK;
	m_pLed->m_pWnd = this;
	if (!m_pLed->InitThread((LPBEGINTHREADEX_START_ROUTINE)ClockProc))
		return -1;

	if (style & LCS_AUTOSTART)
		OnStart(0, 0);
	return 0;
}

void CLedClockCtrl::OnSize(UINT flag, int cx, int cy)
{
	if (m_pLed && flag != SIZE_MINIMIZED) {
		if (!m_pLed->m_bmBkgnd.IsNull())
			m_pLed->m_bmBkgnd.DeleteObject();
	}
}

void CLedClockCtrl::OnDestroy()
{
	if (m_pLed) {
		m_pLed->TermThread();
		delete m_pLed;
		m_pLed = NULL;
	}
}

_inline LRESULT CLedClockCtrl::OnGetDlgCode(WPARAM wParam, LPARAM lParam)
	{ return DLGC_STATIC; }

void CLedClockCtrl::OnPaint()
{
	m_pLed->Lock();

	CPaintDC dc(m_hWnd);
	m_pLed->m_hdc = dc;
	Draw(m_pLed);

	m_pLed->Unlock();
}

BOOL CLedClockCtrl::OnEraseBkgnd(HDC hdc)
{
	CRect rc;
	GetClientRect(&rc);

	CDCHandle dc = hdc;
	m_pLed->Lock();

	// 背景ビットマップ作成
	CDC dcw; dcw.CreateCompatibleDC(dc);
	HBITMAP hbmw;
	if (m_pLed->m_bmBkgnd.IsNull()) {
		m_pLed->m_bmBkgnd.CreateCompatibleBitmap(dc, rc.Width(), rc.Height());
		hbmw = dcw.SelectBitmap(m_pLed->m_bmBkgnd);
		HBRUSH hbr = (HBRUSH)GetParent().SendMessage(WM_CTLCOLORSTATIC, (WPARAM)(HDC)dcw, (LPARAM)m_hWnd);
		if (hbr)
			dcw.FillRect(&rc, hbr);
		else
			dcw.FillRect(&rc, COLOR_3DFACE);
		dcw.SelectBitmap(hbmw);
	}

	hbmw = dcw.SelectBitmap(m_pLed->m_bmBkgnd);
	dc.BitBlt(0, 0, rc.Width(), rc.Height(), dcw, 0, 0, SRCCOPY);
	dcw.SelectBitmap(hbmw);

	m_pLed->Unlock();

	return TRUE;
}

LRESULT CLedClockCtrl::OnStart(WPARAM wParam, LPARAM lParam)
{
	if (!m_pLed->IsStarted())
		OnReset(wParam, lParam);
	return 0;
}

LRESULT CLedClockCtrl::OnReset(WPARAM wParam, LPARAM lParam)
{
	if (m_pLed->IsStarted())
		OnStop(0, 0);

	DWORD dwCurTime = GetTickCount() / 1000;
	if (m_pLed->m_nMode == WLC_TIMER) {
		m_pLed->m_nIniValue = dwCurTime;
		m_pLed->m_nLimValue = dwCurTime + (int)(lParam / 1000);
	}
	else if (m_pLed->m_nMode == WLC_COUNTDOWN) {
		m_pLed->m_nLimValue = dwCurTime + (int)(lParam / 1000);
		m_pLed->m_nIniValue = m_pLed->m_nLimValue;
	}
	m_pLed->Resume();
	return 0;
}

LRESULT CLedClockCtrl::OnStop(WPARAM wParam, LPARAM lParam)
{
	if (m_pLed->IsStarted())
		m_pLed->Suspend();
	return 0;
}

void CLedClockCtrl::Draw(CLedClock* led)
{
	if (!led->IsStarted()) return;

	CDCHandle dc = led->m_hdc;

	DWORD dwCurTime = GetTickCount() / 1000;

	int md;
	SYSTEMTIME st;
	TCHAR buf[MAX_PATH];
	if (led->m_nMode == WLC_TIMER) {
		st.wHour = (WORD)((dwCurTime - led->m_nIniValue) / 3600);
		md = (dwCurTime - led->m_nIniValue) % 3600;
		st.wMinute = (WORD)(md / 60);
		st.wSecond = (WORD)(md % 60);
	}
	else if (led->m_nMode == WLC_COUNTDOWN) {
	}
	else
		GetLocalTime(&st);
	wsprintf(buf, _T("%02d:%02d:%02d"), st.wHour, st.wMinute, st.wSecond);

	CDC dcw; dcw.CreateCompatibleDC(dc);
	HBITMAP hbmw = dcw.SelectBitmap(led->m_bmLed);
	CRect rcw(0, 0, led->m_cx * 8, led->m_cy);
	if (!led->m_bmBkgnd.IsNull()) {
		CDC dcb; dcb.CreateCompatibleDC(dc);
		HBITMAP hbmb = dcb.SelectBitmap(led->m_bmBkgnd);
		dcw.BitBlt(0, 0, rcw.Width(), rcw.Height(), dcb, 0, 0, SRCCOPY);
		dcb.SelectBitmap(hbmb);
	}
	else
		dcw.FillRect(&rcw, COLOR_3DFACE);

	rcw.right = rcw.left + led->m_cx;
	LPTSTR p = buf;
	int nIndex;
	while (*p) {
		if (IsDigit(*p))
			nIndex = *p - '0';
		else if (IsAlpha(*p))
			nIndex = ToUpper(*p) - 'A' + 10;
		else if (*p == ':')
			nIndex = 16;
		else if (*p == '/')
			nIndex = 17;
		else
			nIndex = 18;

		ImageList_Draw(led->m_hImage, nIndex, dcw, rcw.left, rcw.top, ILD_TRANSPARENT);
		p++;
		rcw.OffsetRect(led->m_cx, 0);
	}

	rcw.SetRect(0, 0, led->m_cx * 8, led->m_cy);
	CRect rc;
	led->m_pWnd->GetClientRect(&rc);
	int xLeft = rc.left + (rc.Width() - rcw.Width()) / 2;
	int yTop = rc.top + (rc.Height() - rcw.Height()) / 2;

	dc.BitBlt(xLeft, yTop, rcw.Width(), rcw.Height(), dcw, 0, 0, SRCCOPY);
	dcw.SelectBitmap(hbmw);
}

unsigned WINAPI CLedClockCtrl::ClockProc(CLedClock* led)
{
	HANDLE handle[3];
	handle[0] = led->m_kill;
	handle[1] = led->m_start;
	handle[2] = led->m_stop;

	DWORD timeout = INFINITE;
	while (1) {
		DWORD ret = WaitForMultipleObjects(3, handle, FALSE, timeout);
		if (ret == WAIT_OBJECT_0) {
			led->m_dwStat &= ~LCS_STARTED;
			break;
		}
		else if (ret == WAIT_OBJECT_0 + 1 ||
				 ret == WAIT_TIMEOUT) {
			timeout = 1000;
			led->m_dwStat |= LCS_STARTED;
		}
		else if (ret == WAIT_OBJECT_0 + 2) {
			timeout = INFINITE;
			led->m_dwStat &= ~LCS_STARTED;
		}
		led->Lock();
		if (led->m_pWnd->IsWindow()) {
			CClientDC dc(led->m_pWnd->m_hWnd);
			led->m_hdc = dc;
			led->m_pWnd->Draw(led);
		}
		led->Unlock();
	}
	_endthreadex(0);
	return 0;
}

////////////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK LedClockWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CLedClockCtrl* pWnd = new CLedClockCtrl;
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}
