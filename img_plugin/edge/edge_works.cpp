//////////////////////////////////////////////////////////////////////
// edge_works.cpp

#include "stdafx.h"
#include "ffactory_sup.h"


//////////////////////////////////////////////////////////////////////
//

static void __fastcall Artistic(RGBA* ps, mif& src, int x, int y)
{
// dif(i,ctl(0))^c
// dif(i,ctl(1))^c
// dif(i,ctl(2))^c

	int i = yuvY(ps);
	ps->red = (BYTE)dif(i, ctl(0)) ^ ps->red;
	ps->green = (BYTE)dif(i, ctl(1)) ^ ps->green;
	ps->blue = (BYTE)dif(i, ctl(0)) ^ ps->blue;
}


static void __fastcall Bright(RGBA* ps, mif& src, int x, int y)
{
// c-(ctl(1)-g-b)%ctl(0)
// c-(ctl(2)-g-b)%ctl(0)
// c-(ctl(3)-g-b)%ctl(0)

	int mod = ctl(0);
	if (!mod)
		mod = 1;

	int g = ps->green;
	int b = ps->blue;
	int c;
	c = (int)ps->red - (ctl(1) - g - b) % mod;
	ps->red = CLAMP(c);

	c = (int)ps->green - (ctl(2) - g - b) % mod;
	ps->green = CLAMP(c);

	c = (int)ps->blue - (ctl(3) - g - b) % mod;
	ps->blue = CLAMP(c);
}

static void __fastcall Chromium(RGBA* ps, mif& src, int x, int y)
{
// abs(cos((i+ctl(1))+i*ctl(0)/32))+ctl(2)
// abs(cos((i+ctl(1))+i*ctl(0)/32))+ctl(3)
// abs(cos((i+ctl(1))+i*ctl(0)/32))+ctl(4)

	int i = yuvY(ps);
	int n = abs(icos((i+ctl(1))+i*ctl(0)/32));
	int c;
	c = n + ctl(2);
	ps->red = CLAMP(c);
	c = n + ctl(3);
	ps->green = CLAMP(c);
	c = n + ctl(4);
	ps->blue = CLAMP(c);
}

static void __fastcall Corrode(RGBA* ps, mif& src, int x, int y)
{
// dif(i,ctl(0))+dif(ctl(1),c&i)
// dif(i,ctl(0))+dif(ctl(2),c&i)
// dif(i,ctl(0))+dif(ctl(3),c&i)

	int i = yuvY(ps);
	int n = dif(i, ctl(0));
	int c;
	c = n + dif(ctl(1), (int)ps->red & i);
	ps->red = CLAMP(c);

	c = n + dif(ctl(2), (int)ps->green & i);
	ps->green = CLAMP(c);

	c = n + dif(ctl(3), (int)ps->blue & i);
	ps->blue = CLAMP(c);
}

static void __fastcall Contour(RGBA* ps, mif& src, int x, int y)
{
// dif(i,ctl(0))*dif(i,ctl(1))+c
// dif(i,ctl(0))*dif(i,ctl(1))+c
// dif(i,ctl(0))*dif(i,ctl(1))+c

	int i = yuvY(ps);
	int n = dif(i, ctl(0)) * dif(i, ctl(1));
	int c;
	c = n + (int)ps->red;
	ps->red = CLAMP(c);

	c = n + (int)ps->green;
	ps->green = CLAMP(c);

	c = n + (int)ps->blue;
	ps->blue = CLAMP(c);
}

static void __fastcall Depth(RGBA* ps, mif& src, int x, int y)
{
// c+ctl(0)%i
// c+ctl(0)%i
// c+ctl(0)%i

	int i = yuvY(ps);
	if (!i)
		i = 1;
	int n = ctl(0) % i;
	int c;
	c = n + (int)ps->red;
	ps->red = CLAMP(c);

	c = n + (int)ps->green;
	ps->green = CLAMP(c);

	c = n + (int)ps->blue;
	ps->blue = CLAMP(c);
}

static void __fastcall Digitize(RGBA* ps, mif& src, int x, int y)
{
//c^ctl(0)/5
//c^ctl(0)/5
//c^ctl(0)/5

	ps->red = ps->red ^ ctl(0) / 5;
	ps->green = ps->green ^ ctl(0) / 5;
	ps->blue = ps->blue ^ ctl(0) / 5;
}

