////////////////////////////////////////////////////////////////////////
// psdcode.cpp - Converts from/to Adobe PSD image

#include "stdafx.h"
#include "filemap.h"
#include "dib_sup.h"
#include "sg_sup.h"

#define PSD_BITMAP			0
#define PSD_GRAYSCALE		1
#define PSD_INDEX			2
#define PSD_RGB				3
#define PSD_CMYK			4
#define PSD_MULTICHANNEL	7
#define PSD_DUOTONE			8
#define PSD_LAB				9

#define RANDOM_TABLE_SIZE	4096
#define RANDOM_SEED			314159265
#define INT_MULT(a,b,t)		((t) = (a) * (b) + 0x80, ((((t) >> 8) + (t)) >> 8))
#define MAX(a,b)			((a)<(b)?(b):(a))
#define MIN(a,b)			((a)<(b)?(a):(b))

#pragma pack(1)

static char* sszBlends[] = {
	"norm",						// 0 = 通常
	"dark",						// 1 = 比較（暗）
	"lite",						// 2 = 比較（明）
	"hue ",						// 3 = 色相
	"sat ",						// 4 = 彩度
	"colr",						// 5 = カラー
	"lum ",						// 6 = 輝度
	"mul ",						// 7 = 乗算
	"scrn",						// 8 = スクリーン
	"diss",						// 9 = ディザ合成
	"over",						// 10 = オーバーレイ
	"hLit",						// 11 = ハードライト
	"sLit",						// 12 = ソフトライト
	"diff",						// 13 = 差の絶対値
	"smud",						// 14 = 除外
	"div ",						// 15 = 覆い焼きカラー
	"idiv",						// 16 = 焼き込みカラー
	"",
};

