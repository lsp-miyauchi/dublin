/*
  Copyright (c) 1990-2009 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2009-Jan-02 or later
  (the contents of which are also included in unzip.h) for terms of use.
  If, for some reason, all these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
/*
   This is a very simplistic example of how to load and make a call into the
   dll. This has been compiled and tested for a 32-bit console version, but
   not under 16-bit windows. However, the #ifdef's have been left in for the
   16-bit code, simply as an example.

 */

#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <vector>
#include <memory>
#include <string>
#include <format>
#include <iostream>
#include "uzexampl.h"
#include "unzvers.h"
#include <winver.h>

#ifndef _MAX_PATH
#  define _MAX_PATH 260           /* max total file or directory name path */
#endif

constexpr auto UNZ_DLL_NAME = "LSPUNZIP.DLL";

constexpr auto DLL_WARNING =
  "Cannot find {}. The Dll must be in the application directory, the path, the Windows directory or the Windows System directory.";
constexpr auto DLL_VERSION_WARNING = 
  "{} has the wrong version number. Insure that you have the correct dll's installed, and that an older dll is not in your path or Windows System directory.";

/* Forward References */
int WINAPI DisplayBuf(LPSTR, unsigned long);
int WINAPI GetReplaceDlgRetVal(LPSTR, unsigned);
int WINAPI password(LPSTR, int, LPCSTR, LPCSTR);

ZCONST UzpVer * UZ_EXP UzpVersion  OF((void));
_DLL_UZVER pUzpVersion;
_DLL_UNZIP pfnSingleEntryUnzip;

static void FreeUpMemory(void);

int main(int argc, char **argv)
{
  std::string dllpath(_MAX_PATH, 0);

  if (argc < 2) {       /* We must have an archive to unzip */
     std::cout << "usage:\n    testdll <zipfile> [entry1 [entry2 [...]]] [-x xentry1 [...]\n";
     return 0;
  }

  DCL dcl = { 0 };
  USERFUNCTIONS uf = { 0 };
  uf.password = password;
  uf.print = DisplayBuf;
  uf.sound = nullptr;
  uf.replace = GetReplaceDlgRetVal;
  uf.SendApplicationMessage = ReceiveDllMessage;

  /* First we go look for the unzip dll */
  if (SearchPath(nullptr, UNZ_DLL_NAME, nullptr, _MAX_PATH, dllpath.data(), nullptr) == 0) {
     std::string str = std::format(DLL_WARNING, UNZ_DLL_NAME);
     std::cout << str << "\n";
     return -1;
  }

  /* Okay, now we know that the dll exists, and has the proper version
   * information in it. We can go ahead and load it.
   */
  HMODULE hdll = LoadLibrary(UNZ_DLL_NAME);
  if (hdll != nullptr) {
     pUzpVersion = (_DLL_UZVER)GetProcAddress(hdll, "UzpVersion");
     pfnSingleEntryUnzip = (_DLL_UNZIP)GetProcAddress(hdll, "LuzpSingleEntryUnzip");
  }
  else {
     std::string str = std::format("Could not load {}", UNZ_DLL_NAME);
     std::cout << str << "\n";
     return -1;
  }


  /*
     Here is where the actual extraction process begins. First we set up the
     flags to be passed into the dll.
   */
  dcl.StructVersID = UZ_DCL_STRUCTVER; /* version of this structure */
  dcl.ncflag = 0;              /* write to stdout if true */
  dcl.fQuiet = 0;              /* we want all messages
                                     1 = fewer messages,
                                     2 = no messages */
  dcl.ntflag = 0;              /* test zip file if true */
  dcl.nvflag = 1;              /* give a verbose listing if true */
  dcl.nzflag = 0;              /* display zip file comment if true */
  dcl.ndflag = 1;              /* recreate directories != 0,
                                     skip "../" if < 2 */
  dcl.naflag = 0;              /* do not convert CR to CRLF */
  dcl.nfflag = 0;              /* do not freshen existing files only */
  dcl.noflag = 0;              /* over-write all files if true */
  dcl.nZIflag = 0;             /* no ZipInfo output mode */
  dcl.B_flag = 0;              /* do not backup existing files */
  dcl.C_flag = 0;              /* do not match case-insensitive */
  dcl.D_flag = 0;              /* restore all timestamps */
  dcl.U_flag = 0;              /* do not disable UTF-8 support */
  dcl.ExtractOnlyNewer = 0;    /* do not extract only newer */
  dcl.SpaceToUnderscore = 0;   /* do not convert space to '_' in filenames */
  dcl.PromptToOverwrite = 0;   /* "overwrite all" selected -> no query mode */
  dcl.lpszZipFN = argv[1];     /* the archive name */
  dcl.lpszExtractDir = nullptr;   /* the directory to extract to.
                                     This is set to NULL if you are extracting
                                     to the current directory.
                                   */
  /*
     As this is a quite short example, intended primarily to show how to
     load and call in to the dll, the command-line parameters are only
     parsed in a very simplistic way:
     We assume that the command-line parameters after the zip archive
     make up a list of file patterns:
     " [file_i1] [file_i2] ... [file_iN] [-x file_x1 [file_x2] ...]".
     We scan for an argument "-x"; all arguments in front are
     "include file patterns", all arguments after are "exclude file patterns".
     If no more arguments are given, we extract ALL files.

     In summary, the example program should be run like:
     example <archive.name> [files to include] [-x files to exclude]
     ("<...> denotes mandatory arguments, "[...]" optional arguments)
   */
  int exfc = 0;
  int infc = 0;
  char** exfv = nullptr;
  char** infv = nullptr;
  char* x_opt = nullptr;
  if (argc > 2) {
      infv = &argv[2];
      for (infc = 0; infc < argc - 2; infc++) {
          if (!strcmp("-x", infv[infc])) {
              x_opt = infv[infc];
              infv[infc] = nullptr;
              break;
          }
      }
      exfc = argc - infc - 3;
      if (exfc > 0)
          exfv = &argv[infc+3];
      else {
          exfc = 0;
          exfv = nullptr;
      }
  }
  int res = (*pfnSingleEntryUnzip)(infc, infv, exfc, exfv, &dcl, &uf);
  if (x_opt) {
      infv[infc] = x_opt;
      x_opt = nullptr;
  }

  if (res != 0) {
      std::string str = std::format("Error unzipping (error/warning code {})...", res);
      std::cout << str << "\n";
  }
  FreeLibrary(hdll);
  return res;
}

