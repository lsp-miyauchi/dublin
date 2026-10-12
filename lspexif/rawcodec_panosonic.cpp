/*
   dcraw.c -- Dave Coffin's raw photo decoder
   Copyright 1997-2007 by Dave Coffin, dcoffin a cybercom o net

   $Revision: 1.397 $
   $Date: 2007/12/10 07:43:31 $
 */

#include "stdafx.h"
#include "exifparse.h"
/*
#include <tchar.h>
#include <windows.h>
#include <shlwapi.h>
#include <fcntl.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

#include "exiftag.h"

#pragma comment(lib, "shlwapi.lib")
*/

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef __int64 INT64;
typedef unsigned __int64 UINT64;

#ifdef USE_WINSOCK_FUNCTIONS
#pragma comment(lib, "ws2_32.lib")
#define _htonl htonl
#define _ntohl ntohl
#else
static __inline ulong _htonl(register ulong s)
	{ return (((s & 0xff) << 24) | ((s & 0xff00) << 8) | ((s & 0xff0000) >> 8) | ((s & 0xff000000) >> 24)); }
static __inline ulong _ntohl(register ulong s)
	{ return (((s & 0xff) << 24) | ((s & 0xff00) << 8) | ((s & 0xff0000) >> 8) | ((s & 0xff000000) >> 24)); }
#endif	// USE_WINSOCK_FUNCTIONS




static const double xyz_rgb[3][3] = {			/* XYZ from RGB */
			{ 0.412453, 0.357580, 0.180423 },
			{ 0.212671, 0.715160, 0.072169 },
			{ 0.019334, 0.119193, 0.950227 } };
static const float d65_white[3] = { 0.950456F, 1.0F, 1.088754F };

static struct decode {
	struct decode *branch[2];
	int leaf;
} first_decode[2048], *second_decode, *free_decode;

static struct {
	int width;
	int height;
	int bps;
	int comp;
	int phint;
	int offset;
	int flip;
	int samples;
	int bytes;
} tiff_ifd[10];

/*
   Not a full implementation of Lossless JPEG, just
   enough to decode Canon, Kodak and Adobe DNG images.
 */
typedef struct tagJHEAD {
	int bits, high, wide, clrs, psv, restart, vpred[4];
	struct decode *huff[4];
	ushort *row;
} JHEAD;


#define FORC3			for (c=0; c < 3; c++)
#define FORC4			for (c=0; c < 4; c++)
#define FORCC			for (c=0; c < m_colors; c++)

#define SQR(x)			((x)*(x))
#define ABS(x)			(((int)(x) ^ ((int)(x) >> 31)) - ((int)(x) >> 31))
#define MIN(a,b)		((a) < (b) ? (a) : (b))
#define MAX(a,b)		((a) > (b) ? (a) : (b))
#define LIM(x,min,max)	MAX(min,MIN(x,max))
#define ULIM(x,y,z)		((y) < (z) ? LIM(x,y,z) : LIM(x,z,y))
#define CLIP(x)			LIM(x,0,65535)
#define SWAP(a,b)		{ a ^= b; a ^= (b ^= a); }

/*
   All global variables are defined here, and all functions that
   access them are prefixed with "CLASS".  Note that a thread-safe
   C++ class cannot have non-const static local variables.
 */


/*
   In order to inline this calculation, I make the risky
   assumption that all filter patterns can be described
   by a repeating pattern of eight rows and two columns

   Do not use the FC or BAYER macros with the Leaf CatchLight,
   because its pattern is 16x16, not 2x8.

   Return values are either 0/1/2/3 = G/M/C/Y or 0/1/2/3 = R/G1/B/G2

	PowerShot 600	PowerShot A50	PowerShot Pro70	Pro90 & G1
	0xe1e4e1e4:	0x1b4e4b1e:	0x1e4b4e1b:	0xb4b4b4b4:

	  0 1 2 3 4 5	  0 1 2 3 4 5	  0 1 2 3 4 5	  0 1 2 3 4 5
	0 G M G M G M	0 C Y C Y C Y	0 Y C Y C Y C	0 G M G M G M
	1 C Y C Y C Y	1 M G M G M G	1 M G M G M G	1 Y C Y C Y C
	2 M G M G M G	2 Y C Y C Y C	2 C Y C Y C Y
	3 C Y C Y C Y	3 G M G M G M	3 G M G M G M
			4 C Y C Y C Y	4 Y C Y C Y C
	PowerShot A5	5 G M G M G M	5 G M G M G M
	0x1e4e1e4e:	6 Y C Y C Y C	6 C Y C Y C Y
			7 M G M G M G	7 M G M G M G
	  0 1 2 3 4 5
	0 C Y C Y C Y
	1 G M G M G M
	2 C Y C Y C Y
	3 M G M G M G

   All RGB cameras use one of these Bayer grids:

	0x16161616:	0x61616161:	0x49494949:	0x94949494:

	  0 1 2 3 4 5	  0 1 2 3 4 5	  0 1 2 3 4 5	  0 1 2 3 4 5
	0 B G B G B G	0 G R G R G R	0 G B G B G B	0 R G R G R G
	1 G R G R G R	1 B G B G B G	1 R G R G R G	1 G B G B G B
	2 B G B G B G	2 G R G R G R	2 G B G B G B	2 R G R G R G
	3 G R G R G R	3 B G B G B G	3 R G R G R G	3 G B G B G B
 */



////////////////////////////////////////////////////////////////////////
// class panaraw

class panaraw
{
public:
	FILE *m_ifp;
	const TCHAR* m_ifname;
	short m_order;
	unsigned m_filters;
	long m_data_offset;
	long m_thumb_offset;
	unsigned m_thumb_misc;
	unsigned* m_oprof;
	unsigned m_tiff_nifds;
	unsigned m_tiff_samples;
	unsigned m_tiff_bps;
	unsigned m_tiff_compress;
	unsigned m_black;
	unsigned m_maximum;
	unsigned m_mix_green;
	unsigned m_raw_color;
	unsigned m_use_gamma;
	unsigned m_zero_is_bad;
	unsigned m_zero_after_ff;
	unsigned m_is_raw;
	unsigned m_data_error;
	ushort m_raw_height;
	ushort m_raw_width;
	ushort m_height;
	ushort m_width;
	ushort m_top_margin;
	ushort m_left_margin;
	ushort m_shrink;
	ushort m_iheight;
	ushort m_iwidth;
	ushort m_fuji_width;
	ushort m_thumb_width;
	ushort m_thumb_height;
	int m_flip;
	int m_tiff_flip;
	int m_colors;
	int m_four_color_rgb;
	int m_med_passes;
	double m_aber[4];				// 色収差
	ushort (*m_image)[4];
	ushort m_white[8][8];
	ushort m_cr2_slice[3];
	ushort* m_curve;
	float m_bright;
	float m_user_mul[4];
	int m_output_colorspace;
	unsigned m_greybox[4];
	float m_cam_mul[4];
	float m_pre_mul[4];
	float m_cmatrix[3][4];
	float m_rgb_cam[3][4];
	double m_pixel_aspect;
	int (*m_histogram)[4];
	void (*m_load_raw)(void*);

	// --------------------
	// EXIF情報
	// --------------------
	char m_make[64];
	char m_model[64];
	char m_date[32];
	float m_iso_speed;
	float m_shutter;
	float m_aperture;
	float m_max_aperture;
	float m_focal_len;
	float m_exposure_bias;
	int m_flash_used;
	int m_exposure_program;
	int m_metering_mode;
	int m_white_balance;
	int m_orientation;
	int m_half_size;								// 0 = original_size, 1 = 1/2, 2 = 1/4, 3 = 1/8, 4 = 1/16

	// --------------------
	// boolean
	// --------------------
	bool m_use_camera_wb;
	bool m_use_camera_matrix;
	bool m_no_auto_bright;
//@	bool m_verbose;

public:
	panaraw()
	{
		m_bright = 1;
		m_output_colorspace = 1;
		m_aber[0] = m_aber[1] = m_aber[2] = m_aber[3] = 1;
		m_user_mul[0] = m_user_mul[1] = m_user_mul[2] = m_user_mul[3] = 0;
		m_greybox[0] = m_greybox[1] = 0; m_greybox[2] = m_greybox[3] = UINT_MAX;
		m_four_color_rgb = 0;
		m_med_passes = 0;

		m_half_size = 0;
		m_use_camera_wb = false;
		m_use_camera_matrix = false;
		m_no_auto_bright = false;
//@		m_verbose = false;

		m_curve = new ushort[0x4001];
		m_histogram = new int[0x2000][4];
	}
	~panaraw()
	{
		if (m_curve) delete [] m_curve;
		if (m_histogram) delete [] m_histogram;
	}

//@	#define FC(row,col) 		(m_filters >> ((((row) << 1 & 14) + ((col) & 1)) << 1) & 3)
//@	#define BAYER(row,col) 		m_image[((row) >> m_shrink) * m_iwidth + ((col) >> m_shrink)][FC(row, col)]
//@	#define BAYER2(row,col) 	m_image[((row) >> m_shrink) * m_iwidth + ((col) >> m_shrink)][fc(row, col)]
	inline ushort FC(int row, int col) { return (m_filters >> ((((row) << 1 & 14) + ((col) & 1)) << 1) & 3); }
	inline ushort& BAYER(int row, int col) { return m_image[((row) >> m_shrink) * m_iwidth + ((col) >> m_shrink)][FC(row, col)]; }
	inline ushort& BAYER2(int row, int col) { return m_image[((row) >> m_shrink) * m_iwidth + ((col) >> m_shrink)][fc(row, col)]; }

	inline void AdjustBrightness(float bright) { m_bright = bright; }
	inline void SetCustomWhiteBalance(float r, float g1, float b, float g2) { m_user_mul[0] = r; m_user_mul[1] = g1; m_user_mul[2] = b; m_user_mul[3] = g2; }
	inline void AverageGreyBox(unsigned x, unsigned y, unsigned w, unsigned h) { m_greybox[0] = x; m_greybox[1] = y; m_greybox[2] = w; m_greybox[3] = h; }
	inline void CorrectChromaticAberration(float r, float b) { m_aber[0] = 1 / r; m_aber[1] = 1 / b; }
	inline void SetColorspace(int type) { m_output_colorspace = type; }
	inline void SetHalfSizeImage(int half) { m_half_size = half; m_four_color_rgb = 1; }
	inline void UseCameraWhiteBalance(bool use) { m_use_camera_wb = use; }
	inline void UseCameraMatrix(bool use) { m_use_camera_matrix = use; }
	inline void NoAutoBright(bool yesno) { m_no_auto_bright = yesno; }
//@	inline void Verbose(bool enable) { m_verbose = enable; }

	int fc(int row, int col);
	void merror(void *ptr, char *where);
	void derror();
	ushort sget2(uchar *s);
	ushort get2();
	unsigned sget4(uchar *s);
	unsigned get4();
	unsigned getint(int type);
	float int_to_float(int i);
	double getreal(int type);
	void read_shorts(ushort *pixel, int count);
	void remove_zeroes();
	unsigned getbits(int nbits);
	unsigned ph1_bits(int nbits);
	void init_decoder();
	uchar* make_decoder(const uchar *source, int level);
	int ljpeg_start(JHEAD* jh, int info_only);
	int ljpeg_diff(struct decode *dindex);
	ushort* ljpeg_row(int jrow, JHEAD* jh);
	int bayer(unsigned row, unsigned col);
	unsigned pana_bits(int nbits);
	void pseudoinverse(double (*in)[3], double (*out)[3], int size);
	void cam_xyz_coeff(double cam_xyz[4][3]);
	void scale_colors();
	void pre_interpolate();
	void border_interpolate(int border);
	void lin_interpolate();
	void ahd_interpolate();
	void tiff_get(unsigned base, unsigned *tag, unsigned *type, unsigned *len, unsigned *save);
	void parse_exif(int base);
	void linear_table(unsigned len);
	int parse_tiff_ifd(int base);
	void parse_tiff(int base);
	void adobe_coeff(char *maker, char *model);
	void identify();
	void convert_to_rgb();
	void fuji_rotate();
	void stretch();
	int flip_index(int row, int col);
	void gamma_lut(uchar* lut);
#ifdef WIN32_CONSOLE_EXECUTE
	void write_dib(FILE *ofp);
#endif
	void write_dib_stream(IStream* pstrm);
	int develop(LPCTSTR pszName, IStream* pstrm = NULL);

	static void stp_lossless_jpeg_load_raw(void* param) { ((panaraw*)param)->lossless_jpeg_load_raw(); }
	static void stp_packed_12_load_raw(void* param) { ((panaraw*)param)->packed_12_load_raw(); }
	static void stp_unpacked_load_raw(void* param) { ((panaraw*)param)->unpacked_load_raw(); }
	static void stp_panasonic_load_raw(void* param) { ((panaraw*)param)->panasonic_load_raw(); }
	static void stp_olympus_e300_load_raw(void* param) { ((panaraw*)param)->olympus_e300_load_raw(); }
	static void stp_nikon_load_raw(void* param) { ((panaraw*)param)->nikon_load_raw(); }
	static void stp_hasselblad_load_raw(void* param) { ((panaraw*)param)->hasselblad_load_raw(); }
	static void stp_eight_bit_load_raw(void* param) { ((panaraw*)param)->eight_bit_load_raw(); }

