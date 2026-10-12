//////////////////////////////////////////////////////////////////////
// common.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"


BOOL __fastcall grad_table(COLORREF* rgbs, int numcolors, PALETTEENTRY* ppe, int numpalettes)
{
	if (numcolors < 1 || numpalettes < 1 || !rgbs || !ppe)
		return FALSE;

	memset(ppe, 0, sizeof(PALETTEENTRY) * numpalettes);

	register PALETTEENTRY* pw = ppe;

	int i;
	if (numcolors == 1)
	{
		for (i = 0; i < numpalettes; i++)
		{
			pw->peRed = GetRValue(*rgbs);
			pw->peGreen = GetGValue(*rgbs);
			pw->peBlue = GetBValue(*rgbs);
			pw++;
		}
	}
	else
	{
		int r1, r2, g1, g2, b1, b2;
		double dr, dg, db;

		int delta_loop = numpalettes / (numcolors - 1);
		int startpos = 0, endpos;
//		int left = numpalettes;
		double delta_div = (double)(delta_loop - 1);

		for (int n = 0; n < numcolors - 1; n++)
		{
			r1 = GetRValue(*rgbs);
			g1 = GetGValue(*rgbs);
			b1 = GetBValue(*rgbs);
			rgbs++;
			r2 = GetRValue(*rgbs);
			g2 = GetGValue(*rgbs);
			b2 = GetBValue(*rgbs);

			endpos = startpos + delta_loop;
			if (endpos + delta_loop > numpalettes)
				endpos = numpalettes;

			dr = (double)((r2 - r1) / delta_div);
			dg = (double)((g2 - g1) / delta_div);
			db = (double)((b2 - b1) / delta_div);

			for (i = startpos; i < endpos; i++)
			{
				pw->peRed = ADJRGB(r1 + (dr * (i - startpos)));
				pw->peGreen = ADJRGB(g1 + (dg * (i - startpos)));
				pw->peBlue = ADJRGB(b1 + (db * (i - startpos)));
				pw++;
			}
			startpos += delta_loop;
		}
	}
	return TRUE;
}

LOGPALETTE* __fastcall grad_table_ex(GRADCOLORS* pcol, int numcolors, int numtables)
{
	if (numcolors < 1 || !pcol)
		return NULL;

	LOGPALETTE* plgp = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) +
										sizeof(PALETTEENTRY) * numtables);
	plgp->palNumEntries = numtables;
	register PALETTEENTRY* pw = plgp->palPalEntry;
	int i;
	if (numcolors == 1 || (numcolors == 2 && pcol->cr == (pcol + 1)->cr))
	{
		for (i = 0; i < plgp->palNumEntries; i++)
		{
			pw->peRed = GetRValue(pcol->cr);
			pw->peGreen = GetGValue(pcol->cr);
			pw->peBlue = GetBValue(pcol->cr);
			pw++;
		}
	}
	else
	{
		int r1, r2, g1, g2, b1, b2;
		double dr, dg, db;

		int startpos = 0, endpos;
		for (int n = 0; n < numcolors - 1; n++)
		{
			startpos = pcol->pos;
			r1 = GetRValue(pcol->cr);
			g1 = GetGValue(pcol->cr);
			b1 = GetBValue(pcol->cr);
			pcol++;
			endpos = pcol->pos;
			r2 = GetRValue(pcol->cr);
			g2 = GetGValue(pcol->cr);
			b2 = GetBValue(pcol->cr);
			if (endpos == startpos)
				continue;

			int delta = endpos - startpos;
			dr = (double)(r2 - r1) / (double)delta;
			dg = (double)(g2 - g1) / (double)delta;
			db = (double)(b2 - b1) / (double)delta;
			for (i = 0; i < delta; i++)
			{
				pw->peRed = ADJRGB(r1 + (dr * i));
				pw->peGreen = ADJRGB(g1 + (dg * i));
				pw->peBlue = ADJRGB(b1 + (db * i));
				pw++;
			}
		}
	}
	return plgp;
}

