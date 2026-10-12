//////////////////////////////////////////////////////////////////////
// mezzy_works.cpp

#include "stdafx.h"
#include "ffactory_sup.h"


//////////////////////////////////////////////////////////////////////
//

static void __fastcall Dusty(RGBA* ps, mif& src, int x, int y)
{
// c+(rnd(0,(ctl(0)/4))<<rnd(0,(ctl(1)/4)))
// c+(rnd(0,(ctl(0)/4))<<rnd(0,(ctl(1)/4)))
// c+(rnd(0,(ctl(0)/4))<<rnd(0,(ctl(1)/4)))

	int c;
	c = (int)ps->red;
	c = c+(rnd(0,(ctl(0)/4))<<rnd(0,(ctl(1)/4)));
	ps->red = CLAMP(c);

	c = (int)ps->green;
	c = c+(rnd(0,(ctl(0)/4))<<rnd(0,(ctl(1)/4)));
	ps->green = CLAMP(c);

	c = (int)ps->blue;
	c = c+(rnd(0,(ctl(0)/4))<<rnd(0,(ctl(1)/4)));
	ps->blue = CLAMP(c);
}

static void __fastcall Interlace(RGBA* ps, mif& src, int x, int y)
{
// (c-x*ctl(1)-y*ctl(0))&c
// (c-x*ctl(1)-y*ctl(0))&c
// (c-x*ctl(1)-y*ctl(0))&c

	int c;
	c = (int)ps->red;
	ps->red = ((c-x*ctl(1)-y*ctl(0))&c) & 0xff;

	c = (int)ps->green;
	ps->green = ((c-x*ctl(1)-y*ctl(0))&c) & 0xff;

	c = (int)ps->blue;
	ps->blue = ((c-x*ctl(1)-y*ctl(0))&c) & 0xff;
}

static void __fastcall Mixer(RGBA* ps, mif& src, int x, int y)
{
// rnd(0,ctl(1))-rnd(0,ctl(0))+c
// rnd(0,ctl(2))-rnd(0,ctl(0))+c
// rnd(0,ctl(3))-rnd(0,ctl(0))+c

	int c;
	c = (int)ps->red;
	c = rnd(0,ctl(1))-rnd(0,ctl(0))+c;
	ps->red = CLAMP(c);

	c = (int)ps->green;
	c = rnd(0,ctl(2))-rnd(0,ctl(0))+c;
	ps->green = CLAMP(c);

	c = (int)ps->blue;
	c = rnd(0,ctl(3))-rnd(0,ctl(0))+c;
	ps->blue = CLAMP(c);
}

static void __fastcall Whiten(RGBA* ps, mif& src, int x, int y)
{
// rnd(c,c+4*ctl(0)/4)+ctl(1)
// rnd(c,c+4*ctl(0)/4)+ctl(1)
// rnd(c,c+4*ctl(0)/4)+ctl(1)

	int c;
	c = (int)ps->red;
	c = rnd(c,c+4*ctl(0)/4)+ctl(1);
	ps->red = CLAMP(c);

	c = (int)ps->green;
	c = rnd(c,c+4*ctl(0)/4)+ctl(1);
	ps->green = CLAMP(c);

	c = (int)ps->blue;
	c = rnd(c,c+4*ctl(0)/4)+ctl(1);
	ps->blue = CLAMP(c);
}

static void __fastcall BandingSuppressNoise(RGBA* ps, mif& src, int x, int y)
{
	put(max(ps->red,(max(ps->green,ps->blue))),0);
	put(rnd(0,(ctl(0)*get(0)/512)),1);
	int c;
	c = (int)ps->red-get(1);
	ps->red = CLAMP(c);
	c = (int)ps->green-get(1);
	ps->green = CLAMP(c);
	c = (int)ps->blue-get(1);
	ps->blue = CLAMP(c);
}

static void __fastcall BlackStain(RGBA* ps, mif& src, int x, int y)
{
	put(rnd(0,255)<ctl(0)?ctl(1):255,0);
	int c;
	c = (int)ps->red*get(0)/255;
	ps->red = CLAMP(c);
	c = (int)ps->green*get(0)/255;
	ps->green = CLAMP(c);
	c = (int)ps->blue*get(0)/255;
	ps->blue = CLAMP(c);
}

static void __fastcall BackgroundNoise(RGBA* ps, mif& src, int x, int y)
{
	int i = yuvY(ps);
	int I = 255;
	int C = 255;

	put(rnd(-(I-i),I-i), 0);

	int c;
	c = ps->red;
	c = c+get(0)*ctl(0)/C;
	ps->red = CLAMP(c);

	c = ps->green;
	c = c+get(0)*ctl(0)/C;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = c+get(0)*ctl(0)/C;
	ps->blue = CLAMP(c);
}

