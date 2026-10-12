// Common/StdOutStream.h

#ifndef __MEMOUTSTREAM_H
#define __MEMOUTSTREAM_H

#include <stdio.h>

#include "Types.h"

class CMemOutStream
{
private:
	BYTE* ms_allocator;
	BYTE* ms_top;
	BYTE* ms_ptr;
	size_t ms_size;
	size_t ms_offset;
	size_t m_glowsize;
public:
	CMemOutStream(size_t size = 0)
	{
		m_glowsize = 4096;
		ms_offset = 0;
		ms_allocator = ms_top = ms_ptr = (BYTE*)malloc(size);
		ms_size = size;
	}
	~CMemOutStream()
	{
		if (ms_allocator) free(ms_allocator);
	}
private:
	size_t spacesize()
	{
		return (ms_size - ms_offset);
	}
	BOOL glow(size_t glow)
	{
		if (!ms_allocator) return FALSE;

		size_t sizeNew = ms_size + glow;
		BYTE* memNew = (BYTE*)realloc(ms_allocator, sizeNew);
		if (memNew) {
			ms_allocator = ms_top = memNew;
			ms_ptr = ms_top + ms_offset;
			ms_size = sizeNew;
			return TRUE;
		}
		return FALSE;
	}
	BOOL putc(BYTE ch)
	{
		if (spacesize() < 1) {
			if (!glow(m_glowsize))
				return FALSE;
		}
		*ms_ptr++ = ch;
		ms_offset++;
		return TRUE;
	}
	size_t write(void* pv, size_t)
	{
		if (spacesize() < size + 1) {
			if (!glow(max(size, m_glowsize)))
				return 0;
		}
		memcpy(ms_ptr, pv, size);
		ms_ptr += size;
		ms_offset += size;
		return size;
	}
public:
	BYTE* getdataptr() { return m_top; }
	size_t offsetsize() { return ms_offset; }
	CMemOutStream & operator<<(CMemOutStream & (* aFunction)(CMemOutStream  &));
	CMemOutStream & operator<<(const char *string);
	CMemOutStream & operator<<(const wchar_t *string);
	CMemOutStream & operator<<(char c);
	CMemOutStream & operator<<(int number);
	CMemOutStream & operator<<(UInt64 number);
};

CMemOutStream& endl(CMemOutStream& outStream);

extern CMemOutStream g_MemOut;

#endif
