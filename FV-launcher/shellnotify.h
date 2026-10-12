////////////////////////////////////////////////////////
// shellnotify.h

#pragma once
#ifndef __SHELLNOTIFY_H__
#define __SHELLNOTIFY_H__

/////////////////////////////////////////////////////////////////////
// CShellNotify

extern UINT nRM_SHNOTIFY;

#ifndef SHCNRF_InterruptLevel
	#define SHCNRF_InterruptLevel		0x0001
	#define SHCNRF_ShellLevel			0x0002
	#define SHCNRF_RecursiveInterrupt	// ??
	#define SHCNRF_NewDelivery			0x8000
#endif


class CShellNotify
{
private:
	UINT m_id;
public:
	CShellNotify()
		: m_id(0)
	{
	}
	~CShellNotify()
	{
		Stop();
	}
	bool Start(HWND hwnd, LONG events)
	{
		if (!m_id) {
			SHChangeNotifyEntry nr;
			nr.pidl = NULL;
			nr.fRecursive = TRUE;
			DWORD flags = SHCNRF_ShellLevel | SHCNRF_NewDelivery;
			m_id = SHChangeNotifyRegister(hwnd, flags, events, nRM_SHNOTIFY, 1, &nr);
		}
		return (m_id != 0);
	}
	void Stop()
	{
		if (m_id) {
			SHChangeNotifyDeregister(m_id);
			m_id = 0;
		}
	}
	HANDLE LockEvent(HANDLE hMap, DWORD dwProcessId, LPITEMIDLIST** ppidls, LONG* pdwEventId)
	{
		return SHChangeNotification_Lock(hMap, dwProcessId, ppidls, pdwEventId);
	}
	BOOL UnlockEvent(HANDLE handle)
	{
		return SHChangeNotification_Unlock(handle);
	}
};

#endif			// __SHELLNOTIFY_H__
