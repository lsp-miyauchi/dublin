////////////////////////////////////////////////////////////////////////
// common.cpp

#include "stdafx.h"


void InitDlgCtrl(HWND hctl, UINT idResource)
{
	TCHAR buf[MAX_PATH * 2];
	if (GetClassName(hctl, buf, _COUNTOF(buf)) < 1) return;
	CString str;
	if (str.LoadString(idResource) < 1) return;

	int type = -1;
	if (!StrCmpI(buf, WC_COMBOBOX))
		type = 1;
	else if (!StrCmpI(buf, WC_COMBOBOXEX))
		type = 2;

	lstrcpy(buf, str);
	LPTSTR p = buf;
	COMBOBOXEXITEM item;
	while (p && *p) {
		LPTSTR q = StrChr(p, ';');
		if (q) *q++ = 0;

		switch (type) {
		case 1:
			::SendMessage(hctl, CB_ADDSTRING, 0, (LPARAM)p);
			break;
		case 2:
			item.mask = CBEIF_TEXT;
			item.iItem = -1;
			item.pszText = p;
			SendMessage(hctl, CBEM_INSERTITEM, 0, (LPARAM)&item);
			break;
		}
		p = q;
	}
}
