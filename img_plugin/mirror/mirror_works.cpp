//////////////////////////////////////////////////////////////////////
// mirror_works.cpp

#include "stdafx.h"
#include "ffactory_sup.h"

#define MAXPARAMS		8


static void __fastcall SeamlessWelder(RGBA* ps, mif& src, int x, int y)
{
	int C = 255;
	put(min(isin(scl(x,0,X,0,512))*X/2/(val(0,1,X)),C),0);
	put(min(isin(scl(y,0,Y,0,512))*Y/2/(val(0,1,Y)),C),1);
	RGBA* p1 = getsrc3(src,X-x,y);
	RGBA* p2 = getsrc3(src,x,Y-y);
	int c;
	c = ps->red;
	c = (mix(c,p1->red,get(0),C)+mix(c,p2->red,get(1),C))/2;
	ps->red = CLAMP(c);
	c = ps->green;
	c = (mix(c,p1->green,get(0),C)+mix(c,p2->green,get(1),C))/2;
	ps->green = CLAMP(c);
	c = ps->blue;
	c = (mix(c,p1->blue,get(0),C)+mix(c,p2->blue,get(1),C))/2;
	ps->blue = CLAMP(c);
}

static void __fastcall QuadFlip(RGBA* ps, mif& src, int x, int y)
{
	int m = c_dist(x,y);
	put(val(1,-255,255),0);
	put(val(2,-255,255),1);
	put(scl(m,0,M,0,255),2);

	RGBA p1;
//	getsrc_linear(src,(-(x>X/2)*X)+x*2,(-(y>Y/2)*Y)+y*2,&p1);

	put(ctl(3)%128<64?(-(x>X/2)*X)+x*2:x<X/2?x*2:X-(x-X/2)*2,3);
	put(ctl(3)/128<1?(-(y>Y/2)*Y)+y*2:y<Y/2?y*2:Y-(y-Y/2)*2,4);
	getsrc_linear(src,get(3),get(4),&p1);

	int c;
	c = ps->red;
	c = mix(c+get(0),p1.red+get(1),ctl(0)-get(2),128);
	ps->red = CLAMP(c);
	c = ps->green;
	c = mix(c+get(0),p1.green+get(1),ctl(0)-get(2),128);
	ps->green = CLAMP(c);
	c = ps->blue;
	c = mix(c+get(0),p1.blue+get(1),ctl(0)-get(2),128);
	ps->blue = CLAMP(c);
}

static void __fastcall Perverse(RGBA* ps, mif& src, int x, int y)
{
	RGBA* p1 = getsrc3(src,abs(x-ctl(0)*X/256),abs(y-ctl(1)*Y/256));
	RGBA* p2 = getsrc3(src,abs(y-ctl(0)*X/256),abs(x-ctl(1)*Y/256));

	ps->red = CLAMP((int)p1->red+(int)p2->red-ctl(2));
	ps->green = CLAMP((int)p1->green+(int)p2->green-ctl(2));
	ps->blue = CLAMP((int)p1->blue+(int)p2->blue-ctl(2));
}

static void __fastcall Promille(RGBA* ps, mif& src, int x, int y)
{
	put(x-ctl(0)*X/256,0);
	put(x-ctl(2)*X/256,1);
	put(y-ctl(1)*Y/256,2);
	put(y-ctl(3)*Y/256,3);

	RGBA* p1 = getsrc3(src,abs(get(0)),abs(get(2)));
	RGBA* p2 = getsrc3(src,abs(get(1)),abs(get(3)));
	ps->red = CLAMP((int)p1->red + (int)p2->red - ctl(4));
	ps->green = CLAMP((int)p1->green + (int)p2->red - ctl(4));
	ps->blue = CLAMP((int)p1->blue + (int)p2->red - ctl(4));
}

static void __fastcall RadialShoved(RGBA* ps, mif& src, int x, int y)
{
	int d = c_dir(x, y);
	int m = c_dist(x, y);
	int n = val(0,1,10);
	RGBA p1, p2;
	getsrc_linear(src,d,m+n,&p1);
	getsrc_linear(src,d,m-n,&p2);
	ps->red = ((int)ps->red+(int)p1.red+(int)p2.red)/3;
	ps->green = ((int)ps->green+(int)p1.green+(int)p2.green)/3;
	ps->blue = ((int)ps->blue+(int)p1.blue+(int)p2.blue)/3;
/* @@@
	double d = dc_dir(x, y);
	double m = dc_dist(x, y);
	double n = dval(0,1,10);
	RGBA p1, p2;
	getsrc_linear(src,d,m+n,&p1);
	getsrc_linear(src,d,m-n,&p2);
	ps->red = ((int)ps->red+(int)p1.red+(int)p2.red)/3;
	ps->green = ((int)ps->green+(int)p1.green+(int)p2.green)/3;
	ps->blue = ((int)ps->blue+(int)p1.blue+(int)p2.blue)/3;
@@@ */
}

