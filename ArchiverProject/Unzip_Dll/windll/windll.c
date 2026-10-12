/*
 Copyright (C) 1996 Mike White
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as all of the original files are included,
 that it is not sold for profit, and that this copyright notice is retained.

*/
/* Windows Info-ZIP Unzip DLL module
 *
 * Author: Mike White
 *
 * Original: 1996
 *
 * This module has the entry points for "unzipping" a zip file.
 */

/*---------------------------------------------------------------------------

  This file is the WINDLL replacement for the generic ``main program source
  file'' unzip.c.

  See the general comments in the header part of unzip.c.

  Copyrights:  see accompanying file "COPYING" in UnZip source distribution.
               (This software is free but NOT IN THE PUBLIC DOMAIN.  There
               are some restrictions on commercial use.)

  ---------------------------------------------------------------------------*/

#include <windows.h>
#include <windowsx.h>
#ifdef __RSXNT__
#  include "win32/rsxntwin.h"
#endif
#define UNZIP_INTERNAL
#include "..\unzip.h"
#include "..\crypt.h"
#include "..\version.h"
#include "windll.h"
#include "structs.h"
#include "..\consts.h"

/* Added type casts to prevent potential "type mismatch" error messages. */
#ifdef REENTRANT
#  undef __G
#  undef __G__
#  define __G                 (Uz_Globs *)pG
#  define __G__               (Uz_Globs *)pG,
#endif

HANDLE hwildZipFN;
HANDLE hInst;               /* current instance */
HANDLE hDCL;
DWORD gdwLocked;
int fNoPrinting = 0;
extern jmp_buf dll_error_return;

BOOL gbTryLastPassword;
static char sszStat[] = "Status";
static char sszPass[] = "Password";
static char sszTryp[] = "TryLastPassword";



/* For displaying status messages and error messages */
static int UZ_EXP DllMessagePrint(zvoid *pG, uch *buf, ulg size, int flag);

#if 0 /* currently unused */
/* For displaying files extracted to the display window */
int DllDisplayPrint(zvoid *pG, uch *buf, ulg size, int flag);
#endif /* never */

/* Callback function for status report and/or user interception */
static int UZ_EXP LuzStatReportCB(zvoid *pG, int fnflag, ZCONST char *zfn,
                                   ZCONST char *efn, ZCONST zvoid *details);

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID plvReserved)
{
	char *p, buf[MAX_PATH * 2];

	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			hInst = hInstance;

			// モジュール名が変更されていないか確認する
			GetModuleFileName(hInst, buf, MAX_PATH * 2);
			p = (char*)_mbsrchr((BYTE*)buf, '\\');
			if (!p)
				p = buf;
			else
				p++;
			if (lstrcmpi(p, "lspuzp.dll"))
				return FALSE;

			gbTryLastPassword = FALSE;
			rgGetValue(sszStat, sszTryp, &gbTryLastPassword);
			gdwLocked = 0;
			break;

		case DLL_PROCESS_DETACH:
		default:
			break;
	}
	return TRUE;
}

int FAR PASCAL WEP ( int bSystemExit )
{
	return 1;
}

/*
    fOnlyNewer  = true if you are to extract only newer
    fSpc2Undscr = true if convert space to underscore
    fAskOverwrite = true if prompt to overwrite is wanted
    fQuiet    = quiet flag. 1 = few messages, 2 = no messages, 0 = all messages
    fConsole  = write to stdout if true
    fTest     = test zip file
    fVrbose   = verbose listing
    fUpdate   = "update" (extract only newer/new files)
    fComment  = display zip file comment
    fMkdir    = all args are files/dir to be extracted
    fOverwrite= overwrite all files
    fConvCR   = do end-of-line translation
    fInfo     = get Zip Info if TRUE
    fCase     = be case insensitive if TRUE
    fPrivilege = restore ACL's if 1, use privileges if 2
    lpszZipFN = zip file name
    lpszExtractDir = directory to extract to; NULL means: current directory
*/

