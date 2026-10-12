//////////////////////////////////////////////////////////////////////
// gifload.cpp  リソースからGIFイメージをロード

#include "stdafx.h"
#include "common.h"
#include "mfmem.h"

#define MAXCOLORMAPSIZE			256
#define MAX_LZW_BITS			12

#define BIT_INTERLACE			0x40
#define BIT_COLORMAP			0x80
#define ISBITSET(byte, bit)		(((byte) & (bit)) == (bit))
#define LM_TO_UNIT(a,b)			(((b)<<8)|(a))
#define LZW_TABLE_SIZE			((short)1 << MAX_LZW_BITS)

typedef struct tagGIFIMAGE {
	RGBQUAD map[MAXCOLORMAPSIZE];
	int colors;
	int width;
	int height;
	int xparent;
	BYTE cbData[1];
} GIFIMAGE;
typedef GIFIMAGE* LPGIFIMAGE;

typedef struct tagGIF89 {
	int     transparent;
	int     delayTime;
	int     inputFlag;
	int     disposal;
} GIF89;
typedef GIF89* LPGIF89;

static GIF89 gif89 = { -1, -1, -1, 0 };
static int read_colormap(memstream*, int n, RGBQUAD*);
static int do_extension(memstream*, int, int*);
static int getdatablock(memstream*, BYTE*);
static int getcode(memstream*, int, int);
static int lzw_reatebyte(memstream*, int, int);
static void read_image(LPGIFIMAGE, memstream*, int, int, RGBQUAD*, BOOL);


int ZeroDataBlock;

LPGIFIMAGE AllocImageStruct(int sx, int sy)
{
	DWORD dw = sizeof(GIFIMAGE) + sx * sy;
	LPGIFIMAGE pgi = (LPGIFIMAGE)GlobalAllocPtr(GMEM_MOVEABLE, dw);
	if (pgi) {
		memset(pgi, 0, dw);
		pgi->width = sx;
		pgi->height = sy;
		pgi->xparent = -1;
	}
	return pgi;
}

void FreeImageStruct(LPGIFIMAGE pgi)
{
	if (pgi)
		GlobalFreePtr(pgi);
}

LPGIFIMAGE gifDecorder(memstream* pmf)
{
	int num_color_global;
	int num_color_local;
	int color_res;
	int color_bkgnd;
	int aspect_ratio;
	int Transparent = (-1);
	BYTE buf[16];
	BYTE c;
	RGBQUAD cpMap[MAXCOLORMAPSIZE];
	RGBQUAD cpLocalMap[MAXCOLORMAPSIZE];
	int sx, sy;
	int useGlobalColormap;
	LPGIFIMAGE pgi = NULL;
	BOOL bInterlace;
	ZeroDataBlock = FALSE;

	if (pmf->read(buf, 6) != 6)
		return NULL;

	if (*((DWORD*)buf) != 0x38464947 ||
		(*((WORD*)&buf[4]) != 0x6137 && *((WORD*)&buf[4]) != 0x6139))
		return NULL;

	if (pmf->read(buf, 7) != 7)
		return NULL;

	num_color_global = 1 << ((buf[4] & 0x07) + 1);
	color_res        = (int)(((buf[4] & 0x70) >> 3) + 1);
	color_bkgnd      = buf[5];
	aspect_ratio     = buf[6];

	// グルーバルカラーマップがあれば取得
	if (ISBITSET(buf[4], BIT_COLORMAP)) {
		if (read_colormap(pmf, num_color_global, cpMap))
			return NULL;
	}

	for (;;) {
		if (pmf->read(&c, 1) != 1)
			return NULL;

		if (c == ';')						/* GIF terminator */
			break;

		if (c == '!') {						/* Extension */
			if (pmf->read(&c, 1) != 1)
				return NULL;

			do_extension(pmf, c, &Transparent);
			continue;
		}

		if (c != ',')						/* Not a valid start character */
			continue;

		if (pmf->read(buf, 9) != 9)
			return NULL;

		useGlobalColormap = !ISBITSET(buf[8], BIT_COLORMAP);

		num_color_local = 1 << ((buf[8] & 0x07) + 1);

		sx = LM_TO_UNIT(buf[4], buf[5]);
		sy = LM_TO_UNIT(buf[6], buf[7]);

		pgi = AllocImageStruct(sx, sy);
		if (pgi == NULL)
			return NULL;

		bInterlace = ISBITSET(buf[8], BIT_INTERLACE);
		if (!useGlobalColormap) {
			if (read_colormap(pmf, num_color_local, cpLocalMap))
				return NULL;

			read_image(pgi, pmf, sx, sy, cpLocalMap, bInterlace);
			pgi->colors = num_color_local;
		}
		else {
			read_image(pgi, pmf, sx, sy, cpMap, bInterlace);
			pgi->colors = num_color_global;
		}

		if (Transparent != (-1))
			pgi->xparent = Transparent;
		break;
	}
	return pgi;
}

