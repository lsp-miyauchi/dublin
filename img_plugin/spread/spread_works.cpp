//////////////////////////////////////////////////////////////////////
// spread_works.cpp

#include "stdafx.h"
#include "ffactory_sup.h"


//////////////////////////////////////////////////////////////////////
//

static void __fastcall SpreadBlast(RGBA* ps, mif& src, int x, int y)
{
	put(scl(rnd(-ctl(0),ctl(0)),0,255,0,X/2),0);
	put(scl(rnd(-ctl(0),ctl(0)),0,255,0,Y/2),1);

	RGBA* p1 = getsrc(src,x+get(0),y+get(1));
	int c;
	c = mix(p1->red,ps->red,ctl(1),255);
	ps->red = CLAMP(c);
	c = mix(p1->green,ps->green,ctl(1),255);
	ps->green = CLAMP(c);
	c = mix(p1->blue,ps->blue,ctl(1),255);
	ps->blue = CLAMP(c);
}

static void __fastcall SpreadBlur(RGBA* ps, mif& src, int x, int y)
{
	put(rnd(0,val(0,0,X/2)),0);
	put(rnd(0,val(1,0,Y/2)),1);

	RGBA* p1 = getsrc(src,x+get(0),y+get(1));
	RGBA* p2 = getsrc(src,x-get(0),y+get(1));
	RGBA* p3 = getsrc(src,x+get(0),y-get(1));
	RGBA* p4 = getsrc(src,x-get(0),y-get(1));

	ps->red = ((int)p1->red + (int)p2->red + (int)p3->red + (int)p4->red) / 4;
	ps->green = ((int)p1->green + (int)p2->green + (int)p3->green + (int)p4->green) / 4;
	ps->blue = ((int)p1->blue + (int)p2->blue + (int)p3->blue + (int)p4->blue) / 4;
}

static void __fastcall SpreadRotation(RGBA* ps, mif& src, int x, int y)
{
	put(rnd(-ctl(0),ctl(0)),0);
//@	radxy(src,x,y,get(0),ps);
	radxyex(src,x,y,val(1,0,X),val(2,0,Y),get(0),ps);
}

static void __fastcall SpreadZoom(RGBA* ps, mif& src, int x, int y)
{
	int d = c_dir(x, y);
	int m = c_dist(x, y);

	put(val(1,0,M),0);
	put(val(0,M,0),1);
	put(rnd(-get(0),get(0)),2);
	put(scl(m,0,M,0,get(2)+get(1)),3);

	rad(src,d,get(3),ps);
}

static void __fastcall SpreadShift(RGBA* ps, mif& src, int x, int y)
{
//@	put(rnd(-val(0,0,X/2),val(0,0,X/2)), 0);
//@	put(rnd(-val(1,0,Y/2),val(1,0,Y/2)), 1);
	put(rnd(-ctl(0),ctl(0)), 0);
	put(rnd(-ctl(1),ctl(1)), 1);

	RGBA* p1 = getsrc3(src, x-get(0),y-get(1));
	*ps = *p1;
}

static void __fastcall SpreadCross(RGBA* ps, mif& src, int x, int y)
{
	put(rnd(0, ctl(0)), 2);
	put(scl(get(2), 0, 255, 0, X/4), 0);
	put(scl(get(2), 0, 255, 0, Y/4), 1);

	RGBA* p1 = getsrc(src,x+get(0),y+get(1));
	RGBA* p2 = getsrc(src,x-get(0),y+get(1));
	RGBA* p3 = getsrc(src,x+get(0),y-get(1));
	RGBA* p4 = getsrc(src,x-get(0),y-get(1));

	int c;
	c = ((int)p1->red+(int)p2->red+(int)p3->red+(int)p4->red)/4;
	ps->red = CLAMP(c);

	c = ((int)p1->green+(int)p2->green+(int)p3->green+(int)p4->green)/4;
	ps->green = CLAMP(c);

	c = ((int)p1->blue+(int)p2->blue+(int)p3->blue+(int)p4->blue)/4;
	ps->blue = CLAMP(c);
}