BOOL WINAPI LuzSetOpts(zvoid* pG, LPDCL pDcl)
{
	uO.qflag = pDcl->fQuiet;				// Quiet flag
	G.pfnames = (char**)&fnames[0];			// assign default file name vector
	G.pxnames = (char**)&fnames[1];

	uO.jflag = !pDcl->fMkdir;
	uO.cflag = pDcl->fConsole;
	uO.overwrite_all = pDcl->fOverwrite;
	uO.tflag = pDcl->fTest;
	uO.vflag = pDcl->fVerbose;
	uO.zflag = pDcl->fComment;
	uO.uflag = pDcl->fUpdate;
	uO.aflag = pDcl->fConvCR;
	uO.C_flag = pDcl->fCase;
	uO.uflag = uO.fflag = pDcl->fOnlyNewer;
	G.prompt_always = pDcl->fAskOverwrite;
	uO.X_flag = pDcl->fPrivilege;
	uO.overwrite_none = !uO.overwrite_all;
	uO.sflag = pDcl->fSpc2Undscr;			// Translate spaces to underscores?
	if (pDcl->fInfo)
	{
		uO.zipinfo_mode = TRUE;
		uO.hflag = TRUE;
		uO.lflag = 10;
		uO.qflag = 2;
	}
	else
	{
		uO.zipinfo_mode = FALSE;
	}

#if CRYPT
	uO.pwdarg = pDcl->lpszPassword;
#endif		// ?CRYPT

	G.extract_flag = (!uO.zipinfo_mode &&
                      !uO.cflag && !uO.tflag && !uO.vflag && !uO.zflag
#ifdef TIMESTAMP
                      && !uO.T_flag
#endif
                     );

	if (pDcl->lpszExtractDir != NULL && G.extract_flag)
	{
#ifndef CRTL_CP_IS_ISO
       char *pExDirRoot = (char *)malloc(lstrlen(pDcl->lpszExtractDir)+1);

       if (pExDirRoot == NULL)
           return FALSE;
       ISO_TO_INTERN(pDcl->lpszExtractDir, pExDirRoot);
#else
#  define pExDirRoot pDcl->lpszExtractDir
#endif
       uO.exdir = pExDirRoot;
	}
	else
	{
		uO.exdir = NULL;
	}

	// G.wildzipfn needs to be initialized so that do_wild does not wind
	// up clearing out the zip file name when it returns in process.c
	hwildZipFN = GlobalAlloc(GPTR, FILNAMSIZ);
	if (hwildZipFN == (HGLOBAL) NULL)
       return FALSE;

    G.wildzipfn = GlobalLock(hwildZipFN);
    lstrcpy(G.wildzipfn, pDcl->lpszZipFN);
    _ISO_INTERN(G.wildzipfn);

	return TRUE;
}

void FreeDllMem(__GPRO)
{
    if (G.wildzipfn) {
        GlobalUnlock(hwildZipFN);
        G.wildzipfn = NULL;
    }
    if (hwildZipFN)
        hwildZipFN = GlobalFree(hwildZipFN);

    uO.zipinfo_mode = FALSE;
}

int WINAPI LuzSingleEntryUnzip(int ifnc, char **ifnv, int xfnc, char **xfnv, LPDCL pDcl, LPUSERFUNCTIONS lpUserFunc)
{
	int retcode;
	CONSTRUCTGLOBALS();

	if (!LuzInit((zvoid*)&G, lpUserFunc))
	{
		DESTROYGLOBALS();
		return PK_BADERR;
	}

	// Something has screwed up, we don't have a filename
	if (pDcl->lpszZipFN == NULL)
	{
		DESTROYGLOBALS();
		return PK_NOZIP;
	}

	LuzSetOpts((zvoid*)&G, pDcl);

	// Here is the actual call to "unzip" the files (or whatever else you
	// are doing.)
	retcode = LuzUnzip((zvoid*)&G, ifnc, ifnv, xfnc, xfnv);

	DESTROYGLOBALS();
	return retcode;
}


