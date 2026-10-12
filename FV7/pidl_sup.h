////////////////////////////////////////////////////////////////////////
// pidl_sup.h

#pragma once

template <bool t_bManaged>
class CPidlT
{
public:
// Data members
	LPITEMIDLIST m_pidl;

// Constructor/destructor/operators
	CPidlT(LPITEMIDLIST pidl = NULL) : m_pidl(pidl)
	{ }

	~CPidlT()
	{
		if(t_bManaged && m_pidl != NULL)
			CoTaskMemFree(m_pidl);
	}

	CPidlT<t_bManaged>& operator =(LPITEMIDLIST pidl)
	{
		Attach(pidl);
		return *this;
	}

	void Attach(LPITEMIDLIST pidl)
	{
		if(t_bManaged && m_pidl != NULL && m_pidl != pidl) {
			Delete();
		}
		m_pidl = pidl;
	}

	LPITEMIDLIST Detach()
	{
		LPITEMIDLIST pdil = m_pidl;
		m_pidl = NULL;
		return pidl;
	}

	CPidlT<t_bManaged>& operator =(LPITEMIDLIST pidl)
	{
		Attach(pidl);
		return *this;
	}

	void Delete()
	{
		LPITEMIDLIST pdil = Detach();
		if (pidl)
			CoTaskMemFree(pidl);
	}

	operator LPITEMIDLIST() const { return m_pidl; }

	bool IsNull() const { return (m_pidl == NULL); }

	inline LPITEMIDLIST Next()
	{
		return (m_pidl != NULL) ? ILGetNext(m_pidl) : NULL;
	}

	inline UINT GetSize()
	{
		return ILGetSize(m_pidl);
	}

	inline UINT GetNumIds()
	{
		return idl_GetNumIds(m_pidl);
	}

	BOOL Append(LPITEMIDLIST pidl)
	{
		BOOL bOK = FALSE;
		if (pidl) {
			LPITEMIDLIST pidlNew;
			if (m_pidl)
				pidlNew = ILCombine(m_pidl, pidl);
			else
				pidlNew = idl_Cpoy(pidl);

			if (pidlNew) {
				Attach(pidlNew);
				bOK = TRUE;
			}
		}
		return bOK;
	}

	inline CPidlT<t_bManaged>& operator +=(LPITEMIDLIST pidl)
	{
		Append(pidl);
		return *this;
	}

	inline LPITEMIDLIST CopyITEMID()
	{
		return ILCloneFirst(m_pidl);
	}

	inline LPITEMIDLIST Copy()
	{
		return ILClone(m_pidl);
	}
};

typedef CPidlT<false>   CIDListPtrHandle;
typedef CPidlT<true>    CIDListPtr;
