//////////////////////////////////////////////////////////////////////
// mosaic_works.cpp

#include "stdafx.h"
#include "ffactory_sup.h"

extern PROGRESSCALLBACK pfnProgress;

//////////////////////////////////////////////////////////////////////
//


static void __fastcall PlainMosaicBlur(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,1,255),2);
	put(val(1,1,255),3);
	put(x%get(2),0);
	put(y%get(3),1);

	RGBA* p1 = getsrc(src,x-get(0)+get(2),y-get(1)+get(3));
	RGBA* p2 = getsrc(src,x-get(0),y-get(1));

	int c;
	c = mix(p1->red,p2->red,get(0)*get(1),get(2)*get(3));
	c = ctl(3)<128?(get(0)==0||get(1)==0)?0:c:c;
	ps->red = CLAMP(c);

	c = mix(p1->green,p2->green,get(0)*get(1),get(2)*get(3));
	c = ctl(3)<128?(get(0)==0||get(1)==0)?0:c:c;
	ps->green = CLAMP(c);

	c = mix(p1->blue,p2->blue,get(0)*get(1),get(2)*get(3));
	c = ctl(3)<128?(get(0)==0||get(1)==0)?0:c:c;
	ps->blue = CLAMP(c);

//	ps->alpha = mix(p1->alpha,p2->alpha,get(0)*get(1),get(2)*get(3));
}

static void __fastcall BlendMosaic(RGBA* ps, mif& src, int x, int y)
{
	put(ctl(0)?ctl(0):1,0);
	RGBA* p1 = getsrc(src,x-x%get(0),y-y%get(0));
	RGBA* p2 = getsrc(src,x-x%get(0)+(get(0)-1),y-y%get(0)+(get(0)-1));
	int c;
	c = (!((x%(get(0)*2))<get(0))^((y%(get(0)*2))<get(0)))*255?scl(x%get(0),0,get(0)-1,
	p1->red,p2->red):scl(y%get(0),0,get(0)-1,p1->red,p2->red);
	ps->red = CLAMP(c);

	c = (!((x%(get(0)*2))<get(0))^((y%(get(0)*2))<get(0)))*255?scl(x%get(0),0,get(0)-1,
	p1->green,p2->green):scl(y%get(0),0,get(0)-1,p1->green,p2->green);
	ps->green = CLAMP(c);

	c = (!((x%(get(0)*2))<get(0))^((y%(get(0)*2))<get(0)))*255?scl(x%get(0),0,get(0)-1,
	p1->blue,p2->blue):scl(y%get(0),0,get(0)-1,p1->blue,p2->blue);
	ps->blue = CLAMP(c);
}

static void __fastcall PlainMosaic2(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,1,255),2);
	put(val(1,1,255),3);
	put(x%get(2),0);
	put(y%get(3),1);
	put(scl(abs(isin(scl(x,0,get(2)*2,-512,512))*isin(scl(y,0,get(3)*2,-512,512))),0,262144,0,255),5);

	RGBA* p1 = getsrc(src,x-get(0)+get(2)/2,y-get(1)+get(3)/2);

	put(p1->red,4);
	int c;
	c = ctl(3)<85?(get(0)==0||get(1)==0)?0:get(4):ctl(3)>170?get(4):get(5)*get(4)/255;
	ps->red = CLAMP(c);

	put(p1->green,4);
	c = ctl(3)<85?(get(0)==0||get(1)==0)?0:get(4):ctl(3)>170?get(4):get(5)*get(4)/255;
	ps->green = CLAMP(c);

	put(p1->blue,4);
	c = ctl(3)<85?(get(0)==0||get(1)==0)?0:get(4):ctl(3)>170?get(4):get(5)*get(4)/255;
	ps->blue = CLAMP(c);
}