BOOL WINAPI LuzInit(zvoid* pG, LPUSERFUNCTIONS pUserFn)
{
	G.message = DllMessagePrint;
	G.statreportcb = LuzStatReportCB;
	G.pUserFn = pUserFn;

	if (!G.pUserFn->print || !G.pUserFn->replace)
		return FALSE;

	return TRUE;
}

int WINAPI LuzUnzip(zvoid* pG, int ifnc, char** ifnv, int xfnc, char** xfnv)
{
	int retcode, f_cnt;
#ifndef CRTL_CP_IS_ISO
	char **intern_ifv = NULL, **intern_xfv = NULL;
#endif

	if (ifnv == NULL && ifnc != 0)
		ifnc = 0;
	else
	{
		for (f_cnt = 0; f_cnt < ifnc; f_cnt++)
		{
			if (ifnv[f_cnt] == NULL)
			{
				ifnc = f_cnt;
				break;
			}
		}
	}

	if (xfnv == NULL && xfnc != 0)
		xfnc = 0;
	else
	{
		for (f_cnt = 0; f_cnt < xfnc; f_cnt++)
		{
			if (xfnv[f_cnt] == NULL)
			{
				xfnc = f_cnt;
				break;
			}
		}
	}

	G.process_all_files = (ifnc == 0 && xfnc == 0);         /* for speed */
	G.filespecs = ifnc;
	G.xfilespecs = xfnc;

	if (ifnc > 0)
	{
#ifdef CRTL_CP_IS_ISO
		G.pfnames = ifnv;
#else /* !CRTL_CP_IS_ISO */
		unsigned bufsize = 0;

		intern_ifv = (char**)malloc((ifnc + 1) * sizeof(char**));
		if (intern_ifv == NULL)
		{
			FreeDllMem(__G);
			return PK_BADERR;
		}

		for (f_cnt = ifnc; --f_cnt >= 0;)
			bufsize += lstrlen(ifnv[f_cnt]) + 1;

		intern_ifv[0] = (char*)malloc(bufsize);
		if (intern_ifv[0] == NULL)
		{
			free(intern_ifv);
			FreeDllMem(__G);
			return PK_BADERR;
		}

		for (f_cnt = 0; ; f_cnt++)
		{
			ISO_TO_INTERN(ifnv[f_cnt], intern_ifv[f_cnt]);
			if ((f_cnt+1) >= ifnc)
				break;
			intern_ifv[f_cnt+1] = intern_ifv[f_cnt] +
                              (lstrlen(intern_ifv[f_cnt]) + 1);
		}
		intern_ifv[ifnc] = (char *)NULL;
		G.pfnames = intern_ifv;
#endif /* ?CRTL_CP_IS_ISO */
	}

	if (xfnc > 0)
	{
#ifdef CRTL_CP_IS_ISO
		G.pxnames = xfnv;
#else /* !CRTL_CP_IS_ISO */
		unsigned bufsize = 0;

		intern_xfv = (char**)malloc((xfnc + 1) * sizeof(char**));
		if (intern_xfv == NULL)
		{
			if (ifnc > 0)
			{
				free(intern_ifv[0]);
				free(intern_ifv);
			}
			FreeDllMem(__G);
			return PK_BADERR;
		}

		for (f_cnt = xfnc; --f_cnt >= 0;)
			bufsize += lstrlen(xfnv[f_cnt]) + 1;
		intern_xfv[0] = (char*)malloc(bufsize);
		if (intern_xfv[0] == NULL)
		{
			free(intern_xfv);
			if (ifnc > 0)
			{
				free(intern_ifv[0]);
				free(intern_ifv);
			}
			FreeDllMem(__G);
			return PK_BADERR;
		}

		for (f_cnt = 0; ; f_cnt++)
		{
			ISO_TO_INTERN(xfnv[f_cnt], intern_xfv[f_cnt]);
			if ((f_cnt+1) >= xfnc)
				break;

			intern_xfv[f_cnt+1] = intern_xfv[f_cnt] +
                              (lstrlen(intern_xfv[f_cnt]) + 1);
		}
		intern_xfv[xfnc] = NULL;
		G.pxnames = intern_xfv;
#endif /* ?CRTL_CP_IS_ISO */
	}

/*---------------------------------------------------------------------------
    Okey dokey, we have everything we need to get started.  Let's roll.
  ---------------------------------------------------------------------------*/

	retcode = setjmp(dll_error_return);
	if (retcode)
	{
#ifndef CRTL_CP_IS_ISO
		if (xfnc > 0)
		{
			free(intern_xfv[0]);
			free(intern_xfv);
		}
		if (ifnc > 0)
		{
			free(intern_ifv[0]);
			free(intern_ifv);
		}
#endif
		FreeDllMem(__G);
		return PK_BADERR;
	}

	retcode = process_zipfiles(__G);
#ifndef CRTL_CP_IS_ISO
	if (xfnc > 0)
	{
		free(intern_xfv[0]);
		free(intern_xfv);
	}
	if (ifnc > 0)
	{
		free(intern_ifv[0]);
		free(intern_ifv);
	}
#endif
	FreeDllMem(__G);
	return retcode;
}

