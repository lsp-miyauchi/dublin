/*
   dcraw.c -- Dave Coffin's raw photo decoder
   Copyright 1997-2007 by Dave Coffin, dcoffin a cybercom o net

   $Revision: 1.397 $
   $Date: 2007/12/10 07:43:31 $
 */

#include "stdafx.h"
#include "exifparse.h"

#pragma comment(lib, "ws2_32.lib")

typedef __int64 INT64;
typedef unsigned __int64 UINT64;

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;


#ifndef DIB_HEADER_MARKER
#define DIB_HEADER_MARKER   ((ushort)('M'<< 8)|'B')
#endif
#ifndef WIDTHBYTES
#define WIDTHBYTES(bits)    (((bits)+31)/32*4)
#endif

const double xyz_rgb[3][3] = {			/* XYZ from RGB */
  { 0.412453, 0.357580, 0.180423 },
  { 0.212671, 0.715160, 0.072169 },
  { 0.019334, 0.119193, 0.950227 } };

const float d65_white[3] = { 0.950456F, 1.0F, 1.088754F };


static struct decode {
	struct decode *branch[2];
	int leaf;
} first_decode[2048], *second_decode, *free_decode;

static struct {
	int width, height, bps, comp, phint, offset, flip, samples, bytes;
} tiff_ifd[10];

static struct {
	int format, key_off, m_black, black_off, split_col, tag_21a;
	float tag_210;
} ph1;

static struct jhead {
	int bits, high, wide, clrs, psv, restart, vpred[4];
	struct decode *huff[4];
	ushort *row;
};

#define SQR(x) ((x)*(x))
#define ABS(x) (((int)(x) ^ ((int)(x) >> 31)) - ((int)(x) >> 31))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define LIM(x,min,max) MAX(min,MIN(x,max))
#define ULIM(x,y,z) ((y) < (z) ? LIM(x,y,z) : LIM(x,z,y))
#define CLIP(x) LIM(x,0,65535)
#define SWAP(a,b) { a ^= b; a ^= (b ^= a); }

#define FORC3 for (c=0; c < 3; c++)
#define FORC4 for (c=0; c < 4; c++)
#define FORCC for (c=0; c < m_colors; c++)

class dcraw
{
public:
	FILE* m_ifp;
	short m_order;
	LPCTSTR m_ifname;
	unsigned m_filters;
	off_t    m_data_offset;
	off_t    m_thumb_offset;
	unsigned m_thumb_length;
	unsigned m_thumb_misc, *m_oprof;
	unsigned m_tiff_nifds, m_tiff_samples, m_tiff_bps, m_tiff_compress;
	unsigned m_black, m_maximum, m_mix_green, m_raw_color, m_zero_is_bad;
	unsigned m_zero_after_ff, m_is_raw, m_dng_version;
	unsigned m_tile_width, m_tile_length;
	ushort m_raw_height, m_raw_width, m_height, m_width, m_top_margin, m_left_margin;
	ushort m_shrink, m_iheight, m_iwidth, m_fuji_width, m_thumb_width, m_thumb_height;
	int m_flip, m_tiff_flip, m_colors;
	double m_pixel_aspect, m_aber[4];
	ushort (*m_image)[4], m_white[8][8], m_cr2_slice[3];
	ushort* m_curve;
	float m_bright, m_user_mul[4];
	int m_half_size, m_four_color_rgb, m_highlight, m_scaler_plus;
	int m_use_auto_wb, m_use_camera_wb, m_use_camera_matrix;
	int m_colorspace;
	int m_no_auto_bright;
	unsigned m_greybox[4];
	float m_cam_mul[4], m_pre_mul[4], m_cmatrix[3][4], m_rgb_cam[3][4];
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

public:
	dcraw()
	{
		m_aber[0]=m_aber[1]=m_aber[2]=m_aber[3]=1;
		m_user_mul[0]=m_user_mul[1]=m_user_mul[2]=m_user_mul[3]=0;
		m_bright=1;
		m_half_size=m_four_color_rgb=m_scaler_plus=m_highlight=0;
		m_use_auto_wb=m_use_camera_wb=0;
		m_use_camera_matrix=-1;
		m_colorspace=1;
		m_no_auto_bright=0;
		m_greybox[0]=m_greybox[1]=0; m_greybox[2]=m_greybox[3]=UINT_MAX;

		m_histogram = new int[0x2000][4];
		m_curve = new ushort[0x4001];

	}
	~dcraw()
	{
		if (m_histogram) delete [] m_histogram;
		if (m_curve) delete [] m_curve;
	}

	int fc(int row, int col);
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
	void init_decoder();
	uchar* make_decoder(const uchar *source, int level);
	int ljpeg_start(struct jhead *jh, int info_only);
	int ljpeg_diff(struct decode *dindex);
	ushort* ljpeg_row(int jrow, struct jhead *jh);
	void adobe_copy_pixel(int row, int col, ushort **rp);
	unsigned ph1_bits(int nbits);
	unsigned pana_bits(int nbits);
	const int* make_decoder_int(const int *source, int level);
	void sony_decrypt(unsigned *data, int len, int start, int key);
	void pseudoinverse(double (*in)[3], double (*out)[3], int size);
	void cam_xyz_coeff(double cam_xyz[4][3]);
	void scale_colors();
	void pre_interpolate();
	void border_interpolate(int border);
	void lin_interpolate();
	void vng_interpolate();
	void ppg_interpolate();
	void ahd_interpolate();
	void blend_highlights();
	void recover_highlights();
	void tiff_get(unsigned base, unsigned *tag, unsigned *type, unsigned *len, unsigned *save);
	void parse_thumb_note(int base, unsigned toff, unsigned tlen);
	void parse_makernote(int base, int uptag);
	void parse_exif(int base);
	void linear_table(unsigned len);
	int parse_tiff_ifd(int base);
	void parse_tiff(int base);
	void adobe_coeff(char *m_make, char *m_model);
	void identify();
	void convert_to_rgb();
	void fuji_rotate();
	void stretch();
	int flip_index(int row, int col);
	void gamma_lut(uchar lut[0x10000]);
	void write_dib(FILE *ofp);
	void write_dib_stream(IStream* pstrm);

	void lossless_jpeg_load_raw();
	void packed_12_load_raw();
	void unpacked_load_raw();
	void panasonic_load_raw();
	void olympus_e300_load_raw();
	void nikon_load_raw();
	void hasselblad_load_raw();
	void eight_bit_load_raw();
	void adobe_dng_load_raw_lj();
	void adobe_dng_load_raw_nc();

	static void stp_lossless_jpeg_load_raw(void* param) { ((dcraw*)param)->lossless_jpeg_load_raw(); }
	static void stp_packed_12_load_raw(void* param) { ((dcraw*)param)->packed_12_load_raw(); }
	static void stp_unpacked_load_raw(void* param) { ((dcraw*)param)->unpacked_load_raw(); }
	static void stp_panasonic_load_raw(void* param) { ((dcraw*)param)->panasonic_load_raw(); }
	static void stp_olympus_e300_load_raw(void* param) { ((dcraw*)param)->olympus_e300_load_raw(); }
	static void stp_nikon_load_raw(void* param) { ((dcraw*)param)->nikon_load_raw(); }
	static void stp_hasselblad_load_raw(void* param) { ((dcraw*)param)->hasselblad_load_raw(); }
	static void stp_eight_bit_load_raw(void* param) { ((dcraw*)param)->eight_bit_load_raw(); }
	static void stp_adobe_dng_load_raw_lj(void* param) { ((dcraw*)param)->adobe_dng_load_raw_lj(); }
	static void stp_adobe_dng_load_raw_nc(void* param) { ((dcraw*)param)->adobe_dng_load_raw_nc(); }

	int develop(LPCTSTR pszName, IStream* pstrm, int scale);
};

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

#define FC(row,col) \
	(m_filters >> ((((row) << 1 & 14) + ((col) & 1)) << 1) & 3)

#define BAYER(row,col) \
	m_image[((row) >> m_shrink)*m_iwidth + ((col) >> m_shrink)][FC(row,col)]

#define BAYER2(row,col) \
	m_image[((row) >> m_shrink)*m_iwidth + ((col) >> m_shrink)][fc(row,col)]

int dcraw::fc(int row, int col)
{
	static const char filter[16][16] =
	{	{ 2,1,1,3,2,3,2,0,3,2,3,0,1,2,1,0 },
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
		{ 0,3,1,0,0,2,0,3,2,1,3,1,1,3,1,3 }	};

	if (m_filters != 1) return FC(row,col);
	return filter[(row+m_top_margin) & 15][(col+m_left_margin) & 15];
}

void dcraw::derror() { }


ushort dcraw::sget2(uchar *s)
{
  if (m_order == 0x4949)		/* "II" means little-endian */
    return s[0] | s[1] << 8;
  else				/* "MM" means big-endian */
    return s[0] << 8 | s[1];
}

ushort dcraw::get2()
{
  uchar str[2] = { 0xff,0xff };
  fread (str, 1, 2, m_ifp);
  return sget2(str);
}

unsigned dcraw::sget4(uchar *s)
{
  if (m_order == 0x4949)
    return s[0] | s[1] << 8 | s[2] << 16 | s[3] << 24;
  else
    return s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3];
}

unsigned dcraw::get4()
{
  uchar str[4] = { 0xff,0xff,0xff,0xff };
  fread (str, 1, 4, m_ifp);
  return sget4(str);
}

unsigned dcraw::getint(int type)
{
  return type == 3 ? get2() : get4();
}

float dcraw::int_to_float(int i)
{
  union { int i; float f; } u;
  u.i = i;
  return u.f;
}

double dcraw::getreal(int type)
{
  union { char c[8]; double d; } u;
  int i, rev;

  switch (type) {
    case 3: return (unsigned short) get2();
    case 4: return (unsigned int) get4();
    case 5:  u.d = (unsigned int) get4();
      return u.d / (unsigned int) get4();
    case 8: return (signed short) get2();
    case 9: return (signed int) get4();
    case 10: u.d = (signed int) get4();
      return u.d / (signed int) get4();
    case 11: return int_to_float (get4());
    case 12:
      rev = 7 * ((m_order == 0x4949) == (ntohs(0x1234) == 0x1234));
      for (i=0; i < 8; i++)
	u.c[i ^ rev] = fgetc(m_ifp);
      return u.d;
    default: return fgetc(m_ifp);
  }
}

void dcraw::read_shorts(ushort *pixel, int count)
{
  if (fread (pixel, 2, count, m_ifp) < count) derror();
  if ((m_order == 0x4949) == (ntohs(0x1234) == 0x1234))
    _swab((char*)pixel, (char*)pixel, count*2);
}

void dcraw::remove_zeroes()
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
unsigned dcraw::getbits(int nbits)
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

