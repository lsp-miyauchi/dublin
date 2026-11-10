/*
  Copyright (c) 1990-2009 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2009-Jan-02 or later
  (the contents of which are also included in unzip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
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

#define __WINDLL_C      /* identifies this source module */

#define WIN32_LEAN_AND_MEAN
#define UNZIP_INTERNAL
#include <iostream>
#include <string>
#include <format>
#include <malloc.h>
#include <cctype>
#include <algorithm>
#include <ranges>
#include "../unzip.h"
#include "../crypt.h"
#include "../unzvers.h"
#include "../windll/windll.h"
#include "../windll/structs.h"
#include "../consts.h"

/* Added type casts to prevent potential "type mismatch" error messages. */
#ifdef REENTRANT
#  undef __G
#  undef __G__
#  define __G                 (Uz_Globs *)pG
#  define __G__               (Uz_Globs *)pG,
#endif

HANDLE hwildZipFN;
HMODULE hUzInstance;               /* current instance */
BOOL gbTryLastPassword;
static char sszStat[] = "Status";
static char sszPass[] = "Password";
static int fNoPrinting = 0;

extern "C" jmp_buf dll_error_return;


/*
 * Helper function to convert a string to uppercase
 */
std::string toupper_string(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
  return s;
}

#ifndef UNZIPLIB
/*  DLL Entry Point */
/*  Windows 95/NT entry point for DLLs.  This is called when the DLL is
 *  loaded and unloaded, and when threads are created and destroyed.
 */
BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID plvReserved)
{
  BOOL rc = TRUE;
  switch( dwReason ) {
    case DLL_PROCESS_ATTACH:
      // DLL is loaded. Do your initialization here.
      // If cannot init, set rc to FALSE.
      hUzInstance = hInstance;
      break;
    case DLL_PROCESS_DETACH:
      // DLL is unloaded. Do your cleanup here.
      break;
    default:
      break;
   }
   return rc;
}

/**
 *  Windows 3.1/95/NT entry point for DLLs.  This is called when the DLL is
 *  loaded and unloaded, and when threads are created and destroyed.
 */
int FAR PASCAL WEP(int /*bSystemExit*/)
{
   return 1;
}
#endif /* !UNZIPLIB */

static void UnzipFreeArguments(unsigned int count, char*** pargVee)
{
  /* Free the arguments in the arrays */
  if (pargVee == nullptr || *pargVee == nullptr)
    return;

  auto parray = *pargVee;
  while (count-- > 0) {
    if (*parray != nullptr) {
      free(*parray);
      *parray = nullptr;
    }
    ++parray;
  }
  /* Then free the arrays themselves */
  free(*pargVee);
}

/**/
static int UnzipAllocMemory(unsigned int i, const char* cmd, char*** pargVee, unsigned int* pargCee, const char* error)
{
  int cmdlen = (int)strlen(cmd) + 1;
  auto ptr = (char*)malloc(sizeof(char) * cmdlen + 1);
  if (ptr == nullptr) {
    UnzipFreeArguments(*pargCee, pargVee);
    std::cout << "Unable to allocate memory in unzip library at " << error << '\n';
    return PK_MEM;
  }
  lstrcpyn(ptr, cmd, cmdlen);
  (*pargVee)[i] = ptr;
  (*pargCee)++;
  return PK_OK;
}

