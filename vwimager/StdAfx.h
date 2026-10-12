// stdafx.h : 標準のシステム インクルード ファイル、
//            または参照回数が多く、かつあまり変更されない
//            プロジェクト専用のインクルード ファイルを記述します。
//

#pragma once

#include "targetver.h"

#define _WTL_NO_UNION_CLASSES
#define _WTL_NO_CSTRING

#include <atlbase.h>
#include <atlwin.h>
#include <wininet.h>
#include <atlstr.h>
#include <atlcoll.h>

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

#include "..\lscore\atl_special_handler.h"
#include "..\lscore\osplatform.h"
#include "..\lscore\lscore.h"
#include "..\lscore\lccctrls.h"
#include "..\lscore\commctrl_vista.h"
#include "..\lscore\gdiplus_sup.h"
#include "..\lsfop\lsfop.h"
#include "..\lsimg\lsimg.h"
#include "..\lsimg\lsmif\lsmif.h"
#include "..\fv7\oeminc.h"
#include "reg.h"

// user libraries
#pragma comment(lib, "lscore.lib")
#pragma comment(lib, "lsfop.lib")
#pragma comment(lib, "lsmif.lib")
#pragma comment(lib, "lsimg.lib")

struct __declspec(uuid("f490eb00-1240-11d1-9888-006097deacf9")) IActiveDesktop;

#if (_ATL_VER >= 0x0700)
	#define MODINST		(ATL::_AtlBaseModule.GetModuleInstance())
	#define RESINST		(ATL::_AtlBaseModule.GetResourceInstance())
#else
	#define MODINST		(_Module.GetModuleInstance())
	#define RESINST		(_Module.GetResourceInstance())
#endif

#include "archive.h"
#include "debug.h"
