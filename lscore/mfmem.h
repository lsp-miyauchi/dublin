//{{NO_DEPENDENCIES}}
//
// FUNCTION PROTOTYPE DEFINITION
//


#ifndef __MFMEM_INCLUDE
#define __MFMEM_INCLUDE

////////////////////////////////////////////////////////////////
// local memoryfile
//

class memstream
{
public:
	memstream(HGLOBAL hmem, DWORD size = 0);
	~memstream();
	BYTE getc();
	WORD getw();
	int read(void* pv, DWORD size);
	long seek(long lOffset, int nOrign);

protected:
	HGLOBAL m_hmem;
	BYTE* m_pTop;
	BYTE* m_pRead;
	DWORD m_dwSize;
	DWORD m_dwOffset;
};

#endif		// __MFMEM_INCLUDE
