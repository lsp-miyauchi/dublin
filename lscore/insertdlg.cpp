///////////////////////////////////////////////////////
// insertdlg.cpp: create control from dialog resource

#include "stdafx.h"
#include "lscore.h"
#include "fnc.h"

///////////////////////////////////////////////////////
//

#define ALIGNLONG(i)				(((i) + 3) / 4 * 4)


#pragma pack(1)

typedef struct tagFONTINFO {
	short int PointSize;
} FONTINFO;
typedef FONTINFO FAR *LPFONTINFO;

typedef struct
{
	WORD dlgVer;
	WORD signature;
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	WORD cDlgItems;
	short x;
	short y;
	short cx;
	short cy;
} DLGTEMPLATEEX;

typedef struct
{
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	short x;
	short y;
	short cx;
	short cy;
	DWORD id;
//	sz_Or_Ord windowClass; 
//	sz_Or_Ord title; 
//	WORD extraCount; 
} DLGITEMTEMPLATEEX;

#pragma pack()


///////////////////////////////////////////////////////
//

WCHAR *swzSystemClass[] = {						// for UNICODE strings
	WC_BUTTONW,
	WC_EDITW,
	WC_STATICW,
	WC_LISTBOXW,
	WC_SCROLLBARW,
	WC_COMBOBOXW
};

static _inline BOOL IsDialogEx(DLGTEMPLATE* pTemplate)
	{ return ((DLGTEMPLATEEX*)pTemplate)->signature == 0xFFFF; }

static _inline BOOL HasFont(DLGTEMPLATE* pTemplate)
	{ return (DS_SETFONT &
		(IsDialogEx(pTemplate) ? ((DLGTEMPLATEEX*)pTemplate)->style :
		pTemplate->style)); }

static _inline int FontAttrSize(BOOL bDialogEx)
	{ return sizeof(WORD) * (bDialogEx ? 3 : 1); }

static inline WCHAR* _SkipString(WCHAR* p)
	{ while (*p++); return p; }


///////////////////////////////////////////////////////
//

static WORD* __fastcall GetFontSizeField(DLGTEMPLATE* pTempl)
{
	BOOL bDialogEx = IsDialogEx(pTempl);
	register WORD* pw;
	if (bDialogEx)
		pw = (WORD*)((DLGTEMPLATEEX*)pTempl + 1);
	else
		pw = (WORD*)(pTempl + 1);

	if (*pw == 0xFFFF)        // Skip menu name string or ordinal
		pw += 2; // WORDs
	else
		while(*pw++);

	if (*pw == 0xFFFF)        // Skip class name string or ordinal
		pw += 2; // WORDs
	else
		while(*pw++);

	while (*pw++);         		// Skip caption string

	return pw;
}

extern "C"
int WINAPI lccInsertControls(HWND hwnd, LPRECT prcArea, HWND hwndAfter,
						HINSTANCE hInst, UINT idTemplate, HWND* pArray)
{
	HRSRC hrsrc = FindResource(hInst, _MKIR(idTemplate), RT_DIALOG);
	if (!hrsrc)
		return 0;

	HGLOBAL hmem = LoadResource(hInst, hrsrc);
	DLGTEMPLATE* pTempl = (DLGTEMPLATE*)LockResource(hmem);
	if (!pTempl)
		return 0;

	BOOL bDialogEx = IsDialogEx(pTempl);
	int cnt = bDialogEx ? (int)((DLGTEMPLATEEX*)pTempl)->cDlgItems :
		(int)pTempl->cdit;

	WORD* pw = GetFontSizeField(pTempl);
	if (pTempl->style & DS_SETFONT) {		// フォントが定義されていればスキップ
		pw++;
		pw = (WORD*)_SkipString((WCHAR*)pw);
	}

	DLGITEMTEMPLATE* pi = (LPDLGITEMTEMPLATE)pw;

	HINSTANCE hInstApp = GetWindowInstance(hwnd);
	int nCreate = 0;
	while (cnt-- != 0)
	{
		LPWSTR pText;

		// 次のアイテム配列を事前に計算
		BYTE* piNext = (BYTE*)pi;
		piNext += (bDialogEx ?
						sizeof(DLGITEMTEMPLATEEX) : sizeof(DLGITEMTEMPLATE));
		if (*(WORD*)piNext == 0xFFFF)	// Skip class name string or ordinal
			piNext += 2 * sizeof(WORD);
		else
			piNext = (BYTE*)_SkipString((WCHAR*)piNext);

		if (*(WORD*)piNext == 0xFFFF)	// Skip text string or ordinal
			piNext += 2 * sizeof(WORD);
		else
			piNext = (BYTE*)_SkipString((WCHAR*)piNext);

		WORD cbExtra = *(WORD*)piNext;	// Skip extra data
		piNext += sizeof(WORD) + cbExtra;
		piNext = (BYTE*)(((DWORD)piNext + 3) & ~3);		// DWORD align
		// ... 計算終了

		// クラス名の取得
		LPWSTR pClass = bDialogEx ?
				(LPWSTR)((DLGITEMTEMPLATEEX*)pi + 1) : (LPWSTR)(pi + 1);
		if (*pClass == 0xFFFF)				// 定義済みクラスのチェック
		{
			pClass++;
			pText = pClass + 1;				// タイトル配列を指す
			pClass = swzSystemClass[*pClass - 0x80];
		}
		else
			pText = _SkipString(pClass);

		// タイトル名
		LPWSTR pExtra;
		if (*pText == 0xFFFF)
		{
			pExtra = pText + 2;
			pText = NULL;
		}
		else
		{
			pExtra = _SkipString(pText);
			if (*pText == 0)
				pText = NULL;
		}

		CRect rc;
		DWORD style, exStyle;
		int id;
		if (bDialogEx)
		{
			DLGITEMTEMPLATEEX* pe = (DLGITEMTEMPLATEEX*)pi;
			rc.SetRect(pe->x, pe->y, pe->x + pe->cx, pe->y + pe->cy);
			style = pe->style;
			exStyle = pe->exStyle;
			id = pe->id;
		}
		else
		{
			rc.SetRect(pi->x, pi->y, pi->x + pi->cx, pi->y + pi->cy);
			style = pi->style;
			exStyle = pi->dwExtendedStyle;
			id = pi->id;
		}
		MapDialogRect(hwnd, &rc);
		rc.OffsetRect(prcArea->left, prcArea->top);

		HWND hctl = CreateWindowEx(exStyle, pClass, pText, style,
									rc.left, rc.top, rc.Width(), rc.Height(),
									hwnd, (HMENU)id, hInstApp, pExtra + 1);

		if (hctl)
		{
			SetWindowFont(hctl, GetWindowFont(hwnd), FALSE);
			if (hwndAfter)
				SetWindowPos(hctl, hwndAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

			if (pArray)
				pArray[nCreate] = hctl;
			nCreate++;
			hwndAfter = hctl;
		}

		pi = (DLGITEMTEMPLATE*)piNext;
	}
	return nCreate;
}
