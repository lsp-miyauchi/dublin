//////////////////////////////////////////////////////////////////////
// frame_works.cpp

#include "stdafx.h"
#include "resource.h"
#include "ffactory_sup.h"

extern PROGRESSCALLBACK pfnProgress;

#define MAXPARAMS		8

//////////////////////////////////////////////////////////////////////
//

static void __fastcall GFrame3D_Round(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,0,X/2),0);
	put(val(0,0,Y/2),1);
	put(val(1,30,5),2);

	put(isin(scl(x,0,get(0), 200, 712))/get(2),11);
	put(isin(scl(X-x,0,get(0), -200, 312))/get(2),13);
	put(isin(scl(y,0,get(1), 200, 712))/get(2),15);
	put(isin(scl(Y-y,0,get(1), -200, 312))/get(2),17);

	int c;
	c = ps->red;
	c = (x<get(0)&&x*Y<=X*y&&x*Y<=X*(Y-y))?c+get(11):(y<get(1)&&x*Y>=X*y&&Y*(X-x)>=X*y)?c+get(15):(x>val(0,X,X/2)&&(X-x)*Y<=X*y&&x*(Y-y)>=y*(X-x))?c+get(13):(y>val(0,Y,Y/2)&&x*Y>=X*(Y-y)&&x*(Y-y)<=y*(X-x))?c+get(17):c;
	ps->red = CLAMP(c);

	c = ps->green;
	c = (x<get(0)&&x*Y<=X*y&&x*Y<=X*(Y-y))?c+get(11):(y<get(1)&&x*Y>=X*y&&Y*(X-x)>=X*y)?c+get(15):(x>val(0,X,X/2)&&(X-x)*Y<=X*y&&x*(Y-y)>=y*(X-x))?c+get(13):(y>val(0,Y,Y/2)&&x*Y>=X*(Y-y)&&x*(Y-y)<=y*(X-x))?c+get(17):c;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = (x<get(0)&&x*Y<=X*y&&x*Y<=X*(Y-y))?c+get(11):(y<get(1)&&x*Y>=X*y&&Y*(X-x)>=X*y)?c+get(15):(x>val(0,X,X/2)&&(X-x)*Y<=X*y&&x*(Y-y)>=y*(X-x))?c+get(13):(y>val(0,Y,Y/2)&&x*Y>=X*(Y-y)&&x*(Y-y)<=y*(X-x))?c+get(17):c;
	ps->blue = CLAMP(c);
}

static void __fastcall GFrame3D_Sunken(RGBA* ps, mif& src, int x, int y)
{
	put(x<val(0,0,X/2)&&x*Y<=X*y&&x*Y<=X*(Y-y), 0);
	put(y<val(0,0,Y/2)&&x*Y>=X*y&&Y*(X-x)>=X*y, 1);
	put(x>val(0,X,X/2)&&(X-x)*Y<=X*y&&x*(Y-y)>=y*(X-x), 2);
	put(y>val(0,Y,Y/2)&&x*Y>=X*(Y-y)&&x*(Y-y)<=y*(X-x), 3);
	put(X, 4);
	put(scl(x, 0, X, 0, ctl(1)*10), 5);
	put(scl(y, 0, Y, 0, ctl(1)*10), 6);
	put(scl(x, 0, X, ctl(1)*10, 0), 7);
	put(scl(y, 0, Y, ctl(1)*10, 0), 8);

	int c;
	c = ps->red;
	c = get(0) ? c-get(5) : get(1) ? c-get(6) : get(2) ? c+get(7) : get(3) ? c+get(8) : c;
	ps->red = CLAMP(c);

	c = ps->green;
	c = get(0) ? c-get(5) : get(1) ? c-get(6) : get(2) ? c+get(7) : get(3) ? c+get(8) : c;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = get(0) ? c-get(5) : get(1) ? c-get(6) : get(2) ? c+get(7) : get(3) ? c+get(8) : c;
	ps->blue = CLAMP(c);
}

static void __fastcall GFrame3D_Normal(RGBA* ps, mif& src, int x, int y)
{
	int C = 255;

	put(min(X/2,Y/2),1);
	put(val(0,0,get(1)),2);
	put(sbd(x,y),0);
	put(get(0) <= get(2) ? scl(get(0), 0, get(2), 0, C) : 0,3);
//@	put(val(1,0,get(3)),4);
	put(ctl(1)<128?-scl(127-ctl(1),0,127,0,get(3)):scl(ctl(1)-128,0,127,0,get(3)),4);

	int c;
	c = ps->red;
	c = c+get(4);
	ps->red = CLAMP(c);

	c = ps->green;
	c = c+get(4);
	ps->green = CLAMP(c);

	c = ps->blue;
	c = c+get(4);
	ps->blue = CLAMP(c);
}

