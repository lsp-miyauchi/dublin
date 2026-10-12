//////////////////////////////////////////////////////////////////////
// distort_works.cpp

#include "stdafx.h"
#include "ffactory_sup.h"
#include "resource.h"

#define MAXPARAMS		8

//////////////////////////////////////////////////////////////////////
//

static void __fastcall RadialWave(RGBA* ps, mif& src, int x, int y)
{
	double m = dc_dist(x, y);
	double n = dsin(m*ctl(0)*128/M)/(dval(1,256,36)*128/M);
	getsrc_linear(src,x+n,y+n,ps);
}

static void __fastcall WaterSurface(RGBA* ps, mif& src, int x, int y)
{
	double n2 = dc2m(x-dval(0,-X/2,3*X/2),(y-dval(1,-Y/2,3*Y/2))*ctl(2)/64);
	double n1 = n2*ctl(3)*128/M;
	double n3 = dcos(n2*128/M);
	double n0 = dsin(dval(5,n1,n1*n3/1024)+ctl(7)*4)/(dval(4,256,8)*128/M);
	n0 = dval(6,n0,n0*n3/1024);
	getsrc_linear(src,x+n0,y+n0,ps);
}

static void __fastcall Wonderland(RGBA* ps, mif& src, int x, int y)
{
	double d = dc_dir(x, y);
	double m = dc_dist(x, y);
	dput(dsin(ctl(0)*x)-dsin(ctl(1)*y),1);
	dput(dsin(ctl(2)*m)-dsin(ctl(3)*d),2);
	dput((dget(1)+dget(2))/(dval(4,1,255)*2),0);
	getsrc_linear(src,x-dget(0),y-dget(0),ps);
}

static void __fastcall Crumple(RGBA* ps, mif& src, int x, int y)
{
	double d = dc_dir(x,y);
	double n = dsin(d*dval(0, 1, 64))/(dval(1,256,16)*256.0/(double)M);
	getsrc_linear(src,x+n,y+n,ps);
}

static void __fastcall BubbleCrossWave(RGBA* ps, mif& src, int x, int y)
{
/* @@@
	put(isin(x*1024/val(0,50,3))/(512/val(2,3,16)),0);
	put(isin(y*1024/val(1,50,3))/(512/val(3,3,16)),1);
	put(val(4,0,dif(get(0),get(1))*2),4);

	RGBA* p1 = getsrc(src,x+get(0)+get(4),y+get(1)+get(4));
	RGBA* p2 = getsrc(src,x-get(0)-get(4),y-get(1)-get(4));
	RGBA* p3 = getsrc(src,x+get(1)+get(4),y+get(0)+get(4));
	RGBA* p4 = getsrc(src,x-get(1)-get(4),y-get(0)-get(4));

	ps->red = ((int)p1->red+(int)p2->red+(int)p3->red+(int)p4->red)/4;
	ps->green = ((int)p1->green+(int)p2->green+(int)p3->green+(int)p4->green)/4;
	ps->blue = ((int)p1->blue+(int)p2->blue+(int)p3->blue+(int)p4->blue)/4;
@@@ */
	double n0 = dsin(x*1024/dval(0,50,3))/(512/dval(2,3,16));
	double n1 = dsin(y*1024/dval(1,50,3))/(512/dval(3,3,16));
	double n4 = dval(4,0,fabs(n0-n1)*2);

	RGBA p1, p2, p3, p4;
	getsrc_linear(src,x+n0+n4,y+n1+n4,&p1);
	getsrc_linear(src,x-n0-n4,y-n1-n4,&p2);
	getsrc_linear(src,x+n1+n4,y+n0+n4,&p3);
	getsrc_linear(src,x-n1-n4,y-n0-n4,&p4);

	ps->red = ((int)p1.red+(int)p2.red+(int)p3.red+(int)p4.red)/4;
	ps->green = ((int)p1.green+(int)p2.green+(int)p3.green+(int)p4.green)/4;
	ps->blue = ((int)p1.blue+(int)p2.blue+(int)p3.blue+(int)p4.blue)/4;
}

