///////////////////////////////////////////////////////////////////////
// filter3x3.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

void __cdecl qsort_ex(void *base, size_t n, size_t size,
	int (__cdecl *comp)(const void*, const void*, const void*), const void* param);


static int arSoften[9] = { 8, 12, 8, 12, 20, 12, 8, 12, 8 };
static int arSharpen[9] = { -1, -1, -1, -1, 18, -1, -1, -1, -1 };
//static int arSharpenMore[9] = { -3, -3, -3, -3, 34, -3, -3, -3, -3 };		// div10
static int arSharpenMore[9] = { -1, -1, -1, -1, 13, -1, -1, -1, -1 };		// div05
static int arEmboss[9] = { -1, 0, 0, 0, 0, 0, 0, 0, 1 };
//static int arEmboss[9] = { -1, -2, 0, -2, 0, 2, 0, 2, 1 };
static int arEnhance[9] = { 0, -1, 0, -1, 5, -1, 0, -1, 0 };
static int arEnhanceMore[9] = { -1, -1, -1, -1, 9, -1, -1, -1, -1 };
static int arLaplacian[9] = { 1, 1, 1, 1, -8, 1, 1, 1, 1 };
static int arSobelVert[9] = { 1, 2, 1, 0, 0, 0, -1, -2, -1 };
static int arSobelHorz[9] = { 1, 0, -1, 2, 0, -2, 1, 0, -1 };

#define FILTER_THRSLD_MIN		6
#define FILTER_THRSLD_MAX		12


///////////////////////////////////////////////////////////////////////
//

static void __fastcall calc_soften(RGBA** par, RGBA* /*ps*/, RGBA* pd)
{
	register int i, red = 0, green = 0, blue = 0;
	for (i = 0; i < 9; i++)
	{
		red += par[i]->red * arSoften[i];
		green += par[i]->green * arSoften[i];
		blue += par[i]->blue * arSoften[i];
	}
	pd->red = ADJRGB(red / 100);
	pd->green = ADJRGB(green / 100);
	pd->blue = ADJRGB(blue / 100);
}

static void __fastcall calc_softenwoedge(RGBA** par, RGBA* ps, RGBA* pd)
{
	register int i, red = 0, green = 0, blue = 0, sr = 0, sg = 0, sb = 0;
	for (i = 0; i < 9; i++)
	{
		red += par[i]->red * arSoften[i];
		green += par[i]->green * arSoften[i];
		blue += par[i]->blue * arSoften[i];
		sr += par[i]->red * arSharpen[i];
		sg += par[i]->green * arSharpen[i];
		sb += par[i]->blue * arSharpen[i];
	}
	red = ADJRGB(red / 100);
	green = ADJRGB(green / 100);
	blue = ADJRGB(blue / 100);
	sr = ADJRGB(sr / 10);
	sg = ADJRGB(sg / 10);
	sb = ADJRGB(sb / 10);

	int delta = abs(ps->red - sr);
	if (delta <= 4)
		pd->red = red;
	else if (delta <= 5)
		pd->red = ADJRGB((red * 8 + ps->red * 2) / 10);
	else if (delta <= 6)
		pd->red = ADJRGB((red * 6 + ps->red * 4) / 10);
	else if (delta <= 7)
		pd->red = ADJRGB((red * 4 + ps->red * 6) / 10);
	else if (delta <= 8)
		pd->red = ADJRGB((red * 2 + ps->red * 8) / 10);
	else
		pd->red = ps->red;

	delta = abs(ps->green - sg);
	if (delta <= 4)
		pd->green = green;
	else if (delta <= 5)
		pd->green = ADJRGB((green * 8 + ps->green * 2) / 10);
	else if (delta <= 6)
		pd->green = ADJRGB((green * 6 + ps->green * 4) / 10);
	else if (delta <= 7)
		pd->green = ADJRGB((green * 4 + ps->green * 6) / 10);
	else if (delta <= 8)
		pd->green = ADJRGB((green * 2 + ps->green * 8) / 10);
	else
		pd->green = ps->green;

	delta = abs(ps->green - sb);
	if (delta <= 4)
		pd->blue = blue;
	else if (delta <= 5)
		pd->blue = ADJRGB((blue * 8 + ps->blue * 2) / 10);
	else if (delta <= 6)
		pd->blue = ADJRGB((blue * 6 + ps->blue * 4) / 10);
	else if (delta <= 7)
		pd->blue = ADJRGB((blue * 4 + ps->blue * 6) / 10);
	else if (delta <= 8)
		pd->blue = ADJRGB((blue * 2 + ps->blue * 8) / 10);
	else
		pd->blue = ps->blue;
}

