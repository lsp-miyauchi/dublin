//////////////////////////////////////////////////////////////////////
// stroke_works.cpp

#include "stdafx.h"
#include "ffactory_sup.h"

#define MAXPARAMS		8

static void __fastcall DeppCrunch(RGBA* ps, mif& src, int x, int y)
{
	int i = yuvY(ps);

	int c;
	c = ps->red;
	put(x==0?i:get(0),0);
	put(i>get(0)?c:(get(1)*4-ctl(1))/4,1);
	c = get(1);
	ps->red = CLAMP(c);

	c = ps->green;
	put(i>get(0)?c:(get(2)*4-ctl(2))/4,2);
	c = get(2);
	ps->green = CLAMP(c);

	c = ps->blue;
	put(i>get(0)?c:(get(3)*4-ctl(3))/4,3);
	put(i>get(0)?i:(get(0)*4-val(0,0,128))/4,0);
	c = get(3);
	ps->blue = CLAMP(c);
}

static void __fastcall NeedlesAndSpins(RGBA* ps, mif& src, int x, int y)
{
	put(rnd(1,abs(val(0,-X,X))),0);
	put(get(1)-1*(get(1)>0)+(get(1)==0)*get(0),1);
	put((get(0)==get(1))*get(0)+(get(0)!=get(1))*get(2),2);

	RGBA* p1 = getsrc3(src,x-(val(0,-X,X)<0)*get(1)+(val(0,-X,X)>=0)*get(1),y);
	int c;
	c=mix(ctl(1),p1->red,get(1),get(2));
	ps->red = CLAMP(c);

	c=mix(ctl(1),p1->green,get(1),get(2));
	ps->green = CLAMP(c);

	c=mix(ctl(1),p1->blue,get(1),get(2));
	ps->blue = CLAMP(c);
}

static void __fastcall PixelSlime(RGBA* ps, mif& src, int x, int y)
{
	int r,g,b;
	r = ps->red;
	g = ps->green;
	b = ps->blue;
	put(((get(x/8)&(1<<(x%8)))==(1<<(x%8)))||(dif(r,ctl(1))+dif(g,ctl(2))+dif(b,ctl(3))<val(4,0,768))?
(get(x/8)&(255-(1<<(x%8))))|((rnd(0,255)<ctl(0))<<(x%8)):(get(x/8)&(255-(1<<(x%8)))),x/8);

	int c;
	c = ps->red;
	c = ((get(x/8)&(1<<(x%8)))==(1<<(x%8)))?ctl(1):c;
	ps->red = CLAMP(c);

	c = ps->green;
	c = ((get(x/8)&(1<<(x%8)))==(1<<(x%8)))?ctl(2):c;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = ((get(x/8)&(1<<(x%8)))==(1<<(x%8)))?ctl(3):c;
	ps->blue = CLAMP(c);
}

static void __fastcall Xaggerate(RGBA* ps, mif& src, int x, int y)
{
	RGBA* p1 = getsrc3(src,x-(x-y)*(y-x)%val(0,1,X),y-(x+y)*(y+x)%val(1,1,X));
	*ps = *p1;
}

static void __fastcall ZigZaggerate(RGBA* ps, mif& src, int x, int y)
{
	RGBA* p1 = getsrc3(src,x+((abs(y-Y/2))*y%val(0,1,X/2)),y+((abs(x-X/2))*x%val(1,1,Y/2)));
	*ps = *p1;
}

static void __fastcall ZigZack(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,0*X/255,255*X/255),4);
	put(get(4)?get(4):1,4);
	put(val(1,0*X/255,255*X/255),5);
	put(get(5)?get(5):1,5);

	put(x+y%get(4),0);
	put(val(3,255*X/255,-255*X/255),1);
	put(y+x%get(5),2);
	put(val(2,255*X/255,-255*X/255),3);
	RGBA* p1 = getsrc3(src,get(0)+get(1),get(2)+get(3));
	*ps = *p1;
}

static void __fastcall TVDistortion(RGBA* ps, mif& src, int x, int y)
{
	put(x==0? rnd(-val(0,0,X/2),val(0,0,X/2)):get(0),0);
	RGBA* p = getsrc3(src, x+get(0),y);
	*ps = *p;
}

//////////////////////////////////////////////////////////////////////
//

// for DeppCrunch
static MAGIC parrDeppCrunch = {
	_T("ワープ イン"),
	DeppCrunch,
	4,
	{	{  10,  10, _T("深度") },
		{  11,  11, _T("赤") },
		{  13,  13, _T("緑") },
		{  13,  13, _T("青") },		}
};

// for NeedlesAndSpins
static MAGIC parrNeedlesAndSpins = {
	_T("烈風"),
	NeedlesAndSpins,
	2,
	{	{  98,  98, _T("強さ") },
		{  16,  16, _T("明度") },		}
};

// for PixelSlime
static MAGIC parrPixelSlime = {
	_T("スライム"),
	PixelSlime,
	5,
	{	{ 210, 210, _T("最大長") },
		{ 255, 255, _T("赤") },
		{ 255, 255, _T("緑") },
		{ 255, 255, _T("青") },
		{  86,  86, _T("適用範囲") },			}
};

// for Xaggerate
static MAGIC parrXaggerate = {
	_T("引っ掻き 1"),
	Xaggerate,
	2,
	{	{  16,  16, _T("左上から右下") },
		{  16,  16, _T("右上から左下") },		}
};

// for ZigZaggerate
static MAGIC parrZigZaggerate = {
	_T("引っ掻き 2"),
	ZigZaggerate,
	2,
	{	{  14,  14, _T("左右") },
		{  20,  20, _T("上下") },		}
};

// for TVDistortion
static MAGIC parrTVDistortion = {
	_T("引っ掻き 3"),
	TVDistortion,
	1,
	{	{  12,  12, _T("偏り") },		}
};

// for ZigZack
static MAGIC parrZigZack = {
	_T("ジグザックリ"),
	ZigZack,
	4,
	{	{  10,  10, _T("セル幅") },
		{  11,  11, _T("セル高さ") },
		{ 128, 128, _T("横オフセット") },
		{ 128, 128, _T("縦オフセット") },		}
};


//////////////////////////////////////////////////////////////////////
//

HANDLE stroke_deppcrunch_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrDeppCrunch, PREVIEW_WHOLE);
}

HANDLE stroke_needlespin_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrNeedlesAndSpins, PREVIEW_WHOLE);
}

HANDLE stroke_pixelslime_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrPixelSlime, PREVIEW_WHOLE);
}

HANDLE stroke_xaggerate_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrXaggerate, PREVIEW_WHOLE);
}

HANDLE stroke_zigzagerate_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrZigZaggerate, PREVIEW_WHOLE);
}

HANDLE stroke_zigzack_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrZigZack, PREVIEW_WHOLE);
}

HANDLE stroke_tvdistortion_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrTVDistortion, PREVIEW_WHOLE);
}
