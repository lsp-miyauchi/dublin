#pragma once

#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NON_CONFORMING_SWPRINTFS

#define WINVER			0x0501
#define _WIN32_WINNT	0x0501
#define _WIN32_IE		0x0601
#define _RICHEDIT_VER	0x0400

//#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _WTL_NO_CSTRING
#define _WTL_NO_UNION_CLASSES

#ifdef _DEBUG
	#include <windows.h>
	#include <atldbgmem.h>
#endif

#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <shlobj.h>
#include <atlcoll.h>

#include <atlframe.h>
#include <atlsplit.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atlmisc.h>
#include <atltheme.h>

using namespace ATL;

#include "..\lscore\atl_special_handler.h"
#include "..\lscore\lscore.h"


