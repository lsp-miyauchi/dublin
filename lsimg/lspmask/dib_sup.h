//////////////////////////////////////////////////////////
// dib_sup.h

HDIB dib_create(DWORD dwWidth, DWORD dwHeight, DWORD dwBits);
HDIB dib_copy(HDIB hsrc);
void* dib_copyptr(void* pSrc);
int dib_numcolors(void* pv);
BOOL dib_getinfo(HDIB hdib, int* dx, int* dy, int* colors, int* bits);
HDIB dib_createsolid(DWORD dwWidth, DWORD dwHeight, COLORREF rgb);
RGBPK* dib_getpacket(HDIB hdib, int sx, int sy, int dx, int dy);
void dib_setpacket(HDIB hdib, RGBPK* pkt, int sx, int sy, int dx, int dy);
BOOL dib_getplane(HDIB hDIB, UINT nPlane, LPBYTE pPlane);
BOOL dib_setplane(HDIB hDIB, UINT nPlane, LPBYTE pPlane);
BOOL dib_planeblt(BYTE* pDst, int sxDst, int syDst, int cxDst, int cyDst, BYTE* pSrc, int sxSrc, int sySrc, int cxSrc, int cySrc, int xDelta, int yDelta);
HDIB dib_createfrombitmap(HBITMAP hbm, HPALETTE hpal);

_inline int dib_palsize(void* pdib)
	{ return (dib_numcolors(pdib) * sizeof(RGBQUAD)); }
_inline BYTE* dib_findbits(void* pv)
	{ return ((BYTE*)pv + *(DWORD*)pv + dib_palsize(pv)); }
_inline RGBPK* dib_makepacket(int width, int height)
	{ return (RGBPK*)GlobalAllocPtr(GHND, width * height * sizeof(RGBPK)); }
_inline void dib_freepacket(RGBPK* pkt)
	{ GlobalFreePtr(pkt); }

