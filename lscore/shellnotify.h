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
public:
	CShellNotify();
	~CShellNotify();

	BOOL Start(HWND hwnd, LONG events);
	void Stop();
	HANDLE LockEvent(HANDLE hMap, DWORD dwProcessId, LPITEMIDLIST** ppilds, LONG* pdwEventId);
	BOOL UnlockEvent(HANDLE handle);

protected:
	ULONG m_id;
	BOOL m_running;
};
