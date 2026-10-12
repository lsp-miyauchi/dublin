////////////////////////////////////////////////////////////////////////
// stdafx.h

#ifndef STRICT
#define STRICT
#endif

#define NTDDI_VERSION	NTDDI_WIN8

#include "targetver.h"

#define __USE_CREATETHREAD__

#pragma warning(disable: 4100)	// 引数が関数内で使われないという警告を無視

#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>
#include <atlapp.h>
#include <atlcoll.h>
#include <atlcom.h>
#include <atltime.h>

extern CAppModule _Module;

#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlx.h>
#include <atlmisc.h>
#include <atluser.h>
#include <atlsync.h>
#include <atltheme.h>

using namespace ATL;

#define CString CAtlString

#include <shlobj.h>
#include <shlwapi.h>
#include <vssym32.h>
//#include <tmschema.h>
#include <uxtheme.h>
#include <htmlhelp.h>
#include <htmlguid.h>

#include <gdiplus.h>
using namespace Gdiplus;
#include "gdiplus_sup.h"
#include <nlohmann\json.hpp>
using json = nlohmann::json;


#include "js-array.h"

#pragma comment(lib, "gdiplus.lib")

#include "memmgr.h"
#include "debug.h"

#include "..\lscore\atl_special_rx.h"
#include "..\lscore\atl_special_handler.h"
#include "archive.h"

// #include "..\lscore\commctrl_vista.h"
#include "..\lscore\xwin.h"
#include "..\lscore\lscore.h"
#include "..\lscore\lccctrls.h"
#include "..\lscore\mrulist.h"
#include "..\lscore\osplatform.h"
#include "..\lscore\menubar.h"
#include "..\libarcspec\libarcspec.h"
#include "..\lspexif\lspexifapi.h"
#include "..\lsfop\lsfop.h"
#include "..\lsfop\lsfop_i.h"
#include "..\gnu_libs\tinyxml_2_5_3\tinyxml.h"
//#include "..\gnu_libs\sqlite_3_7_5\source\sqlite3.h"
#include "..\..\sqlite_3_40_0\x64-Release\sqlite3.h"
#include "globaldefs.h"
#include "usermsg.h"
#include "stringz.h"

typedef DWORD DROPEFFECT;
typedef unsigned (WINAPI *PBEGINTHREADEX_START_ROUTINE)(void* pThreadParameter);
typedef PBEGINTHREADEX_START_ROUTINE LPBEGINTHREADEX_START_ROUTINE;

// Undocumented window messages for LISTBOX NOTIFY
#define	WM_LBTRACKPOINT			0x131


#ifndef INT_MAX
	#define INT_MAX					2147483647		// maximum (signed) int value
#endif

#ifndef MAX_URL_LENGTH
	#define MAX_URL_LENGTH			(2048 + 32 + 8)
#endif

#include "oeminc.h"				// OEM用

#if (_ATL_VER >= 0x0700)
	#define MODINST		(ATL::_AtlBaseModule.GetModuleInstance())
	#define RESINST		(ATL::_AtlBaseModule.GetResourceInstance())
#else
	#define MODINST		(_Module.GetModuleInstance())
	#define RESINST		(_Module.GetResourceInstance())
#endif

#ifndef WM_HELPHITTEST
#define WM_HELPHITTEST      0x0366  // lResult = dwContext,
#define WM_SETMESSAGESTRING 0x0362  // wParam = nIDS (or 0),
#endif

// user libraries
// #pragma comment(lib, "lscore.lib")
// #pragma comment(lib, "lsfop.lib")
// #pragma comment(lib, "libsqlite3.lib")
// #ifdef _DEBUG
//	#pragma comment(lib, "tinyxmld.lib")
//#else
//	#pragma comment(lib, "tinyxml.lib")
//#endif

#if defined _M_IX86
/* --- ix86リリース版が外部マニフェストの場合
	#ifdef _DEBUG
	#pragma comment(linker, \
			"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' " \
			"processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
	#endif
 --------------------------------------------- */
	#pragma comment(linker, \
			"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' " \
			"processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
	#pragma comment(linker, \
			"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' " \
			"processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
	#pragma comment(linker, \
			"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' " \
			"processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
	#pragma comment(linker, \
			"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' " \
			"processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
