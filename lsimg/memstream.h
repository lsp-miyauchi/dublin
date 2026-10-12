//////////////////////////////////////////////////////////////////////
// memstream.h

#ifndef __MEMSTREAM_H__
#define __MEMSTREAM_H_


#pragma once

#define MEM_GROWSIZE	4096


static void* file2mem(const wchar_t* fname, size_t* psize)
{
	*psize = 0;
	void* memptr = NULL;
	HANDLE hf = CreateFileW(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		ULARGE_INTEGER filesize;
		filesize.LowPart = GetFileSize(hf, &filesize.HighPart);
		void* memptr = malloc(filesize.LowPart + 16);
		if (memptr) {
			DWORD read;
			if (ReadFile(hf, memptr, filesize.LowPart, &read, NULL)) {
				*psize = (size_t)read;
			}
			else {
				free(memptr);
				memptr = NULL;
			}
		}
		CloseHandle(hf);
	}
	return memptr;
}

class memstream 
{
private:
	uint8_t* m_allocator;		// 自身以外でセットたメモリを利用する場合は NULL
	uint8_t* m_baseptr;
	uint8_t* m_readptr;
	size_t m_memsize;
	size_t m_offset;
	size_t m_glowsize;

public:
	typedef enum { 
		begin = 0,
		cur,
		end
	} seekp;

	memstream() : m_glowsize(MEM_GROWSIZE), m_allocator(NULL), m_baseptr(NULL), m_readptr(NULL), m_offset(0), m_memsize(0) { }

	memstream(size_t size) : m_glowsize(MEM_GROWSIZE), m_offset(0)
	{
		m_allocator = m_baseptr = m_readptr = (uint8_t*)malloc(size);
		m_memsize = size;
	}

	memstream(void* pmem, size_t size) : m_glowsize(MEM_GROWSIZE), m_allocator(NULL), m_offset(0)
	{
		m_baseptr = m_readptr = (uint8_t*)pmem;
		m_memsize = size;
	}

	memstream(const wchar_t* filename) : m_glowsize(MEM_GROWSIZE), m_offset(0)
	{
		m_allocator = m_baseptr = m_readptr = (uint8_t*)file2mem(filename, &m_memsize);
	}	

	memstream(memstream* src) : m_glowsize(MEM_GROWSIZE), m_offset(0), m_memsize(0)
	{
		m_allocator = m_baseptr = m_readptr = (uint8_t*)malloc(src->m_memsize);
		if (m_allocator) {
			m_memsize = src->m_memsize;
			memcpy(m_baseptr, src->m_baseptr, m_memsize);
			m_offset = src->tellp();
			m_readptr = m_baseptr + m_offset;
		}
	}

	memstream(HGLOBAL hmem) : m_glowsize(MEM_GROWSIZE), m_allocator(NULL), m_baseptr(NULL), m_readptr(NULL), m_memsize(0), m_offset(0)
	{
		void* pv = GlobalLock(hmem);
		if (pv) {
			m_memsize = GlobalSize(hmem);
			m_allocator = m_baseptr = m_readptr = (uint8_t*)malloc(m_memsize);
			if (m_allocator) {
				memcpy(m_baseptr, pv, m_memsize);
			}
		}
		GlobalUnlock(hmem);
	}

	~memstream()
	{
		if (m_allocator) free(m_allocator);
	}


	long seek(long offset, memstream::seekp origin)
	{
		switch (origin) {
		case begin:
			if (offset < 0) offset = 0;
			if (offset > m_memsize) offset = (long)m_memsize;
			m_offset = offset;
			break;
		case cur:
			if (offset < 0) {
				if ((size_t)(-offset) > m_offset) m_offset = 0;
				else m_offset -= -offset;
			}
			else {
				if ((size_t)offset > m_memsize - m_offset) m_offset = m_memsize;
				else m_offset += offset;
			}
			break;
		case end:
			if (offset > 0) offset = 0;
			if ((size_t)(-offset) > m_memsize) offset = -(long)m_memsize;
			m_offset = m_memsize + offset;
			break;
		}
		m_readptr = m_baseptr + m_offset;
		return (long)m_offset;
	}

	size_t tellp() const
	{
		return m_offset;
	}

	size_t freespace() const
	{
		return (m_memsize - m_offset);
	}

	uint8_t* detach(size_t* pmemsize)
	{
		uint8_t* memptr = m_baseptr;
		if (pmemsize)
			*pmemsize = m_memsize;
		delete this;
		return memptr;
	}

	size_t size() const
	{
		return m_memsize;
	}

