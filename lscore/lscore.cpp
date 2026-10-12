///////////////////////////////////////////////////////
// lscore.cpp

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "cdll.h"
#include "fnc.h"

///////////////////////////////////////////////////////
//

static WORD stwLccVersion = 0x0700;

///////////////////////////////////////////////////////
//

CAppModule _Module;
COSPlatform gosp;
static CSimpleMap<int, HIMAGELIST> mapImageList;


///////////////////////////////////////////////////////

TCHAR gszSoftware[] = _T("Software");
TCHAR gszCompany[]  = _T("LightShip");
TCHAR gszCommonSetting[] = _T("CommonSetting");



///////////////////////////////////////////////////////
//

TCHAR gszAst[] = _T("*.*");


///////////////////////////////////////////////////////
//

static BOOL InitDLL(HINSTANCE hInst);
static void ExitDLL();


///////////////////////////////////////////////////////
// Initialize Library

extern "C"
BOOL WINAPI DllMain(HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:	return InitDLL(hDLL);
	case DLL_THREAD_ATTACH:		break;
	case DLL_THREAD_DETACH:		break;
	case DLL_PROCESS_DETACH:	ExitDLL(); 	break;
	}
	return TRUE;
}

ATOM Register_HeaderCtrl();
// ATOM Register_RebarCtrl();

static BOOL InitDLL(HINSTANCE hInst)
{
	DisableThreadLibraryCalls(hInst);

	OleInitialize(NULL);
	::DefWindowProc(NULL, 0, 0, 0L);
	_Module.Init(NULL, hInst);

	// commctrl
	AtlInitCommonControls(0xffff);

	WNDCLASS    wc;
	memset(&wc, 0, sizeof(WNDCLASS));
	wc.hInstance     = MODINST;
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hIcon         = NULL;
	wc.lpszMenuName  = NULL;
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS | CS_PARENTDC;
	wc.lpszClassName = WC_LSC_STATIC;
	wc.lpfnWndProc   = StaticWndFn;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	wc.lpszClassName = WC_LSC_RECT;
	wc.lpfnWndProc   = RectWndFn;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	wc.lpszClassName = WC_LSC_SIGNAL;
	wc.lpfnWndProc   = SignalWndFn;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	wc.lpszClassName = WC_LSC_COLOR;
	wc.lpfnWndProc   = ColorWndFn;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	wc.lpszClassName = WC_LSC_TASKLINK;
	wc.lpfnWndProc   = TaskLinkWndFn;
	wc.hCursor       = LoadCursor(NULL, IDC_HAND);
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	wc.lpszClassName = WC_LSC_URL;
	wc.lpfnWndProc   = UrlWndFn;
	wc.hCursor       = LoadCursor(NULL, IDC_HAND);
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	wc.lpszClassName = WC_LSC_METER;
	wc.lpfnWndProc   = MeterWndFn;
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	wc.lpszClassName = WC_LSC_PROGRESS;
	wc.lpfnWndProc   = ColoredProgressBarWndFn;
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	wc.lpszClassName = WC_LSC_CHECK;
	wc.lpfnWndProc   = CheckWndFn;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	wc.lpszClassName = WC_LSC_FLATBUTTON;
	wc.lpfnWndProc   = FlatButtonWndFn;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	wc.lpszClassName = WC_LSC_SHADE;
	wc.lpfnWndProc   = ShadeWndFn;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	wc.lpszClassName = WC_LSC_INFOBAR;
	wc.lpfnWndProc   = InfoBarWndFn;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	wc.lpszClassName = WC_LSC_LEDCLOCK;
	wc.lpfnWndProc   = LedClockWndFn;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	wc.lpszClassName = WC_LSC_ANIIMAGE;
	wc.lpfnWndProc   = AniImageWndFn;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

#ifdef USE_PRGLIST
	// PrgList Window Class
	wc.lpszClassName = WC_LSC_PRGLIST;
	wc.lpfnWndProc   = PrgListWndFn;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;
#endif

	// Palette Window Class
	wc.lpszClassName = WC_LSC_PAL;
	wc.lpfnWndProc   = PalWndFn;
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS | CS_DBLCLKS;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	// Log Window Class
	wc.lpszClassName = WC_LSC_LOG;
	wc.lpfnWndProc   = LogWndFn;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.style         = CS_GLOBALCLASS;
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	// Degree Window Class
	wc.lpszClassName = WC_LSC_DEGREE;
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.lpfnWndProc   = DegreeWndFn;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS | CS_PARENTDC;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	// Track Window Class
	wc.lpszClassName = WC_LSC_TRACK;
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.lpfnWndProc   = TrackWndFn;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS | CS_PARENTDC;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	// Slider Window Class
	wc.lpszClassName = WC_LSC_SLIDER;
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.lpfnWndProc   = SliderWndFn;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS | CS_PARENTDC;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	// FontPick Window Class
	wc.lpszClassName = WC_LSC_FONTPICK;
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.lpfnWndProc   = FontPickWndFn;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS | CS_PARENTDC;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	// ColorPick Window Class
	wc.lpszClassName = WC_LSC_COLORPICK;
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.lpfnWndProc   = ColorPickWndFn;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS | CS_DBLCLKS | CS_PARENTDC;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	// TipEdit Window Class
	wc.lpszClassName = WC_LSC_TIPEDIT;
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.lpfnWndProc   = TipEditWndFn;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.style         = CS_GLOBALCLASS;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	// List Window Class
#ifdef USE_LISTCONTROL
	wc.lpszClassName = WC_LSC_LIST;
	wc.lpfnWndProc   = ListWndFn;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;
#endif	// USE_LISTCONTROL

	// ZoomView
	wc.lpszClassName = WC_LSC_ZOOMVIEW;
	wc.lpfnWndProc   = ZoomViewWndFn;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS | CS_DBLCLKS;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	// DrawListView
	wc.lpszClassName = WC_LSC_LISTVIEW;
	wc.lpfnWndProc   = DrawListViewWndFn;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS | CS_DBLCLKS;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	// DrawListView
	wc.lpszClassName = WC_LSC_CHECKTREEVIEW;
	wc.lpfnWndProc   = CheckTreeViewWndFn;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	// GradEditor
	wc.lpszClassName = WC_LSC_GRADEDIT;
	wc.lpfnWndProc   = GradEditWndFn;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS | CS_DBLCLKS;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	// FolderTree Window Class
	wc.lpszClassName = WC_LSC_FOLDERTREE;
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS | CS_PARENTDC;
	wc.lpfnWndProc   = FolderTreeWnd;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	// IDLISTPick Window Class
	wc.lpszClassName = WC_LSC_IDLISTPICK;
	wc.lpfnWndProc   = IDListPickWnd;
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	// Splitter Window Class
	wc.lpszClassName = WC_LSC_SPLITTER;
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.style         = CS_DBLCLKS | CS_GLOBALCLASS;
	wc.lpfnWndProc   = SplitterWndFn;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	// Tab Window Class
	wc.lpszClassName = WC_LSC_TAB;
	wc.style         = CS_GLOBALCLASS | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = TabWndFn;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void*);
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	if (!RegisterClass(&wc)) return FALSE;

	// TabEx Window Class
	wc.lpszClassName = WC_LSC_TAB_EX;
	wc.style         = CS_GLOBALCLASS | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = TabExWndFn;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void*);
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	if (!RegisterClass(&wc)) return FALSE;

	// SearchPathCtrl Window Class
	wc.lpszClassName = WC_LSC_SEARCHPATH;
	wc.hCursor       = LoadCursor(0, IDC_ARROW);
	wc.lpfnWndProc   = SearchPathWndFn;
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(void*);
	if (!RegisterClass(&wc)) return FALSE;

	if (!Register_HeaderCtrl()) return FALSE;