static int read_colormap(memstream* pmf, int number, RGBQUAD* map)
{
	int i;
	BYTE rgb[3];
	for (i = 0; i < number; ++i) {
		if (pmf->read(rgb, sizeof(rgb)) != sizeof(rgb))
			return TRUE;

		map[i].rgbRed   = rgb[0];
		map[i].rgbGreen = rgb[1];
		map[i].rgbBlue  = rgb[2];
	}
	return FALSE;
}

static int do_extension(memstream* pmf, int label, int *Transparent)
{
	static BYTE     buf[256];

	switch (label) {
		case 0xf9:              /* Graphic Control Extension */
			getdatablock(pmf, (BYTE*) buf);
			gif89.disposal    = (buf[0] >> 2) & 0x7;
			gif89.inputFlag   = (buf[0] >> 1) & 0x1;
			gif89.delayTime   = LM_TO_UNIT(buf[1],buf[2]);
			if ((buf[0] & 0x1) != 0)
				*Transparent = buf[3];

			while (getdatablock(pmf, (BYTE*) buf) != 0)
				;
			return FALSE;

		default:
			break;
	}

	while (getdatablock(pmf, (BYTE*) buf) != 0)
		;

	return FALSE;
}

static int getdatablock(memstream* pmf, BYTE *buf)
{
	BYTE count;
	if (pmf->read(&count, 1) != 1)
		return -1;

	ZeroDataBlock = count == 0;

	if (count && pmf->read(buf, count) != count)
		return -1;
	return count;
}

static int getcode(memstream* pmf, int code_size, int flag)
{
	static BYTE buf[280];
	static int curbit, lastbit, done, last_byte;
	int i, j, ret;
	BYTE count;

	if (flag) {
		curbit = 0;
		lastbit = 0;
		done = FALSE;
		return 0;
	}

	if ( (curbit+code_size) >= lastbit) {
		if (done) {
			if (curbit >= lastbit) {
				/* Oh well */
			}
			return -1;
		}
		buf[0] = buf[last_byte-2];
		buf[1] = buf[last_byte-1];

		count = getdatablock(pmf, &buf[2]);
		if (count == 0)
			done = TRUE;
		else if (count < 0)
			return -1;

		last_byte = 2 + count;
		curbit = (curbit - lastbit) + 16;
		lastbit = (2 + count) * 8;
	}

	ret = 0;
	for (i = curbit, j = 0; j < code_size; ++i, ++j)
		ret |= ((buf[i / 8 ] & (1 << (i % 8))) != 0) << j;

	curbit += code_size;
	return ret;
}

