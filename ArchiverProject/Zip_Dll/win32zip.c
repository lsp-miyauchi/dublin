/*

 Copyright (C) 1990-1997 Mark Adler, Richard B. Wales, Jean-loup Gailly,
 Kai Uwe Rommel, Onno van der Linden and Igor Mandrichenko.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as all of the original files are included,
 that it is not sold for profit, and that this copyright notice is retained.

*/

#ifndef UTIL    /* this file contains nothing used by UTIL */

#include "zip.h"

#ifndef __EMX__
#include <direct.h>     /* for rmdir() */
#endif
#include <time.h>

#ifndef __BORLANDC__
#include <sys/utime.h>
#else
#include <utime.h>
#endif
#include <windows.h> /* for findfirst/findnext stuff */
#ifdef __RSXNT__
#  include "rsxntwin.h"
#endif
char *GetLongPathEA OF((char *name));

#include <io.h>
#define MATCH         dosmatch

#define PAD           0
#define PATH_END      '/'
#define HIDD_SYS_BITS (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)


typedef struct zdirent {
  ush    d_date, d_time;
  ulg    d_size;
  char   d_attr;
  char   d_name[MAX_PATH * 2];
  int    d_first;
  HANDLE d_hFindFile;
  int    d_dosify;
} zDIR;

#include "win32zip.h"
#include "nt.h"

/* Local functions */
local zDIR           *Opendir      OF((ZCONST char *n, int dosify));
local struct zdirent *Readdir      OF((zDIR *d));
local void            Closedir     OF((zDIR *d));

local char           *readd        OF((ZIPINFO*, zDIR *));
local int             wild_recurse OF((ZIPINFO*, char *, char *));
#ifdef NTSD_EAS
   local void GetSD OF((ZIPINFO*, char *path, char **bufptr, size_t *size,
                        char **cbufptr, size_t *csize));
#endif
#ifdef USE_EF_UT_TIME
   local int GetExtraTime OF((struct zlist*z, iztimes *z_utim));
#endif

/* Module level variables */
extern char *label /* = NULL */ ;       /* defined in fileio.c */
local ulg label_time = 0;
local ulg label_mode = 0;
local time_t label_utim = 0;

/* Module level constants */
local ZCONST char wild_match_all[] = "*.*";

local zDIR *Opendir(ZCONST char* n, int dosify)
/* n: directory to open */
/* Start searching for files in the MSDOS directory n */
{
  zDIR *d;              /* malloc'd return value */
  char *p;              /* malloc'd temporary string */
  char *q;
  WIN32_FIND_DATA fd;

  if ((d = (zDIR *)malloc(sizeof(zDIR))) == NULL ||
      (p = (char*)malloc(lstrlen(n) + 5)) == NULL) {
    if (d != NULL) free((zvoid *)d);
    return NULL;
  }
  d->d_dosify = dosify;

  lstrcpy(p, n);
  q = p + lstrlen(p);
  if (q[-1] == ':')
      *q++ = '.';
  if ((q - p) > 0 && *(q - 1) != '/')
    *q++ = '/';
  lstrcpy(q, wild_match_all);

#ifdef __RSXNT__        /* RSXNT/EMX C rtl uses OEM charset */
  OemToAnsi(p, p);
#endif
  d->d_hFindFile = FindFirstFile(p, &fd);
  free((zvoid *)p);

  if (d->d_hFindFile == INVALID_HANDLE_VALUE)
  {
    free((zvoid *)d);
    return NULL;
  }

  if (dosify && fd.cAlternateFileName[0])
    lstrcpy(d->d_name, fd.cAlternateFileName);
  else
    lstrcpy(d->d_name, fd.cFileName);
  d->d_attr = (unsigned char) fd.dwFileAttributes;
  d->d_first = 1;
  return d;
}

