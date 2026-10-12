#include "stdafx.h"
#include "lsimg.h"
#include "common.h"

#define MAXCOLORMAPSIZE			256
#define MAX_LZW_BITS			12

#define BIT_INTERLACE			0x40
#define BIT_COLORMAP			0x80
#define ISBITSET(byte, bit)		(((byte) & (bit)) == (bit))
#define LM_TO_UNIT(a,b)			(((b)<<8)|(a))
#define LZW_TABLE_SIZE			((short)1 << MAX_LZW_BITS)
#define HSIZE					5003		// hash table size for 80% occupancy
#define MAXCODE(a)				(((short)1 << (a)) - 1)
#define HASH_ENTRY(a,b) 		((((long)(a)) << 8) | (b))

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

typedef struct tagGIFSAVE {
	MemStream* pmf;

	// State for packing variable-width codes into a bitstream
	int n_bits;						// current number of bits/code
	short maxcode;				// maximum code, given n_bits
	int init_bits;					// initial n_bits ... restored after clear
	long cur_accum;					// holds bits not yet output
	int cur_bits;					// # of bits in cur_accum

	// LZW string construction
	short waiting_code;			// symbol not yet output; may be extendable
	BOOL first_byte;				// if TRUE, waiting_code is not valid

	// State for LZW code assignment
	short ClearCode;				// clear code (doesn't change)
	short EOFCode;					// EOF code (ditto)
	short free_code;				// first not-yet-used symbol code

	// LZW hash table
	short *hash_code;			// => hash table of symbol codes
	long *hash_value;			// => hash table of symbol values

	// GIF data packet construction buffer
	int bytesinpkt;					// # of bytes in current packet
	char packetbuf[256];			// workspace for accumulating packet

} GIFSAVE;
typedef GIFSAVE* LPGIFSAVE;

static GIF89 gif89 = { -1, -1, -1, 0 };
static int read_colormap(MemStream*, int n, RGBQUAD*);
static int do_extension(MemStream*, int, int*);
static int getdatablock(MemStream*, BYTE*);
static int getcode(MemStream*, int, int);
static int lzw_reatebyte(MemStream*, int, int);
static void read_image(LPGIFIMAGE, MemStream*, int, int, RGBQUAD*, BOOL);
static void char_out(LPGIFSAVE pgs, int c);
static void flush_packet(LPGIFSAVE pgs);
static void output(LPGIFSAVE pgs, short code);
static void clear_hash(LPGIFSAVE pgs);
static void clear_block(LPGIFSAVE pgs);
static void compress_init(LPGIFSAVE pgs, int i_bits);
static void compress_byte(LPGIFSAVE pgs, int c);
static void compress_term(LPGIFSAVE pgs);
static void __fastcall put_byte(int cb, HANDLE hf);
static void __fastcall put_word(WORD w, HANDLE hf);
static void __fastcall put_3bytes(int val, HANDLE hf);
static void put_pixel_rows(LPGIFSAVE pgs, int width, RGBA* pData);


int ZeroDataBlock;

