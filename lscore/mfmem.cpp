#include "stdafx.h"
#include "mfmem.h"

memstream::memstream(HGLOBAL hmem, DWORD dwSize)
{
	m_hmem = NULL;
	m_pTop = NULL;
	m_pRead = NULL;
	m_dwSize = 0;
	m_dwOffset = 0;

	if (!hmem)
		return;

	m_hmem = hmem;
	m_dwSize = dwSize ? dwSize : (DWORD)GlobalSize(m_hmem);
	m_pTop = m_pRead = (BYTE*)GlobalLock(m_hmem);
	if (!m_pRead) {
		m_hmem = NULL;
		m_dwSize = 0;
		return;
	}
}

memstream::~memstream()
{
	if (m_hmem)
		GlobalUnlock(m_hmem);
}

BYTE memstream::getc()
{
	if (m_hmem)
	{
		if (m_dwSize > m_dwOffset)
		{
			BYTE ch = *m_pRead++;
			m_dwOffset++;
			return ch;
		}
	}
	return 0;
}

WORD memstream::getw()
{
	if (m_hmem)
	{
		if (m_dwSize > m_dwOffset + 1)
		{
			WORD w = *(WORD*)m_pRead;
			m_pRead += 2;
			m_dwOffset += 2;
			return w;
		}
	}
	return 0;
}

int memstream::read(void* pv, DWORD size)
{
	DWORD nRead = 0;
	if (m_hmem)
	{
		if (m_dwSize >= m_dwOffset + size)
			nRead = size;
		else
			nRead = m_dwSize - m_dwOffset;

		if (nRead > 0)
		{
			memcpy(pv, m_pRead, nRead);
			m_pRead += nRead;
			m_dwOffset += nRead;
		}
	}
	return nRead;
}

long memstream::seek(long lOffset, int nOrign)
{
	if (!m_hmem)
		return 0;

	switch (nOrign)
	{
		case FILE_CURRENT:
			lOffset += m_dwOffset;
			if (lOffset < 0)
				lOffset = 0;
			else if (lOffset > (long)m_dwSize)
				lOffset = (long)m_dwSize;
			// fall through

		case FILE_BEGIN:
			m_pRead = m_pTop;
			m_dwOffset = 0;

			if (lOffset <= 0)
				break;

			if (lOffset > (long)m_dwSize)
				lOffset = (long)m_dwSize;
			m_dwOffset = lOffset;
			m_pRead += lOffset;
			break;

		case FILE_END:
			m_pRead = m_pTop;
			m_dwOffset = m_dwSize;
			m_pRead += m_dwSize;

			if (lOffset >= 0)
				break;

			lOffset *= (-1);
			if (lOffset > (long)m_dwSize)
				lOffset = (long)m_dwSize;
			m_dwOffset -= lOffset;
			m_pRead -= lOffset;
			break;

		default:
			break;
	}
	return m_dwOffset;
}
