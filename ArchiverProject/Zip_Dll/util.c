/*

 Copyright (C) 1990-1997 Mark Adler, Richard B. Wales, Jean-loup Gailly,
 Kai Uwe Rommel, Onno van der Linden and Igor Mandrichenko.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as all of the original files are included,
 that it is not sold for profit, and that this copyright notice is retained.

*/

/*
 *  util.c by Mark Adler.
 */

#include "zip.h"
#include <ctype.h>

#ifdef MSDOS16
#  include <dos.h>
#endif

uch upper[256], lower[256];
/* Country-dependent case map table */


#ifndef UTIL /* UTIL picks out namecmp code (all utils) */

/* Local functions */
local int recmatch OF((uch *, uch *));
local int count_args OF((char *s));

#ifdef MSDOS16
  local unsigned ident OF((unsigned chr));
#endif

#ifdef NO_MKTIME
#include "mktime.c"
#endif

#ifndef HAVE_FSEEKABLE
int fseekable(fp)
FILE *fp;
{
    long x;

    return (fp == NULL || (fseek(fp, -1L, SEEK_CUR) == 0 &&
            (x = ftell(fp)) >= 0 &&
            fseek(fp,  1L, SEEK_CUR) == 0 &&
            ftell(fp) == x + 1));
}
#endif /* HAVE_FSEEKABLE */

char *isshexp(char* p)
/* p: candidate sh expression */
/* If p is a sh expression, a pointer to the first special character is
   returned.  Otherwise, NULL is returned. */
{
  for (; *p; p = (char*)_mbsinc((const unsigned char*)p))
    if (*p == '\\' && _mbsnextc((const unsigned char*)p))
      p = (char*)_mbsinc((const unsigned char*)p);
    else if (*p == '?' || *p == '*' || *p == '[')
      return p;
  return NULL;
}

static UINT curchar(BYTE* pstr)
{
	if (_ismbblead(*pstr))
		return (((UINT)(*pstr) << 8) | (UINT)(*(pstr + 1)));
	return *pstr;
}

local int recmatch(uch* p, uch* s)
/* p: sh pattern to match */
/* s: string to match it to */
/* Recursively compare the sh pattern p with the string s and return 1 if
   they match, and 0 or 2 if they don't or if there is a syntax error in the
   pattern.  This routine recurses on itself no deeper than the number of
   characters in the pattern. */
{
  unsigned int c;       /* pattern char or start of range in [-] loop */
  /* Get first character, the pattern for new recmatch calls follows */
  c = curchar(p);
  p = _mbsinc(p);

  /* If that was the end of the pattern, match if string empty too */
  if (c == 0)
    return (curchar(s) == 0);

  /* '?' (or '%') matches any character (but not an empty string) */
  if (c == '?')
    return curchar(s) ? recmatch(p, _mbsinc(s)) : 0;

  /* '*' matches any number of characters, including zero */
  if (c == '*')
  {
    if (curchar(p) == 0)
      return 1;
    for (; curchar(s); s = _mbsinc(s))
      if ((c = recmatch(p, s)) != 0)
        return (int)c;
    return 2;           /* 2 means give up--shmatch will return false */
  }

#ifdef WILD_NORMALIZE
  /* Parse and process the list of characters and ranges in brackets */
  if (c == '[')
  {
    int e;              /* flag true if next char to be taken literally */
    uch *q;   /* pointer to end of [-] group */
    uch *prev;
    int r;              /* flag true to match anything but the range */

    if (curchar(s) == 0)                        /* need a character to match */
      return 0;

    prev = p;
    r = (curchar(p) == '!' || curchar(p) == '^'); /* see if reverse */
    if (r)
      p = _mbsinc(p);

    for (q = p, e = 0; curchar(q); q = _mbsinc(q))      /* find closing bracket */
      if (e)
        e = 0;
      else
        if (curchar(q) == '\\')
          e = 1;
        else if (curchar(q) == ']')
          break;
    if (curchar(q) != ']')                    /* nothing matches if bad syntax */
      return 0;
    for (c = 0, e = (curchar(p) == '-'); p < q; p = _mbsinc(p))   /* go through the list */
    {
      if (e == 0 && curchar(p) == '\\')         /* set escape flag if \ */
        e = 1;
      else if (e == 0 && curchar(p) == '-')     /* set start of range if - */
        c = curchar(_mbsdec(prev, p));
      else
      {
        uch cc = case_map(curchar(s));
        if (_mbsnextc(p) != '-')
          for (c = c ? c : curchar(p); c <= curchar(p); c++)    /* compare range */
            if (case_map(c) == cc)
              return r ? 0 : recmatch(_mbsinc(q), _mbsinc(s));
        c = e = 0;                      /* clear range, escape flags */
      }
    }
    return r ? recmatch(_mbsinc(q), _mbsinc(s)) : 0;      /* bracket match failed */
  }

  /* If escape ('\'), just compare next character */
  if (c == '\\')
  {
    c = curchar(p);                  /* if \ at end, then syntax error */
	p = _mbsinc(p);
    if (c == 0)                /* if \ at end, then syntax error */
      return 0;
    if (curchar(s) == '\\')
      s = _mbsinc(s);
  }
#endif		/* end WILD_NORMALIZE */


  /* Just a character--compare it */
//  return case_map(c) == case_map(curchar(s)) ? recmatch(p, _mbsinc(s)) : 0;
  return (c == curchar(s)) ? recmatch(p, _mbsinc(s)) : 0;
}