	void lossless_jpeg_load_raw();
	void packed_12_load_raw();
	void unpacked_load_raw();
	void panasonic_load_raw();
	void olympus_e300_load_raw();
	void nikon_load_raw();
	void hasselblad_load_raw();
	void eight_bit_load_raw();

};

int panaraw::fc(int row, int col)
{
  static const char filter[16][16] =
  { { 2,1,1,3,2,3,2,0,3,2,3,0,1,2,1,0 },
    { 0,3,0,2,0,1,3,1,0,1,1,2,0,3,3,2 },
    { 2,3,3,2,3,1,1,3,3,1,2,1,2,0,0,3 },
    { 0,1,0,1,0,2,0,2,2,0,3,0,1,3,2,1 },
    { 3,1,1,2,0,1,0,2,1,3,1,3,0,1,3,0 },
    { 2,0,0,3,3,2,3,1,2,0,2,0,3,2,2,1 },
    { 2,3,3,1,2,1,2,1,2,1,1,2,3,0,0,1 },
    { 1,0,0,2,3,0,0,3,0,3,0,3,2,1,2,3 },
    { 2,3,3,1,1,2,1,0,3,2,3,0,2,3,1,3 },
    { 1,0,2,0,3,0,3,2,0,1,1,2,0,1,0,2 },
    { 0,1,1,3,3,2,2,1,1,3,3,0,2,1,3,2 },
    { 2,3,2,0,0,1,3,0,2,0,1,2,3,0,1,0 },
    { 1,3,1,2,3,2,3,2,0,2,0,1,1,0,3,0 },
    { 0,2,0,3,1,0,0,1,1,3,3,2,3,2,2,1 },
    { 2,1,3,2,3,1,2,1,0,3,0,2,0,2,0,2 },
    { 0,3,1,0,0,2,0,3,2,1,3,1,1,3,1,3 } };

	if (m_filters != 1) return FC(row,col);
	return filter[(row+m_top_margin) & 15][(col+m_left_margin) & 15];
}

void panaraw::merror(void *ptr, char *where)
{
#ifdef _DEBUG
	if (ptr) return;
	TCHAR buf[256];
	wsprintf(buf, _T("%s: Out of memory in %s"), m_ifname, where);
	::MessageBox(NULL, buf, _T("error"), MB_ICONSTOP | MB_OK);
#endif
}


void panaraw::derror()
{
	if (!m_data_error) {
#ifdef _DEBUG
		TCHAR buf[256];
		wsprintf(buf, _T("%s: "), m_ifname);
		if (feof(m_ifp))
			lstrcat(buf, _T("Unexpected end of file."));
		else {
			TCHAR* p = buf + lstrlen(buf);
			wsprintf(p, _T("Corrupt data near 0x%llx\n"), (INT64)ftell(m_ifp));
		}
		::MessageBox(NULL, buf, _T("error"), MB_ICONSTOP | MB_OK);
#endif
	}
	m_data_error = 1;
}

ushort panaraw::sget2(uchar *s)
{
	if (m_order == 0x4949)			/* "II" means little-endian */
		return s[0] | s[1] << 8;
	else							/* "MM" means big-endian */
		return s[0] << 8 | s[1];
}

ushort panaraw::get2()
{
	uchar str[2] = { 0xff,0xff };
	fread(str, 1, 2, m_ifp);
	return sget2(str);
}

unsigned panaraw::sget4(uchar *s)
{
	if (m_order == 0x4949)
		return s[0] | s[1] << 8 | s[2] << 16 | s[3] << 24;
	else
		return s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3];
}
#define sget4(s) sget4((uchar *)s)

unsigned panaraw::get4()
{
	uchar str[4] = { 0xff,0xff,0xff,0xff };
	fread(str, 1, 4, m_ifp);
	return sget4(str);
}

unsigned panaraw::getint(int type)
{
	return type == 3 ? get2() : get4();
}

float panaraw::int_to_float(int i)
{
	union { int i; float f; } u;
	u.i = i;
	return u.f;
}

double panaraw::getreal(int type)
{
	union { char c[8]; double d; } u;
	int i, rev;

	switch (type) {
	case 3: return (unsigned short)get2();
	case 4: return (unsigned int)get4();
	case 5: u.d = (unsigned int)get4();
			return u.d / (unsigned int)get4();
	case 8: return (signed short)get2();
	case 9: return (signed int)get4();
	case 10: u.d = (signed int)get4();
			return u.d / (signed int)get4();
	case 11: return int_to_float(get4());
	case 12:
			rev = 7 * (m_order != 0x4949);
			for (i=0; i < 8; i++)
				u.c[i ^ rev] = fgetc(m_ifp);
			return u.d;
	default: return fgetc(m_ifp);
	}
}

void panaraw::read_shorts(ushort* pixel, int count)
{
	if (fread(pixel, 2, count, m_ifp) < (unsigned)count) derror();
	if (m_order != 0x4949)
		_swab((char*)pixel, (char*)pixel, count * 2);
}

void panaraw::remove_zeroes()
{
  unsigned row, col, tot, n, r, c;

  for (row=0; row < m_height; row++)
    for (col=0; col < m_width; col++)
      if (BAYER(row,col) == 0) {
	tot = n = 0;
	for (r = row-2; r <= row+2; r++)
	  for (c = col-2; c <= col+2; c++)
	    if (r < m_height && c < m_width &&
		FC(r,c) == FC(row,col) && BAYER(r,c))
	      tot += (n++,BAYER(r,c));
	if (n) BAYER(row,col) = tot/n;
      }
}


/*
   getbits(-1) initializes the buffer
   getbits(n) where 0 <= n <= 25 returns an n-bit integer
 */
unsigned panaraw::getbits(int nbits)
{
  static unsigned bitbuf=0;
  static int vbits=0, reset=0;
  unsigned c;

  if (nbits == -1)
    return bitbuf = vbits = reset = 0;
  if (nbits == 0 || reset) return 0;
  while (vbits < nbits) {
    if ((c = fgetc(m_ifp)) == EOF) derror();
    if ((reset = m_zero_after_ff && c == 0xff && fgetc(m_ifp))) return 0;
    bitbuf = (bitbuf << 8) + (uchar) c;
    vbits += 8;
  }
  vbits -= nbits;
  return bitbuf << (32-nbits-vbits) >> (32-nbits);
}

unsigned panaraw::ph1_bits(int nbits)
{
	static UINT64 bitbuf = 0;
	static int vbits = 0;

	if (nbits == -1)
		return bitbuf = vbits = 0;
	if (nbits == 0) return 0;
	if (vbits < nbits) {
		bitbuf = bitbuf << 32 | get4();
		vbits += 32;
	}
	vbits -= nbits;
	return bitbuf << (64 - nbits - vbits) >> (64 - nbits);
}

void panaraw::init_decoder()
{
	memset(first_decode, 0, sizeof(first_decode));
	free_decode = first_decode;
}

/*
   Construct a decode tree according the specification in *source.
   The first 16 bytes specify how many codes should be 1-bit, 2-bit
   3-bit, etc.  Bytes after that are the leaf values.

   For example, if the source is

    { 0,1,4,2,3,1,2,0,0,0,0,0,0,0,0,0,
      0x04,0x03,0x05,0x06,0x02,0x07,0x01,0x08,0x09,0x00,0x0a,0x0b,0xff  },

   then the code is

	00		0x04
	010		0x03
	011		0x05
	100		0x06
	101		0x02
	1100		0x07
	1101		0x01
	11100		0x08
	11101		0x09
	11110		0x00
	111110		0x0a
	1111110		0x0b
	1111111		0xff
 */
uchar* panaraw::make_decoder(const uchar *source, int level)
{
	struct decode *cur;
	static int leaf;
	int i, next;

	if (level==0) leaf=0;
	cur = free_decode++;
	if (free_decode > first_decode + 2048) {
		// decoder table overflow
		return NULL;
	}
	for (i=next=0; i <= leaf && next < 16; )
		i += source[next++];
		if (i > leaf) {
			if (level < next) {
				cur->branch[0] = free_decode;
				if (!make_decoder(source, level + 1)) return NULL;
				cur->branch[1] = free_decode;
				if (!make_decoder(source, level + 1)) return NULL;
			}
			else
				cur->leaf = source[16 + leaf++];
		}
	return (uchar*)source + 16 + leaf;
}

int panaraw::ljpeg_start(JHEAD* jh, int info_only)
{
	int i, tag, len;

	uchar* data = new uchar[0x10000];
	if (!data)
		return 0;

	init_decoder();
	memset(jh, 0, sizeof(JHEAD));
	for (i=0; i < 4; i++)
		jh->huff[i] = free_decode;
	jh->restart = INT_MAX;
	fread(data, 2, 1, m_ifp);
	if (data[1] != 0xd8) {
		delete [] data;
		return 0;
	}

	uchar* dp;
	do {
		fread (data, 2, 2, m_ifp);
		tag =  data[0] << 8 | data[1];
		len = (data[2] << 8 | data[3]) - 2;
		if (tag <= 0xff00) {
			delete [] data;
			return 0;
		}
		fread(data, 1, len, m_ifp);
		switch (tag) {
		case 0xffc0:
			data[7] = 0;
		case 0xffc3:
			jh->bits = data[0];
			jh->high = data[1] << 8 | data[2];
			jh->wide = data[3] << 8 | data[4];
			jh->clrs = data[5] + (data[7] == 0x21);
			if (len == 9)
				getc(m_ifp);
			break;
		case 0xffc4:
			if (info_only) break;
			for (dp = data; dp < data+len && *dp < 4; ) {
				jh->huff[*dp] = free_decode;
				dp = make_decoder(++dp, 0);
				if (!dp) {
					delete [] data;
					return 0;
				}
			}
			break;
 		case 0xffda:
			jh->psv = data[1+data[0]*2];
			break;
		case 0xffdd:
			jh->restart = data[0] << 8 | data[1];
		}
	} while (tag != 0xffda);

	delete [] data;

	if (info_only) return 1;
	if (jh->clrs == 4) {
		jh->huff[3] = jh->huff[2] = jh->huff[1];
		jh->huff[1] = jh->huff[0];
	}
	jh->row = (ushort*)calloc(jh->wide * jh->clrs, 4);
	merror(jh->row, "ljpeg_start()");
	return m_zero_after_ff = 1;
}

int panaraw::ljpeg_diff(struct decode *dindex)
{
  int len, diff;

  while (dindex->branch[0])
    dindex = dindex->branch[getbits(1)];
  len = dindex->leaf;
  if (len == 16)
    return -32768;
  diff = getbits(len);
  if ((diff & (1 << (len-1))) == 0)
    diff -= (1 << len) - 1;
  return diff;
}

ushort* panaraw::ljpeg_row(int jrow, JHEAD* jh)
{
  int col, c, diff, pred;
  ushort mark=0, *row[3];

  if (jrow * jh->wide % jh->restart == 0) {
    FORC4 jh->vpred[c] = 1 << (jh->bits-1);
    if (jrow)
      do mark = (mark << 8) + (c = fgetc(m_ifp));
      while (c != EOF && mark >> 4 != 0xffd);
    getbits(-1);
  }
  FORC3 row[c] = jh->row + jh->wide*jh->clrs*((jrow+c) & 1);
  for (col=0; col < jh->wide; col++)
    for (c=0; c < jh->clrs; c++) {
      diff = ljpeg_diff (jh->huff[c]);
      if (jh->clrs == 4 && c < 2 && (col | c))
		    pred = row[0][(c << 1)-3];
      else if (col) pred = row[0][-jh->clrs];
      else	    pred = (jh->vpred[c] += diff) - diff;
      if (jrow && col) switch (jh->psv) {
	case 1:	break;
	case 2: pred = row[1][0];					break;
	case 3: pred = row[1][-jh->clrs];				break;
	case 4: pred = pred +   row[1][0] - row[1][-jh->clrs];		break;
	case 5: pred = pred + ((row[1][0] - row[1][-jh->clrs]) >> 1);	break;
	case 6: pred = row[1][0] + ((pred - row[1][-jh->clrs]) >> 1);	break;
	case 7: pred = (pred + row[1][0]) >> 1;				break;
	default: pred = 0;
      }
      if ((**row = pred + diff) >> jh->bits) derror();
      row[0]++; row[1]++;
    }
  return row[2];
}

