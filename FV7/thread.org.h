////////////////////////////////////////////////////////////////////////////////
// thread.h

#pragma once
#ifndef __XTHREAD_H__
#define __XTHREAD_H__

#include "folderitem.h"

template <class T>
class CGrepThread
{
public:
	CFileStack* m_pArray;
	IShellFolder* m_psf;
	HWND m_hWnd;						// attend to window
	HANDLE m_stop;						// manual reset event
	HANDLE m_start;						// manual reset event
	HANDLE m_kill;						// auto reset event
	HANDLE m_ack;						// auto reset event
	CRITICAL_SECTION m_critsec;			// 
	DWORD m_magic;
	HANDLE m_thread;
	bool m_running;
	int m_numloops;						// 配列を何回ループして処理するか
	UINT m_msgcode;

	CGrepThread()
	{
		m_pArray = NULL;
		m_psf = NULL;
		m_hWnd = NULL;
		m_stop = CreateEvent(NULL, TRUE, FALSE, NULL);	// イベントは手動リセット！
		m_start = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_kill = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_ack = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_magic = 0;
		m_running = false;
		m_numloops = 1;
		m_msgcode = WM_NULL;

		InitializeCriticalSection(&m_critsec);

		DWORD id;
		m_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)_ThreadProcEntry, this, 0, &id);
		SetThreadPriority(m_thread, static_cast<T*>(this)->PRIORITY);
	}

	~CGrepThread()
	{
		if (m_thread) {
			SetEvent(m_stop);
			WaitForSingleObject(m_ack, INFINITE);
			SetEvent(m_kill);

			// スレッド終了を待つ
			WaitForSingleObject(m_thread, INFINITE);
			CloseHandle(m_thread);
			m_thread = NULL;
		}
		DeleteCriticalSection(&m_critsec);

		if (m_stop) CloseHandle(m_stop);
		if (m_start) CloseHandle(m_start);
		if (m_kill) CloseHandle(m_kill);
		if (m_ack) CloseHandle(m_ack);
	}

	inline bool IsRunning()
	{
		return m_running;
	}

	inline void Lock()
	{
		EnterCriticalSection(&m_critsec);
	}

	inline void Unlock()
	{
		LeaveCriticalSection(&m_critsec);
	}

	inline bool IsCanceled()
	{
		return (WaitForSingleObject(m_stop, 0) != WAIT_TIMEOUT);
	}

	BOOL Reset(HWND hwnd, IShellFolder* psf = NULL, CFileStack* pArray = NULL, DWORD magic = 0, int numLoops = 1)
	{
		if (m_thread) {
			SetEvent(m_stop);
			WaitForSingleObject(m_ack, INFINITE);

			Lock();
			if (m_psf) {
				m_psf->Release();
				m_psf = NULL;
			}
			m_hWnd = hwnd;
			m_pArray = pArray;
			m_magic = magic;
			m_psf = psf;
			m_numloops = max(1, numLoops);
			if (m_psf)
				m_psf->AddRef();
			Unlock();

			m_running = (m_pArray != NULL);
			if (m_running) SetEvent(m_start);
			return TRUE;
		}
		return FALSE;
	}

	virtual void GrepItem(INT_PTR index, CFileItem* pf)
	{
	}

	DWORD ThreadProc()
	{
		HANDLE arrEvent[3];
		arrEvent[0] = m_kill;
		arrEvent[1] = m_stop;
		arrEvent[2] = m_start;

		register INT_PTR index = -1;
		DWORD fWait = INFINITE;
		while (1) {
			DWORD events = WaitForMultipleObjects(3, arrEvent, FALSE, fWait);
			if (events == WAIT_OBJECT_0)
				break;

			Lock();
			INT_PTR cnt = m_pArray ? m_pArray->GetCount() : 0;
			if (events == WAIT_OBJECT_0 + 1) {
				// イベントリセット
				ResetEvent(m_stop);
				index = -1;
				fWait = INFINITE;
				SetEvent(m_ack);
			}
			else if (events == WAIT_OBJECT_0 + 2) {
				m_numloops--;				// ループ実行回数減
				index = (m_pArray && cnt > 0) ? 0 : -1;
				fWait = (index < 0) ? INFINITE : 0;
			}
			else if (events == WAIT_TIMEOUT) {
				if (m_pArray && m_psf && index < cnt) {
					CFileItem* pf = m_pArray->GetAt(index);
					if (!pf->IsUpdir()) {
						T* pT = static_cast<T*>(this);
						pT->GrepItem(index, pf);
					}
					if ((++index) >= cnt) {
						if (--m_numloops < 0)
							index = -1;
						else {
							index = 0;
						}
					}
				}
				fWait = (index < 0) ? INFINITE : 0;
				PostItemNotify(-1);
			}
			Unlock();
		}
		return 0;
	}

	void PostItemNotify(INT_PTR index)
	{
		::PostMessage(m_hWnd, m_msgcode, (WPARAM)index, (LPARAM)m_magic);
	}

	static DWORD WINAPI _ThreadProcEntry(CGrepThread* pThis)
	{
		CoInitialize(NULL);
		DWORD res = pThis->ThreadProc();
		CoUninitialize();
		return res;
	}
};

#endif				// __XTHREAD_H__