static void __fastcall SpreadSpeed(RGBA* ps, mif& src, int x, int y)
{
	put(rnd(val(2,0,val(1,0,M)),val(1,0,M)),0);

	RGBA* p1 = getsrc3(src,x+r2x(val(0,-512,512),get(0)),y+r2y(val(0,-512,512),get(0)));

	int c;
	c=ps->red;
	c=mix(c,p1->red,get(0),val(1,0,M)+ctl(3));
	ps->red = CLAMP(c);

	c=ps->green;
	c=mix(c,p1->green,get(0),val(1,0,M)+ctl(3));
	ps->green = CLAMP(c);

	c=ps->blue;
	c=mix(c,p1->blue,get(0),val(1,0,M)+ctl(3));
	ps->blue = CLAMP(c);
}

static void __fastcall Twister(RGBA* ps, mif& src, int x, int y)
{
	int d = c_dir(x, y);
	int m = c_dist(x, y);

	put(d-(d-m)*(m-d)%val(0,1,255),0);
	put(m-(d-m)*(m-d)%val(0,1,255),1);

	rad(src,get(0),get(1), ps);
}


/////////////////////////////////////////////////////////////////////
//

// for SpreadBlast
static MAGIC parrSpreadBlast = {
	_T("ブラスト"),
	SpreadBlast,
	2,
	{	{  20,  20, _T("拡散の度合い") },
		{ 211, 211, _T("不透明度") },	}
};

// for SpreadBlur
static MAGIC parrSpreadBlur = {
	_T("ぼかし "),
	SpreadBlur,
	2,
	{	{  14,  14, _T("横方向のぼかし") },
		{  12,  12, _T("縦方向のぼかし") },	}
};

// for SpreadRotation
static MAGIC parrSpreadRotation = {
	_T("ローテーション"),
	SpreadRotation,
	3,
	{	{  26,  26, _T("回転") },
		{ 128, 128, _T("水平方向の中心") },
		{ 128, 128, _T("垂直方向の中心") },		}
};

// for SpreadZoom
static MAGIC parrSpreadZoom = {
	_T("ズーム"),
	SpreadZoom,
	2,
	{	{  41,  41, _T("ズーム") },
		{  26,  26, _T("ノイズ頻度") },		}
};

// for SpreadShift
static MAGIC parrSpreadShift = {
	_T("拡散"),
	SpreadShift,
	2,
	{	{  40,  40, _T("水平方向のレベル") },
		{   0,   0, _T("垂直方向のレベル") },		}

};

// for SpreadCross
static MAGIC parrSpreadCross = {
	_T("拡散 クロス"),
	SpreadCross,
	1,
	{	{  40,  40, _T("距離") },		}
};

// for SpreadSpeed
static MAGIC parrSpreadSpeed = {
	_T("拡散 スピード"),
	SpreadSpeed,
	4,
	{	{ 104, 104, _T("方向") },
		{  85,  85, _T("スピード") },
		{  28,  28, _T("ギャップ") },
		{   0,   0, _T("強度") },		}
};

// for Twister
static MAGIC parrTwister = {
	_T("ツイスター"),
	Twister,
	1,
	{	{  18,  18, _T("強さ") },		}
};


/////////////////////////////////////////////////////////////////////
//


HANDLE spread_shift_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSpreadShift, PREVIEW_PART);
}

HANDLE spread_blast_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSpreadBlast, PREVIEW_PART);
}

HANDLE spread_blur_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSpreadBlur, PREVIEW_PART);
}

HANDLE spread_rotation_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSpreadRotation, PREVIEW_WHOLE);
}

HANDLE spread_zoom_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSpreadZoom, PREVIEW_WHOLE);
}

HANDLE spread_cross_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSpreadCross, PREVIEW_PART);
}

HANDLE spread_speed_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSpreadSpeed, PREVIEW_WHOLE);
}

HANDLE spread_twister_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrTwister, PREVIEW_WHOLE);
}
