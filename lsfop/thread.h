////////////////////////////////////////////////////////////////////////////////
// thread.h

#pragma once
#ifndef __XTHREAD_H__
#define __XTHREAD_H__

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

#define THREAD_STOP_TIMEOUT			6000		// 6sec
#define THREAD_EXIT_TIMEOUT			1000		// 1sec

template <class T>
class CGrepThread
{
public:
	HWND m_hWnd;						// attend to window
	HANDLE m_stop;						// manual reset event
	HANDLE m_reset;						// manual reset event
	CAccessLock m_cs;					// 
	DWORD m_magic;
	HANDLE m_thread;
	UINT m_msgcode;
	int m_priority;
	LPARAM m_userdata;

	CGrepThread() :	m_thread(NULL),
					m_hWnd(NULL),
					m_stop(NULL),
					m_reset(NULL),
					m_magic(0),
					m_priority(THREAD_PRIORITY_BELOW_NORMAL),
					m_userdata(0),
					m_msgcode(WM_NULL)
	{
	}

	~CGrepThread()
	{
		Stop(THREAD_EXIT_TIMEOUT);
	}

	inline void PostItemNotify(INT_PTR index) { ::PostMessage(m_hWnd, m_msgcode, (WPARAM)index, (LPARAM)m_magic); }

	BOOL Start(HWND hwnd, DWORD magic, LPARAM userdata)
	{
		Stop();

#ifdef __USE_CREATETHREAD__
		DWORD id;
		m_thread = CreateThread(NULL, 0, _ThreadProcEntry, this, CREATE_SUSPENDED, &id);
#else
		unsigned id;
		m_thread = (HANDLE)_beginthreadex(NULL, 0, _ThreadProcEntry, this, CREATE_SUSPENDED, &id);
#endif
		if (m_thread) {
			::SetThreadPriority(m_thread, m_priority);

			m_cs.Enter();
			m_stop = CreateEvent(NULL, TRUE, FALSE, NULL);	// イベントは手動リセット！
			m_reset = CreateEvent(NULL, TRUE, FALSE, NULL);	// イベントは手動リセット！
			m_hWnd = hwnd;
			m_userdata = userdata;
			m_magic = magic;
			m_cs.Leave();

			ResumeThread(m_thread);
			return TRUE;
		}
		return FALSE;
	}

	void Stop(DWORD dwTimeout = THREAD_STOP_TIMEOUT)
	{
		if (m_thread) {
			DWORD code;
			GetExitCodeThread(m_thread, &code);
			if (code == STILL_ACTIVE) {
				// スレッドに終了通知し、終了させる
				SetEvent(m_stop);
				if (WaitForSingleObject(m_thread, dwTimeout) != WAIT_OBJECT_0) {
					// タイムアウトしたら強制終了させる
					TerminateThread(m_thread, (DWORD)-1);
				}
			}
			CloseHandle(m_reset);
			CloseHandle(m_stop);
			CloseHandle(m_thread);

			m_thread = NULL;
			m_hWnd = NULL;
			m_userdata = 0;
			m_magic = 0;
			m_stop = NULL;
			m_reset = NULL;
		}
	}

	bool isRunning()
	{
		DWORD code;
		return ((m_thread != NULL) && GetExitCodeThread(m_thread, &code) && (code == STILL_ACTIVE));
	}

	inline bool isIdle() { return !isRunning(); }
	inline bool isCanceled() { return (WaitForSingleObject(m_stop, 0) != WAIT_TIMEOUT); }

	unsigned ThreadProc()
	{
		HANDLE handle[2];
		handle[0] = m_stop;
		handle[1] = m_reset;

		T* pT = static_cast<T*>(this);
		pT->Reset();

		bool bContinue = true;
		while (bContinue) {
			DWORD events = WaitForMultipleObjects(ARRAYSIZE(handle), handle, FALSE, 0);
			if (events == WAIT_OBJECT_0)
				break;
			else if (events == WAIT_OBJECT_1) {
				pT->Reset();
			}
			else if (events == WAIT_TIMEOUT) {
				m_cs.Enter();
				pT->Grep();
				m_cs.Leave();
			}
		}

		return 0;
	}

#ifdef __USE_CREATETHREAD__
	static DWORD WINAPI _ThreadProcEntry(void* param)
#else
	static unsigned __stdcall _ThreadProcEntry(void* param)
#endif
	{
		CGrepThread* pThis = (CGrepThread*)param;
		CoInitialize(NULL);
		unsigned ret = pThis->ThreadProc();
		CoUninitialize();

#ifdef __USE_CREATETHREAD__
		ExitThread((DWORD)ret);
#else
		_endthreadex(ret);
#endif
		return 0;
	}

	virtual void Reset() { }
	virtual void Grep() { }
};

#endif				// __XTHREAD_H__