static void __fastcall KaleidoscopeI(RGBA* ps, mif& src, int x, int y)
{
	int d = c_dir(x, y);
	int m = c_dist(x, y);
	int C = 255;
	int D = 1024;

	put(0,10);
	put(85,11);
	put(127,12);
	put(153,13);
	put(170,14);
	put(182,15);
	put(191,16);
	put(198,17);
	put(204,18);
	put(209,19);
	put(212,20);
	put(216,21);
	put(221,22);
	put(225,23);
	put(228,24);
	put(236,25);

	put((d+512)%D,0);
	put(C-get(val(0,10,25)),1);
	put(get(0)%get(1)+val(1,-512,512),2);
	put(get(0)%get(1)<get(0)%(get(1)*2),3);
	put(get(1)-get(0)%get(1)+val(1,-512,512),4);

	put(get(3)?get(2):get(4),5);
	rad(src,get(5),m,ps);
}

static void __fastcall KaleidoscopeII(RGBA* ps, mif& src, int x, int y)
{
	int d = c_dir(x, y);
	int m = c_dist(x, y);
	int C = 255;
	int D = 1024;

	put(0,10);
	put(85,11);
	put(127,12);
	put(153,13);
	put(170,14);
	put(182,15);
	put(191,16);
	put(198,17);
	put(204,18);
	put(209,19);
	put(212,20);
	put(216,21);
	put(221,22);
	put(225,23);
	put(228,24);
	put(236,25);

	put((d+512)%D,0);
	put(C-get(val(0,10,25)),1);
	put(get(0)%get(1)+val(1,-512,512),2);
	put(get(0)%get(1)<get(0)%(get(1)*2),3);
	put(get(1)+get(0)%get(1)+val(1,-512,512),4);

	put(get(3)?get(2):get(4),5);
	rad(src,get(5),m,ps);
}

static void __fastcall KaleidoscopeIII(RGBA* ps, mif& src, int x, int y)
{
	int d = c_dir(x, y);
	int m = c_dist(x, y);

	put(1<<val(0,0,9),0);
	RGBA* p1 = getsrc3(src,(d+512)%get(0)+val(1,-512,512),m);
	RGBA* p2 = getsrc3(src,get(0)-((d+512)%get(0))+val(1,-512,512),m);
	*ps = (d+512)%get(0)<(d+512)%(get(0)*2) ? *p1 : *p2;
}

static void __fastcall KaleidoscopeIV(RGBA* ps, mif& src, int x, int y)
{
	int d = c_dir(x, y);
	double m = dc_dist(x, y);
	rad(src,((ctl(2)*4)+(512+d))%(1024/val(0,1,60))+ctl(1)*4,m,ps);
}

static void __fastcall RingMirror(RGBA* ps, mif& src, int x, int y)
{
	int m = c_dist(x, y);
//	getsrc_linear(src,abs(x+m%val(0,64,0)),abs(y+m%val(0,64,0)),ps);
	getsrc_linear(src,abs(x+m%val(0,X/4,1)),abs(y+m%val(0,Y/4,1)),ps);
}

static void __fastcall DistortionMirrorHorz(RGBA* ps, mif& src, int x, int y)
{
	double m = dc_dist(x, y);
	dput((X-m*ctl(0)/32.0), 0);
	getsrc_linear(src,dget(0),y,ps);
}

static void __fastcall DistortionMirrorVert(RGBA* ps, mif& src, int x, int y)
{
	double m = dc_dist(x, y);

	dput((Y-m*ctl(0)/32.0), 0);
	getsrc_linear(src,x,dget(0),ps);
}

//////////////////////////////////////////////////////////////////////
//

// for SeamlessWelder
static MAGIC parrSeamlessWelder = {
	_T("シームレス接着"),
	SeamlessWelder,
	1,
	{	{ 195, 195, _T("強度") },		}
};

