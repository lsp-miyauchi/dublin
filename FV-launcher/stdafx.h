// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change these values to use different versions
#include "targetver.h"

#define _WTL_NO_CSTRING

#include <atlbase.h>
#include <atlstr.h>
#include <atlwin.h>
#include <atlapp.h>
#include <atlframe.h>
#include <atlcoll.h>
#include <atlcom.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlx.h>
#include <atlmisc.h>
#include <atluser.h>
#include <atlsync.h>

extern CAppModule _Module;

#include <shlobj.h>
#include <shlwapi.h>
#include <atltheme.h>
#include <memory.h>
#include "..\lscore\atl_special_handler.h"
#include "..\lscore\lscore.h"
#include "..\lscore\dwmapi_sup.h"
#include "..\lscore\gdiplus_sup.h"
#include "..\lscore\osplatform.h"
#include "..\lsfop\lsfop.h"

#include "..\fv7\oeminc.h"

#include "debug.h"

#if (_ATL_VER >= 0x0700)
	#define MODINST		(ATL::_AtlBaseModule.GetModuleInstance())
	#define RESINST		(ATL::_AtlBaseModule.GetResourceInstance())
#else
	#define MODINST		(_Module.GetModuleInstance())
	#define RESINST		(_Module.GetResourceInstance())
#endif

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
