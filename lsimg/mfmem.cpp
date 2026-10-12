#define     STRICT      // enable strict type checking

#include "stdafx.h"

#ifdef __MFMEM_H__

#define GLOWSIZE				0x10000

static MemStream* _alloc_stream_struct(size_t size)
{
	MemStream* pmf = (MemStream*)malloc(sizeof(MemStream));
	if (pmf) {
		memset(pmf, 0, sizeof(MemStream));
		if (size)
			pmf->m_hmem = GlobalAlloc(GMEM_MOVEABLE, size);
	}
	return pmf;
}

static HGLOBAL readfileall(LPCTSTR pFile, LPDWORD pdwRead)
{
	HGLOBAL hmem = NULL;
	HANDLE hf = CreateFile(pFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		size_t size = GetFileSize(hf, NULL);
		hmem = GlobalAlloc(GMEM_MOVEABLE, size + 16);
		if (hmem) {
			void* pv = GlobalLock(hmem);
			BOOL bOK = ReadFile(hf, pv, size, pdwRead, NULL);		
			GlobalUnlock(hmem);	
			if (!bOK) {
				*pdwRead = 0;
				GlobalFree(hmem);
				hmem = NULL;
			}
		}
		CloseHandle(hf);
	}
	return hmem;
}


static BOOL _mfglow(MemStream* pmf, size_t glow)
{
	GlobalLock(pmf->m_hmem);

	size_t nsize = pmf->m_size + glow;
	HGLOBAL hNew = GlobalReAlloc(pmf->m_hmem, nsize, GMEM_MOVEABLE);
	if (hNew) {
		pmf->m_size = nsize;
		pmf->m_hmem = hNew;
		pmf->m_baseptr = (uint8_t*)GlobalLock(hNew);
		pmf->m_readptr = pmf->m_baseptr + pmf->m_offset;
		return TRUE;
	}

	pmf->m_baseptr = (uint8_t*)GlobalLock(pmf->m_hmem);
	pmf->m_readptr = pmf->m_baseptr + pmf->m_offset;
	return FALSE;
}

/**
* @brief Create a memory stream from a file.
* * @param fname File name.
* * @return Pointer to the MemStream structure.
*/
MemStream* _mfcreate(const wchar_t* fname)
{
	MemStream* pmf = _alloc_stream_struct(0);
	if (!pmf)
		return NULL;

	DWORD size;
	pmf->m_hmem = readfileall(fname, &size);
	if (!pmf->m_hmem) {
		free(pmf);
		return NULL;
	}
	pmf->m_size = size;

	pmf->m_baseptr = pmf->m_readptr = (uint8_t*)GlobalLock(pmf->m_hmem);
	if (!pmf->m_readptr) {
		GlobalFree(pmf->m_hmem);
		free(pmf);
		return NULL;
	}
	return pmf;
}

/**
* @brief Create a memory stream by size.
* * @param size memory size.
* * @return Pointer to the MemStream structure.
*/
MemStream* _mfcreate(size_t m_size)
{
	MemStream* pmf = _alloc_stream_struct(m_size);
	if (!pmf)
		return NULL;

	if (!pmf->m_hmem) {
		free(pmf);
		return NULL;
	}

	pmf->m_size = m_size;
	pmf->m_baseptr = pmf->m_readptr = (uint8_t*)GlobalLock(pmf->m_hmem);
	if (!pmf->m_readptr) {
		GlobalFree(pmf->m_hmem);
		free(pmf);
		return NULL;
	}
	return pmf;
}

/**
* 
*/
MemStream* _mfcreate(HGLOBAL hmem)
{
	if (!hmem)
		return NULL;

	size_t size = GlobalSize(hmem);
	MemStream* pmf = _alloc_stream_struct(size);
	if (!pmf)
		return NULL;
	if (!pmf->m_hmem) {
		free(pmf);
		return NULL;
	}

	pmf->m_size = size;
	pmf->m_baseptr = pmf->m_readptr = (uint8_t*)GlobalLock(pmf->m_hmem);
	if (!pmf->m_readptr) {
		GlobalFree(pmf->m_hmem);
		free(pmf);
		return NULL;
	}
	void* pv = GlobalLock(hmem);
	memcpy(pmf->m_baseptr, pv, size);
	GlobalUnlock(hmem);

	return pmf;
}

HGLOBAL _mfdetach(MemStream* pmf, size_t* pmemsize)
{
	if (!pmf)
		return NULL;

	GlobalUnlock(pmf->m_hmem);
	HGLOBAL hmem = pmf->m_hmem;
	if (pmemsize)
		*pmemsize = pmf->m_offset;
	free(pmf);

	return hmem;
}

void _mfclose(MemStream* pmf)
{
	if (pmf) {
		if (pmf->m_hmem) {
			if (pmf->m_baseptr)
				GlobalUnlock(pmf->m_hmem);
			GlobalFree(pmf->m_hmem);
		}
		free(pmf);
	}
}

uint8_t _mfgetc(MemStream* pmf)
{
	if (pmf && pmf->m_size > pmf->m_offset) {
		BYTE ch = *pmf->m_readptr++;
		pmf->m_offset++;
		return ch;
	}
	return 0;
}

uint8_t _mfgetc_ns(MemStream* pmf)
{
	if (pmf && pmf->m_size > pmf->m_offset)
		return (*pmf->m_readptr);
	return 0;
}

