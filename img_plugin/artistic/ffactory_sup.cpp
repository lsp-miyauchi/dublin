//////////////////////////////////////////////////////////////////////
// ffactory_sup.cpp

#include "stdafx.h"
#include "ffactory_sup.h"

void progress_init(int maxprogress);
void progress_continue();
void progress_term();

extern PROGRESSCALLBACK pfnProgress;

int val_stack_array[256];
double dval_stack_array[256];
int X;
int Y;
int X_center;
int Y_center;
int M;
PARAMITEM* item_ptr;
BOOL gamma_init = FALSE;

#include "ff_table.h"

void __fastcall init_stack()
{
	gamma_init = FALSE;
	memset(val_stack_array, 0, sizeof(val_stack_array));
	memset(dval_stack_array, 0, sizeof(dval_stack_array));
}

int __fastcall itan2(int x)
{
	if ((x%512)==0)
		return 0;
	if ((x%256)==0)
		return 512;
	double t = tan(D2R(x));
	int tt = (int)floor(t*128+0.5);
	return min(512, max(-512, tt));
}

double __fastcall mval(double n, double a, double b)
{
	return ((n*(b-a))/255)+a;
}

double __fastcall mscl(double a, double il, double ih, double ol, double oh)
{
	return (ih!=il) ? (((a-il)*(oh-ol))/(ih-il))+ol : 0;
}

int __fastcall mix(int a, int b, int n, int d)
{
	return (d!=0) ? (a*n + b*(d-n))/d : 0;
}

double __fastcall dmix(double a, double b, double n, double d)
{
	return (d!=0) ? (a*n + b*(d-n))/d : 0;
}

int __fastcall dif(int a, int b)
{
	return abs(a - b);
}

double __fastcall dc2d(int x, int y)
{
	double d = (x<0) ? 512 : 0;
	if (y!=0)
		d=(256.0 * (atan(x / fabs((double)y))/(M_PI/2.0)))-256;
	return (y<0) ? d : -d;
}

double __fastcall dr2x(double d, double m)
{
	return (cos(M_PI * d / 512.0)*m);
}

double __fastcall dr2y(double d, double m)
{
	return (sin(M_PI * d / 512.0)*m); 
}

int __fastcall yuvY(RGBA* ps)
{
	return (2990 * (int)ps->red + 5870 * (int)ps->green + 1140 * (int)ps->blue) / 10000;
}

int __fastcall yuvU(RGBA* ps)
{
	int n = (-1474 * (int)ps->red - 2895 * (int)ps->green + 4369 * (int)ps->blue) / 20000;
	return (n >> 1);
}

int __fastcall yuvV(RGBA* ps)
{
	int n = (6150 * (int)ps->red - 5150 * (int)ps->green + 1000 * (int)ps->blue) / 10000;
	return (n >> 1);
}

int __fastcall add(int a, int b, int c)
{
	int n = a + b;
	return min(n, c);
}

int __fastcall hue(int x, int z)
{
	if (z == 0)
		x = scl(x,0,255,-768,256);
	else if (z == 1)
		x = scl(x,0,255,-64,900);
	else
		x = scl(x,0,255,-128,512);
	return (isin(x)+512)/4;
}

RGBA* __fastcall getsrc(mif& src, int x, int y)
{
	x = min(src.m_width - 1, max(0, x));
	y = min(src.m_height - 1, max(0, y));
	return src.pos(x, y);
}

static const RGBA pix = { 0, 0, 0, 0 };

RGBA* __fastcall getsrc1(mif& src, int x, int y)
{
	if (x < 0 || y < 0 || x >= src.m_width || y >= src.m_height)
		return (RGBA*)&pix;
	return src.pos(x, y);
}

RGBA* __fastcall getsrc2(mif& src, int x, int y)
{
	x = x % src.m_width;
	if (x < 0)
		x = src.m_width + x;
	y = y % src.m_height;
	if (y < 0)
		y = src.m_height + y;
	return src.pos(x, y);
}

RGBA* __fastcall getsrc3(mif& src, int x, int y)
{
	if (x < 0)
		x = abs(x) % src.m_width;
	if (x >= src.m_width)
		x = src.m_width - x % (src.m_width - 1);
	if (y < 0)
		y = abs(y) % src.m_height;
	if (y >= src.m_height)
		y = src.m_height - y % (src.m_height - 1);
	return src.pos(x, y);
}

void __fastcall getsrc_linear(mif& src, double x, double y, RGBA* ps)
{
	double m = floor(y);
	double n = floor(x);
	double q = y - m;
	double p = x - n;

	RGBA* p1 = getsrc3(src,(int)n,     (int)m);
	RGBA* p2 = getsrc3(src,(int)(n+1), (int)m);
	RGBA* p3 = getsrc3(src,(int)n,     (int)(m+1));
	RGBA* p4 = getsrc3(src,(int)(n+1), (int)(m+1));

	double d;
	d = (1.0-q)*((1.0-p)*(double)p1->red + p*(double)p2->red) +
				q*((1.0-p)*(double)p2->red + p * (double)p4->red);
	ps->red = CLAMP((int)d);

	d = (1.0-q)*((1.0-p)*(double)p1->green + p*(double)p2->green) +
				q*((1.0-p)*(double)p2->green + p * (double)p4->green);
	ps->green = CLAMP((int)d);

	d = (1.0-q)*((1.0-p)*(double)p1->blue + p*(double)p2->blue) +
				q*((1.0-p)*(double)p2->blue + p * (double)p4->blue);
	ps->blue = CLAMP((int)d);
}

#define INTENSITY(r,g,b)	((DWORD)((DWORD)(r)*77+(DWORD)(g)*150+(DWORD)(b)*29)>>8)

