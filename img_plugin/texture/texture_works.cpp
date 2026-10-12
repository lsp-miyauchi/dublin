//////////////////////////////////////////////////////////////////////
// texture_works.cpp

#include "stdafx.h"
#include "ffactory_sup.h"

#define MAXPARAMS		8

static void __fastcall WeaveDistortion(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,1,min(X,Y)),8);
	put(val(1,0,get(8)/2),1);
	put(x,10);
	put(y,11);
	put((get(10)%get(8)<get(10)%(get(8)*2))^(get(11)%get(8)<get(11)%(get(8)*2)),9);
	put(dif(get(10+get(9))%get(8),get(8)/2),4);
	put(dif(get(11-get(9))%get(8),get(8)/2),5);
	put(get(4)<get(1)?scl(get(5),0,get(8)/2,255,ctl(5)):scl(get(4),get(1),get(8)/2,0,ctl(5)),3);

	RGBA* p1 = getsrc3(src,x,y+(get(3)-128)/32);
	int c;
	put(p1->red,20);
	c=(get(5)<get(1))||(get(4)<get(1))?(get(3)*get(20)/255)+(255-((255-get(20)))*(255-get(3))/255)/2:ctl(2);
	ps->red = CLAMP(c);

	put(p1->green,20);
	c=(get(5)<get(1))||(get(4)<get(1))?(get(3)*get(20)/255)+(255-((255-get(20)))*(255-get(3))/255)/2:ctl(3);
	ps->green = CLAMP(c);

	put(p1->blue,20);
	c=(get(5)<get(1))||(get(4)<get(1))?(get(3)*get(20)/255)+(255-((255-get(20)))*(255-get(3))/255)/2:ctl(4);
	ps->blue = CLAMP(c);
}

static void __fastcall CrossWeaveDistortion(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,1,min(X,Y)),8);
	put(val(1,0,get(8)/2),1);
	put(x+y,10);
	put(dif(x+X,y+Y),11);
	put((get(10)%get(8)<get(10)%(get(8)*2))^(get(11)%get(8)<get(11)%(get(8)*2)),9);
	put(dif(get(10+get(9))%get(8),get(8)/2),4);
	put(dif(get(11-get(9))%get(8),get(8)/2),5);
	put(get(4)<get(1)?scl(get(5),0,get(8)/2,255,ctl(5)):scl(get(4),get(1),get(8)/2,0,ctl(5)),3);

	int c;
	c = ps->red;
	c = (get(5)<get(1))||(get(4)<get(1))?(get(3)*c/255)+(255-((255-c))*(255-get(3))/255)/2:ctl(2);
	ps->red = CLAMP(c);

	c = ps->green;
	c = (get(5)<get(1))||(get(4)<get(1))?(get(3)*c/255)+(255-((255-c))*(255-get(3))/255)/2:ctl(3);
	ps->green = CLAMP(c);

	c = ps->blue;
	c = (get(5)<get(1))||(get(4)<get(1))?(get(3)*c/255)+(255-((255-c))*(255-get(3))/255)/2:ctl(4);
	ps->blue = CLAMP(c);
}

