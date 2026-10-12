// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。

#pragma once

#include "targetver.h"

#define _WTL_NO_UNION_CLASSES
#define _WTL_NO_CSTRING


#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>
#include <atlcoll.h>

#include <shlobj.h>

#include <atlapp.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atlpath.h>
#include <atlmisc.h>

extern CAppModule _Module;

using namespace ATL;

#include <commoncontrols.h>

#include "..\lscore\atl_special_handler.h"
#include "..\lscore\osplatform.h"
#include "..\lscore\lscore.h"
#include "..\lscore\lccctrls.h"
#include "..\lscore\cdll.h"
#include "..\lscore\gdiplus_sup.h"
#include "..\lsfop\lsfop.h"
#include "..\lsimg\lsimg.h"
#include "..\libarcspec\libarcspec.h"
#include "..\lsarcsvr\lsarcsvr_i.h"
#include "..\fv7\oeminc.h"
#include "reg.h"

// user libraries
#pragma comment(lib, "lscore.lib")
#pragma comment(lib, "lsfop.lib")
#pragma comment(lib, "libarcspec.lib")

#if (_ATL_VER >= 0x0700)
	#define MODINST		(ATL::_AtlBaseModule.GetModuleInstance())
	#define RESINST		(ATL::_AtlBaseModule.GetResourceInstance())
#else
	#define MODINST		(_Module.GetModuleInstance())
	#define RESINST		(_Module.GetResourceInstance())
#endif


#include "debug.h"