void panaraw::lossless_jpeg_load_raw()
{
	int jidx, i, j, row=0, col=0;
	int min=INT_MAX;
	ushort *rp;

	JHEAD jh;
	if (!ljpeg_start(&jh, 0)) return;

	int jwide = jh.wide * jh.clrs;

	for (int jrow=0; jrow < jh.high; jrow++) {
		ushort* rp = ljpeg_row(jrow, &jh);
		for (int jcol = 0; jcol < jwide; jcol++) {
			int val = *rp++;
			if (jh.bits <= 12)
				val = m_curve[val];
			if (m_cr2_slice[0]) {
				jidx = jrow * jwide + jcol;
				i = jidx / (m_cr2_slice[1] * jh.high);
				if ((j = i >= m_cr2_slice[0]))
					i  = m_cr2_slice[0];
				jidx -= i * (m_cr2_slice[1] * jh.high);
				row = jidx / m_cr2_slice[1 + j];
				col = jidx % m_cr2_slice[1 + j] + i * m_cr2_slice[1];
			}
			if (m_raw_width == 3984 && (col -= 2) < 0)
				col += (row--, m_raw_width);
			if ((unsigned)(row - m_top_margin) < m_height) {
				if ((unsigned)(col - m_left_margin) < m_width) {
					BAYER(row - m_top_margin,col - m_left_margin) = val;
					if (min > val)
						min = val;
				}
				else
					m_black += val;
			}
			if (++col >= m_raw_width)
				col = (row++, 0);
		}
	}
	free(jh.row);
	if (m_raw_width > m_width)
		m_black /= (m_raw_width - m_width) * m_height;
}

int panaraw::bayer(unsigned row, unsigned col)
{
	return (row < m_height && col < m_width) ? BAYER(row,col) : 0;
}

void panaraw::packed_12_load_raw()
{
	if (m_raw_width * 2 < m_width * 3)
		m_raw_width = m_raw_width * 3 / 2;	/* Convert m_raw_width to bytes */
	getbits(-1);

	int row, col;
	for (row = 0; row < m_height; row++) {
		for (col=0; col < m_left_margin; col++)
			getbits(12);
		for (col=0; col < m_width; col++)
			BAYER(row,col) = getbits(12);
		for (col = (m_width+m_left_margin)*3/2; col < m_raw_width; col++)
			if (getbits(8) && m_raw_width-col < 35 && m_width != 3896) derror();
	}
}

void panaraw::unpacked_load_raw()
{
	int bits = 0;
	while (1 << ++bits < (int)m_maximum)
		;
	fseek(m_ifp, (m_top_margin * m_raw_width + m_left_margin) * 2, SEEK_CUR);

	ushort* pixel = new ushort[m_width];
	if (pixel) {
		for (int row = 0; row < m_height; row++) {
			read_shorts(pixel, m_width);
			fseek(m_ifp, 2 * (m_raw_width - m_width), SEEK_CUR);
			for (int col = 0; col < m_width; col++)
				if ((BAYER2(row, col) = pixel[col]) >> bits) derror();
		}
		delete [] pixel;
	}
}

unsigned panaraw::pana_bits(int nbits)
{
	static uchar buf[16], vbits = 0;

	if (!vbits && fread (buf, 1, 16, m_ifp) < 16) derror();
	vbits = (vbits - nbits) & 127;
	return (buf[(vbits >> 3)+1] << 8 | buf[vbits >> 3]) >> (vbits & 7) & ~(-1 << nbits);
}

void panaraw::panasonic_load_raw()
{
	int row, col, i, j, sh=0, pred[2], nonz[2];

	m_raw_width = (m_raw_width + 13) / 14 * 14;
	for (row = 0; row < m_height; row++)
		for (col = 0; col < m_raw_width; col++) {
			if ((i = col % 14) < 2)
				nonz[i] = pred[i] = pana_bits(12);
			else {
				if (i % 3 == 2)
					sh = 4 >> (3 - pana_bits(2));
				if ((j = pana_bits(8))) {
					if ((pred[i & 1] -= 0x80 << sh) < 0 || sh == 4)
						pred[i & 1] &= ~(-1 << sh);
					pred[i & 1] += nonz[i & 1] ? j << sh : j;
					nonz[i & 1] = 1;
				}
			}
			if (col < m_width)
				if ((BAYER(row,col) = pred[col & 1]) >> 12) derror();
		}
	m_maximum = 0xf96;
	m_black = 15;
}

void panaraw::olympus_e300_load_raw()
{
	int dwide = m_raw_width * 16 / 10;
	fseek(m_ifp, dwide * m_top_margin, SEEK_CUR);
	uchar* data = (uchar*)malloc(dwide + m_raw_width * 2);
	merror(data, "olympus_e300_load_raw()");
	ushort* pixel = (ushort*)(data + dwide);
	for (int row = 0; row < m_height; row++) {
		if (fread(data, 1, dwide, m_ifp) < dwide) derror();
		uchar* dp;
		ushort* pix;
		for (dp = data, pix = pixel; pix < pixel + m_raw_width; dp+=3, pix+=2) {
			if (((dp - data) & 15) == 15)
				if (*dp++ && pix < pixel + m_width + m_left_margin) derror();
			pix[0] = dp[1] << 8 | dp[0];
			pix[1] = dp[2] << 4 | dp[1] >> 4;
		}
		for (int col = 0; col < m_width; col++)
			BAYER(row,col) = (pixel[col + m_left_margin] & 0xfff);
	}
	free(data);
	m_maximum >>= 4;
	m_black >>= 4;
}

void panaraw::nikon_load_raw()
{
	getbits(-1);
	for (int irow = 0; irow < m_height; irow++) {
		int row = irow;
		if (m_make[0] == 'O' || m_model[0] == 'E') {
			// for Olympus E-XXX
			row = irow * 2 % m_height + irow / (m_height / 2);
			if (row == 1 && m_data_offset == 0) {
				fseek(m_ifp, 0, SEEK_END);
				fseek(m_ifp, ftell(m_ifp) / 2, SEEK_SET);
				getbits(-1);
			}
		}
		for (int col = 0; col < m_raw_width; col++) {
			int i = getbits(12);
			if ((unsigned)(col - m_left_margin) < m_width)
				BAYER(row, col - m_left_margin) = i;
			if (m_tiff_compress > 32768 && (col % 10) == 9)
				if (getbits(8)) derror();
		}
	}
}

void panaraw::hasselblad_load_raw()
{
	JHEAD jh;
	if (!ljpeg_start(&jh, 0)) return;

	free(jh.row);
	ph1_bits(-1);		// init

	int pred[2], len[2];
	for (int row = - m_top_margin; row < m_height; row++) {
		pred[0] = pred[1] = 0x8000;
		for (int col = - m_left_margin; col < m_raw_width - m_left_margin; col += 2) {
			int i;
			for (i = 0; i < 2; i++) {
				struct decode* dindex;
				for (dindex = jh.huff[0]; dindex->branch[0]; )
					dindex = dindex->branch[ph1_bits(1)];
				len[i] = dindex->leaf;
			}
			for (i = 0; i < 2; i++) {
				int diff = ph1_bits(len[i]);
				if ((diff & (1 << (len[i]-1))) == 0)
					diff -= (1 << len[i]) - 1;
				pred[i] += diff;
				if (row >= 0 && (unsigned)(col + i) < m_width)
					BAYER(row, col + i) = pred[i];
			}
		}
	}
	m_maximum = 0xffff;
}

void panaraw::eight_bit_load_raw()
{
	uchar* pixel = new uchar[m_raw_width];
	merror(pixel, "eight_bit_load_raw()");

	unsigned lblack = 0;
	fseek(m_ifp, m_top_margin * m_raw_width, SEEK_CUR);
	for (unsigned row = 0; row < m_height; row++) {
		if (fread(pixel, 1, m_raw_width, m_ifp) < m_raw_width) derror();
		for (unsigned col = 0; col < m_raw_width; col++) {
			unsigned val = m_curve[pixel[col]];
			if ((unsigned)(col - m_left_margin) < m_width)
				BAYER(row, col - m_left_margin) = val;
			else
				lblack += val;
		}
	}
	delete [] pixel;

	if (m_raw_width > m_width + 1)
		m_black = lblack / ((m_raw_width - m_width) * m_height);
	m_maximum = m_curve[0xff];
}

static void sony_decrypt(register unsigned* data, int len, int start, int key)
{
	static unsigned pad[128], p;

	if (start) {
		for (p=0; p < 4; p++)
			pad[p] = key = key * 48828125 + 1;
		pad[3] = pad[3] << 1 | (pad[0]^pad[2]) >> 31;
		for (p=4; p < 127; p++)
			pad[p] = (pad[p-4]^pad[p-2]) << 1 | (pad[p-3]^pad[p-1]) >> 31;
		for (p=0; p < 127; p++)
			pad[p] = htonl(pad[p]);
	}
	while (len--)
		*data++ ^= pad[p++ & 127] = pad[(p+1) & 127] ^ pad[(p+65) & 127];
}



void panaraw::pseudoinverse(double (*in)[3], double (*out)[3], int size)
{
	double work[3][6];
	int i, j, k;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 6; j++)
			work[i][j] = j == i + 3;
		for (j = 0; j < 3; j++)
			for (k = 0; k < size; k++)
				work[i][j] += in[k][i] * in[k][j];
	}
	for (i = 0; i < 3; i++) {
		double num = work[i][i];
		for (j = 0; j < 6; j++)
			work[i][j] /= num;
		for (k = 0; k < 3; k++) {
			if (k==i) continue;
			num = work[k][i];
			for (j = 0; j < 6; j++)
				work[k][j] -= work[i][j] * num;
		}
	}
	for (i = 0; i < size; i++)
		for (j = 0; j < 3; j++)
			for (out[i][j] = k = 0; k < 3; k++)
				out[i][j] += work[j][k+3] * in[i][k];
}

void panaraw::cam_xyz_coeff(double cam_xyz[4][3])
{
	double cam_rgb[4][3], inverse[4][3], num;
	int i, j, k;

	for (i=0; i < m_colors; i++)		/* Multiply out XYZ colorspace */
		for (j=0; j < 3; j++)
			for (cam_rgb[i][j] = k=0; k < 3; k++)
				cam_rgb[i][j] += cam_xyz[i][k] * xyz_rgb[k][j];

	for (i=0; i < m_colors; i++) {		/* Normalize cam_rgb so that */
		for (num=j=0; j < 3; j++)		/* cam_rgb * (1,1,1) is (1,1,1,1) */
			num += cam_rgb[i][j];
		for (j=0; j < 3; j++)
			cam_rgb[i][j] /= num;
		m_pre_mul[i] = (float)(1 / num);
	}
	pseudoinverse (cam_rgb, inverse, m_colors);
	for (m_raw_color = i=0; i < 3; i++)
		for (j=0; j < m_colors; j++)
			m_rgb_cam[i][j] = (float)inverse[j][i];
}

void panaraw::scale_colors()
{
	unsigned bottom, right, row, col, ur, uc, i, x, y, c, sum[8];
	int val, dblack;
	double dsum[8], dmin, dmax;
	float scale_mul[4], fr, fc;
	ushort *pix;

	if (m_user_mul[0])
		memcpy(m_pre_mul, m_user_mul, sizeof m_pre_mul);
	if (m_use_camera_wb && m_cam_mul[0] == -1) {
		memset(dsum, 0, sizeof dsum);
		bottom = MIN (m_greybox[1]+m_greybox[3], m_height);
		right  = MIN (m_greybox[0]+m_greybox[2], m_width);
		for (row=m_greybox[1]; row < bottom; row += 8)
			for (col=m_greybox[0]; col < right; col += 8) {
				memset (sum, 0, sizeof sum);
				for (y=row; y < row+8 && y < bottom; y++)
					for (x=col; x < col+8 && x < right; x++)
						FORC4 {
							if (m_filters) {
								c = FC(y,x);
								val = BAYER(y,x);
							} else
								val = m_image[y*m_width+x][c];
							if (val > (int)m_maximum- 25) goto skip_block;
							if ((val -= m_black) < 0) val = 0;
							sum[c] += val;
							sum[c+4]++;
							if (m_filters) break;
						}
						for (c = 0; c < 8; c++) dsum[c] += sum[c];
skip_block: ;
			}
		FORC4 if (dsum[c]) m_pre_mul[c] = dsum[c+4] / dsum[c];
	}
	if (m_use_camera_wb && m_cam_mul[0] != -1) {
		memset (sum, 0, sizeof sum);
		for (row=0; row < 8; row++)
			for (col=0; col < 8; col++) {
				c = FC(row,col);
				if ((val = m_white[row][col] - m_black) > 0)
					sum[c] += val;
				sum[c+4]++;
			}
		if (sum[0] && sum[1] && sum[2] && sum[3])
			FORC4 m_pre_mul[c] = (float) sum[c+4] / sum[c];
		else if (m_cam_mul[0] && m_cam_mul[2])
			memcpy (m_pre_mul, m_cam_mul, sizeof m_pre_mul);
#ifdef _DEBUG
		else _ftprintf(stderr, _T("%s: Cannot use camera white balance.\n"), m_ifname);
#endif
	}
	if (m_pre_mul[3] == 0) m_pre_mul[3] = m_colors < 4 ? m_pre_mul[1] : 1;
	dblack = m_black;
	m_maximum -= m_black;
	for (dmin=DBL_MAX, dmax=c=0; c < 4; c++) {
		if (dmin > m_pre_mul[c])
			dmin = m_pre_mul[c];
		if (dmax < m_pre_mul[c])
			dmax = m_pre_mul[c];
	}
	dmax = dmin;
	FORC4 scale_mul[c] = (m_pre_mul[c] /= dmax) * 65535.0 / m_maximum;
//@	if (m_verbose) {
//@		_ftprintf(stderr, _T("Scaling with m_black %d, multipliers"), dblack);
//@		FORC4 _ftprintf(stderr, _T(" %f"), m_pre_mul[c]);
//@		_fputtc('\n', stderr);
//@	}

	unsigned size = m_iheight * m_iwidth;
	for (i=0; i < size * 4; i++) {
		val = m_image[0][i];
		if (!val) continue;
		val -= m_black;
		val *= scale_mul[i & 3];
		m_image[0][i] = CLIP(val);
	}
	if ((m_aber[0] != 1 || m_aber[2] != 1) && m_colors == 3) {
//@		if (m_verbose) _ftprintf(stderr, _T("Correcting chromatic aberration...\n"));

		ushort* imgdata = new ushort[size];
		merror(imgdata, "scale_colors()");
		for (c = 0; c < 4; c += 2) {
			if (m_aber[c] == 1) continue;
			for (i = 0; i < size; i++)
				imgdata[i] = m_image[i][c];
			for (row=0; row < m_iheight; row++) {
				ur = fr = (row - m_iheight*0.5) * m_aber[c] + m_iheight*0.5;
				if (ur > m_iheight-2) continue;
				fr -= ur;
				for (col=0; col < m_iwidth; col++) {
					uc = fc = (col - m_iwidth*0.5) * m_aber[c] + m_iwidth*0.5;
					if (uc > m_iwidth-2) continue;
					fc -= uc;
					pix = imgdata + ur*m_iwidth + uc;
					m_image[row*m_iwidth+col][c] =
								(pix[     0]*(1-fc) + pix[       1]*fc) * (1-fr) +
								(pix[m_iwidth]*(1-fc) + pix[m_iwidth+1]*fc) * fr;
				}
			}
		}
		delete [] imgdata;
	}
}

