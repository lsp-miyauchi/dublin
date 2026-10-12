////////////////////////////////////////////////////////
// cdll.h

#ifndef __CDLL_H__
#define __CDLL_H__

#define MKIR(a)		(MAKEINTRESOURCE((a)))

template <bool t_bManaged>
class CDllT
{
public:
	HMODULE m_hmod;

public:
	CDllT(HMODULE hmod = NULL) : m_hmod(hmod)
	{ }
	~CDllT()
	{
		if (t_bManaged && m_hmod != NULL)
			FreeLibrary();
	}
	CDllT<t_bManaged>& operator =(HMODULE hmod)
	{
		Attach(hmod);
		return *this;
	}
	void Attach(HMODULE hmod)
	{
		if (t_bManaged && m_hmod != NULL && m_hmod != hmod)
			FreeLibrary();
		m_hmod = hmod;
	}
	HMODULE Detach()
	{
		HMODULE hmod = m_hmod;
		m_hmod = NULL;
		return hmod;
	}
	HMODULE LoadLibrary(LPCTSTR pszDll, DWORD flags = 0)
	{
		ATLASSERT(m_hmod == NULL);
		m_hmod = LoadLibraryEx(pszDll, NULL, flags);
		return m_hmod;
	}
	BOOL FreeLibrary()
	{
		ATLASSERT(m_hmod != NULL);
		BOOL bRet = ::FreeLibrary(m_hmod);
		if (bRet)
			m_hmod = NULL;
		return bRet;
	}
	inline FARPROC FindFunction(LPCSTR pszFunc)
	{
		return (m_hmod != NULL) ? GetProcAddress(m_hmod, pszFunc) : NULL;
	}
	inline FARPROC FindFunction(INT_PTR nProcEntry)
	{
		return FindFunction((LPCSTR)nProcEntry);
	}
	operator HMODULE() const { return m_hmod; }
	bool IsNull() const { return (m_hmod == NULL); }
};

typedef CDllT<false>   CDllHandle;
typedef CDllT<true>    CDll;


#endif				// __CDLL_H__