/**/
static int UnzipParseString(LPCSTR s, unsigned int* pargCee, char*** pargVee)
{
  auto str1 = (char*)malloc(lstrlen(s) + 4);
  lstrcpy(str1, s);
  lstrcat(str1, " @");

  auto str2 = strchr(str1, '\"'); // get first occurance of double quote
  auto str3 = (char*)nullptr;
  while ((str3 = strchr(str1, '\t')) != nullptr) {
      str3[0] = ' '; // Change tabs into a single space
  }

  /* Note that if a quoted string contains multiple adjacent spaces, they
      will not be removed, because they could well point to a valid
      folder/file name.
    */
  unsigned int i = 0;
  size_t size;
  while ((str2 = strchr(str1, '\"')) != nullptr) {
      // Found an opening double quote; get the corresponding closing quote
      str3 = strchr(str2 + 1, '\"');
      if (str3 == nullptr) {
          free(str1);
          return PK_PARAM; /* Something is screwy with the string, bail out */
      }
      str3[0] = '\0';  // terminate str2 with a NULL

      size = _msize(*pargVee);
      if ((*pargVee = (char**)realloc(*pargVee, size + sizeof(char *))) == nullptr) {
          std::cout << "Unable to allocate memory in unzip dll\n";
          return PK_MEM;
      }
      // argCee is incremented in UnzipAllocMemory
      if (UnzipAllocMemory(i, str2 + 1, pargVee, pargCee, "Creating file list from string") != PK_OK) {
          free(str1);
          return PK_MEM;
      }
      i++;
      str3 += 2;  // Point past the whitespace character
      str2[0] = '\0'; // Terminate str1
      lstrcat(str1, str3);
  }    // end while

  /* points to first occurance of a space */
  str2 = strchr(str1, ' ');

  /*  Go through the string character by character, looking for instances
      of two spaces together. Terminate when you find the trailing @
  */
  while ((str2[0] != '\0') && (str2[0] != '@')) {
      while ((str2[0] == ' ') && (str2[1] == ' ')) {
          str3 = &str2[1];
          str2[0] = '\0';
          lstrcat(str1, str3);
      }
      str2++;
  }

  /* Do we still have a leading space? */
  if (str1[0] == ' ') {
      str3 = &str1[1];
      lstrcpy(str1, str3); // Dump the leading space
  }


  /* Okay, now we have gotten rid of any tabs and replaced them with
      spaces, and have replaced multiple spaces with a single space. We
      couldn't do this before because the folder names could have actually
      contained these characters.
  */

  str2 = str3 = str1;

  while ((str2[0] != '\0') && (str3[0] != '@')) {
    str3 = strchr(str2+1, ' ');
    str3[0] = '\0';
    size = _msize(pargVee);
    if ((*pargVee = (char **)realloc(*pargVee, size + sizeof(char*))) == nullptr) {
      std::cout << "Unable to allocate memory in unzip dll\n";
      return PK_MEM;
    }
    if (UnzipAllocMemory(i, str2, pargVee, pargCee, "Creating file list from string") != PK_OK) {
      free(str1);
      return PK_MEM;
    }
    i++;
    str3++;
    str2 = str3;
  }
  free(str1);
  return PK_OK;
}

/* DLL calls */

/*
    StructVersID      = version of this structure (= UZ_DCL_STRUCTVER)
    ExtractOnlyNewer  = TRUE for "update" without interaction
                        (extract only newer/new files, without queries)
    SpaceToUnderscore = TRUE if convert space to underscore
    PromptToOverwrite = TRUE if prompt to overwrite is wanted
    fQuiet            = quiet flag:
                         0 = all messages, 1 = few messages, 2 = no messages
    ncflag            = write to stdout if TRUE
    ntflag            = test zip file
    nvflag            = verbose listing
    nfflag            = "freshen" (replace existing files by newer versions)
    nzflag            = display zip file comment
    ndflag            = controls (sub)directory recreation during extraction
                        0 = junk paths from filenames
                        1 = "safe" usage of paths in filenames (skip "../")
                        2 = allow also unsafe path components (dir traversal)
    noflag            = always overwriting existing files if TRUE
    naflag            = do end-of-line translation
    nZIflag           = get ZipInfo if TRUE
    B_flag            = backup existing files if TRUE
    C_flag            = be case insensitive if TRUE
    D_flag            = controls restoration of timestamps
                        0 = restore all timestamps (default)
                        1 = skip restoration of timestamps for folders
                            created on behalf of directory entries in the
                            Zip archive
                        2 = do not restore any timestamps; extracted files
                            and directories get stamped with the current time
    U_flag            = controls UTF-8 filename coding support
                        0 = automatic UTF-8 translation enabled (default)
                        1 = recognize UTF-8 coded names, but all non-ASCII
                            characters are "escaped" into "#Uxxxx"
                        2 = UTF-8 support is disabled, filename handling
                            works exactly as in previous UnZip versions
    fPrivilege        = 1 => restore ACLs in user mode,
                        2 => try to use privileges for restoring ACLs
    lpszZipFN         = zip file name
    lpszExtractDir    = directory to extract to. This should be NULL if you
                        are extracting to the current directory.
 */