local struct zdirent *Readdir(zDIR* d)
/* d: directory stream to read from */
/* Return pointer to first or next directory entry, or NULL if end. */
{
  if (d->d_first)
    d->d_first = 0;
  else
  {
    WIN32_FIND_DATA fd;

    if (!FindNextFile(d->d_hFindFile, &fd))
        return NULL;

    if (d->d_dosify && fd.cAlternateFileName[0])
      lstrcpy(d->d_name, fd.cAlternateFileName);
    else
      lstrcpy(d->d_name, fd.cFileName);
    d->d_attr = (unsigned char) fd.dwFileAttributes;
  }
#ifdef __RSXNT__        /* RSXNT/EMX C rtl uses OEM charset */
  AnsiToOem(d->d_name, d->d_name);
#endif
  return (struct zdirent *)d;
}

local void Closedir(zDIR* d)
/* d: directory stream to close */
{
  FindClose(d->d_hFindFile);
  free((zvoid *)d);
}

local char *readd(ZIPINFO* pz, zDIR* d)
/* d: directory stream to read from */
/* Return a pointer to the next name in the directory stream d, or NULL if
   no more entries or an error occurs. */
{
	struct zdirent *e;

	do
		e = Readdir(d);
	while (!pz->hidden_files && e && e->d_attr & HIDD_SYS_BITS);
	return e == NULL ? (char *) NULL : e->d_name;
}


#define ONENAMELEN 255

/* whole is a pathname with wildcards, wildtail points somewhere in the  */
/* middle of it.  All wildcards to be expanded must come AFTER wildtail. */

local int wild_recurse(ZIPINFO* pz, char* whole, char* wildtail)
{
    zDIR *dir;
    char *subwild, *name, *newwhole = NULL, *glue = NULL, plug = 0, plug2;
    ush newlen, amatch = 0;
    int e = ZE_MISS;

    if (!isshexp(wildtail))
        if (GetFileAttributes(whole) != 0xFFFFFFFF)     /* file exists? */
            return procname(pz, whole);
        else
            return ZE_MISS;                     /* woops, no wildcards! */

    /* back up thru path components till existing dir found */
    do {
        name = wildtail + lstrlen(wildtail) - 1;
        for (;;)
            if (name-- <= wildtail || *name == PATH_END) {
                subwild = name + 1;
                plug2 = *subwild;
                *subwild = 0;
                break;
            }
        if (glue)
            *glue = plug;
        glue = subwild;
        plug = plug2;
        dir = Opendir(whole, pz->dosify);
    } while (!dir && subwild > wildtail);
    wildtail = subwild;                 /* skip past non-wild components */

    if ((subwild = strchr(wildtail + 1, PATH_END)) != NULL) {
        /* this "+ 1" dodges the  ^^^ hole left by *glue == 0 */
        *(subwild++) = 0;               /* wildtail = one component pattern */
        newlen = lstrlen(whole) + lstrlen(subwild) + ONENAMELEN + 2;
    } else
        newlen = lstrlen(whole) + ONENAMELEN + 1;
    if (!dir || ((newwhole = (char*)malloc(newlen)) == NULL)) {
        if (glue)
            *glue = plug;
        e = dir ? ZE_MEM : ZE_MISS;
        goto ohforgetit;
    }

    lstrcpy(newwhole, whole);
    newlen = lstrlen(newwhole);
    if (glue)
        *glue = plug;                           /* repair damage to whole */
    if (!isshexp(wildtail)) {
        e = ZE_MISS;                            /* non-wild name not found */
        goto ohforgetit;
    }

    while ((name = readd(pz, dir)) != NULL) {
        if (lstrcmp(name, ".") && lstrcmp(name, "..") && MATCH(wildtail, name)) {
            lstrcpy(newwhole + newlen, name);
            if (subwild) {
                name = newwhole + lstrlen(newwhole);
                *(name++) = PATH_END;
                lstrcpy(name, subwild);
                e = wild_recurse(pz, newwhole, name);
            } else
                e = procname(pz, newwhole);
            newwhole[newlen] = 0;
            if (e == ZE_OK)
                amatch = 1;
            else if (e != ZE_MISS)
                break;
        }
    }

  ohforgetit:
    if (dir) Closedir(dir);
    if (subwild) *--subwild = PATH_END;
    if (newwhole) free(newwhole);
    if (e == ZE_MISS && amatch)
        e = ZE_OK;
    return e;
}

