//////////////////////////////////////////////////////////////////////
// expression_works.cpp

#include "stdafx.h"
#include "ffactory_sup.h"

#define MAXPARAMS		8

static void __fastcall PrintScreen(RGBA* ps, mif& src, int x, int y)
{
	put(scl(x,0,X,-512,512)*12,0);
	put(scl(y,0,X,-512,512)*12,1);
	put(isin((val(3,0,8)*get(0)+(8-val(3,0,8))*get(1))/val(0,1,50))+icos(((8-val(3,0,8))*get(0)-val(3,0,8)
*get(1))/val(0,1,50)),2);

	int c;
	c = ps->red;
	c = c+scl(get(2),-val(1,1,1024),val(2,1,1024),-128,128);
	ps->red = CLAMP(c);

	c = ps->green;
	c = c+scl(get(2),-val(1,1,1024),val(2,1,1024),-128,128);
	ps->green = CLAMP(c);

	c = ps->blue;
	c = c+scl(get(2),-val(1,1,1024),val(2,1,1024),-128,128);
	ps->blue = CLAMP(c);
}

static void __fastcall AcidRain(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,1,X),9);
	put(x%get(9),0);
	put(y%get(9),1);
	put(abs(itan((x-get(0)+ctl(4)+1)*itan(ctl(4)+y-get(1))))%255,2);
	put(scl(get(2),0,255,0,get(9)/2),4);
	put(min(get(4),dif(get(9),get(4))),6);
	put(c2m(get(4)-get(0),get(4)-get(1)),7);
	put(scl(get(7),0,get(6),ctl(1),ctl(2)),5);

	RGBA p1;
	getsrc_linear(src,x,y-scl(get(7),0,get(6),val(3,-Y/8,Y/8),-val(3,-Y/8,Y/8)),&p1);

	int c;
	c=ps->red;
	c=get(7)<get(6)?p1.red+get(5)-128:c;
	ps->red = CLAMP(c);

	c=ps->green;
	c=get(7)<get(6)?p1.green+get(5)-128:c;
	ps->green = CLAMP(c);

	c=ps->blue;
	c=get(7)<get(6)?p1.blue+get(5)-128:c;
	ps->blue = CLAMP(c);
}

static void __fastcall PlasticStarfish(RGBA* ps, mif& src, int x, int y)
{
	int d = c_dir(x, y);
	int m = c_dist(x, y);

	put(2<<val(2,3,12),12);
	put(m,11);
	put(val(0,0,M),9);
	put(val(1,0,M),8);
	put(dif(get(11),get(9)),0);
	put(scl(get(0),0,get(8),0,255),1);
	put(get(11)<get(9)?scl(get(11),0,get(9),255,0):0,7);
	put(c2m(dif(255,scl((d+val(4,0,1024)+1024)%get(12),0,get(12),0,512)),get(1)),2);
	put(get(2)<ctl(3)?max(get(7),scl(get(2),0,ctl(3),255,0)):get(7),10);

	RGBA p1;
	getsrc_linear(src,x,y+dscl(get(10),0,255,0,dval(6,0,Y/4)),&p1);
	int c;
	c = ps->red;
	c = scl(get(10),0,10,0,255)>0?p1.red-128+get(10):c*ctl(5)/128;
	ps->red = CLAMP(c);

	c = ps->green;
	c = scl(get(10),0,10,0,255)>0?p1.green-128+get(10):c*ctl(5)/128;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = scl(get(10),0,10,0,255)>0?p1.blue-128+get(10):c*ctl(5)/128;
	ps->blue = CLAMP(c);
}

static void __fastcall CrystalBall(RGBA* ps, mif& src, int x, int y)
{
	double d = dc_dir(x, y);
	double m = dc_dist(x, y);

	dput(255-dscl(dc2m(dval(0,0,X)-x,dval(1,0,Y)-y),0,M,0,dval(2,0,512)),0);

	RGBA p1;
	rad(src,d,dscl(dcos(dscl(m,0,M,0,512)),-512,512,M-M/dval(3,1,50),0),&p1);

	double c;
	c = ps->red;
	c = m<dval(4,0,M)?mix((int)p1.red+dget(0)*(dget(0)>0)-dscl(dcos(dscl(m,0,M,0,512)),-256,512,255,0),c,ctl(6),255):mix(ctl(5),c,fabs(dval(5,-128,128)),128);
	ps->red = CLAMP((int)c);

	c = ps->green;
	c = m<dval(4,0,M)?mix((int)p1.green+dget(0)*(dget(0)>0)-dscl(dcos(dscl(m,0,M,0,512)),-256,512,255,0),c,ctl(6),255):mix(ctl(5),c,fabs(dval(5,-128,128)),128);
	ps->green = CLAMP(c);

	c = ps->blue;
	c = m<dval(4,0,M)?mix((int)p1.blue+dget(0)*(dget(0)>0)-dscl(dcos(dscl(m,0,M,0,512)),-256,512,255,0),c,ctl(6),255):mix(ctl(5),c,fabs(dval(5,-128,128)),128);
	ps->blue = CLAMP(c);
}