static void __fastcall GrayscaleNoise(RGBA* ps, mif& src, int x, int y)
{
	int i = yuvY(ps);
	int C = 255;

	put(rnd(-C/2,C/2),0);
	int c = mix(i+get(0),i,val(0,1,C),C);
	ps->red = ps->green = ps->blue = CLAMP(c);
}

static void __fastcall Vtr(RGBA* ps, mif& src, int x, int y)
{
	int r = ps->red;
	int g = ps->green;
	int b = ps->blue;

	RGBA* p1 = getsrc3(src,x-ctl(3),y);
	int c = ps->red;
	c = ((c2m(0,(y%(ctl(0)/10))) >ctl(1)? c:c+ctl(2)*2-254)+(int)p1->red)/2;
	ps->red = CLAMP(c);

	c = ps->green;
	c = ((c2m(0,(y%(ctl(0)/10))) >ctl(1)? c:c+ctl(2)*2-254)+(int)p1->green)/2;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = ((c2m(0,(y%(ctl(0)/10))) >ctl(1)? c:c+ctl(2)*2-254)+(int)p1->blue)/2;
	ps->blue = CLAMP(c);
}

/////////////////////////////////////////////////////////////////////
//

// for Dusty
static MAGIC parrDusty = {
	_T("埃っぽさ"),
	Dusty,
	2,
	{	{ 128, 128, _T("強度") },
		{  10,  10, _T("コントラスト") },		}
};

// for Interlace
static MAGIC parrInterlace = {
	_T("インタレース"),
	Interlace,
	2,
	{	{ 128, 128, _T("横方向のレース") },
		{   0,   0, _T("縦方向のレース") },		}
};

// for Mixer
static MAGIC parrMixer = {
	_T("ミキサー"),
	Mixer,
	4,
	{	{ 128, 128, _T("コントラスト") },
		{ 128, 128, _T("水色 ←→ 赤") },
		{ 128, 128, _T("紫 ←→ 緑") },
		{ 128, 128, _T("黄 ←→ 青") },		}
};

// for Whiten
static MAGIC parrWhiten = {
	_T("漂白"),
	Whiten,
	2,
	{	{ 128, 128, _T("ノイズ") },
		{ 0, 0, _T("白の割合") },		}
};

// for BandingSuppressNoise
static MAGIC parrBandingSuppressNoise = {
	_T("帯域抑制ノイズ"),
	BandingSuppressNoise,
	1,
	{	{  73,  73, _T("ノイズ") },		}
};

// for BlackStain
static MAGIC parrBlackStain = {
	_T("ブラック ステイン"),
	BlackStain,
	2,
	{	{ 128, 128, _T("密度") },
		{ 0, 0, _T("グレー値") },		}
};

// for BackgroundNoise
static MAGIC parrBackgroundNoise = {
	_T("背景 ノイズ "),
	BackgroundNoise,
	1,
	{	{  71,  71, _T("ノイズレベル") },		}
};

// for GrayscaleNoise
static MAGIC parrGrayscaleNoise = {
	_T("グレースケール ノイズ "),
	GrayscaleNoise,
	1,
	{	{  67,  67, _T("ノイズレベル") },		}
};

// for Vtr
static MAGIC parrVtr = {
	_T("TV ゴースト"),
	Vtr,
	4,
	{	{  26,  26, _T("走査距離") },
		{   0,   0, _T("走査線幅") },
		{ 169, 169, _T("コントラスト") },
		{   2,   2, _T("ゴースト") },		}
};

/////////////////////////////////////////////////////////////////////
//

HANDLE noise_dusty_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrDusty, PREVIEW_PART);
}

HANDLE noise_mixer_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrMixer, PREVIEW_PART);
}

HANDLE noise_whiten_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrWhiten, PREVIEW_PART);
}

HANDLE noise_bandsuppress_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrBandingSuppressNoise, PREVIEW_PART);
}

HANDLE noise_blackstain_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrBlackStain, PREVIEW_PART);
}

HANDLE noise_interlace_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrInterlace, PREVIEW_PART);
}

HANDLE noise_grayscale_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrGrayscaleNoise, PREVIEW_PART);
}

HANDLE noise_background_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrBackgroundNoise, PREVIEW_PART);
}

HANDLE tv_ghost_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrVtr, PREVIEW_WHOLE);
}

