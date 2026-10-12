//////////////////////////////////////////////////////////////////////
// color_works.cpp

#include "stdafx.h"
#include "resource.h"
#include "ffactory_sup.h"

//////////////////////////////////////////////////////////////////////
//

static void __fastcall Contrast(RGBA* ps, mif& src, int x, int y)
{
// mix(c,ctl(0)+16,ctl(1)+16,c)
// mix(c,ctl(0)+16,ctl(1)+16,c)
// mix(c,ctl(0)+16,ctl(1)+16,c)

	int c;
	c = ps->red;
	c = mix(c, ctl(0) + 16, ctl(1) + 16, c);
	ps->red = min(255, max(0, c));

	c = ps->green;
	c = mix(c, ctl(0) + 16, ctl(1) + 16, c);
	ps->green = min(255, max(0, c));

	c = ps->blue;
	c = mix(c, ctl(0) + 16, ctl(1) + 16, c);
	ps->blue = min(255, max(0, c));
}

static void __fastcall Converse(RGBA* ps, mif& src, int x, int y)
{
// mix(i,ctl(0),i,ctl(1))
// mix(i,ctl(0),i,ctl(2))
// mix(i,ctl(0),i,ctl(3))

	int i = yuvY(ps);
	int c;
	c = mix(i, ctl(0), i, ctl(1));
	ps->red = min(255, max(0, c));

	c = mix(i, ctl(0), i, ctl(2));
	ps->green = min(255, max(0, c));

	c = mix(i, ctl(0), i, ctl(3));
	ps->blue = min(255, max(0, c));
}

static void __fastcall Dual(RGBA* ps, mif& src, int x, int y)
{
// dif(i,ctl(0))+dif(i,ctl(0))
// dif(i,ctl(1))+dif(i,ctl(1))
// dif(i,ctl(2))+dif(i,ctl(2))

	int i = yuvY(ps);
	int c;
	c = dif(i, ctl(0)) + dif(i, ctl(0));
	ps->red = min(255, max(0, c));

	c = dif(i, ctl(1)) + dif(i, ctl(1));
	ps->green = min(255, max(0, c));

	c = dif(i, ctl(2)) + dif(i, ctl(2));
	ps->blue = min(255, max(0, c));
}

static void __fastcall Grey(RGBA* ps, mif& src, int x, int y)
{
// mix(i,ctl(0),ctl(2),ctl(1))
// mix(i,ctl(0),ctl(2),ctl(1))
// mix(i,ctl(0),ctl(2),ctl(1))

	int i = yuvY(ps);
	int c = mix(i, ctl(0), ctl(2), ctl(1));
	ps->red = ps->green = ps->blue = min(255, max(0, c));
}

static void __fastcall Inverse(RGBA* ps, mif& src, int x, int y)
{
// dif(c,ctl(0))
// dif(c,ctl(0))
// dif(c,ctl(0))

	int c;
	c = dif((int)ps->red, ctl(0));
	ps->red = min(255, max(0, c));

	c = dif((int)ps->green, ctl(0));
	ps->green = min(255, max(0, c));

	c = dif((int)ps->blue, ctl(0));
	ps->blue = min(255, max(0, c));
}

static void __fastcall Reverse(RGBA* ps, mif& src, int x, int y)
{
// dif(c,ctl(0))
// dif(c,ctl(1))
// dif(c,ctl(2))

	int c;
	c = dif((int)ps->red, ctl(0));
	ps->red = min(255, max(0, c));

	c = dif((int)ps->green, ctl(1));
	ps->green = min(255, max(0, c));

	c = dif((int)ps->blue, ctl(2));
	ps->blue = min(255, max(0, c));
}

static void __fastcall Mono(RGBA* ps, mif& src, int x, int y)
{
// i+ctl(1)-ctl(0)
// i+ctl(2)-ctl(0)
// i+ctl(3)-ctl(0)

	int i = yuvY(ps);
	int c;
	c = i + ctl(1) - ctl(0);
	ps->red = min(255, max(0, c));

	c = i + ctl(2) - ctl(0);
	ps->green = min(255, max(0, c));

	c = i + ctl(3) - ctl(0);
	ps->blue = min(255, max(0, c));
}

