// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#define _WTL_NO_CSTRING
#define _WTL_NO_UNION_CLASSES

#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>
#include <atltime.h>
#include <atlcoll.h>

#include <atlapp.h>
#include <atlframe.h>
#include <atlsplit.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlx.h>
#include <atlgdi.h>
#include <atlmisc.h>

extern CAppModule _Module;

using namespace ATL;

#include <htmlhelp.h>
#include <htmlguid.h>

#include "..\lscore\atl_special_handler.h"
#include "..\lscore\osplatform.h"
#include "..\lscore\lscore.h"
#include "..\lscore\lccctrls.h"
#include "..\lscore\mrulist.h"
#include "..\lscore\gdiplus_sup.h"
#include "..\libarcspec\libarcspec.h"
#include "..\lsfop\lsfop.h"
#include "..\lsimg\lsimg.h"
#include "..\fv7\oeminc.h"
#include "..\fv7\color_theme.h"
#include "reg.h"

// user libraries
#pragma comment(lib, "lscore.lib")
#pragma comment(lib, "lsfop.lib")
#pragma comment(lib, "lsimg.lib")
#pragma comment(lib, "libarcspec.lib")

#include "debug.h"

#if (_ATL_VER >= 0x0700)
	#define MODINST		(ATL::_AtlBaseModule.GetModuleInstance())
	#define RESINST		(ATL::_AtlBaseModule.GetResourceInstance())
#else
	#define MODINST		(_Module.GetModuleInstance())
	#define RESINST		(_Module.GetResourceInstance())
#endif

#define CSpinButtonCtrl CUpDownCtrl