void panaraw::pre_interpolate()
{
	int row, col, c;

	if (m_shrink) {
		if (m_half_size) {
			m_height = m_iheight;
			m_width  = m_iwidth;
		}
		else {
			ushort (*img)[4] = new ushort[m_height * m_width][4];
			merror(img, "unshrink()");
			for (row = 0; row < m_height; row++)
				for (col = 0; col < m_width; col++) {
					c = fc(row, col);
					img[row * m_width + col][c] = m_image[(row >> 1) * m_iwidth +(col >> 1)][c];
				}
			delete [] m_image;
			m_image = img;
			m_shrink = 0;
		}
	}

	if (m_filters && m_colors == 3) {
		if ((m_mix_green = m_four_color_rgb))
			m_colors++;
		else {
			for (row = FC(1,0) >> 1; row < m_height; row += 2)
				for (col = FC(row, 1) & 1; col < m_width; col += 2)
					m_image[row * m_width + col][1] = m_image[row * m_width + col][3];
			m_filters &= ~((m_filters & 0x55555555) << 1);
		}
	}
	if (m_half_size) m_filters = 0;
}

void panaraw::border_interpolate(int border)
{
  unsigned row, col, y, x, f, c, sum[8];

  for (row=0; row < m_height; row++)
    for (col=0; col < m_width; col++) {
      if (col==border && row >= border && row < m_height-border)
	col = m_width-border;
      memset (sum, 0, sizeof sum);
      for (y=row-1; y != row+2; y++)
	for (x=col-1; x != col+2; x++)
	  if (y < m_height && x < m_width) {
	    f = fc(y,x);
	    sum[f] += m_image[y*m_width+x][f];
	    sum[f+4]++;
	  }
      f = fc(row,col);
      FORCC if (c != f && sum[c+4])
	m_image[row*m_width+col][c] = sum[c] / sum[c+4];
    }
}

void panaraw::lin_interpolate()
{
  int code[16][16][32], *ip, sum[4];
  int c, i, x, y, row, col, shift, color;
  ushort *pix;

//@  if (m_verbose) _ftprintf(stderr, _T("Bilinear interpolation...\n"));

  border_interpolate(1);
  for (row=0; row < 16; row++)
    for (col=0; col < 16; col++) {
      ip = code[row][col];
      memset (sum, 0, sizeof sum);
      for (y=-1; y <= 1; y++)
	for (x=-1; x <= 1; x++) {
	  shift = (y==0) + (x==0);
	  if (shift == 2) continue;
	  color = fc(row+y,col+x);
	  *ip++ = (m_width*y + x)*4 + color;
	  *ip++ = shift;
	  *ip++ = color;
	  sum[color] += 1 << shift;
	}
      FORCC
	if (c != fc(row,col)) {
	  *ip++ = c;
	  *ip++ = 256 / sum[c];
	}
    }
  for (row=1; row < m_height-1; row++)
    for (col=1; col < m_width-1; col++) {
      pix = m_image[row*m_width+col];
      ip = code[row & 15][col & 15];
      memset (sum, 0, sizeof sum);
      for (i=8; i--; ip+=3)
	sum[ip[2]] += pix[ip[0]] << ip[1];
      for (i=m_colors; --i; ip+=2)
	pix[ip[0]] = sum[ip[0]] * ip[1] >> 8;
    }
}

/*
   Adaptive Homogeneity-Directed interpolation is based on
   the work of Keigo Hirakawa, Thomas Parks, and Paul Lee.
 */
#define TS 256		/* Tile Size */

void panaraw::ahd_interpolate()
{
  int i, j, k, top, left, row, col, tr, tc, c, d, val, hm[2];
  ushort (*pix)[4], (*rix)[3];
  static const int dir[4] = { -1, 1, -TS, TS };
  unsigned ldiff[2][4], abdiff[2][4], leps, abeps;
  float r, cbrt[0x10000], xyz[3], xyz_cam[3][4];
  ushort (*rgb)[TS][TS][3];
   short (*lab)[TS][TS][3], (*lix)[3];
   char (*homo)[TS][TS];

//@  if (m_verbose) _ftprintf(stderr, _T("AHD interpolation...\n"));

  for (i=0; i < 0x10000; i++) {
    r = i / 65535.0;
    cbrt[i] = r > 0.008856 ? powf(r, 1 / 3.0) : 7.787 * r + 16 / 116.0;
  }
  for (i=0; i < 3; i++)
    for (j=0; j < m_colors; j++)
      for (xyz_cam[i][j] = k=0; k < 3; k++)
	xyz_cam[i][j] += xyz_rgb[i][k] * m_rgb_cam[k][j] / d65_white[i];

  border_interpolate(5);

  char* buffer = new char[26 * TS *TS];		/* 1664 kB */
  merror(buffer, "ahd_interpolate()");
  rgb  = (ushort(*)[TS][TS][3]) buffer;
  lab  = (short (*)[TS][TS][3])(buffer + 12*TS*TS);
  homo = (char  (*)[TS][TS])   (buffer + 24*TS*TS);

  for (top=2; top < m_height-5; top += TS-6)
    for (left=2; left < m_width-5; left += TS-6) {

/*  Interpolate green horizontally and vertically:		*/
      for (row = top; row < top+TS && row < m_height-2; row++) {
	col = left + (FC(row,left) & 1);
	for (c = FC(row,col); col < left+TS && col < m_width-2; col+=2) {
	  pix = m_image + row*m_width+col;
	  val = ((pix[-1][1] + pix[0][c] + pix[1][1]) * 2
		- pix[-2][c] - pix[2][c]) >> 2;
	  rgb[0][row-top][col-left][1] = ULIM(val,pix[-1][1],pix[1][1]);
	  val = ((pix[-m_width][1] + pix[0][c] + pix[m_width][1]) * 2
		- pix[-2*m_width][c] - pix[2*m_width][c]) >> 2;
	  rgb[1][row-top][col-left][1] = ULIM(val,pix[-m_width][1],pix[m_width][1]);
	}
      }
/*  Interpolate red and blue, and convert to CIELab:		*/
      for (d=0; d < 2; d++)
	for (row=top+1; row < top+TS-1 && row < m_height-3; row++)
	  for (col=left+1; col < left+TS-1 && col < m_width-3; col++) {
	    pix = m_image + row*m_width+col;
	    rix = &rgb[d][row-top][col-left];
	    lix = &lab[d][row-top][col-left];
	    if ((c = 2 - FC(row,col)) == 1) {
	      c = FC(row+1,col);
	      val = pix[0][1] + (( pix[-1][2-c] + pix[1][2-c]
				 - rix[-1][1] - rix[1][1] ) >> 1);
	      rix[0][2-c] = CLIP(val);
	      val = pix[0][1] + (( pix[-m_width][c] + pix[m_width][c]
				 - rix[-TS][1] - rix[TS][1] ) >> 1);
	    } else
	      val = rix[0][1] + (( pix[-m_width-1][c] + pix[-m_width+1][c]
				 + pix[+m_width-1][c] + pix[+m_width+1][c]
				 - rix[-TS-1][1] - rix[-TS+1][1]
				 - rix[+TS-1][1] - rix[+TS+1][1] + 1) >> 2);
	    rix[0][c] = CLIP(val);
	    c = FC(row,col);
	    rix[0][c] = pix[0][c];
	    xyz[0] = xyz[1] = xyz[2] = 0.5;
	    FORCC {
	      xyz[0] += xyz_cam[0][c] * rix[0][c];
	      xyz[1] += xyz_cam[1][c] * rix[0][c];
	      xyz[2] += xyz_cam[2][c] * rix[0][c];
	    }
	    xyz[0] = cbrt[CLIP((int) xyz[0])];
	    xyz[1] = cbrt[CLIP((int) xyz[1])];
	    xyz[2] = cbrt[CLIP((int) xyz[2])];
	    lix[0][0] = 64 * (116 * xyz[1] - 16);
	    lix[0][1] = 64 * 500 * (xyz[0] - xyz[1]);
	    lix[0][2] = 64 * 200 * (xyz[1] - xyz[2]);
	  }
/*  Build homogeneity maps from the CIELab images:		*/
      memset (homo, 0, 2*TS*TS);
      for (row=top+2; row < top+TS-2 && row < m_height-4; row++) {
	tr = row-top;
	for (col=left+2; col < left+TS-2 && col < m_width-4; col++) {
	  tc = col-left;
	  for (d=0; d < 2; d++) {
	    lix = &lab[d][tr][tc];
	    for (i=0; i < 4; i++) {
	       ldiff[d][i] = ABS(lix[0][0]-lix[dir[i]][0]);
	      abdiff[d][i] = SQR(lix[0][1]-lix[dir[i]][1])
			   + SQR(lix[0][2]-lix[dir[i]][2]);
	    }
	  }
	  leps = MIN(MAX(ldiff[0][0],ldiff[0][1]),
		     MAX(ldiff[1][2],ldiff[1][3]));
	  abeps = MIN(MAX(abdiff[0][0],abdiff[0][1]),
		      MAX(abdiff[1][2],abdiff[1][3]));
	  for (d=0; d < 2; d++)
	    for (i=0; i < 4; i++)
	      if (ldiff[d][i] <= leps && abdiff[d][i] <= abeps)
		homo[d][tr][tc]++;
	}
      }
/*  Combine the most homogenous pixels for the final result:	*/
      for (row=top+3; row < top+TS-3 && row < m_height-5; row++) {
	tr = row-top;
	for (col=left+3; col < left+TS-3 && col < m_width-5; col++) {
	  tc = col-left;
	  for (d=0; d < 2; d++)
	    for (hm[d]=0, i=tr-1; i <= tr+1; i++)
	      for (j=tc-1; j <= tc+1; j++)
		hm[d] += homo[d][i][j];
	  if (hm[0] != hm[1])
	    FORC3 m_image[row*m_width+col][c] = rgb[hm[1] > hm[0]][tr][tc][c];
	  else
	    FORC3 m_image[row*m_width+col][c] =
		(rgb[0][tr][tc][c] + rgb[1][tr][tc][c]) >> 1;
	}
      }
    }
  delete [] buffer;
}
#undef TS

void panaraw::tiff_get(unsigned base, unsigned *tag, unsigned *type, unsigned *len, unsigned *save)
{
	*tag  = get2();
	*type = get2();
	*len  = get4();
	*save = ftell(m_ifp) + 4;
	if (*len * ("11124811248488"[*type < 14 ? *type:0]-'0') > 4)
		fseek (m_ifp, get4()+base, SEEK_SET);
}