static void __fastcall Sepia(RGBA* ps, mif& src, int x, int y)
{
	int i = yuvY(ps);
	gamma(ctl(0)*2,0,0);

	ps->red = gam(i+40,0);
	ps->green = gam(i,0);
	ps->blue = gam(i-40,0);
}


static void __fastcall Solar(RGBA* ps, mif& src, int x, int y)
{
// dif(c,ctl(0))+dif(i,ctl(1))
// dif(c,ctl(0))+dif(i,ctl(2))
// dif(c,ctl(0))+dif(i,ctl(3))

	int i = yuvY(ps);
	int c;
	c = dif((int)ps->red, ctl(0)) + dif(i, ctl(1));
	ps->red = min(255, max(0, c));

	c = dif((int)ps->green, ctl(0)) + dif(i, ctl(2));
	ps->green = min(255, max(0, c));

	c = dif((int)ps->blue, ctl(0)) + dif(i, ctl(3));
	ps->blue = min(255, max(0, c));
}

static void __fastcall OldPhoto(RGBA* ps, mif& src, int x, int y)
{
	int i = yuvY(ps);
	ps->red = mix(ps->red,i,ctl(0),255);
	ps->green = mix(ps->green,i,ctl(1),255);
	ps->blue = mix(ps->blue,i,ctl(2),255);
}

static void __fastcall ColorizeRGB(RGBA* ps, mif& src, int x, int y)
{
	int I = 255;
	int i = yuvY(ps);

	int c;
	c = scl(i,0,I,0,ctl(0)*2);
	ps->red = CLAMP(c);
	c = scl(i,0,I,0,ctl(1)*2);
	ps->green = CLAMP(c);
	c = scl(i,0,I,0,ctl(2)*2);
	ps->blue = CLAMP(c);
}

static void __fastcall ColorizeHSB(RGBA* ps, mif& src, int x, int y)
{
	put(val(1,-150,150), 4);

	int i = yuvY(ps);
	int c;
	c = mix(hue(ctl(0),0) * i / 255, i, ctl(2), 255)+get(4);
	ps->red = CLAMP(c);
	c = mix(hue(ctl(0),1) * i / 255, i, ctl(2), 255)+get(4);
	ps->green = CLAMP(c);
	c = mix(hue(ctl(0),2) * i / 255, i, ctl(2), 255)+get(4);
	ps->blue = CLAMP(c);
}

static void __fastcall MaxComapre(RGBA* ps, mif& src, int x, int y)
{
	for (int i = -1; i <= 1; i++)
		for (int j = -1; j <= 1; j++)
		{
			RGBA* p=getsrc3(src,x+j,y+i);
			ps->red = max(ps->red,p->red);
			ps->green = max(ps->green,p->green);
			ps->blue = max(ps->blue,p->blue);
		}
}

static void __fastcall MinComapre(RGBA* ps, mif& src, int x, int y)
{
	for (int i = -1; i <= 1; i++)
		for (int j = -1; j <= 1; j++)
		{
			RGBA* p=getsrc3(src,x+j,y+i);
			ps->red = min(ps->red,p->red);
			ps->green = min(ps->green,p->green);
			ps->blue = min(ps->blue,p->blue);
		}
}

static void __fastcall Exposure(RGBA* ps, mif& src, int x, int y)
{
// i-ctl(0)+c
// i-ctl(0)+c
// i-ctl(0)+c

	int i = yuvY(ps);
	int c;
	c = i - ctl(0) + (int)ps->red;
	ps->red = min(255, max(0, c));

	c = i - ctl(0) + (int)ps->green;
	ps->green = min(255, max(0, c));

	c = i - ctl(0) + (int)ps->blue;
	ps->blue = min(255, max(0, c));
}

static void __fastcall ColorBooster(RGBA* ps, mif& src, int x, int y)
{
	int C = 255;
	int r = ps->red;
	int g = ps->green;
	int b = ps->blue;

	int c;
	c = mix(r*2,g+b,C,val(0,C,1))/2;
	ps->red = CLAMP(c);

	c = mix(g*2,r+b,C,val(0,C,1))/2;
	ps->green = CLAMP(c);

	c = mix(b*2,r+g,C,val(0,C,1))/2;
	ps->blue = CLAMP(c);
}

