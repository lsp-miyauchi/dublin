////////////////////////////////////////////////////////////////
//

#pragma once

#ifndef __MFMEM_H__
#define __MFMEM_H__

////////////////////////////////////////////////////////////////
// local memoryfile
//

typedef struct tagMemStream {
	HGLOBAL m_hmem;
	uint8_t* m_baseptr;
	uint8_t* m_readptr;
	size_t m_offset;
	size_t m_size;

	size_t size() const { return m_size; }
	size_t freespace() const { return (m_size - m_offset); }
	long tellp() const { return (long)m_offset; }
	bool eof() const { return (m_size <= m_offset); }

} MemStream;


////////////////////////////////////////////////////////////////
// function prototypes
//


MemStream* _mfcreate(const wchar_t* pszName);
MemStream* _mfcreate(size_t memsize);
MemStream* _mfcreate(HGLOBAL hmem);

HGLOBAL _mfdetach(MemStream* pmf, size_t* pmemsize);
void _mfclose(MemStream* lp);
uint8_t _mfgetc(MemStream* lp);
uint8_t _mfgetc_ns(MemStream* lp);
uint16_t _mfgetw(MemStream* lp);
uint16_t _mfgetw_ns(MemStream* lp);
uint32_t _mfgetl(MemStream* pmf);
int _mfread(MemStream* lp, void* lpv, size_t size);
void* _mfdataptr(MemStream* lp, size_t offset, size_t size, uint32_t* canRead);
BOOL _mfputc(MemStream* lp, BYTE ch);
BOOL _mfputw(MemStream* lp, WORD ch);
size_t _mfwrite(MemStream* lp, void* lpv, size_t size);
long _mfseek(MemStream* lp, long lOffset, int nOrign);
unsigned long _mfgetl_be(MemStream* lp);
uint16_t _mfgetw_be(MemStream* lp);
size_t _mfputl_be(MemStream* pmf, unsigned long value);
size_t _mfputw_be(MemStream* pmf, unsigned long value);

#define _mfgetw_le			_mfgetw
#define _mfgetl_le			_mfgetl



#endif		// __MFMEM_H__
