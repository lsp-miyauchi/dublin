/////////////////////////////////////////////////////////////////////////////
//
// Implementation of parameterized Array
//
/////////////////////////////////////////////////////////////////////////////
// NOTE: we allocate an array of 'm_nMaxSize' elements, but only
//  the current size 'm_nSize' contains properly constructed
//  objects.

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////

CPtrArr::CPtrArr()
{
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

CPtrArr::~CPtrArr()
{
	if (m_pData)
		delete[] m_pData;
}

void CPtrArr::SetSize(int nNewSize, int nGrowBy)
{
	if (nGrowBy != -1)
		m_nGrowBy = nGrowBy;  // set new size

	if (nNewSize == 0)
	{
		// shrink to nothing
		delete[] m_pData;
		m_pData = NULL;
		m_nSize = m_nMaxSize = 0;
	}
	else if (m_pData == NULL)
	{
		// create one with exact size
		m_pData = (LPVOID*) new BYTE[nNewSize * sizeof(LPVOID)];
		ZeroMemory(m_pData, nNewSize * sizeof(LPVOID));  // zero fill
		m_nSize = m_nMaxSize = nNewSize;
	}
	else if (nNewSize <= m_nMaxSize)
	{
		// it fits
		if (nNewSize > m_nSize)
			ZeroMemory(&m_pData[m_nSize], (nNewSize-m_nSize) * sizeof(LPVOID));
		m_nSize = nNewSize;
	}
	else
	{
		// otherwise, grow array
		int nGrowBy = m_nGrowBy;
		if (nGrowBy == 0)
		{
			// heuristically determine growth when nGrowBy == 0
			//  (this avoids heap fragmentation in many situations)
			nGrowBy = min(1024, max(4, m_nSize / 8));
		}
		int nNewMax;
		if (nNewSize < m_nMaxSize + nGrowBy)
			nNewMax = m_nMaxSize + nGrowBy;  // granularity
		else
			nNewMax = nNewSize;  // no slush

		LPVOID* pNewData = (LPVOID*) new BYTE[nNewMax * sizeof(LPVOID)];

		// copy new data from old
		CopyMemory(pNewData, m_pData, m_nSize * sizeof(LPVOID));

		// construct remaining elements
		ZeroMemory(&pNewData[m_nSize], (nNewSize - m_nSize) * sizeof(LPVOID));

		// get rid of old stuff (note: no destructors called)
		delete[] m_pData;
		m_pData = pNewData;
		m_nSize = nNewSize;
		m_nMaxSize = nNewMax;
	}
}

int CPtrArr::Append(const CPtrArr& src)
{
	int nOldSize = m_nSize;
	SetSize(m_nSize + src.m_nSize);
	CopyMemory(m_pData + nOldSize, src.m_pData, src.m_nSize * sizeof(LPVOID));
	return nOldSize;
}

void CPtrArr::Copy(const CPtrArr& src)
{
	SetSize(src.m_nSize);
	CopyMemory(m_pData, src.m_pData, src.m_nSize * sizeof(LPVOID));
}

void CPtrArr::FreeExtra()
{
	if (m_nSize != m_nMaxSize)
	{
		// shrink to desired size
		LPVOID* pNewData = NULL;
		if (m_nSize != 0)
		{
			pNewData = (LPVOID*) new BYTE[m_nSize * sizeof(LPVOID)];
			// copy new data from old
			CopyMemory(pNewData, m_pData, m_nSize * sizeof(LPVOID));
		}

		// get rid of old stuff (note: no destructors called)
		delete[] m_pData;
		m_pData = pNewData;
		m_nMaxSize = m_nSize;
	}
}

/////////////////////////////////////////////////////////////////////////////

void CPtrArr::SetAtGrow(int nIndex, LPVOID newElement)
{
	if (nIndex >= m_nSize)
		SetSize(nIndex + 1);
	m_pData[nIndex] = newElement;
}

void CPtrArr::InsertAt(int nIndex, LPVOID newElement, int nCount)
{
	if (nIndex >= m_nSize)
	{
		// adding after the end of the array
		SetSize(nIndex + nCount);  // grow so nIndex is valid
	}
	else
	{
		// inserting in the middle of the array
		int nOldSize = m_nSize;
		SetSize(m_nSize + nCount);  // grow it to new size
		// shift old data up to fill gap
		MoveMemory(&m_pData[nIndex+nCount], &m_pData[nIndex],
			(nOldSize-nIndex) * sizeof(LPVOID));

		// re-init slots we copied from

		ZeroMemory(&m_pData[nIndex], nCount * sizeof(LPVOID));

	}

	// insert new value in the gap
	while (nCount--)
		m_pData[nIndex++] = newElement;
}

void CPtrArr::RemoveAt(int nIndex, int nCount)
{
	// just remove a range
	int nMoveCount = m_nSize - (nIndex + nCount);

	if (nMoveCount)
		CopyMemory(&m_pData[nIndex], &m_pData[nIndex + nCount],
			nMoveCount * sizeof(LPVOID));
	m_nSize -= nCount;
}

void CPtrArr::InsertAt(int nStartIndex, CPtrArr* pNewArray)
{
	if (pNewArray->GetSize() > 0)
	{
		InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
		for (int i = 0; i < pNewArray->GetSize(); i++)
			SetAt(nStartIndex + i, pNewArray->GetAt(i));
	}
}
