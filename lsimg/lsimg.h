////////////////////////////////////////////////////////////////
// lsimg.h

#pragma once
#ifndef __LSIMG_H__
#define __LSIMG_H__

#ifndef _COUNTOF
#define _COUNTOF(a)		(sizeof(a)/sizeof(a[0]))
#endif

#include "lsmif\lsmif.h"
#include "imagetypes.h"

////////////////////////////////////////////////////////////////
// Handle to a DIB
//

#ifndef	HDIB
	#define HDIB HANDLE
#endif

enum {
	IMG_DITHER_NO,
	IMG_DITHER_FS,
	IMG_DITHER_NODE,
	IMG_DITHER_FSLOWBLEED,
	IMG_DITHER_FIXED
};

enum {
	FPXCOMPRESS_NONE                = 0,	// no compression used.
	FPXCOMPRESS_SINGLE_COLOR        = 1,    // use single color compression
	FPXCOMPRESS_JPEG_UNSPECIFIED    = 2,    // let the toolkit pick the tables.
	FPXCOMPRESS_JPEG_BY_QUALITY     = 3,    // will specify the quality.
	FPXCOMPRESS_JPEG_BY_TABLE_GROUP = 4     // will specify the table group to use
};

enum {
	IMG_ZOOM_BOX,
	IMG_ZOOM_TRIANGLE,
	IMG_ZOOM_HERMITE,
	IMG_ZOOM_HANNING,
	IMG_ZOOM_HAMMING,
	IMG_ZOOM_BLACKMAN,
	IMG_ZOOM_GAUSSIAN,
	IMG_ZOOM_QUADRATIC,
	IMG_ZOOM_CUBIC,
	IMG_ZOOM_CATROM,
	IMG_ZOOM_MITCHELL,
	IMG_ZOOM_LANCZOS,
	IMG_ZOOM_BESSEL,
	IMG_ZOOM_SINC,
	IMG_ZOOM_CUSTOM,
	IMG_ZOOM_LAST
};

//@ #define WC_MIFVIEW				(_T("mifview"))
//@ #define WC_MIFSHADE				(_T("mifshade"))
#define WC_MIFGRAPH				(_T("mifgraph"))
//@ #define WC_MIFGRADATION			(_T("mifgradation"))
//@ #define WC_MIFDIBVIEW			(_T("mifdibview"))
#define WC_PREVIEW				(_T("mifpreview"))
#define IVM_SETDIB				(WM_USER + 0x101)
#define IVM_SETBKCOLOR			(WM_USER + 0x102)
#define IVM_SETRANGE			(WM_USER + 0x103)
#define IVM_SETPOS				(WM_USER + 0x104)
#define IVM_UPDATEEFFECT		(WM_USER + 0x105)
#define IVM_SETPALETTE			(WM_USER + 0x106)
//@ #define DVM_SETDIB				(WM_USER + 0x107)
//@ #define DVM_SETZOOM				(WM_USER + 0x108)
//@ #define DVM_SETBKCOLOR			(WM_USER + 0x109)
#define IVM_SETWHOLEVIEW		(WM_USER + 0x10a)
#define IVS_EFFECTCALLBACK		0x00000001L

//@ #define MSM_SETTEXTCOLOR		(WM_USER + 0x200)
//@ #define MSM_SETBKCOLOR			(WM_USER + 0x201)

//@ #define MSS_LEFT				0x00000000
//@ #define MSS_CENTER				0x00000001
//@ #define MSS_RIGHT				0x00000002
//@ #define MSS_TYPEMASK			0x0000000F
//@ #define MSS_SHADELEFT			0x00000000
//@ #define MSS_SHADERIGHT			0x00000010
//@ #define MSS_SHADETOP			0x00000020
//@ #define MSS_SHADEBOTTOM			0x00000030
//@ #define MSS_SHADEMASK			0x000000F0

#define MGM_SETTONE				(WM_USER + 0x210)
//@ #define GDM_SETCOLORS			(WM_USER + 0x220)

#pragma pack(1)

typedef struct tagRGBPixel {
	uint8_t blue;
	uint8_t green;
	uint8_t red;
} RGBPixel;

typedef struct tagCMYKPixel {
	uint8_t cyan;
	uint8_t magenta;
	uint8_t yellow;
	uint8_t black;
} CMYKPixel;

typedef struct tagMIFPixel {
	uint8_t red;
	uint8_t blue;
	uint8_t green;
	uint8_t alpha;
	int index;
} MIFPixel;