typedef struct _THUMBINFO
{
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

typedef struct _CHANNELINFO
{
	SHORT type;
	ULONG size;
} CHANNELINFO;

typedef struct _LAYERINFO
{
	UINT width;
	UINT height;
	int x;
	int y;
	USHORT channels;
	CHANNELINFO channel_info[24];
	char blend[4];
	int blendtype;
	BYTE opacity;
	BYTE clipping;
	BYTE flags;
	SGIMAGE* image;
} LAYERINFO;

typedef struct _PSDHEADER				// PSD v3.0 HEADER
{
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

_inline int do_mix(int src, int dst, int opaque)
{
	if (opaque == 0) return src;
	if (opaque == 255) return dst;
	return ADJRGB((src * (255 - opaque) + dst * opaque) / 255);
}

class waitcursor
{
public:
	waitcursor() { m_hcur = SetCursor(LoadCursor(NULL, IDC_WAIT)); }
	~waitcursor() { SetCursor(m_hcur); }
	HCURSOR m_hcur;
};

void msb1stwritelong(HANDLE hf, ULONG value)
{
	BYTE buf[4];
	DWORD dw;
	buf[0] = (BYTE)((value >> 24) & 0xFF);
	buf[1] = (BYTE)((value >> 16) & 0xFF);
	buf[2] = (BYTE)((value >>  8) & 0xFF);
	buf[3] = (BYTE)( value        & 0xFF);
	WriteFile(hf, buf, 4, &dw, NULL);
}

void msb1stwriteshort(HANDLE hf, UINT value)
{
	BYTE buf[2];
	DWORD dw;

	buf[0] = (BYTE)((value >> 8) & 0xFF);
	buf[1] = (BYTE)( value       & 0xFF);
	WriteFile(hf, buf, 2, &dw, NULL);
}

/* @@@

	RGBtoCMY
		Cyan = 1 - red
		Magenta = 1 - Green
		Yellow = 1 - Blue

	CMYtoCMYK
		Black = min(Cyan, Magemta, Yellow)
		Cyan = (Cyan - Black) / (1 - Balck)
		Magenta = (Magenta - Black) / (1 - Black)
		Yellow = (Yellow - Balck) / (1 - Black)

	CMYKtoCMY
		Cyan = min(1, Cyanm * (1 - Balck) + Black)
		Magenta = min(1, Magenta * (1 - Back) + Black)
		Yellow = min(1, Yellow * (1 - Balck) + Black)

	CMYtoRGB
		Red = 1 - Cyan
		Green = 1 - Magenta
		Blue = 1 - Yellow

@@@ */

//////////////////////////////////////////////////////////////////////
// transform_pixel

static int __fastcall hls_value(double n1, double n2, double hue)
{
  double value;

  if (hue > 255)
    hue -= 255;
  else if (hue < 0)
    hue += 255;
  if (hue < 42.5)
    value = n1 + (n2 - n1) * (hue / 42.5);
  else if (hue < 127.5)
    value = n2;
  else if (hue < 170)
    value = n1 + (n2 - n1) * ((170 - hue) / 42.5);
  else
    value = n1;

  return (int)(value * 255);
}

void __fastcall rgb_to_hls(int *r, int *g, int *b)
{
	int red, green, blue;
	double h, l, s;
	int min, max;
	int delta;

	red = *r;
	green = *g;
	blue = *b;

	if (red > green)
	{
		if (red > blue)
			max = red;
		else
			max = blue;

		if (green < blue)
			min = green;
		else
			min = blue;
	}
	else
	{
		if (green > blue)
			max = green;
		else
			max = blue;

		if (red < blue)
			min = red;
		else
			min = blue;
	}
	l = (max + min) / 2.0;

	if (max == min)
	{
		s = 0.0;
		h = 0.0;
	}
	else
	{
		delta = (max - min);

		if (l < 128)
			s = 255 * (double)delta / (double)(max + min);
		else
			s = 255 * (double)delta / (double)(511 - max - min);

		if (red == max)
			h = (green - blue) / (double) delta;
		else if (green == max)
			h = 2 + (blue - red) / (double) delta;
		else
			h = 4 + (red - green) / (double) delta;

		h = h * 42.5;
		if (h < 0)
			h += 255;
		else if (h > 255)
			h -= 255;
	}
	*r = (int)h;
	*g = (int)l;
	*b = (int)s;
}

void __fastcall hls_to_rgb (int *h, int *l, int *s)
{
  double hue, lightness, saturation;
  double m1, m2;

  hue = *h;
  lightness = *l;
  saturation = *s;

  if (saturation == 0)
    {
      /*  achromatic case  */
      *h = (int)lightness;
      *l = (int)lightness;
      *s = (int)lightness;
    }
  else
    {
      if (lightness < 128)
	m2 = (lightness * (255 + saturation)) / 65025.0;
      else
	m2 = (lightness + saturation - (lightness * saturation)/255.0) / 255.0;

      m1 = (lightness / 127.5) - m2;

      /*  chromatic case  */
      *h = hls_value (m1, m2, hue + 85);
      *l = hls_value (m1, m2, hue);
      *s = hls_value (m1, m2, hue - 85);
    }
}

void __fastcall rgb_to_hsv(BYTE r, BYTE g, BYTE b, double* hue, double* sat, double* val)
{
	double red   = (double)r / 255.0;
	double green = (double)g / 255.0;
	double blue  = (double)b / 255.0;

	double vmin, vmax;
	double h = 0.0;		/* Shut up -Wall */
	if (red > green)
	{
		vmax = MAX (red,   blue);
		vmin = MIN (green, blue);
	}
	else
	{
		vmax = MAX (green, blue);
		vmin = MIN (red,   blue);
	}

	double s, v = vmax;
	if (vmax != 0.0)
		s = (vmax - vmin) / vmax;
	else
		s = 0.0;

	if (s == 0.0)
		h = 0.0;
	else
	{
		double delta = vmax - vmin;
		if (delta == 0.0)
			delta = 1.0;

		if (red == vmax)
			h = (green - blue) / delta;
		else if (green == vmax)
			h = 2 + (blue - red) / delta;
		else if (blue == vmax)
			h = 4 + (red - green) / delta;

		h /= 6.0;
		if (h < 0.0)
			h += 1.0;
		else if (h > 1.0)
			h -= 1.0;
	}

	*hue = h;
	*sat = s;
	*val = v;
}

void __fastcall hsv_to_rgb(BYTE* pr, BYTE* pb, BYTE* pg, double hue, double sat, double val)
{
	if (sat == 0.0)
	{
		hue = val;
		sat = val;
		val = val;
	}
	else
	{
		double h = hue * 6.0;
		double s = sat;
		double v = val;
		if (h == 6.0)
			h = 0.0;

		double f = h - (int)h;
		double p = v * (1.0 - s);
		double q = v * (1.0 - s * f);
		double t = v * (1.0 - s * (1.0 - f));

		switch ((int)h)
		{
			case 0:
				hue = v;
				sat = t;
				val = p;
				break;

			case 1:
				hue = q;
				sat = v;
				val = p;
				break;

			case 2:
				hue = p;
				sat = v;
				val = t;
				break;

			case 3:
				hue = p;
				sat = q;
				val = v;
				break;

			case 4:
				hue = t;
				sat = p;
				val = v;
				break;

			case 5:
				hue = v;
				sat = p;
				val = q;
				break;
		}
	}

	*pr = ADJRGB((int)(hue * 255));
	*pb = ADJRGB((int)(sat * 255));
	*pg = ADJRGB((int)(val * 255));
}

/////////////////////////////////////////////////////////////////////
//

PIXELPACKET __fastcall destcopy_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix = *q;
	return pix;
}

#define MIXRGB(a,b,c) (((int)(a)*(255-(c))+(int)(b)*(c))/255)

PIXELPACKET __fastcall blend_pixel(PIXELPACKET* p, PIXELPACKET* q, int blend)
{
	PIXELPACKET pix;
	pix.red = MIXRGB(p->red, q->red, blend);
	pix.green = MIXRGB(p->green, q->green, blend);
	pix.blue = MIXRGB(p->blue, q->blue, blend);
	return pix;
}

void __fastcall blend_to(PIXELPACKET* p, PIXELPACKET* q, int blend)
{
	p->red = MIXRGB(p->red, q->red, blend);
	p->green = MIXRGB(p->green, q->green, blend);
	p->blue = MIXRGB(p->blue, q->blue, blend);
}

PIXELPACKET __fastcall darken_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix;
	pix.red = min(p->red, q->red);
	pix.green = min(p->green, q->green);
	pix.blue = min(p->blue, q->blue);
	pix.alpha = min(p->alpha, q->alpha);
	return pix;
}

PIXELPACKET __fastcall lighten_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix;
	pix.red = max(p->red, q->red);
	pix.green = max(p->green, q->green);
	pix.blue = max(p->blue, q->blue);
	pix.alpha = max(p->alpha, q->alpha);
	return pix;
}