//	if (!Register_RebarCtrl()) return FALSE;

	return TRUE;
}

static void ExitDLL()
{
	for (int i = 0; i < mapImageList.GetSize(); i++) {
		HIMAGELIST him = mapImageList.GetValueAt(i);
		if (him)
			ImageList_Destroy(him);
	}

	_Module.Term();
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
	OleUninitialize();
}


////////////////////////////////////////////////////////////////////////
//

HIMAGELIST GetCommonImageList(int type)
{
	HIMAGELIST him = NULL;
	if (IsRGBA()) {
		int pos = mapImageList.FindKey(type);
		if (pos < 0) {
			int idResource = 0;
			int cxWidth = 0;
			COLORREF clrMask = CLR_DEFAULT;
			switch (type) {
			case IMAGERESOURCE_DOTCOLOR:	idResource = IDB_DOT_COLOR;			cxWidth = 11; break;
			}

			if (idResource) {
				him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(idResource), cxWidth, CLR_DEFAULT);
				mapImageList.Add(type, him);
			}
		}
		else
			him = mapImageList.GetValueAt(pos);
	}
	return him;
}


///////////////////////////////////////////////////////
// Version infomation

extern "C"
WORD WINAPI lccGetVersion(void)
{
	return stwLccVersion;
}


extern "C"
void WINAPI lccSetVersion(WORD ver)
{
//	stwLccVersion = ver;
}