typedef struct tagMIFImage {
	int width;
	int height;
	int bits;
	int colors;
	int packets;
	MIFPixel colormap[256];
	MIFPixel pixels[1];
} MIFImage;

typedef struct tagMIFLAYER {
	int x;
	int y;
	int blendtype;
	uint8_t opacity;
	uint8_t flags;
	MIFImage* image;
} MIFLAYER;


#define MAXTONEPOINT	8

typedef struct tagTONE {
	int count;
	int ref[MAXTONEPOINT];
	int val[MAXTONEPOINT];
} TONE, *LPTONE;

typedef struct tagTONEINFO {
	TONE rgb;
	TONE red;
	TONE blue;
	TONE green;
} TONEINFO, *LPTONEINFO;

typedef struct tagMV_GETEFFECT {
	NMHDR hdr;
	HDIB hdibSrc;
	HDIB hdibDst;
} MV_GETEFFECT, *LPMV_GETEFFECT;
#define MVN_GETEFFECT		1

typedef struct tagMV_LBUTTONDOWN {
	NMHDR hdr;
	UINT fwKeys;
	POINT ptClient;
	POINT ptPicture;
} MV_LBUTTONDOWN, *LPMV_LBUTTONDOWN;
#define MVN_LBUTTONDOWN		2

typedef struct tagMV_CHANGEVIEW {
	NMHDR hdr;
	POINT ptTopLeft;
} MV_CHANGEVIEW, *LPMV_CHANGEVIEW;
#define MVN_CHANGEVIEW		3

typedef struct tagMG_SELCHANGE {
	NMHDR hdr;
	int ref;
	int val;
} MG_SELCHANGE, *LPMG_SELCHANGE;
#define MGN_SELCHANGE		1


typedef struct tagGIF_CPARAMS {
	BOOL gif87a;
	BOOL interlaced;
	BOOL xparent;
	int xparent_index;
} GIF_CPARAMS;

typedef struct tagJPEG_CPARAMS {
	int	quality;
	int dct;
	int smooth;
	int progressive;
	int baseline;
	int grayscale;
	void* cdata;
} JPEG_CPARAMS;

typedef struct tagJP2_CPARAMS {
	int	rate;
	int xcb;
	int ycb;
	BOOL nomct;
	int reslevel;
} JP2_CPARAMS;

#define JP2_DEF_RATE			8			// 1/8 compress
#define JP2_DEF_XCB				0
#define JP2_DEF_YCB				0
#define JP2_DEF_NOMCT			FALSE
#define JP2_DEF_RESLEVEL		0

typedef struct tagPNG_CPARAMS {
	UINT mask;
	BOOL interlace;
	int compression_level;
	COLORREF clr_bkgnd;
	int num_trans;
	union {
		uint8_t* alpha_trans_ptr;					// for 8bit paletted
		COLORREF clr_trans;						// for 24bit / 24bit alpha / gray
	};
} PNG_CPARAMS;

#define PNGF_INTERLACE			0x00000001
#define PNGF_COMPRESSIONLEVEL	0x00000002
#define PNGF_BKGNDCOLOR			0x00000004
#define PNGF_XPARENTCOLOR		0x00000008
#define PNGF_TIME				0x00000010


typedef struct _HistogramArray {
	int value[256];
	int red[256];
	int green[256];
	int blue[256];
} HistogramArray;

#pragma pack()

enum {
	PLANE_RED,
	PLANE_GREEN,
	PLANE_BLUE,
	PLANE_ALPHA,
};

enum {
	JC_ISLOW,			// slow but accurate integer algorithm
	JC_IFAST,			// faster, less accurate integer method
	JC_FLOAT			// floating-point: accurate, fast on fast HW
};

enum {
	JD_NONE,			// no dithering
	JD_ORDERED,			// simple ordered dither
	JD_FS				// Floyd-Steinberg error diffusion dither
};

enum {
	TIFF_COMPRESS_NONE,
	TIFF_COMPRESS_PACKBITS,
	TIFF_COMPRESS_LZW,
	TIFF_COMPRESS_JPEG,
	TIFF_COMPRESS_ZIP
};

////////////////////////////////////////////////////////////////
// xjg format

#define XJGMAGIC			0x4A58			// "XJ"
#define XJGVERSION			0x0100			// バージョン

#define RXJG_NOERROR		0
#define RXJG_ERROR			-1
#define RXJG_NOTXJG			-2
#define RXJG_PASSREQUIRED	-3
#define RXJG_INVALIDPASS	-4
#define RXJG_CRCERROR		-5