RGBA __fastcall destcopy_pixel(RGBA* /*p*/, RGBA* q)
{
	return *q;
}

#define MIXRGB(a,b,c) (((int)(a)*(255-(c))+(int)(b)*(c))/255)

RGBA __fastcall blend_pixel(RGBA* p, RGBA* q, int blend)
{
	RGBA pix;
	pix.red = MIXRGB(p->red, q->red, blend);
	pix.green = MIXRGB(p->green, q->green, blend);
	pix.blue = MIXRGB(p->blue, q->blue, blend);
	return pix;
}

void __fastcall blend_to(RGBA* p, RGBA* q, int blend)
{
	p->red = MIXRGB(p->red, q->red, blend);
	p->green = MIXRGB(p->green, q->green, blend);
	p->blue = MIXRGB(p->blue, q->blue, blend);
}

RGBA __fastcall darken_pixel(RGBA* p, RGBA* q)
{
	RGBA pix;
	pix.red = min(p->red, q->red);
	pix.green = min(p->green, q->green);
	pix.blue = min(p->blue, q->blue);
	pix.alpha = min(p->alpha, q->alpha);
	return pix;
}

RGBA __fastcall lighten_pixel(RGBA* p, RGBA* q)
{
	RGBA pix;
	pix.red = max(p->red, q->red);
	pix.green = max(p->green, q->green);
	pix.blue = max(p->blue, q->blue);
	pix.alpha = max(p->alpha, q->alpha);
	return pix;
}

RGBA __fastcall hsv_hue_pixel(RGBA* p, RGBA* q)
{
	RGBA pix;
	double h1, s1, v1, h2, s2, v2;

	rgb_to_hsv(p->red, p->green, p->blue, &h1, &s1, &v1);
	rgb_to_hsv(q->red, q->green, q->blue, &h2, &s2, &v2);

	h1 = h2;
	hsv_to_rgb(&pix.red, &pix.green, &pix.blue, h1, s1, v1);
	return pix;
}

RGBA __fastcall hsv_sat_pixel(RGBA* p, RGBA* q)
{
	RGBA pix;
	double h1, s1, v1, h2, s2, v2;

	rgb_to_hsv(p->red, p->green, p->blue, &h1, &s1, &v1);
	rgb_to_hsv(q->red, q->green, q->blue, &h2, &s2, &v2);

	s1 = s2;
	hsv_to_rgb(&pix.red, &pix.green, &pix.blue, h1, s1, v1);
	return pix;
}

RGBA __fastcall hsv_lum_pixel(RGBA* p, RGBA* q)
{
	RGBA pix;
	double h1, s1, v1, h2, s2, v2;

	rgb_to_hsv(p->red, p->green, p->blue, &h1, &s1, &v1);
	rgb_to_hsv(q->red, q->green, q->blue, &h2, &s2, &v2);

	v1 = v2;
	hsv_to_rgb(&pix.red, &pix.green, &pix.blue, h1, s1, v1);

	return pix;
}

RGBA __fastcall hsv_only_pixel(RGBA* p, RGBA* q, int mode)
{
	RGBA pix;
	double h1, s1, v1, h2, s2, v2;

	rgb_to_hsv(p->red, p->green, p->blue, &h1, &s1, &v1);
	rgb_to_hsv(q->red, q->green, q->blue, &h2, &s2, &v2);

	if (mode == blendHue)
		h1 = h2;
	else if (mode == blendSat)
		s1 = s2;
	else if (mode == blendLum)
		v1 = v2;

	hsv_to_rgb(&pix.red, &pix.green, &pix.blue, h1, s1, v1);
	return pix;
}

