#ifndef __ARRAY_P_INCLUDE
#define __ARRAY_P_INCLUDE


class CPtrArr
{
public:
// 構築
	CPtrArr();

// 属性
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

// 操作
	// Clean up
	void FreeExtra();
	void RemoveAll();

	// Accessing elements
	LPVOID GetAt(int nIndex) const;
	LPVOID GetHead() const;
	LPVOID GetTail() const;
	void SetAt(int nIndex, LPVOID newElement);
	LPVOID& ElementAt(int nIndex);

	// Direct Access to the element data (may return NULL)
	LPVOID* GetData();

	// Potentially growing the array
	void SetAtGrow(int nIndex, LPVOID newElement);
	int Add(LPVOID newElement);
	int Append(const CPtrArr& src);
	void Copy(const CPtrArr& src);

	// overloaded operator helpers
	LPVOID operator[](int nIndex) const;
	LPVOID& operator[](int nIndex);

	// Operations that move elements around
	void InsertAt(int nIndex, LPVOID newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CPtrArr* pNewArray);

// インプリメンテーション
protected:
	LPVOID* m_pData;   // the actual array of data
	int m_nSize;     // # of elements (upperBound - 1)
	int m_nMaxSize;  // max allocated
	int m_nGrowBy;   // grow amount

public:
	~CPtrArr();
};

_inline int CPtrArr::GetSize() const
	{ return m_nSize; }
_inline int CPtrArr::GetUpperBound() const
	{ return m_nSize - 1; }
_inline void CPtrArr::RemoveAll()
	{ SetSize(0); }
_inline LPVOID CPtrArr::GetAt(int nIndex) const
	{ return (m_nSize > 0 && nIndex >= 0 && nIndex < m_nSize) ? m_pData[nIndex] : NULL; }
_inline LPVOID CPtrArr::GetHead() const
	{ return GetAt(0); }
_inline LPVOID CPtrArr::GetTail() const
	{ return GetAt(m_nSize - 1); }
_inline void CPtrArr::SetAt(int nIndex, LPVOID newElement)
	{ m_pData[nIndex] = newElement; }
_inline LPVOID& CPtrArr::ElementAt(int nIndex)
	{ return m_pData[nIndex]; }
_inline LPVOID* CPtrArr::GetData()
	{ return m_pData; }
_inline int CPtrArr::Add(LPVOID newElement)
	{ int nIndex = m_nSize; SetAtGrow(nIndex, newElement); return nIndex; }
_inline LPVOID CPtrArr::operator[](int nIndex) const
	{ return GetAt(nIndex); }
_inline LPVOID& CPtrArr::operator[](int nIndex)
	{ return ElementAt(nIndex); }


#endif		// __ARRAY_P_INCLUDE