void panaraw::parse_exif(int base)
{
	unsigned entries, tag, type, len, save;
	double expo;

	entries = get2();
	while (entries--) {
		tiff_get(base, &tag, &type, &len, &save);
		switch (tag) {
		case TAG_EXPOSURE_PROGRAM:
			m_exposure_program = (int)getreal(type);
			break;
		case TAG_EXPOSURETIME:
			m_shutter = getreal(type);
			break;
		case TAG_FNUMBER:
			m_aperture = getreal(type);
			break;
		case TAG_ISO_EQUIVALENT:
			m_iso_speed = get2();
			break;
		case TAG_DATETIME_ORIGINAL:
			fgets(m_date, 32, m_ifp);
			break;
		case TAG_SHUTTERSPEED:
			if ((expo = -getreal(type)) < 128)
				m_shutter = pow(2, expo);
			break;
		case TAG_FLASH:
			m_flash_used = getint(type);
			break;
		case TAG_APERTURE:
			m_aperture = pow(2, getreal(type) / 2);
			break;
		case TAG_MAXAPERTURE:
			m_max_aperture = getreal(type);
			break;
		case TAG_FOCALLENGTH:
			m_focal_len = getreal(type);
			break;
		case TAG_METERING_MODE:
			m_metering_mode = getint(type);
			break;
		case TAG_EXPOSURE_BIAS:
			m_exposure_bias = (int)getreal(type);
			break;
		case TAG_EX_WHITEBALANCE:
			m_white_balance = getint(type);
			break;
		}
		fseek(m_ifp, save, SEEK_SET);
	}
}

void panaraw::linear_table(unsigned len)
{
	if (len > 0x1000)
		len = 0x1000;
	read_shorts(m_curve, len);
	for (int i = len; i < 0x1000; i++)
		m_curve[i] = m_curve[i - 1];
	m_maximum = m_curve[0xfff];
}

int panaraw::parse_tiff_ifd(int base)
{
	unsigned entries, tag, type, len, plen=16, save;
	int ifd, use_cm=0, cfa, i, j, c, ima_len=0;
	uchar cfa_pat[16], cfa_pc[] = { 0,1,2,3 }, tab[256];
	double dblack, cc[4][4], cm[4][3], cam_xyz[4][3];
	double ab[]={ 1,1,1,1 }, asn[] = { 0,0,0,0 }, xyz[] = { 1,1,1 };
	unsigned sony_curve[] = { 0,0,0,0,0,4095 };
	unsigned sony_offset=0, sony_length=0, sony_key=0;
	JHEAD jh;

	if (m_tiff_nifds >= ARRAYSIZE(tiff_ifd))
		return 1;
	ifd = m_tiff_nifds++;
	for (j=0; j < 4; j++)
		for (i=0; i < 4; i++)
			cc[j][i] = i == j;
	entries = get2();
	if (entries > 512) return 1;
	while (entries--) {
		tiff_get(base, &tag, &type, &len, &save);
		switch (tag) {
		case 23:
			if (type == 3) m_iso_speed = get2();
			break;
		case 36:
		case 37:
		case 38:
			m_cam_mul[tag - 0x24] = get2();
			break;
		case 39:
			if (len < 50 || m_cam_mul[0]) break;
			fseek(m_ifp, 12, SEEK_CUR);
			FORC3 m_cam_mul[c] = get2();
			break;
		case 46:
			if (type != 7 || fgetc(m_ifp) != 0xff || fgetc(m_ifp) != 0xd8)
				break;
			m_thumb_offset = ftell(m_ifp) - 2;
//			m_thumb_length = len;
			break;
		case 2:
		case TAG_IMAGEWIDTH:			// 0x100: ImageWidth
			tiff_ifd[ifd].width = getint(type);
			break;
		case 3:
		case TAG_IMAGELENGTH:			// 0x101: ImageHeight
			tiff_ifd[ifd].height = getint(type);
			break;
		case TAG_BITS_SAMPLE:			// 0x102: BitsPerSample
			tiff_ifd[ifd].samples = len & 7;
			tiff_ifd[ifd].bps = get2();
			break;
		case TAG_MAKE:					// 0x10F: Make
			fgets(m_make, 64, m_ifp);
			break;
		case TAG_MODEL:					// 0x110: Model
			fgets(m_model, 64, m_ifp);
			break;
		case TAG_STRIP_OFFSET:			// 0x111: StripOffset
		case TAG_THUMBNAIL_OFFSET:		// 0x201: 
			tiff_ifd[ifd].offset = get4() + base;
			if (!tiff_ifd[ifd].bps) {
				fseek (m_ifp, tiff_ifd[ifd].offset, SEEK_SET);
				if (ljpeg_start(&jh, 1)) {
					tiff_ifd[ifd].comp    = 6;
					tiff_ifd[ifd].width   = jh.wide << ((jh.clrs == 2) ? 1 : 0);
					tiff_ifd[ifd].height  = jh.high;
					tiff_ifd[ifd].bps     = jh.bits;
					tiff_ifd[ifd].samples = jh.clrs;
				}
			}
			break;
		case TAG_ORIENTATION:			// 0x112: Orientation
			m_orientation = getint(type);
			tiff_ifd[ifd].flip = "50132467"[m_orientation & 7]-'0';
			break;
		case TAG_SAMPLE_PIXEL:			// 0x115: SamplesPerPixel
			tiff_ifd[ifd].samples = getint(type) & 7;
			break;
		case TAG_STRIP_BYTE_COUNT:		// 0x117: StripByteCounts
		case TAG_THUMBNAIL_LENGTH:		// 0x202: 
			tiff_ifd[ifd].bytes = get4();
			break;
		case TAG_DATETIME:				// 0x132:
			break;
		case TAG_SUBIFD:				// 0x14A: SubIFDs
			while (len--) {
				i = ftell(m_ifp);
				fseek (m_ifp, get4()+base, SEEK_SET);
				if (parse_tiff_ifd (base)) break;
				fseek (m_ifp, i+4, SEEK_SET);
			}
			break;
		case TAG_EXPOSURETIME:			// 0x829A: ExposureTime
			m_shutter = getreal(type);
			break;
		case TAG_FNUMBER:				// 0x829D: FNumber
			m_aperture = getreal(type);
			break;
		case TAG_EXIF_OFFSET:			// 0x8769: EXIF tag
			fseek(m_ifp, get4() + base, SEEK_SET);
			parse_exif(base);
			break;
		case TAG_CFA_REPEATPATTERN:		// 0x828E: CFA Pattern
			if ((plen = len) > 16) plen = 16;
			fread(cfa_pat, 1, plen, m_ifp);
			for (m_colors = cfa = i = 0; i < plen; i++) {
				m_colors += !(cfa & (1 << cfa_pat[i]));
				cfa |= 1 << cfa_pat[i];
			}
			if (cfa == 070) memcpy(cfa_pc,"\003\004\005", 3);			/* CMY */
			if (cfa == 072) memcpy(cfa_pc,"\005\003\004\001", 4);		/* GMCY */
			goto guess_cfa_pc;
		case 50459:						// Hasselblad tag
			i = m_order;
			j = ftell(m_ifp);
			c = m_tiff_nifds;
			m_order = get2();
			fseek(m_ifp, j + (get2(), get4()), SEEK_SET);
			parse_tiff_ifd(j);
			m_maximum = 0xffff;
			m_tiff_nifds = c;
			m_order = i;
			break;
		case 50710:						// CFAPlaneColor
			if (len > 4) len = 4;
			m_colors = len;
			fread(cfa_pc, 1, m_colors, m_ifp);
guess_cfa_pc:
			FORCC tab[cfa_pc[c]] = c;
//			cdesc[c] = 0;
			for (i = 16; i--; )
				m_filters = m_filters << 2 | tab[cfa_pat[i % plen]];
			break;
		case 50711:						// CFALayout
			if (get2() == 2) {
				m_fuji_width = 1;
				m_filters = 0x49494949;
			}
			break;
		case 291:
		case 50712:						// LinearizationTable
			linear_table(len);
			break;
		case 50714:						// BlackLevel
		case 50715:						// BlackLevelDeltaH
		case 50716:						// BlackLevelDeltaV
			for (dblack = i = 0; i < len; i++)
				dblack += getreal(type);
			m_black += dblack / len + 0.5;
			break;
		case 50717:						// WhiteLevel
			m_maximum = getint(type);
			break;
		case 50718:						// DefaultScale
			m_pixel_aspect  = getreal(type);
			m_pixel_aspect /= getreal(type);
			break;
		case 50721:						// ColorMatrix1
		case 50722:						// ColorMatrix2
			FORCC for (j = 0; j < 3; j++)
				cm[c][j] = getreal(type);
			use_cm = 1;
			break;
		case 50723:						// CameraCalibration1
		case 50724:						// CameraCalibration2
			for (i = 0; i < m_colors; i++)
				FORCC cc[i][c] = getreal(type);
		case 50727:						// AnalogBalance
			FORCC ab[c] = getreal(type);
			break;
		case 50728:						// AsShotNeutral
			FORCC asn[c] = getreal(type);
			break;
		case 50729:						// AsShotWhiteXY
			xyz[0] = getreal(type);
			xyz[1] = getreal(type);
			xyz[2] = 1 - xyz[0] - xyz[1];
			FORC3 xyz[c] /= d65_white[c];
			break;
		case 50752:
			read_shorts(m_cr2_slice, 3);
			break;
		case 50829:						// ActiveArea
			m_top_margin = getint(type);
			m_left_margin = getint(type);
			m_height = getint(type) - m_top_margin;
			m_width = getint(type) - m_left_margin;
			break;
		}
		fseek (m_ifp, save, SEEK_SET);
	}
	for (i = 0; i < m_colors; i++)
		FORCC cc[i][c] *= ab[i];
	if (use_cm) {
		FORCC for (i=0; i < 3; i++)
		for (cam_xyz[c][i]=j=0; j < m_colors; j++)
			cam_xyz[c][i] += cc[c][j] * cm[j][i] * xyz[i];
		cam_xyz_coeff(cam_xyz);
	}
	if (asn[0]) {
		m_cam_mul[3] = 0;
		FORCC m_cam_mul[c] = 1 / asn[c];
	}
	if (!use_cm)
		FORCC m_pre_mul[c] /= cc[c][c];
	return 0;
}

void panaraw::parse_tiff(int base)
{
	int doff, max_samp=0, raw=-1, thm=-1, i;
	JHEAD jh;

	fseek(m_ifp, base, SEEK_SET);
	m_order = get2();
	if (m_order != 0x4949 && m_order != 0x4d4d) return;

	get2();
	memset(tiff_ifd, 0, sizeof tiff_ifd);
	m_tiff_nifds = 0;
	while ((doff = get4())) {
		fseek(m_ifp, doff + base, SEEK_SET);
		if (parse_tiff_ifd(base)) break;
	}
	m_thumb_misc = 16;
	if (m_thumb_offset) {
		fseek(m_ifp, m_thumb_offset, SEEK_SET);
		if (ljpeg_start(&jh, 1)) {
			m_thumb_misc   = jh.bits;
			m_thumb_width  = jh.wide;
			m_thumb_height = jh.high;
		}
	}
	for (i = 0; i < m_tiff_nifds; i++) {
		if (max_samp < tiff_ifd[i].samples)
			max_samp = tiff_ifd[i].samples;
		if (max_samp > 3)
			max_samp = 3;
		if ((tiff_ifd[i].comp != 6 || tiff_ifd[i].samples != 3) &&
			tiff_ifd[i].width*tiff_ifd[i].height > m_raw_width*m_raw_height) {
			m_raw_width     = tiff_ifd[i].width;
			m_raw_height    = tiff_ifd[i].height;
			m_tiff_bps      = tiff_ifd[i].bps;
			m_tiff_compress = tiff_ifd[i].comp;
			m_data_offset   = tiff_ifd[i].offset;
			m_tiff_flip     = tiff_ifd[i].flip;
			m_tiff_samples  = tiff_ifd[i].samples;
			raw = i;
		}
	}
	m_fuji_width *= (m_raw_width + 1) / 2;
	if (tiff_ifd[0].flip)
		m_tiff_flip = tiff_ifd[0].flip;
	if (raw >= 0 && !m_load_raw)
		switch (m_tiff_compress) {
		case 0:
		case 1:
			switch (m_tiff_bps) {
			case  8: m_load_raw = &stp_eight_bit_load_raw;	break;
			case 12: m_load_raw = &stp_packed_12_load_raw;	break;
			case 14:
			case 16: m_load_raw = &stp_unpacked_load_raw;	break;
			}
			if (tiff_ifd[raw].bytes * 5 == m_raw_width * m_raw_height * 8)
				m_load_raw = &stp_olympus_e300_load_raw;
			break;
		case 6:
		case 7:
		case 99: 	m_load_raw = &stp_lossless_jpeg_load_raw;		break;
		case 32769:	m_load_raw = &stp_nikon_load_raw;				break;
 		case 32773:	m_load_raw = &stp_packed_12_load_raw;			break;
		case 32867: break;
		default: m_is_raw = 0;
		}

	if (m_tiff_samples == 3)
		if (tiff_ifd[raw].bytes && m_tiff_bps != 14 && m_tiff_bps != 2048)
			m_is_raw = 0;
	if (m_tiff_bps == 8 && m_tiff_compress == 1 && tiff_ifd[raw].phint == 1)
			m_is_raw = 0;
	for (i = 0; i < m_tiff_nifds; i++)
		if (i != raw && tiff_ifd[i].samples == max_samp &&
			tiff_ifd[i].width * tiff_ifd[i].height / SQR(tiff_ifd[i].bps+1) > m_thumb_width * m_thumb_height / SQR(m_thumb_misc+1)) {
			m_thumb_width  = tiff_ifd[i].width;
			m_thumb_height = tiff_ifd[i].height;
			m_thumb_offset = tiff_ifd[i].offset;
//			m_thumb_length = tiff_ifd[i].bytes;
			m_thumb_misc   = tiff_ifd[i].bps;
			thm = i;
		}
	if (thm >= 0) {
		m_thumb_misc |= tiff_ifd[thm].samples << 5;
	}
}