static void __fastcall WaterColor(RGBA* ps, mif& src, int x, int y)
{
	int C = 255;

	RGBA* p[13];
	p[ 0] =	getsrc(src,x,y-3);
	p[ 1] =	getsrc(src,x-2,y-2);
	p[ 2] =	getsrc(src,x+2,y-2);
	p[ 3] =	getsrc(src,x-1,y-1);
	p[ 4] =	getsrc(src,x+1,y-1);
	p[ 5] =	getsrc(src,x-3,y);
	p[ 6] =	ps;
	p[ 7] =	getsrc(src,x+3,y);
	p[ 8] =	getsrc(src,x-1,y+1);
	p[ 9] =	getsrc(src,x+1,y+1);
	p[10] =	getsrc(src,x-2,y+2);
	p[11] =	getsrc(src,x+2,y+2);
	p[12] =	getsrc(src,x,y+3);

	int r, g, b;
	r = g = b = 0;
	for (int i = 0; i < 13; i++)
	{
		r += (int)p[i]->red;
		g += (int)p[i]->green;
		b += (int)p[i]->blue;
	}

	int c;
	c = ps->red;
	c = mix(((r/13) & C << 6)*100/80,c,ctl(0)+1, C);
	ps->red = CLAMP(c);

	c = ps->green;
	c = mix(((g/13) & C << 6)*100/80,c,ctl(0)+1, C);
	ps->green = CLAMP(c);

	c = ps->blue;
	c = mix(((b/13) & C << 6)*100/80,c,ctl(0)+1, C);
	ps->blue = CLAMP(c);
}

static void __fastcall HistogramStretch(RGBA* ps, mif& src, int x, int y)
{
	int C = 255;
	put(val(0,-100,100),0);
	put(val(1,C-100,C+100),1);
	gamma(ctl(3)*2,ctl(3)*2,ctl(3)*2);

	ps->red = gam(scl(ps->red,0,C,get(0),get(1)),0);
	ps->green = gam(scl(ps->green,0,C,get(0),get(1)),1);
	ps->blue = gam(scl(ps->blue,0,C,get(0),get(1)),2);
}

static void __fastcall Solarize(RGBA* ps, mif& src, int x, int y)
{
	int c;
	c = ps->red;
	ps->red=c>ctl(0)?255-c:c;
	c = ps->green;
	ps->green=c>ctl(0)?255-c:c;
	c = ps->blue;
	ps->blue=c>ctl(0)?255-c:c;
}

static void __fastcall RemoveGray(RGBA* ps, mif& src, int x, int y)
{
	int r = ps->red;
	int g = ps->green;
	int b = ps->blue;
	int i = yuvY(ps);

	put(min(r,min(g,b)),0);

	int c;
	c = ps->red;
	put(mix(c,i,ctl(1),255),1);
	c = (get(1)-get(0))*val(0,1,20);
	ps->red = CLAMP(c);

	c = ps->green;
	put(mix(c,i,ctl(1),255),1);
	c = (get(1)-get(0))*val(0,1,20);
	ps->green = CLAMP(c);

	c = ps->blue;
	put(mix(c,i,ctl(1),255),1);
	c = (get(1)-get(0))*val(0,1,20);
	ps->blue = CLAMP(c);
}

static void __fastcall ColorTemperature(RGBA* ps, mif& src, int x, int y)
{
	int r = ps->red;
	int g = ps->green;
	int b = ps->blue;
	int C = 255;

	put(val(0, -128, 128), 0);

	int c;
	c = ps->red;
	c = ctl(0)<128 ? mix(0, r, -get(0), C) : ctl(0)>128 ? mix(r*2, r, get(0), C) : c;
	ps->red = CLAMP(c);

	c = ps->green;
	c = ctl(0)<128 ? mix(g*170/128, g, -get(0), C) : ctl(0)>128 ?mix(g*85/128, g, get(0), C) : c;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = ctl(0)<128 ? mix(b*170/128, b, -get(0), C) : ctl(0)>128 ?mix(b*85/128, b, get(0), C) : c;
	ps->blue = CLAMP(c);
}

