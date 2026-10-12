////////////////////////////////////////////////////////
// dirgrep.cpp

#include "stdafx.h"
#include "fopstd.h"
#include "dirgrep.h"


#define IN_CRISEC(a)		(EnterCriticalSection((a)))
#define OUT_CRISEC(a)		(LeaveCriticalSection((a)))


////////////////////////////////////////////////////////
// thread function

static DWORD WINAPI dir_grep(void* pv);


////////////////////////////////////////////////////////
// CDirGrepThread

CDirGrepThread::CDirGrepThread()
{
	InitializeCriticalSection(&m_info.m_crit);
}

CDirGrepThread::~CDirGrepThread()
{
	Stop();
	DeleteCriticalSection(&m_info.m_crit);
}

BOOL CDirGrepThread::Start(HWND hwnd, LPCTSTR pszPath)
{
	if (pszPath && _pathisdirectory(pszPath)) {
		m_info.m_kill = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_info.m_hwnd = hwnd;
		lstrcpy(m_info.m_path, pszPath);

		m_thread = CreateThread(NULL, 0, dir_grep, &m_info, 0, &m_id);
		SetThreadPriority(m_thread, THREAD_PRIORITY_BELOW_NORMAL);
		return TRUE;
	}
	return FALSE;
}

void CDirGrepThread::Stop()
{
	if (m_thread) {
		if (m_info.m_stat == DGS_RUNNING) {
			SetEvent(m_info.m_kill);

			// スレッド終了を待つ
			WaitForSingleObject(m_thread, INFINITE);
		}
		CloseHandle(m_thread);
		m_thread = NULL;
	}
}

BOOL CDirGrepThread::IsRunning()
{
	return (m_thread != NULL && m_info.m_stat == DGS_RUNNING);
}

void CDirGrepThread::GetSizes(int* numfiles, int* numfolders, DWORDLONG* tot, DWORDLONG* occ)
{
	IN_CRISEC(&m_info.m_crit);
	*numfiles = m_info.m_numfiles;
	*numfolders = m_info.m_numfolders;
	*tot = m_info.m_total;
	*occ = m_info.m_occupy;
	OUT_CRISEC(&m_info.m_crit);
}

////////////////////////////////////////////////////////
// Thread Process

static BOOL dir_recur(LPCTSTR pszPath, CDirInfo* info)
{
	TCHAR dir[MAX_PATH * 2];
	PathCombine(dir, pszPath, _T("*.*"));

	WIN32_FIND_DATA wfd;
	HANDLE hf = FindFirstFile(dir, &wfd);
	if (hf != INVALID_HANDLE_VALUE) {
		do {
			DWORD events = WaitForSingleObject(info->m_kill, 0);
			if (events != WAIT_TIMEOUT) {
				FindClose(hf);
				return FALSE;
			}

			if (wfd.dwFileAttributes & FA_SUBDIR) {
				if (!_CHECKDOT(wfd.cFileName) && !(wfd.dwFileAttributes & FA_REPARSE)) {
					IN_CRISEC(&info->m_crit);
					info->m_numfolders++;
					OUT_CRISEC(&info->m_crit);

					PathCombine(dir, pszPath, wfd.cFileName);
					if (!dir_recur(dir, info)) {
						FindClose(hf);
						return FALSE;
					}
				}
			}
			else {
				IN_CRISEC(&info->m_crit);

				DWORDLONG dl;
				SETDWORDLONG(dl, wfd.nFileSizeLow, wfd.nFileSizeHigh);
				info->m_total += dl;

				// 占有サイズ
				if (wfd.dwFileAttributes & FA_CMPRSD) {
					DWORD hi, lo;
					TCHAR name[MAX_PATH * 2];
					PathCombine(name, pszPath, wfd.cFileName);
					lo = GetCompressedFileSize(name, &hi);
					if (lo != 0xffffffff || GetLastError() == NO_ERROR)
						SETDWORDLONG(dl, lo, hi);
				}
				info->m_occupy += info->m_pds->ToClustersBytes(dl);
				info->m_numfiles++;
				OUT_CRISEC(&info->m_crit);
			}
		} while (FindNextFile(hf, &wfd));
		FindClose(hf);
	}
	return TRUE;
}

static DWORD WINAPI dir_grep(void* pObject)
{
	CDirInfo* info = (CDirInfo*)pObject;
	TCHAR path[MAX_PATH * 2];
	lstrcpy(path, info->m_path);

	info->m_stat = DGS_RUNNING;
	info->m_total = 0;
	info->m_occupy = 0;
	info->m_numfiles = 0;
	info->m_numfolders = 0;
	info->m_pds = new CDiskSpace(path, true);

	dir_recur(path, info);
	delete info->m_pds;

	info->m_stat = DGS_STOPPED;
	return 0;
}

