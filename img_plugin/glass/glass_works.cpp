//////////////////////////////////////////////////////////////////////
// glass_works.cpp

#include "stdafx.h"
#include "ffactory_sup.h"

#define MAXPARAMS		8

static void __fastcall Reflection(RGBA* ps, mif& src, int x, int y)
{
/* @@@
	put(x%val(0,1,M/2),0);
	put(y%val(1,1,M/2),1);
	RGBA* p1 = getsrc3(src,x+get(0),y+get(1));
	*ps = *p1;
@@@ */
	put(x%val(0,1,255),0);
	put(y%val(1,1,255),1);
	getsrc_linear(src,x+get(0),y+get(1),ps);
}


static void __fastcall TiledGlass(RGBA* ps, mif& src, int x, int y)
{
/* @@
	put((itan(val(0,0,64)*x)-itan(val(1,0,64)*y))/val(2,1,128),0);
	RGBA* p1 = getsrc3(src,x-get(0),y-get(0));
	*ps = *p1;
@@ */
	double n0 = (double)(itan2(val(0,48,0)*x)-itan2(val(1,48,0)*y))/dval(2,128,2);
	getsrc_linear(src,x-n0,y-n0,ps);
}

static void __fastcall GlassTint(RGBA* ps, mif& src, int x, int y)
{
	put(x^ctl(0)/20,0);
	put(y^ctl(1)/20,1);

	RGBA* p1 = getsrc3(src,get(0),get(1));

	int c;
	c = (int)p1->red + 128 - ctl(2);
	ps->red = CLAMP(c);

	c = (int)p1->green + 128 - ctl(3);
	ps->green = CLAMP(c);

	c = (int)p1->blue + 128 - ctl(4);
	ps->blue = CLAMP(c);
}

static void __fastcall BitchesCrystal(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,1,X),2);
	put(val(1,1,Y),3);
	put(x%get(2),0);
	put(y%get(3),1);

	RGBA* p1 = getsrc3(src,(x-get(0)+get(2)/2+X)%X,(y-get(1)+get(3)/2+Y)%Y);
	RGBA* p2 = getsrc3(src,(x+get(0)-get(2)/2+X)%X,(y+get(1)-get(3)/2+Y)%Y);

	int c;
	put(p1->red,4);
	put(p2->red,5);
	c = mix(get(4),get(5),get(0)*get(1),get(2)*get(3));
	ps->red = CLAMP(c);

	put(p1->green,4);
	put(p2->green,5);
	c = mix(get(4),get(5),get(0)*get(1),get(2)*get(3));
	ps->green = CLAMP(c);

	put(p1->blue,4);
	put(p2->blue,5);
	c = mix(get(4),get(5),get(0)*get(1),get(2)*get(3));
	ps->blue = CLAMP(c);
}

static void __fastcall GlassSlices(RGBA* ps, mif& src, int x, int y)
{
	put(get(1)==val(1,0,Y)?0:get(1)+1,1);
	put(get(1)/val(0,1,Y),2);
	RGBA* p1 = getsrc3(src,x-get(2),y-get(2));
	*ps = *p1;
}

static void __fastcall MosaicRipple(RGBA* ps, mif& src, int x, int y)
{
	put(x%val(0,1,X),1);
	put((get(1)+y%val(0,1,X))*val(1,0,1024)/M*2,0);
	getsrc_linear(src,x+dr2x(get(0),get(1)),y+dr2y(get(0),get(1)),ps);
}

static void __fastcall GlassPane(RGBA* ps, mif& src, int x, int y)
{
	put(scl(x,0,X,0,512),4);
	put(scl(y,0,Y,0,512),5);
	put(512-val(2,-200,1220),6);
	put(512-val(3,-200,1220),7);
	put(get(6)?itan(get(4)*1024/val(0,380,3))/get(6):1,0);
	put(get(7)?itan(get(5)*1024/val(1,380,3))/get(7):1,1);

	RGBA* p1 = getsrc3(src,get(0),get(1));
	RGBA* p2 = getsrc3(src,get(1),get(0));

	put(dif(p1->green,p2->blue),2);
	put(dif(c2m(get(0)+get(1),get(2)),220),3);
	RGBA* p3 = getsrc3(src,x+get(0)/2,y+get(1)/2);

	int c;
	c = ((int)p3->red-get(3))+150;
	ps->red = CLAMP(c);

	c = ((int)p3->green-get(3))+150;
	ps->green = CLAMP(c);

	c = ((int)p3->blue-get(3))+150;
	ps->blue = CLAMP(c);
}

static void __fastcall Legoland(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,1,X),3);
	put(y%get(3)<y%(get(3)*2)&&ctl(6)>0?x:x+val(3,0,get(3)/2),2);
	put(c2d(get(2)%get(3),get(3)-y%get(3)),0);
	put(c2m(abs(get(3)/2-get(2)%get(3)),abs(get(3)/2-y%get(3))),1);

	if (get(1)<val(1,0,get(3)))
	{
		RGBA p1;
		getsrc_linear(src,x,y+scl(get(0),0,255,0,val(2,0,Y)),&p1);
		int c;
		c = p1.red+get(0)-128;
		ps->red = CLAMP(c);
		c = p1.green+get(0)-128;
		ps->green = CLAMP(c);
		c = p1.blue+get(0)-128;
		ps->blue = CLAMP(c);
	}
}

