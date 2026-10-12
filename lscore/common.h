//////////////////////////////////////////////////////////////////////
// common.h

#define SEL_NORMAL			0
#define SEL_POP				1
#define SEL_PUSH			2
#define SEL_BORDER			3

//////////////////////////////////////////////////////////////////////
//

extern TCHAR gszAst[];

//////////////////////////////////////////////////////////////////////
//

class CSetRedraw
{
private:
	bool m_redraw;
	HWND m_hwnd;
public:
	CSetRedraw(HWND hwnd)
	{
		m_redraw = false;
		m_hwnd = NULL;
		if (hwnd && ::IsWindow(hwnd)) {
			m_hwnd = hwnd;
			m_redraw = true;
			::SendMessage(m_hwnd, WM_SETREDRAW, FALSE, 0);
		}
	}

	~CSetRedraw()
	{
		if (m_redraw && ::IsWindow(m_hwnd)) {
			::SendMessage(m_hwnd, WM_SETREDRAW, TRUE, 0);
			::InvalidateRect(m_hwnd, NULL, TRUE);
		}
	}
};

//////////////////////////////////////////////////////////////////////
//

void FillShadeRect(HDC hdc, CRect& rc, COLORREF crFrom, COLORREF crTo, BOOL bVert);
void FillShadeRectEx(HDC hdc, CRect& rc, COLORREF* crs, int numcolors, BOOL bVert);
COLORREF __fastcall mix_color(COLORREF clr1, COLORREF clr2, int clr1_per);
void _FillG(HDC hdc, CRect& rc, COLORREF cr);
void draw_rect(HDC hdc, CRect& rcPane, int mode);
void draw_rect(HDC hdc, LPRECT prc, COLORREF rgbBorder, COLORREF rgbInner);
COLORREF __fastcall LightenColor(const COLORREF crColor, UINT nIncreaseVal);
COLORREF __fastcall DarkenColor(const COLORREF crColor, UINT nReduceVal);
void __fastcall rpl_str(LPTSTR pText, LPCTSTR pSrc, LPCTSTR pDst);
BOOL __fastcall _pathfileexists(LPCTSTR pszName);
BOOL __fastcall _pathisfile(LPCTSTR pszName);
BOOL __fastcall _pathisdirectory(LPCTSTR pszName);
BOOL __fastcall parselinkpath(LPCTSTR pszName, LPTSTR pszTarget, int cchMax);
BOOL getlinkinfo(LPCTSTR pszName, CString& target);
HGLOBAL __fastcall dupmemhandle(HGLOBAL hsrc);
HWND __fastcall gettopparent(HWND hwnd);

COLORREF __fastcall calc_innercolor(COLORREF base);
void DrawAlphaImageList(HIMAGELIST him, int index, HDC hdc, int x, int y, BOOL bAlpha);

HBITMAP CreateBitmapFromImageList(HIMAGELIST him, int iImage);
HBITMAP LoadImageFromResourceData(HINSTANCE hInst, LPCTSTR typeResource, UINT idResource);

void* FindLoadLockResource(HINSTANCE hInst, LPCTSTR resType, LPCTSTR resName, DWORD* pSize = NULL);
IStream* StreamFromResource(HINSTANCE hInst, LPCTSTR resType, LPCTSTR resName, DWORD* pSize);