static void __fastcall Ommadawn(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,1,255),2);
	put(val(1,1,255),3);
	put(x%get(2),0);
	put(y%get(3),1);

	RGBA* p4 = getsrc(src,x-get(0),y-get(1));
	RGBA* p5 = getsrc(src,x-get(0)+get(2),y-get(1)+get(3));
	RGBA* p7 = getsrc(src,x-get(0),y-get(1)+get(3));
	RGBA* p8 = getsrc(src,x-get(0)+get(2),y-get(1));

	int c;
	put(p4->red,4);
	put(p5->red,5);
	put(p7->red,7);
	put(p8->red,8);
	put((get(5)*get(0)*get(1)+get(4)*(get(2)-get(0))*(get(3)-get(1))+get(8)*get(0)*(get(3)-get(1))+get(7)*(get(2)-get(0))*get(1))/(get(2)*get(3)),6);
	c = ctl(3)<128?(get(0)==0||get(1)==0)?0:get(6):get(6);
	ps->red = CLAMP(c);

	put(p4->green,4);
	put(p5->green,5);
	put(p7->green,7);
	put(p8->green,8);
	put((get(5)*get(0)*get(1)+get(4)*(get(2)-get(0))*(get(3)-get(1))+get(8)*get(0)*(get(3)-get(1))+get(7)*(get(2)-get(0))*get(1))/(get(2)*get(3)),6);
	c = ctl(3)<128?(get(0)==0||get(1)==0)?0:get(6):get(6);
	ps->green = CLAMP(c);

	put(p4->blue,4);
	put(p5->blue,5);
	put(p7->blue,7);
	put(p8->blue,8);
	put((get(5)*get(0)*get(1)+get(4)*(get(2)-get(0))*(get(3)-get(1))+get(8)*get(0)*(get(3)-get(1))+get(7)*(get(2)-get(0))*get(1))/(get(2)*get(3)),6);
	c = ctl(3)<128?(get(0)==0||get(1)==0)?0:get(6):get(6);
	ps->blue = CLAMP(c);
}

static void __fastcall Weaver(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,1,255),2);
	put(val(1,1,255),3);
	put(x%get(2),0);
	put(y%get(3),1);

	RGBA* p4 = getsrc(src,x,y-get(1)+get(3));
	RGBA* p5 = getsrc(src,x-get(0)+get(2),y);
	RGBA* p7 = getsrc(src,x-get(0),y);
	RGBA* p8 = getsrc(src,x,y-get(1));

	int c;
	put(p4->red,4);
	put(p5->red,5);
	put(p7->red,7);
	put(p8->red,8);
	put((get(5)*get(0)*get(1)+get(4)*(get(2)-get(0))*(get(3)-get(1))+get(8)*get(0)*(get(3)-get(1))+get(7)*(get(2)-get(0))*get(1))/(get(2)*get(3)),6);
	c = ctl(2)<128?(get(0)==0||get(1)==0)?0:get(6):get(6);
	ps->red = CLAMP(c);

	put(p4->green,4);
	put(p5->green,5);
	put(p7->green,7);
	put(p8->green,8);
	put((get(5)*get(0)*get(1)+get(4)*(get(2)-get(0))*(get(3)-get(1))+get(8)*get(0)*(get(3)-get(1))+get(7)*(get(2)-get(0))*get(1))/(get(2)*get(3)),6);
	c = ctl(2)<128?(get(0)==0||get(1)==0)?0:get(6):get(6);
	ps->green = CLAMP(c);

	put(p4->blue,4);
	put(p5->blue,5);
	put(p7->blue,7);
	put(p8->blue,8);
	put((get(5)*get(0)*get(1)+get(4)*(get(2)-get(0))*(get(3)-get(1))+get(8)*get(0)*(get(3)-get(1))+get(7)*(get(2)-get(0))*get(1))/(get(2)*get(3)),6);
	c = ctl(2)<128?(get(0)==0||get(1)==0)?0:get(6):get(6);
	ps->blue = CLAMP(c);
}