static void __fastcall WeaveTexture(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,0,255),0);
	put(val(1,1,10),1);
	put(x%(get(0)*2),2);
	put(y%(get(0)*2),3);
	int c = ps->red;
	c = (get(2)<get(0)&&get(3)<get(0))||(get(2)>(get(0)-1)&&get(3)>(get(0)-1))?
			(y%get(0))<(get(0)/get(1))?
			(y%get(0))*(256*get(1)/get(0))*c/256:
			(y%get(0)>(get(0)-1-get(0)/get(1)))?
			(get(0)-y%get(0))*(256*get(1)/get(0))*c/256:c:
			(x%get(0))<(get(0)/get(1))?(x%get(0))*(256*get(1)/get(0))*c/256:
			(x%get(0)>(get(0)-1-get(0)/get(1)))?
			(get(0)-x%get(0))*(256*get(1)/get(0))*c/256:c;
	ps->red = CLAMP(c);

	c = ps->green;
	c = (get(2)<get(0)&&get(3)<get(0))||(get(2)>(get(0)-1)&&get(3)>(get(0)-1))?
			(y%get(0))<(get(0)/get(1))?
			(y%get(0))*(256*get(1)/get(0))*c/256:
			(y%get(0)>(get(0)-1-get(0)/get(1)))?
			(get(0)-y%get(0))*(256*get(1)/get(0))*c/256:c:
			(x%get(0))<(get(0)/get(1))?(x%get(0))*(256*get(1)/get(0))*c/256:
			(x%get(0)>(get(0)-1-get(0)/get(1)))?
			(get(0)-x%get(0))*(256*get(1)/get(0))*c/256:c;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = (get(2)<get(0)&&get(3)<get(0))||(get(2)>(get(0)-1)&&get(3)>(get(0)-1))?
			(y%get(0))<(get(0)/get(1))?
			(y%get(0))*(256*get(1)/get(0))*c/256:
			(y%get(0)>(get(0)-1-get(0)/get(1)))?
			(get(0)-y%get(0))*(256*get(1)/get(0))*c/256:c:
			(x%get(0))<(get(0)/get(1))?(x%get(0))*(256*get(1)/get(0))*c/256:
			(x%get(0)>(get(0)-1-get(0)/get(1)))?
			(get(0)-x%get(0))*(256*get(1)/get(0))*c/256:c;
	ps->blue = CLAMP(c);
}

static void __fastcall Pyramids(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,1,255),3);
//	put((x*255/X)%get(3),1);
//	put((y*255/Y)%get(3),2);
	put(x%get(3),1);
	put(y%get(3),2);
	put(ctl(0)-get(1)<=get(2)?get(1)>=get(2)?ctl(4):ctl(5):get(1)<=get(2)?ctl(3):ctl(2),0);

	int c;
	c = ps->red;
	c = mix(c,get(0),ctl(1),255);
	ps->red = CLAMP(c);

	c = ps->green;
	c = mix(c,get(0),ctl(1),255);
	ps->green = CLAMP(c);

	c = ps->blue;
	c = mix(c,get(0),ctl(1),255);
	ps->blue = CLAMP(c);
}

static void __fastcall PanelStripes(RGBA* ps, mif& src, int x, int y)
{
//	int n = val(0,0,X);
	int n = val(0,1,X);

	int c;
	c = ps->red;
	c = x%n<=val(2,0,val(0,0,X/2))?scl(c,0,255,0,scl(x%n,0,val(2,0,n),val(1,255,0),255)):n-x%n<=val(2,0,n)?scl(c,0,255,255-scl(n-x%n,0,val(2,0,n),val(1,255,0),255),255):c;
	ps->red = CLAMP(c);

	c = ps->green;
	c = x%n<=val(2,0,val(0,0,X/2))?scl(c,0,255,0,scl(x%n,0,val(2,0,n),val(1,255,0),255)):n-x%n<=val(2,0,n)?scl(c,0,255,255-scl(n-x%n,0,val(2,0,n),val(1,255,0),255),255):c;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = x%n<=val(2,0,val(0,0,X/2))?scl(c,0,255,0,scl(x%n,0,val(2,0,n),val(1,255,0),255)):n-x%n<=val(2,0,n)?scl(c,0,255,255-scl(n-x%n,0,val(2,0,n),val(1,255,0),255),255):c;
	ps->blue = CLAMP(c);
}

static void __fastcall SquareBumps(RGBA* ps, mif& src, int x, int y)
{
	int C = 255;

	put(X/val(0, 1, 64), 11);
	put(scl(x%get(11), 0, get(11), 0, C+val(1, -C, C)), 13);
	put(scl(y%get(11), 0, get(11), 0, C+val(1, -C, C)), 14);

	int c;
	c = ps->red;
	c = c-(get(13)+get(14))/2;
	ps->red = CLAMP(c);

	c = ps->green;
	c = c-(get(13)+get(14))/2;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = c-(get(13)+get(14))/2;
	ps->blue = CLAMP(c);
}