	HGLOBAL memhandle()
	{
		HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, m_memsize);
		if (hmem) {
			void* pmem = GlobalLock(hmem);
			if (pmem) {
				memcpy(pmem, m_baseptr, m_memsize);
				GlobalUnlock(hmem);
			}
		}
		return hmem;
	}

	uint8_t* dataptr()
	{
		return m_baseptr;
	}

	uint8_t* dataptr_direct(size_t offset, size_t size, uint32_t* available)
	{
		if (offset < m_memsize - 1) {
			if (m_memsize >= offset + size)
				*available = (uint32_t)size;
			else
				*available = (uint32_t)(m_memsize - offset);
			return m_baseptr + offset;
		}
		*available = 0;
		return NULL;
	}

	bool glow(size_t glow)
	{
		if (!m_allocator) return false;

		size_t sizeNew = m_memsize + glow;
		uint8_t* memNew = (uint8_t*)realloc(m_allocator, sizeNew);
		if (memNew) {
			m_allocator = m_baseptr = memNew;
			m_readptr = m_baseptr + m_offset;
			m_memsize = sizeNew;
			return true;
		}
		return false;
	}

	uint8_t getc()
	{
		if (m_memsize > m_offset) {
			uint8_t ch = *m_readptr++;
			m_offset++;
			return ch;
		}
		return 0;
	}

	uint8_t getc_ns()
	{
		return (m_memsize > m_offset) ? *m_readptr : 0;
	}

	uint16_t getw()
	{
		if (m_memsize > m_offset + 1) {
			uint16_t w = *((uint16_t*)m_readptr);
			m_readptr += 2;
			m_offset += 2;
			return w;
		}
		return 0;
	}

	uint16_t getw_ns()
	{
		return (m_memsize > m_offset + 1) ? *((uint16_t*)m_readptr) : 0;
	}

	uint32_t getl()
	{
		if (m_memsize > m_offset + 3) {
			uint32_t ul = *((uint32_t*)m_readptr);
			m_readptr += sizeof(uint32_t);
			m_offset += sizeof(uint32_t);
			return ul;
		}
		return 0;
	}

	uint16_t msb1streadshort()
	{
		uint8_t cb[2];
		uint16_t value;
		if (read(cb, 2) == 2) {
			value = (uint16_t)(cb[0] << 8);
			value |= (uint16_t)(cb[1]);
			return value;
		}
		return (uint16_t)(~0);
	}

	unsigned long msb1streadlong()
	{
		uint8_t cb[4];
		unsigned long value;
		if (read(cb, 4) == 4) {
			value = (unsigned long)cb[0] << 24;
			value |= (unsigned long)cb[1] << 16;
			value |= (unsigned long)cb[2] << 8;
			value |= (unsigned long)cb[3];
			return value;
		}
		return (unsigned long)(~0);
	}

	int read(void* pv, size_t size)
	{
		size_t r = (m_memsize >= m_offset + size) ? size : m_memsize - m_offset;
		if (r > 0) {
			memcpy(pv, m_readptr, r);
			m_readptr += r;
			m_offset += r;
		}
		return (int)r;
	}

	bool eof() const
	{
		return (m_memsize <= m_offset) ? true : false;
	}

	bool putc(uint8_t ch)
	{
		if (freespace() < 1) {
			if (!glow(m_glowsize))
				return false;
		}

		*m_readptr++ = ch;
		m_offset++;
		return true;
	}

	BOOL putw(uint16_t ch)
	{
		if (freespace() < 2) {
			if (!glow(m_glowsize))
				return FALSE;
		}

		*((uint16_t*)m_readptr) = ch;
		m_readptr += 2;
		m_offset += 2;
		return TRUE;
	}

	size_t msb1stwritelong(unsigned long value)
	{
		uint8_t cb[4] = { 0 };
		cb[0] = (uint8_t)((value & 0xff000000) >> 24);
		cb[1] = (uint8_t)((value & 0xff0000) >> 16);
		cb[2] = (uint8_t)((value & 0xff00) >> 8);
		cb[3] = (uint8_t)(value & 0xff);
		return write(cb, 4);
	}

	size_t msb1stwriteshort(unsigned long value)
	{
		uint8_t cb[2] = { 0 };
		cb[0] = (uint8_t)((value) >> 8);
		cb[1] = (uint8_t)(value & 0xff);
		return write(cb, 2);
	}

	size_t write(void* lpv, size_t size)
	{
		if (freespace() < size && !glow(max(size, m_glowsize))) {
			return 0;
		}

		memcpy(m_readptr, lpv, size);
		m_readptr += size;
		m_offset += size;
		return size;
	}

	uint16_t getw_le() { return getw(); }
	uint32_t getl_le() { return getl(); }	
	uint16_t getw_be() { return msb1streadshort(); }
	uint32_t getl_be() { return msb1streadlong(); }
};

#endif			// __MEMSTREAM_H_
