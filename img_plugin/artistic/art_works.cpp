//////////////////////////////////////////////////////////////////////
// art_works.cpp

#include "stdafx.h"
#include "ffactory_sup.h"

#define MAXPARAMS		8

#define INTENSITY(r,g,b)	((DWORD)((DWORD)(r)*77+(DWORD)(g)*150+(DWORD)(b)*29)>>8)

static void __fastcall PlasticSurface(RGBA* ps, mif& src, int x, int y)
{
	int C = 255;
	put(val(1,C,1),10);
	put(ctl(0)*4,11);
	put(val(2,128,-128),12);

	int n = ps->green;
	put(scl(isin(get(11)+n*val(3,10,100)),-511,512,0,C)+get(12),0);

	int c;
	c = ps->red;
	c = mix(c,get(0),C,get(10));
	ps->red = CLAMP(c);

	c = ps->green;
	c = mix(c,get(0),C,get(10));
	ps->green = CLAMP(c);

	c = ps->blue;
	c = mix(c,get(0),C,get(10));
	ps->blue = CLAMP(c);
}

static void __fastcall PlasticSurgery(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,1,255),8);
	put(scl(cnvsrc(src,x,y,1,2,1,2,0,2,1,2,1,8)%get(8),0,get(8),-512,512),0);
	put(dif(255,(get(0)+512)/2),1);

	RGBA p;
	getsrc_linear(src,x+dr2x(get(0)+dval(2,-512,512),dval(1,0,M/4)),y+dr2y(get(0)+dval(2,-512,512),dval(1,0,M/4)),&p);

	int c;
	put(p.red,2);
	put(get(1)*get(2)/255,3);
	put(255-(255-get(1))*(255-get(2))/255,4);
	c=(get(3)+get(4)+get(2))/3;
	ps->red = CLAMP(c);

	put(p.green,2);
	put(get(1)*get(2)/255,3);
	put(255-(255-get(1))*(255-get(2))/255,4);
	c=(get(3)+get(4)+get(2))/3;
	ps->green = CLAMP(c);

	put(p.blue,2);
	put(get(1)*get(2)/255,3);
	put(255-(255-get(1))*(255-get(2))/255,4);
	c=(get(3)+get(4)+get(2))/3;
	ps->blue = CLAMP(c);
}

static void __fastcall Metalwork(RGBA* ps, mif& src, int x, int y)
{
	int i = yuvY(ps);
	int r = ps->red;
	int g = ps->green;
	int b = ps->blue;

	put((i-isin(i*val(0,5,20))/val(2,20,3)-isin(i*val(1,10,30))/val(3,17,3)),1);
	put(get(1)>255?512-get(1):get(1),1);
	put((255-max(max(r,g),b)),0);

	int c;
	c = ps->red;
	c = ctl(4)*(get(0)+c)*get(1)/65025+(255-ctl(4))*get(1)/255;
	ps->red = CLAMP(c);

	c = ps->green;
	c = ctl(4)*(get(0)+c)*get(1)/65025+(255-ctl(4))*get(1)/255;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = ctl(4)*(get(0)+c)*get(1)/65025+(255-ctl(4))*get(1)/255;
	ps->blue = CLAMP(c);
}

static void __fastcall PlasticWrap(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,1,255),8);
	put(scl(cnvsrc(src,x,y,1,2,1,2,0,2,1,2,1,8)%get(8),0,get(8),-512,512),0);
	put(dif(255,(get(0)+512)/2),1);

	RGBA p = *ps;

	int c;
	put(p.red,2);
	put(get(1)*get(2)/255,3);
	put(255-(255-get(1))*(255-get(2))/255,4);
	c=(get(3)+get(4)+get(2))/3;
	ps->red = CLAMP(c);

	put(p.green,2);
	put(get(1)*get(2)/255,3);
	put(255-(255-get(1))*(255-get(2))/255,4);
	c=(get(3)+get(4)+get(2))/3;
	ps->green = CLAMP(c);

	put(p.blue,2);
	put(get(1)*get(2)/255,3);
	put(255-(255-get(1))*(255-get(2))/255,4);
	c=(get(3)+get(4)+get(2))/3;
	ps->blue = CLAMP(c);
}