BOOL WINAPI LuzpSetOpts(zvoid *pG, LPDCL lpDCL)
{
    if (lpDCL->StructVersID != UZ_DCL_STRUCTVER)
        return FALSE;

    uO.qflag = lpDCL->fQuiet;  /* Quiet flag */
    G.pfnames = (char **)&fnames[0];    /* assign default file name vector */
    G.pxnames = (char **)&fnames[1];

    uO.jflag = (lpDCL->ndflag == 0);
    uO.ddotflag = (lpDCL->ndflag >= 2);
    uO.cflag = lpDCL->ncflag;
    uO.tflag = lpDCL->ntflag;
    uO.vflag = lpDCL->nvflag;
    uO.zflag = lpDCL->nzflag;
    uO.aflag = lpDCL->naflag;
#ifdef UNIXBACKUP
    uO.B_flag = lpDCL->B_flag;
#endif
    uO.C_flag = lpDCL->C_flag;
    uO.D_flag = lpDCL->D_flag;
    uO.U_flag = lpDCL->U_flag;
    uO.overwrite_all = lpDCL->noflag;
    uO.overwrite_none = !(lpDCL->noflag || lpDCL->PromptToOverwrite);
    uO.uflag = lpDCL->ExtractOnlyNewer || lpDCL->nfflag;
    uO.fflag = lpDCL->nfflag;
    uO.X_flag = lpDCL->fPrivilege;
    uO.sflag = lpDCL->SpaceToUnderscore; /* Translate spaces to underscores? */
    if (lpDCL->nZIflag) {
      uO.zipinfo_mode = TRUE;
      uO.hflag = TRUE;
      uO.lflag = 10;
      uO.qflag = 2;
    }
    else {
      uO.zipinfo_mode = FALSE;
    }
    G.extract_flag = (!uO.zipinfo_mode &&
                      !uO.cflag && !uO.tflag && !uO.vflag && !uO.zflag
                      && !uO.T_flag
                     );

    if (lstrlen(lpDCL->extractDir) > 0 && G.extract_flag) {
#define pExDirRoot lpDCL->extractDir
       if (lstrlen(pExDirRoot) >= FILNAMSIZ) {
           /* The supplied extract root path exceed the filename size limit. */
           return FALSE;
       }
       uO.exdir = pExDirRoot;
    }
    else {
       uO.exdir = (char *)NULL;
    }

/* G.wildzipfn needs to be initialized so that do_wild does not wind
   up clearing out the zip file name when it returns in process.c
*/
    if (strlen(lpDCL->zipfile) >= FILNAMSIZ)
       /* length of supplied archive name exceed the system's filename limit */
       return FALSE;

    hwildZipFN = GlobalAlloc(GPTR, FILNAMSIZ);
    if (hwildZipFN == nullptr)
       return FALSE;

    G.wildzipfn = (char*)GlobalLock(hwildZipFN);
    lstrcpy(G.wildzipfn, lpDCL->zipfile);
    _ISO_INTERN(G.wildzipfn);

    return TRUE;    /* set up was OK */
}

/** */
static void FreeDllMem(__GPRO)
{
    if (G.wildzipfn) {
        GlobalUnlock(hwildZipFN);
        G.wildzipfn = nullptr;
    }
    if (hwildZipFN)
        hwildZipFN = GlobalFree(hwildZipFN);

    uO.zipinfo_mode = FALSE;
}

