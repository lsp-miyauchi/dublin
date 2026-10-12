////////////////////////////////////////////////////////////////////////
// globaldefs.h

#pragma once

#ifndef __GLOBALDEFS_H__
#define __GLOBALDEFS_H__


#define MY_INFOTIP_LEN			400
#define MAX_URL_LENGTH			(2048 + 32 + 8)
#define MENU_OWNERDRAW_HEIGHT	(16 + 4)

class _STRorCH
{
public:
	LPCTSTR m_pstr;
	_STRorCH(LPCTSTR pstr) : m_pstr(pstr) { }
	_STRorCH(TCHAR ch) : m_pstr(MAKEINTRESOURCE(ch)) { }
};


////////////////////////////////////////////////////////////////////
// FILE ATTRIBUTES
//
#define FAT_SUBDIR				FILE_ATTRIBUTE_DIRECTORY
#define FAT_SYSTEM				FILE_ATTRIBUTE_SYSTEM
#define FAT_HIDDEN				FILE_ATTRIBUTE_HIDDEN
#define FAT_RDONLY				FILE_ATTRIBUTE_READONLY
#define FAT_ARCH				FILE_ATTRIBUTE_ARCHIVE
#define FAT_NORMAL				FILE_ATTRIBUTE_NORMAL
#define FAT_TEMP				FILE_ATTRIBUTE_TEMPORARY
#define FAT_CMPRSD				FILE_ATTRIBUTE_COMPRESSED
#define FAT_ENCRYPT				FILE_ATTRIBUTE_ENCRYPTED
#define FAT_REPP				FILE_ATTRIBUTE_REPARSE_POINT
#define FAT_ATOMIC				FILE_ATTRIBUTE_ATOMIC_WRITE

////////////////////////////////////////////////////////////////////
// ポインタマクロ
#define MakePtr(cast, base, ofs)	((cast)((base) + (ofs)))

////////////////////////////////////////////////////////////////////
//

#define VALIDHANDLE(handle)		((handle) != INVALID_HANDLE_VALUE)
#define VALIDSTR(str)			(*((LPCTSTR)(str)) != '\0')
#define _ISKEYDOWN(vkey)		((GetKeyState((vkey)) & 0x8000) != 0)
#define _INRANGE(a,b,c)			((b) <= (a) && (a) <= (c))
#define _INRANGEEX(a,b,c)		((b) < (a) && (a) < (c))
#define _OUTRANGE(a,b,c)		((a) < (b) || (a) > (c))
#define ISVKEYALPHA(a)			(_INRANGE((a), 'A', 'Z'))
#define ISVKEYNUM(a)			(_INRANGE((a), '0', '9'))
#define ISVKEYNUMPAD(a)			(_INRANGE((a), VK_NUMPAD0, VK_NUMPAD9))
#define ISVKEYDIGIT(a)			(ISVKEYNUM((a)) || ISVKEYNUMPAD((a)))
#define ISVKEYALNUM(a)			(ISVKEYALPHA((a)) || ISVKEYDIGIT((a)))
#define TOGGLEFLAG(a,b)			(((a) & (b)) ? ((a) & ~(b)) : ((a) | (b)))
#define SETFLAG(a,b)			((a) |= (b))
#define RESETFLAG(a,b)			((a) &= ~(b))

#define ISALPHA(a)				(((a)>='a'&&(a)<='z')||((a)>='A'&&(a)<='Z'))
#define ISDIGIT(a)				((a)>='0'&&(a)<='9')

////////////////////////////////////////////////////////////////////
// MessageBox ACTION
//
#define MB_ATTENTION		(MB_OK | MB_ICONEXCLAMATION)
#define MB_QUESTION			(MB_YESNOCANCEL | MB_ICONQUESTION)
#define MB_QYESNO			(MB_YESNO | MB_ICONQUESTION)
#define MB_QOKCANCEL		(MB_OKCANCEL | MB_ICONQUESTION)
#define MB_QABORT			(MB_ICONEXCLAMATION | MB_ABORTRETRYIGNORE)
#define MB_HAND				(MB_OKCANCEL | MB_ICONHAND)
#define MB_STOP				(MB_OK | MB_ICONSTOP)
#define MB_QINFO			(MB_OKCANCEL | MB_ICONINFORMATION)
#define MB_QAST				(MB_OKCANCEL | MB_ICONASTERISK)
#define MB_QEXCM			(MB_OKCANCEL | MB_ICONEXCLAMATION)
#define MB_QRETRY			(MB_RETRYCANCEL | MB_ICONEXCLAMATION)


////////////////////////////////////////////////////////////////////
//
#define	SYSCOLOR_WINDOWTEXT		(GetSysColor(COLOR_WINDOWTEXT))
#define	SYSCOLOR_WINDOW			(GetSysColor(COLOR_WINDOW))
#define	SYSCOLOR_MENUTEXT		(GetSysColor(COLOR_MENUTEXT))
#define	SYSCOLOR_MENU			(GetSysColor(COLOR_MENU))
#define	SYSCOLOR_HIGHLIGHT		(GetSysColor(COLOR_HIGHLIGHT))
#define	SYSCOLOR_HIGHLIGHTTEXT	(GetSysColor(COLOR_HIGHLIGHTTEXT))



#endif		// __GLOBALDEFS_H__