int win_fprintf(zvoid *pG, FILE *file, unsigned int size, char far *buffer)
{
	if ((file != stderr) && (file != stdout))
	{
		return write(fileno(file), (char*)(buffer),size);
	}
	return DllMessagePrint(pG, buffer, size, 0);
}

#define STDIO_BUF_SIZE 16384

int __cdecl printf_dll(const char *format, ...)
{
	va_list argptr;
	LPSTR pszBuffer;
	int len;
    GETGLOBALS();

	va_start(argptr, format);
	pszBuffer = (LPSTR)GlobalAllocPtr(GHND, STDIO_BUF_SIZE);
	if (!pszBuffer)
		return 0;
	len = wvsprintf(pszBuffer, format, argptr);
	va_end(argptr);
	len = win_fprintf(__G__ stdout, len, pszBuffer);
	GlobalFreePtr(pszBuffer);
	return len;
}

/* fprintf clone for code in zip.c, etc. */
int __cdecl fprintf_dll(FILE *fp, const char *format, ...)
{
	va_list argptr;
	LPSTR pszBuffer;
	int len;
    GETGLOBALS();

	va_start(argptr, format);
	pszBuffer = (LPSTR)GlobalAllocPtr(GHND, STDIO_BUF_SIZE);
	if (!pszBuffer)
		return 0;
	len = wvsprintf(pszBuffer, format, argptr);
	va_end(argptr);
	if (!fp || fp == stderr || fp == stdout)
		len = win_fprintf(__G__ fp, len, pszBuffer);
	else
		len = write(fileno(fp), pszBuffer, len);
	GlobalFreePtr(pszBuffer);
	return len;
}

void __cdecl perror(const char *parm1)
{
	printf(parm1);
}

/**********************************
 * Function DllMessagePrint()     *
 *                                *
 * Send messages to status window *
 **********************************/
static int UZ_EXP DllMessagePrint(zvoid* pG, uch* buf, ulg size, int flag)
{
/* @@@@
	if (!fNoPrinting)
	{
		if (G.pUserFn->nResultLength > (int)size)
		{
			lstrcpy(G.pUserFn->pResult, buf);
			G.pUserFn->pResult += size;
			G.pUserFn->nResultLength -= size;
		}
		return (int)size;
	}
	else
		return (int)size;
@@@@ */
	if (!fNoPrinting && G.pUserFn->print)
		return (*G.pUserFn->print)(G.pUserFn, buf, size);
	return (int)size;
}

#if 0 /* currently unused */
/********************************
 * Function DllDisplayPrint()   *
 *                              *
 * Send files to display window *
 ********************************/
int DllDisplayPrint(zvoid* pG, uch buf, ulg size, int flag)
{
	return DllMessagePrint(pG, buf, size);
}
#endif /* never */


/**********************************
 * Function UzpPassword()         *
 *                                *
 * Prompt for decryption password *
 **********************************/