static void __fastcall DiamondGlass(RGBA* ps, mif& src, int x, int y)
{
/* @@@
	int d = c_dir(x, y);
	int m = c_dist(x, y);
	int n = val(2,1,256);
	put(isin(x*ctl(0)),0);
	put(isin(y*ctl(1)),1);
	put(isin(m)-isin(d)-get(1)-get(0),2);
	RGBA* p1 = getsrc3(src,x-get(2)/n,y-get(2)/n);
	*ps = *p1;
@@@ */
	double d = dc_dir(x, y);
	double m = dc_dist(x, y);
	double n = dval(2,1,256);
	double n0 = dsin(x*ctl(0));
	double n1 = dsin(y*ctl(1));
	double n2 = dsin(m)-dsin(d)-n1-n0;
	getsrc_linear(src,x-n2/n,y-n2/n,ps);
}

static void __fastcall Sectorize(RGBA* ps, mif& src, int x, int y)
{
	int d = c_dir(x, y);
	int m = c_dist(x, y);
//	getsrc_linear(src,abs(x+m%val(0,64,1)+abs(d)%val(1,64,1)),abs(y+m%val(0,64,1)+abs(d)%val(1,64,1)),ps);
	getsrc_linear(src,abs(x+m%val(0,X/4,1)+abs(d)%val(1,X/4,1)),abs(y+m%val(0,Y/4,1)+abs(d)%val(1,Y/4,1)),ps);
}

//////////////////////////////////////////////////////////////////////
//

// for GlassTint
static MAGIC parrGlassTint = {
	_T("ガラス ティント"),
	GlassTint,
	5,
	{	{ 128, 128, _T("横方向のカット") },
		{ 128, 128, _T("縦方向のカット") },
		{ 128, 128, _T("赤 ←→ 水色") },
		{ 128, 128, _T("緑 ←→ 紫") },
		{ 128, 128, _T("青 ←→ 黄") },		}
};

// for BitchesCrystal
static MAGIC parrBitchesCrystal = {
	_T("クリスタル"),
	BitchesCrystal,
	2,
	{	{  37,  37, _T("セルの幅") },
		{  39,  39, _T("セルの高さ") },	}
};

// for TiledGlass
static MAGIC parrTiledGlass = {
	_T("タイル ガラス 1"),
	TiledGlass,
	3,
	{	{ 158, 158, _T("タイルの幅") },
		{ 158, 158, _T("タイルの高さ") },
		{ 200, 200, _T("ガラスの厚み") },		}
};

// for GlassPane
static MAGIC parrGlassPane = {
	_T("タイル ガラス 2"),
	GlassPane,
	4,
	{	{ 140, 140, _T("タイルの幅") },
		{ 140, 140, _T("タイルの高さ") },
		{ 106, 106, _T("水平ベベル") },
		{ 106, 106, _T("垂直ベベル") },		}
};

// for GlassSlices
static MAGIC parrGlassSlices = {
	_T("ガラス スライス"),
	GlassSlices,
	2,
	{	{   8,   8, _T("効果") },
		{  60,  60, _T("スライシング") },		}
};

// for MosaicRipple
static MAGIC parrMosaicRipple = {
	_T("ガラス リプル"),
	MosaicRipple,
	2,
	{	{  22,  22, _T("サイズ") },
		{  60,  60, _T("ゆがみ") },		}
};

// for Reflection
static MAGIC parrReflection = {
	_T("リフレクション"),
	Reflection,
	2,
	{	{  32,  32, _T("水平方向分割数") },
		{  32,  32, _T("垂直方向分割数") },	}
};

// for Legoland
static MAGIC parrLegoland = {
	_T("ガラス レゴランド"),
	Legoland,
	4,
	{	{  14,  14, _T("ポイント") },
		{ 126, 126, _T("半径") },
		{   7,   7, _T("ゆがみ") },
		{   0,   0, _T("オーダー") },		}
};

// for DiamondGlass
static MAGIC parrDiamondGlass = {
	_T("ダイヤモンド ガラス"),
	DiamondGlass,
	3,
	{	{ 161, 161, _T("横方向のゆがみ") },
		{ 167, 167, _T("縦方向のゆがみ") },
		{ 179, 179, _T("適用度") },		}
};

// for Sectorize
static MAGIC parrSectorize = {
	_T("セクタライズ"),
	Sectorize,
	2,
	{	{ 213, 213, _T("リング数") },
		{ 234, 234, _T("分布度") },		}
};

//////////////////////////////////////////////////////////////////////
//

HANDLE glass_reflection_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrReflection, PREVIEW_PART);
}

HANDLE glass_tiledglass_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrTiledGlass, PREVIEW_PART);
}

HANDLE glass_glasspane_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrGlassPane, PREVIEW_WHOLE);
}

HANDLE glass_glasstint_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrGlassTint, PREVIEW_PART);
}

HANDLE glass_bitchescrystal_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrBitchesCrystal, PREVIEW_WHOLE);
}

HANDLE glass_glassslices_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrGlassSlices, PREVIEW_BOTH);
}

HANDLE glass_mosaicripple_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrMosaicRipple, PREVIEW_WHOLE);
}

HANDLE glass_legoland_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrLegoland, PREVIEW_WHOLE);
}

HANDLE glass_diamondglass_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrDiamondGlass, PREVIEW_PART);
}

HANDLE glass_sectorize_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSectorize, PREVIEW_PART);
}