static void __fastcall FluteWave(RGBA* ps, mif& src, int x, int y)
{
/* @@@
	put(isin(y*1024/val(0,50,3))/(512/val(2,3,16)),0);
	put(isin(x*1024/val(1,50,3))/(512/val(3,3,16)),1);
	put(ctl(4)>128?get(0):get(1),2);
	put(ctl(4)>128?get(1):get(0),3);
	RGBA* p1 = getsrc(src,x+get(0),y+get(1));
	RGBA* p2 = getsrc(src,x-get(3),y-get(2));
	ps->red = mix(p1->red,p2->red,128,255);
	ps->green = mix(p1->green,p2->green,128,255);
	ps->blue = mix(p1->blue,p2->blue,128,255);
@@@ */
	double n0 = dsin(y*1024/dval(0,50,3))/(512/dval(2,3,16));
	double n1 = dsin(x*1024/dval(1,50,3))/(512/dval(3,3,16));
	double n2 = ctl(4)>128?n0:n1;
	double n3 = ctl(4)>128?n1:n0;

	RGBA p1, p2;
	getsrc_linear(src,x+n0,y+n1,&p1);
	getsrc_linear(src,x-n3,y-n2,&p2);
	ps->red = mix(p1.red,p2.red,128,255);
	ps->green = mix(p1.green,p2.green,128,255);
	ps->blue = mix(p1.blue,p2.blue,128,255);
}

static void __fastcall SausageWave(RGBA* ps, mif& src, int x, int y)
{
/* @
	put(isin(scl(c2m(x-val(1,0,X),y-val(2,0,Y)),0,M,-512,512)*50/val(0,1,255)),0);
	put(isin(scl(c2m(x-val(4,0,X),y-val(5,0,Y)),0,M,-512,512)*50/val(3,1,255)),1);
	put(x+r2x(abs((get(0)+get(1))),val(6,0,M/4)),2);
	put(y+r2y(abs((get(0)+get(1))),val(6,0,M/4)),3);
	getsrc_linear(src,get(2),get(3),ps);
@ */
	dput(dsin(dscl(dc2m(x-dval(1,0,X),y-dval(2,0,Y)),0,M,-512,512)*50/dval(0,1,255)),0);
	dput(dsin(dscl(dc2m(x-dval(4,0,X),y-dval(5,0,Y)),0,M,-512,512)*50/dval(3,1,255)),1);
	dput(x+dr2x(fabs((dget(0)+dget(1))),dval(6,0,M/4)),2);
	dput(y+dr2y(fabs((dget(0)+dget(1))),dval(6,0,M/4)),3);
	getsrc_linear(src,dget(2),dget(3),ps);
}

static void __fastcall Smallpox(RGBA* ps, mif& src, int x, int y)
{
	dput(sqrt((double)M)/2,1);
	dput(dval(0,1,dget(1)),2);
	dput(dval(3,-512,512),3);
	dput(dval(2,0,1024),4);
	dput(dval(1,0,dget(1)*2),5);

	dput((dsin(dget(2)*(dscl(x,0,M,-512,512)+dget(3)))+dsin(dget(2)*(dscl(y,0,M,-512,512)+dget(3))))/2,0);
	getsrc_linear(src,x+dr2x(dget(0)+dget(4),dget(5)),y+dr2y(dget(0)+dget(4),dget(5)),ps);
}

static void __fastcall CyberMosh(RGBA* ps, mif& src, int x, int y)
{
//	int n = val(0,0,M);
	int n = val(0,1,M);
	int i = yuvY(ps);
	put(scl(i,0,255,val(1,-Y,Y),-val(1,-Y,Y)),1);

	int c;
	c = ps->red;
	c = (y-get(1))%n<val(3,1,n)||(x-get(1))%n<val(3,1,n)?255:mix(c,0,ctl(2),255);
	ps->red = CLAMP(c);

	c = ps->green;
	c = (y-get(1))%n<val(3,1,n)||(x-get(1))%n<val(3,1,n)?255:mix(c,0,ctl(2),255);
	ps->green = CLAMP(c);

	c = ps->blue;
	c = (y-get(1))%n<val(3,1,n)||(x-get(1))%n<val(3,1,n)?255:mix(c,0,ctl(2),255);
	ps->blue = CLAMP(c);
}