int UZ_EXP UzpPassword(pG, rcnt, pwbuf, size, zfn, efn)
    zvoid *pG;          /* globals struct: always passed */
    int *rcnt;          /* retry counter */
    char *pwbuf;        /* buffer for password */
    int size;           /* size of password buffer */
    ZCONST char *zfn;   /* name of zip archiv */
    ZCONST char *efn;   /* name of archiv entry being processed */
{
	int res = IZ_PW_ERROR;	/* internal error, function should never get called */
#if CRYPT
	if (G.pUserFn->password)
	{
		if (*rcnt == 0)
		{
			// コマンドラインパスワード
			if (uO.pwdarg && *uO.pwdarg)
			{
				lstrcpy(pwbuf, uO.pwdarg);
				*rcnt = 3;
				return IZ_PW_ENTERED;
			}
			else if (gbTryLastPassword &&
					 rgGetString(sszStat, sszPass, pwbuf, 80) > 0)
			{
				// レジストリをチェックし、過去のパスワードを試みる
				*rcnt = 3;
			}
			else
			{
				*rcnt = 2;
				pwbuf[0] = '\0';
			}
		}
		else
		{
			(*rcnt)--;
		}
		res = (*G.pUserFn->password)((LPSTR)pwbuf, size, (LPSTR)efn);
	}
#endif /* ?CRYPT */
	return res;
}

void UZ_EXP UzpAcceptPassword(pG, pwbuf)
    zvoid *pG;          /* globals struct: always passed */
    char *pwbuf;        /* buffer for password */
{
	if (gbTryLastPassword)
	{
		FILE* fp;
		char *p, buf[MAX_PATH * 2];
		int len;

		// パスワードが記録されていないか、過去のパスワードと同じでないなら
		// 記録する
		len = rgGetString(sszStat, sszPass, buf, 80);
		if (len < 1 || lstrcmp(pwbuf, buf))
		{
			GetModuleFileName(hInst, buf, MAX_PATH * 2);
			p = (char*)_mbsrchr(buf, '.');
			lstrcpy(++p, "pwd");
			fp = fopen(buf, "a+");
			if (fp)
			{
				fprintf(fp, "%s\n", pwbuf);
				fclose(fp);
			}
		}

		rgSetString(sszStat, sszPass, pwbuf);
	}
}

//////////////////////////////////////////////////////////////////////
// Turn off all messages to the calling application 
//
void WINAPI LuzNoPrinting(int f)
{
	fNoPrinting = f;
}

//////////////////////////////////////////////////////////////////////
// Interface between WINDLL specific service callback functions and the
// generic DLL's "status report & user interception" callback
//
static int WINAPI LuzStatReportCB(zvoid *pG, int fnflag, ZCONST char *zfn,
                    ZCONST char *efn, ZCONST zvoid *details)
{
	int rval = UZ_ST_CONTINUE;
	if (G.pUserFn->report)
	{
		if ((*G.pUserFn->report)(G.pUserFn, fnflag, zfn, efn, (ULONG)details))
			rval = UZ_ST_BREAK;
	}
	return rval;
}


int WINAPI LuzUnzipToMemory(LPSTR zip, LPSTR file,
    LPUSERFUNCTIONS pUserFn, UzpBuffer *retstr)
{
    int r;
#ifndef CRTL_CP_IS_ISO
    char *intern_zip, *intern_file;
#endif

    CONSTRUCTGLOBALS();
#ifndef CRTL_CP_IS_ISO
    intern_zip = (char *)malloc(lstrlen(zip)+1);
    if (intern_zip == NULL)
	{
       DESTROYGLOBALS()
       return PK_MEM;
    }

    intern_file = (char*)malloc(lstrlen(file)+1);
    if (intern_file == NULL)
	{
       DESTROYGLOBALS()
       free(intern_zip);
       return PK_MEM;
    }
    ISO_TO_INTERN(zip, intern_zip);
    ISO_TO_INTERN(file, intern_file);
#   define zip intern_zip
#   define file intern_file
#endif
	if (!LuzInit((zvoid*)&G, pUserFn))
	{
       DESTROYGLOBALS();
       return PK_BADERR;
    }
    G.redirect_data = 1;

    r = (unzipToMemory(__G__ zip, file, retstr) == PK_COOL);

    DESTROYGLOBALS()
#ifndef CRTL_CP_IS_ISO
#  undef file
#  undef zip
    free(intern_file);
    free(intern_zip);
#endif
	if (!r && retstr->strlength)
	{
       free(retstr->strptr);
       retstr->strptr = NULL;
    }
    return r;
}