void dcraw::init_decoder()
{
  memset (first_decode, 0, sizeof first_decode);
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
uchar* dcraw::make_decoder(const uchar *source, int level)
{
  struct decode *cur;
  static int leaf;
  int i, next;

  if (level==0) leaf=0;
  cur = free_decode++;
  if (free_decode > first_decode+2048) {
	return 0;
  }
  for (i=next=0; i <= leaf && next < 16; )
    i += source[next++];
  if (i > leaf) {
    if (level < next) {
      cur->branch[0] = free_decode;
      make_decoder (source, level+1);
      cur->branch[1] = free_decode;
      make_decoder (source, level+1);
    } else
      cur->leaf = source[16 + leaf++];
  }
  return (uchar *) source + 16 + leaf;
}

int dcraw::ljpeg_start(struct jhead *jh, int info_only)
{
  int i, tag, len;
  uchar data[0x10000], *dp;

  init_decoder();
  memset (jh, 0, sizeof *jh);
  for (i=0; i < 4; i++)
    jh->huff[i] = free_decode;
  jh->restart = INT_MAX;
  fread (data, 2, 1, m_ifp);
  if (data[1] != 0xd8) return 0;
  do {
    fread (data, 2, 2, m_ifp);
    tag =  data[0] << 8 | data[1];
    len = (data[2] << 8 | data[3]) - 2;
    if (tag <= 0xff00) return 0;
    fread (data, 1, len, m_ifp);
    switch (tag) {
      case 0xffc0: data[7] = 0;
      case 0xffc3:
	jh->bits = data[0];
	jh->high = data[1] << 8 | data[2];
	jh->wide = data[3] << 8 | data[4];
	jh->clrs = data[5] + (data[7] == 0x21);
	if (len == 9 && !m_dng_version) getc(m_ifp);
	break;
      case 0xffc4:
	if (info_only) break;
	for (dp = data; dp < data+len && *dp < 4; ) {
	  jh->huff[*dp] = free_decode;
	  dp = make_decoder (++dp, 0);
	}
	break;
      case 0xffda:
	jh->psv = data[1+data[0]*2];
	break;
      case 0xffdd:
	jh->restart = data[0] << 8 | data[1];
    }
  } while (tag != 0xffda);
  if (info_only) return 1;
  if (jh->clrs == 4) {
    jh->huff[3] = jh->huff[2] = jh->huff[1];
    jh->huff[1] = jh->huff[0];
  }
  jh->row = (ushort *) calloc (jh->wide*jh->clrs, 4);
  return m_zero_after_ff = 1;
}

int dcraw::ljpeg_diff(struct decode *dindex)
{
  int len, diff;

  while (dindex->branch[0])
    dindex = dindex->branch[getbits(1)];
  len = dindex->leaf;
  if (len == 16 && (!m_dng_version || m_dng_version >= 0x1010000))
    return -32768;
  diff = getbits(len);
  if ((diff & (1 << (len-1))) == 0)
    diff -= (1 << len) - 1;
  return diff;
}

ushort* dcraw::ljpeg_row(int jrow, struct jhead *jh)
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

void dcraw::lossless_jpeg_load_raw()
{
  int jwide, jrow, jcol, val, jidx, i, j, row=0, col=0;
  struct jhead jh;
  int min=INT_MAX;
  ushort *rp;

  if (!ljpeg_start (&jh, 0)) return;
  jwide = jh.wide * jh.clrs;

  for (jrow=0; jrow < jh.high; jrow++) {
    rp = ljpeg_row (jrow, &jh);
    for (jcol=0; jcol < jwide; jcol++) {
      val = *rp++;
      if (jh.bits <= 12)
	val = m_curve[val];
      if (m_cr2_slice[0]) {
	jidx = jrow*jwide + jcol;
	i = jidx / (m_cr2_slice[1]*jh.high);
	if ((j = i >= m_cr2_slice[0]))
		 i  = m_cr2_slice[0];
	jidx -= i * (m_cr2_slice[1]*jh.high);
	row = jidx / m_cr2_slice[1+j];
	col = jidx % m_cr2_slice[1+j] + i*m_cr2_slice[1];
      }
      if (m_raw_width == 3984 && (col -= 2) < 0)
	col += (row--,m_raw_width);
      if ((unsigned) (row-m_top_margin) < m_height) {
	if ((unsigned) (col-m_left_margin) < m_width) {
	  BAYER(row-m_top_margin,col-m_left_margin) = val;
	  if (min > val) min = val;
	} else m_black += val;
      }
      if (++col >= m_raw_width)
	col = (row++,0);
    }
  }
  free(jh.row);
  if (m_raw_width > m_width)
    m_black /= (m_raw_width - m_width) * m_height;
}

void dcraw::adobe_copy_pixel(int row, int col, ushort **rp)
{
  unsigned r, c;

  r = row -= m_top_margin;
  c = col -= m_left_margin;
  if (m_filters) {
    if (m_fuji_width) {
      r = row + m_fuji_width - 1 - (col >> 1);
      c = row + ((col+1) >> 1);
    }
    if (r < m_height && c < m_width)
      BAYER(r,c) = **rp < 0x1000 ? m_curve[**rp] : **rp;
    *rp += m_is_raw;
  } else {
    if (r < m_height && c < m_width)
      for (c=0; c < m_tiff_samples; c++)
	m_image[row*m_width+col][c] = (*rp)[c] < 0x1000 ? m_curve[(*rp)[c]]:(*rp)[c];
    *rp += m_tiff_samples;
  }
}

void dcraw::adobe_dng_load_raw_lj()
{
  unsigned save, trow=0, tcol=0, jwide, jrow, jcol, row, col;
  struct jhead jh;
  ushort *rp;

  while (trow < m_raw_height) {
    save = ftell(m_ifp);
    if (m_tile_length < INT_MAX)
      fseek (m_ifp, get4(), SEEK_SET);
    if (!ljpeg_start (&jh, 0)) break;
    jwide = jh.wide;
    if (m_filters) jwide *= jh.clrs;
    jwide /= m_is_raw;
    for (row=col=jrow=0; jrow < jh.high; jrow++) {
      rp = ljpeg_row (jrow, &jh);
      for (jcol=0; jcol < jwide; jcol++) {
	adobe_copy_pixel (trow+row, tcol+col, &rp);
	if (++col >= m_tile_width || col >= m_raw_width)
	  row += 1 + (col = 0);
      }
    }
    fseek (m_ifp, save+4, SEEK_SET);
    if ((tcol += m_tile_width) >= m_raw_width)
      trow += m_tile_length + (tcol = 0);
    free(jh.row);
  }
}

void dcraw::adobe_dng_load_raw_nc()
{
  ushort *pixel, *rp;
  int row, col;

  pixel = new ushort[m_raw_width * m_tiff_samples];
  for (row=0; row < m_raw_height; row++) {
    if (m_tiff_bps == 16)
      read_shorts (pixel, m_raw_width * m_tiff_samples);
    else {
      getbits(-1);
      for (col=0; col < m_raw_width * m_tiff_samples; col++)
	pixel[col] = getbits(m_tiff_bps);
    }
    for (rp=pixel, col=0; col < m_raw_width; col++)
      adobe_copy_pixel (row, col, &rp);
  }
  delete [] pixel;
}

void dcraw::nikon_load_raw()
{
  int irow, row, col, i;

  getbits(-1);
  for (irow=0; irow < m_height; irow++) {
    row = irow;
    if (m_make[0] == 'O' || m_model[0] == 'E') {
      row = irow * 2 % m_height + irow / (m_height/2);
      if (row == 1 && m_data_offset == 0) {
	fseek (m_ifp, 0, SEEK_END);
	fseek (m_ifp, ftell(m_ifp)/2, SEEK_SET);
	getbits(-1);
      }
    }
    for (col=0; col < m_raw_width; col++) {
      i = getbits(12);
      if ((unsigned) (col-m_left_margin) < m_width)
	BAYER(row,col-m_left_margin) = i;
      if (m_tiff_compress > 32768 && (col % 10) == 9)
	if (getbits(8)) derror();
    }
  }
}

unsigned dcraw::ph1_bits(int nbits)
{
  static UINT64 bitbuf=0;
  static int vbits=0;

  if (nbits == -1)
    return bitbuf = vbits = 0;
  if (nbits == 0) return 0;
  if (vbits < nbits) {
    bitbuf = bitbuf << 32 | get4();
    vbits += 32;
  }
  vbits -= nbits;
  return bitbuf << (64-nbits-vbits) >> (64-nbits);
}

void dcraw::hasselblad_load_raw()
{
  struct jhead jh;
  struct decode *dindex;
  int row, col, pred[2], len[2], diff, i;

  if (!ljpeg_start (&jh, 0)) return;
  free(jh.row);
  ph1_bits(-1);
  for (row=-m_top_margin; row < m_height; row++) {
    pred[0] = pred[1] = 0x8000;
    for (col=-m_left_margin; col < m_raw_width-m_left_margin; col+=2) {
      for (i=0; i < 2; i++) {
	for (dindex=jh.huff[0]; dindex->branch[0]; )
	  dindex = dindex->branch[ph1_bits(1)];
	len[i] = dindex->leaf;
      }
      for (i=0; i < 2; i++) {
	diff = ph1_bits(len[i]);
	if ((diff & (1 << (len[i]-1))) == 0)
	  diff -= (1 << len[i]) - 1;
	pred[i] += diff;
	if (row >= 0 && (unsigned)(col+i) < m_width)
	  BAYER(row,col+i) = pred[i];
      }
    }
  }
  m_maximum = 0xffff;
}

void dcraw::packed_12_load_raw()
{
  int row, col;

  if (m_raw_width * 2 < m_width * 3)
    m_raw_width = m_raw_width * 3 / 2;	/* Convert m_raw_width to bytes */
  getbits(-1);
  for (row=0; row < m_height; row++) {
    for (col=0; col < m_left_margin; col++)
      getbits(12);
    for (col=0; col < m_width; col++)
      BAYER(row,col) = getbits(12);
    for (col = (m_width+m_left_margin)*3/2; col < m_raw_width; col++)
      if (getbits(8) && m_raw_width-col < 35 && m_width != 3896) derror();
  }
}

void dcraw::unpacked_load_raw()
{
  ushort *pixel;
  int row, col, bits=0;

  while (1 << ++bits < m_maximum);
  fseek (m_ifp, (m_top_margin*m_raw_width + m_left_margin) * 2, SEEK_CUR);
  pixel = new ushort[m_width];
  for (row=0; row < m_height; row++) {
    read_shorts (pixel, m_width);
    fseek (m_ifp, 2*(m_raw_width - m_width), SEEK_CUR);
    for (col=0; col < m_width; col++)
      if ((BAYER2(row,col) = pixel[col]) >> bits) derror();
  }
  delete [] pixel;
}

unsigned dcraw::pana_bits(int nbits)
{
  static uchar buf[16], vbits=0;

  if (!vbits && fread (buf, 1, 16, m_ifp) < 16) derror();
  vbits = (vbits - nbits) & 127;
  return (buf[(vbits >> 3)+1] << 8 | buf[vbits >> 3])
	>> (vbits & 7) & ~(-1 << nbits);
}

void dcraw::panasonic_load_raw()
{
  int row, col, i, j, sh=0, pred[2], nonz[2];

  m_raw_width = (m_raw_width+13)/14*14;
  for (row=0; row < m_height; row++)
    for (col=0; col < m_raw_width; col++) {
      if ((i = col % 14) < 2)
	nonz[i] = pred[i] = pana_bits(12);
      else {
	if (i % 3 == 2) sh = 4 >> (3 - pana_bits(2));
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

void dcraw::olympus_e300_load_raw()
{
  uchar  *data,  *dp;
  ushort *pixel, *pix;
  int dwide, row, col;

  dwide = m_raw_width * 16 / 10;
  fseek (m_ifp, dwide*m_top_margin, SEEK_CUR);
  data = (uchar *) malloc (dwide + m_raw_width*2);
  pixel = (ushort *) (data + dwide);
  for (row=0; row < m_height; row++) {
    if (fread (data, 1, dwide, m_ifp) < dwide) derror();
    for (dp=data, pix=pixel; pix < pixel+m_raw_width; dp+=3, pix+=2) {
      if (((dp-data) & 15) == 15)
	if (*dp++ && pix < pixel+m_width+m_left_margin) derror();
      pix[0] = dp[1] << 8 | dp[0];
      pix[1] = dp[2] << 4 | dp[1] >> 4;
    }
    for (col=0; col < m_width; col++)
      BAYER(row,col) = (pixel[col+m_left_margin] & 0xfff);
  }
  free(data);
  m_maximum >>= 4;
  m_black >>= 4;
}

const int* dcraw::make_decoder_int(const int *source, int level)
{
  struct decode *cur;

  cur = free_decode++;
  if (level < source[0]) {
    cur->branch[0] = free_decode;
    source = make_decoder_int (source, level+1);
    cur->branch[1] = free_decode;
    source = make_decoder_int (source, level+1);
  } else {
    cur->leaf = source[1];
    source += 2;
  }
  return source;
}

void dcraw::eight_bit_load_raw()
{
  unsigned row, col, val, lblack=0;

  uchar* pixel = new uchar[m_raw_width];
  fseek (m_ifp, m_top_margin*m_raw_width, SEEK_CUR);
  for (row=0; row < m_height; row++) {
    if (fread (pixel, 1, m_raw_width, m_ifp) < m_raw_width) derror();
    for (col=0; col < m_raw_width; col++) {
      val = m_curve[pixel[col]];
      if ((unsigned) (col-m_left_margin) < m_width)
        BAYER(row,col-m_left_margin) = val;
      else lblack += val;
    }
  }
  delete [] pixel;
  if (m_raw_width > m_width+1)
    m_black = lblack / ((m_raw_width - m_width) * m_height);
  m_maximum = m_curve[0xff];
}

void dcraw::sony_decrypt(unsigned *data, int len, int start, int key)
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

void dcraw::pseudoinverse(double (*in)[3], double (*out)[3], int size)
{
  double work[3][6], num;
  int i, j, k;

  for (i=0; i < 3; i++) {
    for (j=0; j < 6; j++)
      work[i][j] = j == i+3;
    for (j=0; j < 3; j++)
      for (k=0; k < size; k++)
	work[i][j] += in[k][i] * in[k][j];
  }
  for (i=0; i < 3; i++) {
    num = work[i][i];
    for (j=0; j < 6; j++)
      work[i][j] /= num;
    for (k=0; k < 3; k++) {
      if (k==i) continue;
      num = work[k][i];
      for (j=0; j < 6; j++)
	work[k][j] -= work[i][j] * num;
    }
  }
  for (i=0; i < size; i++)
    for (j=0; j < 3; j++)
      for (out[i][j]=k=0; k < 3; k++)
	out[i][j] += work[j][k+3] * in[i][k];
}

void dcraw::cam_xyz_coeff(double cam_xyz[4][3])
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
    m_pre_mul[i] = 1 / num;
  }
  pseudoinverse (cam_rgb, inverse, m_colors);
  for (m_raw_color = i=0; i < 3; i++)
    for (j=0; j < m_colors; j++)
      m_rgb_cam[i][j] = inverse[j][i];
}

void dcraw::scale_colors()
{
  unsigned bottom, right, size, row, col, ur, uc, i, x, y, c, sum[8];
  int val, dblack;
  double dsum[8], dmin, dmax;
  float scale_mul[4], fr, fc;
  ushort *img=0, *pix;

  if (m_user_mul[0])
    memcpy (m_pre_mul, m_user_mul, sizeof m_pre_mul);
  if (m_use_auto_wb || (m_use_camera_wb && m_cam_mul[0] == -1)) {
    memset (dsum, 0, sizeof dsum);
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
	      if (val > m_maximum-25) goto skip_block;
	      if ((val -= m_black) < 0) val = 0;
	      sum[c] += val;
	      sum[c+4]++;
	      if (m_filters) break;
	    }
	for (c=0; c < 8; c++) dsum[c] += sum[c];
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
//    else
//      fprintf (stderr,_("%s: Cannot use camera white balance.\n"), ifname);
  }
  if (m_pre_mul[3] == 0) m_pre_mul[3] = m_colors < 4 ? m_pre_mul[1] : 1;
  dblack = m_black;
//  if (threshold) wavelet_denoise();
  m_maximum -= m_black;
  for (dmin=DBL_MAX, dmax=c=0; c < 4; c++) {
    if (dmin > m_pre_mul[c])
	dmin = m_pre_mul[c];
    if (dmax < m_pre_mul[c])
	dmax = m_pre_mul[c];
  }
  if (!m_highlight) dmax = dmin;
  FORC4 scale_mul[c] = (m_pre_mul[c] /= dmax) * 65535.0 / m_maximum;
//  if (verbose) {
//    fprintf (stderr,_("Scaling with black %d, multipliers"), dblack);
//    FORC4 fprintf (stderr, " %f", pre_mul[c]);
//    fputc ('\n', stderr);
//  }
  size = m_iheight*m_iwidth;
  for (i=0; i < size*4; i++) {
    val = m_image[0][i];
    if (!val) continue;
    val -= m_black;
    val *= scale_mul[i & 3];
    m_image[0][i] = CLIP(val);
  }
  if ((m_aber[0] != 1 || m_aber[2] != 1) && m_colors == 3) {
//    if (verbose)
//      fprintf (stderr,_("Correcting chromatic aberration...\n"));
    for (c=0; c < 4; c+=2) {
      if (m_aber[c] == 1) continue;
      img = new ushort[size];
//      merror (img, "scale_colors()");
      for (i=0; i < size; i++)
	img[i] = m_image[i][c];
      for (row=0; row < m_iheight; row++) {
	ur = fr = (row - m_iheight*0.5) * m_aber[c] + m_iheight*0.5;
	if (ur > m_iheight-2) continue;
	fr -= ur;
	for (col=0; col < m_iwidth; col++) {
	  uc = fc = (col - m_iwidth*0.5) * m_aber[c] + m_iwidth*0.5;
	  if (uc > m_iwidth-2) continue;
	  fc -= uc;
	  pix = img + ur*m_iwidth + uc;
	  m_image[row*m_iwidth+col][c] =
	    (pix[       0]*(1-fc) + pix[         1]*fc) * (1-fr) +
	    (pix[m_iwidth]*(1-fc) + pix[m_iwidth+1]*fc) * fr;
	}
      }
      delete [] img;
    }
  }
}

void dcraw::pre_interpolate()
{
  ushort (*img)[4];
  int row, col, c;

  if (m_shrink) {
    if (m_half_size) {
      m_height = m_iheight;
      m_width  = m_iwidth;
    } else {
      img = new ushort[m_height*m_width][4];
      for (row=0; row < m_height; row++)
	for (col=0; col < m_width; col++) {
	  c = fc(row,col);
	  img[row*m_width+col][c] = m_image[(row >> 1)*m_iwidth+(col >> 1)][c];
	}
      delete [] m_image;
      m_image = img;
      m_shrink = 0;
    }
  }
  if (m_filters && m_colors == 3) {
    if ((m_mix_green = m_four_color_rgb)) m_colors++;
    else {
      for (row = FC(1,0) >> 1; row < m_height; row+=2)
	for (col = FC(row,1) & 1; col < m_width; col+=2)
	  m_image[row*m_width+col][1] = m_image[row*m_width+col][3];
      m_filters &= ~((m_filters & 0x55555555) << 1);
    }
  }
  if (m_half_size) m_filters = 0;
}

void dcraw::border_interpolate(int border)
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

void dcraw::lin_interpolate()
{
  int code[16][16][32], *ip, sum[4];
  int c, i, x, y, row, col, shift, color;
  ushort *pix;

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
   This algorithm is officially called:

   "Interpolation using a Threshold-based variable number of gradients"

   described in http://scien.stanford.edu/class/psych221/projects/99/tingchen/algodep/vargra.html

   I've extended the basic idea to work with non-Bayer filter arrays.
   Gradients are numbered clockwise from NW=0 to W=7.
 */
void dcraw::vng_interpolate()
{
  static const signed char *cp, terms[] = {
    -2,-2,+0,-1,0,0x01, -2,-2,+0,+0,1,0x01, -2,-1,-1,+0,0,0x01,
    -2,-1,+0,-1,0,0x02, -2,-1,+0,+0,0,0x03, -2,-1,+0,+1,1,0x01,
    -2,+0,+0,-1,0,0x06, -2,+0,+0,+0,1,0x02, -2,+0,+0,+1,0,0x03,
    -2,+1,-1,+0,0,0x04, -2,+1,+0,-1,1,0x04, -2,+1,+0,+0,0,0x06,
    -2,+1,+0,+1,0,0x02, -2,+2,+0,+0,1,0x04, -2,+2,+0,+1,0,0x04,
    -1,-2,-1,+0,0,0x80, -1,-2,+0,-1,0,0x01, -1,-2,+1,-1,0,0x01,
    -1,-2,+1,+0,1,0x01, -1,-1,-1,+1,0,0x88, -1,-1,+1,-2,0,0x40,
    -1,-1,+1,-1,0,0x22, -1,-1,+1,+0,0,0x33, -1,-1,+1,+1,1,0x11,
    -1,+0,-1,+2,0,0x08, -1,+0,+0,-1,0,0x44, -1,+0,+0,+1,0,0x11,
    -1,+0,+1,-2,1,0x40, -1,+0,+1,-1,0,0x66, -1,+0,+1,+0,1,0x22,
    -1,+0,+1,+1,0,0x33, -1,+0,+1,+2,1,0x10, -1,+1,+1,-1,1,0x44,
    -1,+1,+1,+0,0,0x66, -1,+1,+1,+1,0,0x22, -1,+1,+1,+2,0,0x10,
    -1,+2,+0,+1,0,0x04, -1,+2,+1,+0,1,0x04, -1,+2,+1,+1,0,0x04,
    +0,-2,+0,+0,1,0x80, +0,-1,+0,+1,1,0x88, +0,-1,+1,-2,0,0x40,
    +0,-1,+1,+0,0,0x11, +0,-1,+2,-2,0,0x40, +0,-1,+2,-1,0,0x20,
    +0,-1,+2,+0,0,0x30, +0,-1,+2,+1,1,0x10, +0,+0,+0,+2,1,0x08,
    +0,+0,+2,-2,1,0x40, +0,+0,+2,-1,0,0x60, +0,+0,+2,+0,1,0x20,
    +0,+0,+2,+1,0,0x30, +0,+0,+2,+2,1,0x10, +0,+1,+1,+0,0,0x44,
    +0,+1,+1,+2,0,0x10, +0,+1,+2,-1,1,0x40, +0,+1,+2,+0,0,0x60,
    +0,+1,+2,+1,0,0x20, +0,+1,+2,+2,0,0x10, +1,-2,+1,+0,0,0x80,
    +1,-1,+1,+1,0,0x88, +1,+0,+1,+2,0,0x08, +1,+0,+2,-1,0,0x40,
    +1,+0,+2,+1,0,0x10
  }, chood[] = { -1,-1, -1,0, -1,+1, 0,+1, +1,+1, +1,0, +1,-1, 0,-1 };
  ushort (*brow[5])[4], *pix;
  int prow=7, pcol=1, *ip, *code[16][16], gval[8], gmin, gmax, sum[4];
  int row, col, x, y, x1, x2, y1, y2, t, weight, grads, color, diag;
  int g, diff, thold, num, c;

  lin_interpolate();

  if (m_filters == 1) prow = pcol = 15;
  ip = (int *) calloc ((prow+1)*(pcol+1), 1280);
  for (row=0; row <= prow; row++)		/* Precalculate for VNG */
    for (col=0; col <= pcol; col++) {
      code[row][col] = ip;
      for (cp=terms, t=0; t < 64; t++) {
	y1 = *cp++;  x1 = *cp++;
	y2 = *cp++;  x2 = *cp++;
	weight = *cp++;
	grads = *cp++;
	color = fc(row+y1,col+x1);
	if (fc(row+y2,col+x2) != color) continue;
	diag = (fc(row,col+1) == color && fc(row+1,col) == color) ? 2:1;
	if (abs(y1-y2) == diag && abs(x1-x2) == diag) continue;
	*ip++ = (y1*m_width + x1)*4 + color;
	*ip++ = (y2*m_width + x2)*4 + color;
	*ip++ = weight;
	for (g=0; g < 8; g++)
	  if (grads & 1<<g) *ip++ = g;
	*ip++ = -1;
      }
      *ip++ = INT_MAX;
      for (cp=chood, g=0; g < 8; g++) {
	y = *cp++;  x = *cp++;
	*ip++ = (y*m_width + x) * 4;
	color = fc(row,col);
	if (fc(row+y,col+x) != color && fc(row+y*2,col+x*2) == color)
	  *ip++ = (y*m_width + x) * 8 + color;
	else
	  *ip++ = 0;
      }
    }
  brow[4] = (ushort (*)[4]) calloc (m_width*3, sizeof **brow);
  for (row=0; row < 3; row++)
    brow[row] = brow[4] + row*m_width;
  for (row=2; row < m_height-2; row++) {		/* Do VNG interpolation */
    for (col=2; col < m_width-2; col++) {
      pix = m_image[row*m_width+col];
      ip = code[row & prow][col & pcol];
      memset (gval, 0, sizeof gval);
      while ((g = ip[0]) != INT_MAX) {		/* Calculate gradients */
	diff = ABS(pix[g] - pix[ip[1]]) << ip[2];
	gval[ip[3]] += diff;
	ip += 5;
	if ((g = ip[-1]) == -1) continue;
	gval[g] += diff;
	while ((g = *ip++) != -1)
	  gval[g] += diff;
      }
      ip++;
      gmin = gmax = gval[0];			/* Choose a threshold */
      for (g=1; g < 8; g++) {
	if (gmin > gval[g]) gmin = gval[g];
	if (gmax < gval[g]) gmax = gval[g];
      }
      if (gmax == 0) {
	memcpy (brow[2][col], pix, sizeof *m_image);
	continue;
      }
      thold = gmin + (gmax >> 1);
      memset (sum, 0, sizeof sum);
      color = fc(row,col);
      for (num=g=0; g < 8; g++,ip+=2) {		/* Average the neighbors */
	if (gval[g] <= thold) {
	  FORCC
	    if (c == color && ip[1])
	      sum[c] += (pix[c] + pix[ip[1]]) >> 1;
	    else
	      sum[c] += pix[ip[0] + c];
	  num++;
	}
      }
      FORCC {					/* Save to buffer */
	t = pix[color];
	if (c != color)
	  t += (sum[c] - sum[color]) / num;
	brow[2][col][c] = CLIP(t);
      }
    }
    if (row > 3)				/* Write buffer to m_image */
      memcpy (m_image[(row-2)*m_width+2], brow[0]+2, (m_width-4)*sizeof *m_image);
    for (g=0; g < 4; g++)
      brow[(g-1) & 3] = brow[g];
  }
  memcpy (m_image[(row-2)*m_width+2], brow[0]+2, (m_width-4)*sizeof *m_image);
  memcpy (m_image[(row-1)*m_width+2], brow[1]+2, (m_width-4)*sizeof *m_image);
  free(brow[4]);
  free(code[0][0]);
}

/*
   Patterned Pixel Grouping Interpolation by Alain Desbiolles
*/
void dcraw::ppg_interpolate()
{
  int gr[4], dir[5] = { 1, m_width, -1, -m_width, 1 };
  int row, col, avg, diff[2], guess[2], c, d, i;
  static const short sort[] = { 0,2,1,3,0,1,2,3 };
  ushort (*pix)[4];

  border_interpolate(3);

/*  Fill in the green layer with gradients and pattern recognition: */
  for (row=3; row < m_height-3; row++)
    for (col=3+(FC(row,3) & 1), c=FC(row,col); col < m_width-3; col+=2) {
      pix = m_image + row*m_width+col;
      for (avg=i=0; i < 4; i++)
	avg += gr[i] = pix[dir[i]][1] << 2;
      avg >>= 2;
      for (i=0; i < 8; i+=2)
	if (gr[sort[i]] > gr[sort[i+1]])
	  SWAP(gr[sort[i]],gr[sort[i+1]])
      for (d=0; d < 4; d++) {
	for (i=-2; i < 2; i++)
	  if (pix[i*dir[d] + (i+1)*dir[d+1]][1] <= avg) break;
	if (i == 2) {
	  pix[0][1] = (gr[1]+gr[2]) >> 3;
	  goto next_pixel;
	}
      }
      for (i=0; (d=dir[i]) > 0; i++) {
	guess[i] = (pix[-d][1] + pix[0][c] + pix[d][1]) * 2
		      - pix[-2*d][c] - pix[2*d][c];
	diff[i] = ( ABS(pix[-2*d][c] - pix[ 0][c]) +
		    ABS(pix[ 2*d][c] - pix[ 0][c]) +
		    ABS(pix[  -d][1] - pix[ d][1]) ) * 3 +
		  ( ABS(pix[ 3*d][1] - pix[ d][1]) +
		    ABS(pix[-3*d][1] - pix[-d][1]) ) * 2;
      }
      d = dir[i = diff[0] > diff[1]];
      pix[0][1] = ULIM(guess[i] >> 2, pix[d][1], pix[-d][1]);
next_pixel: ;
    }
/*  Calculate red and blue for each green pixel:		*/
  for (row=1; row < m_height-1; row++)
    for (col=1+(FC(row,2) & 1), c=FC(row,col+1); col < m_width-1; col+=2) {
      pix = m_image + row*m_width+col;
      for (i=0; (d=dir[i]) > 0; c=2-c, i++)
	pix[0][c] = CLIP((pix[-d][c] + pix[d][c] + 2*pix[0][1]
			- pix[-d][1] - pix[d][1]) >> 1);
    }
/*  Calculate blue for red pixels and vice versa:		*/
  for (row=1; row < m_height-1; row++)
    for (col=1+(FC(row,1) & 1), c=2-FC(row,col); col < m_width-1; col+=2) {
      pix = m_image + row*m_width+col;
      for (i=0; (d=dir[i]+dir[i+1]) > 0; i++) {
	diff[i] = ABS(pix[-d][c] - pix[d][c]) +
		  ABS(pix[-d][1] - pix[0][1]) +
		  ABS(pix[ d][1] - pix[0][1]);
	guess[i] = pix[-d][c] + pix[d][c] + 2*pix[0][1]
		 - pix[-d][1] - pix[d][1];
      }
      if (diff[0] != diff[1])
	pix[0][c] = CLIP(guess[diff[0] > diff[1]] >> 1);
      else
	pix[0][c] = CLIP((guess[0]+guess[1]) >> 2);
    }
}

/*
   Adaptive Homogeneity-Directed interpolation is based on
   the work of Keigo Hirakawa, Thomas Parks, and Paul Lee.
 */
#define TS 256		/* Tile Size */

void dcraw::ahd_interpolate()
{
  int i, j, k, top, left, row, col, tr, tc, c, d, val, hm[2];
  ushort (*pix)[4], (*rix)[3];
  static const int dir[4] = { -1, 1, -TS, TS };
  unsigned ldiff[2][4], abdiff[2][4], leps, abeps;
  float r, cbrt[0x10000], xyz[3], xyz_cam[3][4];
  ushort (*rgb)[TS][TS][3];
   short (*lab)[TS][TS][3], (*lix)[3];
   char (*homo)[TS][TS], *buffer;

  for (i=0; i < 0x10000; i++) {
    r = i / 65535.0;
    cbrt[i] = r > 0.008856 ? powf(r,1/3.0) : 7.787*r + 16/116.0;
  }
  for (i=0; i < 3; i++)
    for (j=0; j < m_colors; j++)
      for (xyz_cam[i][j] = k=0; k < 3; k++)
	xyz_cam[i][j] += xyz_rgb[i][k] * m_rgb_cam[k][j] / d65_white[i];

  border_interpolate(5);
  buffer = (char *) malloc (26*TS*TS);		/* 1664 kB */
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
  free(buffer);
}
#undef TS

void dcraw::blend_highlights()
{
  int clip=INT_MAX, row, col, c, i, j;
  static const float trans[2][4][4] =
  { { { 1,1,1 }, { 1.7320508,-1.7320508,0 }, { -1,-1,2 } },
    { { 1,1,1,1 }, { 1,-1,1,-1 }, { 1,1,-1,-1 }, { 1,-1,-1,1 } } };
  static const float itrans[2][4][4] =
  { { { 1,0.8660254,-0.5 }, { 1,-0.8660254,-0.5 }, { 1,0,1 } },
    { { 1,1,1,1 }, { 1,-1,1,-1 }, { 1,1,-1,-1 }, { 1,-1,-1,1 } } };
  float cam[2][4], lab[2][4], sum[2], chratio;

  if ((unsigned)(m_colors-3) > 1) return;
//  if (verbose) fprintf (stderr,_("Blending highlights...\n"));
  FORCC if (clip > (i = 65535*m_pre_mul[c])) clip = i;
  for (row=0; row < m_height; row++)
    for (col=0; col < m_width; col++) {
      FORCC if (m_image[row*m_width+col][c] > clip) break;
      if (c == m_colors) continue;
      FORCC {
	cam[0][c] = m_image[row*m_width+col][c];
	cam[1][c] = MIN(cam[0][c],clip);
      }
      for (i=0; i < 2; i++) {
	FORCC for (lab[i][c]=j=0; j < m_colors; j++)
	  lab[i][c] += trans[m_colors-3][c][j] * cam[i][j];
	for (sum[i]=0,c=1; c < m_colors; c++)
	  sum[i] += SQR(lab[i][c]);
      }
      chratio = sqrt(sum[1]/sum[0]);
      for (c=1; c < m_colors; c++)
	lab[0][c] *= chratio;
      FORCC for (cam[0][c]=j=0; j < m_colors; j++)
	cam[0][c] += itrans[m_colors-3][c][j] * lab[0][j];
      FORCC m_image[row*m_width+col][c] = cam[0][c] / m_colors;
    }
}

#define SCALE (4 >> m_shrink)
void dcraw::recover_highlights()
{
  float *map, sum, wgt, grow;
  int hsat[4], count, spread, change, val, i;
  unsigned high, wide, mrow, mcol, row, col, kc, c, d, y, x;
  ushort *pixel;
  static const signed char dir[8][2] =
    { {-1,-1}, {-1,0}, {-1,1}, {0,1}, {1,1}, {1,0}, {1,-1}, {0,-1} };

//  if (verbose) fprintf (stderr,_("Rebuilding highlights...\n"));

  grow = powf(2, 4-m_highlight);
  FORCC hsat[c] = 32000 * m_pre_mul[c];
  for (kc=0, c=1; c < m_colors; c++)
    if (m_pre_mul[kc] < m_pre_mul[c]) kc = c;
  high = m_height / SCALE;
  wide =  m_width / SCALE;
  map = new float[high*wide];
//  merror (map, "recover_highlights()");
  FORCC if (c != kc) {
    memset (map, 0, high*wide*sizeof *map);
    for (mrow=0; mrow < high; mrow++)
      for (mcol=0; mcol < wide; mcol++) {
	sum = wgt = count = 0;
	for (row = mrow*SCALE; row < (mrow+1)*SCALE; row++)
	  for (col = mcol*SCALE; col < (mcol+1)*SCALE; col++) {
	    pixel = m_image[row*m_width+col];
	    if (pixel[c] / hsat[c] == 1 && pixel[kc] > 24000) {
	      sum += pixel[c];
	      wgt += pixel[kc];
	      count++;
	    }
	  }
	if (count == SCALE*SCALE)
	  map[mrow*wide+mcol] = sum / wgt;
      }
    for (spread = 32/grow; spread--; ) {
      for (mrow=0; mrow < high; mrow++)
	for (mcol=0; mcol < wide; mcol++) {
	  if (map[mrow*wide+mcol]) continue;
	  sum = count = 0;
	  for (d=0; d < 8; d++) {
	    y = mrow + dir[d][0];
	    x = mcol + dir[d][1];
	    if (y < high && x < wide && map[y*wide+x] > 0) {
	      sum  += (1 + (d & 1)) * map[y*wide+x];
	      count += 1 + (d & 1);
	    }
	  }
	  if (count > 3)
	    map[mrow*wide+mcol] = - (sum+grow) / (count+grow);
	}
      for (change=i=0; i < high*wide; i++)
	if (map[i] < 0) {
	  map[i] = -map[i];
	  change = 1;
	}
      if (!change) break;
    }
    for (i=0; i < high*wide; i++)
      if (map[i] == 0) map[i] = 1;
    for (mrow=0; mrow < high; mrow++)
      for (mcol=0; mcol < wide; mcol++) {
	for (row = mrow*SCALE; row < (mrow+1)*SCALE; row++)
	  for (col = mcol*SCALE; col < (mcol+1)*SCALE; col++) {
	    pixel = m_image[row*m_width+col];
	    if (pixel[c] / hsat[c] > 1) {
	      val = pixel[kc] * map[mrow*wide+mcol];
	      if (pixel[c] < val) pixel[c] = CLIP(val);
	    }
	  }
      }
  }
  delete [] map;
}
#undef SCALE

void dcraw::tiff_get(unsigned base, unsigned *tag, unsigned *type, unsigned *len, unsigned *save)
{
  *tag  = get2();
  *type = get2();
  *len  = get4();
  *save = ftell(m_ifp) + 4;
  if (*len * ("11124811248488"[*type < 14 ? *type:0]-'0') > 4)
    fseek (m_ifp, get4()+base, SEEK_SET);
}

void dcraw::parse_thumb_note(int base, unsigned toff, unsigned tlen)
{
  unsigned entries, tag, type, len, save;

  entries = get2();
  while (entries--) {
    tiff_get (base, &tag, &type, &len, &save);
    if (tag == toff) m_thumb_offset = get4()+base;
    if (tag == tlen) m_thumb_length = get4();
    fseek (m_ifp, save, SEEK_SET);
  }
}

void dcraw::parse_makernote(int base, int uptag)
{
  static const uchar xlat[2][256] = {
  { 0xc1,0xbf,0x6d,0x0d,0x59,0xc5,0x13,0x9d,0x83,0x61,0x6b,0x4f,0xc7,0x7f,0x3d,0x3d,
    0x53,0x59,0xe3,0xc7,0xe9,0x2f,0x95,0xa7,0x95,0x1f,0xdf,0x7f,0x2b,0x29,0xc7,0x0d,
    0xdf,0x07,0xef,0x71,0x89,0x3d,0x13,0x3d,0x3b,0x13,0xfb,0x0d,0x89,0xc1,0x65,0x1f,
    0xb3,0x0d,0x6b,0x29,0xe3,0xfb,0xef,0xa3,0x6b,0x47,0x7f,0x95,0x35,0xa7,0x47,0x4f,
    0xc7,0xf1,0x59,0x95,0x35,0x11,0x29,0x61,0xf1,0x3d,0xb3,0x2b,0x0d,0x43,0x89,0xc1,
    0x9d,0x9d,0x89,0x65,0xf1,0xe9,0xdf,0xbf,0x3d,0x7f,0x53,0x97,0xe5,0xe9,0x95,0x17,
    0x1d,0x3d,0x8b,0xfb,0xc7,0xe3,0x67,0xa7,0x07,0xf1,0x71,0xa7,0x53,0xb5,0x29,0x89,
    0xe5,0x2b,0xa7,0x17,0x29,0xe9,0x4f,0xc5,0x65,0x6d,0x6b,0xef,0x0d,0x89,0x49,0x2f,
    0xb3,0x43,0x53,0x65,0x1d,0x49,0xa3,0x13,0x89,0x59,0xef,0x6b,0xef,0x65,0x1d,0x0b,
    0x59,0x13,0xe3,0x4f,0x9d,0xb3,0x29,0x43,0x2b,0x07,0x1d,0x95,0x59,0x59,0x47,0xfb,
    0xe5,0xe9,0x61,0x47,0x2f,0x35,0x7f,0x17,0x7f,0xef,0x7f,0x95,0x95,0x71,0xd3,0xa3,
    0x0b,0x71,0xa3,0xad,0x0b,0x3b,0xb5,0xfb,0xa3,0xbf,0x4f,0x83,0x1d,0xad,0xe9,0x2f,
    0x71,0x65,0xa3,0xe5,0x07,0x35,0x3d,0x0d,0xb5,0xe9,0xe5,0x47,0x3b,0x9d,0xef,0x35,
    0xa3,0xbf,0xb3,0xdf,0x53,0xd3,0x97,0x53,0x49,0x71,0x07,0x35,0x61,0x71,0x2f,0x43,
    0x2f,0x11,0xdf,0x17,0x97,0xfb,0x95,0x3b,0x7f,0x6b,0xd3,0x25,0xbf,0xad,0xc7,0xc5,
    0xc5,0xb5,0x8b,0xef,0x2f,0xd3,0x07,0x6b,0x25,0x49,0x95,0x25,0x49,0x6d,0x71,0xc7 },
  { 0xa7,0xbc,0xc9,0xad,0x91,0xdf,0x85,0xe5,0xd4,0x78,0xd5,0x17,0x46,0x7c,0x29,0x4c,
    0x4d,0x03,0xe9,0x25,0x68,0x11,0x86,0xb3,0xbd,0xf7,0x6f,0x61,0x22,0xa2,0x26,0x34,
    0x2a,0xbe,0x1e,0x46,0x14,0x68,0x9d,0x44,0x18,0xc2,0x40,0xf4,0x7e,0x5f,0x1b,0xad,
    0x0b,0x94,0xb6,0x67,0xb4,0x0b,0xe1,0xea,0x95,0x9c,0x66,0xdc,0xe7,0x5d,0x6c,0x05,
    0xda,0xd5,0xdf,0x7a,0xef,0xf6,0xdb,0x1f,0x82,0x4c,0xc0,0x68,0x47,0xa1,0xbd,0xee,
    0x39,0x50,0x56,0x4a,0xdd,0xdf,0xa5,0xf8,0xc6,0xda,0xca,0x90,0xca,0x01,0x42,0x9d,
    0x8b,0x0c,0x73,0x43,0x75,0x05,0x94,0xde,0x24,0xb3,0x80,0x34,0xe5,0x2c,0xdc,0x9b,
    0x3f,0xca,0x33,0x45,0xd0,0xdb,0x5f,0xf5,0x52,0xc3,0x21,0xda,0xe2,0x22,0x72,0x6b,
    0x3e,0xd0,0x5b,0xa8,0x87,0x8c,0x06,0x5d,0x0f,0xdd,0x09,0x19,0x93,0xd0,0xb9,0xfc,
    0x8b,0x0f,0x84,0x60,0x33,0x1c,0x9b,0x45,0xf1,0xf0,0xa3,0x94,0x3a,0x12,0x77,0x33,
    0x4d,0x44,0x78,0x28,0x3c,0x9e,0xfd,0x65,0x57,0x16,0x94,0x6b,0xfb,0x59,0xd0,0xc8,
    0x22,0x36,0xdb,0xd2,0x63,0x98,0x43,0xa1,0x04,0x87,0x86,0xf7,0xa6,0x26,0xbb,0xd6,
    0x59,0x4d,0xbf,0x6a,0x2e,0xaa,0x2b,0xef,0xe6,0x78,0xb6,0x4e,0xe0,0x2f,0xdc,0x7c,
    0xbe,0x57,0x19,0x32,0x7e,0x2a,0xd0,0xb8,0xba,0x29,0x00,0x3c,0x52,0x7d,0xa8,0x49,
    0x3b,0x2d,0xeb,0x25,0x49,0xfa,0xa3,0xaa,0x39,0xa7,0xc5,0xa7,0x50,0x11,0x36,0xfb,
    0xc6,0x67,0x4a,0xf5,0xa5,0x12,0x65,0x7e,0xb0,0xdf,0xaf,0x4e,0xb3,0x61,0x7f,0x2f } };
  unsigned offset=0, entries, tag, type, len, save, c;
  unsigned ver97=0, serial=0, i, wbi=0, wb[4]={0,0,0,0};
  uchar buf97[324], ci, cj, ck;
  short sorder=m_order;
  char buf[10];
/*
   The MakerNote might have its own TIFF header (possibly with
   its own byte-order!), or it might just be a table.
 */
	fread(buf, 1, 10, m_ifp);
	if (!strncmp (buf,"FUJIFILM",8) ||
		!strncmp (buf,"SONY",4) ||
		!strcmp  (buf,"Panasonic")) {
		m_order = 0x4949;
		fseek(m_ifp,  2, SEEK_CUR);
	}
	else if (!strcmp (buf,"OLYMP") ||
			 !strcmp (buf,"LEICA") ||
			 !strcmp (buf,"Ricoh") ||
			 !strcmp (buf,"EPSON"))
		fseek (m_ifp, -2, SEEK_CUR);
	else
		fseek(m_ifp, -10, SEEK_CUR);

	entries = get2();
	if (entries > 1000) return;

	while (entries--) {
		tiff_get(base, &tag, &type, &len, &save);
		tag |= uptag << 16;
		if (tag == 4 && len > 26 && len < 35) {
			m_iso_speed = 50 * pow (2, (get4(),get2())/32.0 - 4);
			if ((i=(get2(),get2())) != 0x7fff)
				m_aperture = pow(2, i/64.0);
			if ((i=get2()) != 0xffff)
				m_shutter = pow (2, (short) i/-32.0);
			wbi = (get2(),get2());
		}
		if (tag == 0xc && len == 4) {
			m_cam_mul[0] = getreal(type);
			m_cam_mul[2] = getreal(type);
		}
		if (tag == 0x14 && len == 2560 && type == 7) {
			fseek(m_ifp, 1248, SEEK_CUR);
			goto get2_256;
		}
		if (tag == 0x15 && type == 2 && m_is_raw)
			fread (m_model, 64, 1, m_ifp);
		if (tag == 0x1b) tag = 0x1018;
		if (tag == 0x1c) tag = 0x1017;
		if (tag == 0x1d)
			while ((c = fgetc(m_ifp)) && c != EOF)
				serial = serial*10 + (isdigit(c) ? c - '0' : c % 10);
		if (tag == 0x81 && type == 4) {
			m_data_offset = get4();
			fseek (m_ifp, m_data_offset + 41, SEEK_SET);
			m_raw_height = get2() * 2;
			m_raw_width  = get2();
			m_filters = 0x61616161;
		}
		if (tag == 0x29 && type == 1) {
			c = wbi < 18 ? "012347800000005896"[wbi]-'0' : 0;
			fseek (m_ifp, 8 + c*32, SEEK_CUR);
			FORC4 m_cam_mul[c ^ (c >> 1) ^ 1] = get4();
		}
		if ((tag == 0x81  && type == 7) ||
			(tag == 0x100 && type == 7) ||
			(tag == 0x280 && type == 1)) {
			m_thumb_offset = ftell(m_ifp);
			m_thumb_length = len;
		}
		if (tag == 0x88 && type == 4 && (m_thumb_offset = get4()))
			m_thumb_offset += base;
		if (tag == 0x89 && type == 4)
			m_thumb_length = get4();
		if (tag == 0x97) {
			for (i=0; i < 4; i++)
				ver97 = (ver97 << 4) + fgetc(m_ifp)-'0';
			switch (ver97) {
			case 0x100:
				fseek (m_ifp, 68, SEEK_CUR);
				FORC4 m_cam_mul[(c >> 1) | ((c & 1) << 1)] = get2();
				break;
			case 0x102:
				fseek(m_ifp, 6, SEEK_CUR);
				goto get2_rggb;
			case 0x103:
				fseek(m_ifp, 16, SEEK_CUR);
				FORC4 m_cam_mul[c] = get2();
			}
			if (ver97 >> 8 == 2) {
				if (ver97 != 0x205)
					fseek (m_ifp, 280, SEEK_CUR);
				fread (buf97, 324, 1, m_ifp);
			}
		}
		if (tag == 0xa4 && type == 3) {
			fseek (m_ifp, wbi*48, SEEK_CUR);
			FORC3 m_cam_mul[c] = get2();
		}
		if (tag == 0xa7 && ver97 >> 8 == 2) {
			ci = xlat[0][serial & 0xff];
			cj = xlat[1][fgetc(m_ifp)^fgetc(m_ifp)^fgetc(m_ifp)^fgetc(m_ifp)];
			ck = 0x60;
			for (i=0; i < 324; i++)
				buf97[i] ^= (cj += ci * ck++);
			FORC4 m_cam_mul[c ^ (c >> 1)] = sget2(buf97 + (ver97 == 0x205 ? 14:6) + c*2);
			if (ver97 == 0x209)
				FORC4 m_cam_mul[c ^ (c >> 1) ^ 1] = sget2(buf97 + 10 + c*2);
		}
		if (tag == 0x200 && len == 4)
			m_black = (get2()+get2()+get2()+get2())/4;
		if (tag == 0x201 && len == 4)
			goto get2_rggb;
		if (tag == 0x401 && len == 4) {
			m_black = (get4()+get4()+get4()+get4())/4;
		}
		if (tag == 0xe80 && len == 256 && type == 7) {
			fseek (m_ifp, 48, SEEK_CUR);
			m_cam_mul[0] = get2() * 508 * 1.078 / 0x10000;
			m_cam_mul[2] = get2() * 382 * 1.173 / 0x10000;
		}
		if (tag == 0xf00 && type == 7) {
			if (len == 614)
				fseek (m_ifp, 176, SEEK_CUR);
			else if (len == 734 || len == 1502)
				fseek (m_ifp, 148, SEEK_CUR);
			else
				goto next;
			goto get2_256;
		}
		if ((tag == 0x1011 && len == 9) || tag == 0x20400200)
			for (i=0; i < 3; i++)
				FORC3 m_cmatrix[i][c] = ((short) get2()) / 256.0;
		if ((tag == 0x1012 || tag == 0x20400600) && len == 4)
			for (m_black = i=0; i < 4; i++)
				m_black += get2() << 2;
		if (tag == 0x1017 || tag == 0x20400100)
			m_cam_mul[0] = get2() / 256.0;
		if (tag == 0x1018 || tag == 0x20400100)
			m_cam_mul[2] = get2() / 256.0;
		if (tag == 0x2011 && len == 2) {
get2_256:
			m_order = 0x4d4d;
			m_cam_mul[0] = get2() / 256.0;
			m_cam_mul[2] = get2() / 256.0;
		}
		if (tag == 0x2020)
			parse_thumb_note (base, 257, 258);
		if (tag == 0x2040)
			parse_makernote (base, 0x2040);
		if (tag == 0xb028) {
			fseek (m_ifp, get4(), SEEK_SET);
			parse_thumb_note (base, 136, 137);
		}
		if (tag == 0x4001 && type == 3) {
			i = len == 582 ? 50 : len == 653 ? 68 : 126;
			fseek (m_ifp, i, SEEK_CUR);
get2_rggb:
			FORC4 m_cam_mul[c ^ (c >> 1)] = get2();
			fseek (m_ifp, 22, SEEK_CUR);
		}
next:
    	fseek (m_ifp, save, SEEK_SET);
	}
	m_order = sorder;
}

void dcraw::parse_exif(int base)
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
		case TAG_EXPOSURE_TIME:
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
		case TAG_SHUTTER_SPEED:
			if ((expo = -getreal(type)) < 128)
				m_shutter = pow(2, expo);
			break;
		case TAG_FLASH:
			m_flash_used = getint(type);
			break;
		case TAG_APERTURE:
			m_aperture = pow(2, getreal(type) / 2);
			break;
		case TAG_MAX_APERTURE:
			m_max_aperture = getreal(type);
			break;
		case TAG_FOCAL_LENGTH:
			m_focal_len = getreal(type);
			break;
		case TAG_METERING_MODE:
			m_metering_mode = getint(type);
			break;
		case TAG_EXPOSURE_BIAS:
			m_exposure_bias = (int)getreal(type);
			break;
		case TAG_WHITE_BALANCE:
			m_white_balance = getint(type);
			break;
		case 37500:
			parse_makernote(base, 0);
			break;
		}
		fseek(m_ifp, save, SEEK_SET);
	}
}