static void __fastcall Skylight(RGBA* ps, mif& src, int x, int y)
{
	int C = 255;
	put(val(0, 0, C),0);

	int c;
	c = ps->red;
	c = mix(c*2, c, get(0), C*8);
	ps->red = CLAMP(c);

	c = ps->blue;
	c = mix(c/3, c, get(0), C*8);
	ps->blue = CLAMP(c);
}

static void __fastcall Pastel(RGBA* ps, mif& src, int x, int y)
{
	int c;
	c = ps->red;
	c = (int)sqr((double)c*ctl(0))+ctl(1);
	ps->red = CLAMP(c);

	c = ps->green;
	c = (int)sqr((double)c*ctl(0))+ctl(2);
	ps->green = CLAMP(c);

	c = ps->blue;
	c = (int)sqr((double)c*ctl(0))+ctl(3);
	ps->blue = CLAMP(c);
}

/////////////////////////////////////////////////////////////////////
//

// for Contrast
static MAGIC parrContrast = {
	_T("コントラスト"),
	Contrast,
	2,
	{	{ 128, 128, _T("範囲 1") },
		{ 255, 255, _T("範囲 2") },	}
};

// for Converse
static MAGIC parrConverse = {
	_T("コンバース"),
	Converse,
	4,
	{	{ 178, 178, _T("範囲") },
		{ 227, 227, _T("水色 - 赤のミックス") },
		{  87,  87, _T("紫 - 緑のミックス") },
		{  74,  74, _T("黄 - 青のミックス") },	}
};

// for Dual
static MAGIC parrDual = {
	_T("デュアル"),
	Dual,
	3,
	{	{  75,  75, _T("水色 - 赤のミックス") },
		{ 110, 110, _T("紫 - 緑のミックス") },
		{ 147, 147, _T("黄 - 青のミックス") },	}
};

// for Grey
static MAGIC parrGrey = {
	_T("グレー＆グレー"),
	Grey,
	3,
	{	{ 128, 128, _T("輝度") },
		{  37,  37, _T("明るさ 1") },
		{ 192, 192, _T("明るさ 2") },		}
};

// for Inverse
static MAGIC parrInverse = {
	_T("インバース"),
	Inverse,
	1,
	{	{ 128, 128, _T("最小 / 最大") },		}
};

// for Reverse
static MAGIC parrReverse = {
	_T("リバース"),
	Reverse,
	3,
	{	{ 110, 110, _T("赤の反転") },
		{  90,  90, _T("緑の反転") },
		{  70,  70, _T("青の反転") },	}
};

// for Mono
static MAGIC parrMono = {
	_T("セピア"),
	Mono,
	4,
	{	{ 128, 128, _T("コントラスト") },
		{ 164, 164, _T("水色 - 赤の色合い") },
		{ 137, 137, _T("紫 - 緑の色合い") },
		{ 104, 104, _T("黄 - 青の色合い") },	}
};

// for Sepia
static MAGIC parrSepia = {
	_T("セピア"),
	Sepia,
	1,
	{	{ 128, 128, _T("コントラスト") },		}
};

// for Solar
static MAGIC parrSolar = {
	_T("ソーラー"),
	Solar,
	4,
	{	{  98,  98, _T("最小 / 最大") },
		{  98,  98, _T("水色 - 赤のミックス") },
		{  98,  98, _T("紫 - 緑のミックス") },
		{  98,  98, _T("黄 - 青のミックス") },	}
};

// for OldPhoto
static MAGIC parrOldPhoto = {
	_T("褪色"),
	OldPhoto,
	3,
	{	{ 183, 183, _T("赤") },
		{  69,  69, _T("緑") },
		{ 131, 131, _T("青") },	}
};

// for ColorizeRGB
static MAGIC parrColorizeRGB = {
	_T("カラーライズ (RGB)"),
	ColorizeRGB,
	3,
	{	{ 151, 151, _T("赤") },
		{ 187, 187, _T("緑") },
		{ 255, 255, _T("青") },		}
};

// for ColorizeHSB
static MAGIC parrColorizeHSB = {
	_T("カラーライズ (HSB)"),
	ColorizeHSB,
	3,
	{	{  52,  52, _T("色相") },
		{ 212, 212, _T("明るさ") },
		{ 169, 169, _T("彩度") },		}
};

