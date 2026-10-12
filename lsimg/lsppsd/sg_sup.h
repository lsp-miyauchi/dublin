//////////////////////////////////////////////////////////////////////
// sg_sup.h

#ifndef __SG_SUP_H__
#define __SG_SUP_H__

#pragma pack(1)

typedef struct tagCOLORPACKET {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
} COLORPACKET;

typedef struct tagPIXELPACKET {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char index;
	unsigned char alpha;
} PIXELPACKET;

typedef struct tagSGIMAGE {
	int bits;
	int width;
	int height;
	int colors;
	int packets;
	HANDLE hfile;
	HANDLE hmap;
	char tmpfile[MAX_PATH];
	COLORPACKET colormap[256];
	PIXELPACKET* pixels;
} SGIMAGE;

#pragma pack()


SGIMAGE* __fastcall sg_create(int width, int height, int bits, BOOL bOnDisk = FALSE);
BOOL __fastcall sg_destroy(SGIMAGE* image);
BOOL __fastcall sg_move(SGIMAGE* dst, SGIMAGE* src);
BOOL __fastcall sg_convert2bmi(SGIMAGE* image, BITMAPINFO* pbi);
HDIB __fastcall sg_getdib(SGIMAGE* image);
SGIMAGE* __fastcall sg_fromdib(HANDLE hdib);
SGIMAGE* __fastcall sg_get(SGIMAGE* src, int sx, int sy, int dx, int dy);
BOOL __fastcall sg_fill(SGIMAGE* dst, int sx, int sy, int dx, int dy, COLORREF rgb);
BOOL __fastcall sg_blt(SGIMAGE* dst, int xdst, int ydst, int dx, int dy, SGIMAGE* src, int xsrc, int ysrc, DWORD rop);
BOOL __fastcall sg_mirror(SGIMAGE* src, int sx, int sy, int dx, int dy);
BOOL __fastcall sg_flip(SGIMAGE* src, int sx, int sy, int dx, int dy);
BOOL __fastcall sg_getplane(SGIMAGE* image, UINT plane, unsigned char* data);
BOOL __fastcall sg_setplane(SGIMAGE* image, UINT plane, unsigned char* data);
BOOL __fastcall sg_getrow(SGIMAGE* image, PIXELPACKET* line, int row);
BOOL __fastcall sg_setrow(SGIMAGE* image, PIXELPACKET* line, int row);
BOOL __fastcall sg_getcol(SGIMAGE* image, PIXELPACKET* line, int col);
BOOL __fastcall sg_setcol(SGIMAGE* image, PIXELPACKET* line, int col);
SGIMAGE* __fastcall sg_rotate(SGIMAGE* src, BOOL rotate_left);
void __fastcall sg_cmyk2rgb(PIXELPACKET* pkt);
void __fastcall sg_rgb2cmyk(PIXELPACKET* pkt);
void __fastcall sg_sharpen(SGIMAGE* image);

#endif			// __SG_SUP_H__