static void __fastcall GFrame3D_RoundEx(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,0,min(X,Y)/2),0);
	put(val(1,-255,255),1);
	put(val(2,get(0)*4,get(0)/10),2);
	put(isin(scl(x,get(2),0,-767,255)),13);
	put(isin(scl(y,get(2),0,-767,255)),14);
	put(isin(scl(x,X-get(2),X,-255,767)),15);
	put(isin(scl(y,Y-get(2),Y,-255,767)),16);

	int c;
	c = ps->red;
	c = (x<get(0)&&x<=y&&x<=(Y-y)) ? c+get(13)*get(1)/1024:
		(y<get(0)&&x>=y&&(X-x)>=y) ? c+get(14)*get(1)/1024:
		(x>X-get(0)&&(X-x)<=y&&(Y-y)>=(X-x)) ? c+get(15)*get(1)/1024:
		(y>Y-get(0)&&x>=(Y-y)&&(Y-y)<=(X-x)) ? c+get(16)*get(1)/1024:c;
	ps->red = CLAMP(c);

	c = ps->green;
	c = (x<get(0)&&x<=y&&x<=(Y-y)) ? c+get(13)*get(1)/1024:
		(y<get(0)&&x>=y&&(X-x)>=y) ? c+get(14)*get(1)/1024:
		(x>X-get(0)&&(X-x)<=y&&(Y-y)>=(X-x)) ? c+get(15)*get(1)/1024:
		(y>Y-get(0)&&x>=(Y-y)&&(Y-y)<=(X-x)) ? c+get(16)*get(1)/1024:c;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = (x<get(0)&&x<=y&&x<=(Y-y)) ? c+get(13)*get(1)/1024:
		(y<get(0)&&x>=y&&(X-x)>=y) ? c+get(14)*get(1)/1024:
		(x>X-get(0)&&(X-x)<=y&&(Y-y)>=(X-x)) ? c+get(15)*get(1)/1024:
		(y>Y-get(0)&&x>=(Y-y)&&(Y-y)<=(X-x)) ? c+get(16)*get(1)/1024:c;
	ps->blue = CLAMP(c);
}

static void __fastcall MirrorBevel(RGBA* ps, mif& src, int x, int y)
{
	(x<val(0,0,X/2)&&x*Y<=X*y&&x*Y<=X*(Y-y)?ctl(1)<128?getsrc_linear(src,2*x,y,ps) : getsrc_linear(src,X-2*x,y,ps) :
	(y<val(0,0,Y/2)&&x*Y>=X*y&&Y*(X-x)>=X*y?ctl(1)<128?getsrc_linear(src,x,2*y,ps) : getsrc_linear(src,x,Y-2*y,ps) :
	(x>val(0,X,X/2)&&(X-x)*Y<=X*y&&x*(Y-y)>=y*(X-x)?getsrc_linear(src,2*x-X,y,ps) :
	(y>val(0,Y,Y/2)&&x*Y>=X*(Y-y)&&x*(Y-y)<=y*(X-x)?getsrc_linear(src,x,2*y-Y,ps) : ps))));
}

