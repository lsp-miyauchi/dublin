/* apihelp.c */

#ifdef API_DOC

#define UNZIP_INTERNAL
#include "unzip.h"
#include "version.h"


APIDocStruct APIDoc[] = {
    {
        "UZPVERSION"  , "UzpVersion"  ,
        "UzpVer *UzpVersion(void);",
        "Get version numbers of the API and the underlying UnZip code.\n\n"
        "    This is used for comparing the version numbers of the run-time\n"
        "    DLL code with those expected from the unzip.h at compile time.\n"
        "    If the version numbers do not match, there may be compatibility\n"
        "    problems with further use of the DLL.\n\n"
        "  Example:  /* Check the major version number of the DLL code. */\n"
        "    UzpVer *pVersion;\n"
        "    pVersion = UzpVersion();\n"
        "    if (pVersion->unzip.major != UZ_MAJORVER)\n"
        "      fprintf(stderr, \"error: using wrong version of DLL\\n\");\n\n"
        "    See unzip.h for details and unzipstb.c for an example.\n"
    },

    {
        "UZPMAIN"  , "UzpMain"  ,
        "int UzpMain(int argc, char *argv[]);",
        "Provide a direct entry point to the command line interface.\n\n"
        "    This is used by the UnZip stub but you can use it in your\n"
        "    own program as well.  Output is sent to stdout.\n"
        "    0 on return indicates success.\n\n"
        "  Example:  /* Extract 'test.zip' silently, junking paths. */\n"
        "    char *argv[] = { \"-q\", \"-j\", \"test.zip\" };\n"
        "    int argc = 3;\n"
        "    if (UzpMain(argc,argv))\n"
        "      printf(\"error: unzip failed\\n\");\n\n"
        "    See unzip.h for details.\n"
    },

    {
        "UZPALTMAIN"  , "UzpAltMain"  ,
        "int UzpAltMain(int argc, char *argv[], UzpInit *init);",
        "Provide a direct entry point to the command line interface,\n"
        "optionally installing replacement I/O handler functions.\n\n"
        "    As with UzpMain(), output is sent to stdout by default.\n"
        "    `InputFn *inputfn' is not yet implemented.  0 on return\n"
        "    indicates success.\n\n"
        "  Example:  /* Replace normal output and `more' functions. */\n"
        "    char *argv[] = { \"-q\", \"-j\", \"test.zip\" };\n"
        "    int argc = 3;\n"
        "    UzpInit init = { 16, MyMessageFn, NULL, MyPauseFn };\n"
        "    if (UzpAltMain(argc,argv,&init))\n"
        "      printf(\"error: unzip failed\\n\");\n\n"
        "    See unzip.h for details.\n"
    },

    {
        "UZPUNZIPTOMEMORY", "UzpUnzipToMemory",
        "int UzpUnzipToMemory(char *zip, char *file, UzpBuffer *retstr);",
        "Pass the name of the zip file and the name of the file\n"
        "    you wish to extract.  UzpUnzipToMemory will create a\n"
        "    buffer and return it in *retstr;  0 on return indicates\n"
        "    failure.\n\n"
        "    See unzip.h for details.\n"
    },

    {
        "UZPFILETREE", "UzpFileTree",
        "int UzpFileTree(char *name, cbList(callBack),\n"
        "      char *cpInclude[], char *cpExclude[]);",
        "Pass the name of the zip file, a callback function, an\n"
        "    include and exclude file list. UzpFileTree calls the\n"
        "    callback for each valid file found in the zip file.\n"
        "    0 on return indicates failure.\n\n"
        "    See unzip.h for details.\n"
    },

    { 0 }
};


static int function_help OF((__GPRO__ APIDocStruct *doc, char *fname));



static int function_help(__G__ doc, fname)
    __GDEF
    APIDocStruct *doc;
    char *fname;
{
    strcpy(slide, fname);
    /* strupr(slide);    non-standard */
    while (doc->compare && STRNICMP(doc->compare,slide,strlen(fname)))
        doc++;
    if (!doc->compare)
        return 0;
    else
        Info(slide, 0, ((char *)slide,
          "  Function:  %s\n\n  Syntax:  %s\n\n  Purpose:  %s",
          doc->function, doc->syntax, doc->purpose));

    return 1;
}



void APIhelp(__G__ argc, argv)
    __GDEF
    int argc;
    char **argv;
{
    if (argc > 1) {
        struct APIDocStruct *doc;

        if (function_help(__G__ APIDoc, argv[1]))
            return;
#ifdef SYSTEM_API_DETAILS
        if (function_help(__G__ SYSTEM_API_DETAILS, argv[1]))
            return;
#endif
        Info(slide, 0, ((char *)slide,
          "%s is not a documented command.\n\n", argv[1]));
    }

    Info(slide, 0, ((char *)slide, "\
This API provides a number of external C and REXX functions for handling\n\
zipfiles in OS/2.  Programmers are encouraged to expand this API.\n\
\n\
C functions: -- See unzip.h for details\n\
  UzpVer *UzpVersion(void);\n\
  int UzpMain(int argc, char *argv[]);\n\
  int UzpAltMain(int argc, char *argv[], UzpInit *init);\n\
  int UzpUnzipToMemory(char *zip, char *file, UzpBuffer *retstr);\n\
  int UzpFileTree(char *name, cbList(callBack),\n\
                  char *cpInclude[], char *cpExclude[]);\n\n"));

#ifdef SYSTEM_API_BRIEF
    Info(slide, 0, ((char *)slide, SYSTEM_API_BRIEF));
#endif

    Info(slide, 0, ((char *)slide,
      "\nFor more information, type 'unzip -A <function-name>'\n"));
}

#endif /* API_DOC */