PIXELPACKET __fastcall hsv_hue_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix;
	double h1, s1, v1, h2, s2, v2;

	rgb_to_hsv(p->red, p->green, p->blue, &h1, &s1, &v1);
	rgb_to_hsv(q->red, q->green, q->blue, &h2, &s2, &v2);

	h1 = h2;
	hsv_to_rgb(&pix.red, &pix.green, &pix.blue, h1, s1, v1);
	return pix;
}

PIXELPACKET __fastcall hsv_sat_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix;
	double h1, s1, v1, h2, s2, v2;

	rgb_to_hsv(p->red, p->green, p->blue, &h1, &s1, &v1);
	rgb_to_hsv(q->red, q->green, q->blue, &h2, &s2, &v2);

	s1 = s2;
	hsv_to_rgb(&pix.red, &pix.green, &pix.blue, h1, s1, v1);
	return pix;
}

PIXELPACKET __fastcall hsv_lum_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix;
	double h1, s1, v1, h2, s2, v2;

	rgb_to_hsv(p->red, p->green, p->blue, &h1, &s1, &v1);
	rgb_to_hsv(q->red, q->green, q->blue, &h2, &s2, &v2);

	v1 = v2;
	hsv_to_rgb(&pix.red, &pix.green, &pix.blue, h1, s1, v1);
	return pix;
}

PIXELPACKET __fastcall color_only_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix;
	int r1, g1, b1;
	int r2, g2, b2;

	r1 = p->red; g1 = p->green; b1 = p->blue;
	r2 = q->red; g2 = q->green; b2 = q->blue;
	rgb_to_hls(&r1, &g1, &b1);
	rgb_to_hls(&r2, &g2, &b2);

	r1 = r2;
	b1 = b2;
	hls_to_rgb(&r1, &g1, &b1);
	pix.red = r1; pix.green = g1; pix.blue = b1;
	return pix;
}

PIXELPACKET __fastcall multiply_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix;
	int tmp;
	pix.red = INT_MULT(p->red, q->red, tmp);
	pix.green = INT_MULT(p->green, q->green, tmp);
	pix.blue = INT_MULT(p->blue, q->blue, tmp);
	return pix;
}

PIXELPACKET __fastcall divide_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix;
	pix.red = ADJRGB(((int)p->red << 8) / (1 + (int)q->red));
	pix.green = ADJRGB(((int)p->green << 8) / (1 + (int)q->green));
	pix.blue = ADJRGB(((int)p->blue << 8) / (1 + (int)q->blue));
	return pix;
}

PIXELPACKET __fastcall screen_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix;
	int tmp;
	pix.red = 255 - INT_MULT((255 - p->red), (255 - q->red), tmp);
	pix.green = 255 - INT_MULT((255 - p->green), (255 - q->green), tmp);
	pix.blue = 255 - INT_MULT((255 - p->blue), (255 - q->blue), tmp);
	return pix;
}

PIXELPACKET __fastcall overlay_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
/* @@@
	PIXELPACKET pix;
	int tmp;
	pix.red = INT_MULT(p->red, p->red + 
					INT_MULT((int)q->red << 1, 255 - p->red, tmp), tmp);
	pix.green = INT_MULT(p->green, p->green + 
					INT_MULT((int)q->green << 1, 255 - p->green, tmp), tmp);
	pix.blue = INT_MULT(p->blue, p->blue + 
					INT_MULT((int)q->blue << 1, 255 - p->blue, tmp), tmp);
	return pix;
@@@ */

	PIXELPACKET pix;
	double a, b, c;
	int n;

	a = (double)p->red / 255.0;
	b = (double)q->red / 255.0;
	c = a < 0.5 ? 2.0 * a * b : 1.0 - 2.0 * (1 - a) * (1 - b);
	n = (int)(c * 255.0);
	pix.red = ADJRGB(n);

	a = (double)p->green / 255.0;
	b = (double)q->green / 255.0;
	c = a < 0.5 ? 2.0 * a * b : 1.0 - 2.0 * (1 - a) * (1 - b);
	n = (int)(c * 255.0);
	pix.green = ADJRGB(n);

	a = (double)p->blue / 255.0;
	b = (double)q->blue / 255.0;
	c = a < 0.5 ? 2.0 * a * b : 1.0 - 2.0 * (1 - a) * (1 - b);
	n = (int)(c * 255.0);
	pix.blue = ADJRGB(n);
	return pix;
}

PIXELPACKET __fastcall add_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix;
	pix.red = ADJRGB((int)p->red + (int)q->red);
	pix.green = ADJRGB((int)p->green + (int)q->blue);
	pix.blue = ADJRGB((int)p->blue + (int)q->blue);
	return pix;
}

PIXELPACKET __fastcall del_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix;
	pix.red = ADJRGB((int)p->red - (int)q->red);
	pix.green = ADJRGB((int)p->green - (int)q->blue);
	pix.blue = ADJRGB((int)p->blue - (int)q->blue);
	return pix;
}

PIXELPACKET __fastcall idiv_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix;
	pix.red = ADJRGB(p->red - (255 - q->red));
	pix.green = ADJRGB(p->green - (255 - q->green));
	pix.blue = ADJRGB(p->blue - (255 - q->blue));
	return pix;
}

PIXELPACKET __fastcall subtract_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix;
	pix.red = ADJRGB((int)p->red - (int)q->red);
	pix.green = ADJRGB((int)p->green - (int)q->green);
	pix.blue = ADJRGB((int)p->blue - (int)q->blue);
	return pix;
}