static void __fastcall Dot(RGBA* ps, mif& src, int x, int y)
{
	int n0=ctl(0)?ctl(0):1;
	int n1=val(1,1,200);
	put(x%n0-n0/2,0);
	put(y%n0-n0/2,1);
	put((int)(sqrt((double)get(0)*get(0)+get(1)*get(1))*200),2);
	if (get(2)<=n1*n0)
	{
		RGBA* p1 = getsrc(src,x-get(0),y-get(1));
		*ps = *p1;
	}
	else
		ps->red=ps->green=ps->blue=0;
}

static void __fastcall Diagonal(RGBA* ps, mif& src, int x, int y)
{
	int n0=ctl(0)?ctl(0):1;
	int n1=val(1,1,200);
	put(x%n0-n0/2,0);
	put(y%n0-n0/2,1);
	if ((abs(get(0))+abs(get(1)))*100 <= n0*n1)
	{
		RGBA* p1 = getsrc(src,x-get(0),y-get(1));
		*ps = *p1;
	}
	else
		ps->red=ps->green=ps->blue=0;
}

static void __fastcall HoneyCombo(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,1,100),0);
	int s2=2*get(0);
	int s3=3*get(0);
	int s4=4*get(0);
	int s6=6*get(0);
	int xx=x%s6;
	int yy=y%s4;
	int xp, yp;
	if (yy < s2) {
		if (xx < s3) {
			if ((xx * xx + yy * yy) < ((xx - s3) * (xx - s3) + (yy - s2) * (yy - s2))) {
				xp = 0;
				yp = 0;
			}
			else {
				xp = 3;
				yp = 2;
			}
		}
		else  // xx>=3 
		{
			if (((s6 - xx) * (s6 - xx) + yy * yy) < ((xx - s3) * (xx - s3) + (yy - s2) * (yy - s2)))
			{
				xp = 6;
				yp = 0;
			}
			else
			{
				xp = 3;
				yp = 2;
			}
		}
	}
	else // yy>=2 
	{
		if (xx < s3)
		{
			if ((xx * xx + (s4 - yy) * (s4 - yy)) < ((xx - s3) * (xx - s3) + (yy - s2) * (yy - s2)))
			{
				xp = 0;
				yp = 4;
			}
			else
			{
				xp = 3;
				yp = 2;
			}
		}
		else  // xx>3 
		{
			if (((s6 - xx) * (s6 - xx) + (s4 - yy) * (s4 - yy)) < ((xx - s3) * (xx - s3) + (yy - s2) * (yy - s2)))
			{
				xp = 6;
				yp = 4;
			}
			else
			{
				xp = 3;
				yp = 2;
			}
		}
	}
	xx = (x - xx + xp * get(0));
	yy = (y - yy + yp * get(0));
	RGBA* p1 = getsrc(src, xx, yy);
	*ps = *p1;
}

static void __fastcall AliasBlur(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,1,255),0);
	put(val(1,1,255),1);

	RGBA* p1 = getsrc(src,x-x%get(0),y-y%get(1));
	RGBA* p2 = getsrc(src,x-x%(get(0)-get(0)/3),y-y%(get(1)-get(1)/3));
	RGBA* p3 = getsrc(src,x-x%(get(0)-2*get(0)/3),y-y%(get(1)-2*get(1)/3));

	ps->red=((int)p1->red+(int)p2->red+(int)p3->red)/3;
	ps->green=((int)p1->green+(int)p2->green+(int)p3->green)/3;
	ps->blue=((int)p1->blue+(int)p2->blue+(int)p3->blue)/3;
}

static void __fastcall Mosapic(RGBA* ps, mif& src, int x, int y)
{
//	put(val(0,1,X),1);
//	put(val(0,1,Y),2);

	put(val(0,1,255),1);
	if (X>Y)
		put(get(1)*Y/X,2);
	else
		put(get(1)*X/Y,2);
	if (!get(2))
		put(1,2);

	RGBA* p0 = getsrc(src,(x*X/get(1))%X,(y*Y/get(2))%Y);
	put(((int)p0->red+(int)p0->green+(int)p0->blue)/3,0);

	RGBA* p1 = getsrc(src,x-x%get(1)+val(1,0,get(1)),y-y%get(2)+val(2,0,get(2)));

	int c;
	c = get(0)+(int)p1->red-128;
	ps->red = CLAMP(c);

	c = get(0)+(int)p1->green-128;
	ps->green = CLAMP(c);

	c = get(0)+(int)p1->blue-128;
	ps->blue = CLAMP(c);
}

