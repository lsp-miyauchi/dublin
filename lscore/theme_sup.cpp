///////////////////////////////////////////////////////
// theme_sup.cpp - visual-style wrapper/support functions

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"

extern "C"
BOOL WINAPI lccThIsThemeActive()
{
	CTheme t;
	return (t.IsThemingSupported()) ? IsThemeActive() : FALSE;
}

extern "C"
BOOL WINAPI lccThIsAppThemed()
{
	CTheme t;
	return (t.IsThemingSupported()) ? IsAppThemed() : FALSE;
}

