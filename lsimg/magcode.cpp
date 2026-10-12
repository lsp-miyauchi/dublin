#include "stdafx.h"
#include "lsimg.h"
#include "common.h"

#pragma pack(1)

typedef struct tagMAGHEADER {
    BYTE top;
    BYTE machine;
    BYTE machine_depend;
    BYTE screen_mode;
    WORD ltx;
    WORD lty;
    WORD rbx;
    WORD rby;
    DWORD fa_ofs;
    DWORD fb_ofs;
    DWORD fb_size;
    DWORD pxl_ofs;
    DWORD pxl_size;
} MAGHEADER;

typedef struct tagMAKIHEADER {
	BYTE check[8];
	BYTE machine[24];
	WORD FlagBSize;
	WORD PixelASize;
	WORD PixelBSize;
	WORD ExtendFlag;
	WORD ltx;
	WORD lty;
	WORD rbx;
	WORD rby;
} MAKIHEADER;

#pragma pack()

typedef BOOL (*SETLINEPROC)(LPBITMAPINFO, LPBYTE, int);

#define ENDIAN(w)		((WORD)((WORD)LOBYTE((w)) << 8) | (WORD)HIBYTE((w)))
#define issjkanji(c)	((0x80 < (BYTE)(c) && (BYTE)(c) < 0xA0) || \
									(0xDF < (BYTE)(c) && (BYTE)(c) < 0xFD))

int Xfrom[16] = {
	0, 1, 2, 4, 0,   1, 0, 1, 2, 0,
	1, 2, 0, 1, 2,   0
};
int Yfrom[16] = {
	0, 0, 0, 0, 1,   1, 2, 2, 2, 4,
	4, 4, 8, 8, 8,  16
};

BOOL setline(LPBITMAPINFO pbi, LPBYTE pIn, int y )
{
	register int i;
	register LPBYTE pOut;
	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)pbi;

	// 画面の外に描画しようとした場合には無視
	if (y > pbh->biHeight)
		return TRUE;

	pOut = dib_findbits(pbi);
	pOut += WIDTHBYTES(pbh->biWidth * pbh->biBitCount) * (pbh->biHeight - y - 1);
	switch (pbh->biBitCount)
	{
		case 4:
			for (i = 0; i < pbh->biWidth; i++ )
			{
				if (i & 1)
				{
					*pOut |= *pIn & 0x0F;
					pIn++;
					pOut++;
				}
				else
				{
					*pOut = (*pIn << 4);
					pIn++;
				}
			}
			break;

		case 8:
			memcpy(pOut, pIn, pbh->biWidth);
			break;

		case 24:
			memcpy(pOut, pIn, pbh->biWidth * 3);
			break;

		default:
			return FALSE;
			break;
	}
	return TRUE;
}

BOOL setline_4bpp(LPBITMAPINFO pbi, LPBYTE pIn, int y)
{
	LPBYTE pOut;
	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)pbi;

	// 画面の外に描画しようとした場合には無視
	if (y > pbh->biHeight)
		return TRUE;

	pOut = dib_findbits(pbi);
	pOut += WIDTHBYTES(pbh->biWidth * pbh->biBitCount) * (pbh->biHeight - y - 1);

	// 入力も出力も 4bit の場合には幅が半分
	memcpy(pOut, pIn, pbh->biWidth >> 1);
	return TRUE;
}

static void initbmheader(LPBITMAPINFO pbi, DWORD width, DWORD height,
							BYTE bits, int pal)
{
	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)pbi;
	pbh->biSize = sizeof(BITMAPINFOHEADER);
	pbh->biWidth = width;
	pbh->biHeight = height;
	pbh->biPlanes = 1;
	pbh->biBitCount = bits;
	pbh->biCompression = 0;
	pbh->biSizeImage = 0;
	pbh->biXPelsPerMeter = 0;
	pbh->biYPelsPerMeter = 0;
	if (pal)
		pbh->biClrUsed = pbh->biClrImportant = pal;
	else
	{
		switch (bits)
		{
			case 1:
			case 24:
				pbh->biClrUsed = pbh->biClrImportant = 0;
				break;

			case 4:
				pbh->biClrUsed = pbh->biClrImportant = 16;
				break;

			case 8:
			case 16:
				pbh->biClrUsed = pbh->biClrImportant = 256;
				break;
		}
	}
}

