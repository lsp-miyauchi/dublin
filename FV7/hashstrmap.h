//////////////////////////////////////////////////////////////////////
// CHashStringMap

#pragma once
#ifndef __HASHSTRMAP_H__
#define __HASHSTRMAP_H__

class CHashItem
{
public:
	DWORD m_hash;
	LPCTSTR m_psz;

	CHashItem(LPCTSTR psz = NULL)
	{
		m_psz = psz;
		m_hash = Hash(psz);
	}
	DWORD Hash(LPCTSTR psz)
	{
		DWORD h = 0;
		while (psz && *psz) {
			h = (h << 5) + h + (*psz);
			psz++;
		}
		return h;
	}
	__inline BOOL Compare(LPCTSTR psz) { return (m_hash != Hash(psz)) ? FALSE : (lstrcmp(psz, m_psz) == 0); }
};

class CHashStringArray : public CSimpleArray<CHashItem*>
{
public:
	~CHashStringArray()
	{
		DeleteAll();
	}
	void DeleteAll()
	{
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++) {
			CHashItem* pi = (*this)[i];
			delete pi;
		}
		RemoveAll();
	}
	LPCTSTR Lookup(LPCTSTR psz)
	{
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++) {
			CHashItem* pi = (*this)[i];
			if (pi->Compare(psz))
				return pi->m_psz;
		}
		return NULL;
	}
	__inline BOOL AddItem(LPCTSTR psz) { return CSimpleArray<CHashItem*>::Add(new CHashItem(psz)); }
};

#endif		// __HASHSTRMAP_H__
