//////////////////////////////////////////////////////////////////////
// ffactory_sup.h


#ifndef M_PI
	#define M_PI				(3.14159265358979323846)
#endif
#ifndef DE2RAD
	#define	DEG2RAD(x)			((x) * M_PI / 180.0)
#endif

#define D2R(x)					((double)(x) * M_PI / 512.0)

typedef struct tagPARAMITEM {
	int def;						// デフォルト値
	int val;						// 設定値
	LPTSTR label;
} PARAMITEM;

#define MAXPARAMS				8

typedef struct tagMAGIC MAGIC;

typedef void (__fastcall *MAGICPROC)(RGBA*, mif&, int, int);

struct tagMAGIC {
	LPTSTR fnc_name;
	MAGICPROC pfn;
	int numparams;					// パラメータアイテムの数
	PARAMITEM item[MAXPARAMS];
};

BOOL magic_works(mif& src, LPRECT prc, MAGIC* pa);

extern int val_stack_array[];
extern double dval_stack_array[];
extern int X;
extern int Y;
extern int X_center;
extern int Y_center;
extern int M;
extern PARAMITEM* item_ptr;
extern const int cos_tab[];
extern const int tan_tab[];

void __fastcall init_stack();
double __fastcall mval(double n, double vmin, double vmax);
double __fastcall mscl(double n, double il, double ih, double ol, double oh);
int __fastcall itan2(int x);
int __fastcall mix(int a, int b, int n, int d);
double __fastcall dmix(double a, double b, double n, double d);
int __fastcall dif(int a, int b);
double __fastcall dc2d(int x, int y);
int __fastcall yuvY(RGBA* pixel);
int __fastcall yuvU(RGBA* pixel);
int __fastcall yuvV(RGBA* pixel);
int __fastcall add(int a, int b, int c);
double __fastcall dr2x(double d, double m);
double __fastcall dr2y(double d, double m);

#define icos(a)				(cos_tab[abs((a))&1023])
#define isin(a)				(icos((a)-256))
#define itan(a)				(tan_tab[((a)+256)&511])
#define dscl(a,b,c,d,e)		(mscl((a),(b),(c),(d),(e)))
#define scl(a,b,c,d,e)		((int)mscl((a),(b),(c),(d),(e)))
#define c2d(x,y)			((int)dc2d((x),(y)))
#define dc2m(x,y)			(sqrt((double)((x)*(x)+(y)*(y))))
#define c2m(x,y)			((int)dc2m((x),(y)))
#define dist(x1,y1,x2,y2)	(c2m((x1)-(x2),(y1)-(y2)))
#define dc_dist(x,y)		(c2m((x)-X_center,(y)-Y_center))
#define dc_dir(x,y)			(c2d((x)-X_center,(y)-Y_center))
#define c_dist(x,y)			((int)dc_dist((x),(y)))
#define c_dir(x,y)			((int)dc_dir((x),(y)))
#define dsin(x)				(sin(D2R((x)))*512.0)
#define dcos(x)				(cos(D2R((x)))*512.0)
#define dtan(x)				(tan(D2R((x)))*512.0)
#define r2x(d,m)			((int)dr2x(d,m))
#define r2y(d,m)			((int)dr2y(d,m))

#define put(a,b)			(val_stack_array[(b)] = (a))
#define get(a)				(val_stack_array[(a)])
#define dput(a,b)			(dval_stack_array[(b)] = (a))
#define dget(a)				(dval_stack_array[(a)])
#define val(x,y,z)			((int)mval(item_ptr[(x)].val,(y),(z)))
#define dval(x,y,z)			(mval(item_ptr[(x)].val,(y),(z)))
#define ctl(x)				(item_ptr[(x)].val)
#define map(s,t)			(abs((t*(ctl(s*2+1)-ctl(s*2))/255)+ctl(s*2)))
#define CLAMP(a)			(min(255, max(0, (a))))
#define sbd(x,y)			(min(min((x),X-(x)-1),min((y),Y-(y)-1)))
#define sqr(a)				(((a)>0)?sqrt((a)):0)
#define rnd(a,b)			(((a)!=(b))?(rand()%((b)-(a)))+(a):(a))

int __fastcall hue(int x, int z);
void __fastcall gamma(int r, int g, int b);
int __fastcall gam(int n, int z);
void __fastcall radex(mif& src, double rad, double dist, double xcen, double ycen, RGBA* ps);
void __fastcall radxyex(mif& src, int x, int y, int xcen, int ycen, double rad, RGBA* ps);

#define rad(a,b,c,d)		(radex((a),(b),(c),X_center,Y_center,(d)))
#define radxy(a,b,c,d,e)	(radxyex((a),(b),(c),X_center,Y_center,(d),(e)))

RGBA* __fastcall getsrc(mif& src, int x, int y);
RGBA* __fastcall getsrc1(mif& src, int x, int y);
RGBA* __fastcall getsrc2(mif& src, int x, int y);
RGBA* __fastcall getsrc3(mif& src, int x, int y);
void __fastcall getsrc_linear(mif& src, double x, double y, RGBA* ps);
int __fastcall cnvsrc(mif& src, int x, int y, int m11, int m12, int m13, int m21, int m22, int m23, int m31, int m32, int m33, int dv);
void __fastcall blend(RGBA* p, int val);

#define PREVIEW_PART		1
#define PREVIEW_WHOLE		2
#define PREVIEW_BOTH		(PREVIEW_PART | PREVIEW_WHOLE)

typedef struct tagWORKS {
	HDIB hdib;
	int filter_number;
	int val[MAXPARAMS];
	int preview_mode;
	BOOL wholeview;
	MAGIC** magic_array;
	int num_magic;
} WORKS;

typedef struct tagSINGLEWORKS {
	HDIB hdib;
	int val[MAXPARAMS];
	int preview_mode;
	BOOL wholeview;
	MAGIC* magic;
} SINGLEWORKS;

BOOL CALLBACK WorksDlg(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK SingleWorksDlg(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam);

HANDLE single_works_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe, MAGIC* magic, int preview_mode);
HANDLE shade_works_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe, MAGIC* magic, int preview_mode);