int WINAPI GetReplaceDlgRetVal(LPSTR filename, unsigned fnbufsiz)
{
  /* This is where you will decide if you want to replace, rename etc existing files. */
  return 1;
}

/* This is a very stripped down version of what is done in Wiz. Essentially
    what this function is for is to do a listing of an archive contents. It
    is actually never called in this example, but a dummy procedure had to
    be put in, so this was used.
  */
#ifdef Z_UINT8_DEFINED
void WINAPI ReceiveDllMessage(z_uint8 ucsize, z_uint8 csiz,
    unsigned numfactor, unsigned mo, unsigned dy, unsigned yr,
    unsigned hh, unsigned mm, char c, LPCSTR filename,
    LPCSTR method, unsigned long crc, char fCrypt)
#else
void WINAPI ReceiveDllMessage(unsigned long ucsize, unsigned long csiz,
    unsigned numfactor,
    unsigned mo, unsigned dy, unsigned yr, unsigned hh, unsigned mm,
    char c, LPCSTR filename, LPCSTR method, unsigned long crc, char fCrypt)
#endif
{
  std::string sgn;
  if (csiz > ucsize)
    sgn = '-';
  else
    sgn = ' ';
  std::string factor = std::format("{}{}%", sgn, numfactor);
  std::string str = std::format("{:7}  {:7} {:4} {:8} {:02}-{:02}-{:02}  {:02}:{:02}  {}{}", ucsize, csiz, factor, method, mo, dy, yr, hh, mm, c, filename);
  std::cout << str << "\n";
}

/* Password entry routine - see password.c in the wiz directory for how
   this is actually implemented in WiZ. If you have an encrypted file,
   this will probably give you great pain.
 */
int WINAPI password(LPSTR p, int n, LPCSTR m, LPCSTR name)
{
  return 1;
}

/* Dummy "print" routine that simply outputs what is sent from the dll */
int WINAPI DisplayBuf(LPSTR buf, unsigned long size)
{
  std::cout << buf;
  return (int)(unsigned int) size;
}
