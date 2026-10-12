//////////////////////////////////////////////////////////////////////
// memmgr.h

#ifndef _MEM_MANAGER_H
	#define _MEM_MANAGER_H

class mem_manager
{
public:
	mem_manager();
	~mem_manager();

	void* alloc(long size);
	void freeall();

protected:
	int m_arraysize;
	int m_entries;
	void** m_parray;
	unsigned long m_total_allocated;
};

_inline mem_manager::~mem_manager()
	{ freeall(); }

#endif			// _MEM_MANAGER_H
