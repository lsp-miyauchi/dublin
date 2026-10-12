//////////////////////////////////////////////////////////////////////
// shellnotify.cpp

#include "stdafx.h"
#include "shellnotify.h"


UINT nRM_SHNOTIFY = RegisterWindowMessage(_T("wm_shellnotify"));


CShellNotify::CShellNotify()
{
	m_running = FALSE;
	m_id = 0;
}

CShellNotify::~CShellNotify()
{
	Stop();
}

BOOL CShellNotify::Start(HWND hwnd, LONG events)
{
	if (!m_running)
	{
		SHChangeNotifyEntry nr;
		nr.pidl = NULL;
		nr.fRecursive = TRUE;
		DWORD flags = SHCNRF_ShellLevel | SHCNRF_NewDelivery;
		m_id = SHChangeNotifyRegister(hwnd, flags, events, nRM_SHNOTIFY, 1, &nr);
		if (m_id)
			m_running = TRUE;
	}
	return m_running;
}

void CShellNotify::Stop()
{
	if (m_running && m_id)
		SHChangeNotifyDeregister(m_id);
	m_id = NULL;
	m_running = FALSE;
}

HANDLE CShellNotify::LockEvent(HANDLE hMap, DWORD dwProcessId, LPITEMIDLIST** ppidls, LONG* pdwEventId)
{
	return SHChangeNotification_Lock(hMap, dwProcessId, ppidls, pdwEventId);
}

BOOL CShellNotify::UnlockEvent(HANDLE handle)
{
	return SHChangeNotification_Unlock(handle);
}
