////////////////////////////////////////////////////////////////////////////////
// loaderthread.h

#pragma once


////////////////////////////////////////////////////////////////////////
//

class CAccessLock
{
public:
	CRITICAL_SECTION m_crisec;
	CAccessLock() { InitializeCriticalSectionAndSpinCount(&m_crisec, 1024 * 4); }
	~CAccessLock() { DeleteCriticalSection(&m_crisec); }
	__inline void Enter() { EnterCriticalSection(&m_crisec); }
	__inline BOOL TryEnter() { TryEnterCriticalSection(&m_crisec); }
	__inline void Leave() { LeaveCriticalSection(&m_crisec); }
};

////////////////////////////////////////////////////////////////////////
//

#define THREAD_EXIT_TIMEOUT			6000		// 6sec

typedef unsigned (WINAPI *PBEGINTHREADEX_START_ROUTINE)(void* pThreadParameter);
typedef PBEGINTHREADEX_START_ROUTINE LPBEGINTHREADEX_START_ROUTINE;


class CLoaderThread
{
public:
	HWND m_hWnd;						// attend to window
	DWORD m_magic;
	HANDLE m_thread;
	UINT m_msgcode;
	CString m_fname;
	CAccessLock m_al;

	CLoaderThread()
		: m_thread(NULL)
		, m_hWnd(NULL)
		, m_magic(0)
		, m_msgcode(WM_NULL)
	{
	}

	~CLoaderThread()
	{
		Stop();
	}

	void Initialize(HWND hwnd, UINT msgcode)
	{
		m_hWnd = hwnd;
		m_msgcode = msgcode;
	}

	void PostItemNotify(HDIB hdib)
	{
		if (::IsWindow(m_hWnd))
			::PostMessage(m_hWnd, m_msgcode, (WPARAM)m_magic, (LPARAM)hdib);
		else
			GlobalFree(hdib);
	}

	BOOL Start(LPCTSTR pszFileName, DWORD magic)
	{
		Stop();

		ATLASSERT(psf != NULL);
		ATLASSERT(pArray != NULL);

		unsigned id;
		m_thread = (HANDLE)_beginthreadex(NULL, 0, (LPBEGINTHREADEX_START_ROUTINE)_ThreadProcEntry, this, CREATE_SUSPENDED, &id);
		if (m_thread) {
			SetThreadPriority(m_thread, THREAD_PRIORITY_BELOW_NORMAL);

			m_al.Enter();
			m_fname = pszFileName;
			m_magic = magic;
			m_al.Leave();

			ResumeThread(m_thread);
			return TRUE;
		}
		return FALSE;
	}

	void Stop()
	{
		if (m_thread) {
			DWORD code;
			GetExitCodeThread(m_thread, &code);
			if (code == STILL_ACTIVE) {
				if (WaitForSingleObject(m_thread, THREAD_EXIT_TIMEOUT) != WAIT_OBJECT_0) {
					// タイムアウトしたら強制終了させる
					TerminateThread(m_thread, (DWORD)-1);
				}
			}
			CloseHandle(m_thread);

			m_al.Enter();
			m_thread = NULL;
			m_fname.Empty();
			m_magic = 0;
			m_al.Leave();
		}
	}

	bool IsRunning()
	{
		DWORD code;
		return ((m_thread != NULL) && GetExitCodeThread(m_thread, &code) && (code == STILL_ACTIVE));
	}

	inline bool IsIdle() { return !IsRunning(); }

	unsigned ThreadProc()
	{
		m_al.Enter();
		HDIB hdib = imgLoadImage(m_fname);
		PostItemNotify(hdib);
		m_al.Leave();
		return 0;
	}

	static unsigned WINAPI _ThreadProcEntry(CLoaderThread* pThis)
	{
//@		CoInitialize(NULL);
		unsigned ret = pThis->ThreadProc();
//@		CoUninitialize();

		_endthreadex(ret);
		return 0;
	}
};

