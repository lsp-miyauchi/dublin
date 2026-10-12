///////////////////////////////////////////////////////////////////////
// loadimage.cpp
//

#include "stdafx.h"
#include <math.h>
#include "filemap.h"

#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)
#define CLAMP(a)			((BYTE)max(0, min((a), 255)))
#define _ISDIGIT(a)			('0'<=(a)&&(a)<='9')
#ifndef MAX_URL_LENGTH
	#define MAX_URL_LENGTH			(2048 + 32 + 8)
#endif


_inline float CalcScaleRatio(uint32_t width, uint32_t height, uint32_t max_scaled_width, uint32_t max_scaled_height)
{
	float xr = (float)max_scaled_width / (float)width;
	float yr = (float)max_scaled_height / (float)height;
	return min(1.0f, min(xr, yr));
}

///////////////////////////////////////////////////////////////////////
//

static IStream* stream_file(LPCTSTR pszName)
{
/*
	IStream* pstrm = NULL;
	HANDLE hf = CreateFile(pszName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hf != INVALID_HANDLE_VALUE) {
		DWORD size = GetFileSize(hf, NULL);
		void* p = malloc(size);
		if (p) {
			DWORD n;
			ReadFile(hf, p, size, &n, NULL);
			if (CreateStreamOnHGlobal(hmem, TRUE, &pstrm) != S_OK) {
				pstrm = NULL;
				free(p);
			}
		}
		CloseHandle(hf);
	}
	return pstrm;
*/
	IStream* pstrm = NULL;
	LPITEMIDLIST pidlFq = ILCreateFromPath(pszName);
	if (pidlFq) {
		CComPtr<IShellFolder> psf;
		LPCITEMIDLIST pidl;
		if (SHBindToParent(pidlFq, IID_PPV_ARGS(&psf), &pidl) == S_OK) {
			if (psf->BindToObject(pidl, NULL, IID_PPV_ARGS(&pstrm)) != S_OK) {
				pstrm = NULL;
			}
		}
	}
	ILFree(pidlFq);
	return pstrm;
}

static Gdiplus::Bitmap* load_core_gdip(Gdiplus::Bitmap& bm, int width, int height)
{
	Gdiplus::Bitmap* bitmap = NULL;
	float scale = CalcScaleRatio(bm.GetWidth(), bm.GetHeight(), width, height);
	if (scale < 1.0f) {
		uint32_t cx = (uint32_t)floor(bm.GetWidth() * scale);
		uint32_t cy = (uint32_t)floor(bm.GetHeight() * scale);

		Image* image = bm.GetThumbnailImage(cx, cy, NULL, NULL);
		Bitmap* bmCast = (Bitmap*)image;
		bitmap = bmCast->Clone(0, 0, bmCast->GetWidth(), bmCast->GetHeight(), bmCast->GetPixelFormat());
/*
		Image* image = bm.GetThumbnailImage(cx, cy, NULL, NULL);
		bitmap = new Gdiplus::Bitmap(cx, cy, PixelFormat24bppRGB);
		Graphics g(bitmap);
		// 下地を塗る
		SolidBrush sbr(Color(255, 255, 255, 255));
		PixelFormat fmt = bm.GetPixelFormat();
		if (fmt & PixelFormatAlpha) {
			// Alphaチャネルを持っている場合は下地の色を変える
			sbr.SetColor(Color(255, 0xee, 0xf2, 0xfb));
		}
		g.FillRectangle(&sbr, 0, 0, cx, cy);
		// イメージ
		g.DrawImage(image, 0, 0, cx, cy);
*/
	}
	else {
		bitmap = bm.Clone(0, 0, bm.GetWidth(), bm.GetHeight(), PixelFormat24bppRGB);
	}
	return bitmap;
}

///////////////////////////////////////////////////////////////////////
//

class memdib
{
public:
	BITMAPINFO* m_pbi;
	BITMAPINFOHEADER* m_pbh;
	BYTE* m_pixels;
	int m_widthbytes;

	memdib() : m_pbi(NULL), m_pbh(NULL), m_pixels(NULL), m_widthbytes(0)
	{
	}

	memdib(int width, int height, int bits)
	{
		m_pbh = NULL;
		m_pixels = NULL;
		m_widthbytes = WIDTHBYTES(width * bits);
		int headersize = sizeof(BITMAPINFOHEADER) + ((bits <= 8) ? sizeof(RGBQUAD) * ((_int64)1 << bits) : 0);
		int imagesize = m_widthbytes * height;
		m_pbi = (BITMAPINFO*)malloc(headersize + imagesize);
		if (m_pbi) {
			memset((void*)m_pbi, 0, headersize);
			m_pbh = (BITMAPINFOHEADER*)m_pbi;
			m_pbh->biSize = sizeof(BITMAPINFOHEADER);
			m_pbh->biWidth = width;
			m_pbh->biHeight = height;
			m_pbh->biBitCount = (WORD)bits;
			m_pbh->biCompression = BI_RGB;
			m_pbh->biSizeImage = imagesize;
			m_pbh->biPlanes = 1;
			if (bits <= 8)
				m_pbh->biClrUsed = m_pbh->biClrImportant = (1 << bits);
			m_pixels = (BYTE*)m_pbi + headersize;
		}
	}

	~memdib()
	{
		if (m_pbi) free(m_pbi);
	}

	__inline bool IsNull() const { return (m_pbi == NULL); }
	__inline int Width() const { return (int)m_pbh->biWidth; }
	__inline int Height() const { return (int)m_pbh->biHeight; }
	__inline int BitCount() const { return (int)m_pbh->biBitCount; }
	__inline int WidthBytes() const { return m_widthbytes; }
	__inline BITMAPINFO* BitmapInfo() const { return m_pbi; }
	__inline BITMAPINFOHEADER* BitmapInfoHeader() const { return m_pbh; }
	__inline BYTE* BitmapBits() const { return m_pixels; }
	__inline BYTE* RowPosition(int row) const { return (m_pixels + m_widthbytes * (m_pbh->biHeight - row - 1)); }
	__inline RGBQUAD* ColorMap() const { return m_pbi->bmiColors; }

	Bitmap* GetBitmap() const
	{
		if (IsNull()) return NULL;
		return new Bitmap(m_pbi, m_pixels);
	}

	Bitmap* GetThumbnail(int width, int height, bool rotate = false) const
	{
		Bitmap* bitmap = GetBitmap();
		if (!bitmap) return NULL;

		uint32_t cx = m_pbh->biWidth;
		uint32_t cy = m_pbh->biHeight;
		if (rotate) {
			bitmap->RotateFlip(Rotate270FlipNone);
			cx = m_pbh->biHeight;
			cy = m_pbh->biWidth;
		}

		Bitmap* bm = NULL;
		float scale = CalcScaleRatio(cx, cy, width, height);
		if (scale < 1.0f) {
			cx = (uint32_t)float(cx * scale);
			cy = (uint32_t)float(cy * scale);
			Image* image = bitmap->GetThumbnailImage(cx, cy, NULL, NULL);
			Bitmap* bitmapCast = (Bitmap*)image;
			bm = bitmapCast->Clone(0, 0, bitmapCast->GetWidth(), bitmapCast->GetHeight(), bitmapCast->GetPixelFormat());
			delete image;
			delete bitmap;
		}
		else
			bm = bitmap;
		return bm;
	}
};

///////////////////////////////////////////////////////////////////////
// JPEG/GIF/PNG/TIFF

static Bitmap* LoadImageFromFile(LPCTSTR pszName, int width, int height)
{
	Bitmap bitmap(pszName);
	return load_core_gdip(bitmap, width, height);
}