static void __fastcall AfhBevel_I(RGBA* ps, mif& src, int x, int y)
{
	put(val(6,0,min(X,Y)/2),0);

	int c;
	c = ps->red;
	c = ((x<get(0))&&(y>=x)&&(y<Y-x))?scl(c,0,255,ctl(0),ctl(1)*2)+ctl(4)-128:((y<get(0))&&(x>y)&&(x<X-y))?scl(c,0,255,ctl(0),ctl(1)*2)-ctl(4)+128:((X-x<=get(0))&&(Y-y>=X-x)&&(y>x-X))?scl(c,0,255,ctl(2),ctl(3)*2)+ctl(5)-128:((Y-y<=get(0))&&(X-x>Y-y)&&(x>y-Y))?scl(c,0,255,ctl(2),ctl(3)*2)-ctl(5)+128:c;
	ps->red = CLAMP(c);

	c = ps->green;
	c = ((x<get(0))&&(y>=x)&&(y<Y-x))?scl(c,0,255,ctl(0),ctl(1)*2)+ctl(4)-128:((y<get(0))&&(x>y)&&(x<X-y))?scl(c,0,255,ctl(0),ctl(1)*2)-ctl(4)+128:((X-x<=get(0))&&(Y-y>=X-x)&&(y>x-X))?scl(c,0,255,ctl(2),ctl(3)*2)+ctl(5)-128:((Y-y<=get(0))&&(X-x>Y-y)&&(x>y-Y))?scl(c,0,255,ctl(2),ctl(3)*2)-ctl(5)+128:c;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = ((x<get(0))&&(y>=x)&&(y<Y-x))?scl(c,0,255,ctl(0),ctl(1)*2)+ctl(4)-128:((y<get(0))&&(x>y)&&(x<X-y))?scl(c,0,255,ctl(0),ctl(1)*2)-ctl(4)+128:((X-x<=get(0))&&(Y-y>=X-x)&&(y>x-X))?scl(c,0,255,ctl(2),ctl(3)*2)+ctl(5)-128:((Y-y<=get(0))&&(X-x>Y-y)&&(x>y-Y))?scl(c,0,255,ctl(2),ctl(3)*2)-ctl(5)+128:c;
	ps->blue = CLAMP(c);
}

static void __fastcall Bevel_III(RGBA* ps, mif& src, int x, int y)
{
	int d = c_dir(x, y);
	int m = c_dist(x, y);
	int D = 1024;

	put(min(X/2,Y/2),3);
	put(val(1,0,M),4);
	put((ctl(1)==0)*get(3)+(ctl(1)>0)*get(4),1);
	put(val(2,0,get(1)),2);
	put(scl(isin(512*((d+val(0,0,D)+D)%D)/D),0,512,-255,255),0);
	put(scl(isin(512*((ctl(3)*d+val(0,0,D)+ctl(3)*D)%D)/D),0,512,-255,255),5);
	put(get(0)*get(5)/255,7);
	put(mix(get(7),get(0),ctl(4),255),6);

	int c;
	c = ps->red;
	c = m<get(1)?(m>get(2))?get(6)<0?(get(6)+255)*c/255:255-(255-c)*(255-get(6))/255:c:0;
	ps->red = CLAMP(c);

	c = ps->green;
	c = m<get(1)?(m>get(2))?get(6)<0?(get(6)+255)*c/255:255-(255-c)*(255-get(6))/255:c:0;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = m<get(1)?(m>get(2))?get(6)<0?(get(6)+255)*c/255:255-(255-c)*(255-get(6))/255:c:0;
	ps->blue = CLAMP(c);
}

static void __fastcall RoundButton(RGBA* ps, mif& src, int x, int y)
{
	int d = c_dir(x, y);
	int m = c_dist(x, y);
	int D = 1024;

	put(min(X/2,Y/2)-1,10);
	put(abs(scl((512-((d+val(0,D, 0)+512)%D)), -512, 512, -ctl(2)*3, ctl(2)*3))-128,0);
	put(m > get(10),2);
	put(m < get(10)-val(1, 0, get(10)),3);
	put(get(2) || get(3),1);

	int c;
	c = ps->red;
	c = get(1) ? get(2) ? get(3) ? c : 0 : c : (c+get(0));
	ps->red = CLAMP(c);

	c = ps->green;
	c = get(1) ? get(2) ? get(3) ? c : 0 : c : (c+get(0));
	ps->green = CLAMP(c);

	c = ps->blue;
	c = get(1) ? get(2) ? get(3) ? c : 0 : c : (c+get(0));
	ps->blue = CLAMP(c);
}

static void __fastcall BullsEye(RGBA* ps, mif& src, int x, int y)
{
	int d = c_dir(x, y);
	int m = c_dist(x, y);
	int C = 255;

	put(abs(isin(d*val(0,1,50))/2),0);

	int c;
	c = ps->red;
	c = m < val(1,0,M) ? m > val(2,0,M) ? get(0)*ctl(3)/256:c:0;
	ps->red = CLAMP(c);

	c = ps->green;
	c = m < val(1,0,M) ? m > val(2,0,M) ? get(0)*ctl(4)/256:c:0;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = m < val(1,0,M) ? m > val(2,0,M) ? get(0)*ctl(5)/256:c:0;
	ps->blue = CLAMP(c);
}

