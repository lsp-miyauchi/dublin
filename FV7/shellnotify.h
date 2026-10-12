////////////////////////////////////////////////////////
// shellnotify.h

#pragma once

#ifndef __SHELLNOTIFY_H__
#define __SHELLNOTIFY_H__

// #define SHELLNOTIFY_VERBOSE

template<class T> void ifSafeRelease(T** ppT)
{
	if (*ppT) { (*ppT)->Release(); *ppT = NULL; }
}

template<class T> HRESULT ifSetInterface(T** ppT, IUnknown* punk)
{
	SafeRelease(ppT); return punk ? punk->QueryInterface(ppT) : E_NOINTERFACE;
}

///////////////////////////////////////////////////////////////////////
//

template <class T>
class CShellChangeNotifyImpl
{
private:
	ULONG m_ulShellNotifyRegister;
	UINT m_wm_shellnotify;

public:
	CShellChangeNotifyImpl()
		: m_ulShellNotifyRegister(0)
		, m_wm_shellnotify(::RegisterWindowMessage(_T("wm_shellnotify")))
	{ }
	~CShellChangeNotifyImpl() { }

BEGIN_MSG_MAP(CShellChangeNotifyImpl)
	ON_REGISTERD_MESSAGE(m_wm_shellnotify, OnSCHShellNotify)
	MESSAGE_HANDLER(WM_DESTROY, OnHandlerSCHStopNotify)
	MESSAGE_HANDLER(WM_ENDSESSION, OnHandlerSCHStopNotify)
END_MSG_MAP()

private:
	LRESULT OnHandlerSCHStopNotify(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		StopShellChangeNotify();
		bHandled = FALSE;
		return 0;
	}

#ifdef _DEBUG
#ifdef SHELLNOTIFY_VERBOSE
	LPCTSTR _getnotify_text(LPARAM l)
	{
		if (l & SHCNE_RENAMEITEM)		return _T("RENAMEITEM ");
		if (l & SHCNE_CREATE)			return _T("CREATE ");
		if (l & SHCNE_DELETE)			return _T("DELETE ");
		if (l & SHCNE_MKDIR)			return _T("MKDIR ");
		if (l & SHCNE_RMDIR)			return _T("RMDIR ");
		if (l & SHCNE_MEDIAINSERTED)	return _T("MEDIAINSERTED ");
		if (l & SHCNE_MEDIAREMOVED)		return _T("MEDIAREMOVED ");
		if (l & SHCNE_DRIVEREMOVED)		return _T("DRIVEREMOVED ");
		if (l & SHCNE_DRIVEADD)			return _T("DRIVEADD ");
		if (l & SHCNE_NETSHARE)			return _T("NETSHARE ");
		if (l & SHCNE_NETUNSHARE)		return _T("NETUNSHARE ");
		if (l & SHCNE_ATTRIBUTES)		return _T("ATTRIBUTES ");
		if (l & SHCNE_UPDATEDIR)		return _T("UPDATEDIR ");
		if (l & SHCNE_UPDATEITEM)		return _T("UPDATEITEM ");
		if (l & SHCNE_SERVERDISCONNECT)	return _T("SERVERDISCONNECT ");
		if (l & SHCNE_UPDATEIMAGE)		return _T("UPDATEIMAGE ");
		if (l & SHCNE_DRIVEADDGUI)		return _T("DRIVEADDGUI ");
		if (l & SHCNE_RENAMEFOLDER)		return _T("RENAMEFOLDER ");
		if (l & SHCNE_FREESPACE)		return _T("FREESPACE ");
		if (l & SHCNE_EXTENDED_EVENT)	return _T("EXTENDED_EVENT ");
		if (l & SHCNE_ASSOCCHANGED)		return _T("ASSOCCHANGED ");
		return _T("** unknown ** ");
	}
#endif
#endif

