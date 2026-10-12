//////////////////////////////////////////////////////////////////////
// memmgr.cpp

#include "stdafx.h"

#define INITIAL_ARRAY_SIZE	256

mem_manager::mem_manager()
{
	m_arraysize = INITIAL_ARRAY_SIZE;
	m_entries = 0;
	m_parray = (void**)malloc(m_arraysize * sizeof(void*));
	m_total_allocated = 0;
}

void mem_manager::freeall()
{
	if (m_entries > 0)
	{
		for (int i = 0; i < m_entries; i++)
			free(m_parray[i]);
		m_entries = 0;
	}
	free(m_parray);

	m_arraysize = INITIAL_ARRAY_SIZE;
	m_entries = 0;
	m_parray = (void**)malloc(m_arraysize * sizeof(void*));
	m_total_allocated = 0;
}

void* mem_manager::alloc(long size)
{
	if (m_entries + 1 >= m_arraysize)
	{
		int newsize = m_arraysize + INITIAL_ARRAY_SIZE;
		void** ppv = (void**)realloc(m_parray, newsize * sizeof(void*));
		if (!ppv)
			return (void*)0;
		m_parray = ppv;
		m_arraysize = newsize;
	}

	void* pv = malloc(size);
	if (pv)
	{
		m_parray[m_entries++] = pv;
		m_total_allocated += size;
	}
	return pv;
}
