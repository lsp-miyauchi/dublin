//////////////////////////////////////////////////////////////////////
// virtmem.cpp

#include "stdafx.h"

CVirtMem::CVirtMem()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	m_nSysPageSize = si.dwPageSize;
	m_nPageSize = 0;
	m_nDataSize = 0;
}

CVirtMem::~CVirtMem()
{
	Free();
}

BOOL CVirtMem::Alloc(size_t size)
{
	if (m_pPage) Free();

	if (!size) return TRUE;

	size_t nPageSize = (size / m_nSysPageSize) * m_nSysPageSize;
	if (size % m_nSysPageSize)
		nPageSize += m_nSysPageSize;

	m_pPage = (LPBYTE)valloc(nPageSize);
	if (m_pPage) {
		m_nDataSize = size;
		m_nPageSize = nPageSize;
		return TRUE;
	}
	return FALSE;
}

BOOL CVirtMem::Realloc(size_t size)
{
	if (!m_pPage || !size) return Alloc(size);

	if (size > m_nPageSize || size <= m_nPageSize - m_nSysPageSize) {
		BOOL bOK = FALSE;
		void* pv = valloc(size);
		if (pv) {
			size_t n = min(size, m_nDataSize);
			memcpy(pv, m_pPage, n);
			if (Alloc(size)) {
				memcpy(m_pPage, pv, n);
				bOK = TRUE;
			}
			vfree(pv);
		}
		return bOK;
	}

	m_nDataSize = size;
	return TRUE;
}

void CVirtMem::Free()
{
	if (m_pPage) {
		vfree(m_pPage);
		m_pPage = NULL;
		m_nDataSize = 0;
		m_nPageSize = 0;
	}
}

size_t CVirtMem::Read(UINT_PTR posOffset, LPVOID pBuffer, size_t size)
{
	if (m_nDataSize > posOffset) {
		size_t n = min(size, m_nDataSize - posOffset);
		memcpy(pBuffer, m_pPage + posOffset, n);
		return n;
	}
	return 0;
}

size_t CVirtMem::Write(UINT_PTR posOffset, LPVOID pBuffer, size_t size)
{
	if (m_nPageSize < posOffset + size) {
		if (!Realloc(posOffset + size))
			return 0;
	}
	memcpy(m_pPage + posOffset, pBuffer, size);
	return size;
}