static void __fastcall GridTexture(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,1,X),0);
	put(val(0,1,Y),1);

	int c;
	c = ps->red;
	c = (x-1)%get(0)==0 || ((y-1)%get(1)==0) ? c*ctl(1)/256:c &&(x)%get(0)==0 || ((y)%get(1)==0) ? c*ctl(1)/256:c && (x+1)%get(0)==0 || ((y+1)%get(1)==0) ? 255-((255-c)*(ctl(1)))/256:c;
	ps->red = CLAMP(c);

	c = ps->green;
	c = (x-1)%get(0)==0 || ((y-1)%get(1)==0) ? c*ctl(1)/256:c &&(x)%get(0)==0 || ((y)%get(1)==0) ? c*ctl(1)/256:c && (x+1)%get(0)==0 || ((y+1)%get(1)==0) ? 255-((255-c)*(ctl(1)))/256:c;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = (x-1)%get(0)==0 || ((y-1)%get(1)==0) ? c*ctl(1)/256:c &&(x)%get(0)==0 || ((y)%get(1)==0) ? c*ctl(1)/256:c && (x+1)%get(0)==0 || ((y+1)%get(1)==0) ? 255-((255-c)*(ctl(1)))/256:c;
	ps->blue = CLAMP(c);
}

static void __fastcall Stardust(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,1,X),9);
	put(x%get(9),0);
	put(y%get(9),1);
	put(abs(itan((x-get(0)+ctl(6)+1)*itan(ctl(6)+y-get(1))))%255,2);
	put(scl(get(2),0,255,0,get(9)/2),4);
	put(min(get(4),dif(get(9),get(4))),6);
	put(scl(isin(val(5,0,32)*c2d(get(4)-get(0),get(4)-get(1))),-512,512,0,val(4,0,-get(9))),8);
	put(c2m(get(4)-get(0),get(4)-get(1))-get(8),7);

	int c;
	c = ps->red;
	c = get(7)<get(6)?scl(get(7),0,get(6),ctl(1),c):c;
	ps->red = CLAMP(c);

	c = ps->green;
	c = get(7)<get(6)?scl(get(7),0,get(6),ctl(2),c):c;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = get(7)<get(6)?scl(get(7),0,get(6),ctl(3),c):c;
	ps->blue = CLAMP(c);
}

static void __fastcall Sparkle(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,1,X),5);
	put(x%get(5),8);
	put(y%get(5),9);
	put(abs(20*itan((x-get(8)+(y-get(9))*300+ctl(5))))%get(5),0);
	put(abs(25*itan((x-get(8)+(y-get(9))*222+ctl(5))))%get(5),1);
	put(min(get(0),min(get(5)-get(0),min(get(5)-get(1),get(1)))),4);

	int c;
	c = ps->red;
	put(ctl(4)<128?itan((x-get(8)+(y-get(9))*222))&255:ctl(1),7);
	put(get(8)==get(0)?scl(dif(get(9),get(1)),0,get(4),get(7),0):0,2);
	put(get(9)==get(1)?scl(dif(get(8),get(0)),0,get(4),get(7),0):0,3);
	c = 255-((255-c)*(255-max(get(2),get(3)))/255);
	ps->red = CLAMP(c);

	c = ps->green;
	put(ctl(4)<128?itan((x-get(8)+(y-get(9))*222))&255:ctl(2),7);
	put(get(8)==get(0)?scl(dif(get(9),get(1)),0,get(4),get(7),0):0,2);
	put(get(9)==get(1)?scl(dif(get(8),get(0)),0,get(4),get(7),0):0,3);
	c = 255-((255-c)*(255-max(get(2),get(3)))/255);
	ps->green = CLAMP(c);

	c = ps->blue;
	put(ctl(4)<128?itan((x-get(8)+(y-get(9))*222))&255:ctl(3),7);
	put(get(8)==get(0)?scl(dif(get(9),get(1)),0,get(4),get(7),0):0,2);
	put(get(9)==get(1)?scl(dif(get(8),get(0)),0,get(4),get(7),0):0,3);
	c = 255-((255-c)*(255-max(get(2),get(3)))/255);
	ps->blue = CLAMP(c);
}