static void __fastcall Dissolve(RGBA* ps, mif& src, int x, int y)
{
// abs(i<<ctl(0)/i)
// abs(i<<ctl(1)/i)
// abs(i<<ctl(2)/i)

	int i = yuvY(ps);
	i = i ? i : 1;
	int c;
	c = abs(i << ctl(0) / i);
	ps->red = CLAMP(c);

	c = abs(i << ctl(1) / i);
	ps->green = CLAMP(c);

	c = abs(i << ctl(2) / i);
	ps->blue = CLAMP(c);
}

static void __fastcall Inflame(RGBA* ps, mif& src, int x, int y)
{
// c+ctl(3)+128
// c^ctl(0)/(10-i/10)+ctl(1)-ctl(2)
// 0

	int i = yuvY(ps);
	int c;
	c = (int)ps->red + ctl(3) + 128;
	ps->red = CLAMP(c);

	int n = 10-i/10;
	if (!n)
		n = 1;
	c = (int)ps->green^ctl(0)/n+ctl(1)-ctl(2);
	ps->green = CLAMP(c);

	ps->blue = 0;
}

static void __fastcall Freeze(RGBA* ps, mif& src, int x, int y)
{
// c+D/ctl(0)*i%ctl(1)
// c+D/ctl(0)*i%ctl(1)
// c+D/ctl(0)*i%ctl(1)

	int i = yuvY(ps);
	int s = ctl(0) ? ctl(0) : 1;
	int t = ctl(1) ? ctl(1) : 1;
	int n = 1024/s*i%t;

	int c;
	c = (int)ps->red + n;
	ps->red = CLAMP(c);

	c = (int)ps->green + n;
	ps->green = CLAMP(c);

	c = (int)ps->blue + n;
	ps->blue = CLAMP(c);
}

static void __fastcall Glitter(RGBA* ps, mif& src, int x, int y)
{
// add(i>>i-ctl(0),c>>c-ctl(1),c)+c
// add(i>>i-ctl(0),c>>c-ctl(1),c)+c
// add(i>>i-ctl(0),c>>c-ctl(1),c)+c

	int i = yuvY(ps);
	int c;
	c = (int)ps->red;
	c = add(i>>(i-ctl(0)),c>>(c-ctl(1)),c)+c;
	ps->red = CLAMP(c);

	c = (int)ps->green;
	c = add(i>>(i-ctl(0)),c>>(c-ctl(1)),c)+c;
	ps->green = CLAMP(c);

	c = (int)ps->blue;
	c = add(i>>(i-ctl(0)),c>>(c-ctl(1)),c)+c;
	ps->blue = CLAMP(c);
}

static void __fastcall Graphite(RGBA* ps, mif& src, int x, int y)
{
// i&ctl(0)+(i&ctl(1))+i
// i&ctl(0)+(i&ctl(1))+i
// i&ctl(0)+(i&ctl(1))+i

	int i = yuvY(ps);
	int c = i&ctl(0)+(i&ctl(1))+i;
	ps->red = ps->green = ps->blue = CLAMP(c);
}

static void __fastcall Illumine(RGBA* ps, mif& src, int x, int y)
{
// c+i>>(ctl(0)/4+c/8)
// c+i>>(ctl(1)/4+c/8)
// c+i>>(ctl(2)/4+c/8)

	int i = yuvY(ps);
	int c;
	c = (int)ps->red;
	c = (c+i)>>(ctl(0)/4+c/8);
	ps->red = CLAMP(c);

	c = (int)ps->green;
	c = (c+i)>>(ctl(1)/4+c/8);
	ps->green = CLAMP(c);

	c = (int)ps->blue;
	c = (c+i)>>(ctl(2)/4+c/8);
	ps->blue = CLAMP(c);
}

static void __fastcall Medical(RGBA* ps, mif& src, int x, int y)
{
// (i^ctl(0))*v/8+ctl(1)
// (i^ctl(0))*v/8+ctl(2)
// (i^ctl(0))*v/8+ctl(3)

	int i = yuvY(ps);
	int v = yuvV(ps);
	int n = (i^ctl(0))*v/8;
	int c;
	c = n + ctl(1);
	ps->red = CLAMP(c);

	c = n + ctl(2);
	ps->green = CLAMP(c);

	c = n + ctl(3);
	ps->blue = CLAMP(c);
}

static void __fastcall Modern(RGBA* ps, mif& src, int x, int y)
{
// (c-ctl(0))*(c-ctl(1))
// (c-ctl(0))*(c-ctl(1))
// (c-ctl(0))*(c-ctl(1))

	int c;
	c = (int)ps->red;
	c = (c-ctl(0))*(c-ctl(1));
	ps->red = CLAMP(c);

	c = (int)ps->green;
	c = (c-ctl(0))*(c-ctl(1));
	ps->green = CLAMP(c);

	c = (int)ps->blue;
	c = (c-ctl(0))*(c-ctl(1));
	ps->blue = CLAMP(c);
}