	LRESULT OnSCHShellNotify(WPARAM wParam, LPARAM lParam)
	{
		long lEvent;
		PIDLIST_ABSOLUTE* rgpidl;
		HANDLE hNotifyLock = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &rgpidl, &lEvent);
		if (hNotifyLock) {
/*
			CComPtr<IShellItem2> psi1;
			CComPtr<IShellItem2> psi2;
			if (rgpidl[0]) { SHCreateItemFromIDList(rgpidl[0], IID_PPV_ARGS(&psi1)); }
			if (rgpidl[1]) { SHCreateItemFromIDList(rgpidl[1], IID_PPV_ARGS(&psi2)); }
			// derived class implements this method, that is where the events are delivered
			T* pT = static_cast<T*>(this);
			pT->OnShellChangeNotify(lEvent, psi1, psi2);
*/

#ifdef _DEBUG
#ifdef SHELLNOTIFY_VERBOSE
	TCHAR buf[512];
	CString str;
	dbg.printf(_T("%s:"), _getnotify_text(lEvent));
	if (SHGetPathFromIDList(rgpidl[0], buf))
		dbg.printf(_T("%s\n"), buf);
	else if (idl_GetName(rgpidl[0], SHGDN_FORPARSING, str)) {
		CString tmp;
		if (idl_GetName(rgpidl[0], SHGDN_NORMAL, tmp))
			str.Format(_T(" [%s]"), tmp.GetString());
		dbg.printf(_T("%s\n"), str.GetString());
	}
	if (lEvent == SHCNE_RENAMEFOLDER) {
		if (SHGetPathFromIDList(rgpidl[1], buf))
			dbg.printf(_T(" -->%s\n"), buf);
		else if (idl_GetName(rgpidl[1], SHGDN_FORPARSING, str))
			dbg.printf(_T(" -->%s\n"), str.GetString());
	}
#endif
#endif

			T* pT = static_cast<T*>(this);
			pT->OnShellChangeNotify(lEvent, rgpidl);

			SHChangeNotification_Unlock(hNotifyLock);
		}
		return 0;
	}

public:
	// lEvents is SHCNE_XXX values like SHCNE_ALLEVENTS
	// fRecursive means to listen for all events under this folder
	HRESULT StartShellChangeNotify(PIDLIST_ABSOLUTE pidl, long lEvents, BOOL fRecursive)
	{
		T* pT = static_cast<T*>(this);
		StopShellChangeNotify();
		SHChangeNotifyEntry const entries[] = { pidl, fRecursive };
		int const nSources = SHCNRF_ShellLevel | SHCNRF_NewDelivery;
		m_ulShellNotifyRegister = SHChangeNotifyRegister(pT->m_hWnd, nSources, lEvents, m_wm_shellnotify, ARRAYSIZE(entries), entries);
		return m_ulShellNotifyRegister != 0 ? S_OK : E_FAIL;
	}

	HRESULT StartShellChangeNotify(IShellItem *psi, long lEvents, BOOL fRecursive)
	{
		HRESULT hr = E_FAIL;
		PIDLIST_ABSOLUTE pidlWatch;
		hr = SHGetIDListFromObject(psi, &pidlWatch);
		if (SUCCEEDED(hr)) {
			hr = StartShellChangeNotify(pidlWatch, lEvents, fRecursive);
			CoTaskMemFree(pidlWatch);
		}
		return hr;
	}

	void StopShellChangeNotify()
	{
		if (m_ulShellNotifyRegister) {
			SHChangeNotifyDeregister(m_ulShellNotifyRegister);
			m_ulShellNotifyRegister = 0;
		}
	}

	// derived class implements this event
//	virtual void OnShellChangeNotify(long lEvent, IShellItem2 *psi1, IShellItem2 *psi2) = 0;
	virtual void OnShellChangeNotify(long lEvent, PIDLIST_ABSOLUTE* pidls) = 0;
};



#endif			// __SHELLNOTIFY_H__