local int recmatch_ex(uch* p, uch* s)
/* p: sh pattern to match */
/* s: string to match it to */
/* Recursively compare the sh pattern p with the string s and return 1 if
   they match, and 0 or 2 if they don't or if there is a syntax error in the
   pattern.  This routine recurses on itself no deeper than the number of
   characters in the pattern. */
{
  unsigned int c;       /* pattern char or start of range in [-] loop */
  /* Get first character, the pattern for new recmatch calls follows */
  c = curchar(p);
  p = _mbsinc(p);

  /* If that was the end of the pattern, match if string empty too */
  if (c == 0)
    return (curchar(s) == 0);

  /* '?' (or '%') matches any character (but not an empty string) */
  if (c == '?')
    return curchar(s) ? recmatch(p, _mbsinc(s)) : 0;

  /* '*' matches any number of characters, including zero */
  if (c == '*')
  {
    if (curchar(p) == 0)
      return 1;
    for (; curchar(s); s = _mbsinc(s))
      if ((c = recmatch(p, s)) != 0)
        return (int)c;
    return 2;           /* 2 means give up--shmatch will return false */
  }

  /* If escape ('\'), just compare next character */
  if (c == '\\')
  {
    c = curchar(p);                  /* if \ at end, then syntax error */
	p = _mbsinc(p);
    if (c == 0)                /* if \ at end, then syntax error */
      return 0;
    if (curchar(s) == '\\')
      s = _mbsinc(s);
  }

  /* Just a character--compare it */
//  return case_map(c) == case_map(curchar(s)) ? recmatch(p, _mbsinc(s)) : 0;
  return (c == curchar(s)) ? recmatch(p, _mbsinc(s)) : 0;
}

int shmatch(char* p, char* s)
/* p: sh pattern to match */
/* s: string to match it to */
/* Compare the sh pattern p with the string s and return true if they match,
   false if they don't or if there is a syntax error in the pattern. */
{
  return recmatch((uch *) p, (uch *) s) == 1;
}


#if defined(DOS) || defined(WIN32)
/* XXX  also suitable for OS2?  Atari?  Human68K?  TOPS-20?? */

int dosmatch(char* p, char* s)
/* p: dos pattern to match    */
/* s: string to match it to   */
/* Treat filenames without periods as having an implicit trailing period */
{
  char *s1;             /* revised string to match */
  int r;                /* result */

  if ((s1 = (char*)malloc(lstrlen(s) + 2)) == NULL)
    return recmatch((uch *) p, (uch *) s) == 1;  /* will usually be correct */
  lstrcpy(s1, s);
  if (strchr(p, '.') && !strchr(s1, '.'))
    lstrcat(s1, ".");
  r = recmatch((uch *)p, (uch *)s1);
  free((zvoid *)s1);
  return r == 1;
}