// for MaxCompare
static MAGIC parrMaxCompare = {
	_T("明るさの最大値"),
	MaxComapre,
	0,
	{	{ 0, 0, NULL },		}
};

// for MinCompare
static MAGIC parrMinCompare = {
	_T("明るさの最小値"),
	MinComapre,
	0,
	{	{ 0, 0, NULL },		}
};

// for Exposure
static MAGIC parrExposure = {
	_T("露出"),
	Exposure,
	1,
	{	{ 128, 128, _T("明るさ") },	}
};

// for ColorBooster
static MAGIC parrColorBooster = {
	_T("彩度ブースト"),
	ColorBooster,
	1,
	{	{ 149, 149, _T("増進度") },		}
};

// for WaterColor
static MAGIC parrWaterColor = {
	_T("水彩"),
	WaterColor,
	1,
	{	{ 157, 157, _T("フェード") },	}
};

// for HistogramStretch
static MAGIC parrHistogramStretch = {
	_T("ヒストグラム伸張"),
	HistogramStretch,
	4,
	{	{ 100, 100, _T("シャドウ") },
		{ 157, 157, _T("ハイライト") },
		{   0,   0, NULL },
		{ 128, 128, _T("ガンマ") },		}
};

// for Solarize
static MAGIC parrSolarize = {
	_T("ソラライズ"),
	Solarize,
	1,
	{	{ 0, 0, _T("閏値") },		}
};

// for RemoveGray
static MAGIC parrRemoveGray = {
	_T("グレーの除去"),
	RemoveGray,
	2,
	{	{ 180, 180, _T("明るさ") },
		{  76,  76, _T("彩度") },			}
};

// for ColorTemperature
static MAGIC parrColorTemperature = {
	_T("色温度"),
	ColorTemperature,
	1,
	{	{ 144, 144, _T("色温度") },			}
};

// for Skylight
static MAGIC parrSkylight = {
	_T("スカイライト"),
	Skylight,
	1,
	{	{ 144, 144, _T("強度") },			}
};

// for Pastel
static MAGIC parrPastel = {
	_T("パステル"),
	Pastel,
	4,
	{	{ 128, 128, _T("彩度") },
		{  86,  86, _T("赤") },
		{  75,  75, _T("緑") },
		{  63,  63, _T("青") },			}
};


///////////////////////////////////////////////////////////////////////
//

#ifdef MORE_FILTE_FUNCTION

static void __fastcall Fade(RGBA* ps, mif& src, int x, int y)
{
// max(c,ctl(0))
// max(c,ctl(1))
// max(c,ctl(2))

	ps->red   = max(ps->red,   ctl(0));
	ps->green = max(ps->green, ctl(1));
	ps->blue  = max(ps->blue,  ctl(2));
}

static void __fastcall Heat(RGBA* ps, mif& src, int x, int y)
{
// 255
// c+ctl(0)-ctl(1)
// 0

	ps->red = 255;
	int c = (int)ps->green + ctl(0) - ctl(1);
	ps->green = min(255, max(0, c));
	ps->blue = 0;
}

static void __fastcall Tint(RGBA* ps, mif& src, int x, int y)
{
// c+ctl(1)-ctl(0)
// c+ctl(2)-ctl(0)
// c+ctl(3)-ctl(0)

	int c;
	c = (int)ps->red + ctl(1) - ctl(0);
	ps->red = min(255, max(0, c));

	c = (int)ps->green + ctl(2) - ctl(0);
	ps->green = min(255, max(0, c));

	c = (int)ps->blue + ctl(3) - ctl(0);
	ps->blue = min(255, max(0, c));
}

static void __fastcall Pale(RGBA* ps, mif& src, int x, int y)
{
// abs(max(i-ctl(0),u+ctl(0)))
// abs(max(i-ctl(1),u+ctl(1)))
// abs(max(i-ctl(2),u+ctl(2)))

	int i = yuvY(ps);
	int u = yuvU(ps);
	int c;
	c = abs(max(i - ctl(0), u + ctl(0)));
	ps->red = min(255, max(0, c));

	c = abs(max(i - ctl(1), u + ctl(1)));
	ps->green = min(255, max(0, c));

	c = abs(max(i - ctl(2), u + ctl(2)));
	ps->blue = min(255, max(0, c));
}