PIXELPACKET __fastcall difference_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix;
	int diff;
	diff = (int)p->red - (int)q->red;
	pix.red = (diff < 0) ? -diff : diff;
	diff = (int)p->green - (int)q->green;
	pix.green = (diff < 0) ? -diff : diff;
	diff = (int)p->blue - (int)q->blue;
	pix.blue = (diff < 0) ? -diff : diff;
	return pix;
}

PIXELPACKET __fastcall average_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix;
	pix.red = ((int)p->red + (int)q->red) >> 1;
	pix.green = ((int)p->green + (int)q->blue) >> 1;
	pix.blue = ((int)p->blue + (int)q->blue) >> 1;
	return pix;
}

#define HARDWRITE(x,y)	((y)<128?(2*(x)*(y)/255):255-2*(255-(x))*(255-(y))/255)
#define SMP(x)			(((x)<<6)/255)
#define SOFTWRITE(x,y)	((y)<128?2*SMP((x))*(y)/255:255-(2*(255-SMP((x)))*(255-(y))/255))

PIXELPACKET __fastcall hardlight_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
/* @@@
	PIXELPACKET pix;
	pix.red = HARDWRITE((int)p->red, (int)q->red);
	pix.green = HARDWRITE((int)p->green, (int)q->green);
	pix.blue = HARDWRITE((int)p->blue, (int)q->blue);
	return pix;
@@@ */

	PIXELPACKET pix;
	double a, b, c;
	int n;

	a = (double)p->red / 255.0;
	b = (double)q->red / 255.0;
	c = b < 0.5 ? 2 * a * b : 1 - 2 * ( 1 - a ) * ( 1 - b );
	n = (int)(c * 255.0);
	pix.red = ADJRGB(n);

	a = (double)p->green / 255.0;
	b = (double)q->green / 255.0;
	c = b < 0.5 ? 2 * a * b : 1 - 2 * ( 1 - a ) * ( 1 - b );
	n = (int)(c * 255.0);
	pix.green = ADJRGB(n);

	a = (double)p->blue / 255.0;
	b = (double)q->blue / 255.0;
	c = b < 0.5 ? 2 * a * b : 1 - 2 * ( 1 - a ) * ( 1 - b );
	n = (int)(c * 255.0);
	pix.blue = ADJRGB(n);
	return pix;
}

PIXELPACKET __fastcall softlight_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
/* @@@
	PIXELPACKET pix;
	pix.red = SOFTWRITE((int)p->red, (int)q->red);
	pix.green = SOFTWRITE((int)p->green, (int)q->green);
	pix.blue = SOFTWRITE((int)p->blue, (int)q->blue);
	return pix;
@@@ */

	PIXELPACKET pix;
	double a, b, c;
	int n;

	a = (double)p->red / 255.0;
	b = (double)q->red / 255.0;
	c = b < 0.5 ? a + ( a - pow(a, 2) ) * ( 2 * b - 1 ) :
		a <= 32/255 ? a + ( a - pow(a, 2) ) * ( 2 * b - 1 ) * ( 3 - 8 * a ) : a + ( pow(a, 0.5) - a ) * ( 2 * b - 1 );
	n = (int)(c * 255.0);
	pix.red = ADJRGB(n);

	a = (double)p->green / 255.0;
	b = (double)q->green / 255.0;
	c = b < 0.5 ? a + ( a - pow(a, 2) ) * ( 2 * b - 1 ) :
		a <= 32/255 ? a + ( a - pow(a, 2) ) * ( 2 * b - 1 ) * ( 3 - 8 * a ) : a + ( pow(a, 0.5) - a ) * ( 2 * b - 1 );
	n = (int)(c * 255.0);
	pix.green = ADJRGB(n);

	a = (double)p->blue / 255.0;
	b = (double)q->blue / 255.0;
	c = b < 0.5 ? a + ( a - pow(a, 2) ) * ( 2 * b - 1 ) :
		a <= 32/255 ? a + ( a - pow(a, 2) ) * ( 2 * b - 1 ) * ( 3 - 8 * a ) : a + ( pow(a, 0.5) - a ) * ( 2 * b - 1 );
	n = (int)(c * 255.0);
	pix.blue = ADJRGB(n);
	return pix;
}

PIXELPACKET __fastcall dodge_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix;
	double a, b, c;
	int n;

	a = (double)p->red / 255.0;
	b = (double)q->red / 255.0;
	c = 256.0 / (256.0 - 255.0 * a) * b;
	n = (int)(c * 255.0);
	pix.red = ADJRGB(n);

	a = (double)p->green / 255.0;
	b = (double)q->green / 255.0;
	c = 256.0 / (256.0 - 255.0 * a) * b;
	n = (int)(c * 255.0);
	pix.green = ADJRGB(n);

	a = (double)p->blue / 255.0;
	b = (double)q->blue / 255.0;
	c = 256.0 / (256.0 - 255.0 * a) * b;
	n = (int)(c * 255.0);
	pix.blue = ADJRGB(n);
	return pix;
}