#endif /* DOS || WIN32 */

zvoid** search(zvoid* b, zvoid** a, extent n, int (*cmp)(ZCONST zvoid *, ZCONST zvoid*))
/* b: pointer to value to search for */
/* a: table of pointers to values, sorted */
/* n: number of pointers in a[] */
/* cmp: comparison function */

/* Search for b in the pointer list a[0..n-1] using the compare function
   cmp(b, c) where c is an element of a[i] and cmp() returns negative if
   *b < *c, zero if *b == *c, or positive if *b > *c.  If *b is found,
   search returns a pointer to the entry in a[], else search() returns
   NULL.  The nature and size of *b and *c (they can be different) are
   left up to the cmp() function.  A binary search is used, and it is
   assumed that the list is sorted in ascending order. */
{
  zvoid** i;        /* pointer to midpoint of current range */
  zvoid** l;        /* pointer to lower end of current range */
  int r;                /* result of (*cmp)() call */
  zvoid** u;        /* pointer to upper end of current range */

  l = (zvoid**)a;  u = l + (n-1);
  while (u >= l) {
    i = l + ((unsigned)(u - l) >> 1);
    if ((r = (*cmp)(b, (char *)*(struct zlist **)i)) < 0)
      u = i - 1;
    else if (r > 0)
      l = i + 1;
    else
      return (zvoid**)i;
  }
  return NULL;          /* If b were in list, it would belong at l */
}

#endif /* !UTIL */

#ifdef MSDOS16

local unsigned ident(unsigned chr)
{
   return chr; /* in al */
}

void init_upper()
{
  static struct country {
    uch ignore[18];
    int (*casemap)(int);
    uch filler[16];
  } country_info;

  struct country* info = &country_info;
  union REGS regs;
  struct SREGS sregs;
  int c;

  regs.x.ax = 0x3800; /* get country info */
  regs.x.dx = FP_OFF(info);
  sregs.ds  = FP_SEG(info);
  intdosx(&regs, &regs, &sregs);
  for (c = 0; c < 128; c++) {
    upper[c] = (uch) toupper(c);
    lower[c] = (uch) c;
  }
  for (; c < sizeof(upper); c++) {
    upper[c] = (uch) (*country_info.casemap)(ident(c));
    /* ident() required because casemap takes its parameter in al */
    lower[c] = (uch) c;
  }
  for (c = 0; c < sizeof(upper); c++ ) {
    int u = upper[c];
    if (u != c && lower[u] == (uch) u) {
      lower[u] = (uch)c;
    }
  }
  for (c = 'A'; c <= 'Z'; c++) {
    lower[c] = (uch) (c - 'A' + 'a');
  }
}
#else /* !MSDOS16 */
#  ifndef OS2

void init_upper()
{
  int c;
#if defined(ATARI) || defined(CMS_MVS)
#include <ctype.h>
/* this should be valid for all other platforms too.   (HD 11/11/95) */
  for (c = 0; c< sizeof(upper); c++) {
    upper[c] = islower(c) ? toupper(c) : c;
    lower[c] = isupper(c) ? tolower(c) : c;
  }
#else
  for (c = 0; c < sizeof(upper); c++) upper[c] = lower[c] = c;
  for (c = 'a'; c <= 'z';        c++) upper[c] = c - 'a' + 'A';
  for (c = 'A'; c <= 'Z';        c++) lower[c] = c - 'A' + 'a';
#endif
}
#  endif /* !OS2 */

#endif /* ?MSDOS16 */

int namecmp(char* string1, char* string2)
/* Compare the two strings ignoring case, and correctly taking into
 * account national language characters. For operating systems with
 * case sensitive file names, this function is equivalent to strcmp.
 */
{
#ifndef WIN32
  int d;

  for (;;)
  {
    d = (int) (uch) case_map(*string1)
      - (int) (uch) case_map(*string2);

    if (d || *string1 == 0 || *string2 == 0)
      return d;

    string1++;
    string2++;
  }
#else
  return lstrcmpi(string1, string2);
#endif
}