static int lzw_reatebyte(memstream* pmf, int flag, int input_code_size)
{
	static int      fresh = FALSE;
	static int      code_size, set_code_size;
	static int      max_code, max_code_size;
	static int      firstcode, oldcode;
	static int      clear_code, end_code;
	static int      table[2][(1<< MAX_LZW_BITS)];
	static int      stack[(1<<(MAX_LZW_BITS))*2], *sp;
	int code, incode;
	register int    i;

	if (flag) {
		set_code_size = input_code_size;
		code_size = set_code_size+1;
		clear_code = 1 << set_code_size;
		end_code = clear_code + 1;
		max_code_size = 2*clear_code;
		max_code = clear_code+2;

		if (getcode(pmf, 0, TRUE) < 0)
			return -2;

		fresh = TRUE;

		for (i = 0; i < clear_code; ++i) {
			table[0][i] = 0;
			table[1][i] = i;
		}

		for (; i < (1<<MAX_LZW_BITS); ++i)
			table[0][i] = table[1][0] = 0;
		sp = stack;
		return 0;
	}
	else if (fresh) {
		fresh = FALSE;
		do {
			firstcode = oldcode = getcode(pmf, code_size, FALSE);
		} while (firstcode == clear_code);
		return firstcode;
	}

	if (sp > stack)
		return *--sp;

	while ((code = getcode(pmf, code_size, FALSE)) >= 0) {
		if (code == clear_code) {
			for (i = 0; i < clear_code; ++i) {
				table[0][i] = 0;
				table[1][i] = i;
			}

			for (; i < (1<<MAX_LZW_BITS); ++i)
				table[0][i] = table[1][i] = 0;

			code_size = set_code_size+1;
			max_code_size = 2*clear_code;
			max_code = clear_code+2;
			sp = stack;
			firstcode = oldcode = getcode(pmf, code_size, FALSE);
			return firstcode;
		}
		else if (code == end_code) {
			int count;
			BYTE buf[260];

			if (ZeroDataBlock)
				return -2;

			while ((count = getdatablock(pmf, buf)) > 0)
				;

			if (count != 0)
				return -2;
		}

		incode = code;
		if (code >= max_code) {
			*sp++ = firstcode;
			code = oldcode;
		}

		while (code >= clear_code) {
if (sp >= stack + (1 << (MAX_LZW_BITS)) * 2)
	return -2;

			*sp++ = table[1][code];
			if (code == table[0][code]) {
				/* Oh well */
			}
			code = table[0][code];
		}

		*sp++ = firstcode = table[1][code];

		if ((code = max_code) < (1 << MAX_LZW_BITS)) {
			table[0][code] = oldcode;
			table[1][code] = firstcode;
			++max_code;
			if ((max_code >= max_code_size) &&
				(max_code_size < (1 << MAX_LZW_BITS)))
			{
				max_code_size *= 2;
				++code_size;
			}
		}

		oldcode = incode;

		if (sp > stack)
			return *--sp;
	}
	return code;
}

static void read_image(LPGIFIMAGE pgi, memstream* pmf, int len, int height, RGBQUAD* cmap, int interlace)
{
	BYTE   c;
	int v;
	int xpos = 0, ypos = 0, pass = 0;
	int i;

	/* Stash the color map into the image */
	for (i = 0; i < MAXCOLORMAPSIZE; i++) {
		pgi->map[i].rgbRed      = cmap[i].rgbRed;
		pgi->map[i].rgbGreen    = cmap[i].rgbGreen;
		pgi->map[i].rgbBlue     = cmap[i].rgbBlue;
		pgi->map[i].rgbReserved = 1;
	}

	/*
	**  Initialize the Compression routines
	*/
	if (pmf->read(&c, 1) != 1)
		return;

	if (lzw_reatebyte(pmf, TRUE, c) < 0)
		return;

	while ((v = lzw_reatebyte(pmf, FALSE, c)) >= 0) {
		// 使用されている色としてチェック
		if (pgi->map[v].rgbReserved)
			pgi->map[v].rgbReserved = 0;

		// ピクセルデータのセット
		pgi->cbData[ypos * pgi->width + xpos] = v;
		++xpos;
		if (xpos == len) {
			xpos = 0;
			if (interlace) {
				switch (pass) {
					case 0:
					case 1:
						ypos += 8;
						break;

					case 2:
						ypos += 4;
						break;

					case 3:
						ypos += 2;
						break;
				}

				if (ypos >= height) {
					++pass;
					switch (pass) {
						case 1:
							ypos = 4;
							break;

						case 2:
							ypos = 2;
							break;

						case 3:
							ypos = 1;
							break;

						default:
							goto fini;
					}
				}
			}
			else
				++ypos;
		}
        if (ypos >= height)
			break;
	}

fini:
	if (lzw_reatebyte(pmf, FALSE, c) >= 0) {
		;			/* Ignore extra */
	}
}

#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