class memman
{
private:
	uint8_t* m_p;
	size_t m_size;
public:
	memman() : m_p(NULL), m_size(0) {}
	~memman() { if (m_p) free(m_p); }
	void* alloc(size_t size) {
		m_p = (uint8_t*)realloc(m_p, m_size + size);
		return (void*)(m_p + m_size);
	}
};

extern "C"
HDIB WINAPI imgMagLoad(LPCTSTR fname)
{
    int i, j, w, fw, h, pw, bx, by, no;
    DWORD la;
    DWORD Bpos, Ppos;
	LPWORD pw1, pw2;

	memman mm;


	// バッファにファイルを読み込む
	MemStream* pms = _mfcreate(fname);
	if (!pms)
		return NULL;

	BYTE* pb = pms->m_baseptr;
	// チェックデータ＋コメント
	while (*pb != 0x1A) {
		if (issjkanji(*pb))
			pb++;
		pb++;
	}
	pb++;

	// ヘッダ読み込み
	DWORD base = (DWORD)(pb - pms->m_baseptr);
	MAGHEADER mh;
	memcpy(&mh, pb, sizeof(mh));
	pb += sizeof(mh);

	// 200 line
	BOOL b200Line = (mh.screen_mode & 0x1) ? TRUE : FALSE;

	SETLINEPROC fnSetLine;	// バッファへの出力関数
	// パレットサイズ と ビットマップサイズ
	// ビットマップは、左右それぞれの端が 32bitアラインメント
	if (mh.screen_mode & 0x80) {
        j =  256;									// palette size
        pw = 2;										// pixel/word
		fw = (mh.rbx >> 2) - (mh.ltx >> 2) + 1;		// 1行の DWORD 数
		w = fw * 4;									// bitmap の幅
		fnSetLine = setline;						// 出力関数
    }
	else {
        j = 16;										// palette size
        pw = 4;										// pixel/word
		fw = (mh.rbx >> 3) - (mh.ltx >> 3) + 1;		// 1行の DWORD 数
		w = fw * 8;									// bitmap の幅
		fnSetLine = setline_4bpp;					// 出力関数
    }
	h = mh.rby - mh.lty + 1;						// bitmap の高さ

	// ヘッダの設定
	HDIB hdib = NULL;
	LPBITMAPINFO image;
	if (b200Line == TRUE) {
		hdib = dib_create(w, h * 2, 16 / pw);
		image = (LPBITMAPINFO)GlobalLock(hdib);
		initbmheader(image, w, (DWORD)h * 2, (BYTE)(16 / pw), j);
	}
	else {
		hdib = dib_create(w, h, 16 / pw);
		image = (LPBITMAPINFO)GlobalLock(hdib);
		initbmheader(image, w, h, (BYTE)(16 / pw), j);
	}

	for (i = 0; i < j; i++) {
		image->bmiColors[i].rgbGreen = *pb++;
		image->bmiColors[i].rgbRed = *pb++;
		image->bmiColors[i].rgbBlue = *pb++;
	}

	// フラグＡ
	DWORD fa_size = (DWORD)mh.fb_ofs - mh.fa_ofs;
	pb = pms->m_baseptr + mh.fa_ofs + base;
	BYTE* FlgA = pb;
	pb += fa_size;

	// フラグＢバッファ確保
	BYTE* FlgB = (BYTE*)mm.alloc(fw);
	if (FlgB == NULL)
        return NULL;

	BYTE* PreFlgB = (BYTE*)mm.alloc(fw);
	if (PreFlgB == NULL)
        return NULL;

	// Pixel Buffer 確保
	BYTE** Img = (BYTE**)mm.alloc(sizeof(BYTE*) * (h + 1));
	if (Img == NULL)
        return NULL;

	for (i = 0; i < min(h, 17); i++ ) {
		Img[i] = (BYTE*)mm.alloc(sizeof(BYTE) * (w + 1));
		if (Img[i] == NULL)
            return NULL;
    }
	for ( ; i < h; i++)
		Img[i] = NULL;

	// ピクセルデータの展開
    Bpos = (DWORD)mh.fb_ofs + base;
    Ppos = (DWORD)mh.pxl_ofs + base;
	for (by = 0, la = 0L, i = 0x80; by < h; by++) {
		pb = pms->m_baseptr + Bpos;
		if (by != 0) {
            BYTE* FBtmp = FlgB;		// FlgB と PreFlgB の入れかえ
            FlgB = PreFlgB;
            PreFlgB = FBtmp;
        }
        for (bx = 0; bx < fw; bx++) {
			if (i & *(FlgA + la)) {
				*(FlgB + bx) = *pb;
				pb ++; Bpos++;
			}
			else
				*(FlgB + bx) = 0x00;

            i >>= 1;
			if (i == 0) {
                i = 0x80;
                la++;
            }
        }
		if (by != 0) {
			for (bx = 0; bx < fw; bx++)
				*(FlgB + bx) ^= *(PreFlgB + bx);
		}

		pb = pms->m_baseptr + Ppos;
		if (16 < by) {
			Img[by] = Img[by - 17];
			Img[by - 17] = NULL;
		}
		BYTE* CurFlgB = FlgB;
		for (bx = 0; bx < fw * 2; bx++) {
			if (bx & 1) {
				no = *CurFlgB & 0x0F;
				CurFlgB++;
            }
			else
				no = (*CurFlgB & 0xF0) >> 4;

			// ここから先のメモリコピーは、それぞれ 4byte と 2byte なので
			// ポインタだけもらってきて DWORD, WORD にキャストし、DWORD や
			// WORD として代入している。
			if (no == 0) {
//@				if (mh.screen_mode & 0x80) {
//@					pw1 = (LPWORD)(Img[by] + bx * 4);
//@					*pw1 = *(LPWORD)pb;
//@				}
//@				else {
					pw1 = (LPWORD)(Img[by] + bx * 2);
					*pw1 = *(LPWORD)pb;
//@				}
				pb += 2;
				Ppos += 2;
			}
			else {
//@				if (mh.screen_mode & 0x80) {
//@					pdw1 = (LPDWORD)(Img[by] + bx * 4);
//@					pdw2 = (LPDWORD)(Img[by - Yfrom[no]] + (bx - Xfrom[no]) * 4);
//@					*pdw1 = *pdw2;
//@				}
//@				else {
					pw1 = (LPWORD)(Img[by] + bx * 2);
					pw2 = (LPWORD)(Img[by - Yfrom[no]] + (bx - Xfrom[no]) * 2);
					*pw1 = *pw2;
//@				}
			}
        }

		if (b200Line) {
			fnSetLine(image, Img[by], by * 2 );
			fnSetLine(image, Img[by], by * 2 + 1 );
		}
		else
			fnSetLine(image, Img[by], by );
    }

	GlobalUnlock(hdib);
	return hdib;
}