//////////////////////////////////////////////////////////////////////
// Purpose: Determine if file in archive contains the string szSearch
//
// Parameters:	archive = archive name
//				file    = file contained in the archive. This cannot be
//						  a wild card to be meaningful
//				pattern = string to search for
//				cmd     = 0 - case-insensitive search
//						  1 - case-sensitve search
//						  2 - case-insensitive, whole words only
//						  3 - case-sensitive, whole words only
//				SkipBin = if true, skip any files that have control
//						  characters other than CR, LF, or tab in the first
//						  100 characters.
//
// Returns:    TRUE if a match is found
//             FALSE if no match is found
//             -1 on error
//
// Comments: This does not pretend to be as useful as the standard
//           Unix grep, which returns the strings associated with a
//           particular pattern, nor does it search past the first
//           matching occurrence of the pattern.

int WINAPI LuzGrep(LPSTR archive, LPSTR file, LPSTR pattern, int cmd,
                   int SkipBin, LPUSERFUNCTIONS pUserFn)
{
    int retcode = FALSE, compare;
    ulg i, j, patternLen, buflen;
    char * sz, *p;
    UzpBuffer retstr;

    /* Turn off any windows printing functions, as they may not have been
     * identified yet. There is no requirement that we initialize the
     * dll with printing stuff for this. */
    LuzNoPrinting(TRUE);

    if (!LuzUnzipToMemory(archive, file, pUserFn, &retstr))
	{
		LuzNoPrinting(FALSE);
		return -1;   /* not enough memory, file not found, or other error */
    }

    if (SkipBin)
	{
        if (retstr.strlength < 100)
            buflen = retstr.strlength;
        else
            buflen = 100;
        for (i = 0; i < buflen; i++)
		{
            if (iscntrl(retstr.strptr[i]))
			{
                if ((retstr.strptr[i] != 0x0A) &&
                    (retstr.strptr[i] != 0x0D) &&
                    (retstr.strptr[i] != 0x09))
                {
                    /* OK, we now think we have a binary file of some sort */
                    free(retstr.strptr);
                    LuzNoPrinting(FALSE);
                    return FALSE;
                }
            }
        }
    }

    patternLen = lstrlen(pattern);

    if (retstr.strlength < patternLen)
	{
        free(retstr.strptr);
        LuzNoPrinting(FALSE);
        return FALSE;
    }

    sz = malloc(patternLen + 3); /* add two in case doing whole words only */
	if (cmd > 1)
	{
        lstrcpy(sz, " ");
        strcat(sz, pattern);
        strcat(sz, " ");
	}
	else
		lstrcpy(sz, pattern);

    if ((cmd == 0) || (cmd == 2))
	{
        for (i = 0; i < strlen(sz); i++)
            sz[i] = toupper(sz[i]);
        for (i = 0; i < retstr.strlength; i++)
            retstr.strptr[i] = toupper(retstr.strptr[i]);
	}

	for (i = 0; i < (retstr.strlength - patternLen); i++)
	{
        p = &retstr.strptr[i];
        compare = TRUE;
        for (j = 0; j < patternLen; j++)
		{
            /* We cannot do strncmp here, as we may be dealing with a
             * "binary" file, such as a word processing file, or perhaps
             * even a true executable of some sort. */
            if (p[j] != sz[j])
			{
                compare = FALSE;
                break;
            }
        }
        if (compare == TRUE)
		{
            retcode = TRUE;
            break;
        }
    }

    free(sz);
    free(retstr.strptr);
    LuzNoPrinting(FALSE); /* Turn printing back on */
    return retcode;
}

int WINAPI LuzValidate(LPSTR archive, int AllCodes)
{
    return UzpValidate((char *)archive, AllCodes);
}
