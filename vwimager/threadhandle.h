///////////////////////////////////////////////////////////////////////
// threadhandle.h


#pragma once
#ifndef __THREADHANDLE_H__
#define __THREADHANDLE_H__


typedef unsigned int (__stdcall* PBEGIN_THREAD_START_ROUTINE)(void* param);

class CThreadHandle : public CHandle
{
public:
	inline bool IsNull() { return (m_h == NULL); }
	HANDLE Begin(PBEGIN_THREAD_START_ROUTINE routine, void* param, DWORD flags)
	{
		unsigned id;
		m_h = (HANDLE)_beginthreadex(NULL, 0, routine, param, flags, &id);
		return m_h;
	}
	inline BOOL SetPriority(int priority) { return ::SetThreadPriority(m_h, priority); }
	inline DWORD WaitForExit(DWORD timeout) { return ::WaitForSingleObject(m_h, timeout); }
	inline DWORD Suspend() { return ::SuspendThread(m_h); }
	inline DWORD Resume() { return ::ResumeThread(m_h); }
	bool IsRunning()
	{
		DWORD code;
		return ((m_h != NULL) && GetExitCodeThread(m_h, &code) && (code == STILL_ACTIVE));
	}
};



#endif		// __THREADHANDLE_H__