static void __fastcall FlowerFrame(RGBA* ps, mif& src, int x, int y)
{
	int m = c_dist(x, y);
	int d = c_dir(x, y);
	int C = 255;

	put(d,1);
	put(d&isin(C)%C,2);
	put(m+mix(-val(2,0,M),0,isin(get(1)*val(1,0,100))+isin(get(2)),1024),0);
	put(scl(get(0),0,val(0,2,M),C,0),4);

	int c;
	c = ps->red;
	c = get(0)>val(0,2,M)?get(2)^(get(4)+hue(ctl(3),0)+val(4,-C,C)):c;
	ps->red = CLAMP(c);

	c = ps->green;
	c = get(0)>val(0,2,M)?get(2)^(get(4)+hue(ctl(3),1)+val(4,-C,C)):c;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = get(0)>val(0,2,M)?get(2)^(get(4)+hue(ctl(3),2)+val(4,-C,C)):c;
	ps->blue = CLAMP(c);
}

static void __fastcall MirroredFrame(RGBA* ps, mif& src, int x, int y)
{
	put(val(0, 0, min(X,Y)/2), 0);
	if (sbd(x,y) < get(0))
	{
		RGBA* p1 = getsrc(src, X-x, Y-y);
		*ps = *p1;
	}
}

static void __fastcall SimpleColorFrame(RGBA* ps, mif& src, int x, int y)
{
	put(val(2,-200,200), 4);
	put(val(0, 0, min(X,Y)/2), 5);
	if (sbd(x,y) < get(5))
	{
		int c;
		c = hue(ctl(1),0) + get(4);
		ps->red = CLAMP(c);

		c = hue(ctl(1),1) + get(4);
		ps->green = CLAMP(c);

		c = hue(ctl(1),2) + get(4);
		ps->blue = CLAMP(c);
	}
}

static void __fastcall FadeFrame(RGBA* ps, mif& src, int x, int y)
{
	int c;
	put(ctl(1),0);
	c = ps->red;
	c = (x<val(0,0,X/2)&&x*Y<=X*y&&x*Y<=X*(Y-y) ? mix(c,get(0),x,val(0,0,X/2)):
	(y<val(0,0,Y/2)&&x*Y>=X*y&&Y*(X-x)>=X*y ? mix(c,get(0),y,val(0,0,Y/2)) :
	(x>val(0,X,X/2)&&(X-x)*Y<=X*y&&x*(Y-y)>=y*(X-x) ?mix(c,get(0),X-x,val(0,0,X/2)) :
	(y>val(0,Y,Y/2)&&x*Y>=X*(Y-y)&&x*(Y-y)<=y*(X-x) ?mix(c,get(0),Y-y,val(0,0,Y/2)) : c))));
	ps->red = CLAMP(c);

	c = ps->green;
	put(ctl(2),0);
	c = (x<val(0,0,X/2)&&x*Y<=X*y&&x*Y<=X*(Y-y) ? mix(c,get(0),x,val(0,0,X/2)):
	(y<val(0,0,Y/2)&&x*Y>=X*y&&Y*(X-x)>=X*y ? mix(c,get(0),y,val(0,0,Y/2)) :
	(x>val(0,X,X/2)&&(X-x)*Y<=X*y&&x*(Y-y)>=y*(X-x) ?mix(c,get(0),X-x,val(0,0,X/2)) :
	(y>val(0,Y,Y/2)&&x*Y>=X*(Y-y)&&x*(Y-y)<=y*(X-x) ?mix(c,get(0),Y-y,val(0,0,Y/2)) : c))));
	ps->green = CLAMP(c);

	c = ps->blue;
	put(ctl(3),0);
	c = (x<val(0,0,X/2)&&x*Y<=X*y&&x*Y<=X*(Y-y) ? mix(c,get(0),x,val(0,0,X/2)):
	(y<val(0,0,Y/2)&&x*Y>=X*y&&Y*(X-x)>=X*y ? mix(c,get(0),y,val(0,0,Y/2)) :
	(x>val(0,X,X/2)&&(X-x)*Y<=X*y&&x*(Y-y)>=y*(X-x) ?mix(c,get(0),X-x,val(0,0,X/2)) :
	(y>val(0,Y,Y/2)&&x*Y>=X*(Y-y)&&x*(Y-y)<=y*(X-x) ?mix(c,get(0),Y-y,val(0,0,Y/2)) : c))));
	ps->blue = CLAMP(c);
}