static void __fastcall BubbleDistort(RGBA* ps, mif& src, int x, int y)
{
/* @@@
	int n = val(0,1,X);
	int e = val(1,0,Y);
	put(x%n,0);
	put(y%n,1);
	put(scl(c2m(get(0)-n/2,get(1)-n/2),0,n,255,0),2);
	RGBA* p1 = getsrc3(src,x-scl(get(2),0,255,0,e),y-scl(get(2),0,255,0,e));
	*ps = *p1;
@@@ */
	double n = dval(0,1,X);
	double e = dval(1,0,Y);
	double n0 = fmod(x,n);
	double n1 = fmod(y,n);
	double n2 = dscl(dc2m(n0-n/2,n1-n/2),0,n,255,0);
	getsrc_linear(src,x-dscl(n2,0,255,0,e),y-dscl(n2,0,255,0,e),ps);
}


static void __fastcall WarpHorz(RGBA* ps, mif& src, int x, int y)
{
/* @@@
	int D = 1024;
	put(isin(val(1,1,10)*scl(y,0,Y,-512,512)),0);
	put(icos(val(2,1,10)*scl(y,0,Y,-512,512)),1);
	put(scl(get(0)+get(1), -D, D, -X/2,X/2),2);
	put(val(0,x,x+get(2)),3);
	RGBA* p1 = getsrc3(src,get(3),y);
	*ps = *p1;
@@@ */
	double D = 1024.0;
	double n0 = dsin(dval(1,1,10)*dscl(y,0,Y,-512,512));
	double n1 = dcos(dval(2,1,10)*dscl(y,0,Y,-512,512));
	double n2 = dscl(n0+n1, -D, D, -X/2,X/2);
	double n3 = dval(0,x,x+n2);
	getsrc_linear(src,n3,y,ps);
}

static void __fastcall WarpVert(RGBA* ps, mif& src, int x, int y)
{
/* @@@
	int D = 1024;
	put(isin(val(1,1,10)*scl(x,0,X,-512,512)),0);
	put(icos(val(2,1,10)*scl(x,0,X,-512,512)),1);
	put(scl(get(0)+get(1), -D, D, -Y/2,Y/2),2);
	put(val(0,y,y+get(2)),3);
	RGBA* p1 = getsrc3(src,x,get(3));
	*ps = *p1;
@@@ */
	double D = 1024.0;
	double n0 = dsin(dval(1,1,10)*dscl(x,0,X,-512,512));
	double n1 = dcos(dval(2,1,10)*dscl(x,0,X,-512,512));
	double n2 = dscl(n0+n1, -D, D, -Y/2,Y/2);
	double n3 = dval(0,y,y+n2);
	getsrc_linear(src,x,n3,ps);
}

static void __fastcall Vtr(RGBA* ps, mif& src, int x, int y)
{
	int r = ps->red;
	int g = ps->green;
	int b = ps->blue;

	RGBA* p1 = getsrc3(src,x-ctl(3),y);
	int c = ps->red;
	c = ((c2m(0,(y%(ctl(0)/10))) >ctl(1)? c:c+ctl(2)*2-254)+(int)p1->red)/2;
	ps->red = CLAMP(c);

	c = ps->green;
	c = ((c2m(0,(y%(ctl(0)/10))) >ctl(1)? c:c+ctl(2)*2-254)+(int)p1->green)/2;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = ((c2m(0,(y%(ctl(0)/10))) >ctl(1)? c:c+ctl(2)*2-254)+(int)p1->blue)/2;
	ps->blue = CLAMP(c);
}

static void __fastcall FlutteringFlag(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,X*2,X/5),4);
	put(val(2,Y*2,Y/5),1);
	put(val(1,0,Y/8),5);
	put(val(3,0,X/8),6);
	put(val(4,512,-512),0);
	put(get(0)+x*1024/get(4),2);
	put(get(0)+y*1024/get(1),3);
	put(val(5,64,0),55);
	put(y-scl(isin(get(2)),-512,512,-get(5),get(5)),7);
	put(x-scl(isin(get(1)),-512,512,-get(6),get(6)),8);
	put(scl(isin(get(2)),-512,512,get(55),128-get(55)),17);
	put(scl(isin(get(1)),-512,512,get(55),128-get(55)),18);

	getsrc_linear(src,get(8)%X+X*(get(8)<0),get(7)%Y+Y*(get(7)<0),ps);

	int c = ps->red;
	c = c+get(17)-get(18);
	ps->red = CLAMP(c);

	c = ps->green;
	c = c+get(17)-get(18);
	ps->green = CLAMP(c);

	c = ps->blue;
	c = c+get(17)-get(18);
	ps->blue = CLAMP(c);
}