void dcraw::linear_table(unsigned len)
{
  int i;
  if (len > 0x1000) len = 0x1000;
  read_shorts (m_curve, len);
  for (i=len; i < 0x1000; i++)
    m_curve[i] = m_curve[i-1];
  m_maximum = m_curve[0xfff];
}

int dcraw::parse_tiff_ifd(int base)
{
  unsigned entries, tag, type, len, plen=16, save;
  int ifd, use_cm=0, cfa, i, j, c, ima_len=0;
  char software[64], *cbuf, *cp;
  uchar cfa_pat[16], cfa_pc[] = { 0,1,2,3 }, tab[256];
  double dblack, cc[4][4], cm[4][3], cam_xyz[4][3], num;
  double ab[]={ 1,1,1,1 }, asn[] = { 0,0,0,0 }, xyz[] = { 1,1,1 };
  unsigned sony_curve[] = { 0,0,0,0,0,4095 };
  struct jhead jh;

  if (m_tiff_nifds >= sizeof tiff_ifd / sizeof tiff_ifd[0])
    return 1;
  ifd = m_tiff_nifds++;
  for (j=0; j < 4; j++)
    for (i=0; i < 4; i++)
      cc[j][i] = i == j;
  entries = get2();
  if (entries > 512) return 1;
  while (entries--) {
    tiff_get (base, &tag, &type, &len, &save);
    switch (tag) {
// ---- Leica DIGILUX2 で画像が異常になる
//      case 17: case 18:
//	if (type == 3 && len == 1)
//	  m_cam_mul[(tag-17)*2] = get2() / 256.0;
//	break;
      case 23:
	if (type == 3) m_iso_speed = get2();		// leica specific ?
	break;
      case 36: case 37: case 38:
	m_cam_mul[tag-0x24] = get2();
	break;
      case 39:
	if (len < 50 || m_cam_mul[0]) break;
	fseek (m_ifp, 12, SEEK_CUR);
	FORC3 m_cam_mul[c] = get2();
	break;
      case 46:
	if (type != 7 || fgetc(m_ifp) != 0xff || fgetc(m_ifp) != 0xd8) break;
	m_thumb_offset = ftell(m_ifp) - 2;
	m_thumb_length = len;
	break;
      case 2: case 256:			/* ImageWidth */
	tiff_ifd[ifd].width = getint(type);
	break;
      case 3: case 257:			/* ImageHeight */
	tiff_ifd[ifd].height = getint(type);
	break;
      case 258:				/* BitsPerSample */
	tiff_ifd[ifd].samples = len & 7;
	tiff_ifd[ifd].bps = get2();
	break;
      case 259:				/* Compression */
	tiff_ifd[ifd].comp = get2();
	break;
      case 262:				/* PhotometricInterpretation */
	tiff_ifd[ifd].phint = get2();
	break;
      case 271:				/* Make */
	fgets (m_make, 64, m_ifp);
	break;
      case 272:				/* Model */
	fgets (m_model, 64, m_ifp);
	break;
      case 273:				/* StripOffset */
      case 513:
	tiff_ifd[ifd].offset = get4()+base;
	if (!tiff_ifd[ifd].bps) {
	  fseek (m_ifp, tiff_ifd[ifd].offset, SEEK_SET);
	  if (ljpeg_start (&jh, 1)) {
	    tiff_ifd[ifd].comp    = 6;
	    tiff_ifd[ifd].width   = jh.wide << (jh.clrs == 2);
	    tiff_ifd[ifd].height  = jh.high;
	    tiff_ifd[ifd].bps     = jh.bits;
	    tiff_ifd[ifd].samples = jh.clrs;
	  }
	}
	break;
      case 274:				/* Orientation */
	tiff_ifd[ifd].flip = "50132467"[get2() & 7]-'0';
	break;
      case 277:				/* SamplesPerPixel */
	tiff_ifd[ifd].samples = getint(type) & 7;
	break;
      case 279:				/* StripByteCounts */
      case 514:
	tiff_ifd[ifd].bytes = get4();
	break;
      case 305:				/* Software */
	fgets (software, 64, m_ifp);
	if (!strncmp(software,"Adobe",5) ||
	    !strncmp(software,"dcraw",5) ||
	    !strncmp(software,"Bibble",6) ||
	    !strncmp(software,"Nikon Scan",10) ||
	    !strcmp (software,"Digital Photo Professional"))
	  m_is_raw = 0;
	break;
      case 322:				/* TileWidth */
	m_tile_width = getint(type);
	break;
      case 323:				/* TileLength */
	m_tile_length = getint(type);
	break;
      case 324:				/* TileOffsets */
	tiff_ifd[ifd].offset = len > 1 ? ftell(m_ifp) : get4();
	break;
      case 330:				/* SubIFDs */
	while (len--) {
	  i = ftell(m_ifp);
	  fseek (m_ifp, get4()+base, SEEK_SET);
	  if (parse_tiff_ifd (base)) break;
	  fseek (m_ifp, i+4, SEEK_SET);
	}
	break;
      case 28688:
	FORC4 sony_curve[c+1] = get2() >> 2 & 0xfff;
	for (i=0; i < 5; i++)
	  for (j = sony_curve[i]+1; j <= sony_curve[i+1]; j++)
	    m_curve[j] = m_curve[j-1] + (1 << i);
	break;
      case 29443:
	FORC4 m_cam_mul[c ^ (c < 2)] = get2();
	break;
      case 33422:			/* CFAPattern */
      case 64777:			/* Kodak P-series */
	if ((plen=len) > 16) plen = 16;
	fread (cfa_pat, 1, plen, m_ifp);
	for (m_colors=cfa=i=0; i < plen; i++) {
	  m_colors += !(cfa & (1 << cfa_pat[i]));
	  cfa |= 1 << cfa_pat[i];
	}
	if (cfa == 070) memcpy (cfa_pc,"\003\004\005",3);	/* CMY */
	if (cfa == 072) memcpy (cfa_pc,"\005\003\004\001",4);	/* GMCY */
	goto guess_cfa_pc;
      case 33434:			/* ExposureTime */
	m_shutter = getreal(type);
	break;
      case 33437:			/* FNumber */
	m_aperture = getreal(type);
	break;
      case 34306:			/* Leaf white balance */
	FORC4 m_cam_mul[c ^ 1] = 4096.0 / get2();
	break;
      case 34307:			/* Leaf CatchLight color matrix */
	fread (software, 1, 7, m_ifp);
	if (strncmp(software,"MATRIX",6)) break;
	m_colors = 4;
	for (m_raw_color = i=0; i < 3; i++) {
	  FORC4 fscanf (m_ifp, "%f", &m_rgb_cam[i][c^1]);
	  if (!m_use_camera_wb) continue;
	  num = 0;
	  FORC4 num += m_rgb_cam[i][c];
	  FORC4 m_rgb_cam[i][c] /= num;
	}
	break;
      case 34665:			/* EXIF tag */
	fseek (m_ifp, get4()+base, SEEK_SET);
	parse_exif (base);
	break;
      case 37386:			/* FocalLength */
	m_focal_len = getreal(type);
	break;
      case 37400:			/* old Kodak KDC tag */
	for (m_raw_color = i=0; i < 3; i++) {
	  getreal(type);
	  FORC3 m_rgb_cam[i][c] = getreal(type);
	}
	break;
      case 50454:			/* Sinar tag */
      case 50455:
	if (!(cbuf = (char *) malloc(len))) break;
	fread (cbuf, 1, len, m_ifp);
	for (cp = cbuf-1; cp && cp < cbuf+len; cp = strchr(cp,'\n'))
	  if (!strncmp (++cp,"Neutral ",8))
	    sscanf (cp+8, "%f %f %f", m_cam_mul, m_cam_mul+1, m_cam_mul+2);
	free(cbuf);
	break;
      case 50459:			/* Hasselblad tag */
	i = m_order;
	j = ftell(m_ifp);
	c = m_tiff_nifds;
	m_order = get2();
	fseek (m_ifp, j+(get2(),get4()), SEEK_SET);
	parse_tiff_ifd (j);
	m_maximum = 0xffff;
	m_tiff_nifds = c;
	m_order = i;
	break;
      case 50706:			/* DNGVersion */
	FORC4 m_dng_version = (m_dng_version << 8) + fgetc(m_ifp);
	break;
      case 50710:			/* CFAPlaneColor */
	if (len > 4) len = 4;
	m_colors = len;
	fread (cfa_pc, 1, m_colors, m_ifp);
guess_cfa_pc:
	FORCC tab[cfa_pc[c]] = c;
	for (i=16; i--; )
	  m_filters = m_filters << 2 | tab[cfa_pat[i % plen]];
	break;
      case 50711:			/* CFALayout */
	if (get2() == 2) {
	  m_fuji_width = 1;
	  m_filters = 0x49494949;
	}
	break;
      case 291:
      case 50712:			/* LinearizationTable */
	linear_table (len);
	break;
      case 50714:			/* BlackLevel */
      case 50715:			/* BlackLevelDeltaH */
      case 50716:			/* BlackLevelDeltaV */
	for (dblack=i=0; i < len; i++)
	  dblack += getreal(type);
	m_black += dblack/len + 0.5;
	break;
      case 50717:			/* WhiteLevel */
	m_maximum = getint(type);
	break;
      case 50718:			/* DefaultScale */
	m_pixel_aspect  = getreal(type);
	m_pixel_aspect /= getreal(type);
	break;
      case 50721:			/* ColorMatrix1 */
      case 50722:			/* ColorMatrix2 */
	FORCC for (j=0; j < 3; j++)
	  cm[c][j] = getreal(type);
	use_cm = 1;
	break;
      case 50723:			/* CameraCalibration1 */
      case 50724:			/* CameraCalibration2 */
	for (i=0; i < m_colors; i++)
	  FORCC cc[i][c] = getreal(type);
      case 50727:			/* AnalogBalance */
	FORCC ab[c] = getreal(type);
	break;
      case 50728:			/* AsShotNeutral */
	FORCC asn[c] = getreal(type);
	break;
      case 50729:			/* AsShotWhiteXY */
	xyz[0] = getreal(type);
	xyz[1] = getreal(type);
	xyz[2] = 1 - xyz[0] - xyz[1];
	FORC3 xyz[c] /= d65_white[c];
	break;
      case 50752:
	read_shorts (m_cr2_slice, 3);
	break;
      case 50829:			/* ActiveArea */
	m_top_margin = getint(type);
	m_left_margin = getint(type);
	m_height = getint(type) - m_top_margin;
	m_width = getint(type) - m_left_margin;
	break;
      case 64772:			/* Kodak P-series */
	fseek (m_ifp, 16, SEEK_CUR);
	m_data_offset = get4();
	fseek (m_ifp, 28, SEEK_CUR);
	m_data_offset += get4();
	m_load_raw = &stp_packed_12_load_raw;
    }
    fseek (m_ifp, save, SEEK_SET);
  }
  for (i=0; i < m_colors; i++)
    FORCC cc[i][c] *= ab[i];
  if (use_cm) {
    FORCC for (i=0; i < 3; i++)
      for (cam_xyz[c][i]=j=0; j < m_colors; j++)
	cam_xyz[c][i] += cc[c][j] * cm[j][i] * xyz[i];
    cam_xyz_coeff (cam_xyz);
  }
  if (asn[0]) {
    m_cam_mul[3] = 0;
    FORCC m_cam_mul[c] = 1 / asn[c];
  }
  if (!use_cm)
    FORCC m_pre_mul[c] /= cc[c][c];
  return 0;
}