static void __fastcall calc_sharpen(RGBA** par, RGBA* /*ps*/, RGBA* pd)
{
	register int i, red = 0, green = 0, blue = 0;
	for (i = 0; i < 9; i++)
	{
		red += par[i]->red * arSharpen[i];
		green += par[i]->green * arSharpen[i];
		blue += par[i]->blue * arSharpen[i];
	}
	pd->red = ADJRGB(red / 10);
	pd->green = ADJRGB(green / 10);
	pd->blue = ADJRGB(blue / 10);
}

static void __fastcall calc_sharpenmore(RGBA** par, RGBA* /*ps*/, RGBA* pd)
{
	register int i, red = 0, green = 0, blue = 0;
	for (i = 0; i < 9; i++)
	{
		red += par[i]->red * arSharpenMore[i];
		green += par[i]->green * arSharpenMore[i];
		blue += par[i]->blue * arSharpenMore[i];
	}
	pd->red = ADJRGB(red / 5);
	pd->green = ADJRGB(green / 5);
	pd->blue = ADJRGB(blue / 5);
}

static void __fastcall calc_sharpenedge(RGBA** par, RGBA* ps, RGBA* pd)
{
	register int i, red = 0, green = 0, blue = 0;
	for (i = 0; i < 9; i++)
	{
		red += par[i]->red * arSharpen[i];
		green += par[i]->green * arSharpen[i];
		blue += par[i]->blue * arSharpen[i];
	}
	red = ADJRGB(red / 10);
	green = ADJRGB(green / 10);
	blue = ADJRGB(blue / 10);

	int delta;
	delta = abs(ps->red - red);
	if (delta <= 7)
		pd->red = ps->red;
	else if (delta <= 8)
		pd->red = ADJRGB((ps->red * 8 + red * 2) / 10);
	else if (delta <= 9)
		pd->red = ADJRGB((ps->red * 6 + red * 4) / 10);
	else if (delta <= 10)
		pd->red = ADJRGB((ps->red * 4 + red * 6) / 10);
	else if (delta <= 11)
		pd->red = ADJRGB((ps->red * 2 + red * 8) / 10);
	else
		pd->red = red;

	delta = abs(ps->green - green);
	if (delta <= 7)
		pd->green = ps->green;
	else if (delta <= 8)
		pd->green = ADJRGB((ps->green * 8 + green * 2) / 10);
	else if (delta <= 9)
		pd->green = ADJRGB((ps->green * 6 + green * 4) / 10);
	else if (delta <= 10)
		pd->green = ADJRGB((ps->green * 4 + green * 6) / 10);
	else if (delta <= 11)
		pd->green = ADJRGB((ps->green * 2 + green * 8) / 10);
	else
		pd->green = green;

	delta = abs(ps->blue - blue);
	if (delta <= 7)
		pd->blue = ps->blue;
	else if (delta <= 8)
		pd->blue = ADJRGB((ps->blue * 8 + blue * 2) / 10);
	else if (delta <= 9)
		pd->blue = ADJRGB((ps->blue * 6 + blue * 4) / 10);
	else if (delta <= 10)
		pd->blue = ADJRGB((ps->blue * 4 + blue * 6) / 10);
	else if (delta <= 11)
		pd->blue = ADJRGB((ps->blue * 2 + blue * 8) / 10);
	else
		pd->blue = blue;
}

static void __fastcall calc_filter(RGBA** par, RGBA* /*ps*/, RGBA* pd, int* filter, int divide)
{
	register int i, red = 0, green = 0, blue = 0;
	for (i = 0; i < 9; i++)
	{
		red += par[i]->red * filter[i];
		green += par[i]->green * filter[i];
		blue += par[i]->blue * filter[i];
	}
	pd->red = ADJRGB(red / divide);
	pd->green = ADJRGB(green / divide);
	pd->blue = ADJRGB(blue / divide);
}

static void __fastcall calc_edge_enhance(RGBA** par, RGBA* /*ps*/, RGBA* pd)
{
	register int i, red = 0, green = 0, blue = 0;
	for (i = 0; i < 9; i++)
	{
		red += par[i]->red * arEnhance[i];
		green += par[i]->green * arEnhance[i];
		blue += par[i]->blue * arEnhance[i];
	}
	pd->red = ADJRGB(red);
	pd->green = ADJRGB(green);
	pd->blue = ADJRGB(blue);
}