static void __fastcall Blonzer(RGBA* ps, mif& src, int x, int y)
{
	int i = yuvY(ps);
	int c;
	put(scl(isin(i*val(0,80,1)),-512,512,0,255),0);
	c = (int)ps->red*get(0)/255;
	ps->red = CLAMP(c);
	c = (int)ps->green*get(0)/255;
	ps->green = CLAMP(c);
	c = (int)ps->blue*get(0)/255;
	ps->blue = CLAMP(c);
}

static void __fastcall ColorCube(RGBA* ps, mif& src, int x, int y)
{
	int C = 255;
	put(((isin(x*ctl(0)*40/M)*icos(y*ctl(0)*40/M))*(isin(abs(X*val(2,20,60)/M)+abs(Y*val(2,20,60)/M))))/C/100,1);
	put(get(1)*255/22/255+val(1,-50,120),0);

	int c;
	c = ps->red;
	c = get(0)&256?355+ctl(3)-(get(0)&511):get(0)+c-60;
	ps->red = CLAMP(c);

	c = ps->green;
	c = get(0)&256?355+ctl(4)-(get(0)&511):get(0)+c-60;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = get(0)&256?355+ctl(5)-(get(0)&511):get(0)+c-60;
	ps->blue = CLAMP(c);
}

static void __fastcall IceCreamSpiral(RGBA* ps, mif& src, int x, int y)
{
	int V = 255;
	int d = c_dir(x, y);
	int m = c_dist(x, y);

	int c;
	c = ps->red;
	put(((isin(m*ctl(1)*99/M+val(0,512,-512))*icos(m*ctl(1)*99/M-d))*(icos(c+val(2,-277,-130))))/V/111,1);
	put(get(1)/20,0);
	c = get(0)&256?350+ctl(3)-(get(0)&511):get(0)+c-150;
	ps->red = CLAMP(c);

	c = ps->green;
	put(((isin(m*ctl(1)*99/M+val(0,512,-512))*icos(m*ctl(1)*99/M-d))*(icos(c+val(2,-277,-130))))/V/111,1);
	put(get(1)/20,0);
	c = get(0)&256?350+ctl(4)-(get(0)&511):get(0)+c-150;
	ps->green = CLAMP(c);

	c = ps->blue;
	put(((isin(m*ctl(1)*99/M+val(0,512,-512))*icos(m*ctl(1)*99/M-d))*(icos(c+val(2,-277,-130))))/V/111,1);
	put(get(1)/20,0);
	c = get(0)&256?350+ctl(5)-(get(0)&511):get(0)+c-150;
	ps->blue = CLAMP(c);
}

static void __fastcall Squaresville(RGBA* ps, mif& src, int x, int y)
{
	put(dif(isin(x*x),isin(y*y)-7*(min(icos(x+ctl(0)*x)-val(3,-800,2000),icos(y+ctl(1)*y)-val(3,-800,2000))*val(2,70,500)))/100,1);
	put(get(1)*255/22/255,0);

	int c;
	c = ps->red;
	c = get(0)&256?355+ctl(5)-(get(0)&511):get(0)+c-ctl(4);
	ps->red = CLAMP(c);

	c = ps->green;
	c = get(0)&256?355+ctl(5)-(get(0)&511):get(0)+c-ctl(4);
	ps->green = CLAMP(c);

	c = ps->blue;
	c = get(0)&256?355+ctl(5)-(get(0)&511):get(0)+c-ctl(4);
	ps->blue = CLAMP(c);
}

#define xtan(x)		((int)(tan(D2R((x))) * 512))

static void __fastcall CubiclesRGB(RGBA* ps, mif& src, int x, int y)
{
	int c;
	c = ps->red;
	c = mix(c,itan(ctl(0)*max((x),y))-xtan(ctl(1)*min((x),y)),ctl(2),ctl(3));
	ps->red = CLAMP(c);
	c = ps->green;
	c = mix(c,itan(ctl(0)*max((x),y))-xtan(ctl(1)*min((x),y)),ctl(4),ctl(5));
	ps->green = CLAMP(c);
	c = ps->blue;
	c = mix(c,itan(ctl(0)*max((x),y))-xtan(ctl(1)*min((x),y)),ctl(6),ctl(7));
	ps->blue = CLAMP(c);
}


//////////////////////////////////////////////////////////////////////
//