PIXELPACKET __fastcall burn_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix;
	double a, b, c;
	int n;

	a = (double)p->red / 255.0;
	b = (double)q->red / 255.0;
	c = b + a < 1.0 ? 0 : b != 0 ? 1.0 - (1.0 - a) / b : 1.0;
	n = (int)(c * 255.0);
	pix.red = ADJRGB(n);

	a = (double)p->green / 255.0;
	b = (double)q->green / 255.0;
	c = b + a < 1.0 ? 0 : b != 0 ? 1.0 - (1.0 - a) / b : 1.0;
	n = (int)(c * 255.0);
	pix.green = ADJRGB(n);

	a = (double)p->blue / 255.0;
	b = (double)q->blue / 255.0;
	c = b + a < 1.0 ? 0 : b != 0 ? 1.0 - (1.0 - a) / b : 1.0;
	n = (int)(c * 255.0);
	pix.blue = ADJRGB(n);
	return pix;
}

PIXELPACKET __fastcall exclusion_pixel(PIXELPACKET* p, PIXELPACKET* q)
{
	PIXELPACKET pix;
	double a, b, c;
	int n;

	a = (double)p->red / 255.0;
	b = (double)q->red / 255.0;
	c = 2.0 * (0.5 - a) * (0.5 - b) - 0.5;
	n = abs((int)(c * 255.0));
	pix.red = ADJRGB(n);

	a = (double)p->green / 255.0;
	b = (double)q->green / 255.0;
	c = 2.0 * (0.5 - a) * (0.5 - b) - 0.5;
	n = abs((int)(c * 255.0));
	pix.green = ADJRGB(n);

	a = (double)p->blue / 255.0;
	b = (double)q->blue / 255.0;
	c = 2.0 * (0.5 - a) * (0.5 - b) - 0.5;
	n = abs((int)(c * 255.0));
	pix.blue = ADJRGB(n);
	return pix;
}

/////////////////////////////////////////////////////////////////////
//

static int __fastcall getblendmode(char* key)
{
	for (int i = 0;;i++)
	{
		if (!sszBlends[0])
			break;
		if (!memcmp(key, sszBlends[i], 4))
			return i;
	}
	return 0;
}

static BOOL __fastcall override(SGIMAGE* pSrc, LAYERINFO& layer)
{
	// pSrcへpDstをオーバーライドする

	int x = layer.x;
	int y = layer.y;

	CRect rcs(0, 0, pSrc->width, pSrc->height);
	CRect rcd(x, y, layer.image->width + x, layer.image->height + y);
	rcs &= rcd;
	if (rcs.IsRectEmpty())
		return TRUE;

	register PIXELPACKET *ps, *pd;

	if (layer.blendtype == 9)					// ディザ
		srand(RANDOM_SEED);

	PIXELPACKET (__fastcall *pfn)(PIXELPACKET*, PIXELPACKET*);
	if (layer.blendtype == 1)					// 比較（暗）
		pfn = darken_pixel;
	else if (layer.blendtype == 2)				// 比較（明）
		pfn = lighten_pixel;
	else if (layer.blendtype == 3)				// 色相
		pfn = hsv_hue_pixel;
	else if (layer.blendtype == 4)				// 彩度
		pfn = hsv_sat_pixel;
	else if (layer.blendtype == 5)				// 色相と彩度
		pfn = color_only_pixel;
	else if (layer.blendtype == 6)				// 輝度
		pfn = hsv_lum_pixel;
	else if (layer.blendtype == 7)				// 乗算
		pfn = multiply_pixel;
	else if (layer.blendtype == 8)				// スクリーン
		pfn = screen_pixel;
	else if (layer.blendtype == 10)				// オーバーレイ
		pfn = overlay_pixel;
	else if (layer.blendtype == 11)				// ハードライト
		pfn = hardlight_pixel;
	else if (layer.blendtype == 12)				// ソフトライト
		pfn = softlight_pixel;
	else if (layer.blendtype == 13)				// 差の絶対値
		pfn = difference_pixel;
	else if (layer.blendtype == 14)				// 除外
		pfn = exclusion_pixel;
	else if (layer.blendtype == 15)				// 覆い焼きカラー
		pfn = dodge_pixel;
	else if (layer.blendtype == 16)				// 焼き込みカラー
		pfn = burn_pixel;
//	else if (layer.blendtype == 9)				// ディザ
//		pfn = dither_pixel;
	else
		pfn = destcopy_pixel;

	PIXELPACKET pix;
	for (int i = rcs.top; i < rcs.bottom; i++)
	{
		ps = pSrc->pixels + pSrc->width * i + rcs.left;
		pd = layer.image->pixels + layer.image->width * (i - y) + (rcs.left - x);
		for (int j = rcs.left; j < rcs.right; j++)
		{
			if (pd->alpha)
			{
				pix = (*pfn)(ps, pd);
				pix = blend_pixel(ps, &pix, pd->alpha);
				*ps = blend_pixel(ps, &pix, layer.opacity);
			}
			else if (layer.flags & 1)
				*ps = blend_pixel(ps, pd, layer.opacity);
			ps++;
			pd++;
		}
	}

	return TRUE;
}

