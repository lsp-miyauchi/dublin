/////////////////////////////////////////////////////////////////////
// shellitemsearch

#pragma once
#ifndef __shellitemsearch_h__
#define __shellitemsearch_h__

class CShellItemSearch
{
public:
	HWND m_hWnd;
	HANDLE m_thread;
	CComPtr<IShellFolder> m_psf;
	UINT m_wm_shellitemsearch;
	LPARAM m_userdata;

	CShellItemSearch()
		: m_hWnd(NULL)
		, m_thread(NULL)
		, m_userdata(0)
		, m_wm_shellitemsearch(RegisterWindowMessage(_T("wm_shellitemsearch")))
	{
	}
	~CShellItemSearch()
	{
		Stop();
	}

	bool Start(HWND hwnd, REFKNOWNFOLDERID rid, LPARAM userdata = NULL)
	{
		bool ok = false;
		LPITEMIDLIST pidl = NULL;
		if (SHGetKnownFolderIDList(rid, 0, NULL, &pidl) == S_OK) {
			ok = Start(hwnd, pidl);
			CoTaskMemFree(pidl);
		}
		return ok;
	}

	bool Start(HWND hwnd, int csidl, LPARAM userdata = NULL)
	{
		bool ok = false;
		LPITEMIDLIST pidl = NULL;
		if (SHGetFolderLocation(hwnd, csidl, NULL, 0, &pidl) == S_OK) {
			ok = Start(hwnd, pidl);
			CoTaskMemFree(pidl);
		}
		return ok;
	}

	bool Start(HWND hwnd, LPCTSTR path, LPARAM userdata = NULL)
	{
		bool ok = false;
		LPITEMIDLIST pidl = ILCreateFromPath(path);
		if (pidl) {
			ok = Start(hwnd, pidl);
			CoTaskMemFree(pidl);
		}
		return ok;
	}

	bool Start(HWND hwnd, LPCITEMIDLIST pidl, LPARAM userdata = NULL)
	{
		if (pidl != NULL) {
			CComPtr<IShellFolder> psf = idl_GetShellFolder(pidl);
			return !psf.p ? false : Start(hwnd, psf);
		}
		return false;
	}

	bool Start(HWND hwnd, IShellFolder* psf, LPARAM userdata = NULL)
	{
		Stop();
		if (psf == NULL) return false;

#ifdef __USE_CREATETHREAD__
		DWORD id;
		m_thread = CreateThread(NULL, 0, _ThreadProcEntry, this, CREATE_SUSPENDED, &id);
#else
		unsigned id;
		m_thread = (HANDLE)_beginthreadex(NULL, 0, _ThreadProcEntry, this, CREATE_SUSPENDED, &id);
#endif
		if (m_thread) {
			m_psf = psf;
			m_hWnd = hwnd;
			ResumeThread(m_thread);
			return true;
		}
		return false;
	}

	void Stop()
	{
		if (m_thread) {
			const DWORD thread_wait_timeout = 3000;		// 3sec
			if (WaitForSingleObject(m_thread, thread_wait_timeout) != WAIT_OBJECT_0) {
				// スレッドを強制終了
				TerminateThread(m_thread, (DWORD)-1);
			}
			CloseHandle(m_thread);
			m_thread = NULL;
			m_hWnd = NULL;
		}
		m_psf.Release();
		m_userdata = 0;
	}

	unsigned ThreadProc()
	{
		CComPtr<IEnumIDList> pe;
		if (m_psf->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_FASTITEMS, &pe) == S_OK) {
			int numObjects = 0;
			LPITEMIDLIST pidl;
			ULONG celt;
			while (S_OK == pe->Next(1, &pidl, &celt) && celt) {
				if (!pidl) break;

				CoTaskMemFree(pidl);
				numObjects++;
			}
			// 終了を通知
			::PostMessage(m_hWnd, m_wm_shellitemsearch, (WPARAM)numObjects, m_userdata);
		}
		return 0;
	}

#ifdef __USE_CREATETHREAD__
	static DWORD WINAPI _ThreadProcEntry(void* param)
#else
	static unsigned __stdcall _ThreadProcEntry(void* param)
#endif
	{
		CoInitialize(NULL);
		CShellItemSearch* pThis = (CShellItemSearch*)param;
		unsigned ret = pThis->ThreadProc();
		CoUninitialize();
#ifdef __USE_CREATETHREAD__
		ExitThread(ret);
#else
		_endthreadex(ret);
#endif
//		return 0;
	}
};

#endif		// __shellitemsearch_h__
