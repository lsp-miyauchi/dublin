//////////////////////////////////////////////////////////////////////
// blur_works.cpp

#include "stdafx.h"
#include "ffactory_sup.h"

#define MAXPARAMS		8


static void __fastcall BlurShift(RGBA* ps, mif& src, int x, int y)
{
	put(ctl(0)/4,0);
	put(ctl(1)*2,1);

	put(get(0)*icos(get(1))/512 + get(0)*isin(get(1))/512,2);
	put(-get(0)*isin(get(1))/512 + get(0)*icos(get(1))/512,3);
	put(get(0)*icos(get(1)+256)/512 + get(0)*isin(get(1)+256)/512,4);
	put(-get(0)*isin(get(1)+256)/512 + get(0)*icos(get(1)+256)/512,5);
	put(-get(2),6);
	put(-get(3),7);
	put(-get(4),8);
	put(-get(5),9);

	RGBA* (__fastcall *pfn)(mif&, int, int);
	pfn = ctl(2)<64?getsrc:ctl(2)<128?getsrc2:ctl(2)<192?getsrc3:getsrc1;

	RGBA *p1, *p2, *p3, *p4;
	p1 = (*pfn)(src,x+get(2),y+get(3));
	p2 = (*pfn)(src,x+get(4),y+get(5));
	p3 = (*pfn)(src,x+get(6),y+get(7));
	p4 = (*pfn)(src,x+get(8),y+get(9));
	ps->red = ((int)p1->red + (int)p2->red + (int)p3->red + (int)p4->red) / 4;
	ps->green = ((int)p1->green + (int)p2->green + (int)p3->green + (int)p4->green) / 4;
	ps->blue = ((int)p1->blue + (int)p2->blue + (int)p3->blue + (int)p4->blue) / 4;
}

static void __fastcall Shiver(RGBA* ps, mif& src, int x, int y)
{
	RGBA* p1 = getsrc2(src,(x+val(0,0,X/4)),(y+val(1,0,Y/4)));
	RGBA* p2 = getsrc2(src,(X+x-val(0,0,X/4)),(Y+y-val(1,0,Y/4)));
	RGBA* p3 = getsrc2(src,(x+val(0,0,X/2)),(y+val(1,0,Y/2)));
	RGBA* p4 = getsrc2(src,(X+x-val(0,0,X/2)),(Y+y-val(1,0,Y/2)));

	int c = ps->red;
	c = (c*3+(int)p1->red*2+(int)p2->red*2+(int)p3->red+(int)p4->red)/9;
	ps->red = CLAMP(c);

	c = ps->green;
	c = (c*3+(int)p1->green*2+(int)p2->green*2+(int)p3->green+(int)p4->green)/9;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = (c*3+(int)p1->blue*2+(int)p2->blue*2+(int)p3->blue+(int)p4->blue)/9;
	ps->blue = CLAMP(c);
}

static void __fastcall TransBlur(RGBA* ps, mif& src, int x, int y)
{
	RGBA* p[8];
	p[0] = getsrc3(src,x+get(0),y+get(0));
	p[1] = getsrc3(src,x-get(0),y-get(0));
	p[2] = getsrc3(src,x+get(0),y-get(0));
	p[3] = getsrc3(src,x-get(0),y+get(0));
	p[4] = getsrc3(src,x+get(0),y);
	p[5] = getsrc3(src,x,y+get(0));
	p[6] = getsrc3(src,x-get(0),y);
	p[7] = getsrc3(src,x,y-get(0));

	put(val(0,0,64),0);
	put(val(1,1,16),1);

	int r, g, b;
	r = g = b = 0;
	for (int i = 0; i < 8; i++)
	{
		r += p[i]->red;
		g += p[i]->green;
		b += p[i]->blue;
	}

	int c;
	c = ps->red;
	c = (c + r) / get(1);
	ps->red = CLAMP(c);

	c = ps->green;
	c = (c + g) / get(1);
	ps->green = CLAMP(c);

	c = ps->blue;
	c = (c + b) / get(1);
	ps->blue = CLAMP(c);
}

static void __fastcall ZoomIn(RGBA* ps, mif& src, int x, int y)
{
	put((val(0,0,X)-x)*ctl(2),0);
	put((val(1,0,Y)-y)*ctl(2),1);
	put(128*ctl(3)/384,4);
	put(get(4)*ctl(3)/384,5);
	put(get(5)*ctl(3)/384,6);
	put(get(6)*ctl(3)/384,7);

	RGBA* p1 = getsrc(src,x+get(0)/2048,y+get(1)/2048);
	RGBA* p2 = getsrc(src,x+get(0)/1024,y+get(1)/1024);
	RGBA* p3 = getsrc(src,x+get(0)/512,y+get(1)/512);
	RGBA* p4 = getsrc(src,x+get(0)/256,y+get(1)/256);

	int c;
	c = ps->red;
	c = (128*c+get(4)*p1->red+get(5)*p2->red+get(6)*p3->red+get(7)*p4->red)/(128+get(4)+get(5)+get(6)+get(7));
	ps->red = CLAMP(c);

	c = ps->green;
	c = (128*c+get(4)*p1->green+get(5)*p2->green+get(6)*p3->green+get(7)*p4->green)/(128+get(4)+get(5)+get(6)+get(7));
	ps->green = CLAMP(c);

	c = ps->blue;
	c = (128*c+get(4)*p1->blue+get(5)*p2->blue+get(6)*p3->blue+get(7)*p4->blue)/(128+get(4)+get(5)+get(6)+get(7));
	ps->blue = CLAMP(c);
}

