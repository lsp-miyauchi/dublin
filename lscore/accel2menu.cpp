//////////////////////////////////////////////////////////////////////
// accel2menu.cpp

#include "stdafx.h"


static TCHAR* keyname[] = {
/*00-03*/	NULL,		NULL,		NULL,		NULL,
/*04-07*/	NULL,		NULL,		NULL,		NULL,
/*08-0B*/	_T("Bs"),	_T("Tab"),	NULL,		NULL,
/*0C-0F*/	_T("Clear"),_T("Enter"),NULL,		NULL,
/*10-13*/	NULL,		NULL,		NULL,		NULL,
/*14-17*/	NULL,		NULL,		NULL,		NULL,
/*18-1B*/	NULL,		NULL,		NULL,		_T("Esc"),
/*1C-1F*/	NULL,		NULL,		NULL,		NULL,
/*20-23*/	_T("Space"),_T("PgUp"),	_T("PgDn"),	_T("End"),
/*24-27*/	_T("Home"),	_T("��"),	_T("��"),	_T("��"),
/*28-2B*/	_T("��"),	NULL,		NULL,		NULL,
/*2C-2F*/	NULL,		_T("Ins"),	_T("Del"),	NULL,
/*30-33*/	_T("0"),	_T("1"),	_T("2"),	_T("3"),
/*34-37*/	_T("4"),	_T("5"),	_T("6"),	_T("7"),
/*38-3B*/	_T("8"),	_T("9"),	NULL,		NULL,
/*3C-3F*/	NULL,		NULL,		NULL,		NULL,
/*40-43*/	NULL,		_T("A"),	_T("B"),	_T("C"),
/*44-47*/	_T("D"),	_T("E"),	_T("F"),	_T("G"),
/*48-4B*/	_T("H"),	_T("I"),	_T("J"),	_T("K"),
/*4C-4F*/	_T("L"),	_T("M"),	_T("N"),	_T("O"),
/*50-53*/	_T("P"),	_T("Q"),	_T("R"),	_T("S"),
/*54-57*/	_T("T"),	_T("U"),	_T("V"),	_T("W"),
/*58-5B*/	_T("X"),	_T("Y"),	_T("Z"),	NULL,
/*5C-5F*/	NULL, 		NULL,		NULL,		NULL,
/*60-63*/	_T("0"),	_T("1"),	_T("2"),	_T("3"),
/*64-67*/	_T("4"),	_T("5"),	_T("6"),	_T("7"),
/*68-6B*/	_T("8"),	_T("9"),	_T("*"),	_T("+"),
/*6C-6F*/	NULL,		_T("-"),	NULL,		_T("/"),
/*70-73*/	_T("F1"),	_T("F2"),	_T("F3"),	_T("F4"),
/*74-77*/	_T("F5"),	_T("F6"),	_T("F7"),	_T("F8"),
/*78-7B*/	_T("F9"),	_T("F10"),	_T("F11"),	_T("F12"),
/*7C-7F*/	_T("F13"),	_T("F14"),	_T("F15"),	_T("F16"),
};

#define MAXMENUITEMTEXTLENGTH		100
#define _TU(a)						(((a)>='a'&&(a)<='z')?((a)+('a'-'A')):(a))

static void Accel2String(CString& str, ACCEL* pa)
{
	str.Empty();
	if (pa->fVirt & FCONTROL)	str += _T("Ctrl+");
	if (pa->fVirt & FSHIFT)		str += _T("Shift+");
	if (pa->fVirt & FALT)		str += _T("Alt+");

	if (pa->fVirt & FVIRTKEY) {
		if (pa->key >= 0x80 || !keyname[pa->key])
			return;
		str += keyname[pa->key];
	}
	else
		str += (TCHAR)_TU(pa->key);
	return;
}

extern "C"
void WINAPI lccApplyAccelMenu(HMENU hMenu, ACCEL* pa, int numAccl)
{
	if (hMenu != NULL && !IsMenu(hMenu)) return;

	int nItems = ::GetMenuItemCount(hMenu);
	TCHAR szString[MAXMENUITEMTEXTLENGTH];
	for (int i = 0; i < nItems; i++) {
		szString[0] = 0;

		CMenuItemInfo mii;
		mii.fMask = MIIM_STRING | MIIM_SUBMENU | MIIM_ID;
		mii.dwTypeData = szString;
		mii.cch = ARRAYSIZE(szString);
		BOOL bRet = ::GetMenuItemInfo(hMenu, i, TRUE, &mii);
		ATLASSERT(bRet);

		if (mii.hSubMenu)
			lccApplyAccelMenu(mii.hSubMenu, pa, numAccl);
		else if (szString[0] != 0) {
			TCHAR* p = StrChr(szString, _T('\t'));
			if (p) *p = 0;

			CSimpleArray<CString> arr;
			CString str;
			for (int j = 0; j < numAccl; j++) {
				if (pa[j].cmd == mii.wID) {
					Accel2String(str, &pa[j]);
					if (arr.Find(str) < 0)
						arr.Add(str);
				}
			}
			int cnt = arr.GetSize();
			if (cnt > 0) {
				str.Format(_T("%s\t"), szString);
				for (int n = 0; n < cnt; n++) {
					if (n > 0)
						str += _T(", ");
					str += arr[n];
				}
				mii.fMask = MIIM_STRING;
				mii.dwTypeData = (LPTSTR)str.GetString();
				::SetMenuItemInfo(hMenu, i, TRUE, &mii);
			}
		}
	}
}

extern "C"
BOOL WINAPI lccGetAccelString(UINT idCommand, ACCEL* pa, int numAccl, LPTSTR psz, int cchMax)
{
	if (psz && cchMax > 0) {
		CSimpleArray<CString> arr;
		CString str;
		for (int j = 0; j < numAccl; j++) {
			if (pa[j].cmd == idCommand) {
				Accel2String(str, &pa[j]);
				if (arr.Find(str) < 0)
					arr.Add(str);
			}
		}
		int cnt = arr.GetSize();
		if (cnt > 0) {
			str.Empty();
			for (int n = 0; n < cnt; n++) {
				if (n > 0)
					str += _T(", ");
				str += arr[n];
			}
			lstrcpyn(psz, str.GetString(), cchMax);
			return TRUE;
		}
	}
	return FALSE;
}