static void __fastcall PoolShadow(RGBA* ps, mif& src, int x, int y)
{
	int d = c_dir(x, y);
	int m = c_dist(x, y);

	put((isin((y+x)*ctl(0)*16/M)*32+isin((y*ctl(2)*8+m*2)/M)*32+isin(m*ctl(1)*16/M)*32),1);
	put((icos((x-y)*ctl(3)*16/M-icos(ctl(0)+x*ctl(5)*16/M))*32-icos(y*ctl(4)*16/M)*32),2);
	put((get(1)+get(2))*ctl(6)/256/128+ctl(7)+128,0);

	int c;
	c = ps->red;
	c = c*get(0)/255;
	ps->red = CLAMP(c);

	c = ps->green;
	c = c*get(0)/255;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = c*get(0)/255;
	ps->blue = CLAMP(c);
}

static void __fastcall BedSheets(RGBA* ps, mif& src, int x, int y)
{
//@	put(scl(isin(val(4,512,-512)+scl(x+y,0,val(0,X*2,X/10),-512,512)),512,-512,val(1,128,0),val(1,128,255)),0);
//@	put(scl(isin(scl(y+get(0),0,val(2,Y*2,Y/10),-512,512)),512,-512,val(3,128,0),val(3,128,255)),10);

	put(scl(x,0,X,0,512),4);
	put(scl(y,0,Y,0,512),5);
	put(scl(isin(val(4,512,-512)+scl(get(4)+get(5),0,val(0,512,52),-512,512)),512,-512,val(1,128,0),val(1,128,255)),0);
	put(scl(isin(scl(get(5)+get(0),0,val(2,512,52),-512,512)),512,-512,val(3,128,0),val(3,128,255)),10);

	int c;
	c = ps->red;
	c = scl(c,0,128,0,get(10))*(get(10)<=127) + scl(c,128,255,get(10),255)*(get(10)>127);
	ps->red = CLAMP(c);

	c = ps->green;
	c = scl(c,0,128,0,get(10))*(get(10)<=127) + scl(c,128,255,get(10),255)*(get(10)>127);
	ps->green = CLAMP(c);

	c = ps->blue;
	c = scl(c,0,128,0,get(10))*(get(10)<=127) + scl(c,128,255,get(10),255)*(get(10)>127);
	ps->blue = CLAMP(c);
}


//////////////////////////////////////////////////////////////////////
//


// for WeaveDistortion
static MAGIC parrWeaveDistortion = {
	_T("デジタル織り込み"),
	WeaveDistortion,
	6,
	{	{  80,  80, _T("サイズ") },
		{ 164, 164, _T("ギャップ幅") },
		{   0,   0, _T("影 赤") },
		{   0,   0, _T("影 緑") },
		{   0,   0, _T("影 青") },
		{ 128, 128, _T("影の強さ") },		}
};

// for CrossWeaveDistortion
static MAGIC parrCrossWeaveDistortion = {
	_T("デジタル織り込み クロス"),
	CrossWeaveDistortion,
	6,
	{	{  66,  66, _T("サイズ") },
		{ 164, 164, _T("ギャップ幅") },
		{   0,   0, _T("影 赤") },
		{   0,   0, _T("影 緑") },
		{   0,   0, _T("影 青") },
		{ 128, 128, _T("影の強さ") },		}
};

// for WeaveTexture
static MAGIC parrWeaveTexture = {
	_T("デジタル織り込み シェード"),
	WeaveTexture,
	2,
	{	{  66,  66, _T("バンド幅") },
		{ 128, 128, _T("明るさ") },		}
};