#ifdef IZ_ISO2OEM_ARRAY
char *str_iso_to_oem(dst, src)
  ZCONST char *src;
  char *dst;
{
  char *dest_start = dst;
  while (*dst++ = (char)iso2oem[(uch)(*src++)]);
  return dest_start;
}
#endif

#ifdef IZ_OEM2ISO_ARRAY
char *str_oem_to_iso(dst, src)
  ZCONST char *src;
  char *dst;
{
  char *dest_start = dst;
  while (*dst++ = (char)oem2iso[(uch)(*src++)]);
  return dest_start;
}
#endif


#ifndef UTIL

extern char *getenv OF((ZCONST char *));

/*****************************************************************
 | envargs - add default options from environment to command line
 |----------------------------------------------------------------
 | Author: Bill Davidsen, original 10/13/91, revised 23 Oct 1991.
 | This program is in the public domain.
 |----------------------------------------------------------------
 | Minor program notes:
 |  1. Yes, the indirection is a tad complex
 |  2. Parenthesis were added where not needed in some cases
 |     to make the action of the code less obscure.
 ****************************************************************/

void envargs(ZIPINFO*pz, int* Pargc, char*** Pargv, char* envstr, char* envstr2)
{
    char *envptr;                     /* value returned by getenv */
    char *bufptr;                     /* copy of env info */
    int argc;                         /* internal arg count */
    register int ch;                  /* spare temp value */
    char **argv;                      /* internal arg vector */
    char **argvect;                   /* copy of vector address */

    /* see if anything in the environment */
    envptr = getenv(envstr);
    if (envptr != NULL)                                /* usual var */
        while (isspace(*envptr))           /* we must discard leading spaces */
            envptr++;
    if (envptr == NULL || *envptr == '\0')
        if ((envptr = getenv(envstr2)) != NULL)                 /* alternate */
            while (isspace(*envptr))
                envptr++;
    if (envptr == NULL || *envptr == '\0')
        return;

    /* count the args so we can allocate room for them */
    argc = count_args(envptr);
    bufptr = (char*)malloc(1 + lstrlen(envptr));
    if (bufptr == NULL)
        ziperr(pz, ZE_MEM, "Can't get memory for arguments");
    lstrcpy(bufptr, envptr);

    /* allocate a vector large enough for all args */
    argv = (char **)malloc((argc + *Pargc + 1) * sizeof(char *));
    if (argv == NULL) {
        free(bufptr);
        ziperr(pz, ZE_MEM, "Can't get memory for arguments");
    }
    argvect = argv;

    /* copy the program name first, that's always true */
    *(argv++) = *((*Pargv)++);

    /* copy the environment args first, may be changed */
    do {
#if defined(AMIGA) || defined(UNIX)
        if (*bufptr == '"') {
            char *argstart = ++bufptr;
            *(argv++) = argstart;
            for (ch = *bufptr; ch != '\0' && ch != '\"'; ch = *(++bufptr))
                if (ch == '\\' && bufptr[1] != '\0')
                    ++bufptr;                  /* skip char after backslash */
            if (ch != '\0') *(bufptr++) = '\0';     /* overwrite trailing " */

            /* remove escape characters */
            while (argstart = strchr(argstart, '\\')) {
                lstrcpy(argstart, argstart + 1);
                if (*argstart)
                    ++argstart;
            }
        } else {
            *(argv++) = bufptr;
            while ((ch = *bufptr) != '\0' && !isspace(ch)) ++bufptr;
            if (ch != '\0') *(bufptr++) = '\0';
        }
#else
#  ifdef WIN32
        /* We do not support backslash-quoting of quotes in quoted */
        /* strings under Win32, because backslashes are directory  */
        /* separators and double quotes are illegal in filenames.  */
        if (*bufptr == '"') {
            *(argv++) = ++bufptr;
            while ((ch = *bufptr) != '\0' && ch != '\"') ++bufptr;
            if (ch != '\0') *(bufptr++) = '\0';
        } else {
            *(argv++) = bufptr;
            while ((ch = *bufptr) != '\0' && !isspace(ch)) ++bufptr;
            if (ch != '\0') *(bufptr++) = '\0';
        }
#  else
        *(argv++) = bufptr;
        while ((ch = *bufptr) != '\0' && !isspace(ch)) ++bufptr;
        if (ch != '\0') *(bufptr++) = '\0';
#  endif
#endif /* ?(AMIGA || UNIX) */
        while ((ch = *bufptr) != '\0' && isspace(ch)) ++bufptr;
    } while (ch);

    /* now save old argc and copy in the old args */
    argc += *Pargc;
    while (--(*Pargc)) *(argv++) = *((*Pargv)++);

    /* finally, add a NULL after the last arg, like UNIX */
    *argv = NULL;

    /* save the values and return */
    *Pargv = argvect;
    *Pargc = argc;
}