int __fastcall cnvsrc(mif& src, int x, int y, int m11, int m12, int m13, int m21, int m22, int m23, int m31, int m32, int m33, int dv)
{
	RGBA* p[9];

	p[0] = getsrc3(src,x-1,y-1);
	p[1] = getsrc3(src,x  ,y-1);
	p[2] = getsrc3(src,x+1,y-1);
	p[3] = getsrc3(src,x-1,y  );
	p[4] = getsrc3(src,x,  y);
	p[5] = getsrc3(src,x+1,y  );
	p[6] = getsrc3(src,x-1,y+1);
	p[7] = getsrc3(src,x  ,y+1);
	p[8] = getsrc3(src,x+1,y+1);

	int sum = 0;
	sum += m11 * INTENSITY(p[0]->red,p[0]->green,p[0]->blue);
	sum += m12 * INTENSITY(p[1]->red,p[1]->green,p[1]->blue);
	sum += m13 * INTENSITY(p[2]->red,p[2]->green,p[2]->blue);
	sum += m21 * INTENSITY(p[3]->red,p[3]->green,p[3]->blue);
	sum += m22 * INTENSITY(p[4]->red,p[4]->green,p[4]->blue);
	sum += m23 * INTENSITY(p[5]->red,p[5]->green,p[5]->blue);
	sum += m31 * INTENSITY(p[6]->red,p[6]->green,p[6]->blue);
	sum += m32 * INTENSITY(p[7]->red,p[7]->green,p[7]->blue);
	sum += m33 * INTENSITY(p[8]->red,p[8]->green,p[8]->blue);

	return sum / dv;
}

void __fastcall blend(RGBA* p, int val)
{
	int c = (int)p->red*val/255;
	p->red = CLAMP(c);
	c = (int)p->green*val/255;
	p->green = CLAMP(c);
	c = (int)p->blue*val/255;
	p->blue = CLAMP(c);
}

void __fastcall gamma(int r, int g, int b)
{
	if (gamma_init)
		return;

	// r,g,b = 255 means gamma 1.0, r,g,b = 511 means gamma 2.0
	register int i, newclr;
	double clr;

	for (i = 0; i < 256; i++)
	{
		clr = 255.0 * pow((i / 255.0), 255 / (double)r);
		newclr = (int)floor(clr + 0.5);
		gamma_table[i] = CLAMP(newclr);

		clr = 255.0 * pow((i / 255.0), 255 / (double)g);
		newclr = (int)floor(clr + 0.5);
		gamma_table[256 + i] = CLAMP(newclr);

		clr = 255.0 * pow((i / 255.0), 255 / (double)b);
		newclr = (int)floor(clr + 0.5);
		gamma_table[512 + i] = CLAMP(newclr);
	}
	gamma_init = TRUE;
}

int __fastcall gam(int n, int z)
{
	z = min(2, max(0, z));
	n = CLAMP(n);
	return (int)gamma_table[z * 256 + n];
}

void __fastcall radex(mif& src, double rad, double dist, double xcen, double ycen, RGBA* ps)
{
	double xx = xcen + dr2x(rad, dist);
	double yy = ycen + dr2y(rad, dist);
	getsrc_linear(src, xx, yy, ps);
}

void __fastcall radxyex(mif& src, int x, int y, int xcen, int ycen, double rad, RGBA* ps)
{
	double r = D2R(rad);
	double c = cos(r);
	double s = sin(r);

	double yy = ycen + (x - xcen) * s + (y - ycen) * c;
	double xx = xcen + (x - xcen) * c - (y - ycen) * s;
	double m = (yy > ycen) ? yy : yy - 1;
	double n = (xx > xcen) ? xx : xx - 1;
	double q = yy - m;
	double p = xx - n;

	RGBA* p1 = getsrc3(src,(int)n,     (int)m);
	RGBA* p2 = getsrc3(src,(int)(n+1), (int)m);
	RGBA* p3 = getsrc3(src,(int)n,     (int)(m+1));
	RGBA* p4 = getsrc3(src,(int)(n+1), (int)(m+1));

	double d;
	d = (1.0-q)*((1.0-p)*(double)p1->red + p*(double)p2->red) +
				q*((1.0-p)*(double)p2->red + p * (double)p4->red);
	ps->red = CLAMP((int)d);

	d = (1.0-q)*((1.0-p)*(double)p1->green + p*(double)p2->green) +
				q*((1.0-p)*(double)p2->green + p * (double)p4->green);
	ps->green = CLAMP((int)d);

	d = (1.0-q)*((1.0-p)*(double)p1->blue + p*(double)p2->blue) +
				q*((1.0-p)*(double)p2->blue + p * (double)p4->blue);
	ps->blue = CLAMP((int)d);
}

BOOL magic_works(mif& src, LPRECT prc, MAGIC* pa)
{
	CRect rc;
	if (!pa || !src.validaterect(prc, &rc))
		return FALSE;
	src.m_bits = 24;

	init_stack();

	// �L��ϐ��ݒ�
	X = rc.Width();
	Y = rc.Height();
	X_center = X / 2;
	Y_center = Y / 2;
	M = dist(0, 0, X, Y) / 2;
	item_ptr = pa->item;

progress_init(Y);
	mif dst(X, Y, 24);
	dst.blt(0, 0, X, Y, &src, rc.left, rc.top);
	for (int y = 0; y < Y; y++) {
		RGBA* ps = src.pos(rc.left, y + rc.top);
		for (int x = 0; x < X; x++) {
			(*pa->pfn)(ps, dst, x, y);
			ps++;
		}
progress_continue();
	}
progress_term();
	return TRUE;
}