static void __fastcall Sphere(RGBA* ps, mif& src, int x, int y)
{
	int cell = ctl(0)?ctl(0):1;
	int x_cen = x - x%cell + cell/2;
	int y_cen = y - y%cell + cell/2;

	int C = 255;
	int m = dist(x, y, x_cen, y_cen);

	put(val(1,0,cell),2);
	put(scl(c2m(x%cell-val(2,0,cell), y%cell-val(3,0,cell)),0, get(2),0,C), 1);

	RGBA* p1 = getsrc(src, x_cen, y_cen);
	int r = p1->red;
	int g = p1->green;
	int b = p1->blue;

	r = m < get(2) ? scl(get(1),0,C, C, r*200/255) : 0;
	g = m < get(2) ? scl(get(1),0,C, C, g*200/255) : 0;
	b = m < get(2) ? scl(get(1),0,C, C, b*200/255) : 0;

	int c;
	c = mix(r*2,g+b,C,val(4,C,1))/2;
	ps->red = CLAMP(c);

	c = mix(g*2,r+b,C,val(4,C,1))/2;
	ps->green = CLAMP(c);

	c = mix(b*2,r+g,C,val(4,C,1))/2;
	ps->blue = CLAMP(c);
}

static void __fastcall Triangle(RGBA* ps, mif& src, int x, int y)
{
	put(ctl(0)?ctl(0):1,5);

	put(x%get(5),0);
	put(y%get(5),1);
	put((get(0)<get(1)),2);
	put(((get(5)-get(0)<get(1))+1)*2,3);
	put(((get(2)|get(3)-2)+ctl(1))&3,4);

	RGBA* p1;
	switch (get(4))
	{
		case 0: p1 = getsrc(src,x-get(0),y-get(1)-get(5)/2); break;
		case 1: p1 = getsrc(src,x-get(0)-get(5)/2,y-get(1)); break;
		case 2: p1 = getsrc(src,x-get(0),y-get(1)+get(5)/2); break;
		case 3: 
		default:p1 = getsrc(src,x-get(0)+get(5)/2,y-get(1)); break;
	}
	*ps = *p1;
}

static void __fastcall SectorMosaic(RGBA* ps, mif& src, int x, int y)
{
	int d = c_dir(x,y);
	int m = c_dist(x,y);
//	rad(src,(d-d%val(0,1,X/2)),abs(m-m%val(1,1,Y/2)),ps);
	rad(src,(d-d%val(0,1,255)),abs(m-m%val(1,1,255)),ps);
}

/////////////////////////////////////////////////////////////////////
//

// for PlainMosaicBlur
static MAGIC parrPlainMosaicBlur = {
	_T("スクエア シェード 1"),
	PlainMosaicBlur,
	4,
	{	{  15,  15, _T("セルの幅") },
		{  23,  23, _T("セルの高さ") },
		{   0,   0, NULL },
		{  18,  18, _T("エッジのオン・オフ") },	}
};

// for BlendMosaic
static MAGIC parrBlendMosaic = {
	_T("スクエア シェード 2"),
	BlendMosaic,
	1,
	{	{  32,  32, _T("セルのサイズ") },		}
};

// for PlainMosaic2
static MAGIC parrPlainMosaic2 = {
	_T("プレーン"),
	PlainMosaic2,
	4,
	{	{  22,  22, _T("セルの幅") },
		{  18,  18, _T("セルの高さ") },
		{   0,   0, NULL },
		{ 124, 124, _T("エッジのハード・ソフト・オフ") },	}
};