static void __fastcall ShadowCaster(RGBA* ps, mif& src, int x, int y)
{
//	put(val(0,0,X),0);
	put(ctl(0),0);

	RGBA* p1 = getsrc(src,x-ctl(0),y-ctl(0));
	int c;
	c = ps->red;
	c = (x>get(0)-1&&y>get(0)-1)?c:
			(x<get(0)&&y>get(0)-1)?scl(x,0,get(0)-1,ctl(1),255)*c/255:
			(x>get(0)-1&&y<get(0))?scl(y,0,get(0)-1,ctl(1),255)*c/255:
			min(scl(x,0,get(0)-1,ctl(1),255),scl(y,0,get(0)-1,ctl(1),255))*c/255;
	ps->red = CLAMP(c);

	c = ps->green;
	c = (x>get(0)-1&&y>get(0)-1)?c:
			(x<get(0)&&y>get(0)-1)?scl(x,0,get(0)-1,ctl(2),255)*c/255:
			(x>get(0)-1&&y<get(0))?scl(y,0,get(0)-1,ctl(2),255)*c/255:
			min(scl(x,0,get(0)-1,ctl(2),255),scl(y,0,get(0)-1,ctl(2),255))*c/255;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = (x>get(0)-1&&y>get(0)-1)?c:
			(x<get(0)&&y>get(0)-1)?scl(x,0,get(0)-1,ctl(3),255)*c/255:
			(x>get(0)-1&&y<get(0))?scl(y,0,get(0)-1,ctl(3),255)*c/255:
			min(scl(x,0,get(0)-1,ctl(3),255),scl(y,0,get(0)-1,ctl(3),255))*c/255;
	ps->blue = CLAMP(c);
}

static void __fastcall MetalFrame(RGBA* ps, mif& src, int x, int y)
{
	put(val(0,1,min(X/4,Y/4)),0);
	put(min(x,min(y,min(X-x,Y-y))),1);
	put(64*val(2,0,32),8);
	put(scl(get(1),0,get(0),-64*val(1,1,8),64*val(1,1,8))+val(3,-256,256),2);
	put(r2x(get(2)+scl(y,0,Y,x<X/2?0:get(8),x<X/2?get(8):0),get(0)),3);
	put(r2y(get(2)+scl(x,0,X,y<Y/2?0:get(8),y<Y/2?get(8):0),get(0)),4);
	put(min(x+get(3),min(y+get(4),min(X-x+get(3),Y-y+get(4)))),5);
	put(val(5,-200,200), 9);

	int c;
	c = ps->red;
	c = get(1)<2*get(0)?scl(get(5),-2*get(0),3*get(0),0,hue(ctl(4),0)+get(9)):min(x,y)-2*get(0)<get(0)?c*scl(min(x,y)-2*get(0),0,get(0),ctl(6),255)/255:c;
	ps->red = CLAMP(c);

	c = ps->green;
	c = get(1)<2*get(0)?scl(get(5),-2*get(0),3*get(0),0,hue(ctl(4),1)+get(9)):min(x,y)-2*get(0)<get(0)?c*scl(min(x,y)-2*get(0),0,get(0),ctl(6),255)/255:c;
	ps->green = CLAMP(c);

	c = ps->blue;
	c = get(1)<2*get(0)?scl(get(5),-2*get(0),3*get(0),0,hue(ctl(4),2)+get(9)):min(x,y)-2*get(0)<get(0)?c*scl(min(x,y)-2*get(0),0,get(0),ctl(6),255)/255:c;
	ps->blue = CLAMP(c);
}


/////////////////////////////////////////////////////////////////////
//

// for GFrame3D_Round
static MAGIC parrGFrame3D_Round = {
	_T("3D ガラス"),
	GFrame3D_Round,
	2,
	{	{  41,  41, _T("フレームサイズ") },
		{ 230, 230, _T("シェード") },		}
};

// for GFrame3D_RoundEx
static MAGIC parrGFrame3D_RoundEx = {
	_T("3D ベベルガラス"),
	GFrame3D_RoundEx,
	3,
	{	{  97,  97, _T("フレームサイズ") },
		{  46,  46, _T("シェーディングとアングル") },
		{ 233, 233, _T("フレームサイクル") },		}
};