static void __fastcall CDMaker(RGBA* ps, mif& src, int x, int y)
{
	int d = c_dir(x, y);
//	int m = c_dist(x, y);

	put(c2m(x-X/2,y-Y/2),0);
	put((X/2)*ctl(0)/255,1);

	int r = ps->red;
	int g = ps->green;
	int b = ps->blue;

	int c;
	c = (get(0)<get(1)*59/60)&&(get(0)>get(1)*20/60)?mix(max(255-((max(0,(isin(d+(255-ctl(3))*2)))|max(0,(isin(512+d+(255-ctl(3))*2))))*(ctl(2)*2)/255),127),mix(255-max(255-((max(0,(isin(d+(255-ctl(6))*2)))|max(0,(isin(512+d+(255-ctl(6))*2))))*(ctl(2)*2)/255),127),200,ctl(7),255),1,2):(get(0)<get(1)&&get(0)>get(1)*8/60)?mix(r,127,ctl(1),255):r;
	ps->red = CLAMP(c);

	c = (get(0)<get(1)*59/60)&&(get(0)>get(1)*20/60)?mix(max(255-((max(0,(isin(d+(255-ctl(4))*2)))|max(0,(isin(512+d+(255-ctl(4))*2))))*(ctl(2)*2)/255),127),mix(255-max(255-((max(0,(isin(d+(255-ctl(6))*2)))|max(0,(isin(512+d+(255-ctl(6))*2))))*(ctl(2)*2)/255),127),200,ctl(7),255),1,2):(get(0)<get(1)&&get(0)>get(1)*8/60)?mix(g,127,ctl(1),255):g;
	ps->green = CLAMP(c);

	c = (get(0)<get(1)*59/60)&&(get(0)>get(1)*20/60)?mix(max(255-((max(0,(isin(d+(255-ctl(5))*2)))|max(0,(isin(512+d+(255-ctl(5))*2))))*(ctl(2)*2)/255),127),mix(255-max(255-((max(0,(isin(d+(255-ctl(6))*2)))|max(0,(isin(512+d+(255-ctl(6))*2))))*(ctl(2)*2)/255),127),200,ctl(7),255),1,2):(get(0)<get(1)&&get(0)>get(1)*8/60)?mix(b,127,ctl(1),255):b;
	ps->blue = CLAMP(c);
}

/////////////////////////////////////////////////////////////////////
//

// for RadialWave
static MAGIC parrRadialWave = {
	_T("波紋 シンプル"),
	RadialWave,
	2,
	{	{ 122, 122, _T("周波数") },
		{ 157, 157, _T("振幅") },	}
};

// for WaterSurface
static MAGIC parrWaterSurface = {
	_T("波紋 遠近"),
	WaterSurface,
	8,
	{	{ 108, 108, _T("水平方向の中心") },
		{ 108, 108, _T("垂直方向の中心") },
		{ 128, 128, _T("水平方向の伸縮") },
		{ 128, 128, _T("波の幅") },
		{  93,  93, _T("波の高さ") },
		{ 143, 143, _T("幅の減少度") },
		{  68,  68, _T("高さの減少度") },
		{ 128, 128, _T("フェーズ") },		}
};

// for Wonderland
static MAGIC parrWonderland = {
	_T("波紋 合成 1"),
	Wonderland,
	5,
	{	{ 135, 135, _T("横の波") },
		{  61,  61, _T("縦の波") },
		{ 150, 150, _T("円心の波") },
		{  13,  13, _T("しわの波") },
		{  81,  81, _T("振幅") },		}
};

// for SausageWave
static MAGIC parrSausageWave = {
	_T("波紋 合成 2"),
	SausageWave,
	7,
	{	{  38,  38, _T("周波数 1") },
		{ 178, 178, _T("水平中心位置 1") },
		{ 200, 200, _T("垂直中心位置 1") },
		{  29,  29, _T("周波数 2") },
		{  68,  68, _T("水平中心位置 2") },
		{ 123, 123, _T("垂直中心位置 2") },
		{  34,  34, _T("ゆがみ") },			}
};