// for Ommadawn
static MAGIC parrOmmadawn = {
	_T("フレア"),
	Ommadawn,
	4,
	{	{  17,  17, _T("セルの幅") },
		{  16,  16, _T("セルの高さ") },
		{   0,   0, NULL },
		{  17,  17, _T("エッジのオン・オフ") },	}
};

// for Dot
static MAGIC parrDot = {
	_T("ドットイメージ"),
	Dot,
	2,
	{	{  20,  20, _T("セルサイズ") },
		{ 128, 128, _T("ドット比率") },			}
};

// for Diagonal
static MAGIC parrDiagonal = {
	_T("クロスチェッカー"),
	Diagonal,
	2,
	{	{  16,  16, _T("セルサイズ") },
		{  70,  70, _T("チェッカ比率") },			}
};

// for HoneyCombo
static MAGIC parrHoneyCombo = {
	_T("蜂の巣"),
	HoneyCombo,
	1,
	{	{  16,  16, _T("セル スケール") },		}
};

// for AliasBlur
static MAGIC parrAliasBlur = {
	_T("エリアス"),
	AliasBlur,
	2,
	{	{  36,  36, _T("横のエリアス") },
		{  36,  36, _T("縦のエリアス") },	}
};

// for Mosapic
static MAGIC parrMosapic = {
	_T("リバーブ"),
	Mosapic,
	3,
	{	{  20,  20, _T("タイルサイズ") },
		{  20,  20, _T("横方向の調整") },
		{  20,  20, _T("縦方向の調整") },	}
};

// for Triangle
static MAGIC parrTriangle = {
	_T("トライアングル"),
	Triangle,
	2,
	{	{  20,  20, _T("タイルサイズ") },
		{  42,  42, _T("モード") },	}
};

// for SectorMosaic
static MAGIC parrSectorMosaic = {
	_T("セクター"),
	SectorMosaic,
	2,
	{	{  30,  30, _T("円周方向タイルサイズ") },
		{  21,  21, _T("円心方向タイルサイズ") },	}
};

// for Sphere
static MAGIC parrSphere = {
	_T("カラー ビーズ"),
	Sphere,
	5,
	{	{  12,  12, _T("サイズ") },
		{ 148, 148, _T("ビーズの比率") },
		{  98,  98, _T("光源 X") },
		{  98,  98, _T("光源 Y") },
		{ 128, 128, _T("彩度") },	}
};

// for Weaver
static MAGIC parrWeaver = {
	_T("織り込み"),
	Weaver,
	3,
	{	{  59,  59, _T("セルの幅") },
		{  48,  48, _T("セルの高さ") },
		{ 222, 222, _T("エッジのオン・オフ") },	}
};


//////////////////////////////////////////////////////////////////////
//


HANDLE mosaic_sphere_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSphere, PREVIEW_PART);
}

HANDLE mosaic_weaver_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrWeaver, PREVIEW_PART);
}

HANDLE mosaic_plainmosaicblur_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrPlainMosaicBlur, PREVIEW_PART);
}

HANDLE mosaic_blendmosaic_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrBlendMosaic, PREVIEW_PART);
}

HANDLE mosaic_plainmosaic2_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrPlainMosaic2, PREVIEW_PART);
}

HANDLE mosaic_ommadawn_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrOmmadawn, PREVIEW_PART);
}

HANDLE mosaic_dot_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrDot, PREVIEW_PART);
}

HANDLE mosaic_diagonal_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrDiagonal, PREVIEW_PART);
}

HANDLE mosaic_honeycombo_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrHoneyCombo, PREVIEW_PART);
}

HANDLE mosaic_aliasblur_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrAliasBlur, PREVIEW_PART);
}

HANDLE mosaic_mosapic_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrMosapic, PREVIEW_PART);
}

HANDLE mosaic_triangle_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrTriangle, PREVIEW_PART);
}

HANDLE mosaic_sectormosaic_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSectorMosaic, PREVIEW_WHOLE);
}