RGBA __fastcall color_only_pixel(RGBA* p, RGBA* q)
{
	RGBA pix;
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

RGBA __fastcall multiply_pixel(RGBA* p, RGBA* q)
{
	RGBA pix;
	int tmp;
	pix.red = INT_MULT(p->red, q->red, tmp);
	pix.green = INT_MULT(p->green, q->green, tmp);
	pix.blue = INT_MULT(p->blue, q->blue, tmp);
	return pix;
}

RGBA __fastcall divide_pixel(RGBA* p, RGBA* q)
{
	RGBA pix;
	pix.red = ADJRGB(((int)p->red << 8) / (1 + (int)q->red));
	pix.green = ADJRGB(((int)p->green << 8) / (1 + (int)q->green));
	pix.blue = ADJRGB(((int)p->blue << 8) / (1 + (int)q->blue));
	return pix;
}

RGBA __fastcall screen_pixel(RGBA* p, RGBA* q)
{
	RGBA pix;
	int tmp;
	pix.red = 255 - INT_MULT((255 - p->red), (255 - q->red), tmp);
	pix.green = 255 - INT_MULT((255 - p->green), (255 - q->green), tmp);
	pix.blue = 255 - INT_MULT((255 - p->blue), (255 - q->blue), tmp);
	return pix;
}

RGBA __fastcall add_pixel(RGBA* p, RGBA* q)
{
	RGBA pix;
	pix.red = ADJRGB((int)p->red + (int)q->red);
	pix.green = ADJRGB((int)p->green + (int)q->blue);
	pix.blue = ADJRGB((int)p->blue + (int)q->blue);
	return pix;
}

RGBA __fastcall del_pixel(RGBA* p, RGBA* q)
{
	RGBA pix;
	pix.red = ADJRGB((int)p->red - (int)q->red);
	pix.green = ADJRGB((int)p->green - (int)q->blue);
	pix.blue = ADJRGB((int)p->blue - (int)q->blue);
	return pix;
}

RGBA __fastcall idiv_pixel(RGBA* p, RGBA* q)
{
	RGBA pix;
	pix.red = ADJRGB(p->red - (255 - q->red));
	pix.green = ADJRGB(p->green - (255 - q->green));
	pix.blue = ADJRGB(p->blue - (255 - q->blue));
	return pix;
}

RGBA __fastcall subtract_pixel(RGBA* p, RGBA* q)
{
	RGBA pix;
	pix.red = ADJRGB((int)p->red - (int)q->red);
	pix.green = ADJRGB((int)p->green - (int)q->green);
	pix.blue = ADJRGB((int)p->blue - (int)q->blue);
	return pix;
}

RGBA __fastcall difference_pixel(RGBA* p, RGBA* q)
{
	RGBA pix;
	int diff;
	diff = (int)p->red - (int)q->red;
	pix.red = (diff < 0) ? -diff : diff;
	diff = (int)p->green - (int)q->green;
	pix.green = (diff < 0) ? -diff : diff;
	diff = (int)p->blue - (int)q->blue;
	pix.blue = (diff < 0) ? -diff : diff;
	return pix;
}

RGBA __fastcall average_pixel(RGBA* p, RGBA* q)
{
	RGBA pix;
	pix.red = ((int)p->red + (int)q->red) >> 1;
	pix.green = ((int)p->green + (int)q->blue) >> 1;
	pix.blue = ((int)p->blue + (int)q->blue) >> 1;
	return pix;
}

#define HARDLIGHT(x,y)	((y)<128?(2*(x)*(y)/255):255-2*(255-(x))*(255-(y))/255)
#define SMP(x)			(((x)<<6)/255)
#define SOFTLIGHT(x,y)	((y)<128?2*SMP((x))*(y)/255:255-(2*(255-SMP((x)))*(255-(y))/255))

RGBA __fastcall overlay_pixel(RGBA* p, RGBA* q)
{
/* @@
	RGBA pix;
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
@@ */
	RGBA pix;
	pix.red = HARDLIGHT((int)q->red, (int)p->red);
	pix.green = HARDLIGHT((int)q->green, (int)p->green);
	pix.blue = HARDLIGHT((int)q->blue, (int)p->blue);
	return pix;
}

RGBA __fastcall hardlight_pixel(RGBA* p, RGBA* q)
{
/* @@@
	RGBA pix;
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
@@@ */
	RGBA pix;
	pix.red = HARDLIGHT((int)p->red, (int)q->red);
	pix.green = HARDLIGHT((int)p->green, (int)q->green);
	pix.blue = HARDLIGHT((int)p->blue, (int)q->blue);
	return pix;
}

RGBA __fastcall softlight_pixel(RGBA* p, RGBA* q)
{
/* @@@
	RGBA pix;
	pix.red = SOFTLIGHT((int)p->red, (int)q->red);
	pix.green = SOFTLIGHT((int)p->green, (int)q->green);
	pix.blue = SOFTLIGHT((int)p->blue, (int)q->blue);
	return pix;
@@@ */

	RGBA pix;
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

RGBA __fastcall dodge_pixel(RGBA* p, RGBA* q)
{
	RGBA pix;
	double a, b, c;
	int n;

	a = (double)p->red / 255.0;
	b = (double)q->red / 255.0;
	c = (256.0 * a) / (256.0 - 255.0 * b);
	n = (int)(c * 255.0);
	pix.red = ADJRGB(n);

	a = (double)p->green / 255.0;
	b = (double)q->green / 255.0;
	c = (256.0 * a) / (256.0 - 255.0 * b);
	n = (int)(c * 255.0);
	pix.green = ADJRGB(n);

	a = (double)p->blue / 255.0;
	b = (double)q->blue / 255.0;
	c = (256.0 * a) / (256.0 - 255.0 * b);
	n = (int)(c * 255.0);
	pix.blue = ADJRGB(n);
	return pix;
}

RGBA __fastcall burn_pixel(RGBA* p, RGBA* q)
{
	RGBA pix;
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

RGBA __fastcall exclusion_pixel(RGBA* p, RGBA* q)
{
	RGBA pix;
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

BLENDFUNC get_blendfunction(int blendtype)
{
	if (blendtype == blendDark)				// 比較（暗）
		return darken_pixel;
	else if (blendtype == blendLite)		// 比較（明）
		return lighten_pixel;
	else if (blendtype == blendHue)			// 色相
		return hsv_hue_pixel;
	else if (blendtype == blendSat)			// 彩度
		return hsv_sat_pixel;
	else if (blendtype == blendLum)			// 輝度
		return  hsv_lum_pixel;
	else if (blendtype == blendColor)		// 色相と彩度
		return color_only_pixel;
	else if (blendtype == blendMul)			// 乗算
		return multiply_pixel;
	else if (blendtype == blendScreen)		// スクリーン
		return screen_pixel;
	else if (blendtype == blendOverlay)		// オーバーレイ
		return overlay_pixel;
	else if (blendtype == blendDiff)		// 差の絶対値
		return difference_pixel;
	else if (blendtype == blendExclusion)	// 除外
		return exclusion_pixel;
	else if (blendtype == blendDodge)		// 覆い焼きカラー
		return dodge_pixel;
	else if (blendtype == blendBurn)		// 焼き込みカラー
		return burn_pixel;
	else if (blendtype == blendHardLight)	// ハードライト
		return hardlight_pixel;
	else if (blendtype == blendSoftLight)	// ソフトライト
		return softlight_pixel;
	else if (blendtype == blendAdd)			// 加算
		return add_pixel;
	else if (blendtype == blendDel)			// 減算
		return del_pixel;

	return destcopy_pixel;
}

void* load_resource_data(HINSTANCE hInst, UINT typeResource, UINT idResource, DWORD& size)
{
	HRSRC hrsrc = FindResource(hInst, MAKEINTRESOURCE(idResource), MAKEINTRESOURCE(typeResource));
	if (hrsrc) {
		size = SizeofResource(hInst, hrsrc);
		if (size) {
			HGLOBAL hglb = LoadResource(hInst, hrsrc);
			if (hglb) {
				return LockResource(hglb);
			}
		}
	}
	return NULL;
}