#pragma pack(1)

typedef struct tagXJGHEADER {
	WORD magic;
	WORD version;
	uint8_t gtype;
	uint8_t crypttype;
	DWORD size;
	WORD crcCode;
	WORD crcOriginal;
	uint8_t codelen;
	uint8_t dummy;
} XJGHEADER, *LPXJGHEADER;

#pragma pack()

////////////////////////////////////////////////////////////////
// Function prototypes
//

#ifdef __cplusplus
extern "C" {                    // Assume C declarations for C++
#endif

HDIB WINAPI imgLoadImage(LPCTSTR pszName);
HDIB imgLoadThumbnail(LPCTSTR pszName, int width, int height, int* pOrgWidth, int* pOrgHeight, int* pOrgBits);

void WINAPI imgSetVersion(DWORD ver);
void WINAPI imgEnableCallback(BOOL bEnable);
void WINAPI imgSetCallback(PROGRESSCALLBACK pfn);
HDIB WINAPI imgCreateSolid(DWORD dwWidth, DWORD dwHeight, COLORREF rgb);
HPALETTE WINAPI imgCreatePalette(HDIB hdib);
BOOL WINAPI imgCreateBitmapFromDIB(HDIB hdib, HBITMAP* phbm, HPALETTE* phpal);
HDIB WINAPI imgCreateDIBFromBitmap(HBITMAP hbm, HPALETTE hpal);
void WINAPI imgPaintDIB(HDC hdc, LPRECT prcDst, HDIB hdib, LPRECT prcSrc, HPALETTE hpal);
void WINAPI imgPaintBitmap(HDC hdc, LPRECT prcDst, HBITMAP hbm, LPRECT prcSrc, HPALETTE hpal);
void WINAPI imgDrawBitmap(HDC hdc, int x, int y, HBITMAP hbm, HPALETTE hpal);
HDIB WINAPI imgChangeDIBFormat(HDIB hdib, int bits, DWORD compression);
HDIB WINAPI imgChangeBitmapFormat(HBITMAP hbm, WORD bits, DWORD compression, HPALETTE hPal);
HPALETTE WINAPI imgGetSystemPalette();
HBITMAP WINAPI imgCopyBitmap(HBITMAP hbm);
HPALETTE WINAPI imgCopyPalette(HPALETTE hpalSrc);
HDIB WINAPI imgScaleFilter(HDIB hsrc, int cx, int cy, int method, BOOL ondisk);
BOOL WINAPI imgScalePrint(HDIB hdib, int cx, int cy, int method, HDC hdc, int sx, int sy);
HDIB WINAPI imgScaleLinear(HDIB hSrc, int xNew, int yNew);
HDIB WINAPI imgScaleNear(HDIB hSrc, int xNew, int yNew);
HDIB WINAPI imgScaleInFrame(HDIB hSrc, int cx, int cy);
BOOL WINAPI imgRetrieveHistgram(HDIB hdib, int* pred, int* pgreen, int* pblue);
BOOL WINAPI imgCalculateHistogram(HDIB hdib, HistogramArray* ha);
BOOL WINAPI imgLagFill(int* tbl, int x, int value);
BOOL WINAPI imgLagFillEx(int* tbl, LPTONE pt);
HDIB WINAPI imgToneCurve(HDIB hSrc, int bpt, int val);
HDIB WINAPI imgToneCurveEx(HDIB hSrc, LPTONEINFO pti);
int WINAPI imgCheckImageType(LPCTSTR pszFile);
int WINAPI imgSpecifyImageType(LPCTSTR lpName);
int WINAPI imgTypeIsExif(int type);
void WINAPI imgGetReadableTypes(LPTSTR pTypes);
HBITMAP WINAPI imgTiledBitmap(HDC hdc, HBITMAP hbm, HPALETTE hpal, int cx, int cy);
HBITMAP WINAPI imgLoadBitmapFromResource(HINSTANCE hInst, LPCTSTR pszResource, HPALETTE* phPal);
HDIB WINAPI imgLoadDIBFromResource(HINSTANCE hInst, LPCTSTR pszResource);
BOOL WINAPI imgSavePalette(HPALETTE hpal, LPCTSTR pszName);
HPALETTE WINAPI imgLoadPalette(LPCTSTR pszName);
HDIB WINAPI imgReplacePalette(HDIB hsrc, HPALETTE hpal);
HDIB WINAPI imgSetXparentColor(HDIB hdib, int* indexfrom, int entries, int indexto, COLORREF xcolor);
HDIB WINAPI imgMapIndexColor(HDIB hdib, int* idxtbl, int entries, int mapidx, COLORREF xcolor);
BOOL WINAPI imgCopyImageToClipboard(HWND hwnd, LPCTSTR pszName);


//////////////////////////////////////////////////////////////////////
// mif functions

HDIB WINAPI mifIncreaseColor(HDIB hdib, int nBits);
HDIB WINAPI mifDecreaseColorDirect(HDIB hdib, int nBits);

//////////////////////////////////////////////////////////////////////
// jpeg functions

HDIB WINAPI imgJpgLoadMemParams(HGLOBAL hmem, int colors, int scale, int dct, int dither, BOOL smoothing);
HDIB WINAPI imgJpgLoadMem(HGLOBAL hmem, int colors, int scale);
HDIB WINAPI imgJpgLoadParams(LPCTSTR lpFile, int colors, int scale, int dct, int dither, int smoothing);
HDIB WINAPI imgJpgLoad(LPCTSTR lpFile, int colors, int scale);
HDIB WINAPI imgJpgLoadScaled(LPCTSTR lpFile, int xmax, int ymax, int colors, int dct, int dither, int smoothing);
BOOL WINAPI imgJpgGetImageSize(LPCTSTR pszName, int* width, int* height, LPSTR pszCode);
HGLOBAL WINAPI imgJpgSaveMemParams(HDIB hdib, JPEG_CPARAMS* pjc, DWORD* pdwSize);
HGLOBAL WINAPI imgJpgSaveMem(HDIB hdib, int quality, int dct, int smoothing, DWORD* pdwSize);
BOOL WINAPI imgJpgSaveParams(LPCTSTR lpFile, HDIB hDIB, JPEG_CPARAMS* pjc);
BOOL WINAPI imgJpgSave(LPCTSTR lpFile, HDIB hDIB, int quality, int dct, int smoothing);

//////////////////////////////////////////////////////////////////////
// webp functions

typedef enum {
	WEC_PRESET_DEFAULT = 0,  // default preset.
	WEC_PRESET_PICTURE,      // digital picture, like portrait, inner shot
	WEC_PRESET_PHOTO,        // outdoor photograph, with natural lighting
	WEC_PRESET_DRAWING,      // hand or line drawing, with high-contrast details
	WEC_PRESET_ICON,         // small-sized colorful images
	WEC_PRESET_TEXT          // text-like
} WEBP_ENCODE_PRESET;

HDIB WINAPI imgWebpLoadMem(HANDLE hmem);
HDIB WINAPI imgWebpLoad(LPCTSTR lpFile);
HDIB WINAPI imgWebpLoadScaled(LPCTSTR lpFile, int xmax, int ymax);
BOOL WINAPI imgWebpGetImageSize(LPCTSTR pszName, int* width, int* height);
HANDLE WINAPI imgWebpSaveMem(HDIB hdib, int quality, DWORD* pdwSize);
HANDLE WINAPI imgWebpSaveMemParams(HDIB hdib, int method, int quality, WEBP_ENCODE_PRESET preset, DWORD* pdwSize);
BOOL WINAPI imgWebpSave(LPCTSTR pszName, HDIB hDIB, int quality);
BOOL WINAPI imgWebpSaveParams(LPCTSTR pszName, HDIB hDIB, int method, int quality, WEBP_ENCODE_PRESET preset);

//////////////////////////////////////////////////////////////////////
// avif functions

HDIB WINAPI imgAvifLoadMem(HANDLE hmem);
HDIB WINAPI imgAvifLoad(LPCTSTR lpFile);
HDIB WINAPI imgAvifLoadScaled(LPCTSTR lpFile, int xmax, int ymax);
BOOL WINAPI imgAvifGetImageSize(LPCTSTR pszName, int* width, int* height);
HANDLE WINAPI imgAvifSaveMem(HDIB hdib, int quality, DWORD* pdwSize);
HANDLE WINAPI imgAvifSaveMemParams(HDIB hdib, int quality, int speed, DWORD* pdwSize);
BOOL WINAPI imgAvifSave(LPCTSTR pszName, HDIB hDIB, int quality);
BOOL WINAPI imgAvifSaveParams(LPCTSTR pszName, HDIB hDIB, int quality, int speed);

//////////////////////////////////////////////////////////////////////
// xjg functions

BOOL WINAPI imgXjgSave(LPCTSTR pszFile, HDIB hdib, int quality, LPCSTR code);
int WINAPI imgXjgLoad(LPCTSTR pszFile, LPCSTR code, HDIB* phdib);
BOOL WINAPI imgXjgEncryptFile(LPCTSTR pszSrc, LPCTSTR pszDst, LPCSTR pszCode);
int WINAPI imgXjgDecryptFile(LPCTSTR pszSrc, LPCTSTR pszDst, LPCSTR pszCode);
BOOL WINAPI imgXjgGetImageInfo(LPCTSTR pszFile, XJGHEADER* pHeader);


//////////////////////////////////////////////////////////////////////
// dib functions

HANDLE WINAPI imgDibSaveMem(HDIB hdib, DWORD* pdwSizeMem);
BOOL WINAPI imgDibSave(LPCTSTR pszFile, HDIB hdib);
HDIB WINAPI imgDibLoadMem(HANDLE hmem);
HDIB WINAPI imgDibLoad(LPCTSTR pszFile);
HDIB WINAPI imgDibFromClipboard(HWND hwnd);
BOOL WINAPI imgDibToClipboard(HWND hwnd, HDIB hdib);

//////////////////////////////////////////////////////////////////////
// gif functions

HDIB WINAPI imgGifLoadMemEx(HGLOBAL hmem, int* pTransparent);
HDIB WINAPI imgGifLoadMem(HGLOBAL hmem);
HDIB WINAPI imgGifLoad(LPCTSTR fullname);
HANDLE WINAPI imgGifSaveMemParamsSt(HDIB hdib, GIF_CPARAMS* pgc, DWORD* pdwSize);
HANDLE WINAPI imgGifSaveMemParams(HDIB hdib, BOOL bGIF87a, BOOL bInterlaced, DWORD* pdwSize);
BOOL WINAPI imgGifSaveParamsSt(LPCTSTR pszFile, HDIB hdib, GIF_CPARAMS* pgc);
BOOL WINAPI imgGifSaveParams(LPCTSTR pszFile, HDIB hdib, BOOL bGIF87a, BOOL bInterlaced);
BOOL WINAPI imgGifSave(LPCTSTR pszFile, HDIB hdib);

//////////////////////////////////////////////////////////////////////
// tga functions

HANDLE WINAPI imgTgaSaveMem(HDIB hdib, DWORD* pdwSizeMem);
BOOL WINAPI imgTgaSave(LPCTSTR pszFile, HDIB hdib);
HDIB WINAPI imgTgaLoadMem(HANDLE hmem);
HDIB WINAPI imgTgaLoad(LPCTSTR pszFile);

//////////////////////////////////////////////////////////////////////
// pcx functions

HDIB WINAPI imgPcxLoadMem(HANDLE hmem, DWORD dwSize);
HDIB WINAPI imgPcxLoad(LPCTSTR pszName);
HANDLE WINAPI imgPcxSaveMem(HDIB hdib, DWORD* pdwSize);
BOOL WINAPI imgPcxSave(LPCTSTR pszFile, HDIB hdib);

//////////////////////////////////////////////////////////////////////
// psd functions

HDIB WINAPI imgPsdLoad(LPCTSTR pszName);
HDIB WINAPI imgPsdThumbLoad(LPCTSTR pszName, int* pwidth, int* pheight);
BOOL WINAPI imgPsdSave(LPCTSTR pszName, HDIB hdib);

//////////////////////////////////////////////////////////////////////
// pnm functions

HDIB WINAPI imgPnmLoad(LPCTSTR pszName);
BOOL WINAPI imgPnmSave(LPCTSTR pszName, HDIB hdib);


//////////////////////////////////////////////////////////////////////
// ras functions

HDIB WINAPI imgRasLoadMem(HANDLE hmem);
HDIB WINAPI imgRasLoad(LPCTSTR pszName);
BOOL WINAPI imgRasSave(LPCTSTR pszName, HDIB hdib);

//////////////////////////////////////////////////////////////////////
// png functions

HDIB WINAPI imgPngLoadMem(HANDLE hmem);
HDIB WINAPI imgPngLoad(LPCTSTR pszName);
HANDLE WINAPI imgPngSaveMem(HDIB hdib, DWORD* pdwSize);
HANDLE WINAPI imgPngSaveMemParams(HDIB hdib, PNG_CPARAMS* pcpi, DWORD* pdwSize);
BOOL WINAPI imgPngSave(LPCTSTR pszName, HDIB hdib);
BOOL WINAPI imgPngSaveParams(LPCTSTR pszName, HDIB hdib, PNG_CPARAMS* ppc);

//////////////////////////////////////////////////////////////////////
// mag functions

HDIB WINAPI imgMagLoad(LPCTSTR pszName);
HDIB WINAPI imgMkiLoad(LPCTSTR pszName);

//////////////////////////////////////////////////////////////////////
// pcd functions

HDIB WINAPI imgPcdLoad(LPCTSTR pszName);
HDIB WINAPI imgPcdLoadEx(LPCTSTR pszName, int subimage);	// subimage=1 128×192
															// subimage=2 256×384
															// subimage=3 512×768
															// subimage=4 1024×1536
															// subimage=5 2048×3072
															// subimage=6 4096×6144


//////////////////////////////////////////////////////////////////////
// tiff functions

HDIB WINAPI imgTifLoad(LPCTSTR pszName);
BOOL WINAPI imgTifSaveParams(LPCTSTR pszFile, HDIB hdib, int compression);
BOOL WINAPI imgTifSave(LPCTSTR pszName, HDIB hdib);


//////////////////////////////////////////////////////////////////////
// cam functions

HDIB WINAPI imgCamLoad(LPCTSTR pszName);
BOOL WINAPI imgCamSave(LPCTSTR pszName, HDIB hdib);


//////////////////////////////////////////////////////////////////////
// sgi functions

HDIB WINAPI imgSgiLoad(LPCTSTR pszName);
HANDLE WINAPI imgSgiSaveMem(HDIB hdib, int nCompression, DWORD* pdwSize);
BOOL WINAPI imgSgiSave(LPCTSTR pszName, HDIB hdib, int nCompression);


//////////////////////////////////////////////////////////////////////
// wmf functions

HDIB WINAPI imgWmfLoadMem(HGLOBAL hmem, DWORD dwSize);
HDIB WINAPI imgWmfLoad(LPCTSTR fullname);

//////////////////////////////////////////////////////////////////////
// ico functions

HDIB WINAPI imgIcoLoad(LPCTSTR fullname);


//////////////////////////////////////////////////////////////////////
// txr functions

HDIB WINAPI imgTxrLoad(LPCTSTR fullname);
HDIB WINAPI imgTxrLoadFromResource(HINSTANCE hInst, UINT typeResource, UINT idResource);
BOOL WINAPI imgGetTxrInfo(LPCTSTR pszName, LPSTR comment, int* width, int* height, int* bpp);
BOOL WINAPI imgGetResourceTxrInfo(HINSTANCE hInst, UINT typeResource, UINT idResource, LPSTR comment, int* width, int* height, int* bpp);


//////////////////////////////////////////////////////////////////////
// xpm functions

HDIB WINAPI imgXpmLoad(LPCTSTR fullname);


//////////////////////////////////////////////////////////////////////
// xbm functions

HDIB WINAPI imgXbmLoad(LPCTSTR fullname);


//////////////////////////////////////////////////////////////////////
// fpx functions

HDIB WINAPI imgFpxLoad(LPCTSTR pszName);
HDIB WINAPI imgFpxLoadEx(LPCTSTR pszName, UINT target_subimage, SIZE* frame);
HDIB WINAPI imgFpxLoadThumbnail(LPCTSTR pszName, SIZE* frame);
BOOL WINAPI imgFpxSaveEx(LPCTSTR pszName, HDIB hdib, int compress_mode, int quality);
BOOL WINAPI imgFpxSave(LPCTSTR pszName, HDIB hdib);

//////////////////////////////////////////////////////////////////////
// jpeg2000 functions

HDIB WINAPI imgJp2Load(LPCTSTR pszName);
HDIB WINAPI imgJp2LoadMem(HANDLE hmem);
BOOL WINAPI imgJp2Save(LPCTSTR pszName, HDIB hdib);
BOOL WINAPI imgJp2SaveEx(LPCTSTR pszFile, HDIB hdib, JP2_CPARAMS* pjc);
HGLOBAL WINAPI imgJp2SaveMemEx(HDIB hdib, JP2_CPARAMS* pjc, DWORD* pdwSize);


//////////////////////////////////////////////////////////////////////
// CRW(canon raw) functions

HDIB WINAPI imgCrwLoad(LPCTSTR pszFile);
HDIB WINAPI imgDCRawLoad(LPCTSTR pszFile);



#ifdef __cplusplus
}
#endif

#endif			// __LSIMG_H__
