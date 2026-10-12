////////////////////////////////////////////////////////////////////////
// cabmodule.h

#pragma once
#ifndef __CABMODULE_H__
#define __CABMODULE_H__

//////////////////////////////////////////////////////////////////////
// メモリアロケート用

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
	__inline bool SetAt(int index, T t) { m_aT[index] = t; return true; }
	__inline const T& operator[] (int index) const { return m_aT[index]; }
	__inline T& operator[] (int index) { return m_aT[index]; }
};

///////////////////////////////////////////////////////////////////////
//

class CCabModule
{
public:
	CWindow m_owner;
	long m_module_ref;
	DWORD m_module_lock;

	HRESULT Init(ATL::_ATL_OBJMAP_ENTRY* pObjMap, HINSTANCE hInstance, const GUID* pLibID = NULL)
	{
		UNREFERENCED_PARAMETER(pLibID);
		UNREFERENCED_PARAMETER(hInstance);
		UNREFERENCED_PARAMETER(pObjMap);

		m_module_ref = 0;
		m_module_lock = 0;
		m_owner = (HWND)NULL;
		return S_OK;
	}
	void Term()
	{
	}
	HWND SetOwnerWindow(HWND hwndOwner)
	{
		HWND hwndPrev = m_owner.m_hWnd;
		m_owner = hwndOwner;
		return hwndPrev;
	}
	HWND GetOwnerWindow()
	{
		return m_owner;
	}
	inline void AddModuleRef() { m_module_ref++; }
	inline void DecModuleRef() { m_module_ref--; }
	inline bool IsModuleRef() { return (m_module_ref > 0); }
	inline DWORD GetModuleLock() { return m_module_lock; }
	inline void SetModuleLock(DWORD lock) { m_module_lock = lock; }
	inline bool IsModuleLocked() { return (m_module_lock != 0); }
};


extern CCabModule _G;


#endif			// __CABMODULE_H__
