////////////////////////////////////////////////////////
// dirgrep.h

#define __DIRGREP_H__

enum {
	DGS_IDLE,
	DGS_RUNNING,
	DGS_STOPPED,
};

struct CDirInfo {
	HWND m_hwnd;						// attend to window
	HANDLE m_kill;						// auto reset event
	HANDLE m_ack;						// auto reset event
	CRITICAL_SECTION m_crit;			// 
	DWORDLONG m_total;
	DWORDLONG m_occupy;
	int m_numfiles;
	int m_numfolders;
	TCHAR m_path[MAX_PATH * 2];
	CDiskSpace* m_pds;
	int m_stat;
};

////////////////////////////////////////////////////////
// CDirGrepThread

class CDirGrepThread
{
public:
	CDirGrepThread();
	~CDirGrepThread();
	BOOL Start(HWND hwnd, LPCTSTR pszPath);
	void Stop();
	void GetSizes(int* numfiles, int* numfolders, DWORDLONG* tot, DWORDLONG* occ);
	BOOL IsRunning();

public:
	HANDLE m_thread;
	DWORD m_id;
	CDirInfo m_info;
};