static HBITMAP __fastcall gif_load(HGLOBAL hmem, COLORREF* pcrMask)
{
	memstream* pmf = new memstream(hmem, 0);
	LPGIFIMAGE pgi = gifDecorder(pmf);
	delete pmf;

	if (!pgi)
		return NULL;

	int widthbytes = WIDTHBYTES(pgi->width * 8);
	DWORD size = sizeof(BITMAPINFOHEADER) +
					pgi->colors * sizeof(RGBQUAD) +
					widthbytes * pgi->height;
	BITMAPINFO* pbi = (BITMAPINFO*)malloc(size);
	if (!pbi) {
		FreeImageStruct(pgi);
		return NULL;
	}
	memset(pbi, 0, size);

	BITMAPINFOHEADER* pbh = (BITMAPINFOHEADER*)pbi;
	pbh->biSize = sizeof(BITMAPINFOHEADER);
	pbh->biWidth = pgi->width;
	pbh->biHeight = pgi->height;
	pbh->biPlanes = 1;
	pbh->biBitCount = 8;
	pbh->biCompression = BI_RGB;
	pbh->biClrUsed = pgi->colors;
	pbh->biClrImportant = pgi->colors;
	int i;
	for (i = 0; i < pgi->colors; i++) {
		pbi->bmiColors[i].rgbRed = pgi->map[i].rgbRed;
		pbi->bmiColors[i].rgbGreen = pgi->map[i].rgbGreen;
		pbi->bmiColors[i].rgbBlue = pgi->map[i].rgbBlue;
	}

	BYTE* pBits = (BYTE*)&pbi->bmiColors[pgi->colors];
	register BYTE* ps = pBits + (pgi->height - 1) * widthbytes;
	register BYTE* pd = pgi->cbData;
	for (i = 0; i < pgi->height; i++) {
		memcpy(ps, pd, pgi->width);
		ps -= widthbytes;
		pd += pgi->width;
	}

	if (pcrMask) {
		*pcrMask = RGB(pbi->bmiColors[pgi->xparent].rgbRed,
					   pbi->bmiColors[pgi->xparent].rgbGreen,
					   pbi->bmiColors[pgi->xparent].rgbBlue);
	}

	CClientDC dc(NULL);
	HBITMAP hbm = CreateDIBitmap(dc, pbh, CBM_INIT, pBits, pbi, DIB_RGB_COLORS);
	free(pbi);
	FreeImageStruct(pgi);
	return hbm;
}

extern "C"
HBITMAP WINAPI lccLoadGIFImage(HINSTANCE hInst, LPCTSTR typeResource, UINT idResource, COLORREF* pcrMask)
{
	HBITMAP hbm = NULL;
	HRSRC hrsrc = FindResource(hInst, MAKEINTRESOURCE(idResource), typeResource);
	if (hrsrc) {
		DWORD sizeResource = SizeofResource(hInst, hrsrc);
		if (sizeResource) {
			HGLOBAL hglb = LoadResource(hInst, hrsrc);
			if (hglb) {
				void* pr = LockResource(hglb);
				if (pr) {
					HGLOBAL hmem = GlobalAlloc(GHND, sizeResource);
					if (hmem) {
						void* pd = GlobalLock(hmem);
						memcpy(pd, pr, sizeResource);
						GlobalUnlock(hmem);

						hbm = gif_load(hmem, pcrMask);
						GlobalFree(hmem);
					}
				}
			}
		}
	}
	return hbm;
}

extern "C"
HIMAGELIST WINAPI lccLoadGIFImageList(HINSTANCE hInst, LPCTSTR typeResource, UINT idResource, int itemWidth)
{
	COLORREF crMask;
	HBITMAP hbm = lccLoadGIFImage(hInst, typeResource, idResource, &crMask);
	if (hbm) {
		BITMAP bm;
		GetObject(hbm, sizeof(bm), &bm);
		HIMAGELIST him = ImageList_Create(itemWidth, bm.bmHeight, ILC_COLORDDB | ILC_MASK, bm.bmWidth / itemWidth, 2);
		ImageList_AddMasked(him, hbm, crMask);
		DeleteObject(hbm);
		return him;
	}
	return NULL;
}