// for PlasticSurface
static MAGIC parrPlasticSurface = {
	_T("プラスチック サーフェイス"),
	PlasticSurface,
	4,
	{	{ 186, 186, _T("カラーフェイズ") },
		{ 138, 138, _T("ミックス") },
		{ 142, 142, _T("明るさ") },	
		{  54,  54, _T("モアレ") },		}
};

// for PlasticSurgery
static MAGIC parrPlasticSurgery = {
	_T("プラスティック サージェリー"),
	PlasticSurgery,
	3,
	{	{ 222, 222, _T("フェーズ") },
		{  12,  12, _T("ストレッチ") },
		{  16,  16, _T("チューン") },		}
};

// for Metalwork
static MAGIC parrMetalwork = {
	_T("メタル ワーク"),
	Metalwork,
	5,
	{	{  45,  45, _T("反射 1") },
		{ 159, 159, _T("反射 2") },
		{ 205, 205, _T("輝度 1") },
		{ 210, 210, _T("輝度 2") },
		{  99,  99, _T("カラー") },	}
};

// for PlasticWrap
static MAGIC parrPlasticWrap = {
	_T("プラスティック ラップ"),
	PlasticWrap,
	1,
	{	{  73,  73, _T("モアレ") },		}
};

// for Blonzer
static MAGIC parrBlonzer = {
	_T("ブロンズ"),
	Blonzer,
	1,
	{	{ 173, 173, _T("モアレ") },		}
};

// for ColorCube
static MAGIC parrColorCube = {
	_T("カラー キューブ"),
	ColorCube,
	6,
	{	{  48,  48, _T("キューブサイズ") },
		{  75,  75, _T("キューブ形状") },
		{ 139, 139, _T("コントラスト") },
		{ 102, 102, _T("赤") },
		{ 224, 224, _T("緑") },
		{ 198, 198, _T("青") },		}
};

// for IceCreamSpiral
static MAGIC parrIceCreamSpiral = {
	_T("アイスクリーム スパイラル"),
	IceCreamSpiral,
	6,
	{	{  89,  89, _T("ライト アングル") },
		{  28,  28, _T("渦巻きサイズ") },
		{ 255, 255, _T("不透明度") },
		{ 105, 105, _T("赤") },
		{ 201, 201, _T("緑") },
		{ 225, 225, _T("青") },		}
};

// for Squaresville
static MAGIC parrSquaresville = {
	_T("王朝の窓"),
	Squaresville,
	6,
	{	{  16,  16, _T("水平サイズ") },
		{  16,  16, _T("垂直サイズ") },
		{  88,  88, _T("矩形タイプ 1") },
		{  90,  90, _T("矩形タイプ 2") },
		{  83,  83, _T("スクリーン") },
		{  93,  93, _T("黒 ←→ 白") },		}
};

// for CubiclesRGB
static MAGIC parrCubiclesRGB = {
	_T("キュービクルズ"),
	CubiclesRGB,
	8,
	{	{  19,  19, _T("X") },
		{  13,  13, _T("Y") },
		{  66,  66, _T("赤 1") },
		{  66,  66, _T("赤 2") },
		{  77,  77, _T("緑 1") },
		{  76,  76, _T("緑 2") },
		{  72,  72, _T("青 1") },
		{  65,  65, _T("青 2") },		}
};


//////////////////////////////////////////////////////////////////////
//


HANDLE art_plasticsurface_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return shade_works_ui(hdib, prc, pfe, &parrPlasticSurface, PREVIEW_BOTH);
}

HANDLE art_plasticsurgery_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrPlasticSurgery, PREVIEW_WHOLE);
}

HANDLE art_metalwork_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return shade_works_ui(hdib, prc, pfe, &parrMetalwork, PREVIEW_PART);
}

HANDLE art_plasticwrap_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return shade_works_ui(hdib, prc, pfe, &parrPlasticWrap, PREVIEW_PART);
}

HANDLE art_blonzer_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return shade_works_ui(hdib, prc, pfe, &parrBlonzer, PREVIEW_PART);
}

HANDLE art_colorcube_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrColorCube, PREVIEW_WHOLE);
}

HANDLE art_icecreamspiral_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrIceCreamSpiral, PREVIEW_WHOLE);
}

HANDLE art_squaresville_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSquaresville, PREVIEW_WHOLE);
}

HANDLE art_cubiclesrgb_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrCubiclesRGB, PREVIEW_WHOLE);
}