static void __fastcall Noisy(RGBA* ps, mif& src, int x, int y)
{
// rnd(i^ctl(0),i^ctl(1))
// rnd(i^ctl(0),i^ctl(1))
// rnd(i^ctl(0),i^ctl(1))

	int i = yuvY(ps);
	int n1 = i^ctl(0);
	int n2 = i^ctl(1);
	int c;
	c = rnd(n1,n2);
	ps->red = CLAMP(c);

	c = rnd(n1,n2);
	ps->green = CLAMP(c);

	c = rnd(n1,n2);
	ps->blue = CLAMP(c);
}

static void __fastcall Photocopy(RGBA* ps, mif& src, int x, int y)
{
// (i-ctl(0))*ctl(1)
// (i-ctl(0))*ctl(1)
// (i-ctl(0))*ctl(1)

	int i = yuvY(ps);
	int c = (i-ctl(0))*ctl(1);
	ps->red = ps->green = ps->blue = CLAMP(c);
}

static void __fastcall Shine(RGBA* ps, mif& src, int x, int y)
{
// c+abs(cos((i)*ctl(0)/64))-abs(i*ctl(1)/64)
// c+abs(cos((i)*ctl(0)/64))-abs(i*ctl(2)/64)
// c+abs(cos((i)*ctl(0)/64))-abs(i*ctl(3)/64)

	int i = yuvY(ps);
	int n = abs(icos((i)*ctl(0)/64));
	int c;
	c = (int)ps->red+n-abs(i*ctl(1)/64);
	ps->red = CLAMP(c);

	c = (int)ps->green+n-abs(i*ctl(2)/64);
	ps->green = CLAMP(c);

	c = (int)ps->blue+n-abs(i*ctl(3)/64);
	ps->blue = CLAMP(c);
}

static void __fastcall Woodcut(RGBA* ps, mif& src, int x, int y)
{
//(i-ctl(0))*(i-ctl(1))*(i-ctl(2))*(i-ctl(3))
//(i-ctl(0))*(i-ctl(1))*(i-ctl(2))*(i-ctl(3))
//(i-ctl(0))*(i-ctl(1))*(i-ctl(2))*(i-ctl(3))

	int i = yuvY(ps);
	int c = (i-ctl(0))*(i-ctl(1))*(i-ctl(2))*(i-ctl(3));
	ps->red = ps->green = ps->blue = CLAMP(c);
}

static void __fastcall MetallicOnion(RGBA* ps, mif& src, int x, int y)
{
	int i = yuvY(ps);

//@	put(255*((i)%val(0,255,0))/val(0,255,0),0);
	put(255*((i)%val(0,255,1))/val(0,255,1),0);

	int c;
	c = ps->red;
	c = mix(scl(c,0,255,0,get(0)),get(0),ctl(1),255);
	ps->red = CLAMP(c);

	c = ps->green;
	c = mix(scl(c,0,255,0,get(0)),get(0),ctl(1),255);
	ps->green = CLAMP(c);

	c = ps->blue;
	c = mix(scl(c,0,255,0,get(0)),get(0),ctl(1),255);
	ps->blue = CLAMP(c);
}

static void __fastcall InnerBevel(RGBA* ps, mif& src, int x, int y)
{
	int I = 255;
	int C = 255;

	RGBA* p1 = getsrc(src,x-get(0), y-get(0));
	RGBA* p2 = getsrc(src,x+get(0), y+get(0));

	put(val(0,1,4), 0);
	put(I-(
		(int)p1->red*76 +
		(int)p1->green*150+
		(int)p1->blue*29)/(val(1,512,1)),1);
	put((
		(int)p2->red*76 +
		(int)p2->green*150+
		(int)p2->blue*29)/(val(1,512,1)),2);

	int c;
	c = ps->red;
	c = scl((get(1)+get(2)), -C, C, 0, c);
	ps->red = CLAMP(c);

	c = ps->green;
	c = scl((get(1)+get(2)), -C, C, 0, c);
	ps->green = CLAMP(c);

	c = ps->blue;
	c = scl((get(1)+get(2)), -C, C, 0, c);
	ps->blue = CLAMP(c);
}

