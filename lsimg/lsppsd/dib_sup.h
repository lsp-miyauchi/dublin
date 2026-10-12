//////////////////////////////////////////////////////////
// dib_sup.h

#ifndef	HDIB
	#define HDIB HANDLE
#endif

typedef struct tagRGBPK {
	BYTE blue;
	BYTE green;
	BYTE red;
} RGBPK, *LPRGBPK;

#ifndef	WIDTHBYTES
	#undef WIDTHBYTES
	#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)
#endif

#define PALVERSION			0x300
#define ADJRGB(a)			(min(255, max(0, (int)(a))))

enum {
	PLANE_RED,
	PLANE_GREEN,
	PLANE_BLUE,
	PLANE_INDEX,
	PLANE_ALPHA,
};

HDIB __fastcall dib_create(DWORD dwWidth, DWORD dwHeight, DWORD dwBits);
HDIB __fastcall dib_copy(HDIB hsrc);
void* __fastcall dib_copyptr(void* pSrc);
int __fastcall dib_numcolors(void* pv);
BOOL __fastcall dib_getinfo(HDIB hdib, int* dx, int* dy, int* colors, int* bits);
HDIB __fastcall dib_createsolid(DWORD dwWidth, DWORD dwHeight, COLORREF rgb);
RGBPK* __fastcall dib_getpacket(HDIB hdib, int sx, int sy, int dx, int dy);
void __fastcall dib_setpacket(HDIB hdib, RGBPK* pkt, int sx, int sy, int dx, int dy);
BOOL __fastcall dib_getplane(HDIB hDIB, UINT nPlane, LPBYTE pPlane);
BOOL __fastcall dib_setplane(HDIB hDIB, UINT nPlane, LPBYTE pPlane);
BOOL __fastcall dib_planeblt(BYTE* pDst, int sxDst, int syDst, int cxDst, int cyDst, BYTE* pSrc, int sxSrc, int sySrc, int cxSrc, int cySrc, int xDelta, int yDelta);
HDIB __fastcall dib_createfrombitmap(HBITMAP hbm, HPALETTE hpal);

_inline int __fastcall dib_palsize(void* pdib)
	{ return (dib_numcolors(pdib) * sizeof(RGBQUAD)); }
_inline BYTE* __fastcall dib_findbits(void* pv)
	{ return ((BYTE*)pv + *(DWORD*)pv + dib_palsize(pv)); }
_inline RGBPK* __fastcall dib_makepacket(int width, int height)
	{ return (RGBPK*)GlobalAllocPtr(GHND, width * height * sizeof(RGBPK)); }
_inline void __fastcall dib_freepacket(RGBPK* pkt)
	{ GlobalFreePtr(pkt); }