/** */
int WINAPI LuzpUnzip(zvoid* pG, int ifnc, char** ifnv, int xfnc, char** xfnv) 
{
    int f_cnt = 0;
    if (ifnv == nullptr && ifnc != 0)
      ifnc = 0;
    else {
      for (f_cnt = 0; f_cnt < ifnc; f_cnt++) {
        if (ifnv[f_cnt] == nullptr) {
          ifnc = f_cnt;
          break;
        }
      }
    }
    if (xfnv == nullptr && xfnc != 0)
      xfnc = 0;
    else {
      for (f_cnt = 0; f_cnt < xfnc; f_cnt++) {
        if (xfnv[f_cnt] == nullptr) {
          xfnc = f_cnt;
          break;
        }
      }
    }

    G.process_all_files = (ifnc == 0 && xfnc == 0);         /* for speed */
    G.filespecs = ifnc;
    G.xfilespecs = xfnc;

    if (ifnc > 0) {
      for (f_cnt = ifnc; --f_cnt >= 0;) {
        if (strlen(ifnv[f_cnt]) > ((WSIZE >> 2) - 160)) {
          /* include filename pattern is too long for internal buffers */
          FreeDllMem(__G);
          return PK_PARAM;
        }
      }
      G.pfnames = ifnv;
    }

    if (xfnc > 0) {
      for (f_cnt = xfnc; --f_cnt >= 0;) {
        if (strlen(xfnv[f_cnt]) > ((WSIZE >> 2) - 160)) {
          /* exclude filename pattern is too long for internal buffers */
          FreeDllMem(__G);
          return PK_PARAM;
        }
      }
      G.pxnames = xfnv;
    }

    /*---------------------------------------------------------------------------
     Okey dokey, we have everything we need to get started.  Let's roll.
    ---------------------------------------------------------------------------*/

    int retcode = setjmp(dll_error_return);
    if (retcode) {
      FreeDllMem(__G);
      return PK_BADERR;
    }
    retcode = process_zipfiles(__G);
    FreeDllMem(__G);
    return retcode;
}


/*
ifnc       = number of file names being passed. If all files are to be
             extracted, then this can be zero.
ifnv       = file names to be unarchived. Wildcard patterns are recognized
             and expanded. If all files are to be extracted, then this can
             be NULL.
xfnc       = number of "file names to be excluded from processing" being
             passed. If all files are to be extracted, set this to zero.
xfnv       = file names to be excluded from the unarchiving process. Wildcard
             characters are allowed and expanded. If all files are to be
             extracted, set this argument to NULL.
lpUserFunc = pointer to a structure that contains pointers to functions
             in the calling application, as well as sizes passed back to
             the calling application etc.
*/
int WINAPI LuzpSingleEntryUnzip(int ifnc, char** ifnv, int xfnc, char** xfnv, LPUSERFUNCTIONS lpUserFunc)
{
   int retcode;
   CONSTRUCTGLOBALS();
   
   if (!LuzpInit((zvoid *)&G, lpUserFunc)) {
      DESTROYGLOBALS();
      return PK_BADERR;
   }

   LPDCL pDcl = lpUserFunc->pDcl;
   if (pDcl == NULL) {
      DESTROYGLOBALS();
      return PK_BADERR;
   }
   if (pDcl->zipfile[0] == '\0') {
      /* Something has screwed up, we don't have a filename */
      DESTROYGLOBALS();
      return PK_NOZIP;
   }

   if (!LuzpSetOpts((zvoid*)&G, pDcl)) {
      DESTROYGLOBALS();
      return PK_MEM;
   }

#ifdef SFX
   G.zipfn = pDcl->lpszZipFN;
   G.argv0 = pDcl->lpszZipFN;
#endif

   /* Here is the actual call to "unzip" the files (or whatever else you
    * are doing.)
    */
   retcode = LuzpUnzip((zvoid*)&G, ifnc, ifnv, xfnc, xfnv);

   DESTROYGLOBALS();
   return retcode;
}


