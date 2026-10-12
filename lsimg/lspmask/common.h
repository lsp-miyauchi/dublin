////////////////////////////////////////////////////////////////
// common.h

extern HINSTANCE ghInst;

#define MSGOUT(a)				(MessageBox(GetFocus(),(a),"debug",MB_OK))

#ifndef	WIDTHBYTES
	#undef WIDTHBYTES
	#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)
#endif

#define UPSHIFTED(x)			((int)((x) * (1L << 14) + 0.5))
#define UPSHIFT(x)				((int)(x) << 14)
#define DOWNSHIFT(x)			(((int)((x) + (1L << 13))) >> 14)
#define MAXRGB					255
#define FILTERWIDTH				2.0
#define MINVALUE(a,b)			(((a)<(b))?(a):(b))
#define MAXVALUE(a,b)			(((a)>(b))?(a):(b))
#define MIN(a,b)				((a)<(b)?(a):(b))
#define MAX(a,b)				((a)>(b)?(a):(b))

class waitcursor
{
public:
	waitcursor() { m_hcur = SetCursor(LoadCursor(NULL, IDC_WAIT)); }
	~waitcursor() { SetCursor(m_hcur); }
	HCURSOR m_hcur;
};

#define DOWNSCALE(a)		(((WORD)(a)) >> 8)
#define UPSCALE(a)			(((WORD)(a)) * 257)
//#define ADJRGB(a)			(min(255, max(0, (int)(a))))
#define ADJRGB(a)			(gpcbRangeTable[(int)(a)])
#define ADJRGB16(a)			(min(65535, max(0, (int)(a))))
#define ADJHUE(a)			(min(359, max(0, (int)(a))))
#define ADJSAT(a)			(min(228, max(0, (int)(a))))
#define PALLOC(a, b)		(GlobalAllocPtr(GHND, (a) * (b)))
#define PFREE(a)			(GlobalFreePtr((a)))

#define M_PI				(3.14159265358979323846)
#define M_PI_2				(1.57079632679489661923)
#define M_PI_4				(0.78539816339744830962)
#define INTENSITY(r,g,b)	((DWORD)((DWORD)(r)*77+(DWORD)(g)*150+(DWORD)(b)*29)>>8)
#define	DEG2RAD(x)			((x) * M_PI / 180.0)


void loadblock(BYTE* pcb, BYTE* pData, int sx, int sy, int dx, int dy, int cxBlock, int cyBlock);
void saveblock(BYTE* pcb, BYTE* pData, int sx, int sy, int dx, int dy, int cxBlock, int cyBlock);
void loadblock8(LPBYTE pcb, LPBYTE pData, int sx, int sy, int dx, int dy);
void saveblock8(LPBYTE pcb, LPBYTE pData, int sx, int sy, int dx, int dy);
void loadblock16(LPBYTE pcb, LPBYTE pData, int sx, int sy, int dx, int dy);
void saveblock16(LPBYTE pcb, LPBYTE pData, int sx, int sy, int dx, int dy);
void loadpkt8(LPRGBPK pcb, LPRGBPK pData, int sx, int sy, int dx, int dy);
void savepkt8(LPRGBPK pcb, LPRGBPK pData, int sx, int sy, int dx, int dy);
RGBPK __fastcall interpolate(RGBPK* pData, RGBPK* pixel, int cx, int cy, int x, int y);
void form_mirror(LPBYTE pData, int cx, int cy);
void form_flip(LPBYTE pData, int cx, int cy);
BOOL __fastcall make_gradtable(int entry, COLORREF* rgbs, PALETTEENTRY* ppe);
BOOL __fastcall grad_table(COLORREF* rgbs, int numcolors, PALETTEENTRY* ppe, int numpalettes);