int wild(ZIPINFO* pz, char* w)
/* w: path/pattern to match */
/* If not in exclude mode, expand the pattern based on the contents of the
   file system.  Return an error code in the ZE_ class. */
{
    char *p;              /* path */
    char *q;              /* diskless path */
    int e;                /* result */

    if (pz->volume_label == 1) {
      pz->volume_label = 2;
      label = getVolumeLabel((w != NULL && w[1] == ':') ? to_up(w[0]) : '\0',
                             &label_time, &label_mode, &label_utim);
      if (label != NULL)
        (void)newname(pz, label, 0);
      if (w == NULL || (w[1] == ':' && w[2] == '\0')) return ZE_OK;
      /* "zip -$ foo a:" can be used to force drive name */
    }

    /* Allocate and copy pattern, leaving room to add "." if needed */
    if ((p = (char*)malloc(lstrlen(w) + 2)) == NULL)
        return ZE_MEM;
    lstrcpy(p, w);

    /* Normalize path delimiter as '/' */
	replacechar(p, '\\', '/');

    /* Separate the disk part of the path */
    if ((q = strchr(p, ':')) != NULL) {
        if (strchr((++q), ':'))     /* sanity check for safety of wild_recurse */
            return ZE_MISS;
    } else
        q = p;

    /* Normalize bare disk names */
    if (q > p && !*q)
        lstrcpy(q, ".");

    /* Here we go */
    e = wild_recurse(pz, p, q);
    free((zvoid *)p);
    return e;
}

int procname(ZIPINFO* pz, char* n)
/* n: name to process */
/* Process a name or sh expression to operate on (or exclude).  Return
   an error code in the ZE_ class. */
{
  char *a;              /* path and name for recursion */
  zDIR *d;              /* directory stream from opendir() */
  char *e;              /* pointer to name from readd() */
  int m;                /* matched flag */
  char *p;              /* path for recursion */
  struct stat s;        /* result of stat() */
  struct zlist* z;  /* steps through zfiles list */

  if (LSSTAT(n, &s))
  {
    /* Not a file or directory--search for shell expression in zip file */
    p = ex2in(pz, n, 0, (int *)NULL);       /* shouldn't affect matching chars */
    m = 1;
    for (z = pz->zfiles; z != NULL; z = z->nxt) {
      if (MATCH(p, z->iname))
      {
        z->mark = pz->pcount ? filter(pz, z->zname) : 1;
        if (pz->verbose)
            fprintf(pz->mesg, "zip diagnostic: %scluding %s\n",
               z->mark ? "in" : "ex", z->name);
        m = 0;
      }
    }
    free((zvoid *)p);
    return m ? ZE_MISS : ZE_OK;
  }

  /* Live name--use if file, recurse if directory */
  replacechar(n, '\\', '/');
  if ((s.st_mode & S_IFDIR) == 0)
  {
    /* add or remove name of file */
    if ((m = newname(pz, n, 0)) != ZE_OK)
      return m;
  } else {
    /* Add trailing / to the directory name */
    if ((p = (char*)malloc(lstrlen(n)+2)) == NULL)
      return ZE_MEM;
    if (lstrcmp(n, ".") == 0 || lstrcmp(n, "/.") == 0) {
      *p = '\0';  /* avoid "./" prefix and do not create zip entry */
    } else {
      lstrcpy(p, n);
      a = p + lstrlen(p);
      if (a[-1] != '/')
        lstrcpy(a, "/");
      if (pz->dirnames && (m = newname(pz, p, 1)) != ZE_OK) {
        free((zvoid *)p);
        return m;
      }
    }
    /* recurse into directory */
    if (pz->recurse && (d = Opendir(n, pz->dosify)) != NULL)
    {
      while ((e = readd(pz, d)) != NULL) {
        if (lstrcmp(e, ".") && lstrcmp(e, ".."))
        {
          if ((a = (char*)malloc(lstrlen(p) + lstrlen(e) + 1)) == NULL)
          {
            Closedir(d);
            free((zvoid *)p);
            return ZE_MEM;
          }
          lstrcat(lstrcpy(a, p), e);
          if ((m = procname(pz, a)) != ZE_OK)   /* recurse on name */
          {
            if (m == ZE_MISS)
              zipwarn(pz, "name not matched: ", a);
            else
              ziperr(pz, m, a);
          }
          free((zvoid *)a);
        }
      }
      Closedir(d);
    }
    free((zvoid *)p);
  } /* (s.st_mode & S_IFDIR) == 0) */
  return ZE_OK;
}