static void __fastcall Ultra(RGBA* ps, mif& src, int x, int y)
{
// c+ctl(1)-dif(i,u+ctl(0)-128)
// c+ctl(2)-dif(i,u+ctl(0)-128)
// c+ctl(3)-dif(i,u+ctl(0)-128)

	int i = yuvY(ps);
	int u = yuvU(ps);
	int c;
	c = (int)ps->red + ctl(1) - dif(i, u  + ctl(0) - 128);
	ps->red = min(255, max(0, c));

	c = (int)ps->green + ctl(2) - dif(i, u  + ctl(0) - 128);
	ps->green = min(255, max(0, c));

	c = (int)ps->blue + ctl(3) - dif(i, u  + ctl(0) - 128);
	ps->blue = min(255, max(0, c));
}

static void __fastcall NeonWaterColor(RGBA* ps, mif& src, int x, int y)
{
	int C = 255;

	RGBA* p[13];
	p[ 0] =	getsrc(src,x,y-3);
	p[ 1] =	getsrc(src,x-2,y-2);
	p[ 2] =	getsrc(src,x+2,y-2);
	p[ 3] =	getsrc(src,x-1,y-1);
	p[ 4] =	getsrc(src,x+1,y-1);
	p[ 5] =	getsrc(src,x-3,y);
	p[ 6] =	ps;
	p[ 7] =	getsrc(src,x+3,y);
	p[ 8] =	getsrc(src,x-1,y+1);
	p[ 9] =	getsrc(src,x+1,y+1);
	p[10] =	getsrc(src,x-2,y+2);
	p[11] =	getsrc(src,x+2,y+2);
	p[12] =	getsrc(src,x,y+3);

	int r, g, b;
	r = g = b = 0;
	for (int i = 0; i < 13; i++)
	{
		r += (int)p[i]->red;
		g += (int)p[i]->green;
		b += (int)p[i]->blue;
	}

	int c;
	c = ps->red;
	c = mix(((r/13) & C << 6)*100/80,c,C,val(0,255,1));
	ps->red = CLAMP(c);

	c = ps->green;
	c = mix(((g/13) & C << 6)*100/80,c,C,val(0,255,1));
	ps->green = CLAMP(c);

	c = ps->blue;
	c = mix(((b/13) & C << 6)*100/80,c,C,val(0,255,1));
	ps->blue = CLAMP(c);
}

static void __fastcall Neon(RGBA* ps, mif& src, int x, int y)
{
	int r = ps->red;
	int g = ps->green;
	int b = ps->blue;

	int c;
	c = mix((255+ctl(0))-(g+b),r,ctl(0),255);
	ps->red = CLAMP(c);
	c = mix((255+ctl(1))-(r+b),g,ctl(1),255);
	ps->green = CLAMP(c);
	c = mix((255+ctl(2))-(g+r),b,ctl(2),255);
	ps->blue = CLAMP(c);
}

static void __fastcall SplitContrast(RGBA* ps, mif& src, int x, int y)
{
	int c;
	c = ps->red;
	c = (c-dif(c,mix(255,c,ctl(1),128))+dif(c,mix(0,c,ctl(0),128)));
	ps->red = CLAMP(c);

	c = ps->green;
	c = (c-dif(c,mix(255,c,ctl(1),128))+dif(c,mix(0,c,ctl(0),128)));
	ps->green = CLAMP(c);

	c = ps->blue;
	c = (c-dif(c,mix(255,c,ctl(1),128))+dif(c,mix(0,c,ctl(0),128)));
	ps->blue = CLAMP(c);
}

static void __fastcall Posterize(RGBA* ps, mif& src, int x, int y)
{
	int i = yuvY(ps);

	put(i%val(0,255,1),0);
	put(val(0,255,1)/2,1);

	int c;
	c = (int)ps->red-get(0)+get(1);
	ps->red = CLAMP(c);
	c = (int)ps->green-get(0)+get(1);
	ps->green = CLAMP(c);
	c = (int)ps->blue-get(0)+get(1);
	ps->blue = CLAMP(c);
}

