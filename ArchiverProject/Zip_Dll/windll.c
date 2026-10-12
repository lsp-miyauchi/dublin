/*
 Copyright (C) 1996 Mike White
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as all of the original files are included,
 that it is not sold for profit, and that this copyright notice is retained.

*/

/*
 *  windll.c by Mike White loosly based on Mark Adler's zip.c
 */
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <process.h>
#include <signal.h>
#include <stdarg.h>
#include <ctype.h>
#include "zip.h"
#include "windll.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")


HINSTANCE ghInst;
BOOL rgGetValue(LPCSTR pSec, LPCSTR pKey, LPINT pData);

/*  DLL Entry Point */
BOOL WINAPI DllMain( HINSTANCE hInstance,
                     DWORD dwReason,
                     LPVOID plvReserved)
{
	switch( dwReason )
	{
		case DLL_PROCESS_ATTACH:
			// DLL is loaded. Do your initialization here.
			// If cannot init, set rc to FALSE.
			InitCommonControls();
			ghInst = hInstance;
			if (!rgGetValue("Stat", "Locked", (int*)&gdwLocked))
				gdwLocked = 0;
			break;

		case DLL_PROCESS_DETACH:
			// DLL is unloaded. Do your cleanup here.
			break;

		default:
			break;
	}
	return TRUE;
}

LPSTR szCommentBuf;
HANDLE hStr;

void comment(ZIPINFO* pz, unsigned int comlen)
{
	unsigned int i;
	if (comlen > 65534L)
		comlen = (unsigned int) 65534L;
	hStr = GlobalAlloc( GPTR, (DWORD)65535L);
	if ( !hStr )
	{
		hStr = GlobalAlloc( GPTR, (DWORD) 2);
		szCommentBuf = (LPSTR)GlobalLock(hStr);
		szCommentBuf[0] = '\0';
		return;
	}

	szCommentBuf = (LPSTR)GlobalLock(hStr);
	if (comlen)
	{
		for (i = 0; i < comlen; i++)
			szCommentBuf[i] = pz->zcomment[i];
		szCommentBuf[comlen] = '\0';
	}
	else
		szCommentBuf[0] = '\0';
	free(pz->zcomment);
	pz->zcomment = (char*)malloc(1);
	*pz->zcomment = 0;
	szCommentBuf = pz->pzcl->pfnComment(szCommentBuf);
	return;
}

#define STDIO_BUF_SIZE 16384

int __cdecl printf_dll(const char *format, ...)
{
	va_list argptr;
	LPSTR pszBuffer;
	int len;

	va_start(argptr, format);
	pszBuffer = (LPSTR)GlobalAllocPtr(GHND, STDIO_BUF_SIZE);
	if (!pszBuffer)
		return 0;
	len = wvsprintf(pszBuffer, format, argptr);
	va_end(argptr);
	len = pZipInfo->pzcl->pfnPrint(pZipInfo->pzcl, pszBuffer, len);
	GlobalFreePtr(pszBuffer);
	return len;
}

/* fprintf clone for code in zip.c, etc. */
int __cdecl fprintf_dll(FILE *fp, const char *format, ...)
{
	va_list argptr;
	LPSTR pszBuffer;
	int len;

	va_start(argptr, format);
	pszBuffer = (LPSTR)GlobalAllocPtr(GHND, STDIO_BUF_SIZE);
	if (!pszBuffer)
		return 0;
	len = wvsprintf(pszBuffer, format, argptr);
	va_end(argptr);
	if (!fp || fp == stderr || fp == stdout)
		len = pZipInfo->pzcl->pfnPrint(pZipInfo->pzcl, pszBuffer, len);
	else
		len = write(fileno(fp), pszBuffer, len);
	GlobalFreePtr(pszBuffer);
	return len;
}

void __cdecl perror(const char *parm1)
{
	printf(parm1);
}