LPGIFIMAGE AllocImageStruct(int sx, int sy)
{
	DWORD dw = sizeof(GIFIMAGE) + sx * sy;
	LPGIFIMAGE pgi = (LPGIFIMAGE)GlobalAllocPtr(GMEM_MOVEABLE, dw);
	if (pgi)
	{
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

LPGIFIMAGE gifDecorder(MemStream* pmf)
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

	if (_mfread(pmf, buf, 6) != 6)
		return NULL;

	if (*((DWORD*)buf) != 0x38464947 ||
		(*((WORD*)&buf[4]) != 0x6137 && *((WORD*)&buf[4]) != 0x6139))
		return NULL;

	if (_mfread(pmf, buf, 7) != 7)
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
		if (_mfread(pmf, &c, 1) != 1)
			return NULL;

		if (c == ';')						// GIF terminator
			break;

		if (c == '!') {						// Extension
			if (_mfread(pmf, &c, 1) != 1)
				return NULL;

			do_extension(pmf, c, &Transparent);
			continue;
		}

		if (c != ',')						// Not a valid start character
			continue;

		if (_mfread(pmf, buf, 9) != 9)
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

static int read_colormap(MemStream* pmf, int number, RGBQUAD* map)
{
	int i;
	BYTE rgb[3];
	for (i = 0; i < number; ++i) {
		if (_mfread(pmf, rgb, sizeof(rgb)) != sizeof(rgb))
			return TRUE;

		map[i].rgbRed   = rgb[0];
		map[i].rgbGreen = rgb[1];
		map[i].rgbBlue  = rgb[2];
	}
	return FALSE;
}

static int do_extension(MemStream* pmf, int label, int *Transparent)
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

static int getdatablock(MemStream* pmf, BYTE *buf)
{
	BYTE count;
	if (_mfread(pmf, &count, 1) != 1)
		return -1;

	ZeroDataBlock = count == 0;

	if (count && _mfread(pmf, buf, count) != count)
		return -1;
	return count;
}

static int getcode(MemStream* pmf, int code_size, int flag)
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

static int lzw_reatebyte(MemStream* pmf, int flag, int input_code_size)
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
			if ((max_code >= max_code_size) && (max_code_size < (1 << MAX_LZW_BITS))) {
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

static void read_image(LPGIFIMAGE pgi, MemStream* pmf, int len, int height, RGBQUAD* cmap, int interlace)
{
	// Stash the color map into the image
	for (int i = 0; i < MAXCOLORMAPSIZE; i++) {
		pgi->map[i].rgbRed      = cmap[i].rgbRed;
		pgi->map[i].rgbGreen    = cmap[i].rgbGreen;
		pgi->map[i].rgbBlue     = cmap[i].rgbBlue;
		pgi->map[i].rgbReserved = 1;
	}

	// Initialize the Compression routines
	BYTE c;
	if (_mfread(pmf, &c, 1) != 1) return;
	if (lzw_reatebyte(pmf, TRUE, c) < 0) return;

	int v;
	int xpos = 0, ypos = 0, pass = 0;
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
					case 1: ypos += 8; break;
					case 2: ypos += 4; break;
					case 3: ypos += 2; break;
				}

				if (ypos >= height) {
					++pass;
					switch (pass) {
						case 1: ypos = 4; break;
						case 2: ypos = 2; break;
						case 3: ypos = 1; break;
						default: goto fini;
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

static HDIB load_gif(MemStream* pms, int* pTransparent)
{
	LPGIFIMAGE pgi = gifDecorder(pms);
	HDIB hdib = NULL;
	if (pgi) {
		hdib = dib_create(pgi->width, pgi->height, 8);
		if (hdib) {
			BITMAPINFO* pb = dib_lockinfo(hdib);
			memset(pb->bmiColors, 0, 256 * sizeof(RGBQUAD));
			memcpy(pb->bmiColors, pgi->map, pgi->colors * sizeof(RGBQUAD));
			((BITMAPINFOHEADER*)pb)->biClrUsed = pgi->colors;
			((BITMAPINFOHEADER*)pb)->biClrImportant = pgi->colors;

			int width = WIDTHBYTES(pgi->width * 8);
			register LPBYTE ps = dib_findbits(pb);
			ps += (pgi->height - 1) * width;
			register LPBYTE pd = pgi->cbData;
			for (int i = 0; i < pgi->height; i++) {
				memcpy(ps, pd, pgi->width);
				ps -= width;
				pd += pgi->width;
			}
			dib_unlock(hdib);

			if (pTransparent)
				*pTransparent = pgi->xparent;
		}
		FreeImageStruct(pgi);
	}
	return hdib;
}

extern "C"
HDIB WINAPI imgGifLoadMemEx(HGLOBAL hmem, int* pTransparent)
{
	HDIB hdib = NULL;
	MemStream* pms = _mfcreate(hmem);
	if (pms) {
		hdib = load_gif(pms, pTransparent);
		_mfclose(pms);
	}
	return hdib;
}

extern "C"
HDIB WINAPI imgGifLoadMem(HGLOBAL hmem)
{
	return imgGifLoadMemEx(hmem, NULL);
}

extern "C"
HDIB WINAPI imgGifLoad(LPCTSTR fullname)
{
	HDIB hdib = NULL;
	MemStream* pms = _mfcreate(fullname);
	if (pms) {
		hdib = load_gif(pms, NULL);
		_mfclose(pms);
	}
	return hdib;
}

////////////////////////////////////////////////////////////////////////
// 

static void flush_packet(LPGIFSAVE pgs)
{
	if (pgs->bytesinpkt > 0) {
		// never write zero-length packet
		pgs->packetbuf[0] = (char)pgs->bytesinpkt++;
		if (_mfwrite(pgs->pmf, pgs->packetbuf, pgs->bytesinpkt) != (DWORD)pgs->bytesinpkt)
			return;
		pgs->bytesinpkt = 0;
	}
}

LPGIFSAVE InitEncoder(DWORD dwSize)
{
	LPGIFSAVE pgs = (LPGIFSAVE)malloc(sizeof(GIFSAVE));
	if (!pgs)
		return NULL;
	memset(pgs, 0, sizeof(GIFSAVE));

	// ハッシュテーブルの設定
	pgs->hash_code = (short*)malloc(HSIZE * sizeof(short));
	if (!pgs->hash_code) {
		free(pgs);
		return NULL;
	}

	pgs->hash_value = (long*)malloc(HSIZE * sizeof(long));
	if (!pgs->hash_value) {
		free(pgs->hash_code);
		free(pgs);
		return NULL;
	}

	pgs->pmf = _mfcreate(dwSize);
	if (!pgs->pmf) {
		free(pgs->hash_value);
		free(pgs->hash_code);
		free(pgs);
		return NULL;
	}

	return pgs;
}

void TermEncoder(LPGIFSAVE pgs)
{
	if (pgs) {
		if (pgs->pmf) _mfclose(pgs->pmf);
		if (pgs->hash_value) free(pgs->hash_value);
		if (pgs->hash_code) free(pgs->hash_code);
		free(pgs);
	}
}

////////////////////////////////////////////////////////////////////////
// Add a character to current packet; flush to disk if necessary

static void char_out(LPGIFSAVE pgs, int c)
{
	pgs->packetbuf[++pgs->bytesinpkt] = (char)(c);
	if (pgs->bytesinpkt >= 255) flush_packet(pgs);
}


////////////////////////////////////////////////////////////////////////
// Emit a code of n_bits bits
// Uses cur_accum and cur_bits to reblock into 8-bit bytes

static void output(LPGIFSAVE pgs, short code)
{
	pgs->cur_accum |= ((long) code) << pgs->cur_bits;
	pgs->cur_bits += pgs->n_bits;

	while (pgs->cur_bits >= 8) {
		char_out(pgs, pgs->cur_accum & 0xFF);
		pgs->cur_accum >>= 8;
		pgs->cur_bits -= 8;
	}

	// If the next entry is going to be too big for the code size,
	// then increase it, if possible.  We do this here to ensure
	// that it's done in sync with the decoder's codesize increases.
	if (pgs->free_code > pgs->maxcode) {
		pgs->n_bits++;
		if (pgs->n_bits == MAX_LZW_BITS)
			pgs->maxcode = LZW_TABLE_SIZE;		// free_code will never exceed this
		else
			pgs->maxcode = MAXCODE(pgs->n_bits);
	}
}


////////////////////////////////////////////////////////////////////////
// The LZW algorithm proper

////////////////////////////////////////////////////////////////////////
// Fill the hash table with empty entries

static void clear_hash(LPGIFSAVE pgs)
{
	// It's sufficient to zero hash_code[]
	memset(pgs->hash_code, 0, HSIZE * sizeof(short));
}


////////////////////////////////////////////////////////////////////////
// Reset compressor and issue a Clear code

static void clear_block (LPGIFSAVE pgs)
{
	clear_hash(pgs);						// delete all the symbols
	pgs->free_code = pgs->ClearCode + 2;
	output(pgs, pgs->ClearCode);			// inform decoder
	pgs->n_bits = pgs->init_bits;			// reset code size
	pgs->maxcode = MAXCODE(pgs->n_bits);
}


////////////////////////////////////////////////////////////////////////
// Initialize LZW compressor

static void compress_init(LPGIFSAVE pgs, int i_bits)
{
	// init all the state variables
	pgs->n_bits = pgs->init_bits = i_bits;
	pgs->maxcode = MAXCODE(pgs->n_bits);
	pgs->ClearCode = ((short) 1 << (i_bits - 1));
	pgs->EOFCode = pgs->ClearCode + 1;
	pgs->free_code = pgs->ClearCode + 2;
	pgs->first_byte = TRUE;				// no waiting symbol yet

	// init output buffering vars
	pgs->bytesinpkt = 0;
	pgs->cur_accum = 0;
	pgs->cur_bits = 0;

	// clear hash table
	clear_hash(pgs);

	// GIF specifies an initial Clear code
	output(pgs, pgs->ClearCode);
}


////////////////////////////////////////////////////////////////////////
// Accept and compress one 8-bit byte

static void compress_byte(LPGIFSAVE pgs, int c)
{
	register short i;
	register short disp;
	register long probe_value;

	if (pgs->first_byte) {
		// need to initialize waiting_code
		pgs->waiting_code = c;
		pgs->first_byte = FALSE;
		return;
	}

	// Probe hash table to see if a symbol exists for
	// waiting_code followed by c.
	// If so, replace waiting_code by that symbol and return.
	i = ((short) c << (MAX_LZW_BITS - 8)) + pgs->waiting_code;

	// i is less than twice 2**MAX_LZW_BITS, therefore less than twice HSIZE
	if (i >= HSIZE)
		i -= HSIZE;

	probe_value = HASH_ENTRY(pgs->waiting_code, c);

	if (pgs->hash_code[i] != 0) {
		// is first probed slot empty?
		if (pgs->hash_value[i] == probe_value) {
			pgs->waiting_code = pgs->hash_code[i];
			return;
		}

		// secondary hash (after G. Knott)
		if (i == 0)
			disp = 1;
		else
			disp = HSIZE - i;

		for (;;) {
			i -= disp;
			if (i < 0)
				i += HSIZE;
			if (pgs->hash_code[i] == 0)
				break;						// hit empty slot
			if (pgs->hash_value[i] == probe_value) {
				pgs->waiting_code = pgs->hash_code[i];
				return;
			}
		}
	}

	// here when hashtable[i] is an empty slot; desired symbol not in table
	output(pgs, pgs->waiting_code);
	if (pgs->free_code < LZW_TABLE_SIZE) {
		// add symbol to hashtable
		pgs->hash_code[i] = pgs->free_code++;
		pgs->hash_value[i] = probe_value;
	}
	else
		clear_block(pgs);
	pgs->waiting_code = c;
}


////////////////////////////////////////////////////////////////////////
// Clean up at end

static void compress_term(LPGIFSAVE pgs)
{
	// Flush out the buffered code
	if (! pgs->first_byte)
		output(pgs, pgs->waiting_code);

	// Send an EOF code
	output(pgs, pgs->EOFCode);

	// Flush the bit-packing buffer
	if (pgs->cur_bits > 0)
		char_out(pgs, pgs->cur_accum & 0xFF);

	// Flush the packet buffer
	flush_packet(pgs);
}

////////////////////////////////////////////////////////////////////////
// Startup: write the file header.

static void start_output_gif(LPGIFSAVE pgs, mif& image, GIF_CPARAMS* pgc)
{
	int i;
	int mapsize, codesize, flag;

	if (image.m_colors > 256)
		return;							// JERR_TOO_MANY_COLORS

	// Compute bits/pixel and related values
	int bitcnt;
	if (image.m_colors > 128)		bitcnt = 8;
	else if (image.m_colors > 64)	bitcnt = 7;
	else if (image.m_colors > 32)	bitcnt = 6;
	else if (image.m_colors > 16)	bitcnt = 5;
	else if (image.m_colors > 8)	bitcnt = 4;
	else if (image.m_colors > 4)	bitcnt = 3;
	else if (image.m_colors > 2)	bitcnt = 2;
	else							bitcnt = 1;

	mapsize = 1 << bitcnt;
	if (bitcnt <= 1)
		codesize = 2;
	else
		codesize = bitcnt;

	// Write the GIF header.
	// Note that we generate a plain GIF87 header for maximum compatibility.
	if (pgc->gif87a)
		_mfwrite(pgs->pmf, "GIF87a", 6);
	else
		_mfwrite(pgs->pmf, "GIF89a", 6);

	// Write the Logical Screen Descriptor
	_mfputw(pgs->pmf, (WORD)image.m_width);
	_mfputw(pgs->pmf, (WORD)image.m_height);
	flag = 0x80;							// Yes, there is a global color table
	flag |= (bitcnt - 1) << 4;				// color resolution
	flag |= (bitcnt - 1);					// size of global color table
	_mfputc(pgs->pmf, flag);
	if (pgc->xparent)
		_mfputc(pgs->pmf, pgc->xparent_index);	// 背景色インデクス
	else
		_mfputc(pgs->pmf, 0);
	_mfputc(pgs->pmf, 0);					// ピクセルアスペクトレート

	// Write the Global Color Map
	// If the color map is more than 8 bits precision,
	//  we reduce it to 8 bits by shifting
	for (i = 0; i < mapsize; i++) {
		if (i < image.m_colors) {
			// Normal case: RGB color map
			_mfputc(pgs->pmf, image.m_map[i].red);
			_mfputc(pgs->pmf, image.m_map[i].green);
			_mfputc(pgs->pmf, image.m_map[i].blue);
		}
		else {
			// fill out the map to a power of 2
			_mfputc(pgs->pmf, 0);
			_mfputc(pgs->pmf, 0);
			_mfputc(pgs->pmf, 0);
		}
	}

	// グラフィック制御拡張
	if (!pgc->gif87a) {
		_mfputc(pgs->pmf, '!');			// Extension Introducer
		_mfputc(pgs->pmf, 0xf9);			// Graphic Control Label
		_mfputc(pgs->pmf, 4);				// Block Size
		flag = (pgc->xparent) ? 0x01 : 0;	// 背景色フラグ
		_mfputc(pgs->pmf, (BYTE)flag);		// Packed Fields
		_mfputw(pgs->pmf, 0);				// DelayTime
		_mfputc(pgs->pmf, flag ? (BYTE)pgc->xparent_index : 0);
											// Xparet Color Index
		_mfputc(pgs->pmf, 0);				// BlockTerminator
	}

	// Write image separator and Image Descriptor
	_mfputc(pgs->pmf, ',');
	_mfputw(pgs->pmf, 0);
	_mfputw(pgs->pmf, 0);
	_mfputw(pgs->pmf, (WORD)image.m_width);
	_mfputw(pgs->pmf, (WORD)image.m_height);

	// flag byte: interlaced  = 0x40
	//            local_color = 0x8? ... low 4bit is (bits - 1)
	flag = 0;
	if (pgc->interlaced)
		flag |= 0x40;
	_mfputc(pgs->pmf, (BYTE)(flag & 0xFF));

	// Write Initial Code Size byte
	_mfputc(pgs->pmf, codesize);

	// Initialize for LZW compression of image data
	compress_init(pgs, codesize + 1);
}


////////////////////////////////////////////////////////////////////////
// Write some pixel data.
// In this module rows_supplied will always be 1.

static void put_pixel_rows(LPGIFSAVE pgs, int width, RGBA* p)
{
	for (int i = 0; i < width; i++) {
		compress_byte(pgs, p->alpha);
		p++;
	}
}


////////////////////////////////////////////////////////////////////////
// The module selection routine for GIF format output.
// 8bpp以下でないと正常に動作しない

static int interlace_start[4] = { 0, 4, 2, 1 };
static int interlace_rate[4] = { 8, 8, 4, 2 };

extern "C"
HANDLE WINAPI imgGifSaveMemParamsSt(HDIB hdib, GIF_CPARAMS* pgc, DWORD* pdwSize)
{
	mif image;
	if (!image.attachdib(hdib))
		return NULL;

	register RGBA* ps;
	if (image.m_bits == 24)
		image.quantize(ditherFS, 256, NULL);
	else {
		// 利用していない色を無視して保存サイズを小さくする
		int maxindex = 0;
		ps = image.m_pixels;
		int cnt = image.m_packets;
		while (cnt-- > 0) {
			register RGBA* map = image.m_map;
			int n;
			for (n = 0; n < ps->alpha; n++) {
				if (map->red == image.m_map[ps->alpha].red &&
					map->green == image.m_map[ps->alpha].green &&
					map->blue == image.m_map[ps->alpha].blue)
					break;
				map++;
			}
			if (n < ps->alpha)
				ps->alpha = n;
			maxindex = max(maxindex, ps->alpha);
			ps++;
		}
		image.m_colors = min(image.m_colors, maxindex + 1);
	}


	BOOL bOK = FALSE;

	LPGIFSAVE pgs = InitEncoder(image.m_width * image.m_width + sizeof(RGBQUAD) * 256);
	if (!pgs)
		goto exit_gif;

	// ヘッダの書き出し
	start_output_gif(pgs, image, pgc);

	int i, y;
	if (pgc->interlaced) {
		for (i = 0; i < 4; i++) {
			y = interlace_start[i];
			while (y < image.m_height) {
				ps = image.pos(0, y);
				put_pixel_rows(pgs, image.m_width, ps);
				y += interlace_rate[i];
			}
		}
	}
	else {
		ps = image.m_pixels;
		for (y = 0; y < image.m_height; y++) {
			put_pixel_rows(pgs, image.m_width, ps);
			ps += image.m_width;
		}
	}

	// Flush LZW mechanism
	compress_term(pgs);

	// 長さ 0 のデータブロックを出力
	_mfputc(pgs->pmf, 0);

	// GIFターミネータマークを出力
	_mfputc(pgs->pmf, ';');

	GlobalUnlock(hdib);
	bOK = TRUE;

exit_gif:
	HGLOBAL hmem = NULL;
	if (bOK) {
		size_t size;
		hmem = _mfdetach(pgs->pmf, &size);
		pgs->pmf = NULL;
		*pdwSize = (DWORD)size;
	}

	TermEncoder(pgs);
	return hmem;
}

extern "C"
HANDLE WINAPI imgGifSaveMemParams(HDIB hdib, BOOL bGIF87a, BOOL bInterlaced, DWORD* pdwSize)
{
	GIF_CPARAMS gc;
	memset(&gc, 0, sizeof(gc));
	gc.gif87a = bGIF87a;
	gc.interlaced = bInterlaced;
	return imgGifSaveMemParamsSt(hdib, &gc, pdwSize);
}

extern "C"
BOOL WINAPI imgGifSaveParamsSt(LPCTSTR pszFile, HDIB hdib, GIF_CPARAMS* pgc)
{
	DWORD dwSize;
	HGLOBAL hmem = imgGifSaveMemParamsSt(hdib, pgc, &dwSize);
	return SaveMemToFileAndWep(pszFile, hmem, dwSize);
}

extern "C"
BOOL WINAPI imgGifSaveParams(LPCTSTR pszFile, HDIB hdib, BOOL bGIF87a, BOOL bInterlaced)
{
	GIF_CPARAMS gc;
	memset(&gc, 0, sizeof(gc));
	gc.gif87a = bGIF87a;
	gc.interlaced = bInterlaced;
	return imgGifSaveParamsSt(pszFile, hdib, &gc);
}

extern "C"
BOOL WINAPI imgGifSave(LPCTSTR pszFile, HDIB hdib)
{
	return imgGifSaveParams(pszFile, hdib, TRUE, FALSE);
}