static void __fastcall Blackout(RGBA* ps, mif& src, int x, int y)
{
	int i = yuvY(ps);
	ps->red   = max(ctl(0)-i,ps->red);
	ps->green = max(ctl(1)-i,ps->green);
	ps->blue  = max(ctl(2)-i,ps->green);
}

static void __fastcall Bleach(RGBA* ps, mif& src, int x, int y)
{
// c+ctl(0)
// c+ctl(0)
// c+ctl(0)
	int c;
	c = (int)ps->red + ctl(0);
	ps->red = CLAMP(c);
	c = (int)ps->green + ctl(0);
	ps->green = CLAMP(c);
	c = (int)ps->blue + ctl(0);
	ps->blue = CLAMP(c);
}

static void __fastcall Rusty(RGBA* ps, mif& src, int x, int y)
{
// (c+ctl(0)%v*u)+128-ctl(1)
// (c+ctl(0)%v*u)+128-ctl(2)
// (c+ctl(0)%v*u)+128-ctl(3)

	int v = yuvV(ps);
	int u = yuvU(ps);
	int c;

	int n = (v*u) ? v*u : 1;

	c = (int)ps->red;
	c = (c+ctl(0)%n)+128-ctl(1);
	ps->red = CLAMP(c);

	c = (int)ps->green;
	c = (c+ctl(0)%n)+128-ctl(2);
	ps->green = CLAMP(c);

	c = (int)ps->blue;
	c = (c+ctl(0)%n)+128-ctl(3);
	ps->blue = CLAMP(c);
}

static void __fastcall RemoveGrayOrg(RGBA* ps, mif& src, int x, int y)
{
	int r = ps->red;
	int g = ps->green;
	int b = ps->blue;
	put(min(r,min(g,b)),0);

	int c;
	c = (r-get(0)+val(0,0,get(0)))*val(3,1,10);
	ps->red = CLAMP(c);
	c = (g-get(0)+val(1,0,get(0)))*val(3,1,10);
	ps->green = CLAMP(c);
	c = (b-get(0)+val(2,0,get(0)))*val(3,1,10);
	ps->blue = CLAMP(c);
}

// for Fade
static MAGIC parrFade = {
	_T("フェード"),
	Fade,
	3,
	{	{ 128, 128, _T("赤のフェード") },
		{ 128, 128, _T("緑のフェード") },
		{ 128, 128, _T("青のフェード") },		}
};

// for Heat
static MAGIC parrHeat = {
	_T("ヒート"),
	Heat,
	2,
	{	{ 128, 128, _T("範囲 1") },
		{ 128, 128, _T("範囲 2") },		}
};

// for Tint
static MAGIC parrTint = {
	_T("色合い"),
	Tint,
	4,
	{	{ 128, 128, _T("コントラスト") },
		{ 128, 128, _T("水色 - 赤の色合い") },
		{ 128, 128, _T("紫 - 緑の色合い") },
		{ 128, 128, _T("黄 - 青の色合い") },	}
};

// for Pale
static MAGIC parrPale = {
	_T("薄暗さ"),
	Pale,
	3,
	{	{  37,  37, _T("水色 - 赤のミックス") },
		{  64,  64, _T("紫 - 緑のミックス") },
		{ 105, 105, _T("黄 - 青のミックス") },	}
};

// for Ultra
static MAGIC parrUltra = {
	_T("ウルトラ"),
	Ultra,
	4,
	{	{ 128, 128, _T("範囲") },
		{  99,  99, _T("水色 - 赤のミックス") },
		{ 130, 130, _T("紫 - 緑のミックス") },
		{ 168, 168, _T("黄 - 青のミックス") },	}
};

// for NeonWaterColor
static MAGIC parrNeonWaterColor = {
	_T("ネオン水彩"),
	NeonWaterColor,
	1,
	{	{ 151, 151, _T("フェード") },	}
};

// for Neon
static MAGIC parrNeon = {
	_T("ネオン"),
	Neon,
	3,
	{	{ 255, 255, _T("赤") },
		{ 189, 189, _T("緑") },
		{ 167, 167, _T("青") },		}
};