static void __fastcall Scroll(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,0,X),0);
	put(val(1,0,Y),1);
	RGBA* p1 = getsrc2(src,x-get(0),y-get(1));
	*ps = *p1;
}

static void __fastcall ScanlineShifter(RGBA* ps, mif& src, int x, int y)
{
	RGBA* p1 = getsrc3(src,(X+x-val(0, 0, X/2))%X, y);
	RGBA* p2 = getsrc3(src,(x+val(0, 0, X/2))%X, y);
	*ps = y&(1<<val(1,0,14))?*p1:*p2;
}

static void __fastcall MarbleMadnessOne(RGBA* ps, mif& src, int x, int y)
{
//	int v1 = val(0,0,X);
	int v1 = val(0,1,X);

	put(y+val(2,0,Y),4);
	put(x+val(1,0,X)+(get(4)%(v1*2)<get(4)%(v1*4))*val(3,0,v1),3);
	put(get(3)%v1<get(3)%(v1*2)?0:255,5);
	put(get(4)%v1<get(4)%(v1*2)?0:255,6);
	put(scl(get(3)%v1,0,v1,get(5),255-get(5)),0);
	put(scl(get(4)%v1,0,v1,get(6),255-get(6)),1);
	put(255-c2m(get(0),get(1)),2);

	int c;
	c = ps->red;
	c = (get(2)<128)*(c*get(2)/128)+(get(2)>=128)*(c+(get(2)-128)*2-(c*(get(2)-128)*2/255));
	ps->red = CLAMP(c);

	c = ps->green;
	c = (get(2)<128)*(c*get(2)/128)+(get(2)>=128)*(c+(get(2)-128)*2-(c*(get(2)-128)*2/255));
	ps->green = CLAMP(c);

	c = ps->blue;
	c = (get(2)<128)*(c*get(2)/128)+(get(2)>=128)*(c+(get(2)-128)*2-(c*(get(2)-128)*2/255));
	ps->blue = CLAMP(c);
}

static void __fastcall LiftCover(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,Y,X+Y),0);
	put(isin(val(1,-1024,1024)+scl(x+y,get(0)-Y,get(0),-512,512)),1);

	RGBA* p1 = getsrc3(src,X-(y-(get(0)-X)+scl(get(1),-512,512,0,val(7,0,get(0)))),
			Y-(x-(get(0)-Y)+scl(get(1),-512,512,0,val(7,0,get(0)))));
	int c;
	c = ps->red;
	c = x+y>get(0)?scl(x+y,get(0),get(0)+val(2,0,Y),ctl(3),255):
		(x>get(0)-Y)&&(y>get(0)-X)?
		255-(255-mix(scl(get(1),-512,512,0,255),0,ctl(6),255))*
		(255-(mix(p1->red,255,ctl(4),255)*scl(get(1),-512,512,ctl(5),255))/255)/255:c;
	ps->red = CLAMP(c);

	c = ps->green;
	c = x+y>get(0)?scl(x+y,get(0),get(0)+val(2,0,Y),ctl(3),255):
		(x>get(0)-Y)&&(y>get(0)-X)?
		255-(255-mix(scl(get(1),-512,512,0,255),0,ctl(6),255))*
		(255-(mix(p1->green,255,ctl(4),255)*scl(get(1),-512,512,ctl(5),255))/255)/255:c;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = x+y>get(0)?scl(x+y,get(0),get(0)+val(2,0,Y),ctl(3),255):
		(x>get(0)-Y)&&(y>get(0)-X)?
		255-(255-mix(scl(get(1),-512,512,0,255),0,ctl(6),255))*
		(255-(mix(p1->blue,255,ctl(4),255)*scl(get(1),-512,512,ctl(5),255))/255)/255:c;
	ps->blue = CLAMP(c);
}

static void __fastcall CircuitWeaver(RGBA* ps, mif& src, int x, int y)
{
	RGBA* p1 = getsrc(src,x-((ctl(1)*(x^y))%val(0,1,X^2)),y-((ctl(1)*(y^x))%val(0,1,X^1)));
	*ps = *p1;
}

static void __fastcall PatchWork(RGBA* ps, mif& src, int x, int y)
{
	int n = ctl(0)?ctl(0):1;
	int m = c_dist(x, y);

	RGBA* p1 = getsrc3(src,abs(x^m%n),abs(y^m%n));
	*ps = *p1;
}

