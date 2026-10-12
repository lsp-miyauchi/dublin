//////////////////////////////////////////////////////////////////////
// メモリアロケート用

#pragma once
#ifndef __TYPEALLOC_H__
#define __TYPEALLOC_H__

#include <memory.h>

inline void __declspec(noreturn) _xxRaiseException( DWORD dwExceptionCode, DWORD dwExceptionFlags = EXCEPTION_NONCONTINUABLE )
{
	RaiseException( dwExceptionCode, dwExceptionFlags, 0, NULL );
}

template <class T>
class CTypeAlloc
{
private:
	T* m_aT;
	int m_size;
public:
	__inline CTypeAlloc(int size) { m_size = size; m_aT = (T*)malloc(sizeof(T) * size); }
	__inline ~CTypeAlloc() { if (m_aT) free(m_aT); }
	__inline operator T*() const { return m_aT; }
	__inline void SetAt(int index, T t) { m_aT[index] = t; }
//@	__inline T& operator[] (int index) { return m_aT[index]; }
//@	__inline const T& operator[] (int index) const { return m_aT[index]; }
};


#endif			// __TYPEALLOC_H__