// for SplitContrast
static MAGIC parrSplitContrast = {
	_T("コントラスト分割"),
	SplitContrast,
	2,
	{	{  64,  64, _T("明るさ") },
		{  32,  32, _T("暗さ") },		}
};

// for Posterize
static MAGIC parrPosterize = {
	_T("ポスタライズ"),
	Posterize,
	1,
	{	{  227, 227, _T("ステップ") },		}
};

// for Blackout
static MAGIC parrBlackout = {
	_T("ブラック アウト"),
	Blackout,
	3,
	{	{ 190, 190, _T("赤のアウト") },
		{ 164, 164, _T("緑のアウト") },
		{ 138, 138, _T("青のアウト") },	}
};

// for Bleach
static MAGIC parrBleach = {
	_T("ブリーチ"),
	Bleach,
	1,
	{	{ 128, 128, _T("最小 / 最大") },	}
};

// for Rusty
static MAGIC parrRusty = {
	_T("ラスティ"),
	Rusty,
	4,
	{	{ 200, 200, _T("範囲") },
		{ 100, 100, _T("赤 ←→ 水色") },
		{ 150, 150, _T("緑 ←→ 紫") },
		{ 200, 200, _T("青 ←→ 黄") },		}
};

// for RemoveGrayOrg
static MAGIC parrRemoveGrayOrg = {
	_T("グレーの除去（オリジナル）"),
	RemoveGrayOrg,
	4,
	{	{   0,   0, _T("赤") },
		{   0,   0, _T("緑") },
		{   0,   0, _T("青") },
		{  80,  80, _T("ブースト") },		}
};

/////////////////////////////////////////////////////////////////////
//

static MAGIC* filter_array[] = {
	&parrFade,
	&parrHeat,
	&parrTint,
	&parrPale,
	&parrUltra,
	&parrNeonWaterColor,
	&parrNeon,
	&parrSplitContrast,
	&parrPosterize,
	&parrBlackout,
	&parrBleach,
	&parrRusty,
	&parrRemoveGrayOrg,
};
#define MAX_FILTER			(sizeof(filter_array)/sizeof(MAGIC*))

#endif		// MORE_FILTE_FUNCTION

//////////////////////////////////////////////////////////////////////
//

HANDLE colorify_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrColorizeRGB, PREVIEW_BOTH);
}

HANDLE colorify_hsb_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrColorizeHSB, PREVIEW_BOTH);
}

HANDLE combolv_maxcompare(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	mif image(hdib);
	if (magic_works(image, prc, &parrMaxCompare))
		return image.createdib();
	return NULL;
}

HANDLE combolv_mincompare(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	mif image(hdib);
	if (magic_works(image, prc, &parrMinCompare))
		return image.createdib();
	return NULL;
}

HANDLE color_exposure_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrExposure, PREVIEW_BOTH);
}

HANDLE color_booster_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrColorBooster, PREVIEW_BOTH);
}

HANDLE color_watercolor_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrWaterColor, PREVIEW_BOTH);
}

HANDLE color_histogramstretch_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrHistogramStretch, PREVIEW_BOTH);
}

HANDLE color_solarize_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSolarize, PREVIEW_BOTH);
}

HANDLE color_contrast_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrContrast, PREVIEW_BOTH);
}

HANDLE color_converse_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrConverse, PREVIEW_BOTH);
}

HANDLE color_dual_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrDual, PREVIEW_BOTH);
}

HANDLE color_inverse_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrInverse, PREVIEW_BOTH);
}

HANDLE color_reverse_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrReverse, PREVIEW_BOTH);
}

HANDLE color_mono_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrMono, PREVIEW_BOTH);
}

HANDLE color_sepia_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSepia, PREVIEW_BOTH);
}

HANDLE color_solar_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSolar, PREVIEW_BOTH);
}

HANDLE color_removegray_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrRemoveGray, PREVIEW_BOTH);
}

HANDLE color_oldphoto_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrOldPhoto, PREVIEW_BOTH);
}

HANDLE color_graygray_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrGrey, PREVIEW_BOTH);
}

HANDLE color_temperature_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrColorTemperature, PREVIEW_BOTH);
}

HANDLE color_skylight_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSkylight, PREVIEW_BOTH);
}

HANDLE color_pastel_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrPastel, PREVIEW_BOTH);
}