/*
This calling wrapper provided a method to pass file lists as plain strings
instead of the usual string arrays. For VB Users only...
ifnc       = number of file names being passed. If all files are to be
             extracted, then this can be zero.
ExtList    = Pointer to a list of file names to be extracted, separated by
             white space. If all files are to be extracted, then this should
             be NULL. Parameter ifnc should have an accurate count of the
             number of filenames being passed in.
xfnc       = number of "file names to be excluded from processing" being
             passed. If all files are to be extracted, set this to zero.
ExcList    = Pointer to a list of file names to be excluded from processing.
             Parameter xfnc should have an accurate count of the number of
             the number of filenames being passed in.
lpDCL      = pointer to a structure with the flags for setting the
             various options, as well as the zip file name.
lpUserFunc = pointer to a structure that contains pointers to functions
             in the calling application, as well as sizes passed back to
             the calling application etc.
*/

int WINAPI LuzpSingleEntryUnzpList(unsigned int ifnc, LPCSTR ExtList, unsigned int xfnc, LPCSTR ExcList, LPUSERFUNCTIONS lpUserFunc)
{
    int retcode;
    char **argVExt, **argVExc;
    unsigned int argCExt, argCExc;

    argCExt = argCExc = 0;

    if (ExtList != nullptr) {
        if ((argVExt = (char **)malloc((ifnc)*sizeof(char *))) == nullptr) {
            std::cout << "Unable to allocate memory in unzip dll\n";
            return PK_MEM;
        }
        if ((retcode = UnzipParseString(ExtList, &argCExt, &argVExt)) != PK_OK)
            return retcode;
    }

    if (ExcList != nullptr) {
        if ((argVExc = (char **)malloc((ifnc)*sizeof(char *))) == nullptr) {
            std::cout << "Unable to allocate memory in unzip dll\n";
            UnzipFreeArguments(argCExt, &argVExt);
            return PK_MEM;
        }
        if ((retcode = UnzipParseString(ExcList, &argCExc, &argVExc)) != PK_OK) {
            UnzipFreeArguments(argCExt, &argVExt);
            return retcode;
        }
    }

    retcode = LuzpSingleEntryUnzip(argCExt, argVExt, argCExc, argVExc, lpUserFunc);
    UnzipFreeArguments(argCExc, &argVExc);
    UnzipFreeArguments(argCExt, &argVExt);
    return retcode;
}

extern "C" int win_fprintf(zvoid* pG, FILE* file, unsigned int size, const char* buffer)
{
   if ((file != stderr) && (file != stdout)) {
      return _write(_fileno(file), buffer, size);
   }
   if (!fNoPrinting)
      return (*G.lpUserFunctions->print)(G.lpUserFunctions, buffer, size);
   return (int)size;
}

/**********************************
 * Function DllMessagePrint()     *
 *                                *
 * Send messages to status window *
 **********************************/
static int UZ_EXP DllMessagePrint(zvoid* pG, uch* buf, ulg size, int flag)
{
  if (!fNoPrinting)
    return (*G.lpUserFunctions->print)(G.lpUserFunctions, (LPSTR)buf, size);
  return (int)size;
}

/**********************************
 * Function UzpPassword()         *
 *                                *
 * Prompt for decryption password *
 **********************************/
int UZ_EXP UzpPassword(zvoid* pG, int* rcnt, char* pwbuf, int pwbuf_size, ZCONST char*, ZCONST char* efn)
{
  int res = IZ_PW_ERROR;	/* internal error, function should never get called */
#if CRYPT
  if (G.lpUserFunctions->password) {
    if (*rcnt == 0) {
      // コマンドラインパスワード
      if (uO.pwdarg && *uO.pwdarg) {
        lstrcpyn(pwbuf, uO.pwdarg, pwbuf_size);
        *rcnt = 3;
        return IZ_PW_ENTERED;
      }
      else if (gbTryLastPassword && rgGetString(sszStat, sszPass, pwbuf, pwbuf_size) > 0) {
        // レジストリをチェックし、過去のパスワードを試みる
        *rcnt = 3;
      }
      else {
        *rcnt = 2;
        pwbuf[0] = '\0';
      }
    }
    else {
      (*rcnt)--;
    }
    res = (*G.lpUserFunctions->password)(G.lpUserFunctions, pwbuf, pwbuf_size, (LPCSTR)efn);
  }
#endif /* ?CRYPT */
  return res;
} /* end function UzpPassword() */

