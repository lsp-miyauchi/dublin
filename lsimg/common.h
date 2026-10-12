////////////////////////////////////////////////////////////////
// common.h

#include "rects.h"
#include "dib_sup.h"

extern HINSTANCE ghInst;
extern PROGRESSCALLBACK gpfnCallback;

#define MSGOUT(a)				(MessageBox(GetFocus(),(a),"debug",MB_OK))

#ifndef	WIDTHBYTES
	#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)
#endif

#define PALVERSION			0x300

#define UPSHIFTED(x)			((int)((x) * (1L << 14) + 0.5))
#define UPSHIFT(x)				((int)(x) << 14)
#define DOWNSHIFT(x)			(((int)((x) + (1L << 13))) >> 14)
#define MAXRGB					255
#define FILTERWIDTH				2.0
#define MAXVALUE(a,b)			(((a) > (b)) ? (a) : (b))

#pragma pack(1)

typedef struct {
	char cbY;
	char cbCb;
	char cbCr;
} YCCQUAD, LPYCCQUAD;

typedef struct {
	int* Cr_r_tab;		/* => table for Cr to R conversion */
	int* Cb_b_tab;		/* => table for Cb to B conversion */
	long* Cr_g_tab;		/* => table for Cr to G conversion */
	long* Cb_g_tab;		/* => table for Cb to G conversion */
} YCCT;
typedef YCCT* PYCCT;

typedef struct {
	int pixel;
	double weight;
} CONTRIBINFO;

typedef struct {
	int columns;
	int rows;
	RGBPixel* pixels;
	int packets;
} Image;

typedef struct {
	double x;
	double y;
} DBLPOINT;

typedef struct {
	int width;
	int height;
	int x;
	int y;
} RECTINFO;

#pragma pack()

class waitcursor
{
public:
	waitcursor() { m_hcur = SetCursor(LoadCursor(NULL, IDC_WAIT)); }
	~waitcursor() { SetCursor(m_hcur); }
	HCURSOR m_hcur;
};

#define DOWNSCALE(a)		(((WORD)(a)) >> 8)
#define UPSCALE(a)			(((WORD)(a)) * 257)
#define ADJRGB(a)			(min(255, max(0, (int)(a))))
#define ADJRGB16(a)			(min(65535, max(0, (int)(a))))
#define ADJHUE(a)			(min(359, max(0, (int)(a))))
#define ADJSAT(a)			(min(228, max(0, (int)(a))))
#define PALLOC(a, b)		(GlobalAllocPtr(GHND, (a) * (b)))
#define PFREE(a)			(GlobalFreePtr((a)))

#define M_PI				(3.14159265358979323846)
#define M_PI_2				(1.57079632679489661923)
#define M_PI_4				(0.78539816339744830962)
#define INTENSITY(r,g,b)	((DWORD)((DWORD)(r)*77+(DWORD)(g)*150+(DWORD)(b)*29)>>8)
#define	DEG2RAD(x)			((x) * M_PI / 180.0)
#define APPLYALPHA(a,b)		((BYTE)((double)(a)*(double)(b)/255.0))

BOOL DoProgressCallback(int msg, int param);
long* rgb_ycc_start();
void rgb_ycc_finish(long* ctab);
void rgb_ycc_convert(long* ctab, COLORREF rgb, YCCQUAD* pyccq);
BOOL __fastcall GetFileData(LPCTSTR pszName, WIN32_FIND_DATA* pwfd);
void msb1stwritelong(HANDLE hf, ULONG value);
void msb1stwriteshort(HANDLE hf, UINT value);
void __cdecl MsgBox(LPCSTR lpszFormat, ...);
void DumpBox(LPBYTE pDump, int len);
int __fastcall mixcolor(int src, int dst, int dst_opaque);
BOOL __fastcall SaveMemoryToFile(LPCTSTR pszName, void* pdata, size_t size);
BOOL __fastcall SaveMemToFileAndWep(LPCTSTR pszName, HANDLE hmem, DWORD dwSize);

DWORD imgGetVersion();
inline bool isVer7() { return (imgGetVersion() == 0x700); }


//// xform_color.cpp
void __fastcall rgb_to_hls(int *r, int *g, int *b);
void __fastcall hls_to_rgb (int *h, int *l, int *s);
void __fastcall rgb_to_hsv(int *r, int *g, int *b);
void __fastcall hsv_to_rgb(int *h, int *s, int *v);


////////////////////////////////////////////////////////////
// inline functions

_inline int do_mix(int src, int dst, int opaque)
{
	if (opaque == 0) return src;
	if (opaque == 255) return dst;
	return ADJRGB((src * (255 - opaque) + dst * opaque) / 255);
}

_inline float CalcScaleRatio(int width, int height, int max_scaled_width, int max_scaled_height)
{
	float xr = (float)max_scaled_width / (float)width;
	float yr = (float)max_scaled_height / (float)height;
	return min(xr, yr);
}

////////////////////////////////////////////////////////////
//

typedef struct {
	BYTE red;
	BYTE green;
	BYTE blue;
} RGB_PIXEL;
typedef struct {
	BYTE blue;
	BYTE green;
	BYTE red;
} BGR_PIXEL;

RGB_PIXEL* __fastcall RGBAtoRGBPixels(const RGBA* srcp, size_t size, bool applyAlpha = false);
BGR_PIXEL* __fastcall RGBAtoBGRPixels(const RGBA* srcp, size_t size, bool applyAlpha = false);
