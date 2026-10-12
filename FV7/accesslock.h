////////////////////////////////////////////////////////////////////////
// accesslock.h

#pragma once

class CAccessLock
{
public:
	CRITICAL_SECTION m_crisec;
	CAccessLock() { InitializeCriticalSectionAndSpinCount(&m_crisec, 1024 * 4); }
	~CAccessLock() { DeleteCriticalSection(&m_crisec); }
	__inline void Enter() { EnterCriticalSection(&m_crisec); }
	__inline BOOL TryEnter() { TryEnterCriticalSection(&m_crisec); }
	__inline void Leave() { LeaveCriticalSection(&m_crisec); }
};