/*
   Thanks to Adobe for providing these excellent CAM -> XYZ matrices!
 */
void panaraw::adobe_coeff(char *maker, char *model)
{
  static const struct {
    const char *prefix;
    short black, trans[12];
  } table[] = {
    { "EPSON R-D1", 0,
	{ 6827,-1878,-732,-8429,16012,2564,-704,592,7145 } },
    { "Panasonic DMC-FZ8", 0,
	{ 8986,-2755,-802,-6341,13575,3077,-1476,2144,6379 } },
    { "Panasonic DMC-FZ18", 0,
	{ 9932,-3060,-935,-5809,13331,2753,-1267,2155,5575 } },
    { "Panasonic DMC-FZ30", 0,
	{ 10976,-4029,-1141,-7918,15491,2600,-1670,2071,8246 } },
    { "Panasonic DMC-FZ50", 0,	/* aka "LEICA V-LUX1" */
	{ 7906,-2709,-594,-6231,13351,3220,-1922,2631,6537 } },
    { "Panasonic DMC-L10", 0,
	{ 8025,-1942,-1050,-7920,15904,2100,-2456,3005,7039 } },
    { "Panasonic DMC-L1", 0,	/* aka "LEICA DIGILUX 3" */
	{ 8054,-1885,-1025,-8349,16367,2040,-2805,3542,7629 } },
    { "Panasonic DMC-LC1", 0,	/* aka "LEICA DIGILUX 2" */
	{ 11340,-4069,-1275,-7555,15266,2448,-2960,3426,7685 } },
    { "Panasonic DMC-LX1", 0,	/* aka "LEICA D-LUX2" */
	{ 10704,-4187,-1230,-8314,15952,2501,-920,945,8927 } },
    { "Panasonic DMC-LX2", 0,	/* aka "LEICA D-LUX3" */
	{ 8048,-2810,-623,-6450,13519,3272,-1700,2146,7049 } },
  };

	double cam_xyz[4][3];
	char name[130];
	int i, j;

	wsprintfA(name, "%s %s", maker, model);
	for (i = 0; i < ARRAYSIZE(table); i++)
		if (!StrCmpNA(name, table[i].prefix, lstrlenA(table[i].prefix))) {
			if (table[i].black)
				m_black = table[i].black;
			for (j=0; j < 12; j++)
				cam_xyz[0][j] = table[i].trans[j] / 10000.0;
			cam_xyz_coeff (cam_xyz);
			break;
		}
}

/*
   Identify which camera created this file, and set global variables
   accordingly.
 */
static const char *corp[] = {
	"Canon", "NIKON", "EPSON", "KODAK", "Kodak", "OLYMPUS", "PENTAX",
	"MINOLTA", "Minolta", "Konica", "CASIO", "Sinar", "Phase One",
	"SAMSUNG", "Mamiya"
};

void panaraw::identify()
{
	// 各種パラメータ初期化
	m_tiff_flip = m_flip = m_filters = -1;	/* 0 is valid, so -1 is unknown */
	m_raw_height = m_raw_width = m_fuji_width = 0;
	m_maximum = m_height = m_width = m_top_margin = m_left_margin = 0;
	m_make[0] = m_model[0] = m_date[0] = 0;
	m_cr2_slice[0] = 0;
	m_iso_speed = m_shutter = m_aperture = m_max_aperture = m_focal_len = m_exposure_bias = 0;
	m_flash_used = m_exposure_program = m_metering_mode = m_white_balance = 0;
	m_orientation = 1;
	memset(m_white, 0, sizeof m_white);
	m_thumb_offset = m_thumb_width = m_thumb_height = 0;
	m_load_raw = NULL;
	m_data_offset = m_tiff_bps = m_tiff_compress = 0;
	m_zero_after_ff = 0;
	m_tiff_samples = m_black = 0;
	m_mix_green = m_data_error = m_zero_is_bad = 0;
	m_is_raw = m_raw_color = m_use_gamma = 1;
	m_pixel_aspect = 1;

	unsigned i, c;
	for (i = 0; i < 4; i++) {
		m_cam_mul[i] = i == 1;
		m_pre_mul[i] = i < 3;
		FORC3 m_cmatrix[c][i] = 0;
		FORC3 m_rgb_cam[c][i] = c == i;
	}
	m_colors = 3;
	m_tiff_bps = 12;
	for (i = 0; i < 0x4000; i++)
		m_curve[i] = i;

	// II or MM で始まらないデータは処理しない
	m_order = get2();
	if (m_order != 0x4949 && m_order != 0x4d4d) {
		m_is_raw = 0;
		goto notraw;
	}

	unsigned hlen = get4();
	fseek(m_ifp, 0, SEEK_SET);

	char head[32];
	fread(head, 1, 32, m_ifp);
	fseek(m_ifp, 0, SEEK_END);
	unsigned fsize = ftell(m_ifp);

	// tiff tag チェック
	parse_tiff(0);

	// Simplify maker name
	for (i = 0; i < ARRAYSIZE(corp); i++) {
		if (StrStrA(m_make, corp[i])) {				/* Simplify company names */
			lstrcpyA(m_make, corp[i]);
			break;
		}
	}

	char* cp = m_make + lstrlenA(m_make);			/* Remove trailing spaces */
	while (*--cp == ' ')
		*cp = 0;
	cp = m_model + lstrlenA(m_model);
	while (*--cp == ' ')
		*cp = 0;
	i = lstrlenA(m_make);							/* Remove make from model */
	if (!StrCmpNIA(m_model, m_make, i) && m_model[i++] == ' ')
		memmove(m_model, m_model+i, 64-i);
	m_make[63] = m_model[63] = 0;
	if (!m_is_raw) {
		m_is_raw = 0;
		goto notraw;
	}

	if (!m_maximum) m_maximum = (1 << m_tiff_bps) - 1;
	if (!m_height) m_height = m_raw_height;
	if (!m_width)  m_width  = m_raw_width;
	if (m_fuji_width) {
		m_width = m_height + m_fuji_width;
		m_height = m_width - 1;
		m_pixel_aspect = 1;
	}

	if (!StrCmpA(m_make, "LEICA") || !StrCmpA(m_make, "Panasonic")) {
		// ---------------------------------
		// Panasonic / Leica
		// ---------------------------------
		m_maximum = 0xfff0;
		if ((fsize - m_data_offset) / (m_width * 8 / 7) == m_height)
			m_load_raw = &stp_panasonic_load_raw;
		if (!m_load_raw)
			m_load_raw = &stp_unpacked_load_raw;

		// イメージ幅の値でデータタイプを決定
		switch (m_width) {
		case 2568:
			adobe_coeff("Panasonic","DMC-LC1");  break;
		case 3130:
			m_left_margin = -14;
		case 3170:
			m_left_margin += 18;
			m_width = 3096;
			if (m_height > 2326) {
				m_height = 2326;
				m_top_margin = 13;
				m_filters = 0x49494949;
			}
			m_maximum = 0xf7f0;
			m_zero_is_bad = 1;
			adobe_coeff("Panasonic","DMC-FZ8");  break;
		case 3177:
			m_width -= 10;
			m_filters = 0x49494949;
			m_maximum = 0xf7fc;
			m_zero_is_bad = 1;
			adobe_coeff("Panasonic","DMC-L1");  break;
		case 3304:
			m_width -= 16;
			m_maximum = 0xf94c;
			m_zero_is_bad = 1;
			adobe_coeff("Panasonic","DMC-FZ30");  break;
		case 3330:
			m_width = 3291;
			m_left_margin = 9;
			m_maximum = 0xf7f0;
			goto fz18;
		case 3370:
			m_width = 3288;
			m_left_margin = 15;
fz18:
			if (m_height > 2480)
				m_height = 2480 - (m_top_margin = 10);
			m_filters = 0x49494949;
			m_zero_is_bad = 1;
			break;
		case 3690:
			m_height += 36;
			m_left_margin = -14;
			m_filters = 0x49494949;
			m_maximum = 0xf7f0;
		case 3770:
			m_width = 3672;
			if ((m_height -= 39) == 2760)
				m_top_margin = 15;
			m_left_margin += 17;
			m_zero_is_bad = 1;
			adobe_coeff("Panasonic","DMC-FZ50");  break;
		case 3710:
			m_width = 3682;
			m_filters = 0x49494949;
			break;
		case 3880:
			m_width -= 22;
			m_left_margin = 6;
			m_maximum = 0xf7f0;
			m_zero_is_bad = 1;
			adobe_coeff("Panasonic","DMC-LX1");  break;
		case 4290:
			m_height += 38;
			m_left_margin = -14;
			m_filters = 0x49494949;
		case 4330:
			m_width = 4248;
			if ((m_height -= 39) == 2400)
				m_top_margin = 15;
			m_left_margin += 17;
			adobe_coeff("Panasonic","DMC-LX2");  break;
		}
	}
	else if (!StrCmpA(m_make, "Hasselblad")) {
		// ---------------------------------
		// Hasselblad
		// ---------------------------------
		if (m_load_raw == &stp_lossless_jpeg_load_raw)
			m_load_raw = &stp_hasselblad_load_raw;
		if (m_raw_width == 7262) {
			m_height = 5444;
			m_width = 7248;
			m_top_margin = 4;
			m_left_margin = 7;
			m_filters = 0x61616161;
		}
	}
	else
		goto notraw;


	if (m_filters == UINT_MAX)
		m_filters = 0x94949494;

	if (m_raw_color)
		adobe_coeff(m_make, m_model);

	if (m_thumb_offset && !m_thumb_height) {
		fseek(m_ifp, m_thumb_offset, SEEK_SET);
		JHEAD jh;
		if (ljpeg_start(&jh, 1)) {
			m_thumb_width  = jh.wide;
			m_thumb_height = jh.high;
		}
	}

dng_skip:
	if (!m_load_raw || m_height < 22) m_is_raw = 0;
	if (!m_raw_height) m_raw_height = m_height;
	if (!m_raw_width ) m_raw_width  = m_width;
	if (m_filters && m_colors == 3)
		for (i=0; i < 32; i+=4) {
			if ((m_filters >> i & 15) == 9)
				m_filters |= 2 << i;
			if ((m_filters >> i & 15) == 6)
				m_filters |= 8 << i;
		}

notraw:
	if (m_flip == -1) m_flip = m_tiff_flip;
	if (m_flip == -1) m_flip = 0;
}