// for QuadFlip
static MAGIC parrQuadFlip = {
	_T("方形フリップ"),
	QuadFlip,
	4,
	{	{ 157, 157, _T("インバース強度") },
		{ 123, 123, _T("前景の明るさ") },
		{ 136, 136, _T("背景の明るさ") },
		{  64,  64, _T("ミラーリングモード") },		}
};

// for Perverse
static MAGIC parrPerverse = {
	_T("ミラースコープ 1"),
	Perverse,
	3,
	{	{ 128, 128, _T("水平方向中心") },
		{ 128, 128, _T("垂直方向中心") },
		{ 162, 162, _T("明るさ") },		}
};

// for Promille
static MAGIC parrPromille = {
	_T("ミラースコープ 2"),
	Promille,
	5,
	{	{ 128, 128, _T("水平位置") },
		{ 128, 128, _T("垂直位置") },
		{   0,   0, _T("背景 水平位置") },
		{   0,   0, _T("背景 垂直位置") },
		{ 128, 128, _T("強さ") },			}
};

// for RadialShoved
static MAGIC parrRadialShoved = {
	_T("放射"),
	RadialShoved,
	1,
	{	{ 190, 190, _T("プッシュ アウト") },		}
};

// for KaleidoscopeI
static MAGIC parrKaleidoscopeI = {
	_T("カレイドスコープ 1"),
	KaleidoscopeI,
	2,
	{	{  48,  48, _T("分割数") },
		{ 192, 192, _T("回転") },		}
};

// for KaleidoscopeII
static MAGIC parrKaleidoscopeII = {
	_T("カレイドスコープ 2"),
	KaleidoscopeII,
	2,
	{	{  65,  65, _T("分割数") },
		{ 128, 128, _T("回転") },		}
};

// for KaleidoscopeIII
static MAGIC parrKaleidoscopeIII = {
	_T("カレイドスコープ 3"),
	KaleidoscopeIII,
	2,
	{	{ 224, 224, _T("分割数") },
		{  19,  19, _T("回転") },		}
};

// for KaleidoscopeIV
static MAGIC parrKaleidoscopeIV = {
	_T("カレイドスコープ 4"),
	KaleidoscopeIV,
	3,
	{	{  18,  18, _T("分割数") },
		{ 126, 126, _T("アングル") },
		{   0,   0, _T("オフセット アングル") },		}
};

// for RingMirror
static MAGIC parrRingMirror = {
	_T("リングミラー"),
	RingMirror,
	1,
	{	{ 200, 200, _T("リング数") },		}
};

// for DistortionMirrorHorz
static MAGIC parrDistortionMirrorHorz = {
	_T("ゆがみミラー（水平）"),
	DistortionMirrorHorz,
	1,
	{	{  55,  55, _T("ゆがみ") },		}
};

// for DistortionMirrorVert
static MAGIC parrDistortionMirrorVert = {
	_T("ゆがみミラー（垂直）"),
	DistortionMirrorVert,
	1,
	{	{  55,  55, _T("ゆがみ") },		}
};

//////////////////////////////////////////////////////////////////////
//

HANDLE mirror_seamlesswelder_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSeamlessWelder, PREVIEW_WHOLE);
}

HANDLE mirror_quadflip_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrQuadFlip, PREVIEW_WHOLE);
}

HANDLE mirror_perverse_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrPerverse, PREVIEW_WHOLE);
}

HANDLE mirror_promille_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrPromille, PREVIEW_WHOLE);
}

HANDLE mirror_radialshoved_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrRadialShoved, PREVIEW_WHOLE);
}

HANDLE mirror_kaleidoscopeI_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrKaleidoscopeI, PREVIEW_WHOLE);
}

HANDLE mirror_kaleidoscopeII_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrKaleidoscopeII, PREVIEW_WHOLE);
}

HANDLE mirror_kaleidoscopeIII_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrKaleidoscopeIII, PREVIEW_WHOLE);
}

HANDLE mirror_kaleidoscopeIV_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrKaleidoscopeIV, PREVIEW_WHOLE);
}

HANDLE mirror_ringmirror_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrRingMirror, PREVIEW_WHOLE);
}

HANDLE mirror_distmirrorhorz_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrDistortionMirrorHorz, PREVIEW_WHOLE);
}

HANDLE mirror_distmirrorvert_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrDistortionMirrorVert, PREVIEW_WHOLE);
}
