///////////////////////////////////////////////////////
// AniImage.cpp

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"

static unsigned WINAPI AnimateProc(void* data);

///////////////////////////////////////////////////////
// CAnimateImageCtrl implementation

class CAniImage
{
public:
	CAniImage();
	~CAniImage();

	BOOL Init(HWND hwnd);
	void Term();
	void SetImage(HIMAGELIST him, int interval);
	void SetRect(LPRECT prc);
	void Lock() { EnterCriticalSection(&m_crit); }
	void Unlock() { LeaveCriticalSection(&m_crit); }
	DWORD Suspend() { return SuspendThread(m_thread); }
	DWORD Resume() { return ResumeThread(m_thread); }
	void Sleep() { ::Sleep(m_interval); }
	void Draw(HDC hdc);
	void Start();
	void Stop();
	void SetBkColor(COLORREF clrBkgnd);

	CRITICAL_SECTION m_crit;
	HWND m_hwnd;
	CRect m_rect;
	HBRUSH m_brush;
	COLORREF m_clrBkgnd;
	HIMAGELIST m_him;						// イメージリスト
	int m_cx;								// 各イメージの幅
	int m_cy;								// 各イメージの高さ
	int m_numcell;							// イメージリストに含まれるイメージ数
	int m_index;							// 現在のイメージ
	int m_interval;							// 表示間隔（ミリ秒）
	HBITMAP m_hbm;
	HANDLE m_thread;
	bool m_killwaiting:1;
	bool m_running:1;
};

CAniImage::CAniImage()
{
	m_hwnd = NULL;
	m_thread = NULL;
	m_brush = (HBRUSH)GetStockObject(GRAY_BRUSH);
	m_clrBkgnd = INVALID_COLOR;

	m_him = NULL;
	m_cx = 0;
	m_cy = 0;
	m_numcell = 0;
	m_index = 0;
	m_interval = 0;
	m_hbm = NULL;
	m_running = false;
	m_killwaiting = false;

	InitializeCriticalSection(&m_crit);
}

CAniImage::~CAniImage()
{
	DeleteCriticalSection(&m_crit);
	if (m_hbm) DeleteObject(m_hbm);
	if (m_clrBkgnd != INVALID_COLOR) DeleteObject(m_brush);
}

BOOL CAniImage::Init(HWND hwnd)
{
	m_hwnd = hwnd;
#ifdef _WIN64
	m_brush = (HBRUSH)GetClassLongPtr(m_hwnd, GCLP_HBRBACKGROUND);
#else
	m_brush = (HBRUSH)UIntToPtr(GetClassLongPtr(m_hwnd, GCLP_HBRBACKGROUND));
#endif
	unsigned id;
	m_thread = (HANDLE)_beginthreadex(NULL, 0, AnimateProc, this, CREATE_SUSPENDED, &id);
	if (!m_thread) return FALSE;

	SetThreadPriority(m_thread, THREAD_PRIORITY_BELOW_NORMAL);
	return TRUE;
}

void CAniImage::Term()
{
	if (m_thread) {
		if (m_running)
			m_killwaiting = true;
		else
			TerminateThread(m_thread, 0);
		CloseHandle(m_thread);
		m_thread = NULL;
	}
}

void CAniImage::SetImage(HIMAGELIST him, int interval)
{
	if (m_hbm) DeleteObject(m_hbm);

	m_hbm = nullptr;
	m_him = 0;
	m_numcell = 0;
	m_cx = 0;
	m_cy = 0;
	m_interval = 0;
	m_index = 0;
	if (him) {
		m_him = him;
		m_numcell = ImageList_GetImageCount(m_him);
		ImageList_GetIconSize(m_him, &m_cx, &m_cy);
		m_interval = interval;

		CClientDC dc(nullptr);
		m_hbm = CreateCompatibleBitmap(dc, m_cx, m_cy);
	}
}

void CAniImage::SetRect(LPRECT prc)
{
	m_rect = *prc;
}

void CAniImage::SetBkColor(COLORREF clrBkgnd)
{
	if (m_clrBkgnd != INVALID_COLOR) DeleteObject(m_brush);

	Lock();

	m_clrBkgnd = clrBkgnd;
	m_brush = CreateSolidBrush(clrBkgnd);

	Unlock();
}

void CAniImage::Draw(HDC hdc)
{
	if (m_him) {
		Lock();

		HDC hdcw = CreateCompatibleDC(hdc);
		HBITMAP hbm = (HBITMAP)SelectObject(hdcw, m_hbm);

		CRect rc(0, 0, m_cx, m_cy);
		FillRect(hdcw, &rc, m_brush);

		int index = 0;
		if (m_running) {
			index = ++m_index;
			m_index %= (m_numcell - 1);
		}

		ImageList_Draw(m_him, index, hdcw, 0, 0, ILD_TRANSPARENT);
		int x = m_rect.left + (m_rect.Width() - rc.Width()) / 2;
		int y = m_rect.top + (m_rect.Height() - rc.Height()) / 2;
		BitBlt(hdc, x, y, rc.Width(), rc.Height(), hdcw, 0, 0, SRCCOPY);

		SelectObject(hdcw, hbm);
		DeleteDC(hdcw);

		Unlock();
	}
}

void CAniImage::Start()
{
	if (!m_running && m_him) {
		m_running = TRUE;
		Resume();
	}
}

void CAniImage::Stop()
{
	if (m_running) {
		m_running = FALSE;
		Suspend();
	}
}

///////////////////////////////////////////////////////
//

static unsigned WINAPI AnimateProc(void* data)
{
	CAniImage* pai = (CAniImage*)data;
	for (;;) {
		if (pai->m_killwaiting) break;

		if (pai->m_hbm) {
			CClientDC dc(pai->m_hwnd);
			pai->Draw(dc);
		}
		pai->Sleep();
	}
	_endthreadex(0);
	return 0;
}

////////////////////////////////////////////////////////////////////////
//

class CAnimateImageCtrl : public CWindowImpl<CAnimateImageCtrl>
{
private:
	CAniImage m_ai;

public:
	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

BEGIN_MSG_MAP(CAnimateImageCtrl)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_GETDLGCODE()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_MESSAGE_VOID(AIM_START, OnStart)
	ON_MESSAGE_VOID(AIM_STOP, OnStop)
	ON_MESSAGE_VOID(AIM_SETANIMATION, OnSetImage)
	ON_MESSAGE_VOID(AIM_SETBKCOLOR, OnSetBkColor)
END_MSG_MAP()

	int OnCreate(LPCREATESTRUCT pcs) { return m_ai.Init(m_hWnd) ? 0 : -1; }
	UINT OnGetDlgCode() { return DLGC_STATIC; }
	void OnSize(UINT nType, int cx, int cy)
	{
		CRect rc(0, 0, cx, cy);
		m_ai.SetRect(&rc);
	}
	void OnPaint()
	{
		CPaintDC dc(m_hWnd);
		m_ai.Draw(dc);
	}
	void OnDestroy() { m_ai.Term(); }
	void OnStart(WPARAM wParam, LPARAM lParam) { m_ai.Start(); }
	void OnStop(WPARAM wParam, LPARAM lParam) { m_ai.Stop(); }
	void OnSetImage(WPARAM wParam, LPARAM lParam) { m_ai.SetImage((HIMAGELIST)lParam, (int)wParam); }
	void OnSetBkColor(WPARAM wParam, LPARAM lParam) { m_ai.SetBkColor((COLORREF)lParam); }
};

////////////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK AniImageWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CAnimateImageCtrl* pWnd = new CAnimateImageCtrl;
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}