void panaraw::convert_to_rgb()
{
	int row, col, c, i, j, k;
	ushort *img;
	float out[3], out_cam[3][4];
	double num, inverse[3][3];
	static const double xyzd50_srgb[3][3] =
		{ { 0.436083, 0.385083, 0.143055 },
		  { 0.222507, 0.716888, 0.060608 },
		  { 0.013930, 0.097097, 0.714022 } };
	static const double rgb_rgb[3][3] =
		{ { 1,0,0 }, { 0,1,0 }, { 0,0,1 } };
	static const double adobe_rgb[3][3] =
		{ { 0.715146, 0.284856, 0.000000 },
		  { 0.000000, 1.000000, 0.000000 },
		  { 0.000000, 0.041166, 0.958839 } };
	static const double wide_rgb[3][3] =
		{ { 0.593087, 0.404710, 0.002206 },
		  { 0.095413, 0.843149, 0.061439 },
		  { 0.011621, 0.069091, 0.919288 } };
	static const double prophoto_rgb[3][3] =
		{ { 0.529317, 0.330092, 0.140588 },
		  { 0.098368, 0.873465, 0.028169 },
		  { 0.016879, 0.117663, 0.865457 } };
	static const double (*out_rgb[])[3] = { rgb_rgb, adobe_rgb, wide_rgb, prophoto_rgb, xyz_rgb };
	static const char *name[] = { "sRGB", "Adobe RGB (1998)", "WideGamut D65", "ProPhoto D65", "XYZ" };
	static const unsigned phead[] =
		{ 1024, 0, 0x2100000, 0x6d6e7472, 0x52474220, 0x58595a20, 0, 0, 0,
		  0x61637370, 0, 0, 0x6e6f6e65, 0, 0, 0, 0, 0xf6d6, 0x10000, 0xd32d };
	unsigned pbody[] =
		{	10, 0x63707274, 0, 36,	/* cprt */
			0x64657363, 0, 40,		/* desc */
			0x77747074, 0, 20,		/* wtpt */
			0x626b7074, 0, 20,		/* bkpt */
			0x72545243, 0, 14,		/* rTRC */
			0x67545243, 0, 14,		/* gTRC */
			0x62545243, 0, 14,		/* bTRC */
			0x7258595a, 0, 20,		/* rXYZ */
			0x6758595a, 0, 20,		/* gXYZ */
			0x6258595a, 0, 20 };	/* bXYZ */
	static const unsigned pwhite[] = { 0xf351, 0x10000, 0x116cc };
	unsigned pcurve[] = { 0x63757276, 0, 1, 0x1000000 };

	memcpy(out_cam, m_rgb_cam, sizeof out_cam);
	m_raw_color |= m_colors == 1 || m_output_colorspace < 1 || m_output_colorspace > 5;
	if (!m_raw_color) {
		m_oprof = new unsigned[phead[0]];
		merror(m_oprof, "convert_to_rgb()");
		memcpy(m_oprof, phead, sizeof phead);
		if (m_output_colorspace == 5) m_oprof[4] = m_oprof[5];
		m_oprof[0] = 132 + 12 * pbody[0];
		for (i=0; i < pbody[0]; i++) {
			m_oprof[m_oprof[0]/4] = i ? (i > 1 ? 0x58595a20 : 0x64657363) : 0x74657874;
			pbody[i*3+2] = m_oprof[0];
			m_oprof[0] += (pbody[i*3+3] + 3) & -4;
		}
		memcpy(m_oprof+32, pbody, sizeof pbody);
		m_oprof[pbody[5]/4+2] = lstrlenA(name[m_output_colorspace-1]) + 1;
		memcpy((char*)m_oprof + pbody[8] + 8, pwhite, sizeof pwhite);
#ifdef SRGB_GAMMA
		pcurve[3] = 0x2330000;
#else
		pcurve[3] = 0x1f00000;
#endif
		for (i=4; i < 7; i++)
			memcpy((char*)m_oprof + pbody[i * 3 + 2], pcurve, sizeof pcurve);
		pseudoinverse((double (*)[3]) out_rgb[m_output_colorspace-1], inverse, 3);
		for (i=0; i < 3; i++)
			for (j=0; j < 3; j++) {
				for (num = k=0; k < 3; k++)
					num += xyzd50_srgb[i][k] * inverse[j][k];
				m_oprof[pbody[j * 3 + 23] / 4 + i + 2] = num * 0x10000 + 0.5;
			}
		for (i = 0; i < phead[0] / 4; i++)
			m_oprof[i] = _htonl(m_oprof[i]);
		lstrcpyA((char*)m_oprof + pbody[2] + 8, "auto-generated by dcraw");
		lstrcpyA((char*)m_oprof + pbody[5] + 12, name[m_output_colorspace-1]);
		for (i=0; i < 3; i++)
			for (j=0; j < m_colors; j++)
				for (out_cam[i][j] = k=0; k < 3; k++)
					out_cam[i][j] += out_rgb[m_output_colorspace-1][i][k] * m_rgb_cam[k][j];
	}
//@	if (m_verbose) _ftprintf(stderr, m_raw_color ? _T("Building histograms...\n") : _T("Converting to %s colorspace...\n"), name[m_output_colorspace-1]);

	memset(m_histogram, 0, sizeof(int) * 0x2000 * 4);
	for (img=m_image[0], row=0; row < m_height; row++)
		for (col=0; col < m_width; col++, img+=4) {
			if (!m_raw_color) {
				out[0] = out[1] = out[2] = 0;
				FORCC {
					out[0] += out_cam[0][c] * img[c];
					out[1] += out_cam[1][c] * img[c];
					out[2] += out_cam[2][c] * img[c];
				}
				FORC3 img[c] = CLIP((int) out[c]);
			}
			FORCC m_histogram[img[c] >> 3][c]++;
		}
	if (m_colors == 4 && m_output_colorspace) m_colors = 3;
}

void panaraw::fuji_rotate()
{
	int i, row, col;
	double step;
	float r, c, fr, fc;
	unsigned ur, uc;
	ushort wide, high, (*pix)[4];

	if (!m_fuji_width) return;
//@	if (m_verbose) _ftprintf(stderr, _T("Rotating image 45 degrees...\n"));
	m_fuji_width = (m_fuji_width - 1 + m_shrink) >> m_shrink;
	step = sqrt(0.5);
	wide = m_fuji_width / step;
	high = (m_height - m_fuji_width) / step;
	ushort (*img)[4] = new ushort[wide * high][4];
	merror (img, "fuji_rotate()");

	for (row=0; row < high; row++)
		for (col=0; col < wide; col++) {
			ur = r = m_fuji_width + (row-col)*step;
			uc = c = (row+col)*step;
			if (ur > m_height-2 || uc > m_width-2) continue;
			fr = r - ur;
			fc = c - uc;
			pix = m_image + ur*m_width + uc;
			for (i=0; i < m_colors; i++)
				img[row * wide + col][i] =
								(pix[    0][i]*(1-fc) + pix[      1][i]*fc) * (1-fr) +
								(pix[m_width][i]*(1-fc) + pix[m_width+1][i]*fc) * fr;
		}
	delete [] m_image;
	m_width  = wide;
	m_height = high;
	m_image  = img;
	m_fuji_width = 0;
}

void panaraw::stretch()
{
	if (m_pixel_aspect == 1) return;

//@	if (m_verbose) fprintf (stderr,_("Stretching the image...\n"));

	ushort newdim, (*img)[4], *pix0, *pix1;
	int row, col, c;
	double rc, frac;

	if (m_pixel_aspect < 1) {
		newdim = m_height / m_pixel_aspect + 0.5;
		img = new ushort[m_width * newdim][4];
		merror(img, "stretch()");
		for (rc = row = 0; row < newdim; row++, rc += m_pixel_aspect) {
			frac = rc - (c = rc);
			pix0 = pix1 = m_image[c * m_width];
			if (c + 1 < m_height)
				pix1 += m_width * 4;
			for (col=0; col < m_width; col++, pix0 += 4, pix1 += 4)
				FORCC img[row * m_width + col][c] = pix0[c] * (1 - frac) + pix1[c] * frac + 0.5;
		}
		m_height = newdim;
	}
	else {
		newdim = m_width * m_pixel_aspect + 0.5;
		img = new ushort[m_height * newdim][4];
		merror(img, "stretch()");
		for (rc = col = 0; col < newdim; col++, rc += 1 / m_pixel_aspect) {
			frac = rc - (c = rc);
			pix0 = pix1 = m_image[c];
			if (c + 1 < m_width)
				pix1 += 4;
			for (row = 0; row < m_height; row++, pix0 += m_width * 4, pix1 += m_width * 4)
				FORCC img[row * newdim + col][c] = pix0[c] * (1 - frac) + pix1[c] * frac + 0.5;
		}
		m_width = newdim;
	}
	delete [] m_image;
	m_image = img;
}

int panaraw::flip_index(int row, int col)
{
	if (m_flip & 4) SWAP(row, col);
	if (m_flip & 2) row = m_iheight - 1 - row;
	if (m_flip & 1) col = m_iwidth  - 1 - col;
	return row * m_iwidth + col;
}

void panaraw::gamma_lut(uchar* lut)
{
	int perc, c, val, total, i;
	float white=0, r;

	perc = m_width * m_height * 0.01;		/* 99th percentile white level */
	if (m_fuji_width) perc /= 2;
	if (m_no_auto_bright) perc = -1;
	FORCC {
		for (val = 0x2000, total=0; --val > 32; )
			if ((total += m_histogram[val][c]) > perc) break;
		if (white < val) white = val;
	}
	white *= 8 / m_bright;
	for (i=0; i < 0x10000; i++) {
		r = i / white;
		val = 256 * ( !m_use_gamma ? r :
#ifdef SRGB_GAMMA
			r <= 0.00304 ? r*12.92 : powf(r, 2.5 / 6) * 1.055 - 0.055 );
#else
			r <= 0.018 ? r*4.5 : powf(r, 0.45) * 1.099 - 0.099 );
#endif
		if (val > 255) val = 255;
		lut[i] = val;
	}
}

#ifndef DIB_HEADER_MARKER
#define DIB_HEADER_MARKER   ((ushort)('M'<< 8)|'B')
#endif
#ifndef WIDTHBYTES
#define WIDTHBYTES(bits)    (((bits)+31)/32*4)
#endif

#ifdef WIN32_CONSOLE_EXECUTE
void panaraw::write_dib(FILE *ofp)
{
	int row, col, soff, rstep, cstep;
	BITMAPFILEHEADER* pf;
	BITMAPINFOHEADER* pi;
	uchar* dib;
	uchar* pixel;
	ulong total_size;

	m_iheight = m_height;
	m_iwidth  = m_width;
	if (m_flip & 4)
		SWAP(m_height, m_width);

	// 出力バッファ確保
	total_size = WIDTHBYTES(m_width * 24) * m_height + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	dib = new uchar[total_size];
	merror(dib, "write_dib()");

	memset(dib, 0, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));

	// BITMAPFILEHEADER 初期化
	pf = (BITMAPFILEHEADER*)dib;
	pf->bfType = DIB_HEADER_MARKER;
	pf->bfSize = total_size;
	pf->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// BITMAPINFOHEADER 初期化
	pi = (BITMAPINFOHEADER*)((uchar*)dib + sizeof(BITMAPFILEHEADER));
	pi->biSize = sizeof(BITMAPINFOHEADER);
	pi->biWidth = m_width;
	pi->biHeight = m_height;
	pi->biPlanes = 1;
	pi->biBitCount = 24;
	pi->biCompression = BI_RGB;
	pi->biSizeImage = WIDTHBYTES(m_width * 24) * m_height;

	// ガンマLUT 初期化
	uchar* lut = new uchar[0x10000];
	merror(lut, "write_dib()");
	gamma_lut(lut);

	soff  = flip_index(0, 0);
	cstep = flip_index(0, 1) - soff;
	rstep = flip_index(1, 0) - flip_index(0, m_width);
	for (row = 0; row < m_height; row++, soff += rstep) {
		// ピクセルデータ出力先位置
		pixel = dib + pf->bfOffBits + WIDTHBYTES(m_width * 24) * (m_height - row - 1);
		for (col = 0; col < m_width; col++, soff += cstep) {
			*pixel++ = lut[m_image[soff][2]];		// B
			*pixel++ = lut[m_image[soff][1]];		// G
			*pixel++ = lut[m_image[soff][0]];		// R
		}
	}
	fwrite(dib, 1, total_size, ofp);

	delete [] lut;
	delete [] dib;
}
#endif		// WIN32_CONSOLE_EXECUTE

void panaraw::write_dib_stream(IStream* pstrm)
{
	m_iheight = m_height;
	m_iwidth  = m_width;
	if (m_flip & 4)
		SWAP(m_height, m_width);

	// ポインタを先頭へ
	LARGE_INTEGER li;
	li.QuadPart = 0;
	pstrm->Seek(li, STREAM_SEEK_SET, NULL);

	ulong written;
	ulong widthbytes = WIDTHBYTES(m_width * 24);
	ulong total_size = widthbytes * m_height + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	// ストリームサイズをビットマップデータサイズに拡大
	ULARGE_INTEGER uli;
	uli.QuadPart = total_size;
	pstrm->SetSize(uli);

	// BITMAPFILEHEADER 初期化
	BITMAPFILEHEADER bfh;
	memset(&bfh, 0, sizeof(bfh));
	bfh.bfType = DIB_HEADER_MARKER;
	bfh.bfSize = total_size;
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	pstrm->Write(&bfh, sizeof(BITMAPFILEHEADER), &written);

	// BITMAPINFOHEADER 初期化
	BITMAPINFOHEADER bih;
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = m_width;
	bih.biHeight = m_height;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;
	bih.biSizeImage = WIDTHBYTES(m_width * 24) * m_height;
	pstrm->Write(&bih, sizeof(BITMAPINFOHEADER), &written);

	// 出力バッファ確保
	uchar* baseptr = new uchar[widthbytes * m_height];
	merror(baseptr, "write_dib()");

	// ガンマLUT 初期化
	uchar* lut = new uchar[0x10000];
	merror(lut, "write_dib()");
	gamma_lut(lut);

	int soff  = flip_index(0, 0);
	int cstep = flip_index(0, 1) - soff;
	int rstep = flip_index(1, 0) - flip_index(0, m_width);
	for (int row = 0; row < m_height; row++, soff += rstep) {
		uchar* pixel = baseptr + widthbytes * (m_height - row - 1);
		for (int col = 0; col < m_width; col++, soff += cstep) {
			*pixel++ = lut[m_image[soff][2]];		// B
			*pixel++ = lut[m_image[soff][1]];		// G
			*pixel++ = lut[m_image[soff][0]];		// R
		}
	}
	pstrm->Write(baseptr, widthbytes * m_height, &written);

	delete [] lut;
	delete [] baseptr;

	// ポインタを先頭へ戻す
	li.QuadPart = 0;
	pstrm->Seek(li, STREAM_SEEK_SET, NULL);
}