static BOOL __fastcall PackbitsDecodeImage(filemap& fm, SGIMAGE* image, int mode, int channel)
{
	int count, pixel;
	long length;
	register int i;

	BOOL bPseudoClass = FALSE;
	if (mode == PSD_BITMAP || mode == PSD_GRAYSCALE || mode == PSD_INDEX)
		bPseudoClass = TRUE;

	register PIXELPACKET* q = image->pixels;
	length = image->width * image->height;
	while (length > 0)
	{
		count = fm.getc();
		if (count >= 128)
			count -= 256;
		if (count < 0)
		{
			if (count == -128)
				continue;
			count = (-count + 1);
			pixel = fm.getc();
			for ( ; count > 0; count--)
			{
				switch (channel)
				{
					case 0:
						q->red = pixel;
						if (bPseudoClass)
							q->index = pixel;
						break;

					case 1:
						q->green = pixel;
						break;

					case 2:
						q->blue = pixel;
						break;

					case 3:
						q->index = pixel;
						break;

					default:
						q->alpha = pixel;
						break;
				}
				q++;
				length--;
			}
			continue;
		}
		count++;

		for (i = count; i > 0; i--)
		{
			pixel = fm.getc();
			switch (channel)
			{
				case 0:
					q->red = pixel;
					if (bPseudoClass)
						q->index = pixel;
					break;

				case 1:
					q->green = pixel;
					break;

				case 2:
					q->blue = pixel;
					break;

				case 3:
					q->index = pixel;
					break;

				default:
					q->alpha = pixel;
					break;
			}
			q++;
			length--;
		}
	}

	// Guarentee the correct number of pixel packets.
	if (length > 0)
		return FALSE;
	else
	{
		if (length < 0)
			return FALSE;
	}
	return TRUE;
}

// カラーモード
#define PSD_BITMAP			0
#define PSD_GRAYSCALE		1
#define PSD_RGB				3
#define PSD_INDEX			2
#define PSD_CMYK			4
#define PSD_MULTICHANNEL	7
#define PSD_DUOTONE			8
#define PSD_LAB				9


inline BYTE ReadQuantumFile(filemap& fm, int depth)
{
	if (depth <= 8)
		return fm.getc();
	else
		return (BYTE)(((WORD)fm.getmsbshort()) >> (depth - 8));
}

HDIB psd_load_iecW(LPCWSTR pszName, int width, int height)
{
	IMalloc* pm;
	SHGetMalloc(&pm);

	HDIB hdib = NULL;
	IShellFolder* psfDesk;
	if (SHGetDesktopFolder(&psfDesk) == NOERROR)
	{
		// Convert the path to an ITEMIDLIST
		LPITEMIDLIST pidlFq;
		ULONG eat;
		if (psfDesk->ParseDisplayName(NULL, NULL, (OLECHAR*)pszName, &eat, &pidlFq, NULL) == NOERROR)
		{
			IShellFolder* psf;
			LPCITEMIDLIST pidl;
			if (SUCCEEDED(SHBindToParent(pidlFq, IID_IShellFolder, (void**)&psf, &pidl)))
			{
				IExtractImage* pe;
				if (NOERROR == psf->GetUIObjectOf(NULL, 1, &pidl, IID_IExtractImage, 0, (void**)&pe))
				{
					HBITMAP hbm = NULL;
					SIZE s;
					s.cx = width;
					s.cy = height;
					DWORD prior = 0;
					DWORD flags = IEIFLAG_SCREEN | IEIFLAG_ORIGSIZE;
					OLECHAR osz[MAX_PATH * 2];
					if (NOERROR == pe->GetLocation(osz, _COUNTOF(osz), &prior, &s, 24, &flags))
					{
						pe->Extract(&hbm);
						if (hbm)
						{
							hdib = dib_createfrombitmap(hbm, NULL);
							DeleteObject(hbm);
						}
					}
					pe->Release();
				}
				psf->Release();
			}
			pm->Free(pidlFq);
		}
		psfDesk->Release();
	}
	pm->Release();
	return hdib;
}

HDIB psd_load_iec(LPCSTR pszName, int width, int height)
{
	OLECHAR osz[MAX_PATH * 2];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszName, -1, osz, _COUNTOF(osz));
	return psd_load_iecW(osz, width, height);
}