uint16_t _mfgetw(MemStream* pmf)
{
	if (pmf && pmf->m_size > pmf->m_offset + 1) {
		uint16_t w = *((uint16_t*)pmf->m_readptr);
		pmf->m_readptr += sizeof(uint16_t);
		pmf->m_offset += sizeof(uint16_t);
		return w;
	}
	return 0;
}

uint32_t _mfgetl(MemStream* pmf)
{
	if (pmf && pmf->m_size > pmf->m_offset + 3) {
		uint32_t dw = *((uint32_t*)pmf->m_readptr);
		pmf->m_readptr += sizeof(uint32_t);
		pmf->m_offset += sizeof(uint32_t);
		return dw;
	}
	return 0;
}

uint16_t _mfgetw_ns(MemStream* pmf)
{
	if (pmf && pmf->m_size > pmf->m_offset + 1)
		return *((uint16_t*)pmf->m_readptr);
	return 0;
}

int _mfread(MemStream* pmf, void* lpv, size_t size)
{
	if (!pmf)
		return 0;

	int nRead;
	if (pmf->m_size >= pmf->m_offset + size)
		nRead = (int)size;
	else
		nRead = (int)(pmf->m_size - pmf->m_offset);

	if (nRead > 0) {
		memcpy(lpv, pmf->m_readptr, nRead);
		pmf->m_readptr += nRead;
		pmf->m_offset += nRead;
	}
	return nRead;
}

void* _mfdataptr(MemStream* pmf, size_t offset, size_t size, uint32_t* canRead)
{
	size_t cr = 0;
	if (pmf) {
		cr = max(0, min(pmf->m_size - offset, size));
	}
	*canRead = (uint32_t)cr;
	return (cr > 0) ? (void*)(pmf->m_baseptr + offset) : NULL;
}

long _mfseek(MemStream* pmf, long offset, int origin)
{
	if (!pmf || !pmf->m_hmem || !pmf->m_readptr)
		return 0;

	switch (origin) {
		case FILE_CURRENT:
			offset += (long)pmf->m_offset;
			if (offset < 0)
				offset = 0;
			else if (offset > (long)pmf->m_size)
				offset = (long)pmf->m_size;
			// fall through

		case FILE_BEGIN:
			pmf->m_readptr = pmf->m_baseptr;
			pmf->m_offset = 0;
			if (offset <= 0)
				break;

			if (offset > (long)pmf->m_size)
				offset = (long)pmf->m_size;
			pmf->m_offset = offset;
			pmf->m_readptr += offset;
			break;

		case FILE_END:
			pmf->m_offset = pmf->m_size;
			pmf->m_readptr = pmf->m_baseptr + pmf->m_size;
			if (offset >= 0)
				break;

			offset *= (-1);
			if (offset > (long)pmf->m_size)
				offset = (long)pmf->m_size;
			pmf->m_offset -= offset;
			pmf->m_readptr -= offset;
			break;

		default:
			break;
	}
	return (long)pmf->m_offset;
}

BOOL _mfputc(MemStream* pmf, BYTE ch)
{
	if (!pmf)
		return 0;

	if (pmf->freespace() < 1) {
		if (!_mfglow(pmf, GLOWSIZE))
			return FALSE;
	}

	*pmf->m_readptr++ = ch;
	pmf->m_offset++;
	return TRUE;
}

BOOL _mfputw(MemStream* pmf, WORD ch)
{
	if (!pmf)
		return 0;

	if (pmf->freespace() < 2) {
		if (!_mfglow(pmf, GLOWSIZE))
			return FALSE;
	}

	*((uint16_t*)pmf->m_readptr) = ch;
	pmf->m_readptr += sizeof(uint16_t);
	pmf->m_offset += sizeof(uint16_t);
	return TRUE;
}

size_t _mfwrite(MemStream* pmf, void* lpv, size_t size)
{
	if (!pmf)
		return 0;

	if (pmf->freespace() < size) {
		if (!_mfglow(pmf, max(size, GLOWSIZE)))
			return 0;
	}

	memcpy(pmf->m_readptr, lpv, size);
	pmf->m_readptr += size;
	pmf->m_offset += size;

	return size;
}

USHORT _mfgetw_be(MemStream* pmf)
{
	uint8_t cb[2];
	if (_mfread(pmf, cb, 2) == 2) {
		return (uint16_t)(cb[0] << 8) | (uint16_t)(cb[1]);
	}
	return (uint16_t)(~0);
}

unsigned long _mfgetl_be(MemStream* pmf)
{
	uint8_t cb[4];
	if (_mfread(pmf, cb, 4) == 4) {
		return (unsigned long)(cb[0]) << 24 | (unsigned long)(cb[1]) << 16 | (unsigned long)(cb[2]) << 8 | (unsigned long)(cb[3]);
	}
	return (unsigned long)(~0);
}

size_t _mfputl_be(MemStream* pmf, unsigned long value)
{
	uint8_t cb[4] = { 0 };
	cb[0] = (uint8_t)((value & 0xff000000) >> 24);
	cb[1] = (uint8_t)((value & 0xff0000) >> 16);
	cb[2] = (uint8_t)((value & 0xff00) >> 8);
	cb[3] = (uint8_t)(value & 0xff);
	return _mfwrite(pmf, cb, 4);
}

size_t _mfputw_be(MemStream* pmf, unsigned long value)
{
	uint8_t cb[2] = { 0 };
	cb[0] = (uint8_t)((value) >> 8);
	cb[1] = (uint8_t)(value & 0xff);
	return _mfwrite(pmf, cb, 2);
}

#endif		// __MFMEM_H__