int panaraw::develop(LPCTSTR pszName, IStream* pstrm)
{
	int status = 1;
	m_ifname = pszName;

	if (!m_use_camera_matrix)
		m_use_camera_matrix = m_use_camera_wb;

	m_image = NULL;
	m_oprof = NULL;
	if (!(m_ifp = _tfopen(m_ifname, _T("rb")))) {
#ifdef WIN32_CONSOLE_EXECUTE
		_tperror(m_ifname);
#endif
		return 0;
	}
	identify();
	if (!m_is_raw) {
		status = 0;
		goto cleanup;
	}
	switch ((m_flip + 3600) % 360) {
	case 270:  m_flip = 5;  break;
	case 180:  m_flip = 3;  break;
	case  90:  m_flip = 6;
	}
	m_shrink = m_filters && (m_half_size || m_aber[0] != 1 || m_aber[2] != 1);
	if (m_shrink && m_half_size)
		m_shrink += (m_half_size - 1);
	m_iheight = ((m_height - 1) >> m_shrink) + 1;
	m_iwidth  = ((m_width - 1) >> m_shrink) + 1;
	if (m_use_camera_matrix && m_cmatrix[0][0] > 0.25) {
		memcpy(m_rgb_cam, m_cmatrix, sizeof m_cmatrix);
		m_raw_color = 0;
	}

	m_image = new ushort[m_iheight * m_iwidth][4];
	merror(m_image, "develop()");
//@	if (m_verbose) _ftprintf(stderr, _T("Loading %s %s image from %s ...\n"), m_make, m_model, m_ifname);
	fseek(m_ifp, m_data_offset, SEEK_SET);

	m_load_raw(this);
	if (m_zero_is_bad) remove_zeroes();
	scale_colors();
	pre_interpolate();
	if (m_filters) {
		ahd_interpolate();
	}

	int i;
	if (m_mix_green)
		for (m_colors = 3, i = 0; i < m_height * m_width; i++)
			m_image[i][1] = (m_image[i][1] + m_image[i][3]) >> 1;

	fuji_rotate();
	convert_to_rgb();
	stretch();

#ifdef WIN32_CONSOLE_EXECUTE
	TCHAR ofname[MAX_PATH * 2];
	lstrcpy(ofname, m_ifname);
	PathRenameExtension(ofname, _T(".bmp"));
	FILE* ofp = _tfopen(ofname, _T("wb"));
	if (!ofp) {
		status = 0;
		_tperror(ofname);
		goto cleanup;
	}

//@	if (m_verbose) _ftprintf(stderr, _T("Writing data to %s ...\n"), ofname);
	write_dib(ofp);
#else
	if (pstrm) {
		write_dib_stream(pstrm);
	}
#endif

cleanup:
	fclose(m_ifp);
	m_ifp = NULL;

	if (m_oprof) delete [] m_oprof;
	if (m_image) delete [] m_image;

	return status;
}

////////////////////////////////////////////////////////////////////////
//

BOOL LoadPanasonicRaw(LPCTSTR fname, IStream* pstrm, BOOL bLargeImage, EXIFINFOSTRUCT* ei)
{
	panaraw raw;
	raw.UseCameraWhiteBalance(1);
	raw.SetHalfSizeImage(bLargeImage ? 1 : 3);
	BOOL ok = raw.develop(fname, pstrm);
	if (ok && ei) {
		MultiByteToWideChar(CP_ACP, 0, raw.m_model, -1, ei->ModelName, ARRAYSIZE(ei->ModelName));	// モデル名
		_str2datetime(raw.m_date, &ei->DateTime);					// 撮影日時
		ei->ImageWidth = raw.m_raw_width;							// イメージ幅
		ei->ImageHeight = raw.m_raw_height;							// イメージ高さ
		ei->ExposureProgram = raw.m_exposure_program;				// 露出制御モード
		ei->FocalLength = raw.m_focal_len;							// 焦点距離
		ei->FocalLength35 = 0;										// 焦点距離(35mm換算)
		ei->ExposureTime = raw.m_shutter;							// 露出時間
		ei->ExposureBias = raw.m_exposure_bias;						// 露出補正
		ei->Aperture = raw.m_aperture;								// 絞り
		ei->Aperture = raw.m_max_aperture;							// レンズ開放F値
		ei->ISOEquiv = raw.m_iso_speed;								// ISO感度 (0 for Auto)
		ei->FlashFired = raw.m_flash_used;							// フラッシュの使用
		ei->Orientation = 1;										// 画像の向き
		ei->MeteringMode = raw.m_metering_mode;						// 測光モード(EXIF)
		ei->WhiteBalance = raw.m_white_balance;						// ホワイトバランス(EXIF)
		ei->MaxAperture = 0;										// 最大絞り(EXIF)
		MultiByteToWideChar(CP_ACP, 0, raw.m_make, -1, ei->MakerName, ARRAYSIZE(ei->MakerName));	// メーカー名
		ei->Lens[0] = 0;											// レンズ名称(MAKER_NOTE)
		ei->WhiteBalanceText[0] = 0;								// ホワイトバランス文字列(MAKER_NOTE)

		// Infotip
		CString str;
		TCHAR wsz[256];
		if (ei->pszTipText && ei->cchTipText > 0) {
			*ei->pszTipText = 0;
			str.Format(_T("モデル名: %s\n"), ei->ModelName);
			str.AppendFormat(_T("大きさ: %d x %d\n"), ei->ImageWidth, ei->ImageHeight);

			GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &ei->DateTime, NULL, wsz, ARRAYSIZE(wsz));
			str.AppendFormat(_T("撮影日時: %s "), wsz);
			GetTimeFormat(LOCALE_USER_DEFAULT, 0, &ei->DateTime, NULL, wsz, ARRAYSIZE(wsz));
			str += wsz;

			// format = "撮影情報: P 1/250 F4.6 ±0EV ISO200"
			str += _T("\n撮影情報: ");
			switch (ei->ExposureProgram) {
			case 1: str += _T("M "); break;
			case 2: str += _T("P "); break;
			case 3: str += _T("A "); break;
			case 4: str += _T("S "); break;
			case 5: str += _T("Cr "); break;
			case 6: str += _T("Ac "); break;
			case 7: str += _T("Po "); break;
			case 8: str += _T("La "); break;
			}
			// シャッター速度
			_format_shutter(wsz, ei->ExposureTime);
			_append_sec(wsz);
			str.AppendFormat(_T("%s "), wsz);
			// F値
			_format_Fnumber(wsz, ei->Aperture);
			str.AppendFormat(_T("%s "), wsz);
			// 露出補正
			if (ei->ExposureBias != 0.0F) {
				_format_ev_comp(wsz, ei->ExposureBias);
				str.AppendFormat(_T("%s "), wsz);
			}
			// iso
			str += _T("ISO");
			if (ei->ISOEquiv != 0)
				_format_int(wsz, ei->ISOEquiv);
			else
				lstrcpy(wsz, _T("オート"));
			str += wsz;
			lstrcpyn(ei->pszTipText, str, ei->cchTipText);
		}

		// Lavel
		if (ei->pszLabelText && ei->cchLabelText > 0) {
			*ei->pszLabelText = 0;
			str.Empty();
			// シャッター速度
			if (ei->ExposureTime < 0.3F)
				str.AppendFormat(_T("%3d "), (int)(0.5F + 1.0F / ei->ExposureTime));
			else {
				float f = ei->ExposureTime + 0.05F;
				str.AppendFormat(_T("%d\"%d "), (int)f, (int)floorf(f * 10) % 10);
			}
			// F値
			_format_Fnumber(wsz, ei->Aperture);
			str.AppendFormat(_T("%s "), wsz);
			// 露出補正
			if (ei->ExposureBias != 0.0F) {
				_format_float_comp(wsz, ei->ExposureBias);
				if (*wsz == 0x00b1) *wsz = 0x3b;
				str.AppendFormat(_T("%s "), wsz);
			}
			// iso
			if (ei->ISOEquiv != 0)
				str.AppendFormat(_T("%d "), ei->ISOEquiv);
			else
				str.AppendFormat(_T("%s "), _T("Auto"));
			// フラッシュ
			if (ei->FlashFired)
				str += _T("* ");

			lstrcpyn(ei->pszLabelText, str, ei->cchLabelText);
		}
	}
	return ok;
}


////////////////////////////////////////////////////////////////////////
//

#ifdef WIN32_CONSOLE_EXECUTE
int _tmain(int argc, _TCHAR* argv[])
{
	int arg, status=0;
	int i, c;
	TCHAR opm, opt, *sp, *cp;

	if (argc == 1) {
		_tprintf(_T("\nRaw photo decoder \"dcraw\" v%s"), VERSION);
		_tprintf(_T("\nby Dave Coffin, dcoffin a cybercom o net\n"));
		_tprintf(_T("\nUsage:  %s [OPTION]... [FILE]...\n\n"), argv[0]);
		_putts(_T("-v        Print verbose messages"));
		_putts(_T("-c        Write image data to standard output"));
		_putts(_T("-e        Extract embedded thumbnail image"));
		_putts(_T("-i        Identify files without decoding them"));
		_putts(_T("-i -v     Identify files and show metadata"));
		_putts(_T("-z        Change file dates to camera timestamp"));
		_putts(_T("-w        Use camera white balance, if possible"));
		_putts(_T("-a        Average the whole image for white balance"));
		_putts(_T("-A <x y w h> Average a grey box for white balance"));
		_putts(_T("-r <r g b g> Set custom white balance"));
		_putts(_T("+M/-M     Use/don't use an embedded color matrix"));
		_putts(_T("-C <r b>  Correct chromatic aberration"));
		_putts(_T("-n <num>  Set threshold for wavelet denoising"));
		_putts(_T("-k <num>  Set the black level"));
		_putts(_T("-K <file> Subtract dark frame (16-bit raw PGM)"));
		_putts(_T("-H [0-9]  Highlight mode (0=clip, 1=unclip, 2=blend, 3+=rebuild)"));
		_putts(_T("-t [0-7]  Flip image (0=none, 3=180, 5=90CCW, 6=90CW)"));
		_putts(_T("-o [0-5]  Output colorspace (raw,sRGB,Adobe,Wide,ProPhoto,XYZ)"));
		_putts(_T("-d        Document mode (no color, no interpolation)"));
		_putts(_T("-D        Document mode without scaling (totally raw)"));
		_putts(_T("-j        Don't stretch or rotate raw pixels"));
		_putts(_T("-W        Don't automatically brighten the image"));
		_putts(_T("-b <num>  Adjust brightness (default = 1.0)"));
		_putts(_T("-q [0-3]  Set the interpolation quality"));
		_putts(_T("-h        Half-size color image (twice as fast as \"-q 0\")"));
		_putts(_T("-f        Interpolate RGGB as four colors"));
		_putts(_T("-m <num>  Apply a 3x3 median filter to R-G and B-G"));
		_putts(_T("-s [0..N-1] Select one raw image or \"all\" from each file"));
		_putts(_T("-4        Write 16-bit linear instead of 8-bit with gamma"));
		_putts(_T("-T        Write TIFF instead of PPM"));
		_putts(_T("-B        Write BMP instead of TIFF or PPM"));
		_putts(_T(""));
		return 1;
	}

	panaraw info;

	argv[argc] = NULL;
	for (arg=1; (((opm = argv[arg][0]) - 2) | 2) == '+'; ) {
		opt = argv[arg++][1];
		if ((cp = StrChr(sp = _T("nbrktqmHAC"), opt)))
			for (i=0; i < _T("1141111142")[cp-sp]-'0'; i++)
				if (!isdigit(argv[arg+i][0])) {
					_ftprintf(stderr, _T("Non-numeric argument to \"-%c\"\n"), opt);
					return 1;
				}

		float fv[4];
		unsigned uv[4];
		switch (opt) {
		case 'b':	info.AdjustBrightness(_tstof(argv[arg++]));
					break;
		case 'r':	FORC4 fv[c] = _tstof(argv[arg++]);
					info.SetCustomWhiteBalance(fv[0], fv[1], fv[2], fv[3]);
					break;
		case 'C':	fv[0] = _tstof(argv[arg++]);
					fv[1] = _tstof(argv[arg++]);
					info.CorrectChromaticAberration(fv[0], fv[1]);
					break;
		case 'o':	if (isdigit(argv[arg][0]) && !argv[arg][1])
						info.SetColorspace(_ttoi(argv[arg++]));
					break;
		case 'h':	info.SetHalfSizeImage(1);
					break;
		case 'A':	FORC4 uv[c] = _ttoi(argv[arg++]);
					info.AverageGreyBox(uv[0], uv[1], uv[2], uv[3]);
					break;
		case 'w':	info.UseCameraWhiteBalance(1);
					break;
		case 'M':	info.UseCameraMatrix((opm == '+') ? 1 : 0);
					break;
		case 'W':	info.NoAutoBright(1);
					break;
//@		case 'v':	info.Verbose(true);
//@					break;
		default:	_ftprintf(stderr, _T("Unknown option \"-%c\".\n"), opt);
					return 1;
		}
	}
	if (arg == argc) {
		_ftprintf(stderr, _T("No files to process.\n"));
		return 1;
	}
	if (arg < argc) {
		status = info.develop(argv[arg]);
	}
	return status;
}
#endif