/////////////////////////////////////////////////////////////////////
// Convert the external file name to a zip file name,
// returning the malloc'ed string or NULL if not enough memory.

char *ex2in(ZIPINFO* pz, char* x, int isdir, int* pdosflag)
/* x: external file name */
/* isdir: input: x is a directory */
/* pdosflag: output: force MSDOS file attributes? */
{
	char *n;              /* internal file name (malloc'ed) */
	char *t;              /* shortened name */
	char name[MAX_PATH * 2];

	lstrcpy(name, x);
	replacechar(name, '/', '\\');
	if (pz->dosify)
		GetShortPathName(name, name, MAX_PATH * 2);
	x = name;
	replacechar(x, '\\', '/');

	// Find starting point in name before doing malloc
	t = (*x && *(x + 1) == ':') ? x + 2 : x;
	while (*t == '/')
		t++;

	// Strip leading "./" as well as drive letter
	while (*t == '.' && t[1] == '/')
		t += 2;

	if (!pz->pathput)
		t = last(t, PATH_END);

	// Malloc space for internal name and copy it
	if ((n = (char*)malloc(lstrlen(t) + 1)) == NULL)
		return NULL;
	lstrcpy(n, t);

	// Returned malloc'ed name
	if (pdosflag)
		*pdosflag = pz->dosify;
	return n;
}


char *in2ex(ZIPINFO* pz, char* n)
/* n: internal file name */
/* Convert the zip file name to an external file name, returning the malloc'ed
   string or NULL if not enough memory. */
{
	char *x;              /* external file name */

	if ((x = (char*)malloc(lstrlen(n) + 1 + PAD)) == NULL)
		return NULL;
	lstrcpy(x, n);

	return x;
}


void stamp(char* f, ulg d)
/* f: name of file to change */
/* d: dos-style time to change it to */
/* Set last updated and accessed time of file f to the DOS time d. */
{
#if defined(__TURBOC__) && !defined(__BORLANDC__)
  int h;                /* file handle */

  if ((h = open(f, 0)) != -1)
  {
    setftime(h, (struct ftime *)&d);
    close(h);
  }
#else /* !__TURBOC__ */

  struct utimbuf u;     /* argument for utime() */

  /* Convert DOS time to time_t format in u.actime and u.modtime */
  u.actime = u.modtime = dos2unixtime(d);

  /* Set updated and accessed times of f */
  utime(f, &u);
#endif /* ?__TURBOC__ */
}


ulg filetime(char* f, ulg* a, long* n, iztimes* t)
/* f: name of file to get info on */
/* a: return value: file attributes */
/* n: return value: file size */
/* t: return value: access, modific. and creation times */
/* If file *f does not exist, return 0.  Else, return the file's last
   modified date and time as an MSDOS date and time.  The date and
   time is returned in a long with the date most significant to allow
   unsigned integer comparison of absolute times.  Also, if a is not
   a NULL pointer, store the file attributes there, with the high two
   bytes being the Unix attributes, and the low byte being a mapping
   of that to DOS attributes.  If n is not NULL, store the file size
   there.  If t is not NULL, the file's access, modification and creation
   times are stored there as UNIX time_t values.
   If f is "-", use standard input as the file. If f is a device, return
   a file size of -1 */
{
  struct stat s;        /* results of stat() */
  char name[FNMAX];
  int len = lstrlen(f);

  if (f == label) {
    if (a != NULL)
      *a = label_mode;
    if (n != NULL)
      *n = -2L; /* convention for a label name */
    if (t != NULL)
      t->atime = t->mtime = t->ctime = label_utim;
    return label_time;
  }
  lstrcpy(name, f);
  if (name[len - 1] == '/')
    name[len - 1] = '\0';
  /* not all systems allow stat'ing a file with / appended */

  if (LSSTAT(name, &s) != 0)
             /* Accept about any file kind including directories
              * (stored with trailing / with -r option)
              */
    return 0;

  if (a != NULL) {
    *a = ((ulg)s.st_mode << 16) | (ulg)GetFileMode(name);
  }
  if (n != NULL)
    *n = (s.st_mode & S_IFMT) == S_IFREG ? s.st_size : -1L;
  if (t != NULL) {
    t->atime = s.st_atime;
    t->mtime = s.st_mtime;
    t->ctime = s.st_ctime;
  }

  return unix2dostime((time_t *)&s.st_mtime);
}