static void __fastcall ZoomOut(RGBA* ps, mif& src, int x, int y)
{
	put((val(0,0,X)-x)*ctl(2),0);
	put((val(1,0,Y)-y)*ctl(2),1);
	put(128*ctl(3)/384,4);
	put(get(4)*ctl(3)/384,5);
	put(get(5)*ctl(3)/384,6);
	put(get(6)*ctl(3)/384,7);

	RGBA* p1 = getsrc(src,x-get(0)/2048,y-get(1)/2048);
	RGBA* p2 = getsrc(src,x-get(0)/1024,y-get(1)/1024);
	RGBA* p3 = getsrc(src,x-get(0)/512,y-get(1)/512);
	RGBA* p4 = getsrc(src,x-get(0)/256,y-get(1)/256);

	int c;
	c = ps->red;
	c = (128*c+get(4)*p1->red+get(5)*p2->red+get(6)*p3->red+get(7)*p4->red)/(128+get(4)+get(5)+get(6)+get(7));
	ps->red = CLAMP(c);

	c = ps->green;
	c = (128*c+get(4)*p1->green+get(5)*p2->green+get(6)*p3->green+get(7)*p4->green)/(128+get(4)+get(5)+get(6)+get(7));
	ps->green = CLAMP(c);

	c = ps->blue;
	c = (128*c+get(4)*p1->blue+get(5)*p2->blue+get(6)*p3->blue+get(7)*p4->blue)/(128+get(4)+get(5)+get(6)+get(7));
	ps->blue = CLAMP(c);
}

static void __fastcall Splash(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,-X,2*X),8);
	put(val(1,-Y,2*Y),9);
	put(c2m(x-get(8),y-get(9)),10);
	put(c2d(x-get(8),y-get(9)),1);
	put(scl(isin(val(2,0,32)*get(1)),-512,512,val(3,1,X),val(4,1,X)),5);
	put(get(10)-get(10)%get(5),0);
	put(r2x(get(1),get(0)),2);
	put(r2y(get(1),get(0)),3);
	put(r2x(get(1),get(0)+get(5)),6);
	put(r2y(get(1),get(0)+get(5)),7);

	RGBA p1, p2;
	getsrc_linear(src,get(8)+get(6),get(9)+get(7),&p1);
	getsrc_linear(src,get(8)+get(2),get(9)+get(3),&p2);

	int c;
	c = mix(p1.red,p2.red,get(10)%get(5),get(5));
	ps->red = CLAMP(c);

	c = mix(p1.green,p2.green,get(10)%get(5),get(5));
	ps->green = CLAMP(c);

	c = mix(p1.blue,p2.blue,get(10)%get(5),get(5));
	ps->blue = CLAMP(c);
}


//////////////////////////////////////////////////////////////////////
//

// for BlurShift
static MAGIC parrBlurShift = {
	_T("ぶれ (シフト)"),
	BlurShift,
	3,
	{	{  20,  20, _T("ぶれの大きさ") },
		{   0,   0, _T("アングル") },
		{ 160, 160, _T("エッジ（複製/ラップ/折返し/黒）") },	}
};

// for Shiver
static MAGIC parrShiver = {
	_T("ぶれ（振動）"),
	Shiver,
	2,
	{	{  27,  27, _T("水平方向の振動") },
		{  22,  22, _T("垂直方向の振動") },	}
};

// for TransBlur
static MAGIC parrTransBlur = {
	_T("ぶれ（トランス）"),
	TransBlur,
	2,
	{	{  20,  20, _T("トランス") },
		{ 128, 128, _T("ミックス") },		}
};

// for ZoomIn
static MAGIC parrZoomIn = {
	_T("視点ズーム イン"),
	ZoomIn,
	4,
	{	{ 128, 128, _T("水平方向の中心") },
		{ 128, 128, _T("垂直方向の中心") },
		{ 255, 255, _T("スームスピード") },
		{ 196, 196, _T("強度") },		}
};

// for ZoomOut
static MAGIC parrZoomOut = {
	_T("視点ズーム アウト"),
	ZoomOut,
	4,
	{	{ 128, 128, _T("水平方向の中心") },
		{ 128, 128, _T("垂直方向の中心") },
		{ 180, 180, _T("スームスピード") },
		{ 255, 255, _T("強度") },		}
};

// for Splash
static MAGIC parrSplash = {
	_T("視点ズーム スプラッシュ"),
	Splash,
	5,
	{	{ 128, 128, _T("水平方向中心") },
		{ 128, 128, _T("垂直方向中心") },
		{  23,  23, _T("強さ") },
		{  13,  13, _T("光線長 1") },
		{  15,  15, _T("光線長 2") },		}
};



//////////////////////////////////////////////////////////////////////
//

HANDLE blur_shift_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrBlurShift, PREVIEW_PART);
}

HANDLE blur_shiver_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrShiver, PREVIEW_WHOLE);
}

HANDLE blur_trans_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrTransBlur, PREVIEW_WHOLE);
}

HANDLE blur_zoomin_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrZoomIn, PREVIEW_WHOLE);
}

HANDLE blur_zoomout_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrZoomOut, PREVIEW_WHOLE);
}

HANDLE blur_zoomsplash_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSplash, PREVIEW_WHOLE);
}