/* Turn off all messages to the calling application */
void WINAPI LuzpNoPrinting(int f)
{
    fNoPrinting = f;
}

/* Dummy sound function for those applications that don't use sound */
static void WINAPI DummySound(void)
{
}

/* Interface between WINDLL specific service callback functions and the
   generic DLL's "status report & user interception" callback */
static int WINAPI LuzpStatReportCB(zvoid* pG, int fnflag, ZCONST char *zfn, ZCONST char *efn, ZCONST zvoid* details)
{
    int rval = UZ_ST_CONTINUE;

    switch (fnflag) {
      case UZ_ST_START_EXTRACT:
        if (G.lpUserFunctions->sound != NULL)
            (*G.lpUserFunctions->sound)();
        break;
      case UZ_ST_FINISH_MEMBER:
        if ((G.lpUserFunctions->ServCallBk != NULL) &&
            (*G.lpUserFunctions->ServCallBk)(efn,
                                             (details == NULL ? 0L :
#ifdef Z_UINT8_DEFINED
                                              (z_uint8)(*((zusz_t *)details))
#else
                                              *((zusz_t *)details)
#endif
                                             )))
            rval = UZ_ST_BREAK;
        else if (G.lpUserFunctions->ServCallBk_i32 != NULL) {
             unsigned long siz_u4L = 0L;
             unsigned long siz_u4H = 0L;
             if (details != NULL) {
                 siz_u4L = (unsigned long)(*(zusz_t *)details);
#ifdef ZIP64_SUPPORT
                 siz_u4H = (unsigned long)((*(zusz_t *)details) >> 32);
#endif
             }
             if ((*G.lpUserFunctions->ServCallBk_i32)(efn, siz_u4L, siz_u4H))
                 rval = UZ_ST_BREAK;
        }
        break;
      case UZ_ST_IN_PROGRESS:
        break;
      default:
        break;
    }
    return rval;
}


#ifndef SFX

/* Purpose: Unzip a single file from a zip archive into memory
   Parameters: zip     = zip file name
               file    = file contained in the zip archive to be unzipped
               lpUserFunctions = pointer to a structure that contains pointers
                                 to functions in the calling application, as
                                 well as sizes passed back to the calling
                                 application etc.
               retstr  = pointer to a structure that will contain the unzipped
                         data and its length.
   Returns:    TRUE if successful, FALSE if not successful
   Comments: This function is provided for those applications that want to
             unzip a single file from a zip archive into memory. It is not
             intended to be a replacement for the unzip command line utility,
             which has more options.
 */
int WINAPI LuzpUnzipToMemory(LPSTR zip, LPSTR file, LPUSERFUNCTIONS lpUserFunctions, UzpBuffer *retstr)
{
    int r;

    CONSTRUCTGLOBALS();
    if (!LuzpInit((zvoid *)&G, lpUserFunctions)) {
       DESTROYGLOBALS();
       return PK_BADERR;
    }
    G.redirect_data = 1;

    r = (unzipToMemory(__G__ zip, file, retstr) == PK_COOL);

    DESTROYGLOBALS();
    if (!r && retstr->strlength) {
       free(retstr->strptr);
       retstr->strlength = 0;
       retstr->strptr = nullptr;
    }
    return r;
}


/**/
class PrintControl {
public:
  PrintControl() { LuzpNoPrinting(TRUE); }
  ~PrintControl() { LuzpNoPrinting(FALSE); }
};

/* Purpose: Determine if file in archive contains the string szSearch

   Parameters: archive  = archive name
               file     = file contained in the archive. This cannot be
                          a wildcard to be meaningful
               pattern  = string to search for
               cmd      = 0 - case-insensitive search
                          1 - case-sensitve search
                          2 - case-insensitive, whole words only
                          3 - case-sensitive, whole words only
               SkipBin  = if true, skip any files that have control
                          characters other than CR, LF, or tab in the first
                          100 characters.

   Returns:    TRUE if a match is found
               FALSE if no match is found
               -1 on error

   Comments: This does not pretend to be as useful as the standard
             Unix grep, which returns the strings associated with a
             particular pattern, nor does it search past the first
             matching occurrence of the pattern.
 */
