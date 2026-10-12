//////////////////////////////////////////////////////////
// dib_sup.h


HANDLE dib_create(DWORD dwWidth, DWORD dwHeight, DWORD dwBits);
int dib_numcolors(void* pv);
int dib_palsize(void* pv);
HANDLE dib_createfrombitmap(HBITMAP hbm, HPALETTE hpal);
HPALETTE dib_createpalette(HANDLE hdib);

_inline BYTE* dib_findbits(void* pv)
	{ return ((BYTE*)pv + *(DWORD*)pv + dib_palsize(pv)); }

