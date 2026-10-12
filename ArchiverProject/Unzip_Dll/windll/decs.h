#ifndef __decs_h   /* prevent multiple inclusions */
#define __decs_h

#include <windows.h>
#ifndef __unzip_h
#  include "unzip.h"
#endif
#ifndef __structs_h
#  include "structs.h"
#endif

#define WizMatch match

void    WINAPI LuzNoPrinting(int f);
int     WINAPI LuzValidate(LPSTR archive, int AllCodes);
BOOL    WINAPI LuzInit(zvoid *, LPUSERFUNCTIONS);
BOOL    WINAPI LuzSetOpts(zvoid *, LPDCL);
int     WINAPI LuzUnzip(zvoid *, int, char **, int, char **);
int     WINAPI LuzSingleEntryUnzip(int, char **, int, char **,
                                    LPDCL, LPUSERFUNCTIONS);

int     WINAPI LuzUnzipToMemory(LPSTR zip, LPSTR file,
                                 LPUSERFUNCTIONS lpUserFunctions,
                                 UzpBuffer *retstr);
int     WINAPI LuzGrep(LPSTR archive, LPSTR file, LPSTR pattern,
                        int cmd, int SkipBin,
                        LPUSERFUNCTIONS lpUserFunctions);

#endif /* __decs_h */