#ifdef NTSD_EAS

local void GetSD(ZIPINFO* pz, char *path, char **bufptr, size_t *size,
                        char **cbufptr, size_t *csize)
{
  unsigned char stackbuffer[NTSD_BUFFERSIZE];
  unsigned long bytes = NTSD_BUFFERSIZE;
  unsigned char *buffer = stackbuffer;
  unsigned char *DynBuffer = NULL;
  long cbytes;
  PEF_NTSD_L_HEADER pLocalHeader;
  PEF_NTSD_C_HEADER pCentralHeader;
  VOLUMECAPS VolumeCaps;

#ifndef NO_NTSD_WITH_RSXNT
  /* check target volume capabilities */
  if (!ZipGetVolumeCaps(path, path, &VolumeCaps) ||
     !(VolumeCaps.dwFileSystemFlags & FS_PERSISTENT_ACLS)) {
    return;
  }

  VolumeCaps.bUsePrivileges = pz->use_privileges;
  VolumeCaps.dwFileAttributes = 0; /* should set to file attributes, if possible */

  if (!SecurityGet(path, &VolumeCaps, buffer, &bytes)) {

    /* try to malloc the buffer if appropriate */
    if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        DynBuffer = (unsigned char*)malloc(bytes);
        if(DynBuffer == NULL) return;

        buffer = DynBuffer; /* switch to the new buffer and try again */

        if(!SecurityGet(path, &VolumeCaps, buffer, &bytes)) {
            free(DynBuffer);
            return;
        }

    } else {
        return; /* bail */
    }
  }
#else /* NO_NTSD_WITH_RSXNT */
  return; /* bail */
#endif /* ?NO_NTSD_WITH_RSXNT */

  /* # bytes to compress: compress type, CRC, data bytes */
  cbytes = sizeof(USHORT) + sizeof(ULONG) + bytes;


  /* our two possible failure points.  don't allow trashing of any data
     if either fails - notice that *size and *csize don't get updated.
     *bufptr leaks if malloc() was used and *cbufptr alloc fails - this
     isn't relevant because it's probably indicative of a bigger problem. */

  if(*size)
    *bufptr = (char*)realloc(*bufptr, *size + sizeof(EF_NTSD_L_HEADER) + cbytes);
  else
    *bufptr = (char*)malloc(sizeof(EF_NTSD_L_HEADER) + cbytes );

  if(*csize)
    *cbufptr = (char*)realloc(*cbufptr, *csize + sizeof(EF_NTSD_C_HEADER));
  else
    *cbufptr = (char*)malloc(sizeof(EF_NTSD_C_HEADER));

  if(*bufptr == NULL || *cbufptr == NULL) {
    if(DynBuffer) free(DynBuffer);
    return;
  }

  /* local header */

  pLocalHeader = (PEF_NTSD_L_HEADER) (*bufptr + *size);

  cbytes = memcompress(pz, (char *)(pLocalHeader + 1), cbytes,
                       (char *)buffer, bytes);

  *size += sizeof(EF_NTSD_L_HEADER) + cbytes;

  pLocalHeader->nID = EF_NTSD;
  pLocalHeader->nSize = sizeof(EF_NTSD_L_HEADER) - EB_HEADSIZE + cbytes;
  pLocalHeader->lSize = bytes; /* uncompressed size */
  pLocalHeader->Version = 0;

  /* central header */

  pCentralHeader = (PEF_NTSD_C_HEADER) (*cbufptr + *csize);
  *csize += sizeof(EF_NTSD_C_HEADER);

  pCentralHeader->nID = EF_NTSD;
  pCentralHeader->nSize = sizeof(EF_NTSD_C_HEADER) - EB_HEADSIZE; /* sbz */
  pCentralHeader->lSize = bytes;

  if (pz->noisy)
    printf(" (%ld bytes security)", bytes);

  if(DynBuffer) free(DynBuffer);
}
#endif /* NTSD_EAS */