/////////////////////////////////////////////////////////////////////
//

static BOOL mkiCheck(MemStream* pms)
{
	char buf[16];
	_mfread(pms, buf, 8);
	_mfseek(pms, 0, FILE_BEGIN);
	if (buf[6] == 'A' || buf[6] == 'B') {
		buf[6] = '\0';
		if (!strcmp("MAKI01", buf))
			return TRUE;
	}
	return FALSE;
}

extern "C"
HDIB WINAPI imgMkiLoad(LPCTSTR fname)
{
    BYTE c, c2;
    int i, j, w, h, pre, p;
	ULONG la;
    MAKIHEADER mh;
	LPBYTE FlgA, FlgB, *Img;
	MemStream* pmf;
	HDIB hdib;
	WORD ww;
	LPBITMAPINFO image;
	LPBITMAPINFOHEADER pbh;

	pmf = _mfcreate(fname);
	if (!pmf)
		return NULL;

	if (!mkiCheck(pmf))
	{
		_mfclose(pmf);
		return NULL;
	}

	if ((FlgB = (LPBYTE)GlobalAllocPtr(GHND, 16001)) == NULL)
	{
		_mfclose(pmf);
		return NULL;
	}

	if ((Img = (LPBYTE*)GlobalAllocPtr(GHND, sizeof(LPBYTE) * 400)) == NULL)
	{
		_mfclose(pmf);
		GlobalFreePtr(FlgB);
		return NULL;
    }

	// ヘッダ
	_mfread(pmf, mh.check, sizeof(mh.check[0]) * 8);
	_mfread(pmf, mh.machine, sizeof(mh.machine[0]) * 24);

	ww = _mfgetw(pmf);	mh.FlagBSize  = ENDIAN(ww);
	ww = _mfgetw(pmf);	mh.PixelASize = ENDIAN(ww);
	ww = _mfgetw(pmf);	mh.PixelBSize = ENDIAN(ww);
	ww = _mfgetw(pmf);	mh.ExtendFlag = ENDIAN(ww);
	ww = _mfgetw(pmf);	mh.ltx        = ENDIAN(ww);
	ww = _mfgetw(pmf);	mh.lty        = ENDIAN(ww);
	ww = _mfgetw(pmf);	mh.rbx        = ENDIAN(ww);
	ww = _mfgetw(pmf);	mh.rby        = ENDIAN(ww);

	hdib = dib_create(mh.rbx - mh.ltx, mh.rby - mh.lty, 4);
	if (!hdib)
	{
		_mfclose(pmf);
		GlobalFreePtr(Img);
		GlobalFreePtr(FlgB);
		return NULL;
	}

	image = (LPBITMAPINFO)GlobalLock(hdib);
	pbh = (LPBITMAPINFOHEADER)image;
	pbh->biClrUsed = pbh->biClrImportant = 16;

	// パレット
	for (i = 0; i < 16; i++)
	{
		image->bmiColors[i].rgbGreen = _mfgetc(pmf);
		image->bmiColors[i].rgbRed = _mfgetc(pmf);
		image->bmiColors[i].rgbBlue = _mfgetc(pmf);
	}

	// フラグＡ
	if ((FlgA = (LPBYTE)GlobalAllocPtr(GHND, 1000)) == NULL)
	{
		GlobalUnlock(hdib);
		GlobalFree(hdib);
		_mfclose(pmf);
		GlobalFreePtr(Img);
		GlobalFreePtr(FlgB);
		return NULL;
	}
	_mfread(pmf, FlgA, 1000);

	// フラグＢの展開
	ZeroMemory(FlgB, 16000);
	for (h = 0, i = 0x80, la = 0; h < 100; h++)
	{
		for (w = 0; w < 80; w++)
		{
			if (i & FlgA[la])
			{
				c = _mfgetc(pmf);
				c2 = _mfgetc(pmf);
				p = h * 4 * 40 + w / 2;
				if (w & 1)
				{
					FlgB[p      ] |= (c & 0xF0) >> 4;
					FlgB[p + 40 ] |= c & 0x0F;
					FlgB[p + 80 ] |= (c2 & 0xF0) >> 4;
					FlgB[p + 120] |= c2 & 0x0F;
				}
				else
				{
					FlgB[p      ] |= c & 0xF0;
					FlgB[p + 40 ] |= (c & 0x0F) << 4;
					FlgB[p + 80 ] |= c2 & 0xF0;
					FlgB[p + 120] |= (c2 & 0x0F) << 4;
				}
			}
			i >>= 1;
			if (i == 0)
			{
				i = 0x80;
				la++;
			}
		}
	}

	// Pixel Buffer 確保
	for (i = 0; i < 5; i++)
	{
		if ((Img[i] = (LPBYTE)GlobalAllocPtr(GHND, 641)) == NULL)
		{
			for (j = 0; j < i; j++)
				GlobalFreePtr(Img[j]);
			GlobalFreePtr(Img);
			GlobalFreePtr(FlgA);
			GlobalFreePtr(FlgB);

			GlobalUnlock(hdib);
			GlobalFree(hdib);
			_mfclose(pmf);
			return NULL;
		}
	}
	for (; i < 400; i++)
		Img[i] = NULL;

	// ピクセルデータの展開
	pre = (mh.check[6] == 'A') ? 2 : 4;
	for (h = 0, i = 0x80, la = 0; h < 400; h++ )
	{
		if (4 < h)
		{
            Img[h] = Img[h - 5];
            Img[h - 5] = NULL;
		}
		for (w = 0; w < 320; w++)
		{
			if (i & FlgB[la])
				Img[h][w] = _mfgetc(pmf);
			else
				Img[h][w] = 0;

			i >>= 1;
			if (i == 0)
			{
				i = 0x80;
				la++;
			}
		}
		if (pre <= h)
		{
			for (w = 0; w < 640; w++)
				Img[h][w] ^= Img[h - pre][w];
		}
		setline_4bpp(image, Img[h], h);
	}

	for (i = 0; i < 400; i++)
	{
		if (Img[i])
			GlobalFreePtr(Img[i]);
	}
	GlobalFreePtr(Img);
	GlobalFreePtr(FlgA);
	GlobalFreePtr(FlgB);
	_mfclose(pmf);

	GlobalUnlock(hdib);
	return hdib;
}