// for Pyramids
static MAGIC parrPyramids = {
	_T("ピラミッド"),
	Pyramids,
	6,
	{	{  18,  18, _T("グリッドサイズ") },
		{ 147, 147, _T("透明度") },
		{ 227, 227, _T("明るさ（上）") },
		{ 193, 193, _T("明るさ（左）") },
		{  86,  86, _T("明るさ（右）") },
		{  29,  29, _T("明るさ（下）") },		}
};

// for PanelStripes
static MAGIC parrPanelStripes = {
	_T("パネル ストライプ"),
	PanelStripes,
	3,
	{	{  31,  31, _T("パネル密度") },
		{ 132, 132, _T("テクスチャ深度") },
		{  70,  70, _T("テクスチャ幅") },		}
};

// for SquareBumps
static MAGIC parrSquareBumps = {
	_T("スクエア バンプ"),
	SquareBumps,
	2,
	{	{  48,  48, _T("密度") },
		{  36,  36, _T("フェード") },	}
};

// for GridTexture
static MAGIC parrGridTexture = {
	_T("グリッド テクスチャ"),
	GridTexture,
	2,
	{	{  16,  16, _T("グリッド密度") },
		{ 158, 158, _T("濃度") },		}
};

// for Stardust
static MAGIC parrStardust = {
	_T("スターダスト"),
	Stardust,
	7,
	{	{  20,  20, _T("頻度") },
		{ 223, 223, _T("赤") },
		{ 225, 225, _T("緑") },
		{ 226, 226, _T("青") },
		{  56,  56, _T("シャープさ") },
		{  86,  86, _T("スパイク数") },
		{  70,  70, _T("ランダムベース") },		}
};

// for Sparkle
static MAGIC parrSparkle = {
	_T("スパークル"),
	Sparkle,
	6,
	{	{  42,  42, _T("最大サイズ") },
		{ 175, 175, _T("赤") },
		{ 140, 140, _T("緑") },
		{ 213, 213, _T("青") },
		{ 177, 177, _T("ランダム ON / OFF") },
		{  86,  86, _T("ランダムベース") },		}
};

// for PoolShadow
static MAGIC parrPoolShadow = {
	_T("プール シャドウ"),
	PoolShadow,
	8,
	{	{ 151, 151, _T("制御 1") },
		{ 208, 208, _T("制御 2") },
		{ 118, 118, _T("制御 3") },
		{ 146, 146, _T("制御 4") },
		{  79,  79, _T("制御 5") },
		{ 192, 192, _T("制御 6") },
		{ 131, 131, _T("影の強度") },
		{ 147, 147, _T("明るさ") },		}
};

// for BedSheets
static MAGIC parrBedSheets = {
	_T("シーツのうねり"),
	BedSheets,
	5,
	{	{  94,  94, _T("縦横サイクル") },
		{ 136, 136, _T("縦横シェーディング") },
		{  34,  34, _T("横サイクル") },
		{ 179, 179, _T("横シェーディング") },
		{   0,   0, _T("オフセット") },			}
};



/////////////////////////////////////////////////////////////////////
//


HANDLE texture_weavedistortion_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrWeaveDistortion, PREVIEW_WHOLE);
}

HANDLE texture_crossweavedistortion_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrCrossWeaveDistortion, PREVIEW_WHOLE);
}

HANDLE texture_weavetexture_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrWeaveTexture, PREVIEW_WHOLE);
}

HANDLE pyramids_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrPyramids, PREVIEW_PART);
}

HANDLE panel_stripes_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrPanelStripes, PREVIEW_WHOLE);
}

HANDLE square_bumps_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSquareBumps, PREVIEW_WHOLE);
}

HANDLE gridtexture_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrGridTexture, PREVIEW_WHOLE);
}

HANDLE stardust_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrStardust, PREVIEW_WHOLE);
}

HANDLE sparkle_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSparkle, PREVIEW_WHOLE);
}

HANDLE texture_poolshadow_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrPoolShadow, PREVIEW_WHOLE);
}

HANDLE texture_bedsheets_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrBedSheets, PREVIEW_WHOLE);
}