static int count_args(char* s)
{
    int count = 0;
    char ch;

    do {
        /* count and skip args */
        ++count;
#if defined(AMIGA) || defined(UNIX)
        if (*s == '\"') {
            for (ch = *(++s); ch != '\0' && ch != '\"'; ch = *(++s))
                if (ch == '\\' && s[1] != '\0')
                    ++s;
            if (*s) ++s;        /* trailing quote */
        } else
            while ((ch = *s) != '\0' && !isspace(ch)) ++s;
#else
#  ifdef WIN32
        if (*s == '\"') {
            ++s;                /* leading quote */
            while ((ch = *s) != '\0' && ch != '\"') ++s;
            if (*s) ++s;        /* trailing quote */
        } else
            while ((ch = *s) != '\0' && !isspace(ch)) ++s;
#  else
        while ((ch = *s) != '\0' && !isspace(ch)) ++s;
#  endif
#endif /* ?(AMIGA || UNIX) */
        while ((ch = *s) != '\0' && isspace(ch)) ++s;
    } while (ch);

    return(count);
}



/* Extended argument processing -- by Rich Wales
 * This function currently deals only with the MKS shell, but could be
 * extended later to understand other conventions.
 *
 * void expand_args(int *argcp, char ***argvp)
 *
 *    Substitutes the extended command line argument list produced by
 *    the MKS Korn Shell in place of the command line info from DOS.
 *
 *    The MKS shell gets around DOS's 128-byte limit on the length of
 *    a command line by passing the "real" command line in the envi-
 *    ronment.  The "real" arguments are flagged by prepending a tilde
 *    (~) to each one.
 *
 *    This "expand_args" routine creates a new argument list by scanning
 *    the environment from the beginning, looking for strings begin-
 *    ning with a tilde character.  The new list replaces the original
 *    "argv" (pointed to by "argvp"), and the number of arguments
 *    in the new list replaces the original "argc" (pointed to by
 *    "argcp").
 */
void expand_args(int* argcp, char*** argvp)
{
#ifdef DOS

/* Do NEVER include (re)definiton of `environ' variable with any version
   of MSC or BORLAND/Turbo C. These compilers supply an incompatible
   definition in <stdlib.h>.  */
#if defined(__GO32__) || defined(__EMX__)
      extern char **environ;          /* environment */
#endif /* __GO32__ || __EMX__ */
      char        **envp;             /* pointer into environment */
      char        **newargv;          /* new argument list */
      char        **argp;             /* pointer into new arg list */
      int           newargc;          /* new argument count */

      /* sanity check */
      if (environ == NULL
          || argcp == NULL
          || argvp == NULL || *argvp == NULL)
              return;
      /* find out how many environment arguments there are */
      for (envp = environ, newargc = 0;
           *envp != NULL && (*envp)[0] == '~';
           envp++, newargc++) ;
      if (newargc == 0)
              return;                 /* no environment arguments */
      /* set up new argument list */
      newargv = (char **) malloc(sizeof(char **) * (newargc+1));
      if (newargv == NULL)
              return;                 /* malloc failed */
      for (argp = newargv, envp = environ;
           *envp != NULL && (*envp)[0] == '~';
           *argp++ = &(*envp++)[1]) ;
      *argp = NULL;                   /* null-terminate the list */
      /* substitute new argument list in place of old one */
      *argcp = newargc;
      *argvp = newargv;
#else /* ?DOS */
      if (argcp || argvp) return;
#endif /* ?DOS */
}

#endif /* UTIL */