// for FlutteringFlag
static MAGIC parrFlutteringFlag = {
	_T("はためき"),
	FlutteringFlag,
	6,
	{	{ 162, 162, _T("水平周波数") },
		{ 255, 255, _T("水平スケール") },
		{ 255, 255, _T("垂直周波数") },
		{ 229, 229, _T("垂直スケール") },
		{  12,  12, _T("シェーディング位置") },
		{ 172, 172, _T("シェーディング量") },		}
};

// for BubbleCrossWave
static MAGIC parrBubbleCrossWave = {
	_T("波 バブルクロス"),
	BubbleCrossWave,
	5,
	{	{  72,  72, _T("X軸 周波数") },
		{  16,  16, _T("X軸 振幅") },
		{  72,  72, _T("Y軸 周波数") },
		{  72,  72, _T("Y軸 振幅") },
		{   0,   0, _T("変形度") },		}
};

// for FluteWave
static MAGIC parrFluteWave = {
	_T("波 フルート"),
	FluteWave,
	5,
	{	{  72,  72, _T("X軸 周波数") },
		{  26,  26, _T("X軸 振幅") },
		{  68,  68, _T("Y軸 周波数") },
		{  68,  68, _T("Y軸 振幅") },
		{   0,   0, _T("モード") },		}
};

// for Smallpox
static MAGIC parrSmallpox = {
	_T("うねり バブル 1"),
	Smallpox,
	3,
	{	{ 152, 152, _T("周波数") },
		{ 116, 116, _T("強さ") },
		{ 172, 172, _T("方向") },			}
};

// for BubbleDistort
static MAGIC parrBubbleDistort = {
	_T("うねり バブル 2"),
	BubbleDistort,
	2,
	{	{  33,  33, _T("ゆがみ") },
		{  34,  34, _T("オフセット") },		}
};

// for Crumple
static MAGIC parrCrumple = {
	_T("うねり 放射"),
	Crumple,
	2,
	{	{ 102, 102, _T("周波数") },
		{ 189, 189, _T("振幅") },	}
};

// for WarpHorz
static MAGIC parrWarpHorz = {
	_T("うねり 水平"),
	WarpHorz,
	3,
	{	{  21,  21, _T("振幅") },
		{  65,  65, _T("周波数 1") },
		{ 125, 125, _T("周波数 2") },	}
};

// for WarpVert
static MAGIC parrWarpVert = {
	_T("うねり 垂直"),
	WarpVert,
	3,
	{	{ 103, 103, _T("振幅") },
		{  48,  48, _T("周波数 1") },
		{  90,  90, _T("周波数 2") },	}
};

// for Vtr
static MAGIC parrVtr = {
	_T("TV ゴースト"),
	Vtr,
	4,
	{	{  26,  26, _T("走査距離") },
		{   0,   0, _T("走査線幅") },
		{ 169, 169, _T("コントラスト") },
		{   2,   2, _T("ゴースト") },		}
};



/////////////////////////////////////////////////////////////////////
//

HANDLE wave_bubblecross_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrBubbleCrossWave, PREVIEW_PART);
}

HANDLE wave_flute_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrFluteWave, PREVIEW_PART);
}

HANDLE wave_smallpox_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSmallpox, PREVIEW_WHOLE);
}

HANDLE wave_bubbledistort_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrBubbleDistort, PREVIEW_WHOLE);
}

HANDLE wave_crumple_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrCrumple, PREVIEW_WHOLE);
}

HANDLE wave_warphorz_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrWarpHorz, PREVIEW_WHOLE);
}

HANDLE wave_warpvert_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrWarpVert, PREVIEW_WHOLE);
}

HANDLE wave_radialwave_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrRadialWave, PREVIEW_WHOLE);
}

HANDLE wave_watersurface_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrWaterSurface, PREVIEW_WHOLE);
}

HANDLE wave_wonderland_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrWonderland, PREVIEW_WHOLE);
}

HANDLE wave_sausagewave_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSausageWave, PREVIEW_WHOLE);
}

HANDLE wave_flutteringflag_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrFlutteringFlag, PREVIEW_WHOLE);
}

HANDLE tv_ghost_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrVtr, PREVIEW_WHOLE);
}

