////////////////////////////////////////////////////////////////////////////////
// threadbase.h

#pragma once
#ifndef __THREADBASE_H__
#define __THREADBASE_H__


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

template <class T>
class CThreadBaseImpl
{
public:
	HWND m_hWnd;						// attend to window
	DWORD m_magic;
	HANDLE m_thread;
	HANDLE m_stop;						// manual reset event
	UINT m_msgcode;
	CAccessLock m_al;

	CThreadBaseImpl()
		: m_thread(NULL)
		, m_stop(NULL)
		, m_hWnd(NULL)
		, m_magic(0)
		, m_msgcode(WM_NULL)
	{
	}

	~CThreadBaseImpl()
	{
		Stop();
	}

	void Initialize(HWND hwnd, UINT msgcode)
	{
		m_hWnd = hwnd;
		m_msgcode = msgcode;
	}

	bool Start(DWORD magic, LPARAM param)
	{
		Stop();

		bool ok = false;
		unsigned id;
		m_thread = (HANDLE)_beginthreadex(NULL, 0, (LPBEGINTHREADEX_START_ROUTINE)_ThreadProcEntry, this, CREATE_SUSPENDED, &id);
		if (m_thread) {
			SetThreadPriority(m_thread, THREAD_PRIORITY_BELOW_NORMAL);

			Lock();
			m_stop = CreateEvent(NULL, TRUE, FALSE, NULL);	// イベントは手動リセット！
			m_magic = magic;
			T* pT = static_cast<T*>(this);
			ok = pT->OnStart(param);
			Unlock();
			if (ok)
				ResumeThread(m_thread);
		}
		return ok;
	}

	void Stop()
	{
		if (IsRunning()) {
			// スレッドに終了通知し、終了させる
			SetEvent(m_stop);
		}

		if (m_thread) {
			if (WaitForSingleObject(m_thread, THREAD_EXIT_TIMEOUT) != WAIT_OBJECT_0) {
				// タイムアウトしたら強制終了させる
				TerminateThread(m_thread, (DWORD)-1);
			}
			CloseHandle(m_thread);
		}
		if (m_stop)
			CloseHandle(m_stop);

		Lock();
		m_thread = NULL;
		m_stop = NULL;
		m_magic = 0;
		T* pT = static_cast<T*>(this);
		pT->OnStop();
		Unlock();
	}

	bool IsRunning()
	{
		DWORD code;
		return ((m_thread != NULL) && GetExitCodeThread(m_thread, &code) && (code == STILL_ACTIVE));
	}

	inline bool IsIdle() { return !IsRunning(); }
	inline bool IsCanceled() { return (WaitForSingleObject(m_stop, 0) != WAIT_TIMEOUT); }
	inline void Lock() { m_al.Enter(); }
	inline void Unlock() { m_al.Leave(); }

	// overridable
	virtual bool OnStart(LPARAM)
	{
		return false;
	}

	virtual void OnStop()
	{
	}

	virtual unsigned Run()
	{
		return 0;
	}

protected:

	bool PostItemNotify(LPARAM param)
	{
		if (::IsWindow(m_hWnd)) {
			::PostMessage(m_hWnd, m_msgcode, (WPARAM)m_magic, param);
			return true;
		}
		return false;
	}

	unsigned ThreadProc()
	{
		T* pT = static_cast<T*>(this);
		unsigned code = pT->Run();
		return code;
	}

	static unsigned WINAPI _ThreadProcEntry(CThreadBaseImpl* pThis)
	{
//@		CoInitialize(NULL);
		unsigned ret = pThis->ThreadProc();
//@		CoUninitialize();

		_endthreadex(ret);
		return 0;
	}
};


#endif		// __THREADBASE_H__
