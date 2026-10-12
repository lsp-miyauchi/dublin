/*

 Copyright (C) 1990-1997 Mark Adler, Richard B. Wales, Jean-loup Gailly,
 Kai Uwe Rommel, Onno van der Linden and Igor Mandrichenko.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as all of the original files are included,
 that it is not sold for profit, and that this copyright notice is retained.

*/

/*
 *  globals.c by Mark Adler.
 */

#define GLOBALS         /* include definition of errors[] in zip.h */
#ifndef UTIL
#define UTIL            /* do not declare the read_buf variable */
#endif

#include "zip.h"

ZIPINFO* pZipInfo = NULL;
DWORD gdwLocked = 0;

/* Handy place to build error messages */
char errbuf[FNMAX+81];