static void __fastcall Accident(RGBA* ps, mif& src, int x, int y)
{
	RGBA* p1 = getsrc3(src,x-1,y);
	RGBA* p2 = getsrc3(src,x+1,y);
	RGBA* p3 = getsrc3(src,x,y-1);
	RGBA* p4 = getsrc3(src,x,y+1);

	int c;
	c = ps->red;
	c = (dif(p1->red,p2->red)+dif(p3->red,p4->red)/2)+128-ctl(0)+c;
	ps->red = CLAMP(c);

	c = ps->green;
	c = (dif(p1->green,p2->green)+dif(p3->green,p4->green)/2)+128-ctl(0)+c;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = (dif(p1->blue,p2->blue)+dif(p3->blue,p4->blue)/2)+128-ctl(0)+c;
	ps->blue = CLAMP(c);
}

/////////////////////////////////////////////////////////////////////
//

// for Chromium
static MAGIC parrChromium = {
	_T("クロムメッキ"),
	Chromium,
	5,
	{	{   0,   0, _T("エッジ 1") },
		{ 128, 128, _T("エッジ 2") },
		{   7,   7, _T("赤") },
		{  22,  22, _T("緑") },
		{  30,  30, _T("青") },	}
};

// for Depth
static MAGIC parrDepth = {
	_T("深度"),
	Depth,
	1,
	{	{ 128, 128, _T("最小 ←→ 最大") },		}
};

// for Digitize
static MAGIC parrDigitize = {
	_T("デジタイズ"),
	Digitize,
	1,
	{	{  48,  48, _T("最小 ←→ 最大") },		}
};

// for Freeze
static MAGIC parrFreeze = {
	_T("氷結"),
	Freeze,
	2,
	{	{ 217, 217, _T("エッジ 1") },
		{ 140, 140, _T("エッジ 2") },	}
};

// for Medical
static MAGIC parrMedical = {
	_T("メディカル"),
	Medical,
	4,
	{	{ 255, 255, _T("範囲") },
		{   5,   5, _T("赤の明るさ") },
		{  14,  14, _T("緑の明るさ") },
		{  28,  28, _T("青の明るさ") },	}
};

// for Shine
static MAGIC parrShine = {
	_T("シャイン"),
	Shine,
	4,
	{	{ 128, 128, _T("範囲") },
		{ 100, 100, _T("赤・水色のミックス") },
		{ 111, 111, _T("緑・紫のミックス") },
		{ 118, 118, _T("青・黄のミックス") },	}
};

// for InnerBevel
static MAGIC parrInnerBevel = {
	_T("インナー ベベル"),
	InnerBevel,
	2,
	{	{ 160, 160, _T("ベベルサイズ") },
		{ 120, 120, _T("コントラスト") },	}
};

// for Accident
static MAGIC parrAccident = {
	_T("ネオンアップ"),
	Accident,
	1,
	{	{ 128, 128, _T("全体の明るさ") },		}
};

/////////////////////////////////////////////////////////////////////
//

#if 0

// for Artistic
static MAGIC parrArtistic = {
	_T("アーティスティック"),
	Artistic,
	3,
	{	{ 180, 180, _T("色 1") },
		{ 172, 172, _T("色 2") },
		{  70,  70, _T("色 3") },	}
};

// for Bright
static MAGIC parrBright = {
	_T("明るさ"),
	Bright,
	4,
	{	{  92,  92, _T("最小 ←→ 最大") },
		{ 107, 107, _T("エッジ 1") },
		{ 121, 121, _T("エッジ 2") },
		{ 133, 133, _T("エッジ 3") },	}
};

// for Corrode
static MAGIC parrCorrode = {
	_T("腐食"),
	Corrode,
	4,
	{	{ 128, 128, _T("範囲") },
		{  39,  39, _T("赤・水色のミックス") },
		{ 111, 111, _T("緑・紫のミックス") },
		{ 103, 103, _T("青・黄のミックス") },	}
};

// for Contour
static MAGIC parrContour = {
	_T("輪郭"),
	Contour,
	2,
	{	{ 128, 128, _T("エッジ 1") },
		{ 128, 128, _T("エッジ 2") },	}
};

// for Dissolve
static MAGIC parrDissolve = {
	_T("溶解"),
	Dissolve,
	3,
	{	{ 150, 150, _T("赤") },
		{  64,  64, _T("緑") },
		{  28,  28, _T("青") },	}
};

// for Inflame
static MAGIC parrInflame = {
	_T("炎上"),
	Inflame,
	4,
	{	{ 155, 155, _T("強度") },
		{  93,  93, _T("熱度 1") },
		{  58,  28, _T("熱度 2") },
		{  17,  17, _T("消去") },	}
};

// for Glitter
static MAGIC parrGlitter = {
	_T("きらめき"),
	Glitter,
	2,
	{	{  25,  25, _T("エッジ 1") },
		{ 100, 100, _T("エッジ 2") },	}
};

