////////////////////////////////////////////////////////////////////
// common.h


#pragma once
#ifndef __TEXTPAD_COMMON_H__
#define __TEXTPAD_COMMON_H__



extern UINT nRM_OPEN;
extern UINT nRM_JUMPTO;
extern UINT nRM_FINDNOTIFY;

extern COSPlatform gosp;
__inline bool OsVISTA() { return gosp.OsVISTA(); }

#ifndef HID_BASE_COMMAND

// Help ID bases
#define HID_BASE_COMMAND    0x00010000UL        // ID and IDM
#define HID_BASE_RESOURCE   0x00020000UL        // IDR and IDD
#define HID_BASE_PROMPT     0x00030000UL        // IDP
#define HID_BASE_NCAREAS    0x00040000UL
#define HID_BASE_CONTROL    0x00050000UL        // IDC
#define HID_BASE_DISPATCH   0x00060000UL        // IDispatch help codes

#endif

////////////////////////////////////////////////////////////////////
// PRIVATE PROFILE ACCESS
//
#define GETMYINI(a,b,c)		(theApp.GetProfileString((a),(b),(c)))
#define SETMYINI(a,b,c)		(theApp.WriteProfileString((a),(b),(c)))

#define FA_HIDDEN		FILE_ATTRIBUTE_HIDDEN
#define FA_SYSTEM		FILE_ATTRIBUTE_SYSTEM
#define FA_RDONLY		FILE_ATTRIBUTE_READONLY

//----------------------------------------------
// MessageBox ACTION
//
#define MB_ATTENTION	(MB_OK | MB_ICONEXCLAMATION)
#define MB_QUESTION		(MB_YESNOCANCEL | MB_ICONQUESTION)
#define MB_QYESNO		(MB_YESNO | MB_ICONQUESTION)
#define MB_QOKCANCEL	(MB_OKCANCEL | MB_ICONEXCLAMATION)
#define MB_QABORT		(MB_ICONEXCLAMATION | MB_ABORTRETRYIGNORE)
#define MB_HAND			(MB_OKCANCEL | MB_ICONHAND)
#define MB_STOP			(MB_OK | MB_ICONSTOP)
#define MB_QAST			(MB_OKCANCEL | MB_ICONASTERISK)
#define MB_QEXCM		(MB_OKCANCEL | MB_ICONEXCLAMATION)
#define MB_QRETRY		(MB_RETRYCANCEL | MB_ICONEXCLAMATION)

////////////////////////////////////////////////

#define LODWORD(a)				(*((DWORD*)&(a)))
#define HIDWORD(a)				(*(((DWORD*)&(a)) + 1))
#define SETDWORDLONG(a,b,c)		(*((DWORD*)&(a))=(DWORD)(b),*(((DWORD*)&(a))+1)=(DWORD)(c))


#define CTLCHAR(a)		((a)<' ')
#define ASCII(a)		(' '<=(a)&&(a)<='~')
#define DIGIT(a)		('0'<=(a)&&(a)<='9')
#define ALPHA(a)		(('a'<=(a)&&(a)<='z')||('A'<=(a)&&(a)<='Z')||(a)=='_')
#define ALNUM(a)		(ALPHA((a))||DIGIT((a)))

#ifdef _UNICODE
	#define JHAN(a)			(0xff61 <= (a) && (a) <= 0xff9f)
	#define KDIGIT(a)		(0xff10 <= (a) && (a) <= 0xff19)
	#define KALPHA(a)		((0xff21 <= (a) && (a) <= 0xff3a) || (0xff41 <= (a) && (a) <= 0xff5a))
	#define KHIRA(a)		(0x3041 <= (a) && (a) <= 0x3093)
	#define KKATA(a)		(0x30a1 <= (a) && (a) <= 0x30fe)
	#define KKANJI(a)		((0x4e00 <= (a) && (a) <= 0x9fa5) || (0xf929 <= (a) && (a) <= 0xfa2d))
	#define KKEISEN(a)		(0x2500 <= (a) && (a) <= 0x256c)
#else
	#define SJISC(a)		(*((BYTE*)(a)) << 8 | *(((BYTE*)(a))+1))
	#define JHAN(a)			(0xa1 <= (BYTE)(a) && (BYTE)(a) <= 0xdf)
	#define KDIGIT(a)		(0x824f<=(a)&&(a)<=0x8258)
	#define KALPHA(a)		((0x8260<=(a)&&(a)<=0x8279)||(0x8281<=(a)&&(a)<=0x829a))
	#define KHIRA(a)		(0x829f<=(a)&&(a)<=0x82f1)
	#define KKATA(a)		((0x8340<=(a)&&(a)<=0x8396)||(a)==0x815b)
	#define KKANJI(a)		((0x889f<=(a))||(0x8152<=(a)&&(a)<=0x8159))
	#define KKEISEN(a)		(0x849f <= (a) && (a) <= 0x84be)
#endif

#define KALNUM(a)		(KALPHA((a))||KDIGIT((a)))


//////////////////////////////////////////////////
//

int AppMessage(UINT nID, LPCTSTR pszInsert = nullptr, UINT nType = MB_STOP);
int LastError(LPCTSTR pszTitle, UINT nType = MB_STOP);
TCHAR* __fastcall SkipTabSpace(TCHAR* p);
int SeparateString(LPCTSTR pszText, int delim, CSimpleArray<CString>& arr);
void __fastcall FormatText(LPCTSTR lpszSrc, LPTSTR lpszDst, int nTabConv, int nMax);
void PopupHint(HWND hwnd, UINT nID, DWORD style = PVS_CAPTION);

void InitHtmlHelp();
void __fastcall ShowHtmlHelp(HWND hwnd, UINT idDlg);
int __fastcall match(LPCTSTR string, LPCTSTR pattern);
void __fastcall ResHelp(HWND hwnd, UINT idResource);

void InitDlgCtrl(HWND hctl, UINT idResource);



#endif			// __TEXTPAD_COMMON_H__