int WINAPI LuzpGrep(LPSTR archive, LPSTR file, LPSTR pattern, int cmd, int SkipBin, LPUSERFUNCTIONS lpUserFunctions)
{

    // new を使わずに std::make_unique で生成する
    auto pcptr = std::make_unique<PrintControl>();

    /* Turn off any windows printing functions, as they may not have been
     * identified yet. There is no requirement that we initialize the
     * dll with printing stuff for this. */

    UzpBuffer retstr;
    if (!LuzpUnzipToMemory(archive, file, lpUserFunctions, &retstr)) {
       return -1;   /* not enough memory, file not found, or other error */
    }

    ulg i;
    ulg buflen;
    if (SkipBin) {
      buflen = (retstr.strlength < 100) ? retstr.strlength : 100;
      std::string s = retstr.strptr;
      for (unsigned char c : s | std::views::take(buflen)) {
        if (std::iscntrl(c) && c != '\n' && c != '\r' && c != '\t') {
          /* OK, we now think we have a binary file of some sort */
          free(retstr.strptr);
          return FALSE;
        }
      }
    }

    ulg patternLen = (int)strlen(pattern);
    if (retstr.strlength < patternLen) {
        free(retstr.strptr);
        return FALSE;
    }

    std::string str = (cmd > 1) ? std::format(" {} ", pattern) : pattern;
    if ((cmd == 0) || (cmd == 2)) {
      str = toupper_string(str);
    }

    int retcode = FALSE;
    for (i = 0; i < (retstr.strlength - patternLen); i++) {
      auto p = (const char*)&retstr.strptr[i];
      auto q = (const char*)str.c_str();
      bool compare = true;
      for (ulg j = 0; j < patternLen; j++) {
        /* We cannot do strncmp here, as we may be dealing with a
         * "binary" file, such as a word processing file, or perhaps
         * even a true executable of some sort.
         */
        if (*p++ != *q++) {
          compare = false;
          break;
        }
      }
      if (compare == true) {
        retcode = TRUE;
        break;
      }
    }

    free(retstr.strptr);
    return retcode;
}

/* Purpose: Validate the integrity of a zip file
   Parameters: archive  = archive name
               AllCodes = if TRUE, return all error codes, otherwise
                          return only the first error code found.
   Returns:    0 if no errors are found
               1 if errors are found and AllCodes is FALSE
               >1 if errors are found and AllCodes is TRUE
   Comments: This function is provided for those applications that want to
             validate a zip file without actually extracting any files.
             It is not intended to be a replacement for the unzip command
             line utility, which has more options and capabilities.
 */
int WINAPI LuzpValidate(LPCSTR archive, int AllCodes)
{
    return UzpValidate(archive, AllCodes);
}

BOOL WINAPI LuzpInit(zvoid* pG, LPUSERFUNCTIONS lpUserFunc)
{
  G.message = DllMessagePrint;
  G.statreportcb = LuzpStatReportCB;
  if (lpUserFunc->sound == nullptr)
    lpUserFunc->sound = DummySound;
  G.lpUserFunctions = lpUserFunc;

  SETLOCALE(LC_CTYPE, "");

  if (!G.lpUserFunctions->print)
    std::cout << "Warning: No print callback provided.\n";
  if (!G.lpUserFunctions->sound)
    std::cout << "Warning: No sound callback provided.\n";
  if (!G.lpUserFunctions->replace)
    std::cout << "Warning: No replace callback provided.\n";

  if (!G.lpUserFunctions->print ||
    !G.lpUserFunctions->sound ||
    !G.lpUserFunctions->replace)
    return FALSE;

  return TRUE;
}

#endif /* !SFX */
