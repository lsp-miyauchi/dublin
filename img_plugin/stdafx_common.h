////////////////////////////////////////////////////////////////////////
// tdafx_common.h

#pragma once

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NON_CONFORMING_SWPRINTFS

#define WINVER			0x0501
#define _WIN32_WINNT	0x0501
#define _WIN32_IE		0x0600
#define _RICHEDIT_VER	0x0300

#define _WTL_NO_UNION_CLASSES
#define _WTL_NO_CSTRING

#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>
#include <atlcoll.h>

#include <atlapp.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlx.h>
#include <atlgdi.h>
#include <atlmisc.h>

extern CAppModule _Module;

using namespace ATL;

#include <math.h>
#include "..\lscore\atl_special_handler.h"
#include "..\lscore\lscore.h"
#include "..\lscore\lccctrls.h"
#include "..\lsimg\lsimg.h"
#include "..\lsimg\lsmif\lsmif.h"
#include "chartype.h"
#include "xwin.h"
#include "plugin.h"
#include "regkey.h"

#pragma comment(lib, "lscore.lib")
#pragma comment(lib, "lsimg.lib")
#pragma comment(lib, "lsmif.lib")

#if (_ATL_VER >= 0x0700)
	#define MODINST		(ATL::_AtlBaseModule.GetModuleInstance())
	#define RESINST		(ATL::_AtlBaseModule.GetResourceInstance())
#else
	#define MODINST		(_Module.GetModuleInstance())
	#define RESINST		(_Module.GetResourceInstance())
#endif

#include "debug.h"
