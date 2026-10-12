////////////////////////////////////////////////////////////////////
// common.h


extern DWORD gdwComCtlVersion;
#define VERSION_XP				MAKELONG(0, 6)

////////////////////////////////////////////////////////////////////
// •¶Žš—ñƒ}ƒNƒ

#define VALIDSTR(str)			(*((LPCTSTR)(str)) != '\0')


////////////////////////////////////////////////////////////////////
// char search

//#define _SSEP(a)		(StrChr((a),'\\'))
//#define _SRSEP(a)		(StrRChr((a),NULL,'\\'))
//#define _SCHR(a,b)		(StrChr((a),(b)))
//#define _SRCHR(a,b)		(StrRChr((a),NULL,(b)))
//#define _SUPR(a)		(CharUpper((a)))
//#define _SLWR(a)		(CharLower((a)))
//#define _SICMP(a,b)		(StrCmpI((a),(b)))
//#define _SNICMP(a,b,c)	(StrCmpNI((a),(b),(c)))
//#define _SBPBRK(a,b)	(StrPBrk((a),(b)))

#define _ISNETPATH(a)	(lstrlen((a)) > 3 && *(a) == '\\' && *((a) + 1) == '\\' && *((a) + 2) != '?')


#define CR_ERROR		-1
#define CR_DONE			0
#define CR_IGNORED		1
#define CR_ABORT		2


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


/////////////////////////////////////////////////
// spin button

#define Spin_GetPos(a)			(SendMessage((a),UDM_GETPOS,0,0))
#define Spin_SetPos(a,b)		(SendMessage((a),UDM_SETPOS,0,MAKELPARAM((b),0)))
#define Spin_SetRange(a,b,c)	(SendMessage((a),UDM_SETRANGE,0,MAKELPARAM((c),(b))))