static void __fastcall calc_edge_enhancemore(RGBA** par, RGBA* /*ps*/, RGBA* pd)
{
	register int i, red = 0, green = 0, blue = 0;
	for (i = 0; i < 9; i++)
	{
		red += par[i]->red * arEnhanceMore[i];
		green += par[i]->green * arEnhanceMore[i];
		blue += par[i]->blue * arEnhanceMore[i];
	}
	pd->red = ADJRGB(red);
	pd->green = ADJRGB(green);
	pd->blue = ADJRGB(blue);
}

static void __fastcall calc_edge_laplacian(RGBA** par, RGBA* /*ps*/, RGBA* pd)
{
	register int i, red = 0, green = 0, blue = 0;
	for (i = 0; i < 9; i++)
	{
		red += par[i]->red * arLaplacian[i];
		green += par[i]->green * arLaplacian[i];
		blue += par[i]->blue * arLaplacian[i];
	}
	pd->red = ADJRGB(red);
	pd->green = ADJRGB(green);
	pd->blue = ADJRGB(blue);
}

static void __fastcall calc_edge_sobelvert(RGBA** par, RGBA* /*ps*/, RGBA* pd)
{
	register int i, red = 0, green = 0, blue = 0;
	for (i = 0; i < 9; i++)
	{
		red += par[i]->red * arSobelVert[i];
		green += par[i]->green * arSobelVert[i];
		blue += par[i]->blue * arSobelVert[i];
	}
	pd->red = ADJRGB(red);
	pd->green = ADJRGB(green);
	pd->blue = ADJRGB(blue);
}

static void __fastcall calc_edge_sobelhorz(RGBA** par, RGBA* /*ps*/, RGBA* pd)
{
	register int i, red = 0, green = 0, blue = 0;
	for (i = 0; i < 9; i++)
	{
		red += par[i]->red * arSobelHorz[i];
		green += par[i]->green * arSobelHorz[i];
		blue += par[i]->blue * arSobelHorz[i];
	}
	pd->red = ADJRGB(red);
	pd->green = ADJRGB(green);
	pd->blue = ADJRGB(blue);
}

static void __fastcall calc_emboss(RGBA** par, RGBA* /*ps*/, RGBA* pd)
{
	register int i, red = 0, green = 0, blue = 0;
	for (i = 0; i < 9; i++)
	{
		if (arEmboss[i])
		{
			red += par[i]->red * arEmboss[i];
			green += par[i]->green * arEmboss[i];
			blue += par[i]->blue * arEmboss[i];
		}
	}
	pd->red = (red + 128) & 0xFF;
	pd->green = (green + 128) & 0xFF;
	pd->blue = (blue + 128) & 0xFF;
}

static void __fastcall calc_mean(RGBA** par, RGBA* /*ps*/, RGBA* pd)
{
	register int i, red = 0, green = 0, blue = 0;
	for (i = 0; i < 9; i++)
	{
		red += par[i]->red;
		green += par[i]->green;
		blue += par[i]->blue;
	}
	pd->red = ADJRGB(red / 9);
	pd->green = ADJRGB(green / 9);
	pd->blue = ADJRGB(blue / 9);
}

static void __fastcall calc_dilate(RGBA** par, RGBA* /*ps*/, RGBA* pd)
{
	register int i;
	pd->red = 0;
	pd->green = 0;
	pd->blue = 0;
	for (i = 0; i < 9; i++) {
		pd->red = max(pd->red, par[i]->red);
		pd->green = max(pd->green, par[i]->green);
		pd->blue = max(pd->blue, par[i]->blue);
	}
}

static void __fastcall calc_erode(RGBA** par, RGBA* /*ps*/, RGBA* pd)
{
	register int i;
	pd->red = 255;
	pd->green = 255;
	pd->blue = 255;
	for (i = 0; i < 9; i++) {
		pd->red = min(pd->red, par[i]->red);
		pd->green = min(pd->green, par[i]->green);
		pd->blue = min(pd->blue, par[i]->blue);
	}
}

static int __cdecl intcomp(const void *ps, const void *pt, const void* /*param*/)
{
	return (*((int*)ps) - *((int*)pt));
}

static void __fastcall calc_median(RGBA** par, RGBA* /*ps*/, RGBA* pd)
{
	int arr[27];
	register int i;
	for (i = 0; i < 9; i++) {
		arr[i] = par[i]->red;
		arr[i + 9] = par[i]->green;
		arr[i + 18] = par[i]->blue;
	}
	qsort_ex(&arr[0], 9, sizeof(int), intcomp, NULL);
	qsort_ex(&arr[9], 9, sizeof(int), intcomp, NULL);
	qsort_ex(&arr[18], 9, sizeof(int), intcomp, NULL);

	pd->red = arr[4];
	pd->green = arr[13];
	pd->blue = arr[22];
}