// for Graphite
static MAGIC parrGraphite = {
	_T("グラファイト"),
	Graphite,
	2,
	{	{ 110, 110, _T("エッジ 1") },
		{ 150, 150, _T("エッジ 2") },	}
};

// for Illumine
static MAGIC parrIllumine = {
	_T("電光"),
	Illumine,
	3,
	{	{  47,  47, _T("赤の明るさ") },
		{  73,  73, _T("緑の明るさ") },
		{  81,  81, _T("青の明るさ") },	}
};

// for Modern
static MAGIC parrModern = {
	_T("モダン"),
	Modern,
	2,
	{	{  84,  84, _T("カラー 1") },
		{ 145, 145, _T("カラー 2") },	}
};

// for Noisy
static MAGIC parrNoisy = {
	_T("ノイズ"),
	Noisy,
	2,
	{	{ 128, 128, _T("エッジ 1") },
		{   0,   0, _T("エッジ 2") },	}
};

// for Photocopy
static MAGIC parrPhotocopy = {
	_T("ファクシミリ 1"),
	Photocopy,
	2,
	{	{ 128, 128, _T("露出") },
		{ 128, 128, _T("コントラスト") },	}
};

// for Woodcut
static MAGIC parrWoodcut = {
	_T("ファクシミリ 2"),
	Woodcut,
	4,
	{	{ 128, 128, _T("カット 1") },
		{  50,  50, _T("カット 2") },
		{ 154, 154, _T("カット 3") },
		{ 128, 128, _T("カット 4") },	}
};

// for MetallicOnion
static MAGIC parrMetallicOnion = {
	_T("メタリック"),
	MetallicOnion,
	2,
	{	{ 177, 177, _T("腐食") },
		{  53,  53, _T("カラー") },	}
};

static MAGIC* filter_array[] = {
	&parrArtistic,
	&parrBright,
	&parrCorrode,
	&parrContour,
	&parrDissolve,
	&parrInflame,
	&parrGlitter,
	&parrGraphite,
	&parrIllumine,
	&parrModern,
	&parrNoisy,
	&parrPhotocopy,
	&parrWoodcut,
	&parrMetallicOnion,
};

#define MAX_FILTER			(sizeof(filter_array)/sizeof(MAGIC*))

//////////////////////////////////////////////////////////////////////
//

static TCHAR secRoot[] = _T("cmagic\\edge_magic");
static TCHAR keyLast[] = _T("last");


HANDLE edge_works_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	WORKS cw;
	int i;
	regkey reg(pfe->hkey, secRoot);
	if (reg.getbinary(keyLast, &cw, sizeof(cw)) != sizeof(cw) ||
		cw.filter_number < 0 || cw.filter_number >= MAX_FILTER)
	{
		cw.filter_number = 0;
		cw.wholeview = FALSE;
	}
	else
	{
		MAGIC* filter = filter_array[cw.filter_number];
		for (i = 0; i < filter->numparams; i++)
			filter->item[i].val = cw.val[i];
	}
	cw.preview_mode = PREVIEW_BOTH;
	cw.magic_array = filter_array;
	cw.num_magic = MAX_FILTER;

	mif image(hdib);
	image.crop(prc);
	cw.hdib = image.createdib();
	image.destroy();

	int r = DialogBoxParam(ghInst, _MKIR(IDD_EDGEWORKS), GetActiveWindow(), WorksDlg, (LPARAM)&cw);
	GlobalFree(cw.hdib);
	if (r == IDOK)
	{
		MAGIC* filter = filter_array[cw.filter_number];
		for (i = 0; i < filter->numparams; i++)
			cw.val[i] = filter->item[i].val;

		reg.setbinary(keyLast, &cw, sizeof(cw));

		cwait wc;
		if (image.attachdib(hdib) &&
			magic_works(image, prc, filter_array[cw.filter_number]))
			return image.createdib();
	}
	return NULL;
}

#endif


HANDLE edge_chromium_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrChromium, PREVIEW_BOTH);
}

HANDLE edge_medical_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrMedical, PREVIEW_BOTH);
}

HANDLE edge_depth_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrDepth, PREVIEW_BOTH);
}

HANDLE edge_digitize_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrDigitize, PREVIEW_BOTH);
}

HANDLE edge_freeze_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrFreeze, PREVIEW_BOTH);
}

HANDLE edge_shine_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrShine, PREVIEW_BOTH);
}

HANDLE edge_innerbevel_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrInnerBevel, PREVIEW_BOTH);
}

HANDLE edge_accident_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrAccident, PREVIEW_BOTH);
}

