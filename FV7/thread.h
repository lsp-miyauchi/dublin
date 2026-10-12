////////////////////////////////////////////////////////////////////////////////
// thread.h

#pragma once
#ifndef __XTHREAD_H__
#define __XTHREAD_H__

#include "folderitem.h"

#ifdef _DEBUG
extern DWORD __gThreadCount;
#endif


////////////////////////////////////////////////////////////////////////
//

#define THREAD_STOP_TIMEOUT			6000		// 6sec
#define THREAD_EXIT_TIMEOUT			1000		// 1sec

template <class T>
class CGrepThread
{
public:
	CFileStack* m_pArray;
	CComPtr<IShellFolder> m_psf;
	HWND m_hWnd;						// attend to window
	HANDLE m_stop;						// manual reset event
	CAccessLock m_cs;					// 
	ULONGLONG m_magic;
	HANDLE m_thread;
	int m_numloops;						// 配列を何回ループして処理するか
	UINT m_msgcode;
	int m_priority;

	CGrepThread()
		: m_thread(NULL)
		, m_pArray(NULL)
		, m_hWnd(NULL)
		, m_stop(NULL)
		, m_magic(0)
		, m_numloops(1)
		, m_msgcode(WM_NULL)
		, m_priority(THREAD_PRIORITY_NORMAL)
	{
	}

	~CGrepThread()
	{
		Stop(THREAD_EXIT_TIMEOUT);
	}

	inline void PostItemNotify(INT_PTR index) { ::PostMessage(m_hWnd, m_msgcode, (WPARAM)index, (LPARAM)m_magic); }

	BOOL Start(HWND hwnd, IShellFolder* psf, CFileStack* pArray, ULONGLONG magic, int numLoops = 1)
	{
		Stop();

		ATLASSERT(psf != NULL);
		ATLASSERT(pArray != NULL);

#ifdef __USE_CREATETHREAD__
		DWORD id;
		m_thread = CreateThread(NULL, 0, _ThreadProcEntry, this, CREATE_SUSPENDED, &id);
#else
		unsigned id;
		m_thread = (HANDLE)_beginthreadex(NULL, 0, _ThreadProcEntry, this, CREATE_SUSPENDED, &id);
#endif
		if (m_thread) {
#ifdef _DEBUG
			__gThreadCount++;
#endif
			::SetThreadPriority(m_thread, m_priority);

			m_cs.Enter();
			m_stop = CreateEvent(NULL, TRUE, FALSE, NULL);	// イベントは手動リセット！
			m_hWnd = hwnd;
			m_pArray = pArray;
			m_magic = magic;
			m_psf = psf;
			m_numloops = max(1, numLoops);
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
			CloseHandle(m_stop);
			CloseHandle(m_thread);

			m_thread = NULL;
			m_psf.Release();
			m_hWnd = NULL;
			m_pArray = NULL;
			m_magic = 0;
			m_numloops = 1;
			m_stop = NULL;
#ifdef _DEBUG
			__gThreadCount--;
#endif
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
		HANDLE handle = m_stop;

		T* pT = static_cast<T*>(this);
		pT->GrepInitialize();

		bool bContinue = true;
		int index = 0;
		while (bContinue) {
			DWORD events = WaitForSingleObject(handle, 0);
			if (events == WAIT_OBJECT_0)
				break;
			else if (events == WAIT_TIMEOUT) {
				m_cs.Enter();
				int cnt = m_pArray->GetSize();
				if (index < cnt) {
					CFileItem* pf = (*m_pArray)[index];
					if (!pf->isUpdir()) {
						pT->GrepItem(index, pf);
					}
					++index;
					if (index >= cnt) {
						--m_numloops;
						if (m_numloops <= 0)
							bContinue = false;
						else
							index = 0;
					}
				}
				PostItemNotify(-1);
				m_cs.Leave();
			}
		}

		pT->GrepUninitialize();
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
//		return 0;
	}

	virtual void GrepInitialize() { }
	virtual void GrepUninitialize() { }
	virtual void GrepItem(INT_PTR, CFileItem*) { }
};

#endif				// __XTHREAD_H__