#ifdef USE_EF_UT_TIME

#define EB_L_UT_SIZE    (EB_HEADSIZE + EB_UT_LEN(3))
#define EB_C_UT_SIZE    (EB_HEADSIZE + EB_UT_LEN(1))

local int GetExtraTime(struct zlist* z, iztimes *z_utim)
{
  char *eb_l_ptr;
  char *eb_c_ptr;

  if(z->ext)
    eb_l_ptr = (char*)realloc(z->extra, (z->ext + EB_L_UT_SIZE));
  else
    eb_l_ptr = (char*)malloc(EB_L_UT_SIZE);

  if (eb_l_ptr == NULL)
    return ZE_MEM;

  if(z->cext)
    eb_c_ptr = (char*)realloc(z->cextra, (z->cext + EB_C_UT_SIZE));
  else
    eb_c_ptr = (char*)malloc(EB_C_UT_SIZE);

  if (eb_c_ptr == NULL)
    return ZE_MEM;

  z->extra = eb_l_ptr;
  eb_l_ptr += z->ext;
  z->ext += EB_L_UT_SIZE;

  eb_l_ptr[0]  = 'U';
  eb_l_ptr[1]  = 'T';
  eb_l_ptr[2]  = EB_UT_LEN(3);          /* length of data part of e.f. */
  eb_l_ptr[3]  = 0;
  eb_l_ptr[4]  = EB_UT_FL_MTIME | EB_UT_FL_ATIME | EB_UT_FL_CTIME;
  eb_l_ptr[5]  = (char)(z_utim->mtime);
  eb_l_ptr[6]  = (char)(z_utim->mtime >> 8);
  eb_l_ptr[7]  = (char)(z_utim->mtime >> 16);
  eb_l_ptr[8]  = (char)(z_utim->mtime >> 24);
  eb_l_ptr[9]  = (char)(z_utim->atime);
  eb_l_ptr[10] = (char)(z_utim->atime >> 8);
  eb_l_ptr[11] = (char)(z_utim->atime >> 16);
  eb_l_ptr[12] = (char)(z_utim->atime >> 24);
  eb_l_ptr[13] = (char)(z_utim->ctime);
  eb_l_ptr[14] = (char)(z_utim->ctime >> 8);
  eb_l_ptr[15] = (char)(z_utim->ctime >> 16);
  eb_l_ptr[16] = (char)(z_utim->ctime >> 24);

  z->cextra = eb_c_ptr;
  eb_c_ptr += z->cext;
  z->cext += EB_C_UT_SIZE;

  memcpy(eb_c_ptr, eb_l_ptr, EB_C_UT_SIZE);
  eb_c_ptr[EB_LEN] = EB_UT_LEN(1);

  return ZE_OK;
}

#endif /* USE_EF_UT_TIME */


////////////////////////////////////////////////////////////
// create extra field and change z->att if desired

int set_extra_field(ZIPINFO* pz, struct zlist* z, iztimes* z_utim)
{
#ifdef NTSD_EAS
	if(ZipIsWinNT())
	{
		// store SECURITY_DECRIPTOR data in local header,
		// and size only in central headers
		GetSD(pz, z->name, &z->extra, &z->ext, &z->cextra, &z->cext);
	}
#endif /* NTSD_EAS */

#ifdef USE_EF_UT_TIME
	// store extended time stamps in both headers
	return GetExtraTime(z, z_utim);
#else /* !USE_EF_UT_TIME */
	return ZE_OK;
#endif /* ?USE_EF_UT_TIME */
}

int deletedir(char* d)
/* d: directory to delete */
/* Delete the directory *d if it is empty, do nothing otherwise.
   Return the result of rmdir(), delete(), or system().
   For VMS, d must be in format [x.y]z.dir;1  (not [x.y.z]).
 */
{
    return rmdir(d);
}

#endif /* !UTIL */