//////////////////////////////////////////////////////////////////////
//

// for PrintScreen
static MAGIC parrPrintScreen = {
	_T("スクリーン プリント"),
	PrintScreen,
	4,
	{	{  14,  14, _T("周波数") },
		{  88,  88, _T("ホワイト コントラスト") },
		{ 136, 136, _T("ブラック コントラスト") },
		{  29,  29, _T("アングル") },		}
};

// for AcidRain
static MAGIC parrAcidRain = {
	_T("レイン ドロップ"),
	AcidRain,
	5,
	{	{  27,  27, _T("粒の大きさ") },
		{ 255, 255, _T("内部の明るさ") },
		{   0,   0, _T("外部の明るさ") },
		{  47,  47, _T("屈折") },
		{ 130, 130, _T("ランダム スピード") },		}
};

// for Scroll
static MAGIC parrScroll = {
	_T("スクロール"),
	Scroll,
	2,
	{	{  18,  18, _T("水平方向の移動") },
		{  29,  29, _T("垂直方向の移動") },		}
};

// for ScanlineShifter
static MAGIC parrScanlineShifter = {
	_T("ライン シフト"),
	ScanlineShifter,
	2,
	{	{   9,   9, _T("シフト") },
		{  23,  23, _T("ライン幅") },	}
};

// for PlasticStarfish
static MAGIC parrPlasticStarfish = {
	_T("プラスティック スターフィッシュ"),
	PlasticStarfish,
	7,
	{	{ 102, 102, _T("半径") },
		{  46,  46, _T("ノブ長さ") },
		{  58,  58, _T("ノブ数") },
		{ 224, 224, _T("ノブ幅") },
		{  55,  55, _T("ローテーション") },
		{   0,   0, _T("背景の明るさ") },
		{  32,  32, _T("ゆがみ") },		}
};

// for CrystalBall
static MAGIC parrCrystalBall = {
	_T("クリスタル ボール"),
	CrystalBall,
	7,
	{	{ 144, 144, _T("横方向のハイライト") },
		{ 159, 159, _T("縦方向のハイライト") },
		{ 179, 179, _T("ハイライトのサイズ") },
		{ 192, 192, _T("ゆがみ") },
		{ 128, 128, _T("半径") },
		{ 129, 129, _T("バックの明るさ") },
		{ 255, 255, _T("球の不透明度") },		}
};

// for MarbleMadnessOne
static MAGIC parrMarbleMadnessOne = {
	_T("遮蔽網"),
	MarbleMadnessOne,
	4,
	{	{  26,  26, _T("サイズ") },
		{   0,   0, _T("横位置") },
		{   0,   0, _T("縦位置") },
		{   0,   0, _T("整列") },		}
};

// for LiftCover
static MAGIC parrLiftCover = {
	_T("ページを折る"),
	LiftCover,
	8,
	{	{ 174, 174, _T("折るサイズ") },
		{  86,  86, _T("ライティング") },
		{  23,  23, _T("ドロップシャドウ") },
		{ 124, 124, _T("インターシティ") },
		{ 144, 144, _T("バックサイド") },
		{ 113, 113, _T("影の強さ") },
		{ 122, 122, _T("ハイライトの強さ") },
		{ 164, 164, _T("折られた面の塗り") },			}
};

// for CircuitWeaver
static MAGIC parrCircuitWeaver = {
	_T("巡回織り"),
	CircuitWeaver,
	2,
	{	{  14,  14, _T("解体度") },
		{  36,  36, _T("織り") },	}
};

// for PatchWork
static MAGIC parrPatchWork = {
	_T("パッチワーク"),
	PatchWork,
	1,
	{	{  24,  24, _T("サイズ") },		}
};


//////////////////////////////////////////////////////////////////////
//

HANDLE expr_printscreen_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrPrintScreen, PREVIEW_WHOLE);
}

HANDLE expr_acidrain_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrAcidRain, PREVIEW_WHOLE);
}

HANDLE expr_scrollshift_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrScroll, PREVIEW_WHOLE);
}

HANDLE expr_scanlineshifter_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrScanlineShifter, PREVIEW_WHOLE);
}

HANDLE expr_plasticstarfish_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrPlasticStarfish, PREVIEW_WHOLE);
}

HANDLE expr_crystalball_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrCrystalBall, PREVIEW_WHOLE);
}

HANDLE expr_marblemadnessone_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrMarbleMadnessOne, PREVIEW_WHOLE);
}

HANDLE expr_liftcover_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrLiftCover, PREVIEW_WHOLE);
}

HANDLE expr_circuitweaver_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrCircuitWeaver, PREVIEW_WHOLE);
}

HANDLE expr_patchwork_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrPatchWork, PREVIEW_WHOLE);
}