static HDIB psd_load_core(filemap& fm)
{
	char type[4];
	long length, size;
	PSDHEADER head;
	register int i, j, k;
	unsigned int status;
	unsigned short compression;
	PIXELPACKET* q;
	HDIB hdib;

	// Read image header.
	status = fm.read(head.signature, 4);
	head.version = fm.getmsbshort();
	if (!status ||
		strncmp(head.signature, "8BPS", 4) ||
		head.version != 1)
	{
		return NULL;
	}

waitcursor wc;

	fm.read(head.reserved, 6);
	head.channels = fm.getmsbshort();
	head.rows = fm.getmsblong();
	head.columns = fm.getmsblong();
	head.depth = fm.getmsbshort();
	head.mode = fm.getmsbshort();

/* @@@
	// フォトショップのIExtractImageがあればそれで読む
	hdib = psd_load_iec(pszName, head.columns, head.rows);
	if (hdib)
		return hdib;
@@@ */

	COLORPACKET* colormap = NULL;
	int numcolors = 0;

	BOOL bPseudoClass = FALSE;
	length = fm.getmsblong();
	if (head.mode == PSD_BITMAP ||
		head.mode == PSD_GRAYSCALE ||
		head.mode == PSD_INDEX ||
		length > 0)
	{
		bPseudoClass = TRUE;

		// カラーマップ作成
		numcolors = 1 << head.depth;

		colormap = (COLORPACKET*)malloc(numcolors * sizeof(COLORPACKET));
		if (!colormap)
			return NULL;

		for (i = 0; i < numcolors; i++)
		{
			colormap[i].red = (255 * i) / (numcolors - 1);
			colormap[i].green = (255 * i) / (numcolors - 1);
			colormap[i].blue = (255 * i) / (numcolors - 1);
		}

		if (length > 0)
		{
			// Read PSD raster colormap.
			for (i = 0; i < numcolors; i++)
				colormap[i].red = fm.getc();
			for (i = 0; i < numcolors; i++)
				colormap[i].green = fm.getc();
			for (i = 0; i < numcolors; i++)
				colormap[i].blue = fm.getc();
		}
	}

	// イメージリソースセクションを読み飛ばす
	length = fm.getmsblong();
	fm.seek( length, SEEK_CUR);

	// レイヤおよびマスクセクション
	int number_layers = 0;
	long layer_length = fm.getmsblong();

	// いったんレイヤセクションを読み飛ばし、ベースイメージを先に取得
	long layer_start_pos = fm.seek(0, SEEK_CUR);
	fm.seek(layer_length, SEEK_CUR);


	SGIMAGE* base = sg_create(head.columns, head.rows, 24);
	compression = fm.getmsbshort();
	if (compression)
	{
		// Packbit圧縮データをチャネル毎に読み出す
		fm.seek( base->height * head.channels * 2, SEEK_CUR);

		for (i = 0; i < (int)head.channels; i++)
			PackbitsDecodeImage(fm, base, bPseudoClass, i);
	}
	else
	{
		// 非圧縮ピクセルデータをチャネルごとに読み出す
		for (i = 0; i < (int)head.channels; i++)
		{
			q = base->pixels;
			for (j = 0; j < (int)base->packets; j++)
			{
				switch (i)
				{
					case 0:
						q->red = ReadQuantumFile(fm, head.depth);
						q->index = q->red;
						break;

					case 1:
						q->green = ReadQuantumFile(fm, head.depth);
						break;

					case 2:
						q->blue = ReadQuantumFile(fm, head.depth);
						break;

					case 3:
						q->index = ReadQuantumFile(fm, head.depth);
						break;

					default:
						q->alpha = ReadQuantumFile(fm, head.depth);
						break;
				}
				q++;
			}
		}
	}

	if (bPseudoClass)
	{
		q = base->pixels;
		for (i = 0; i < base->packets; i++)
		{
			q->red = colormap[q->index].red;
			q->green = colormap[q->index].green;
			q->blue = colormap[q->index].blue;
			q++;
		}
	}
	else if (head.mode == PSD_CMYK)
	{
		// Correct CMYK levels.
		q = base->pixels;
		for (i = 0; i < base->packets; i++)
		{
			sg_cmyk2rgb(q);
			q++;
		}
	}

	// レイヤセクションに戻り、レイヤセクションを読む
	fm.seek(layer_start_pos, SEEK_SET);
	if (layer_length > 0)
	{
		// レイヤマスクブロックを読む
		size = fm.getmsblong();
		number_layers = fm.getmsbshort();
		number_layers = abs(number_layers);
		LAYERINFO* layer = (LAYERINFO*)malloc(number_layers * sizeof(LAYERINFO));

		// レイヤ情報を取得
		for (i = 0; i < number_layers; i++)
		{
			layer[i].y = fm.getmsblong();
			layer[i].x = fm.getmsblong();
			layer[i].height = fm.getmsblong() - layer[i].y;
			layer[i].width = fm.getmsblong() - layer[i].x;
			layer[i].channels = fm.getmsbshort();
			if (layer[i].channels > 24)
			{
				free(layer);
				if (colormap)
					free(colormap);
				sg_destroy(base);
				return NULL;
			}

			for (j = 0; j < layer[i].channels; j++)
			{
				layer[i].channel_info[j].type = fm.getmsbshort();
				layer[i].channel_info[j].size = fm.getmsblong();
			}

			status = fm.read(type, 4);
			if (!status || strncmp(type, "8BIM", 4))
			{
				free(layer);
				if (colormap)
					free(colormap);
				sg_destroy(base);
				return NULL;
			}

			fm.read(layer[i].blend, 4);				// レイヤ描画モード
			layer[i].blendtype = getblendmode(layer[i].blend);
			layer[i].opacity = fm.getc();			// 合成の際の不透明度
			layer[i].clipping = fm.getc();			// クリッピング
			layer[i].flags = fm.getc();				// フラグ
			fm.getc();								// filler
			size = fm.getmsblong();					// エクストラデータサイズ
			fm.seek( size, SEEK_CUR);				//  " 読み飛ばし
		}

		// 各レイヤのピクセルデータ読み出し
		// レイヤサイズ毎のイメージを作成して、そこへ読み込む
		for (i = 0; i < number_layers; i++)
		{
			SGIMAGE* image = sg_create(layer[i].width, layer[i].height, 24);
			layer[i].image = image;
			for (j = 0; j < layer[i].channels; j++)
			{
				compression = fm.getmsbshort();
				if (compression)
				{
					// Packbit圧縮データをチャネル毎に読み出す
					fm.seek(layer[i].height * 2, SEEK_CUR);
					PackbitsDecodeImage(fm, image, bPseudoClass,
										layer[i].channel_info[j].type);
				}
				else
				{
					// 非圧縮のピクセルデータを別々のプレーンへ読み込む
					q = image->pixels;
					for (k = 0; k < image->packets; k++)
					{
						switch (layer[i].channel_info[j].type)
						{
							case 0:
								q->red = ReadQuantumFile(fm, head.depth);
								q->index = q->red;
								break;

							case 1:
								q->green = ReadQuantumFile(fm, head.depth);
								break;

							case 2:
								q->blue = ReadQuantumFile(fm, head.depth);
								break;

							case 3:
								q->index = ReadQuantumFile(fm, head.depth);
								break;

							default:
								q->alpha = ReadQuantumFile(fm, head.depth);
								break;
						}
						q++;
					}
				}
			}

			q = image->pixels;
			if (bPseudoClass)
			{
				for (k = 0; k < image->packets; k++)
				{
					q->red = colormap[q->index].red;
					q->green = colormap[q->index].green;
					q->blue = colormap[q->index].blue;
					q++;
				}
			}
			else if (head.mode == PSD_CMYK)
            {
				// CMYK を RGB へ変換
				for (k = 0; k < image->packets; k++)
				{
					sg_cmyk2rgb(q);
					q++;
				}
			}
			else if (layer[i].opacity != 255)
			{
				// チャネルの不透明度を適用する
				for (k = 0; k < image->packets; k++)
				{
					q->index = (int)(q->index * layer[i].opacity) / 255;
					q++;
				}
			}

			// ベースイメージにレイヤイメージをオーバーラップする
			// flags の visible_bit がセットされていれば、そのレイヤは非表示
			if (layer[i].width && layer[i].height && !(layer[i].flags & 0x2))
				override(base, layer[i]);
			sg_destroy(image);
		}
		free(layer);
	}

	hdib = sg_getdib(base);
	sg_destroy(base);

	return hdib;
}

