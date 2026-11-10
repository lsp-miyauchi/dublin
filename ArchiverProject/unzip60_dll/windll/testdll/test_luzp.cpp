#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <format>
#include <array>
#include "uzexampl.h"
#include "../../unzvers.h"

extern LONG64 muldiv64(LONG64 a, LONG64 b, LONG64 c);
extern void zipfileInfo(ULONGLONG ucsize, ULONGLONG csiz, const SYSTEMTIME* pst, const char* attr, LPCSTR filename, LPCSTR methbuf);
extern void attrib2string(unsigned attrib, char* pattr, size_t len);

extern HINSTANCE hUnzipDll;

static ZCONST UzpVer* UZ_EXP UzpVersion  OF((void));
static _DLL_UZVER pUzpVersion;
static _DLL_UNZIP pSingleEntryUnzip;

/* Password entry routine - see password.c in the wiz directory for how
    this is actually implemented in WiZ. If you have an encrypted file,
    this will probably give you great pain.
  */
static int WINAPI password(void* pInfo, LPSTR p, int n, LPCSTR name)
{
  return 1;
}

/* Dummy "print" routine that simply outputs what is sent from the dll */
static int WINAPI PrintMessage(void* pInfo, LPCSTR lpMessage, unsigned long size)
{
  std::cout << lpMessage << '\n';
  return (int)(unsigned int)size;
}

/**
 * QueryReplaceFile() - This is where you will decide if you want to replace, rename etc existing
 * files. This is a very simple example that always returns 1, which means "replace
 * the file". You can implement your own logic here to prompt the user or check for conditions.
 */
static int WINAPI QueryReplaceFile(void* pInfo, const char* filename)
{
  /* This is where you will decide if you want to replace, rename etc existing
     files.
   */
  return 1;
}

/* This is a very stripped down version of what is done in Wiz. Essentially
   what this function is for is to do a listing of an archive contents. It
   is actually never called in this example, but a dummy procedure had to
   be put in, so this was used.
 */
static void WINAPI ProcessMessage(void* pInfo, const PACKED_FILEITEM* pzi)
{
  char sattr[8];
  attrib2string(pzi->attributs, sattr, sizeof(sattr));
  zipfileInfo(pzi->originalSize, pzi->compressedSize, &pzi->fstime, sattr, pzi->filename, pzi->method);
}


/**/
int luzpSession(int argc, char** argv)
{
  auto dclptr = std::make_unique<DCL>();
  LPDCL pDcl = dclptr.get();
  auto userfuncptr = std::make_unique<USERFUNCTIONS>();
  LPUSERFUNCTIONS pUserFunc = userfuncptr.get();

  pSingleEntryUnzip = (_DLL_UNZIP)GetProcAddress(hUnzipDll, "LuzpSingleEntryUnzip");

  pUserFunc->pDcl = pDcl;
  pUserFunc->password = password;
  pUserFunc->print = PrintMessage;
  pUserFunc->replace = QueryReplaceFile;
  pUserFunc->SendApplicationMessage64 = ProcessMessage;

  /*
     Here is where the actual extraction process begins. First we set up the
     flags to be passed into the dll.
   */
  pDcl->StructVersID = UZ_DCL_STRUCTVER; /* version of this structure */
  pDcl->ncflag = 0;              /* write to stdout if true */
  pDcl->fQuiet = 0;              /* we want all messages
                                     1 = fewer messages,
                                     2 = no messages */
  pDcl->ntflag = 0;              /* test zip file if true */
  pDcl->nvflag = 1;              /* give a verbose listing if true */
  pDcl->nzflag = 0;              /* display zip file comment if true */
  pDcl->ndflag = 1;              /* recreate directories != 0,
                                     skip "../" if < 2 */
  pDcl->naflag = 0;              /* do not convert CR to CRLF */
  pDcl->nfflag = 0;              /* do not freshen existing files only */
  pDcl->noflag = 0;              /* over-write all files if true */
  pDcl->nZIflag = 0;             /* no ZipInfo output mode */
  pDcl->B_flag = 0;              /* do not backup existing files */
  pDcl->C_flag = 0;              /* do not match case-insensitive */
  pDcl->D_flag = 0;              /* restore all timestamps */
  pDcl->U_flag = 0;              /* do not disable UTF-8 support */
  pDcl->ExtractOnlyNewer = 0;    /* do not extract only newer */
  pDcl->SpaceToUnderscore = 0;   /* do not convert space to '_' in filenames */
  pDcl->PromptToOverwrite = 0;   /* "overwrite all" selected -> no query mode */
  lstrcpyn(pDcl->zipfile, argv[1], sizeof(pDcl->zipfile));     /* the archive name */
  pDcl->extractDir[0] = '\0';    /* the directory to extract to.
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
    for (infc = 0; infc < argc - 2; infc++)
      if (!strcmp("-x", infv[infc])) {
        x_opt = infv[infc];
        infv[infc] = nullptr;
        break;
      }
    exfc = argc - infc - 3;
    if (exfc > 0)
      exfv = &argv[infc + 3];
    else {
      exfc = 0;
      exfv = nullptr;
    }
  }
  else {
    infc = exfc = 0;
    infv = exfv = nullptr;
  }
  int res = (*pSingleEntryUnzip)(infc, infv, exfc, exfv, pUserFunc);
  if (x_opt) {
    infv[infc] = x_opt;
    x_opt = nullptr;
  }

  if (res != 0)
    std::cout << std::format("Error unzipping (error/warning code {:d}...\n", res);
  return res;

}