void dcraw::parse_tiff(int base)
{
  int doff, max_samp=0, raw=-1, thm=-1, i;
  struct jhead jh;

  fseek (m_ifp, base, SEEK_SET);
  m_order = get2();
  if (m_order != 0x4949 && m_order != 0x4d4d) return;
  get2();
  memset (tiff_ifd, 0, sizeof tiff_ifd);
  m_tiff_nifds = 0;
  while ((doff = get4())) {
    fseek (m_ifp, doff+base, SEEK_SET);
    if (parse_tiff_ifd (base)) break;
  }
  m_thumb_misc = 16;
  if (m_thumb_offset) {
    fseek (m_ifp, m_thumb_offset, SEEK_SET);
    if (ljpeg_start (&jh, 1)) {
      m_thumb_misc   = jh.bits;
      m_thumb_width  = jh.wide;
      m_thumb_height = jh.high;
    }
  }
  for (i=0; i < m_tiff_nifds; i++) {
    if (max_samp < tiff_ifd[i].samples)
	max_samp = tiff_ifd[i].samples;
    if (max_samp > 3) max_samp = 3;
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
  m_fuji_width *= (m_raw_width+1)/2;
  if (tiff_ifd[0].flip) m_tiff_flip = tiff_ifd[0].flip;
  if (raw >= 0 && !m_load_raw)
    switch (m_tiff_compress) {
      case 0:  case 1:
	switch (m_tiff_bps) {
	  case  8: m_load_raw = &stp_eight_bit_load_raw;	break;
	  case 12: m_load_raw = &stp_packed_12_load_raw;	break;
	  case 14:
	  case 16: m_load_raw = &stp_unpacked_load_raw;		break;
	}

	if (tiff_ifd[raw].bytes * 5 == m_raw_width * m_raw_height * 8)
	  m_load_raw = &stp_olympus_e300_load_raw;
	break;
      case 6:  case 7:  case 99:
	m_load_raw = &stp_lossless_jpeg_load_raw;		break;
      case 32769:
	m_load_raw = &stp_nikon_load_raw;			break;
      case 32773:
	m_load_raw = &stp_packed_12_load_raw;			break;
      default: m_is_raw = 0;
    }
  if (!m_dng_version && m_tiff_samples == 3)
    if (tiff_ifd[raw].bytes && m_tiff_bps != 14 && m_tiff_bps != 2048)
      m_is_raw = 0;
  if (!m_dng_version && m_tiff_bps == 8 && m_tiff_compress == 1 &&
	tiff_ifd[raw].phint == 1) m_is_raw = 0;
  for (i=0; i < m_tiff_nifds; i++)
    if (i != raw && tiff_ifd[i].samples == max_samp &&
	tiff_ifd[i].width * tiff_ifd[i].height / SQR(tiff_ifd[i].bps+1) >
	      m_thumb_width *       m_thumb_height / SQR(m_thumb_misc+1)) {
      m_thumb_width  = tiff_ifd[i].width;
      m_thumb_height = tiff_ifd[i].height;
      m_thumb_offset = tiff_ifd[i].offset;
      m_thumb_length = tiff_ifd[i].bytes;
      m_thumb_misc   = tiff_ifd[i].bps;
      thm = i;
    }
}

/*
   Thanks to Adobe for providing these excellent CAM -> XYZ matrices!
 */
void dcraw::adobe_coeff(char *m_make, char *m_model)
{
  static const struct {
    const char *prefix;
    short black, trans[12];
  } table[] = {
    { "EPSON R-D1", 0,
	{ 6827,-1878,-732,-8429,16012,2564,-704,592,7145 } },
    { "Mamiya ZD", 0,
	{ 7645,2579,-1363,-8689,16717,2015,-3712,5941,5961 } },
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

  sprintf (name, "%s %s", m_make, m_model);
  for (i=0; i < sizeof table / sizeof *table; i++)
    if (!strncmp (name, table[i].prefix, strlen(table[i].prefix))) {
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
void dcraw::identify()
{
  char head[32], *cp;
  unsigned hlen, fsize, i, c;
  struct jhead jh;
  static const char *corp[] =
    { "EPSON", "Mamiya" };

  m_tiff_flip = m_flip = m_filters = -1;	/* 0 is valid, so -1 is unknown */
  m_raw_height = m_raw_width = m_fuji_width = m_cr2_slice[0] = 0;
  m_maximum = m_height = m_width = m_top_margin = m_left_margin = 0;
  m_make[0] = m_model[0] = 0;
  m_iso_speed = m_shutter = m_aperture = m_focal_len = 0;
  memset(m_white, 0, sizeof m_white);
  m_thumb_offset = m_thumb_length = m_thumb_width = m_thumb_height = 0;
  m_load_raw = 0;
  m_data_offset = m_tiff_bps = m_tiff_compress = 0;
  m_zero_after_ff = m_dng_version = 0;
  m_tiff_samples = m_black = 0;
  m_mix_green = m_zero_is_bad = 0;
  m_pixel_aspect = m_is_raw = m_raw_color = 1;
  m_tile_width = m_tile_length = INT_MAX;
  for (i=0; i < 4; i++) {
    m_cam_mul[i] = i == 1;
    m_pre_mul[i] = i < 3;
    FORC3 m_cmatrix[c][i] = 0;
    FORC3 m_rgb_cam[c][i] = c == i;
  }
  m_colors = 3;
  m_tiff_bps = 12;
  for (i=0; i < 0x4000; i++) m_curve[i] = i;

  m_order = get2();
  hlen = get4();
  fseek (m_ifp, 0, SEEK_SET);
  fread (head, 1, 32, m_ifp);
  fseek (m_ifp, 0, SEEK_END);
  fsize = ftell(m_ifp);
  if (m_order == 0x4949 || m_order == 0x4d4d) {
      parse_tiff(0);
  }
  else
	 goto notraw;

  for (i=0; i < sizeof corp / sizeof *corp; i++)
    if (strstr (m_make, corp[i]))		/* Simplify company names */
	strcpy (m_make, corp[i]);
  cp = m_make + strlen(m_make);		/* Remove trailing spaces */
  while (*--cp == ' ') *cp = 0;
  cp = m_model + strlen(m_model);
  while (*--cp == ' ') *cp = 0;
  i = strlen(m_make);			/* Remove make from model */
  if (!StrCmpNIA(m_model, m_make, i) && m_model[i++] == ' ')
    memmove (m_model, m_model+i, 64-i);
  m_make[63] = m_model[63] = 0;
  if (!m_is_raw) goto notraw;

  if (!m_maximum) m_maximum = (1 << m_tiff_bps) - 1;
  if (!m_height) m_height = m_raw_height;
  if (!m_width)  m_width  = m_raw_width;
  if (m_fuji_width) {
    m_width = m_height + m_fuji_width;
    m_height = m_width - 1;
    m_pixel_aspect = 1;
  }
  if (m_dng_version) {
    if (m_filters == UINT_MAX) m_filters = 0;
    if (m_filters) m_is_raw = m_tiff_samples;
    else	 m_colors = m_tiff_samples;
    if (m_tiff_compress == 1)
      m_load_raw = &stp_adobe_dng_load_raw_nc;
    if (m_tiff_compress == 7)
      m_load_raw = &stp_adobe_dng_load_raw_lj;
    goto dng_skip;
  }

/* Set parameters based on camera name (for non-DNG files). */

  if (fsize == 6114240) {
    m_height = 1737;
    m_width  = 2324;
    m_raw_width = 3520;
    m_load_raw = &stp_packed_12_load_raw;
    m_maximum = 0xf7a;
    m_pre_mul[0] = 1.980;
    m_pre_mul[2] = 1.570;
  } else if (!strcmp(m_make,"Hasselblad")) {
    if (m_load_raw == &stp_lossless_jpeg_load_raw)
      m_load_raw = &stp_hasselblad_load_raw;
    if (m_raw_width == 7262) {
      m_height = 5444;
      m_width  = 7248;
      m_top_margin  = 4;
      m_left_margin = 7;
      m_filters = 0x61616161;
    }
  } else if (!strcmp(m_make,"LEICA") || !strcmp(m_make,"Panasonic")) {
    m_maximum = 0xfff0;
    if ((fsize-m_data_offset) / (m_width*8/7) == m_height)
      m_load_raw = &stp_panasonic_load_raw;
    if (!m_load_raw) m_load_raw = &stp_unpacked_load_raw;
    switch (m_width) {
      case 2568:
	adobe_coeff ("Panasonic","DMC-LC1");  break;
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
	adobe_coeff ("Panasonic","DMC-FZ8");  break;
      case 3177:
	m_width -= 10;
	m_filters = 0x49494949;
	m_maximum = 0xf7fc;
	m_zero_is_bad = 1;
	adobe_coeff ("Panasonic","DMC-L1");  break;
      case 3304:
	m_width -= 16;
	m_maximum = 0xf94c;
	m_zero_is_bad = 1;
	adobe_coeff ("Panasonic","DMC-FZ30");  break;
      case 3330:
	m_width = 3291;
	m_left_margin = 9;
	m_maximum = 0xf7f0;
	goto fz18;
      case 3370:
	m_width = 3288;
	m_left_margin = 15;
fz18:	if (m_height > 2480)
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
	adobe_coeff ("Panasonic","DMC-FZ50");  break;
      case 3710:
	m_width = 3682;
	m_filters = 0x49494949;
	break;
      case 3880:
	m_width -= 22;
	m_left_margin = 6;
	m_maximum = 0xf7f0;
	m_zero_is_bad = 1;
	adobe_coeff ("Panasonic","DMC-LX1");  break;
      case 4290:
	m_height += 38;
	m_left_margin = -14;
	m_filters = 0x49494949;
      case 4330:
	m_width = 4248;
	if ((m_height -= 39) == 2400)
	  m_top_margin = 15;
	m_left_margin += 17;
	adobe_coeff ("Panasonic","DMC-LX2");  break;
    }
  }
  if (!m_model[0])
    sprintf (m_model, "%dx%d", m_width, m_height);
  if (m_filters == UINT_MAX) m_filters = 0x94949494;
  if (m_raw_color) adobe_coeff (m_make, m_model);
  if (m_thumb_offset && !m_thumb_height) {
    fseek (m_ifp, m_thumb_offset, SEEK_SET);
    if (ljpeg_start (&jh, 1)) {
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

void dcraw::convert_to_rgb()
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
  static const double (*out_rgb[])[3] =
  { rgb_rgb, adobe_rgb, wide_rgb, prophoto_rgb, xyz_rgb };
  static const char *name[] =
  { "sRGB", "Adobe RGB (1998)", "WideGamut D65", "ProPhoto D65", "XYZ" };
  static const unsigned phead[] =
  { 1024, 0, 0x2100000, 0x6d6e7472, 0x52474220, 0x58595a20, 0, 0, 0,
    0x61637370, 0, 0, 0x6e6f6e65, 0, 0, 0, 0, 0xf6d6, 0x10000, 0xd32d };
  unsigned pbody[] =
  { 10, 0x63707274, 0, 36,	/* cprt */
	0x64657363, 0, 40,	/* desc */
	0x77747074, 0, 20,	/* wtpt */
	0x626b7074, 0, 20,	/* bkpt */
	0x72545243, 0, 14,	/* rTRC */
	0x67545243, 0, 14,	/* gTRC */
	0x62545243, 0, 14,	/* bTRC */
	0x7258595a, 0, 20,	/* rXYZ */
	0x6758595a, 0, 20,	/* gXYZ */
	0x6258595a, 0, 20 };	/* bXYZ */
  static const unsigned pwhite[] = { 0xf351, 0x10000, 0x116cc };
  unsigned pcurve[] = { 0x63757276, 0, 1, 0x1000000 };

  memcpy (out_cam, m_rgb_cam, sizeof out_cam);
  m_raw_color |= m_colors == 1 || m_colorspace < 1 || m_colorspace > 5;
  if (!m_raw_color) {
    m_oprof = new unsigned[phead[0]];
    memcpy (m_oprof, phead, sizeof phead);
    if (m_colorspace == 5) m_oprof[4] = m_oprof[5];
    m_oprof[0] = 132 + 12*pbody[0];
    for (i=0; i < pbody[0]; i++) {
      m_oprof[m_oprof[0]/4] = i ? (i > 1 ? 0x58595a20 : 0x64657363) : 0x74657874;
      pbody[i*3+2] = m_oprof[0];
      m_oprof[0] += (pbody[i*3+3] + 3) & -4;
    }
    memcpy (m_oprof+32, pbody, sizeof pbody);
    m_oprof[pbody[5]/4+2] = strlen(name[m_colorspace-1]) + 1;
    memcpy ((char *)m_oprof+pbody[8]+8, pwhite, sizeof pwhite);
#ifdef SRGB_GAMMA
    pcurve[3] = 0x2330000;
#else
    pcurve[3] = 0x1f00000;
#endif
    for (i=4; i < 7; i++)
      memcpy ((char *)m_oprof+pbody[i*3+2], pcurve, sizeof pcurve);
    pseudoinverse ((double (*)[3]) out_rgb[m_colorspace-1], inverse, 3);
    for (i=0; i < 3; i++)
      for (j=0; j < 3; j++) {
	for (num = k=0; k < 3; k++)
	  num += xyzd50_srgb[i][k] * inverse[j][k];
        m_oprof[pbody[j*3+23]/4+i+2] = num * 0x10000 + 0.5;
      }
    for (i=0; i < phead[0]/4; i++)
      m_oprof[i] = htonl(m_oprof[i]);
    strcpy ((char *)m_oprof+pbody[2]+8, "auto-generated by dcraw");
    strcpy ((char *)m_oprof+pbody[5]+12, name[m_colorspace-1]);
    for (i=0; i < 3; i++)
      for (j=0; j < m_colors; j++)
	for (out_cam[i][j] = k=0; k < 3; k++)
	  out_cam[i][j] += out_rgb[m_colorspace-1][i][k] * m_rgb_cam[k][j];
  }

  memset(m_histogram, 0, sizeof m_histogram);
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
  if (m_colors == 4 && m_colorspace) m_colors = 3;
}

void dcraw::fuji_rotate()
{
  int i, row, col;
  double step;
  float r, c, fr, fc;
  unsigned ur, uc;
  ushort wide, high, (*img)[4], (*pix)[4];

  if (!m_fuji_width) return;
  m_fuji_width = (m_fuji_width - 1 + m_shrink) >> m_shrink;
  step = sqrt(0.5);
  wide = m_fuji_width / step;
  high = (m_height - m_fuji_width) / step;
  img = new ushort[wide*high][4];

  for (row=0; row < high; row++)
    for (col=0; col < wide; col++) {
      ur = r = m_fuji_width + (row-col)*step;
      uc = c = (row+col)*step;
      if (ur > m_height-2 || uc > m_width-2) continue;
      fr = r - ur;
      fc = c - uc;
      pix = m_image + ur*m_width + uc;
      for (i=0; i < m_colors; i++)
	img[row*wide+col][i] =
	  (pix[    0][i]*(1-fc) + pix[      1][i]*fc) * (1-fr) +
	  (pix[m_width][i]*(1-fc) + pix[m_width+1][i]*fc) * fr;
    }
  delete [] m_image;
  m_width  = wide;
  m_height = high;
  m_image  = img;
  m_fuji_width = 0;
}

void dcraw::stretch()
{
  ushort newdim, (*img)[4], *pix0, *pix1;
  int row, col, c;
  double rc, frac;

  if (m_pixel_aspect == 1) return;
  if (m_pixel_aspect < 1) {
    newdim = m_height / m_pixel_aspect + 0.5;
    img = new ushort[m_width*newdim][4];
    for (rc=row=0; row < newdim; row++, rc+=m_pixel_aspect) {
      frac = rc - (c = rc);
      pix0 = pix1 = m_image[c*m_width];
      if (c+1 < m_height) pix1 += m_width*4;
      for (col=0; col < m_width; col++, pix0+=4, pix1+=4)
	FORCC img[row*m_width+col][c] = pix0[c]*(1-frac) + pix1[c]*frac + 0.5;
    }
    m_height = newdim;
  } else {
    newdim = m_width * m_pixel_aspect + 0.5;
    img = new ushort[m_height*newdim][4];
    for (rc=col=0; col < newdim; col++, rc+=1/m_pixel_aspect) {
      frac = rc - (c = rc);
      pix0 = pix1 = m_image[c];
      if (c+1 < m_width) pix1 += 4;
      for (row=0; row < m_height; row++, pix0+=m_width*4, pix1+=m_width*4)
	FORCC img[row*newdim+col][c] = pix0[c]*(1-frac) + pix1[c]*frac + 0.5;
    }
    m_width = newdim;
  }
  delete [] m_image;
  m_image = img;
}

int dcraw::flip_index(int row, int col)
{
  if (m_flip & 4) SWAP(row,col);
  if (m_flip & 2) row = m_iheight - 1 - row;
  if (m_flip & 1) col = m_iwidth  - 1 - col;
  return row * m_iwidth + col;
}

void dcraw::gamma_lut(uchar lut[0x10000])
{
  int perc, c, val, total, i;
  float white=0, r;

  perc = m_width * m_height * 0.01;		/* 99th percentile white level */
  if (m_fuji_width) perc /= 2;
  if ((m_highlight & ~2) || m_no_auto_bright) perc = -1;
  FORCC {
    for (val=0x2000, total=0; --val > 32; )
      if ((total += m_histogram[c][val]) > perc) break;
    if (white < val) white = val;
  }
  white *= 8 / m_bright;
  for (i=0; i < 0x10000; i++) {
    r = i / white;
    val = 256 * (
#ifdef SRGB_GAMMA
	r <= 0.00304 ? r*12.92 : powf(r,2.5/6)*1.055-0.055 );
#else
	r <= 0.018 ? r*4.5 : powf(r,0.45)*1.099-0.099 );
#endif
    if (val > 255) val = 255;
    lut[i] = val;
  }
}

void dcraw::write_dib(FILE *ofp)
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

void dcraw::write_dib_stream(IStream* pstrm)
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

	// ガンマLUT 初期化
	uchar* lut = new uchar[0x10000];
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

int dcraw::develop(LPCTSTR pszName, IStream* pstrm, int scale)
{
	int i, status = 1;
	m_ifname = pszName;

	m_half_size = scale;
	if (m_half_size > 0)
		m_four_color_rgb = 1;
	m_use_camera_wb = 1;
	if (m_use_camera_matrix < 0)
		m_use_camera_matrix = m_use_camera_wb;

    m_image = 0;
    m_oprof = 0;
    if (!(m_ifp = _tfopen(m_ifname, _T("rb"))))
		return 0;

	identify();
	if (!m_is_raw) {
		status = 0;
		goto cleanup;
	}
	switch ((m_flip+3600) % 360) {
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
	m_image = new ushort[m_iheight*m_iwidth][4];
	fseek(m_ifp, m_data_offset, SEEK_SET);
	(*m_load_raw)(this);
	if (m_zero_is_bad)
		remove_zeroes();
	scale_colors();
	pre_interpolate();
	if (m_filters)
		ahd_interpolate();
	if (m_mix_green)
		for (m_colors=3, i=0; i < m_height*m_width; i++)
			m_image[i][1] = (m_image[i][1] + m_image[i][3]) >> 1;

	if (m_highlight == 2) blend_highlights();
	if (m_highlight > 2) recover_highlights();

	fuji_rotate();
	convert_to_rgb();
	stretch();

	write_dib_stream(pstrm);
    fclose(m_ifp);

cleanup:
    if (m_oprof) delete [] m_oprof;
    if (m_image) delete [] m_image;
	return status;
}

////////////////////////////////////////////////////////////////////////
//

BOOL DCRaw(LPCTSTR fname, IStream* pstrm, BOOL bLargeImage, EXIFINFOSTRUCT* ei)
{
	dcraw raw;
	BOOL ok = raw.develop(fname, pstrm, bLargeImage ? 1 : 3);
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