// for AfhBevel_I
static MAGIC parrAfhBevel_I = {
	_T("3D ボタン"),
	AfhBevel_I,
	7,
	{	{  28,  28, _T("左上の明るさ") },
		{ 128, 128, _T("左上のコントラスト") },
		{   0,   0, _T("右下の明るさ") },
		{  76,  76, _T("右下のコントラスト") },
		{  99,  99, _T("左と上のバランス") },
		{ 149, 149, _T("右と下のバランス") },
		{  36,  36, _T("ベベル幅") },	}
};

// for Bevel_III
static MAGIC parrBevel_III = {
	_T("3D 円形ボタン"),
	Bevel_III,
	5,
	{	{  90,  90, _T("明るさの向き") },
		{ 157, 157, _T("外側の半径") },
		{ 223, 223, _T("内側の半径") },
		{   9,   9, _T("外縁部周波数") },
		{  86,  86, _T("外縁部振幅") },		}
};

// for BullsEye
static MAGIC parrBullsEye = {
	_T("円形ボタン"),
	BullsEye,
	6,
	{	{  26,  26, _T("テクスチャ密度") },
		{ 154, 154, _T("外側半径") },
		{ 121, 121, _T("内側半径") },
		{ 190, 190, _T("赤") },
		{ 218, 218, _T("緑") },
		{ 255, 255, _T("青") },		}
};

// for MirrorBevel
static MAGIC parrMirrorBevel = {
	_T("ミラー ベベル"),
	MirrorBevel,
	2,
	{	{ 128, 128, _T("サイズ") },
		{  64,  64, _T("ミラーの選択") },		}
};

// for MirroredFrame
static MAGIC parrMirroredFrame = {
	_T("ミラー"),
	MirroredFrame,
	1,
	{	{  37,  37, _T("サイズ") },		}
};

// for SimpleColorFrame
static MAGIC parrSimpleColorFrame = {
	_T("カラー フレーム"),
	SimpleColorFrame,
	3,
	{	{  26,  26, _T("サイズ") },
		{ 138, 138, _T("色相") },
		{ 106, 106, _T("明るさ") },		}
};

// for FadeFrame
static MAGIC parrFadeFrame = {
	_T("フェード"),
	FadeFrame,
	4,
	{	{  50,  50, _T("サイズ") },
		{   0,   0, _T("赤") },
		{   0,   0, _T("緑") },
		{   0,   0, _T("青") },		}
};

// for ShadowCaster
static MAGIC parrShadowCaster = {
	_T("シャドウ キャスト"),
	ShadowCaster,
	4,
	{	{   6,   6, _T("影の幅") },
		{  36,  36, _T("影の赤") },
		{  36,  36, _T("影の緑") },
		{  36,  36, _T("影の青") },	}
};

// for MetalFrame
static MAGIC parrMetalFrame = {
	_T("メタル フレーム"),
	MetalFrame,
	7,
	{	{  36,  36, _T("フレーム幅") },
		{  94,  94, _T("ライティング") },
		{  39,  39, _T("リフレクション") },
		{ 111, 111, _T("照り") },
		{  36,  36, _T("色相") },
		{ 128, 128, _T("フレームの明るさ") },
		{ 128, 128, _T("影の明るさ") },			}
};

/////////////////////////////////////////////////////////////////////
//

HANDLE frame_simple_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrSimpleColorFrame, PREVIEW_WHOLE);
}

HANDLE frame_fadeframe_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrFadeFrame, PREVIEW_WHOLE);
}

HANDLE frame_bullseye_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, 	&parrBullsEye, PREVIEW_WHOLE);
}

HANDLE frame_3d_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrAfhBevel_I, PREVIEW_WHOLE);
}

HANDLE frame_3dglass_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrGFrame3D_Round, PREVIEW_WHOLE);
}

HANDLE frame_metal_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrMetalFrame, PREVIEW_WHOLE);
}

HANDLE frame_3dglassbevel_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrGFrame3D_RoundEx, PREVIEW_WHOLE);
}

HANDLE frame_bevelIII_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrBevel_III, PREVIEW_WHOLE);
}

HANDLE frame_mirrored_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrMirroredFrame, PREVIEW_WHOLE);
}

HANDLE frame_mirroredbevel_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrMirrorBevel, PREVIEW_WHOLE);
}

HANDLE frame_shdowcaster_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
	return single_works_ui(hdib, prc, pfe, &parrShadowCaster, PREVIEW_WHOLE);
}
