/////////////////////////////////////////////////////////////////////
// CWatchFolder

#pragma once

typedef struct _WATCHFOLDERINFO {
	DWORD action;
	TCHAR name[MAX_PATH * 2];
} WATCHFOLDERINFO;

#define MAX_BUFFER  4096

typedef struct _IOFOLDER {
	BYTE res[MAX_BUFFER];
	DWORD len;
	OVERLAPPED ovl;
	HANDLE dir;
	HANDLE port;
	ULONG_PTR key;
} IOFOLDER;

class CWatchFolder
{
public:
	HWND m_hWnd;
	ULONGLONG m_magic;
	HANDLE m_kill;
	HANDLE m_thread;

	TCHAR m_path[MAX_PATH];
	IOFOLDER* m_pio;
	DWORD m_filter;
	BOOL m_tree;

	CWatchFolder() :	m_hWnd(NULL),
						m_thread(NULL),
						m_magic(0),
						m_tree(FALSE)
	{
		m_pio = (IOFOLDER*)LocalAlloc(LPTR, sizeof(IOFOLDER));
		m_filter =	FILE_NOTIFY_CHANGE_FILE_NAME |
					FILE_NOTIFY_CHANGE_DIR_NAME |
					FILE_NOTIFY_CHANGE_ATTRIBUTES |
//@					FILE_NOTIFY_CHANGE_CREATION |
//@					FILE_NOTIFY_CHANGE_SIZE |
					FILE_NOTIFY_CHANGE_LAST_WRITE;
	}

	~CWatchFolder()
	{
		Stop();
		LocalFree(m_pio);
	}

	bool Start(HWND hwnd, LPCTSTR pszPath, ULONGLONG magic)
	{
		if (m_thread) Stop();
		if (!pszPath || *pszPath == 0) return false;

#ifdef __USE_CREATETHREAD__
		DWORD id;
		m_thread = CreateThread(NULL, 0, _ThreadProcEntry, this, CREATE_SUSPENDED, &id);
#else
		unsigned id;
		m_thread = (HANDLE)_beginthreadex(NULL, 0, _ThreadProcEntry, this, CREATE_SUSPENDED, &id);
#endif
		if (m_thread) {
			SetThreadPriority(m_thread, THREAD_PRIORITY_BELOW_NORMAL);

			m_kill = CreateEvent(NULL, FALSE, FALSE, NULL);
			m_hWnd = hwnd;
			m_magic = magic;
			lstrcpy(m_path, pszPath);
			ResumeThread(m_thread);
			return true;
		}
		return false;
	}

	void Stop()
	{
		if (m_thread) {
			if (m_pio->port) {
				// I/O 完了パケットを I/O 完了ポートにポストして、
				// GetQueuedCompletionStatus 関数呼び出しを終了させる
				PostQueuedCompletionStatus(m_pio->port, 0, 0, NULL);
			}
			const DWORD thread_wait_timeout = 10000;		// 10sec
			if (WaitForSingleObject(m_thread, thread_wait_timeout) != WAIT_OBJECT_0) {
				// スレッドを強制終了
				TerminateThread(m_thread, (DWORD)-1);
			}
			CloseHandle(m_thread);
			CloseHandle(m_kill);

			m_thread = NULL;
			m_kill = NULL;
			m_hWnd = NULL;
			m_magic = 0;
			m_path[0] = 0;
		}
	}

	BOOL ReadFolderChanges()
	{
		memset(&m_pio->res, 0, sizeof(m_pio->res));
		return ReadDirectoryChangesW(m_pio->dir, m_pio->res, sizeof(m_pio->res), m_tree, m_filter, &m_pio->len, &m_pio->ovl, NULL);
	}

	bool OpenFolder()
	{
		m_pio->dir = CreateFile(m_path,
						FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
						NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
		return (m_pio->dir != INVALID_HANDLE_VALUE);
	}

	void CloseFolder()
	{
		if (m_pio->dir != INVALID_HANDLE_VALUE) {
			CloseHandle(m_pio->dir);
			m_pio->dir = INVALID_HANDLE_VALUE;
		}
	}

	void CheckChangedFile(FILE_NOTIFY_INFORMATION* pfni)
	{
		WATCHFOLDERINFO* pi = (WATCHFOLDERINFO*)malloc(sizeof(WATCHFOLDERINFO));
		if (pi) {
			memset(pi, 0, sizeof(WATCHFOLDERINFO));
			TCHAR buf[MAX_PATH];
			pi->action = pfni->Action;
			memcpy(buf, pfni->FileName, pfni->FileNameLength);
			buf[(pfni->FileNameLength / sizeof(TCHAR))] = 0;
			PathCombine(pi->name, m_path, buf);
			PostMessage(m_hWnd, WM_FILENOTIFY, m_magic, (LPARAM)pi);
		}
	}

	unsigned ThreadProc()
	{
		if (OpenFolder()) {
			m_pio->key = GetTickCount();
			if (!m_pio->key) m_pio->key++;		// 0にしない

			m_pio->port = CreateIoCompletionPort(m_pio->dir, NULL, m_pio->key, 0);
			if (m_pio->port) {
				if (ReadFolderChanges()) {
					ULONG_PTR key;
					do {
						// Retrieve the directory info for this directory through the completion key
						DWORD numBytes;
						OVERLAPPED* povl;
						GetQueuedCompletionStatus(m_pio->port, &numBytes, &key, &povl, INFINITE);
						if (key == m_pio->key) {
							FILE_NOTIFY_INFORMATION* pfni = (FILE_NOTIFY_INFORMATION*)m_pio->res;
							DWORD size;
							do {
								size = pfni->NextEntryOffset;
								CheckChangedFile(pfni);
								pfni = (FILE_NOTIFY_INFORMATION*)((BYTE*)pfni + size);
							} while (size);
							ReadFolderChanges();
						}
					} while (key);
				}
				CloseHandle(m_pio->port);
				m_pio->port = NULL;
			}
			m_pio->key = 0;
			CloseFolder();
		}
		return 0;
	}

#ifdef __USE_CREATETHREAD__
	static DWORD WINAPI _ThreadProcEntry(void* param)
#else
	static unsigned __stdcall _ThreadProcEntry(void* param)
#endif
	{
		CWatchFolder* pThis = (CWatchFolder*)param;
		unsigned ret = pThis->ThreadProc();
#ifdef __USE_CREATETHREAD__
		ExitThread(ret);
#else
		_endthreadex(ret);
#endif
//		return 0;
	}
};