extern "C"
HDIB WINAPI psd_load(LPSTR pszName)
{
	HDIB hdib = NULL;
	filemap fm;
	if (fm.open(pszName))
		hdib = psd_load_core(fm);
	return hdib;
}

extern "C"
HDIB WINAPI psd_loadW(LPWSTR pszName)
{
	HDIB hdib = NULL;
	filemap fm;
	if (fm.openW(pszName))
		hdib = psd_load_core(fm);
	return hdib;
}

static BOOL psd_save_core(HANDLE hf, HDIB hdib)
{
	SGIMAGE* image = sg_fromdib(hdib);
	if (!image)
		return FALSE;

	DWORD dw;
	WriteFile(hf, "8BPS", 4, &dw, NULL);		// signature
	msb1stwriteshort(hf, 1);					// version
	WriteFile(hf, "      ", 6, &dw, NULL);		// reserved
	msb1stwriteshort(hf,
			(image->bits < 24) ? 1 : 3);		// チャンネル
	msb1stwritelong(hf, image->height);			// 高さ
	msb1stwritelong(hf, image->width);			// 幅
	msb1stwriteshort(hf, 8);					// DEPTH
	msb1stwriteshort(hf,
		(image->bits < 24) ? PSD_INDEX : PSD_RGB);	// モード

	// カラーマップの書き込み
	if (image->bits == 24)
		msb1stwritelong(hf, 0);
	else {
		BYTE tbl[256];

		// Write PSD raster colormap.
		msb1stwritelong(hf, 768);
		memset(tbl, 0, 256);
		int i;
		for (i = 0; i < image->colors; i++)
			tbl[i] = image->colormap[i].red;
		WriteFile(hf, tbl, 256, &dw, NULL);
		for (i = 0; i < image->colors; i++)
			tbl[i] = image->colormap[i].green;
		WriteFile(hf, tbl, 256, &dw, NULL);
		for (i = 0; i < image->colors; i++)
			tbl[i] = image->colormap[i].blue;
		WriteFile(hf, tbl, 256, &dw, NULL);
	}
	msb1stwritelong(hf, 0);			// image resource block
	msb1stwritelong(hf, 0);			// layer and mask block
	msb1stwriteshort(hf, 0);		// compression

	// Write uncompressed pixel data as separate planes.
	BYTE* pPlane = (BYTE*)GlobalAllocPtr(GHND, image->packets);
	register BYTE* p = pPlane;
	if (image->bits < 24) {
		sg_getplane(image, PLANE_RED, pPlane);
		WriteFile(hf, pPlane, image->packets, &dw, NULL);
	}
	else
	{
		sg_getplane(image, PLANE_RED, pPlane);
		WriteFile(hf, pPlane, image->packets, &dw, NULL);

		sg_getplane(image, PLANE_GREEN, pPlane);
		WriteFile(hf, pPlane, image->packets, &dw, NULL);

		sg_getplane(image, PLANE_BLUE, pPlane);
		WriteFile(hf, pPlane, image->packets, &dw, NULL);
	}
	GlobalFreePtr(pPlane);
	sg_destroy(image);
	return TRUE;
}

extern "C"
BOOL WINAPI psd_save(LPSTR pszName, HDIB hdib)
{
	BOOL bOK = FALSE;
	HANDLE hf = CreateFile(pszName, GENERIC_WRITE, 0, NULL,
						CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hf != INVALID_HANDLE_VALUE)
	{
		bOK = psd_save_core(hf, hdib);
		CloseHandle(hf);
	}
	return bOK;
}

extern "C"
BOOL WINAPI psd_saveW(LPWSTR pszName, HDIB hdib)
{
	BOOL bOK = FALSE;
	HANDLE hf = CreateFileW(pszName, GENERIC_WRITE, 0, NULL,
						CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hf != INVALID_HANDLE_VALUE)
	{
		bOK = psd_save_core(hf, hdib);
		CloseHandle(hf);
	}
	return bOK;
}