static Bitmap* LoadImageFromStream(LPCTSTR pszName, int width, int height)
{
	CComPtr<IStream> ps = stream_file(pszName);
	if (ps.p != NULL) {
		Bitmap bitmap(ps);
		return load_core_gdip(bitmap, width, height);
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////
// Adobe Photoshop .psd

#pragma pack(1)

typedef struct _THUMBINFO {
	ULONG format;			// format = 1 (Jpeg). RawRGB (0).
	ULONG width;			// Width of thumbnail in pixels.
	ULONG height;			// Height of thumbnail in pixels.
	ULONG widthbytes;		// Padded row bytes as (width * bits + 31) / 32 * 4.
	ULONG size;				// Total size as widthbytes * height * planes
	ULONG compressedsize;	// Size after compression. Used for consistentcy check.
	USHORT bitspixel;		// = 24. Bits per pixel.
	USHORT planes;			// = 1. Number of planes.
	// and follow JFIF or BGR
} THUMBINFO;

typedef struct _PSDHEADER {				// PSD v3.0 HEADER
	char signature[4];					// "8BPS"
	USHORT version;						// only '1'
	BYTE reserved[6];
	USHORT channels;					// range 1 - 24
	int rows;							// range 1 - 30,000
	int columns;						// range 1 - 30,000
	USHORT depth;						// 1, 8, 16
	USHORT mode;						//
} PSDHEADER;

#pragma pack()

static Bitmap* psd_thumb_load(filemap& fm, int width, int height)
{
	char type[4];
	long count, length, size;
	PSDHEADER head;
	UINT status;

	// イメージヘッダを読む
	status = fm.read(head.signature, 4);
	head.version = fm.getmsbshort();
	if (!status || StrCmpNA(head.signature, "8BPS", 4) || head.version != 1)
		return NULL;

	fm.read(head.reserved, 6);
	head.channels = fm.getmsbshort();
	head.rows = fm.getmsblong();
	head.columns = fm.getmsblong();
	head.depth = fm.getmsbshort();
	head.mode = fm.getmsbshort();

	// カラーマップセクションを読む
	length = fm.getmsblong();
	if (length > 0)
		fm.seek(length, SEEK_CUR);

	// イメージリソースセクション
	Bitmap* bitmap = NULL;
	length = fm.getmsblong();
	while (length > 0) {
		// リソースブロックの読み込み
		status = fm.read(type, 4);
		if (!status || StrCmpNA(type, "8BIM", 4))
			break;

		// リソースID
		int id = fm.getmsbshort();

		// リソース文字列（偶数サイズ）
		count = fm.getc();
		if (count > 0)
			fm.seek(count, SEEK_CUR);
		if (!(count & 0x01)) {
			fm.seek(1, SEEK_CUR);
			length--;
		}

		// リソースブロックサイズ（偶数サイズ）
		size = fm.getmsblong();

		if (id == 1033 || id == 1036) {					// psp4.0/5.0 サムネイル
			THUMBINFO ti;
			ti.format = fm.getmsblong();
			ti.width = fm.getmsblong();
			ti.height = fm.getmsblong();
			ti.widthbytes = fm.getmsblong();
			ti.size = fm.getmsblong();
			ti.compressedsize = fm.getmsblong();
			ti.bitspixel = fm.getmsbshort();
			ti.planes = fm.getmsbshort();

			BYTE* pb = (BYTE*)malloc(ti.compressedsize);
			if (pb) {
				fm.read(pb, ti.compressedsize);

				if (ti.format != 0) {						// JFIF形式
					CComPtr<IStream> pstrm = SHCreateMemStream(pb, ti.compressedsize);
					if (pstrm) {
						Bitmap bm(pstrm);
						bitmap = load_core_gdip(bm, width, height);
					}
				}
				else {										// BGR形式
					BITMAPINFOHEADER bh;
					memset(&bh, 0, sizeof(bh));
					bh.biSize = sizeof(bh);
					bh.biWidth = ti.width;
					bh.biHeight = ti.height;
					bh.biPlanes = 1;
					bh.biBitCount = 24;
					bh.biCompression = BI_RGB;
					bh.biSizeImage = ti.size;
					Bitmap bm((BITMAPINFO*)&bh, pb);
					bitmap = load_core_gdip(bm, width, height);
				}
				free(pb);
			}
			break;
		}
		else {
			// リソースブロックデータを読み飛ばす
			if (size & 0x01)
				size++;
			fm.seek(size, SEEK_CUR);
			length -= (count + size + 11);
		}
	}
	return bitmap;
}

static Bitmap* LoadPSD(LPCTSTR pszName, int width, int height)
{
	filemap fm;
	if (fm.open(pszName))
		return psd_thumb_load(fm, width, height);
	return NULL;
}


///////////////////////////////////////////////////////////////////////
// Targa .tga

#pragma pack(1)
typedef struct tagTGAHEADER {
	BYTE idlen; 			// Length of Identifier String
	BYTE cmaptype;			// 0 = NoMap
	BYTE imagetype;			// Image Type (1,2,3,9,10,11)
	WORD cmapstart;			// Index of first colour map entry
	WORD cmaplen;			// Length of colour map (number of entries)
	BYTE cmapdepth;			// Length of colour map entry
	WORD xoff;				// X Origin of Image
	WORD yoff;				// Y Origin of Image
	WORD width;				// width of Image
	WORD height; 			// height of Image
	BYTE depth;				// PixelSize (8,16,24)
	BYTE descriptor;		// 00000000
							//     1111 ---- # of Attribute bits / pixel
							//   11     ---- orign location
							// 11       ---- unused (Interlieve)
} TGAHEADER;
#pragma pack()

typedef struct tagTGADECODE {
	TGAHEADER* pHeader;
	int bits;
	RGBQUAD* pColorMap;
	int RLE_count;
	int RLE_flag;
	int Red;
	int Grn;
	int Blu;
} TGADECODE;

// Definitions for Image Types

#define TGA_MAPRGB			1
#define TGA_RAWRGB	 		2
#define TGA_RAWMONO			3
#define TGA_MAPENCODE		9
#define TGA_RAWENCODE		10
#define TGA_MONOENCODE		11

// Define flags for mode - these indicate special conditions
#define GREYSC				0
#define COLOUR				1
#define MAPPED				2
#define RLENCD				4
#define INTERL				8
#define FOURWY				16


////////////////////////////////////////////////////////////////
//

static BOOL __fastcall tga_getmapentry(filemap& fm, RGBQUAD* prgbq, int depth, int mode)
{
	UINT l;
	int r, g, b;

	// read appropriate number of bytes, break into rgb & put in map
	switch (depth) {
		case 8:						// Grey Scale already, read and triplicate
			r = g = b = fm.getc();
			break;

		case 16: 					// 5 bits each of red green and blue
		case 15: 					// Watch for byte order
			l = fm.getshort();
			r = ((l >> 10) & 0x1F) << 3;
			g = ((l >> 5) & 0x1F) << 3;
			b = (l & 0x1F) << 3;
			break;

		case 32: 					// Read alpha byte & throw away
		case 24: 					// Eight bits each of red green and blue
			b = fm.getc();
			g = fm.getc();
			r = fm.getc();
			if (depth == 32)
				fm.getc();
			break;

		default:
			return FALSE;
	}
	prgbq->rgbRed = CLAMP(r);
	prgbq->rgbGreen = CLAMP(g);
	prgbq->rgbBlue = CLAMP(b);
	return TRUE;
}

static BOOL __fastcall tga_getpixel(filemap& fm, RGBQUAD* prgbq, TGADECODE* decode, int mode)
{
	static int Red, Grn, Blu;
	int i;
	UINT l = 0;

	// Check if run length encoded.
	if ((mode & RLENCD) != 0) {
		if (decode->RLE_count == 0) {			// Have to restart run
			i = fm.getc();
			decode->RLE_flag = (i & 0x80) >> 7;
			if (decode->RLE_flag == 0)			// Stream of unencoded pixels
				decode->RLE_count = i + 1;
			else								// Single pixel replicated
				decode->RLE_count = i - 127;
			decode->RLE_count--;				// Decrement count & get pixel
		}
		else {
			// Have already read count & (at least) first pixel
			decode->RLE_count--;
			if (decode->RLE_flag != 0)			// Replicated pixels
				goto PixEncode;
		}
	}

	// Read appropriate number of bytes, break into RGB
	switch (decode->pHeader->depth) {
		case 8:							// Grey Scale - read a byte and triplicate
			i = fm.getc();
			decode->Red = i;
			decode->Grn = i;
			decode->Blu = i;
			l = i;
			break;

		case 16: 						// Five bits each of red green and blue
		case 15: 						// Watch byte order
			l = fm.getshort();
			decode->Red = ((l >> 10) & 0x1F) << 3;
			decode->Grn = ((l >> 5) & 0x1F) << 3;
			decode->Blu = (l & 0x1F) << 3;
			break;

		case 32: 						// Read alpha byte & throw away
		case 24:				 		// Eight bits each of red green and blue
			decode->Blu = fm.getc();
			decode->Grn = fm.getc();
			decode->Red = fm.getc();
			if (decode->pHeader->depth == 32)
				fm.getc();
			l = 0;
			break;

		default:
			return FALSE;
	}

PixEncode:
	if ((mode & MAPPED) == MAPPED) {
		if (decode->pHeader->depth == 8)
			prgbq->rgbBlue = prgbq->rgbGreen = prgbq->rgbRed = CLAMP(l);
		else {
			prgbq->rgbBlue = decode->pColorMap[l].rgbBlue;
			prgbq->rgbGreen = decode->pColorMap[l].rgbGreen;
			prgbq->rgbRed = decode->pColorMap[l].rgbRed;
		}
	}
	else {
		prgbq->rgbBlue = CLAMP(decode->Blu);
		prgbq->rgbGreen = CLAMP(decode->Grn);
		prgbq->rgbRed = CLAMP(decode->Red);
	}
	return TRUE;
}

static BOOL __fastcall tga_read(filemap& fm, BYTE* pixels, TGADECODE* decode)
{
	register UINT i, j, iloop;
	UINT mode;
	ULONG k, width;

	TGAHEADER* tga = decode->pHeader;
	switch (tga->imagetype) {
		case TGA_MAPRGB:
		case TGA_RAWRGB:
		case TGA_RAWMONO:
		case TGA_MAPENCODE:
		case TGA_RAWENCODE:
		case TGA_MONOENCODE: break;
		default: return FALSE;
	}

	if (tga->imagetype == TGA_RAWMONO || tga->imagetype == TGA_MONOENCODE)
		mode = GREYSC;
	else
		mode = COLOUR;

	// ID文字列があれば、それを読みとばす
	if (tga->idlen != 0)
		fm.seek((long)tga->idlen, SEEK_CUR);

	// カラーマップ
	BOOL bOK;
	if (tga->cmaptype != 0) {
		for (i = 0;  i < tga->cmaplen; i++) {
			bOK = tga_getmapentry(fm, &decode->pColorMap[i], tga->cmapdepth, mode);
			if (!bOK)
				return FALSE;
		}

		if ((tga->imagetype != TGA_RAWRGB) &&
			(tga->imagetype != TGA_RAWMONO) &&
			(tga->imagetype != TGA_MONOENCODE) &&
			(tga->imagetype != TGA_RAWENCODE))
			mode = mode | MAPPED;
	}

	// Check Run Length Encoding
	if ((tga->imagetype == TGA_MAPENCODE) ||
		(tga->imagetype == TGA_MONOENCODE)||
		(tga->imagetype == TGA_RAWENCODE))
		mode = mode | RLENCD;

	// Check for interlacing of the Targa file
	switch (tga->descriptor >> 6) {
		case 2: mode = mode | FOURWY;	// Four way interlace
		case 1: mode = mode | INTERL;	// Two way interlace
		case 0: break;					// No interlace
		default: break;  				// Reserved - we'll let it pass
	}

	// DIBに配置する際の１ラインあたりのバイト数を求める
	width = WIDTHBYTES(tga->width * decode->bits);

	// TGAデータの展開
	for (i = 0; i < tga->height;  i++) {
		if ((mode & INTERL) == 0)				// No interlace
			j = i;
		else if ((mode & FOURWY) != 0) {		// Two way interlace
			if ((i << 1) < tga->height)
				j = i << 1;
			else {
				j = i - (tga->height >> 1);
				j = (j << 1) + 1;
			}
		}
		else {									// Four way interlace
			if ((i << 2) < tga->height)
				j = i << 2;
			else if ((i << 1) < tga->height) {
				j = i - (tga->height >> 2);
				j = (j << 2) + 1;
			}
			else if ((i << 2) < 3 * tga->height) {
				j = i - (tga->height >> 1);
				j = (j << 2) + 2;
			}
			else {
				j = i - (tga->height >> 1) - (tga->height >> 2);
				j = (j << 2) + 3;
			}
		}

		if (tga->descriptor & 0x30)
			k = (tga->height - 1 - j) * width;
		else
			k = j * width;

		RGBQUAD rgbq;
		BYTE* pStore = pixels + k;
		for (iloop = 0; iloop < tga->width; iloop++) {
			bOK = tga_getpixel(fm, &rgbq, decode, mode);
			if (!bOK)
				return FALSE;

			*pStore++ = rgbq.rgbBlue;
			if (tga->depth != 8) {
				*pStore++ = rgbq.rgbGreen;
				*pStore++ = rgbq.rgbRed;
			}
		}
	}
	return TRUE;
}

static Bitmap* LoadTGA(LPCTSTR pszName, int width, int height)
{
	filemap fm;
	if (!fm.open(pszName)) return NULL;

	TGAHEADER header;
	if ((fm.read(&header, 18)) != 18) return NULL;
	if (header.imagetype < 1 || header.imagetype > 11 || header.depth > 32) return NULL;

	// DIB作成のためのビットカウントの補正
	int bits;
	switch (header.depth) {
		case 8: bits = 8; break;
		case 15:
		case 16:
		case 24:
		case 32: bits = 24; break;
		default: return NULL;
	}

	memdib dib(header.width, header.height, bits);
	if (dib.IsNull()) return NULL;

	TGADECODE decode;
	memset(&decode, 0, sizeof(decode));
	decode.pHeader = &header;
	decode.bits = bits;
	decode.pColorMap = dib.ColorMap();

	Bitmap* bm = NULL;
	if (tga_read(fm, dib.BitmapBits(), &decode)) {
		bm = dib.GetThumbnail(width, height);
	}
	return bm;
}

////////////////////////////////////////////////////////////////////////
// Adobe PNM/PGM image .pnm, .pgm

// format = '1' PBM		-- 2bit color (ascii)
// format = '2'	PGM		-- gray scale (ascii)
// format = '3'	PPM		-- full color (ascii)
// format = '4'	PBM		-- 2bit color (bin)
// format = '5'	PGM		-- gray scale (bin)
// format = '6'	PPM		-- full color (bin)
// format = '7'			-- indexed color with 332 color map

static UINT pnm_integer(filemap& fm, UINT base)
{
	int c;
	UINT value;

	// Skip any leading whitespace.
	do {
		c = fm.getc();
		if (c == EOF) return 0;

		if (c == '#') {
			// skip comment.
			for ( ; (c != EOF) && (c != '\n');)
				c = fm.getc();
			continue;
		}
	} while (!isdigit(c));

	if (base == 2)
		return (c - '0');

	// Evaluate number.
	value = 0;
	do {
		value *= 10;
		value += c - '0';
		c = fm.getc();
		if (c == EOF) return 0;
	} while (_ISDIGIT(c));
	return value;
}

static void pnm_read_type1(filemap& fm, BYTE* pixels, int width, int height)
{
	for (int y = 0; y < height; y++) {
		BYTE* p = pixels + (height - y - 1) * WIDTHBYTES(8 * width);
		for (int x = 0; x < width; x++)
			*p++ = (BYTE)(!pnm_integer(fm, 2));
	}
}

static void pnm_read_type2(filemap& fm, BYTE* pixels, int width, int height)
{
	for (int y = 0; y < height; y++) {
		BYTE* p = pixels + (height - y - 1) * WIDTHBYTES(8 * width);
		for (int x = 0; x < width; x++)
			*p++ = (BYTE)pnm_integer(fm, 10);
	}
}

static void pnm_read_type3(filemap& fm, BYTE* pixels, int width, int height, BYTE* scale)
{
	int r, g, b;
	for (int y = 0; y < height; y++) {
		BYTE* p = pixels + (height - y - 1) * WIDTHBYTES(24 * width);
		for (int x = 0; x < width; x++) {
			r = pnm_integer(fm, 10);
			g = pnm_integer(fm, 10);
			b = pnm_integer(fm, 10);
			if (scale) {
				r = scale[r];
				g = scale[g];
				b = scale[b];
			}
			*p++ = CLAMP(b);
			*p++ = CLAMP(g);
			*p++ = CLAMP(r);
		}
	}
}

static void pnm_read_type4(filemap& fm, BYTE* pixels, int width, int height)
{
	for (int y = 0; y < height; y++) {
		BYTE* p = pixels + (height - y - 1) * WIDTHBYTES(8 * width);
		BYTE bit = 0;
		BYTE byte = 0;
		for (int x = 0; x < width; x++) {
			if (bit == 0)
				byte = (BYTE)fm.getc();
			*p++ = (byte & 0x80) ? 0 : 1;
			bit++;
			if (bit == 8)
				bit = 0;
			byte <<= 1;
		}
	}
}

static void pnm_read_type57(filemap& fm, BYTE* pixels, int width, int height, UINT max_value)
{
	for (int y = 0; y < height; y++) {
		BYTE* p = pixels + (height - y - 1) * WIDTHBYTES(8 * width);
		for (int x = 0; x < width; x++) {
			UINT index;
			if (max_value <= 255)
				index = fm.getc();
			else
				index = fm.getshort();

			if (index > max_value) index = max_value;
			*p++ = CLAMP(index);
		}
	}
}

static void pnm_read_type6(filemap& fm, BYTE* pixels, int width, int height, BYTE* scale, UINT max_value)
{
	int r, g, b;
	for (int y = 0; y < height; y++) {
		BYTE* p = pixels + (height - y - 1) * WIDTHBYTES(24 * width);
		for (int x = 0; x < width; x++) {
			if (max_value <= 255) {
				r = fm.getc();
				g = fm.getc();
				b = fm.getc();
			}
			else {
				r = fm.getshort();
				g = fm.getshort();
				b = fm.getshort();
			}

			if (scale) {
				r = scale[min(r, (int)max_value)];
				g = scale[min(g, (int)max_value)];
				b = scale[min(b, (int)max_value)];
			}
			*p++ = CLAMP(b);
			*p++ = CLAMP(g);
			*p++ = CLAMP(r);
		}
	}
}

static Bitmap* pnm_read(filemap& fm, int width, int height)
{
	// Read PNM image.
	int format = fm.getc();
	// Verify PNM identifier.
	if (format != 'P') return NULL;

	// Initialize image structure.
	format = fm.getc();
	if (format < '1' || format > '7') return NULL;

	if (format == '7') pnm_integer(fm, 10);

	int columns = pnm_integer(fm, 10);
	int rows = pnm_integer(fm, 10);

	UINT max_value;
	if (format == '1' || format == '4')
		max_value = 1;			// bitmap
	else
		max_value = pnm_integer(fm, 10);

	bool indexed_color = false;
	int colors = 0;
	if (format != '3' && format != '6') {
		indexed_color = true;
		colors = min(max_value, 255) + 1;
	}

	if (columns * rows == 0) return NULL;

	memdib dib(columns, rows, (indexed_color ? 8 : 24));
	if (dib.IsNull()) return NULL;

	int i;
	BYTE* scale = NULL;
	if (indexed_color) {
		// Create colormap.
		RGBQUAD* pc = dib.ColorMap();
		if (format != '7') {
			for (i = 0; i < 256; i++) {
				BYTE b = (BYTE)((255 * i) / (colors - 1));
				pc->rgbRed = pc->rgbGreen = pc->rgbBlue = b;
				pc++;
			}
		}
		else {
			// Initialize 332 colormap.
			i = 0;
			for (int r = 0; r < 8; r++) {
				for (int g = 0; g < 8; g++) {
					for (int b = 0; b < 4; b++) {
						pc->rgbRed   = (BYTE)((r * 255) / 7);
						pc->rgbGreen = (BYTE)((g * 255) / 7);
						pc->rgbBlue  = (BYTE)((b * 255) / 3);
						pc++;
					}
				}
			}
		}
	}
	else if (max_value != 255) {
		// Compute pixel scaling table.
		scale = (BYTE*)malloc(max_value + 1);
		if (scale) {
			for (i = 0; i <= (int)max_value; i++) {
				scale[i] = (BYTE)((i * 255 + (max_value >> 1)) / max_value);
			}
		}
	}

	switch (format) {
		case '1': pnm_read_type1(fm, dib.BitmapBits(), columns, rows); break;
		case '2': pnm_read_type2(fm, dib.BitmapBits(), columns, rows); break;
		case '3': pnm_read_type3(fm, dib.BitmapBits(), columns, rows, scale); break;
		case '4': pnm_read_type4(fm, dib.BitmapBits(), columns, rows); break;
		case '5':
		case '7': pnm_read_type57(fm, dib.BitmapBits(), columns, rows, max_value); break;
		case '6': pnm_read_type6(fm, dib.BitmapBits(), columns, rows, scale, max_value); break;
	}
	if (scale) free(scale);

	return dib.GetThumbnail(width, height);
}

static Bitmap* LoadPNM(LPCTSTR pszName, int width, int height)
{
	filemap fm;
	if (fm.open(pszName))
		return pnm_read(fm, width, height);
	return NULL;
}


///////////////////////////////////////////////////////////////////////
// PC paintbrush .pcx


static BYTE mono[6]= {0, 0, 0, 255, 255, 255};

#pragma pack(1)
typedef struct tagPCXHEADER {
    BYTE manufacturer;
    BYTE version;
    BYTE compression;
    BYTE bpp;
    short x1, y1;
    short x2, y2;
    short prn_hdpi;
    short prn_vdpi;
    BYTE colormap[48];
    BYTE reserved;
    BYTE planes;
    short bytesperline;
    short color;
	short video_hdpi;
	short video_vdpi;
    BYTE filler[54];
} PCXHEADER;
#pragma pack()


static void __fastcall pcx_setcmap(LPBITMAPINFO pbi, BYTE* cmap, int cnt)
{
	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)pbi;
	pbh->biClrUsed = cnt;
	pbh->biClrImportant = cnt;
	for (int i = 0; i < cnt; i++) {
		pbi->bmiColors[i].rgbRed = *cmap++;
		pbi->bmiColors[i].rgbGreen = *cmap++;
		pbi->bmiColors[i].rgbBlue = *cmap++;
	}
}

static void __fastcall pcx_readline(filemap& fm, BYTE* buffer, PCXHEADER* hdr)
{
	static BYTE count= 0, value= 0;
	int bytes = hdr->bytesperline;
	if (hdr->compression) {
		while (bytes--) {
			if (count == 0) {
				value = (BYTE)fm.getc();
				if (value < 0xc0)
					count= 1;
				else {
					count= value - 0xc0;
					value= (BYTE)fm.getc();
				}
			}
			count--;
			*(buffer++)= value;
		}
	}
	else
		fm.read(buffer, bytes);
}

static void __fastcall pcx_load_8(filemap& fm, int width, int height, BYTE *buffer, PCXHEADER* hdr)
{
	BYTE* line = (BYTE*)malloc(hdr->bytesperline);
	for (int row = 0; row < height; buffer+= width, ++row) {
		pcx_readline(fm, line, hdr);
		memcpy(buffer, line, width);
	}
	free(line);
}

static void __fastcall pcx_load_24(filemap& fm, int width, int height, BYTE *buffer, PCXHEADER* hdr)
{
	register int x, y, c;
	BYTE* line = (BYTE*)malloc(hdr->bytesperline * 3);
	for (y = 0; y < height; buffer+= width * 3, ++y) {
		for (c = 0; c < 3; ++c) {
			pcx_readline(fm, line, hdr);
			for (x = 0; x < width; ++x)
				buffer[x * 3 + c]= line[x];
		}
	}
	free(line);
}

static void __fastcall pcx_load_1(filemap& fm, int width, int height, BYTE* buffer, PCXHEADER* hdr)
{
	register int x, y;
	BYTE* line = (BYTE*)malloc(hdr->bytesperline);
	for (y = 0; y < height; buffer+= width, ++y) {
		pcx_readline(fm, line, hdr);
		for (x = 0; x < width; ++x) {
			if (line[x / 8] & (128 >> (x % 8)))
				buffer[x] = 1;
			else
				buffer[x] = 0;
		}
	}
	free(line);
}

static void __fastcall pcx_load_4(filemap& fm, int width, int height, BYTE *buffer, PCXHEADER* hdr)
{
	register int x, y, c;
	BYTE* line = (BYTE*)malloc(hdr->bytesperline);
	for (y = 0; y < height; buffer += width, ++y) {
		memset(buffer, 0, width);
		for (c = 0; c < 4; ++c) {
			pcx_readline(fm, line, hdr);
			for (x = 0; x < width; ++x) {
				if (line[x / 8] & (128 >> (x % 8)))
					buffer[x] += (BYTE)(1 << c);
			}
		}
	}
	free(line);
}

static Bitmap* pcx_read(filemap& fm, int width, int height)
{
	PCXHEADER hdr;
	if (fm.read(&hdr, 128) != 128 || hdr.manufacturer != 10) return NULL;

	int offset_x = (int)hdr.x1;
	int offset_y = (int)hdr.y1;
	int cx = (int)hdr.x2 - offset_x + 1;
	int cy = (int)hdr.y2 - offset_y + 1;

	int bits = (hdr.planes == 3 && hdr.bpp == 8) ? 24 : 8;
	memdib dib(cx, cy, bits);
	if (dib.IsNull()) return NULL;

	BYTE* dest = (BYTE*)malloc(cx * cy * 3);
	if (hdr.planes == 1 && hdr.bpp == 1) {
		pcx_load_1(fm, cx, cy, dest, &hdr);
		pcx_setcmap(dib.BitmapInfo(), mono, 2);
	}
	else if (hdr.planes == 4 && hdr.bpp == 1) {
		pcx_load_4(fm, cx, cy, dest, &hdr);
		pcx_setcmap(dib.BitmapInfo(), hdr.colormap, 16);
	}
	else if (hdr.planes == 1 && hdr.bpp == 8) {
		pcx_load_8(fm, cx, cy, dest, &hdr);
		BYTE cmap[769];
		fm.seek(-769, SEEK_END);
		fm.read(cmap, 769);
		if (cmap[0] == 0x0c)
			pcx_setcmap(dib.BitmapInfo(), &cmap[1], 256);
	}
	else if (hdr.planes == 3 && hdr.bpp == 8) {
		pcx_load_24(fm, cx, cy, dest, &hdr);
	}
	else {
		free(dest);
		return NULL;
	}

	register BYTE* pixels = dest;
	for (int y = 0; y < cy; y++) {
//@		register BYTE* ps = dib.BitmapBits() + dib.WidthBytes() * (cy - y - 1);
		register BYTE* ps = dib.RowPosition(y);
		for (int x = 0; x < cx; x++) {
			if (bits == 24) {
				*ps++ = pixels[2];
				*ps++ = pixels[1];
				*ps++ = pixels[0];
				pixels += 3;
			}
			else
				*ps++ = *pixels++;
		}
	}
	free(dest);

	return dib.GetThumbnail(width, height);
}

static Bitmap* LoadPCX(LPCTSTR pszName, int width, int height)
{
	filemap fm;
	if (fm.open(pszName))
		return pcx_read(fm, width, height);
	return NULL;
}

///////////////////////////////////////////////////////////////////////
// Converts from a Sun Rasterfile to a DIB

#define ROLD			0 /* old format, see /usr/include/rasterfile.h */
#define RSTANDARD		1 /* standard format */
#define RRLENCODED		2 /* run length encoding to compress the image */
#define RRGB			3 /* RGB-format instead of BGR in 24 or 32-bit mode */
#define RTIFF			4 /* TIFF <-> rasterfile */
#define RIFF			5 /* IFF (TAAC) <-> rasterfile */

/* these are the possible colormap types.  if it's in RGB format,
 * the map is made up of three byte arrays (red, green, then blue)
 * that are each 1/3 of the colormap length.
 */

#define RNOMAP			0 /* no colormap follows the header */
#define RRGBMAP			1 /* rgb colormap */
#define RRAWMAP			2 /* raw colormap; good luck */
#define RESC			128 /* run-length encoding escape character */

#pragma pack(1)
typedef struct tagRASHDR {
	DWORD magic;
	DWORD width;
	DWORD height;
	DWORD depth;
	DWORD length;
	DWORD type;
	DWORD cmaptype;
	DWORD cmaplength;
} RASHDR;
typedef RASHDR* LPRASHDR;
#pragma pack()

static void ras_readline(filemap& fm, BYTE* buf, int len, BOOL enc)
{
	static BYTE repchar, remaining = 0;
	if (enc) {		/* rl-encoded read */
		while (len--) {
			if (remaining) {
				remaining--;
				*(buf++)= repchar;
			}
			else {
				repchar = (BYTE)fm.getc();
				if (repchar == RESC) {
					remaining = (BYTE)fm.getc();
					if (remaining == 0)
						*(buf++)= RESC;
					else {
						repchar = (BYTE)fm.getc();
						*(buf++)= repchar;
					}
				}
				else
					*(buf++)= repchar;
			}
		}
	}
	else		/* normal read */
		fm.read(buf, len);
}

static Bitmap* ras_read(filemap& fm, int width, int height)
{
	// ヘッダ情報の取得とRASファイルチェック
	RASHDR hdr;
	hdr.magic = fm.getlong();
	hdr.width = fm.getmsblong();
	hdr.height = fm.getmsblong();
	hdr.depth = fm.getmsblong();
	hdr.length = fm.getmsblong();
	hdr.type = fm.getmsblong();
	hdr.cmaptype = fm.getmsblong();
	hdr.cmaplength = fm.getmsblong();

	if (hdr.magic != 0x956aa659) return NULL;

	int bits = hdr.depth;
	int cx = hdr.width;
	int cy = hdr.height;
	if ((cx * cy) == 0) return NULL;

	bool enc = false;
	bool rgb_format = false;
	switch (hdr.type) {
		case ROLD:
		case RSTANDARD: break;
		case RRLENCODED: enc = true; break;
		case RRGB: rgb_format = true; break;

 		// sorry, don't even know what these are
 		case RTIFF:
		case RIFF:
		default: return NULL;
	}

	memdib dib(cx, cy, bits);
	if (dib.IsNull()) return NULL;

	// パレットの設定
	int x, y;
	if (hdr.cmaptype == RRGBMAP) {
		int mapsize = hdr.cmaplength / 3;
		BITMAPINFOHEADER* pbh = dib.BitmapInfoHeader();
		pbh->biClrUsed = pbh->biClrImportant = mapsize;
		RGBQUAD* pc = dib.ColorMap();
		for (x = 0; x < mapsize; x++)
			pc[x].rgbRed = (BYTE)fm.getc();
		for (x = 0; x < mapsize; x++)
			pc[x].rgbGreen = (BYTE)fm.getc();
		for (x = 0; x < mapsize; x++)
			pc[x].rgbBlue = (BYTE)fm.getc();
	}

	int fill_len = (dib.WidthBytes() % 2) != 0 ? 1 : 0;	// # of fill bytes per raster line

	BYTE fillchar;
	BYTE* pw;
	if (bits >= 24) {
		int packsize = (bits == 24) ? 3 : 4;
		BYTE* buf = (BYTE*)malloc(cx * packsize);
		for (y = 0; y < cy; y++) {
			ras_readline(fm, buf, cx * packsize, enc);

			BYTE* bp = buf;
//@			pw = dib.BitmapBits() + dib.WidthBytes() * (cy - y - 1);
			pw = dib.RowPosition(y);
			if (bits == 24) {
				if (rgb_format) {
					for (x = 0; x < cx; x++) {
						*pw++ = *(bp + 2); /* red */
						*pw++ = *(bp + 1); /* green */
						*pw++ = *bp;       /* blue */
						bp += 3;
					}
				}
				else {
					for (x = 0; x < cx; x++) {
						*pw++ = *bp++; /* red */
						*pw++ = *bp++; /* green */
						*pw++ = *bp++; /* blue */
					}
				}
			}
			else {
				if (rgb_format) {
					for (x = 0; x < cx; x++) {
						*pw++ = *(bp + 3); /* red */
						*pw++ = *(bp + 2); /* green */
						*pw++ = *(bp + 1); /* blue */
						bp += 4;
					}
				}
				else {
					for (x = 0; x < cx; x++) {
						bp++;                  /* skip byte */
						*pw++ = *bp++; /* red */
						*pw++ = *bp++; /* green */
						*pw++ = *bp++; /* blue */
					}
				}
			}
			if (fill_len)
				ras_readline(fm, &fillchar, fill_len, enc);
		}
		free(buf);
	}
	else {
		for (y = 0; y < cy; y++) {
//@			pw = dib.BitmapBits() + dib.WidthBytes() * (cy - y - 1);
			pw = dib.RowPosition(y);
			ras_readline(fm, pw, cx, enc);
			pw += cx;
			if (fill_len)
				ras_readline(fm, &fillchar, fill_len, enc);
		}
	}

	return dib.GetThumbnail(width, height);
}

static Bitmap* LoadRAS(LPCTSTR pszName, int width, int height)
{
	filemap fm;
	if (fm.open(pszName))
		return ras_read(fm, width, height);
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// SGI file

/* GIMP
 * "$Id: sgilib.c,v 1.8 2003/04/07 11:59:33 neo Exp $"
 *   SGI image file format library routines.
 *   Copyright 1997-1998 Michael Sweet (mike@easysw.com)
 */

#define SGI_MAGIC	474	/* Magic number in image file */
#define SGI_COMP_NONE	0	/* No compression */
#define SGI_COMP_RLE	1	/* Run-length encoding */
#define SGI_COMP_ARLE	2	/* Agressive run-length encoding */


////////////////////////////////////////////////////////////////////////
// Image structure...

typedef struct {
	filemap	*file;			/* Image file */
	int		bpp,			/* Bytes per pixel/channel */
			comp;			/* Compression */
	USHORT	dim;			/* Dimension */
	USHORT	xsize,			/* Width in pixels */
			ysize,			/* Height in pixels */
			zsize;			/* Number of channels */
	long	pixMin,
			pixMax;
	long	colMap;
	long	*table;			/* Offset table for compression */
} sgi_t;

////////////////////////////////////////////////////////////////////////
// 'read_rle8()' - Read 8-bit RLE data.

static int sgiread_rle8(sgi_t* sgip, BYTE* row, int width)
{
	BYTE* rowBase = row;
	while (width > 0) {
		int ch = sgip->file->getc();
		if (ch == EOF) return (-1);

		int count = ch & 127;
		if (count == 0) break;

		if (ch & 128)
			sgip->file->read(row, count);
		else {
			ch = sgip->file->getc();
			memset(row, ch, count);
		}
		row += count;
		width -= count;
	}

	int len;
#ifdef _WIN64
	len = (int)(row - rowBase);
#else
	len = PtrToInt(row - rowBase);
#endif // _WIN64
	return (width > 0 ? -1 : len);
}

////////////////////////////////////////////////////////////////////////
// 'read_rle16()' - Read 16-bit RLE data.

static int sgiread_rle16(sgi_t* sgip, BYTE* row, int width)
{
	int i;
	int len = 0;
	while (width > 0) {
		int ch = sgip->file->getmsbshort();
		if (ch == EOF)
			return (-1);
		len++;

		int count = ch & 127;
		if (count == 0) break;

		if (ch & 128) {
			for (i = 0; i < count; i++, width--, len++)
				*row++ = (BYTE)(sgip->file->getmsbshort() >> 8);
		}
		else {
			ch = (sgip->file->getmsbshort() >> 8);
			len++;
			for (i = 0; i < count; i++, width--)
				*row++ = (BYTE)ch;
		}
	}
	return (width > 0 ? -1 : len * 2);
}

////////////////////////////////////////////////////////////////////////
// 'sgiClose()' - Close an SGI image file.

static int sgiClose(sgi_t* sgip)
{
	if (sgip == NULL) return (-1);
	if (sgip->table != NULL) free(sgip->table);
	if (sgip->file) {
		sgip->file->close();
		delete sgip->file;
	}
	free(sgip);
	return 0;
}

////////////////////////////////////////////////////////////////////////
// 'sgiGetRow()' - Get a row of image data from a file.

static int sgiGetRow(sgi_t* sgip, BYTE* row, int y, int z)
{
	long offset;		/* File offset */
	if (sgip == NULL || row == NULL || y < 0 || y >= sgip->ysize || z < 0 || z >= sgip->zsize)
		return (-1);

	switch (sgip->comp) {
    case SGI_COMP_NONE:
		// Seek to the image row - optimize buffering by only seeking if necessary...
		offset = 512 + (y + z * sgip->ysize) * sgip->xsize * sgip->bpp;
		if (offset != sgip->file->seek(0, SEEK_CUR))
			sgip->file->seek(offset, SEEK_SET);

		if (sgip->bpp == 1)
			sgip->file->read(row, sgip->xsize);
		else {
			for (int x = 0; x < sgip->xsize; x++)
				*row++ = (BYTE)(sgip->file->getmsbshort() >> 8);
		}
		break;

	case SGI_COMP_RLE:
		offset = sgip->table[y + sgip->ysize * z];
		if (offset != sgip->file->seek(0, SEEK_CUR))
			sgip->file->seek(offset, SEEK_SET);

		if (sgip->bpp == 1)
			return (sgiread_rle8(sgip, row, sgip->xsize));
		else
			return (sgiread_rle16(sgip, row, sgip->xsize));
		break;
	}
	return (0);
}

////////////////////////////////////////////////////////////////////////
// 'sgiOpenFile()' - Open an SGI image file for reading or writing.

static sgi_t* sgiOpen(LPCTSTR filename)
{
	sgi_t* sgip = (sgi_t*)calloc(sizeof(sgi_t), 1);
	if (sgip == NULL) return NULL;

	filemap* file = new filemap();
	if (file == NULL || !file->open(filename)) {
		free(sgip);
		return NULL;
	}

	sgip->file = file;
	short magic = sgip->file->getmsbshort();
	if (magic != SGI_MAGIC) {
		delete file;
		free(sgip);
		return NULL;
	}

	sgip->comp   = sgip->file->getc();
	sgip->bpp    = sgip->file->getc();
	sgip->dim    = sgip->file->getmsbshort();
	sgip->xsize  = sgip->file->getmsbshort();
	sgip->ysize  = sgip->file->getmsbshort();
	sgip->zsize  = sgip->file->getmsbshort();
	sgip->pixMin = sgip->file->getmsblong();
	sgip->pixMax = sgip->file->getmsblong();
	sgip->file->seek(84, SEEK_CUR);
	sgip->colMap = sgip->file->getmsblong();

	if (sgip->comp) {
		// This file is compressed; read the scanline tables...
		sgip->file->seek(512, SEEK_SET);
		int tblcount = sgip->ysize * sgip->zsize;
		sgip->table = (long*)malloc(tblcount * sizeof(long));
		for (int i = 0; i < tblcount; i++)
			sgip->table[i] = sgip->file->getmsblong();
	}
	return sgip;
}

////////////////////////////////////////////////////////////////////////
//

static Bitmap* LoadSGI(LPCTSTR pszName, int width, int height)
{
	sgi_t* sgip = sgiOpen(pszName);
	if (sgip == NULL) return NULL;

	int i;
	int bits;
	switch (sgip->zsize) {
	case 1:		// Grayscale
	case 2:		// Grayscale + alpha
		bits = 8; break;
	case 3:		// RGB
		bits = 24; break;
	case 4:		// RGBA
		bits = 32; break;
	default:
		sgiClose(sgip); return NULL;
	}

	memdib dib(sgip->xsize, sgip->ysize, bits);
	if (dib.IsNull()) {
		sgiClose(sgip);
		return NULL;
	}

	if (bits == 8) {
		BITMAPINFOHEADER* pbh = dib.BitmapInfoHeader();
		pbh->biClrUsed = pbh->biClrImportant = 256;
		RGBQUAD* pc = dib.ColorMap();
		for (i = 0; i < 256; i++) {
			pc->rgbRed = pc->rgbGreen = pc->rgbBlue = (BYTE)i;
			pc++;
		}
	}

	BYTE* row[4];
	row[0] = (BYTE*)malloc(sizeof(BYTE) * sgip->xsize * sgip->zsize);
	for (i = 1; i < sgip->zsize; i++)
		row[i] = row[0] + i * sgip->xsize;

	bool bOK = true;
	int x, y;
	for (y = 0; y < sgip->ysize; y++) {
		BYTE* p = dib.BitmapBits() + dib.WidthBytes() * y;
		for (i = 0; i < sgip->zsize; i++) {
			if (sgiGetRow(sgip, row[i], y, i) < 0) {
				bOK = false;
				break;
			}
		}
		if (!bOK) break;

		for (x = 0; x < sgip->xsize; x++) {
			switch (sgip->zsize) {
			case 1:
			case 2:
				*p++ = row[0][x];
				break;

			case 4:
				*p++ = row[3][x];
				*p++ = row[2][x];
				*p++ = row[1][x];
				*p++ = row[0][x];
				break;

			case 3:
				*p++ = row[2][x];
				*p++ = row[1][x];
				*p++ = row[0][x];
				break;
			}
		}
	}
	sgiClose(sgip);
	free(row[0]);

	return dib.GetThumbnail(width, height);
}

////////////////////////////////////////////////////////////////////////
//

#define MaxTextExtent  		1664

static int xbm_integer(filemap& fm, short int* hex_digits)
{
	int c, flag, value;
	value = 0;
	flag = 0;
	for ( ;; ) {
		c = fm.getc();
		if (c == EOF) {
			value = (-1);
			break;
		}
		c &= 0xff;
		if (isxdigit(c)) {
			value = (value << 4) + hex_digits[c];
			flag++;
			continue;
		}
		if ((hex_digits[c]) < 0 && flag)
			break;
	}
	return value;
}

static Bitmap* LoadXBM(LPCTSTR pszName, int width, int height)
{
	char buf[MaxTextExtent], name[MaxTextExtent];
	register int x, y;
	SHORT hex_digits[256];
	BYTE bit;
	UINT value;

	filemap fm;
	if (!fm.open(pszName)) return NULL;

	// Read X bitmap header.
	int columns = 0, rows = 0;
	while (fm.gets(buf, ARRAYSIZE(buf))) {
		if (sscanf_s(buf, "#define %s %u", name, (int)ARRAYSIZE(name), &columns) == 2)
			if (lstrlenA(name) >= 6 && !StrCmpIA(name + lstrlenA(name)-6, "_width"))
				break;
	}

	while (fm.gets(buf, ARRAYSIZE(buf))) {
		if (sscanf_s(buf,"#define %s %u", name, (int)ARRAYSIZE(name), &rows) == 2)
			if (lstrlenA(name) >= 7 && !StrCmpIA(name + lstrlenA(name)-7, "_height"))
				break;
	}

	// Scan until hex digits.
	BYTE* p;
	UINT version = 11;
	while (fm.gets(buf, ARRAYSIZE(buf))) {
		if (sscanf_s(buf, "static short %s = {", name, (int)ARRAYSIZE(name)) == 1)
			version = 10;
		else if (sscanf_s(buf, "static unsigned char %s = {", name, (int)ARRAYSIZE(name)) == 1)
			version = 11;
		else if (sscanf_s(buf, "static char %s = {", name, (int)ARRAYSIZE(name)) == 1)
			version = 11;
		else
			continue;

		p = (BYTE*)StrRChrA(name, NULL, '_');
		if (!p)
			p = (BYTE*)name;
		else
			p++;
		if (!StrCmpIA("bits[]", (char*)p))
			break;
	}

	if (columns == 0 || rows == 0 || fm.eof()) {
		fm.close();
		return NULL;
	}

	// Initialize image structure.
	memdib dib(columns, rows, 8);
	RGBQUAD* pc = dib.ColorMap();
	pc->rgbRed = pc->rgbGreen = pc->rgbBlue = 0;
	pc++;
	pc->rgbRed = pc->rgbGreen = pc->rgbBlue = 255;
	BITMAPINFOHEADER* pbh = dib.BitmapInfoHeader();
	pbh->biClrUsed = pbh->biClrImportant = 2;

	UINT padding = 0;
	if ((columns % 16) && ((columns % 16) < 9) && (version == 10))
		padding = 1;
	UINT bytes_per_line = (columns + 7) / 8 + padding;
	BYTE* data = (unsigned char*)malloc(bytes_per_line * rows * sizeof(BYTE*));
	if (!data) {
		fm.close();
		return NULL;
	}

	// Initialize hex values.
	hex_digits['0']=0;		hex_digits['1']=1;		hex_digits['2']=2;
	hex_digits['3']=3;		hex_digits['4']=4;		hex_digits['5']=5;
	hex_digits['6']=6;		hex_digits['7']=7;		hex_digits['8']=8;
	hex_digits['9']=9;		hex_digits['A']=10;		hex_digits['B']=11;
	hex_digits['C']=12;		hex_digits['D']=13;		hex_digits['E']=14;
	hex_digits['F']=15;		hex_digits['a']=10;		hex_digits['b']=11;
	hex_digits['c']=12;		hex_digits['d']=13;		hex_digits['e']=14;
	hex_digits['f']=15;		hex_digits['x']=0;		hex_digits[' ']=(-1);
	hex_digits[',']=(-1);	hex_digits['}']=(-1);	hex_digits['\n']=(-1);
	hex_digits['\t']=(-1);

	// Read hex image data.
	p = data;
	if (version == 10) {
		for (x = 0; x < (int)(bytes_per_line * rows); (x += 2)) {
			value = xbm_integer(fm, hex_digits);
			*p++ = (BYTE)value;
			if (!padding || ((x+2) % bytes_per_line))
				*p++ = (BYTE)(value >> 8);
		}
	}
	else {
		for (x = 0; x < (int) (bytes_per_line * rows); x++) {
			value = xbm_integer(fm, hex_digits);
			*p++ = (BYTE)value;
		}
	}
	fm.close();

	// Convert X bitmap image to runlength-encoded packets.
	UINT byte = 0;
	p = data;
	for (y = 0; y < rows; y++) {
		BYTE* q = dib.BitmapBits() + dib.WidthBytes() * y;
		bit = 0;
		for (x = 0; x < columns; x++) {
			if (bit == 0)
				byte = *p++;
			BYTE index = byte & 0x01 ? 0 : 1;
			*q++ = index;
			bit++;
			byte >>= 1;
			if (bit == 8)
				bit = 0;
		}
	}
	free(data);

	return dib.GetThumbnail(width, height);
}

////////////////////////////////////////////////////////////////////////
//

#define UPSHIFTED(x)			((int)((x) * (1L << 14) + 0.5))
#define DOWNSHIFT(x)			(((int)((x) + (1L << 13))) >> 14)
#define MAXRGB					255

/* Photo CD information beyond 100% white, Gamma 2.2 */
static BYTE YCCMap[351] = {
        0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,
        14, 15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,
        28, 29,  30,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,
        43, 45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  56,  57,  58,
        59, 60,  61,  62,  63,  64,  66,  67,  68,  69,  70,  71,  72,  73,
        74, 76,  77,  78,  79,  80,  81,  82,  83,  84,  86,  87,  88,  89,
        90, 91,  92,  93,  94,  95,  97,  98,  99, 100, 101, 102, 103, 104,
      105, 106, 107, 108, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
      120, 121, 122, 123, 124, 125, 126, 127, 129, 130, 131, 132, 133, 134,
      135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148,
      149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162,
      163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176,
      176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189,
      190, 191, 192, 193, 193, 194, 195, 196, 197, 198, 199, 200, 201, 201,
      202, 203, 204, 205, 206, 207, 207, 208, 209, 210, 211, 211, 212, 213,
      214, 215, 215, 216, 217, 218, 218, 219, 220, 221, 221, 222, 223, 224,
      224, 225, 226, 226, 227, 228, 228, 229, 230, 230, 231, 232, 232, 233,
      234, 234, 235, 236, 236, 237, 237, 238, 238, 239, 240, 240, 241, 241,
      242, 242, 243, 243, 244, 244, 245, 245, 245, 246, 246, 247, 247, 247,
      248, 248, 248, 249, 249, 249, 249, 250, 250, 250, 250, 251, 251, 251,
      251, 251, 252, 252, 252, 252, 252, 253, 253, 253, 253, 253, 253, 253,
      253, 253, 253, 253, 253, 253, 254, 254, 254, 254, 254, 254, 254, 254,
      254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
      255
};

#define B		(MAXRGB + 1) * 2
#define G		(MAXRGB + 1)
#define R		0

enum {
	COLSPC_SRGB,
	COLSPC_YCC,
};

typedef struct tagPCDTABLE {
	UINT length;
	UINT sequence;
	BYTE key;
	UINT mask;
} PCDTABLE;


void pcd_upsample(int width, int height, int scaled_width, BYTE* pixels)
{
	register int x, y;
	register BYTE *p, *q, *r;

	// Create a new image that is a integral size greater than an existing one.
	for (y = 0; y < height; y++) {
		p = pixels + (height - 1 - y) * scaled_width + (width - 1);
		q = pixels + ((height - 1 - y) << 1) * scaled_width + ((width - 1) << 1);
		*q = *p;
		*(q + 1) = *p;
		for (x = 1; x < width; x++) {
			p--;
			q -= 2;
			*q = *p;
			*(q + 1) = (BYTE)((((int)*p) + ((int)*(p + 1)) + 1) >> 1);
		}
	}

	for (y = 0; y < height - 1; y++) {
		p = pixels + (y << 1) * scaled_width;
		q = p + scaled_width;
		r = q + scaled_width;
		for (x = 0; x < width - 1; x++) {
			*q = (BYTE)((((int)*p) + ((int)*r) + 1) >> 1);
			*(q + 1) = (BYTE)((((int)*p) + ((int)*(p + 2)) + ((int)*r) + ((int)*(r + 2)) + 2) >> 2);
			q += 2;
			p += 2;
			r += 2;
		}
		*q++ = (BYTE)((((int)*p++) + ((int)*r++) + 1) >> 1);
		*q++ = (BYTE)((((int)*p++) + ((int)*r++) + 1) >> 1);
	}
	p = pixels + (2 * height - 2) * scaled_width;
	q = pixels + (2 * height - 1) * scaled_width;
	for (x = 0; x < width; x++) {
		*q++ = *p++;
		*q++ = *p++;
	}
}

static void pcd_ycc2rgb(memdib& dib)
{
	// Allocate the tables.
	long* basetable = (long*)malloc(9 * (MAXRGB + 1) * sizeof(long));
	if (!basetable) return;

	long* rtb = basetable;
	long* gtb = &basetable[3 * (MAXRGB + 1)];
	long* btb = &basetable[6 * (MAXRGB + 1)];

	BYTE* range_table = (BYTE*)malloc(4 * (MAXRGB + 1) * sizeof(BYTE));
	if (!range_table) return;

	// Initialize tables.
	int i;
	for (i = 0; i <= MAXRGB; i++) {
		range_table[i] = 0;
		range_table[i + (MAXRGB + 1)] = (BYTE)i;
		range_table[i + (MAXRGB + 1) * 2] = MAXRGB;
	}
	for (i = 0; i <= MAXRGB; i++)
		range_table[i + (MAXRGB + 1) * 3] = MAXRGB;

	BYTE* range_limit = range_table + (MAXRGB + 1);
	// Initialize YCC tables:
	//		R = Y            +1.340762*C2
	//		G = Y-0.317038*C1-0.682243*C2
	//		B = Y+1.632639*C1
	// YCC is scaled by 1.3584.  C1 zero is 156 and C2 is at 137.
	for (i = 0; i <= MAXRGB; i++) {
		rtb[i + R] = UPSHIFTED(1.35840) * i;
		gtb[i + R] = 0;
		btb[i + R] = UPSHIFTED(1.82150) * (i - 137);
		rtb[i + G] = UPSHIFTED(1.35840) * i;
		gtb[i + G] = (-(int)(UPSHIFTED(0.19400 * 2.21790) * (i - 156)));
		btb[i + G] = (-(int)(UPSHIFTED(0.50900 * 1.82150) * (i - 137)));
		rtb[i + B] = UPSHIFTED(1.35840) * i;
		gtb[i + B] = UPSHIFTED(2.21790) * (i - 156);
		btb[i + B] = 0;
		range_table[i + (MAXRGB + 1)] = YCCMap[i];
	}
	for ( ; i < 351; i++)
		range_table[i + (MAXRGB + 1)] = YCCMap[i];

	// Convert to RGB.
	int x, y, z, v;
	for (int row = 0; row < dib.Height(); row++) {
		BYTE* p = dib.BitmapBits() + dib.WidthBytes() * row;
		for (int col = 0; col < dib.Width(); col++) {
			x = p[2];
			y = p[1];
			z = p[0];
			v = DOWNSHIFT(rtb[x + R] + gtb[y + R] + btb[z + R]);
			p[2] = range_limit[v];
			v = DOWNSHIFT(rtb[x + G] + gtb[y + G] + btb[z + G]);
			p[1] = range_limit[v];
			v = DOWNSHIFT(rtb[x + B] + gtb[y + B] + btb[z + B]);
			p[0] = range_limit[v];
			p += 3;
		}
	}
	free(range_table);
	free(basetable);
}

static Bitmap* pcd_read(filemap& fm, int width, int height)
{
	// サブイメージ1の最小サイズしか読まない
	// Determine if this is a PCD file.
	BYTE* header = (BYTE*)malloc(3 * 0x800 * sizeof(BYTE));
	if (header == NULL) return NULL;

	if (fm.read(header, 3 * 0x800) != 3 * 0x800) {
		free(header);
		return NULL;
	}

	UINT rotate = header[0x0e02] & 0x03;
//@	UINT number_images = (header[10] << 8) | header[11];
	free(header);

	int cx = 192;
	int cy = 128;

	memdib dib(cx, cy, 24);
	int packets = cx * cy;
	BYTE* chroma1 = (BYTE*)malloc(packets + 1);
	BYTE* chroma2 = (BYTE*)malloc(packets + 1);
	BYTE* luma = (BYTE*)malloc(packets + 1);
	if (!chroma1 || !chroma2 || !luma) return NULL;

	// Advance to image data.
	fm.seek(0x800, SEEK_CUR);

	// Read interleaved image.
	register BYTE* Y = luma;
	register BYTE* C1 = chroma1;
	register BYTE* C2 = chroma2;
	for (int i = 0; i < cy; i += 2) {
		fm.read(Y, cx); Y += cx;
		fm.read(Y, cx); Y += cx;
		fm.read(C1, cx >> 1); C1 += cx;
		fm.read(C2, cx >> 1); C2 += cx;
	}

	pcd_upsample(cx >> 1, cy >> 1, cx, chroma1);
	pcd_upsample(cx >> 1, cy >> 1, cx, chroma2);

	// Transfer luminance and chrominance channels.
	Y = luma;
	C1 = chroma1;
	C2 = chroma2;
	for (int row = 0; row < cy; row++) {
//@		BYTE* p = dib.BitmapBits() + dib.WidthBytes() * (cy - row - 1);
		BYTE* p = dib.RowPosition(row);
		for (int col = 0; col < cx; col++) {
			p[2] = *Y++;
			p[1] = *C1++;
			p[0] = *C2++;
			p += 3;
		}
	}
	free(chroma2);
	free(chroma1);
	free(luma);

	pcd_ycc2rgb(dib);
	return dib.GetThumbnail(width, height, (rotate == 1 || rotate == 3));
}

static Bitmap* LoadPCD(LPCTSTR pszName, int width, int height)
{
	filemap fm;
	if (fm.open(pszName))
		return pcd_read(fm, width, height);
	return NULL;
}

////////////////////////////////////////////////////////////////////////
// webp

#include "../../libwebp/src/webp/decode.h"

#define APPLYALPHA(a,b)		(b < 255 ? (BYTE)((double)(a)*(double)(b)/255.0) : a)

static Bitmap* webp_read(filemap& fm, int width, int height)
{
	size_t memsize = (size_t)fm.filesize();
	uint8_t* memptr = (uint8_t*)fm.m_base;
	// Init a configuration object
	WebPDecoderConfig config;
	WebPInitDecoderConfig(&config);
	// optional: retrieve the bitstream's features.
	if (WebPGetFeatures(memptr, memsize, &config.input) == VP8_STATUS_OK) {
		config.options.use_threads = 1;
		config.options.no_fancy_upsampling = 1;
		uint32_t scaled_width = config.input.width;
		uint32_t scaled_height = config.input.height;
		float scale = CalcScaleRatio(config.input.width, config.input.height, width, height);
		if (scale < 1.0f) {
			// スケール化して読む
			scaled_width = (uint32_t)float(scaled_width * scale);
			scaled_height = (uint32_t)float(scaled_height * scale);
			config.options.use_scaling = 1;
			config.options.scaled_width = scaled_width;
			config.options.scaled_height = scaled_height;
		}

		// Decode
		if (WebPDecode(memptr, memsize, &config) == VP8_STATUS_OK) {
			// Decoded image is now in config.output (and config.output.u.RGBA)
			uint8_t* dp = (uint8_t*)config.output.u.RGBA.rgba;
			bool hasAlpha = (config.output.colorspace == MODE_RGBA ||
							 config.output.colorspace == MODE_ARGB ||
							 config.output.colorspace == MODE_BGRA);
			Gdiplus::PixelFormat fmt = hasAlpha ? PixelFormat32bppARGB : PixelFormat24bppRGB;
			Bitmap* pbm = new Gdiplus::Bitmap(scaled_width, scaled_height, fmt);
			for (int y = 0; y < scaled_height; ++y) {
				BYTE r, g, b, a;
				r = g = b = a = 255;
				for (int x = 0; x < scaled_width; ++x) {
					switch (config.output.colorspace) {
						case MODE_RGB:
							r = *dp++;	// red
							g = *dp++;	// green
							b = *dp++;	// blue
							break;
						case MODE_RGBA:
							r = *dp++;	// red
							g = *dp++;	// green
							b = *dp++;	// blue
							a = *dp++;
							break;
						case MODE_ARGB:
							a = *dp++;
							r = *dp++;	// red
							g = *dp++;	// green
							b = *dp++;	// blue
							break;
						case MODE_BGR:
							b = *dp++;	// blue
							g = *dp++;	// green
							r = *dp++;	// blue
							break;
						case MODE_BGRA:
							r = *dp++;	// blue
							g = *dp++;	// green
							b = *dp++;	// blue
							a = *dp++;
							break;
					}
					Gdiplus::Color c(a, r, g, b);
					pbm->SetPixel(x, y, c);
				}
			}
			WebPFreeDecBuffer(&config.output);
			return pbm;
		}
	}
	return NULL;
}


static Bitmap* LoadWebp(LPCTSTR pszName, int width, int height)
{
	filemap fm;
	if (fm.open(pszName))
		return webp_read(fm, width, height);
	return NULL;
}


////////////////////////////////////////////////////////////////////////
// avif

#include "../../libavif/include/avif/avif.h"

static Bitmap* avif_read(avifDecoder* decoder, int width, int height)
{
	int jobs = 1;
	avifChromaUpsampling chromaUpsampling = AVIF_CHROMA_UPSAMPLING_AUTOMATIC;
	// AVIF_CHROMA_UPSAMPLING_AUTOMATIC,
	// AVIF_CHROMA_UPSAMPLING_FASTEST,
	// AVIF_CHROMA_UPSAMPLING_BEST_QUALITY,
	// AVIF_CHROMA_UPSAMPLING_NEAREST,
	// AVIF_CHROMA_UPSAMPLING_BILINEAR

	avifCodecChoice codecChoice = AVIF_CODEC_CHOICE_AUTO;
	avifStrictFlags strictFlags = AVIF_STRICT_ENABLED;
	//	avifBool rawColor = AVIF_FALSE;
	avifBool allowProgressive = AVIF_FALSE;
	uint32_t imageSizeLimit = AVIF_DEFAULT_IMAGE_SIZE_LIMIT;
	uint32_t imageDimensionLimit = AVIF_DEFAULT_IMAGE_DIMENSION_LIMIT;
	uint32_t frameIndex = 0;

	decoder->maxThreads = jobs;
	decoder->codecChoice = codecChoice;
	decoder->imageSizeLimit = imageSizeLimit;
	decoder->imageDimensionLimit = imageDimensionLimit;
	decoder->strictFlags = strictFlags;
	decoder->allowProgressive = allowProgressive;

	Bitmap* pbm = NULL;
	if (avifDecoderParse(decoder) == AVIF_RESULT_OK &&
		avifDecoderNthImage(decoder, frameIndex) == AVIF_RESULT_OK) {

		avifImage* avif = decoder->image;
		// Scales the YUV/A planes in-place. dstWidth and dstHeight must both be <= AVIF_DEFAULT_IMAGE_DIMENSION_LIMIT and
		// dstWidth*dstHeight should be <= AVIF_DEFAULT_IMAGE_SIZE_LIMIT.
		if (width < avif->width || height < avif->height) {
			float scale = CalcScaleRatio(avif->width, avif->height, width, height);
			if (scale < 1.0f) {
				avifDiagnostics diag;
				avifImageScale(avif, (uint32_t)(avif->width * scale), (uint32_t)(avif->height * scale), &diag);
			}
		}
		avifRGBImage rgb;
		memset(&rgb, 0, sizeof(rgb));

		avifRGBImageSetDefaults(&rgb, avif);
		rgb.chromaUpsampling = chromaUpsampling;
		rgb.depth = 8;
		rgb.format = avif->alphaPlane ? AVIF_RGB_FORMAT_RGBA : AVIF_RGB_FORMAT_RGB;

		// Override YUV(A)->RGB(A) defaults here: depth, format, chromaUpsampling, ignoreAlpha, alphaPremultiplied, libYUVUsage, etc
		// Alternative: set rgb.pixels and rgb.rowBytes yourself, which should match your chosen rgb.format
		// Be sure to use uint16_t* instead of uint8_t* for rgb.pixels/rgb.rowBytes if (rgb.depth > 8)
		if (avifRGBImageAllocatePixels(&rgb) == AVIF_RESULT_OK) {
			avifImageYUVToRGB(avif, &rgb);
			uint32_t bpp = rgb.rowBytes / rgb.width;
			Gdiplus::PixelFormat fmt = (rgb.format == AVIF_RGB_FORMAT_RGBA) ? PixelFormat32bppARGB : PixelFormat24bppRGB;
			pbm = new Gdiplus::Bitmap(avif->width, avif->height, fmt);
			for (uint32_t y = 0; y < avif->height; ++y) {
				uint8_t* sp = rgb.pixels + rgb.rowBytes * y;
				for (uint32_t x = 0; x < avif->width; ++x) {
					Gdiplus::Color c((fmt == PixelFormat32bppARGB ? sp[3] : 255), sp[0], sp[1], sp[2]);
					pbm->SetPixel(x, y, c);
					sp += bpp;
				}
			}
			avifRGBImageFreePixels(&rgb);
		}
	}
	return pbm;
}


static Bitmap* LoadAvif(LPCTSTR pszName, int width, int height)
{
	Bitmap* pbm = NULL;
	avifDecoder* decoder = avifDecoderCreate();
	filemap fm;
	if (fm.open(pszName)) {
		size_t memsize = (size_t)fm.filesize();
		uint8_t* memptr = (uint8_t*)fm.m_base;
		avifDecoderSetIOMemory(decoder, memptr, memsize);
		pbm = avif_read(decoder, width, height);
	}
	avifDecoderDestroy(decoder);
	return pbm;
}


////////////////////////////////////////////////////////////////////////
//

Bitmap* LoadThumbnail(LPCTSTR pszName, int width, int height)
{
	Bitmap* bm = NULL;
	if (pszName && *pszName) {
		LPTSTR p = PathFindExtension(pszName);
		if (*p == '.') {
			p++;
			if		(!StrCmpI(_T("gif"),  p) ||
					 !StrCmpI(_T("jpg"),  p) ||
					 !StrCmpI(_T("jpe"),  p) ||
					 !StrCmpI(_T("jpeg"), p) ||
					 !StrCmpI(_T("tif"),  p) ||
					 !StrCmpI(_T("tiff"), p) ||
//					 !StrCmpI(_T("webp"), p) ||			//win11では標準で読めるが、デコードが遅いので自前で処理する
					 !StrCmpI(_T("wmf"),  p) ||
					 !StrCmpI(_T("png"),  p))	bm = LoadImageFromFile(pszName, width, height);
			else if (!StrCmpI(_T("bmp"), p) || 
					 !StrCmpI(_T("thm"), p))	bm = LoadImageFromStream(pszName, width, height);
			else if (!StrCmpI(_T("webp"), p))	bm = LoadWebp(pszName, width, height);
			else if (!StrCmpI(_T("avif"), p))	bm = LoadAvif(pszName, width, height);
			else if (!StrCmpI(_T("psd"),  p))	bm = LoadPSD(pszName, width, height);
			else if (!StrCmpI(_T("tga"),  p))	bm = LoadTGA(pszName, width, height);
			else if (!StrCmpI(_T("pnm"),  p) ||
					 !StrCmpI(_T("pgm"),  p) ||
					 !StrCmpI(_T("pbm"),  p) ||
					 !StrCmpI(_T("ppm"),  p))	bm = LoadPNM(pszName, width, height);
			else if (!StrCmpI(_T("pcx"),  p))	bm = LoadPCX(pszName, width, height);
			else if (!StrCmpI(_T("ras"),  p))	bm = LoadRAS(pszName, width, height);
			else if (!StrCmpI(_T("sgi"),  p))	bm = LoadSGI(pszName, width, height);
			else if (!StrCmpI(_T("xbm"),  p))	bm = LoadXBM(pszName, width, height);
			else if (!StrCmpI(_T("pcd"),  p))	bm = LoadPCD(pszName, width, height);
		}
	}
	return bm;
}

// imgspecify.cpp で定義。 exec.cppでインクルードされる
extern "C" int WINAPI imgSpecifyImageType(LPCTSTR lpName);
extern "C" int WINAPI imgTypeIsExif(int type);

Bitmap* LoadRawThumbnail(LPCTSTR pszName, int width, int height, EXIFINFOSTRUCT* pInfo)
{
	Bitmap* bitmap = NULL;
	int type = imgSpecifyImageType(pszName);
	if (type && imgTypeIsExif(type)) {
		CComPtr<IExifParser> pep;
		HRESULT hr = pep.CoCreateInstance(__uuidof(ExifParser));
		if (SUCCEEDED(hr)) {
			CComPtr<IStream> pstrm;
			if (CreateStreamOnHGlobal(NULL, TRUE, &pstrm) == S_OK) {
				if (pep->GetExifInfo(pszName, pstrm, NULL, false, (BYTE*)pInfo) == S_OK) {
					Bitmap bm(pstrm);
					bitmap = load_core_gdip(bm, width, height);
					switch (pInfo->Orientation) {
					case ORIENTATION_FLIPX:					// 左右反転
						bitmap->RotateFlip(RotateNoneFlipX);
						break;
					case ORIENTATION_ROTATE180:				// 180°回転
						bitmap->RotateFlip(Rotate180FlipNone);
						break;
					case ORIENTATION_FLIPY:					// 上下反転
						bitmap->RotateFlip(RotateNoneFlipY);
						break;
					case ORIENTATION_ROTATELEFTFLIPY:		// 左へ90°回転し上下反転
						bitmap->RotateFlip(Rotate270FlipY);
						break;
					case ORIENTATION_ROTATERIGHT:			// 右へ90°回転する
						bitmap->RotateFlip(Rotate90FlipNone);
						break;
					case ORIENTATION_ROTATERIGHTFLIPY:		// 右へ90°回転し上下反転
						bitmap->RotateFlip(Rotate90FlipY);
						break;
					case ORIENTATION_ROTATELEFT:			// 左へ90°回転する
						bitmap->RotateFlip(Rotate270FlipNone);
						break;
					}
				}
			}
		}
	}
	return bitmap;
}

HBITMAP LoadHtmlThumbnail(LPCTSTR pszName, int width, int height)
{
	HBITMAP hbm = NULL;
	CComPtr<IPersistFile> psf;
	if (psf.CoCreateInstance(_T("Shell.ThumbnailExtract.HTML.1")) == S_OK) {
		if (psf->Load(pszName, STGM_READ) == S_OK) {
			CComQIPtr<IExtractImage, &IID_IExtractImage> pei(psf);

			OLECHAR* s = new OLECHAR[MAX_URL_LENGTH];
			DWORD priority = 0;
			CSize size(width, height);
			DWORD flg = IEIFLAG_SCREEN | IEIFLAG_OFFLINE;
			if (pei->GetLocation(s, MAX_URL_LENGTH, &priority, &size, 24, &flg) == NOERROR) {
				pei->Extract(&hbm);
			}
			delete [] s;
		}
	}
	return hbm;
}

Bitmap* ResizeBitmap(HBITMAP hbm, int width, int height)
{
	Bitmap bm(hbm, NULL);
	return load_core_gdip(bm, width, height);
}

Bitmap* LoadImageFromResource(HINSTANCE hInst, LPCTSTR pszResourceType, LPCTSTR pszResourceName)
{
	Bitmap* bitmap = NULL;
	// リソースからイメージをロードする
	// 元データは bmp, png, jpeg, tiff 等何でもOK
	HRSRC hrsrc = FindResource(hInst, pszResourceName, pszResourceType);
	if (hrsrc) {
		DWORD sizeResource = SizeofResource(hInst, hrsrc);
		if (sizeResource) {
			HGLOBAL hglb = LoadResource(hInst, hrsrc);
			if (hglb) {
				void* pr = LockResource(hglb);
				if (pr) {
					BYTE* pb = (BYTE*)malloc(sizeResource);
					if (pb) {
						memcpy(pb, pr, sizeResource);
						CComPtr<IStream> pstrm = SHCreateMemStream(pb, sizeResource);
						if (pstrm) {
							bitmap = new Bitmap(pstrm);
						}
						free(pb);
					}
				}
			}
		}
	}
	return bitmap;
}

HBITMAP LoadBitmapFromResource(HINSTANCE hInst, LPCTSTR typeResource, LPCTSTR pszResource)
{
	HBITMAP hbm = NULL;
	Bitmap* bitmap = LoadImageFromResource(hInst, typeResource, pszResource);
	if (bitmap) {
		Color color;
		if (bitmap->GetHBITMAP(color, &hbm) != Ok)
			hbm = NULL;
		delete bitmap;
	}
	return hbm;
}

/*
 * IShellImageFactoryを使用して縮小イメージを取得
 * 
 */
HBITMAP xxgetThumbnail(LPCTSTR fileName, int width, int height, int options)
{
	// size - Size of the image, SIIGBF_BIGGERSIZEOK - GetImage will stretch down the bitmap (preserving aspect ratio)
	SIZE size = { width, height };
	HBITMAP hbm;
	CComPtr<IShellItemImageFactory> pif;
	if (SHCreateItemFromParsingName(fileName, NULL, IID_PPV_ARGS(&pif)) == S_OK &&
		pif->GetImage(size, SIIGBF_BIGGERSIZEOK, &hbm) == S_OK) {
		return hbm;
	}
	return NULL;
}