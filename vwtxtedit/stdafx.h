// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

#include "targetver.h"

#define _WTL_NO_CSTRING
#define _WTL_NO_UNION_CLASSES

#include <memory>
#include <vector>
#include <string>
#include <string_view>
#include <array>
#include <map>

#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>
#include <atlcoll.h>
//@ #include <atlrx.h>			// VS2008では未サポート？

#include <atlapp.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlx.h>
#include <atlgdi.h>
#include <atlprint.h>
#include <atlmisc.h>

extern CAppModule _Module;

using namespace ATL;

//#include <htmlhelp.h>
//#include <htmlguid.h>

#include "..\lscore\atl_special_rx.h"
#include "..\lscore\atl_special_handler.h"
#include "..\lscore\osplatform.h"
#include "..\lscore\lscore.h"
#include "..\lscore\lccctrls.h"
#include "..\lscore\gdiplus_sup.h"
//#include "..\gnu_libs\chardet\chardetapi.h"
#include "..\fv7\oeminc.h"


////////////////////////////////////////////////////////////////////////
// 大きい方、小さい方

template<typename T> inline T Min(T x,T y) { return (x<y ? x : y); }
template<typename T> inline T Max(T x,T y) { return (y<x ? x : y); }


#include "storage.h"
#include "gapbuf.h"
#include "clipboard.h"
#include "textfile.h"
#include "debug.h"

#ifdef _DEBUG
	#pragma comment(lib, "chardetd.lib")
#else
	#pragma comment(lib, "chardet.lib")
#endif

#if (_ATL_VER >= 0x0700)
	#define MODINST		(ATL::_AtlBaseModule.GetModuleInstance())
	#define RESINST		(ATL::_AtlBaseModule.GetResourceInstance())
#else
	#define MODINST		(_Module.GetModuleInstance())
	#define RESINST		(_Module.GetResourceInstance())
#endif


#pragma comment(lib, "imm32.lib")


#ifndef _USRDLL

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#endif	// _USRDLL
