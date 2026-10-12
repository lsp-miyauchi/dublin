//////////////////////////////////////////////////////////
// common.h

#define DBGMSG(a,b)		(MessageBox(GetFocus(),(a),(b),MB_OK))

#ifndef	WIDTHBYTES
	#undef WIDTHBYTES
	#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)
#endif

#define MIN(a,b)				((a)<(b)?(a):(b))
#define MAX(a,b)				((a)>(b)?(a):(b))
#define ADJRGB(a)				(BYTE)(min(255, max(0, (int)(a))))
#define MAKE556WORD(r,g,b)		((((WORD)(r)>>3)<<11)|(((WORD)(g)>>3)<<6)|((WORD)(b)>>2))
#define MAKE565WORD(r,g,b)		((((WORD)(r)>>3)<<11)|(((WORD)(g)>>2)<<5)|((WORD)(b)>>3))
#define MAKE555WORD(r,g,b)		((((WORD)(r)>>3)<<10)|(((WORD)(g)>>3)<<5)|((WORD)(b)>>3))
#define MAKE565RGB(w,r,g,b)		(r=(w&0xF800)>>8,g=(w&0x07E0)>>3,b=(w&0x001F)<<3)
#define MAKE555RGB(w,r,g,b)		(r=(w&0x7C00)>>7,g=(w&0x03E0)>>2,b=(w&0x001F)<<3)

#define M_PI					(3.14159265358979323846)
#define M_PI_2					(1.57079632679489661923)
#define M_PI_4					(0.78539816339744830962)
#define INTENSITY(r,g,b)		((DWORD)((DWORD)(r)*77+(DWORD)(g)*150+(DWORD)(b)*29)>>8)
#define	DEG2RAD(x)				((x) * M_PI / 180.0)

#define INT_MULT(a,b,t)			((t) = (a) * (b) + 0x80, ((((t) >> 8) + (t)) >> 8))
#define INT_BLEND(a,b,alpha,tmp)	(INT_MULT((a)-(b), alpha, tmp) + (b))
#define INT_MULT3(a,b,c,t)		((t) = (a) * (b) * (c)+ 0x7F5B, ((((t) >> 7) + (t)) >> 16))

#define RINT(x)					floor((x) + 0.5)

_inline int mixed(int src, int dst, int opaque)
	{ if (opaque == 0) return src;
		if (opaque == 255) return dst;
		return ADJRGB((src * (255 - opaque) + dst * opaque) / 255); }


void __fastcall rgb_to_hls(int *r, int *g, int *b);
void __fastcall hls_to_rgb(int *h, int *l, int *s);
_inline int rgb_to_l(int red, int green, int blue)
	{ return ((max(max(red, green), blue) + min(min(red, green), blue)) >> 1); }
_inline int rgba_to_l(RGBA* p)
	{ return ((max(max(p->red, p->green), p->blue) + min(min(p->red, p->green), p->blue)) >> 1); }
void __fastcall rgb_to_hsv(BYTE r, BYTE g, BYTE b, double* hue, double* sat, double* val);
void __fastcall hsv_to_rgb(BYTE* pr, BYTE* pb, BYTE* pg, double hue, double sat, double val);

void __fastcall rgb_lab(double R, double G, double B, int *L, int *a, int *b);
void __fastcall lab_rgb(int L, int a, int b, double *R, double *G, double *B);


BOOL __fastcall grad_table(COLORREF* rgbs, int numcolors, PALETTEENTRY* ppe, int numpalettes);
LOGPALETTE* __fastcall grad_table_ex(GRADCOLORS* pcol, int numcolors, int numtables);
RGBA __fastcall destcopy_pixel(RGBA* p, RGBA* q);
RGBA __fastcall blend_pixel(RGBA* p, RGBA* q, int blend);
RGBA __fastcall darken_pixel(RGBA* p, RGBA* q);
RGBA __fastcall lighten_pixel(RGBA* p, RGBA* q);
RGBA __fastcall hsv_hue_pixel(RGBA* p, RGBA* q);
RGBA __fastcall hsv_sat_pixel(RGBA* p, RGBA* q);
RGBA __fastcall hsv_lum_pixel(RGBA* p, RGBA* q);
RGBA __fastcall hsv_only_pixel(RGBA* p, RGBA* q, int mode);
RGBA __fastcall color_only_pixel(RGBA* p, RGBA* q);
RGBA __fastcall multiply_pixel(RGBA* p, RGBA* q);
RGBA __fastcall divide_pixel(RGBA* p, RGBA* q);
RGBA __fastcall screen_pixel(RGBA* p, RGBA* q);
RGBA __fastcall overlay_pixel(RGBA* p, RGBA* q);
RGBA __fastcall add_pixel(RGBA* p, RGBA* q);
RGBA __fastcall del_pixel(RGBA* p, RGBA* q);
RGBA __fastcall idiv_pixel(RGBA* p, RGBA* q);
RGBA __fastcall subtract_pixel(RGBA* p, RGBA* q);
RGBA __fastcall difference_pixel(RGBA* p, RGBA* q);
RGBA __fastcall hardlight_pixel(RGBA* p, RGBA* q);
RGBA __fastcall softlight_pixel(RGBA* p, RGBA* q);
RGBA __fastcall dodge_pixel(RGBA* p, RGBA* q);
RGBA __fastcall burn_pixel(RGBA* p, RGBA* q);
RGBA __fastcall exclusion_pixel(RGBA* p, RGBA* q);

typedef RGBA (__fastcall *BLENDFUNC)(RGBA*, RGBA*);
BLENDFUNC get_blendfunction(int method);

void __fastcall blend_to(RGBA* p, RGBA* q, int blend);


void* load_resource_data(HINSTANCE hInst, UINT typeResource, UINT idResource, DWORD& size);
