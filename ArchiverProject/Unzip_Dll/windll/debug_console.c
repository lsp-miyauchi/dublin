////////////////////////////////////////////////////////////////////
// debug_console.c

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>

static HANDLE stdOut = NULL;

static void dbgInit()
{
	if (stdOut == NULL) {
		AllocConsole();
		stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	}
}

void dbgTerm()
{
	if (stdOut) {
		FreeConsole();
		stdOut = NULL;
	}
}

void __cdecl dbgPrintf(const char* pszFormat, ...)
{
	char* psz;
	va_list argList;
	int len;
	DWORD dw;

	if (!stdOut) dbgInit();

	psz = (char*)malloc(1024 * sizeof(char));
	va_start(argList, pszFormat);
	len = vsprintf(psz, pszFormat, argList);
	va_end(argList);

	WriteConsole(stdOut, psz, len, &dw, NULL);
	free(psz);
}