static void __fastcall setarray(register RGBA* pd, register RGBA** par,
 int dx, int dy, int x,  int y)
{
	register RGBA* ps;
	int xp, xn, yp, yn;
	xp = (x - 1 < 0) ? 0 : 1;
	xn = (x + 1 >= dx) ? 0 : 1;
	yp = (y - 1 < 0) ? 0 : dx;
	yn = (y + 1 >= dy) ? 0 : dx;

	ps = pd - yp;
	*par++ = ps - xp;
	*par++ = ps;
	*par++ = ps + xn;
	ps = pd;
	*par++ = ps - xp;
	*par++ = ps;
	*par++ = ps + xn;
	ps = pd + yn;
	*par++ = ps - xp;
	*par++ = ps;
	*par++ = ps + xn;
}

BOOL filter_formal3x3(mif& src, LPRECT prc, UINT nType)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;
	src.m_bits = 24;

	void (__fastcall *pfnFilter)(RGBA**, RGBA*, RGBA*);
	if (nType == filterMean)
		pfnFilter = calc_mean;
	else if (nType == filterMedian)
		pfnFilter = calc_median;
	else if (nType == filterSharpen)
		pfnFilter = calc_sharpen;
	else if (nType == filterSharpenMore)
		pfnFilter = calc_sharpenmore;
	else if (nType == filterSharpenEdge)
		pfnFilter = calc_sharpenedge;
	else if (nType == filterSoften)
		pfnFilter = calc_soften;
	else if (nType == filterSoftenEdge)
		pfnFilter = calc_softenwoedge;
	else if (nType == filterEmboss)
		pfnFilter = calc_emboss;
	else if (nType == filterDilate)
		pfnFilter = calc_dilate;
	else if (nType == filterErode)
		pfnFilter = calc_erode;
	else if (nType == filterEdgeEnhance)
		pfnFilter = calc_edge_enhance;
	else if (nType == filterEdgeEnhanceMore)
		pfnFilter = calc_edge_enhancemore;
	else if (nType == filterEdgeLaplacian)
		pfnFilter = calc_edge_laplacian;
	else if (nType == filterEdgeSobelHorz)
		pfnFilter = calc_edge_sobelhorz;
	else if (nType == filterEdgeSobelVert)
		pfnFilter = calc_edge_sobelvert;
	else
		return FALSE;

	// 制御をおこなう矩形の計算
	int sx = rc.left;
	int sy = rc.top;
	int dx = rc.Width();
	int dy = rc.Height();

	RGBA* pSrc = (RGBA*)malloc(dx * dy * sizeof(RGBA));
	RGBA* pDst = (RGBA*)malloc(dx * dy * sizeof(RGBA));
	src.getpacket(pSrc, sx, sy, dx, dy);

	RGBA* ar[9];
	register int y, x;
	register RGBA* ps = pSrc;
	register RGBA* pd = pDst;
	for (y = 0; y < dy; y++)
	{
		for (x = 0; x < dx; x++)
		{
			setarray(ps, ar, dx, dy, x, y);
			(*pfnFilter)(ar, ps, pd);
			ps++;
			pd++;
		}
	}

	free(pSrc);
	src.setpacket(pDst, sx, sy, dx, dy);
	free(pDst);

	return TRUE;
}

BOOL filter_custom3x3(mif& src, LPRECT prc, int* pFilter, int nDivide)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;
	src.m_bits = 24;

	// 制御をおこなう矩形の計算
	int sx = rc.left;
	int sy = rc.top;
	int dx = rc.Width();
	int dy = rc.Height();

	if (nDivide == 0)
		nDivide = 1;

	RGBA* pSrc = (RGBA*)malloc(dx * dy * sizeof(RGBA));
	RGBA* pDst = (RGBA*)malloc(dx * dy * sizeof(RGBA));
	src.getpacket(pSrc, sx, sy, dx, dy);

	RGBA* ar[9];
	register int y, x;
	register RGBA* ps = pSrc;
	register RGBA* pd = pDst;
	for (y = 0; y < dy; y++)
	{
		for (x = 0; x < dx; x++)
		{
			setarray(ps, ar, dx, dy, x, y);
			calc_filter(ar, ps, pd, pFilter, nDivide);
			ps++;
			pd++;
		}
	}

	free(pSrc);
	src.setpacket(pDst, sx, sy, dx, dy);
	free(pDst);

	return TRUE;
}
