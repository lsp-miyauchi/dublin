/*

 Copyright (C) 1990-1997 Mark Adler, Richard B. Wales, Jean-loup Gailly,
 Kai Uwe Rommel, Onno van der Linden and Igor Mandrichenko.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as all of the original files are included,
 that it is not sold for profit, and that this copyright notice is retained.

*/

/*
 *  zipfile.c by Mark Adler.
 */

#define NOCPYRT         /* this is not a main module */
#include "zip.h"
#include "revision.h"

/*
 * XXX start of zipfile.h
 */
/* Macros for converting integers in little-endian to machine format */
#define SH(a) (((ush)(uch)(a)[0]) | (((ush)(uch)(a)[1]) << 8))
#define LG(a) ((ulg)SH(a) | ((ulg)SH((a)+2) << 16))

/* Macros for writing machine integers to little-endian format */
#define PUTSH(a,f) {putc((char)((a) & 0xff),(f)); putc((char)((a) >> 8),(f));}
#define PUTLG(a,f) {PUTSH((a) & 0xffff,(f)) PUTSH((a) >> 16,(f))}


/* -- Structure of a ZIP file -- */

/* Signatures for zip file information headers */
#define LOCSIG     0x04034b50L
#define CENSIG     0x02014b50L
#define ENDSIG     0x06054b50L
#define EXTLOCSIG  0x08074b50L

/* Offsets of values in headers */
#define LOCVER  0               /* version needed to extract */
#define LOCFLG  2               /* encrypt, deflate flags */
#define LOCHOW  4               /* compression method */
#define LOCTIM  6               /* last modified file time, DOS format */
#define LOCDAT  8               /* last modified file date, DOS format */
#define LOCCRC  10              /* uncompressed crc-32 for file */
#define LOCSIZ  14              /* compressed size in zip file */
#define LOCLEN  18              /* uncompressed size */
#define LOCNAM  22              /* length of filename */
#define LOCEXT  24              /* length of extra field */

#define EXTCRC  0               /* uncompressed crc-32 for file */
#define EXTSIZ  4               /* compressed size in zip file */
#define EXTLEN  8               /* uncompressed size */

#define CENVEM  0               /* version made by */
#define CENVER  2               /* version needed to extract */
#define CENFLG  4               /* encrypt, deflate flags */
#define CENHOW  6               /* compression method */
#define CENTIM  8               /* last modified file time, DOS format */
#define CENDAT  10              /* last modified file date, DOS format */
#define CENCRC  12              /* uncompressed crc-32 for file */
#define CENSIZ  16              /* compressed size in zip file */
#define CENLEN  20              /* uncompressed size */
#define CENNAM  24              /* length of filename */
#define CENEXT  26              /* length of extra field */
#define CENCOM  28              /* file comment length */
#define CENDSK  30              /* disk number start */
#define CENATT  32              /* internal file attributes */
#define CENATX  34              /* external file attributes */
#define CENOFF  38              /* relative offset of local header */

#define ENDDSK  0               /* number of this disk */
#define ENDBEG  2               /* number of the starting disk */
#define ENDSUB  4               /* entries on this disk */
#define ENDTOT  6               /* total number of entries */
#define ENDSIZ  8               /* size of entire central directory */
#define ENDOFF  12              /* offset of central on starting disk */
#define ENDCOM  16              /* length of zip file comment */



/* Local functions */

local int zqcmp OF((ZCONST zvoid *, ZCONST zvoid*));
local void zipoddities OF((ZIPINFO*, struct zlist*));
local int readzipfile2 OF((ZIPINFO* pz));
local int rqcmp OF((ZCONST zvoid *, ZCONST zvoid*));
local int zbcmp OF((ZCONST zvoid *, ZCONST zvoid*));
local int ef_scan_ut_time OF((char *ef_buf, extent ef_len, int ef_is_cent, iztimes *z_utim));
local void cutpath OF((char *p));

/*
 * XXX end of zipfile.h
 */

/* Local data */


////////////////////////////////////////////////////////////////////////
// Used by qsort() to compare entries in the zfile list.
// Compares the internal names z->iname

local int zqcmp(ZCONST zvoid* a, ZCONST zvoid* b)
/* pointers to pointers to zip entries */
{
  return namecmp((*(struct zlist**)a)->iname,
                 (*(struct zlist**)b)->iname);
}

////////////////////////////////////////////////////////////////////////
// Used by qsort() to compare entries in the zfile list.
// Compare the internal names z->iname, but in reverse order.

local int rqcmp(ZCONST zvoid* a, ZCONST zvoid* b)
/* pointers to pointers to zip entries */
{
  return namecmp((*(struct zlist**)b)->iname,
                 (*(struct zlist**)a)->iname);
}

////////////////////////////////////////////////////////////////////////////////
// Used by search() to compare a target to an entry in the zfile list.

local int zbcmp(ZCONST zvoid* n, ZCONST zvoid* z)
/* n: string to search for */
/* z: pointer to a pointer to a zip entry */
{
	return namecmp((char*)n, ((struct zlist*)z)->zname);
}

////////////////////////////////////////////////////////////////////////
// Return a pointer to the entry in zfile with the name n, or NULL if
// not found.

struct zlist* zsearch(ZIPINFO* pz, char* n)
{
	zvoid** p;        /* result of search() */

	if (pz->zcount && (p = search(n, (zvoid**)pz->zsort, pz->zcount, zbcmp)) != NULL)
		return *(struct zlist**)p;
	else
		return NULL;
}

////////////////////////////////////////////////////////////////////////
// If the file name *s has a dot (other than the first char), or if
// the -A option is used (adjust self-extracting file) then return
// the name, otherwise append .zip to the name.  Allocate the space for
// the name in either case.  Return a pointer to the new name, or NULL
// if malloc() fails.

#define PATHCUT '/'

char *ziptyp(ZIPINFO* pz, char* s)
{
	char *q;              /* temporary pointer */
	char *t;              /* pointer to malloc'ed string */

	if ((t = (char*)malloc(lstrlen(s) + 5)) == NULL)
		return NULL;
	lstrcpy(t, s);
	replacechar(t, '\\', '/');
	if (pz->adjust)
		return t;
	if (strrchr((q = strrchr(t, PATHCUT)) == NULL ? t : q + 1, '.') == NULL)
		lstrcat(t, ".zip");
	return t;
}


local void zipoddities(ZIPINFO* pz, struct zlist* z)
{
   if ((z->vem >> 8) >= NUM_HOSTS)
   {
     sprintf(errbuf, "made by version %d.%d on system type %d: ",
             (ush)(z->vem & 0xff) / (ush)10, (ush)(z->vem & 0xff) % (ush)10,
             z->vem >> 8);
     zipwarn(pz, errbuf, z->zname);
   }
   if (z->ver != 10 && z->ver != 11 && z->ver != 20)
   {
       sprintf(errbuf, "needs unzip %d.%d on system type %d: ",
               (ush)(z->ver & 0xff) / (ush)10,
               (ush)(z->ver & 0xff) % (ush)10, z->ver >> 8);
       zipwarn(pz, errbuf, z->zname);
    }
    if (z->flg != z->lflg)
    {
       sprintf(errbuf, "local flags = 0x%04x, central = 0x%04x: ",
               z->lflg, z->flg);
       zipwarn(pz, errbuf, z->zname);
     }
     else if (z->flg & ~0xf)
     {
       sprintf(errbuf, "undefined bits used in flags = 0x%04x: ", z->flg);
       zipwarn(pz, errbuf, z->zname);
     }
     if (z->how > DEFLATE)
     {
       sprintf(errbuf, "unknown compression method %u: ", z->how);
       zipwarn(pz, errbuf, z->zname);
     }
     if (z->dsk)
     {
       sprintf(errbuf, "starts on disk %u: ", z->dsk);
       zipwarn(pz, errbuf, z->zname);
      }
      if (z->att!=ASCII && z->att!=BINARY && z->att!=__EBCDIC)
      {
        sprintf(errbuf, "unknown internal attributes = 0x%04x: ", z->att);
        zipwarn(pz, errbuf, z->zname);
      }
     if (z->ext || z->cext)
       if (z->ext && z->cext && z->extra != z->cextra)
       {
         sprintf(errbuf,
                 "local extra (%ld bytes) != central extra (%ld bytes): ",
                 (ulg)z->ext, (ulg)z->cext);
          zipwarn(pz, errbuf, z->zname);
       }
       else if (pz->noisy)
        {
        fprintf(stderr, "zip info: %s has %ld bytes of %sextra data\n",
                z->zname, z->ext ? (ulg)z->ext : (ulg)z->cext,
                z->ext ? (z->cext ? "" : "local ") : "central ");
        }
}

/*
 * readzipfile2 is called with zip -F or zip -FF
 * read the file from front to back and pick up the pieces
 * NOTE: there are still checks missing to see if the header
 *       that was found is *VALID*
 */
local int readzipfile2(ZIPINFO* pz)
/*
   The name of the zip file is pointed to by the global "zipfile".  The
   globals zfiles, zcount, zcomlen, zcomment, and zsort are filled in.
   Return an error code in the ZE_ class.
*/
{
  ulg a = 0L;           /* attributes returned by filetime() */
  char b[CENHEAD];      /* buffer for central headers */
  FILE *f;              /* zip file */
  ush flg;              /* general purpose bit flag */
  int m;                /* mismatch flag */
  extent n;             /* length of name */
  ulg p;                /* current file offset */
  ulg s;                /* size of data, start of central */
  struct zlist** x;    /* pointer last entry's link */
  struct zlist* z;  /* current zip entry structure */

  /* Initialize zip file info */
  pz->zipbeg = 0;
  pz->zfiles = NULL;                        /* Points to first header */
  pz->zcomlen = 0;                          /* zip file comment length */

  /* If zip file exists, read headers and check structure */
  if (pz->zipfile != NULL && *pz->zipfile && (f = fopen(pz->zipfile, FOPR)) != NULL)
  {
    /* Get any file attribute valid for this OS, to set in the central
     * directory when fixing the archive:
     */
    filetime(pz->zipfile, &a, (long*)&s, NULL);
    x = &pz->zfiles;                        /* first link */
    p = 0;                              /* starting file offset */
    pz->zcount = 0;                         /* number of files */

    /* Find start of zip structures */
    for (;;) {
      while ((m = getc(f)) != EOF && m != 0x50)    /* 0x50 == 'P' */
      {
        p++;
      }
      b[0] = (char) m;
      if (fread(b+1, 3, 1, f) != 1 || (s = LG(b)) == LOCSIG || s == ENDSIG)
        break;
      if (fseek(f, -3L, SEEK_CUR))
        return ferror(f) ? ZE_READ : ZE_EOF;
      p++;
    }
    pz->zipbeg = p;

    /* Read local headers */
    while (LG(b) == LOCSIG)
    {
      if ((z = (struct zlist*)malloc(sizeof(struct zlist))) == NULL)
        return ZE_MEM;
      if (fread(b, LOCHEAD, 1, f) != 1) {
          free(z);
          break;
      }

      z->ver = SH(LOCVER + b);
      z->vem = pz->dosify ? 20 : OS_CODE + Z_MAJORVER * 10 + Z_MINORVER;
      z->dosflag = pz->dosify;
      flg = z->flg = z->lflg = SH(LOCFLG + b);
      z->how = SH(LOCHOW + b);
      z->tim = LG(LOCTIM + b);          /* time and date into one long */
      z->crc = LG(LOCCRC + b);
      z->siz = LG(LOCSIZ + b);
      z->len = LG(LOCLEN + b);
      n = z->nam = SH(LOCNAM + b);
      z->cext = z->ext = SH(LOCEXT + b);

      z->com = 0;
      z->dsk = 0;
      z->att = 0;
      z->atx = pz->dosify ? a & 0xff : a;     /* Attributes from filetime() */
      z->mark = 0;
      z->trash = 0;

      s = pz->fix > 1 ? 0L : z->siz; /* discard compressed size with -FF */

      /* Initialize all fields pointing to malloced data to NULL */
      z->zname = z->name = z->iname = z->extra = z->cextra = z->comment = NULL;

      /* Link into list */
      *x = z;
      z->nxt = NULL;
      x = &z->nxt;

      /* Read file name and extra field and skip data */
      if (n == 0)
      {
        sprintf(errbuf, "%ld", (ulg)pz->zcount + 1);
        zipwarn(pz, "zero-length name for entry #", errbuf);
        return ZE_FORM;
      }
      if ((z->iname = (char*)malloc(n+1)) ==  NULL ||
          (z->ext && (z->extra = (char*)malloc(z->ext)) == NULL))
        return ZE_MEM;
      if (fread(z->iname, n, 1, f) != 1 ||
          (z->ext && fread(z->extra, z->ext, 1, f) != 1) ||
          (s && fseek(f, (long)s, SEEK_CUR)))
        return ferror(f) ? ZE_READ : ZE_EOF;
      /* If there is an extended local header, s is either 0 or
       * the correct compressed size.
       */
      z->iname[n] = 0;                  /* terminate name */
      z->zname = in2ex(pz, z->iname);       /* convert to external name */
      if (z->zname == NULL)
        return ZE_MEM;
      z->name = z->zname;
      z->cextra = z->extra;
      if (pz->noisy)
		  fprintf(pz->mesg, "zip: reading %s\n", z->zname);

      /* Save offset, update for next header */
      z->off = p;
      p += 4 + LOCHEAD + n + z->ext + s;
      pz->zcount++;

      /* Skip extended local header if there is one */
      if ((flg & 8) != 0) {
        /* Skip the compressed data if compressed size is unknown.
         * For safety, we should use the central directory.
         */
        if (s == 0) {
          for (;;) {
            while ((m = getc(f)) != EOF && m != 0x50) ;  /* 0x50 == 'P' */
            b[0] = (char) m;
            if (fread(b+1, 15, 1, f) != 1 || LG(b) == EXTLOCSIG)
              break;
            if (fseek(f, -15L, SEEK_CUR))
              return ferror(f) ? ZE_READ : ZE_EOF;
          }
          s = LG(4 + EXTSIZ + b);
          p += s;
          if ((ulg) ftell(f) != p+16L) {
            zipwarn(pz, "bad extended local header for ", z->zname);
            return ZE_FORM;
          }
        } else {
          /* compressed size non-zero, assume that it is valid: */

          if (fseek(f, p, SEEK_SET) || fread(b, 16, 1, f) != 1)
            return ferror(f) ? ZE_READ : ZE_EOF;
          if (LG(b) != EXTLOCSIG) {
            zipwarn(pz, "extended local header not found for ", z->zname);
            return ZE_FORM;
          }
        }
        /* overwrite the unknown values of the local header: */

        /* already in host format */
        z->crc = LG(4 + EXTCRC + b);
        z->siz = s;
        z->len = LG(4 + EXTLEN + b);

        p += 16L;
      }
      else if (pz->fix > 1) {
        /* Don't trust the compressed size */
        for (;;) {
          while ((m = getc(f)) != EOF && m != 0x50) p++; /* 0x50 == 'P' */
          b[0] = (char) m;
          if (fread(b+1, 3, 1, f) != 1 || (s = LG(b)) == LOCSIG || s == CENSIG)
            break;
          if (fseek(f, -3L, SEEK_CUR))
            return ferror(f) ? ZE_READ : ZE_EOF;
          p++;
        }
        s = p - (z->off + 4 + LOCHEAD + n + z->ext);
        if (s != z->siz) {
          fprintf(pz->mesg, " compressed size %ld, actual size %ld for %s\n",
                  z->siz, s, z->zname);
          z->siz = s;
        }
        /* next LOCSIG already read at this point, don't read it again: */
        continue;
      }

      /* Read next signature */
      if (fread(b, 4, 1, f) != 1)
          break;
    }

    s = p;                              /* save start of central */

    if (LG(b) != CENSIG && pz->noisy) {
      fprintf(pz->mesg, "zip warning: %s %s truncated.\n", pz->zipfile,
              pz->fix > 1 ? "has been" : "would be");

      if (pz->fix == 1) {
        fprintf(pz->mesg,
   "Retry with option -qF to truncate, with -FF to attempt full recovery\n");
        ZIPERR(pz, ZE_FORM, NULL);
      }
    }

    pz->cenbeg = s;
    pz->zcomlen = 0;

    if (pz->zipbeg && pz->noisy)
      fprintf(pz->mesg, "%s: adjusting offsets for a preamble of %lu bytes\n",
              pz->zipfile, pz->zipbeg);

    /* Done with zip file for now */
    fclose(f);

    /* If one or more files, sort by name */
    if (pz->zcount)
    {
      if ((x = pz->zsort =
          (struct zlist**)malloc(pz->zcount * sizeof(struct zlist*))) ==
          NULL)
        return ZE_MEM;
      for (z = pz->zfiles; z != NULL; z = z->nxt)
        *x++ = z;
      qsort((char *)pz->zsort, pz->zcount, sizeof(struct zlist*), zqcmp);
    }
  }
  return ZE_OK;
}

/*
 * readzipfile starts searching for the End Signature at the end of the file
 * The End Signature points to the Central Directory Signature which points
 * to the Local Directory Signature
 * XXX probably some more consistency checks are needed
 */
int readzipfile(ZIPINFO* pz)
/*
   The name of the zip file is pointed to by the global "zipfile".  The
   globals zfiles, zcount, zcomlen, zcomment, and zsort are filled in.
   Return an error code in the ZE_ class.
*/
{
  char b[CENHEAD];      /* buffer for central headers */
  FILE *f;              /* zip file */
  ush flg;              /* general purpose bit flag */
  int m;                /* mismatch flag */
  extent n;             /* length of name */
  struct zlist** x;    /* pointer last entry's link */
  struct zlist* z;  /* current zip entry structure */
  char *t;              /* temporary pointer */
  char *u;          /* temporary variable */
  int found;
  char *buf;            /* temp buffer for reading zipfile */
  long deltaoff;

  if (pz->fix && !pz->adjust)
     return readzipfile2(pz);

  /* Initialize zip file info */
  pz->zipbeg = 0;
  pz->zfiles = NULL;                        /* Points to first header */
  pz->zcomlen = 0;                          /* zip file comment length */

  /* If zip file exists, read headers and check structure */
  if (pz->zipfile != NULL && *pz->zipfile && (f = fopen(pz->zipfile, FOPR)) != NULL)
  {
    buf = (char*)malloc(4096 + 4);
    if (buf == NULL)
      return ZE_MEM;

    found = 0;
    t = &buf[4096];
    t[1] = '\0';
    t[2] = '\0';
    t[3] = '\0';
    if (fseek(f, -4096L, SEEK_END) == 0) {
      pz->zipbeg = (ulg) (ftell(f) + 4096L);
      while (!found && pz->zipbeg >= 4096) {
        pz->zipbeg -= 4096L;
        buf[4096] = t[1];
        buf[4097] = t[2];
        buf[4098] = t[3];
/*
 * XXX error check ??
 */
        fread(buf, 1, 4096, f);
        fseek(f, -8192L, SEEK_CUR);
        t = &buf[4095];
/*
 * XXX far pointer arithmetic in DOS
 */
        while (t >= buf) {
          /* Check for ENDSIG ("PK\5\6" in ASCII) */
          if (LG(t) == ENDSIG) {
            found = 1;
/*
 * XXX error check ??
 * XXX far pointer arithmetic in DOS
 */
            pz->zipbeg += (ulg) (t - buf);
            fseek(f, (long) pz->zipbeg + 4L, SEEK_SET);
            break;
          }
          --t;
        }
      }
    }
    else
       pz->zipbeg = 4096L;
/*
 * XXX warn: garbage at the end of the file ignored
 */
    if (!found && pz->zipbeg > 0) {
      size_t s;

      rewind(f);
      s = fread(buf, 1, (size_t)pz->zipbeg, f);
      buf[s] = t[1];
      buf[s + 1] = t[2];
      buf[s + 2] = t[3];
      t = &buf[s - 1];
/*
 * XXX far pointer comparison in DOS
 */
      while (t >= buf) {
        /* Check for ENDSIG ("PK\5\6" in ASCII) */
        if (LG(t) == ENDSIG) {
          found = 1;
/*
 * XXX far pointer arithmetic in DOS
 */
          pz->zipbeg = (ulg) (t - buf);
          fseek(f, (long)pz->zipbeg + 4L, SEEK_SET);
          break;
        }
        --t;
      }
    }
    free(buf);
    if (!found) {
      zipwarn(pz, "missing end signature--probably not a zip file (did you", "");
      zipwarn(pz, "remember to use binary mode when you transferred it?)", "");
      return ZE_FORM;
    }
    /* Read end header */
    if (fread(b, ENDHEAD, 1, f) != 1)
      return ferror(f) ? ZE_READ : ZE_EOF;
    if (SH(ENDDSK + b) || SH(ENDBEG + b) ||
        SH(ENDSUB + b) != SH(ENDTOT + b))
      zipwarn(pz, "multiple disk information ignored", "");
    pz->zcomlen = SH(ENDCOM + b);
    if (pz->zcomlen)
    {
      if ((pz->zcomment = (char*)malloc(pz->zcomlen)) == NULL)
        return ZE_MEM;
      if (fread(pz->zcomment, pz->zcomlen, 1, f) != 1)
      {
        free(pz->zcomment);
        pz->zcomment = NULL;
        return ferror(f) ? ZE_READ : ZE_EOF;
      }
    }
/*
 * XXX assumes central header immediately precedes end header
 */
    pz->cenbeg = pz->zipbeg - LG(ENDSIZ + b);
    deltaoff = pz->adjust ? pz->cenbeg - LG(b + ENDOFF) : 0L;
    if (fseek(f, pz->cenbeg, SEEK_SET) != 0) {
        perror("fseek");
        return ZE_FORM; /* XXX */
    }

    x = &pz->zfiles;                        /* first link */
    pz->zcount = 0;                         /* number of files */

    if (fread(b, 4, 1, f) != 1)
      return ferror(f) ? ZE_READ : ZE_EOF;

    while (LG(b) == CENSIG) {
      /* Read central header. The portion of the central header that should
         be in common with local header is read raw, for later comparison.
         (this requires that the offset of ext in the zlist structure
         be greater than or equal to LOCHEAD) */
      if (fread(b, CENHEAD, 1, f) != 1)
        return ferror(f) ? ZE_READ : ZE_EOF;
      if ((z = (struct zlist*)malloc(sizeof(struct zlist))) == NULL)
        return ZE_MEM;
      z->vem = SH(CENVEM + b);
      for (u = (char*)(&(z->ver)), n = 0; n < (CENNAM-CENVER); n++)
        u[n] = b[CENVER + n];
      z->nam = SH(CENNAM + b);          /* used before comparing cen vs. loc */
      z->cext = SH(CENEXT + b);         /* may be different from z->ext */
      z->com = SH(CENCOM + b);
      z->dsk = SH(CENDSK + b);
      z->att = SH(CENATT + b);
      z->atx = LG(CENATX + b);
      z->off = LG(CENOFF + b) + deltaoff;
      z->dosflag = (z->vem & 0xff00) == 0;

      /* Initialize all fields pointing to malloced data to NULL */
      z->zname = z->name = z->iname = z->extra = z->cextra = z->comment = NULL;

      /* Link into list */
      *x = z;
      z->nxt = NULL;
      x = &z->nxt;

      /* Read file name, extra field and comment field */
      if (z->nam == 0)
      {
        sprintf(errbuf, "%lu", (ulg)pz->zcount + 1);
        zipwarn(pz, "zero-length name for entry #", errbuf);
        free(z);
        return ZE_FORM;
      }
      if ((z->iname = (char*)malloc(z->nam+1)) ==  NULL ||
          (z->cext && (z->cextra = (char*)malloc(z->cext)) == NULL) ||
          (z->com && (z->comment = (char*)malloc(z->com)) == NULL))
        return ZE_MEM;
      if (fread(z->iname, z->nam, 1, f) != 1 ||
          (z->cext && fread(z->cextra, z->cext, 1, f) != 1) ||
          (z->com && fread(z->comment, z->com, 1, f) != 1))
        return ferror(f) ? ZE_READ : ZE_EOF;
      z->iname[z->nam] = 0;                  /* terminate name */
      /* Update zipbeg offset, prepare for next header */
      if (z->off < pz->zipbeg)
         pz->zipbeg = z->off;
      pz->zcount++;
      /* Read next signature */
      if (fread(b, 4, 1, f) != 1)
          return ferror(f) ? ZE_READ : ZE_EOF;
    }

    /* Point to start of header list and read local headers */
    z = pz->zfiles;
    while (z != NULL) {
      /* Read next signature */
      if (fseek(f, z->off, SEEK_SET) != 0 || fread(b, 4, 1, f) != 1)
        return ferror(f) ? ZE_READ : ZE_EOF;
      if (LG(b) == LOCSIG) {
        if (fread(b, LOCHEAD, 1, f) != 1)
            return ferror(f) ? ZE_READ : ZE_EOF;
        z->lflg = SH(LOCFLG + b);
        n = SH(LOCNAM + b);
        z->ext = SH(LOCEXT + b);

        /* Compare name and extra fields */
        if (n != z->nam)
        {
          zipwarn(pz, "name lengths in local and central differ for ", z->iname);
          return ZE_FORM;
        }
        if ((t = (char*)malloc(z->nam)) == NULL)
          return ZE_MEM;
        if (fread(t, z->nam, 1, f) != 1)
        {
          free(t);
          return ferror(f) ? ZE_READ : ZE_EOF;
        }
        if (memcmp(t, z->iname, z->nam))
        {
          free(t);
          zipwarn(pz, "names in local and central differ for ", z->iname);
          return ZE_FORM;
        }
        free(t);
        if (z->ext)
        {
          if ((z->extra = (char*)malloc(z->ext)) == NULL)
            return ZE_MEM;
          if (fread(z->extra, z->ext, 1, f) != 1)
          {
            free(z->extra);
            return ferror(f) ? ZE_READ : ZE_EOF;
          }
          if (z->ext == z->cext && memcmp(z->extra, z->cextra, z->ext) == 0)
          {
            free(z->extra);
            z->extra = z->cextra;
          }
        }

        /* Check extended local header if there is one */
        if ((z->lflg & 8) != 0)
        {
          char buf2[16];
          ulg s;                        /* size of compressed data */

          s = LG(LOCSIZ + b);
          if (s == 0)
            s = LG((CENSIZ-CENVER) + (char*)(&(z->ver)));
          if (fseek(f, (z->off + (4+LOCHEAD) + z->nam + z->ext + s), SEEK_SET)
              || (fread(buf2, 16, 1, f) != 1))
            return ferror(f) ? ZE_READ : ZE_EOF;
          if (LG(buf2) != EXTLOCSIG)
          {
            zipwarn(pz, "extended local header not found for ", z->iname);
            return ZE_FORM;
          }
          /* overwrite the unknown values of the local header: */
          for (n = 0; n < 12; n++)
            b[LOCCRC+n] = buf2[4+n];
        }

        /* Compare local header with that part of central header (except
           for the reserved bits in the general purpose flags and except
           for the already checked entry name length */
        u = (char*)(&(z->ver));
        flg = SH((CENFLG-CENVER) + u);          /* Save central flags word */
        u[CENFLG-CENVER+1] &= 0x1f;             /* Mask reserved flag bits */
        b[LOCFLG+1] &= 0x1f;
        for (m = 0, n = 0; n < LOCNAM; n++)
          if (b[n] != u[n])
          {
            if (!m)
            {
              zipwarn(pz, "local and central headers differ for ", z->zname);
              m = 1;
            }
            if (pz->noisy)
            {
              sprintf(errbuf, " offset %u--local = %02x, central = %02x",
                      (unsigned)n, (uch)b[n], (uch)u[n]);
              zipwarn(pz, errbuf, "");
            }
          }
        if (m && !pz->adjust)
          return ZE_FORM;

        /* Complete the setup of the zlist entry by translating the remaining
         * central header fields in memory, starting with the fields with
         * highest offset. This order of the conversion commands takes into
         * account potential buffer overlaps caused by structure padding.
         */
        z->len = LG((CENLEN-CENVER) + u);
        z->siz = LG((CENSIZ-CENVER) + u);
        z->crc = LG((CENCRC-CENVER) + u);
        z->tim = LG((CENTIM-CENVER) + u);   /* time and date into one long */
        z->how = SH((CENHOW-CENVER) + u);
        z->flg = flg;                       /* may be different from z->lflg */
        z->ver = SH((CENVER-CENVER) + u);

        /* Clear actions */
        z->mark = 0;
        z->trash = 0;
/* We only need z->iname in the utils */
        z->name = z->iname;
        z->zname = z->iname;
        z->zname = in2ex(pz, z->iname);       /* convert to external name */
        if (z->zname == NULL)
          return ZE_MEM;
        z->name = z->zname;
      }
      else {
        zipwarn(pz, "local header not found for ", z->iname);
        return ZE_FORM;
      }
      if (pz->verbose)
        zipoddities(pz, z);
      z = z->nxt;
    }

    if (pz->zipbeg && pz->noisy)
      fprintf(pz->mesg, "%s: %s a preamble of %lu bytes\n",
              pz->zipfile, pz->adjust ? "adjusting offsets for" : "found", pz->zipbeg);

    /* Done with zip file for now */
    fclose(f);

    /* If one or more files, sort by name */
    if (pz->zcount)
    {
      if ((x = pz->zsort =
          (struct zlist**)malloc(pz->zcount * sizeof(struct zlist*))) ==
          NULL)
        return ZE_MEM;
      for (z = pz->zfiles; z != NULL; z = z->nxt)
        *x++ = z;
      qsort((char *)pz->zsort, pz->zcount, sizeof(struct zlist*), zqcmp);
    }
  }
  return ZE_OK;
}


int putlocal(struct zlist* z, FILE* f)
/* z: zip entry to write local header for */
/* f: file to write to */
/* Write a local header described by *z to file *f.  Return an error code
   in the ZE_ class. */
{
  PUTLG(LOCSIG, f);
  PUTSH(z->ver, f);
  PUTSH(z->lflg, f);
  PUTSH(z->how, f);
  PUTLG(z->tim, f);
  PUTLG(z->crc, f);
  PUTLG(z->siz, f);
  PUTLG(z->len, f);
  PUTSH(z->nam, f);
  PUTSH(z->ext, f);
  if (fwrite(z->iname, 1, z->nam, f) != z->nam ||
      (z->ext && fwrite(z->extra, 1, z->ext, f) != z->ext))
    return ZE_TEMP;
  return ZE_OK;
}

int putextended(struct zlist* z, FILE* f)
/* z: zip entry to write local header for */
/* f: file to write to */
/* Write an extended local header described by *z to file *f.
 * Return an error code in the ZE_ class. */
{
  PUTLG(EXTLOCSIG, f);
  PUTLG(z->crc, f);
  PUTLG(z->siz, f);
  PUTLG(z->len, f);
  return ZE_OK;
}

//////////////////////////////////////////////////////////////////////
// Write a central header described by *z to file *f.
// Return an error code in the ZE_ class.

int putcentral(struct zlist* z, FILE* f)
/* z: zip entry to write central header for */
/* f: file to write to */
{
	PUTLG(CENSIG, f);
	PUTSH(z->vem, f);
	PUTSH(z->ver, f);
	PUTSH(z->flg, f);
	PUTSH(z->how, f);
	PUTLG(z->tim, f);
	PUTLG(z->crc, f);
	PUTLG(z->siz, f);
	PUTLG(z->len, f);
	PUTSH(z->nam, f);
	PUTSH(z->cext, f);
	PUTSH(z->com, f);
	PUTSH(z->dsk, f);
	PUTSH(z->att, f);
	PUTLG(z->atx, f);
	PUTLG(z->off, f);
	if (fwrite(z->iname, 1, z->nam, f) != z->nam ||
		(z->cext && fwrite(z->cextra, 1, z->cext, f) != z->cext) ||
		(z->com && fwrite(z->comment, 1, z->com, f) != z->com))
		return ZE_TEMP;
	return ZE_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Write the end of central directory data to file *f.  Return an error code
// in the ZE_ class. */

int putend(int n, ulg s, ulg c, extent m, char* z, FILE* f)
/* n: number of entries in central directory */
/* s: size of central directory */
/* c: offset of central directory */
/* m: length of zip file comment (0 if none) */
/* z: zip file comment if m != 0 */
/* f: file to write to */
{
	PUTLG(ENDSIG, f);
	PUTSH(0, f);
	PUTSH(0, f);
	PUTSH(n, f);
	PUTSH(n, f);
	PUTLG(s, f);
	PUTLG(c, f);
	PUTSH(m, f);
	/* Write the comment, if any */
	if (m && fwrite(z, 1, m, f) != m)
		return ZE_TEMP;

	return ZE_OK;
}


/* Note: a zip "entry" includes a local header (which includes the file
   name), an encryption header if encrypting, the compressed data
   and possibly an extended local header. */

////////////////////////////////////////////////////////////////////////
// Copy the zip entry described by *z from file *x to file *y.  Return an
// error code in the ZE_ class.  Also update tempzn by the number of bytes
// copied.

int zipcopy(ZIPINFO* pz, struct zlist* z, FILE* x, FILE* y)
/* z: zip entry to copy */
/* x, y: source and destination files */
{
	ulg n;                /* holds local header offset */

	n = (ulg)(4 + LOCHEAD) + (ulg)z->nam + (ulg)z->ext;

	if (pz->fix > 1)
	{
		if (fseek(x, z->off + n, SEEK_SET)) /* seek to compressed data */
			return ferror(x) ? ZE_READ : ZE_EOF;

		if (pz->fix > 2)
		{
			// Update length of entry's name, it may have been changed.
			// This is needed to support the ZipNote ability to rename archive entries.
			z->nam = lstrlen(z->iname);
			n = (ulg)(4 + LOCHEAD) + (ulg)z->nam + (ulg)z->ext;
		}

		// do not trust the old compressed size
		if (putlocal(z, y) != ZE_OK)
			return ZE_TEMP;

		z->off = pz->tempzn;
		pz->tempzn += n;
		n = z->siz;
	}
	else
	{
		if (fseek(x, z->off, SEEK_SET))     /* seek to local header */
			return ferror(x) ? ZE_READ : ZE_EOF;

		z->off = pz->tempzn;
		n += z->siz;
	}

	// copy the compressed data and the extended local header if there is one
	if (z->lflg & 8) n += 16;
		pz->tempzn += n;
	return fcopy(x, y, n);
}


local int ef_scan_ut_time(char* ef_buf, extent ef_len, int ef_is_cent, iztimes* z_utim)
/* ef_buf: buffer containing extra field */
/* ef_len: total length of extra field */
/* ef_is_cent: flag indicating "is central extra field" */
/* z_utim: return storage: atime, mtime, ctime */
/* This function scans the extra field for EF_TIME or EF_IZUNIX blocks
 * containing Unix style time_t (GMT) values for the entry's access, creation
 * and modification time.
 * If a valid block is found, all time stamps are copied to the iztimes
 * structure.
 * The presence of an EF_TIME or EF_IZUNIX2 block results in ignoring
 * all data from probably present obsolete EF_IZUNIX blocks.
 * If multiple blocks of the same type are found, only the information from
 * the last block is used.
 * The return value is the EF_TIME Flags field (simulated in case of an
 * EF_IZUNIX block) or 0 in case of failure.
 */
{
  int flags = 0;
  unsigned eb_id;
  extent eb_len;
  int have_new_type_eb = FALSE;

  if (ef_len == 0 || ef_buf == NULL)
    return 0;

  while (ef_len >= EB_HEADSIZE) {
    eb_id = SH(EB_ID + ef_buf);
    eb_len = SH(EB_LEN + ef_buf);

    if (eb_len > (ef_len - EB_HEADSIZE)) {
      /* Discovered some extra field inconsistency! */
      break;
    }

    switch (eb_id) {
      case EF_TIME:
        flags &= ~0x00ff;       /* ignore previous IZUNIX or EF_TIME fields */
        have_new_type_eb = TRUE;
        if ( eb_len >= EB_UT_MINLEN && z_utim != NULL) {
           unsigned eb_idx = EB_UT_TIME1;
           flags |= (ef_buf[EB_HEADSIZE+EB_UT_FLAGS] & 0x00ff);
           if ((flags & EB_UT_FL_MTIME)) {
              if ((eb_idx+4) <= eb_len) {
                 z_utim->mtime = LG((EB_HEADSIZE+eb_idx) + ef_buf);
                 eb_idx += 4;
              } else {
                 flags &= ~EB_UT_FL_MTIME;
              }
           }
           if (ef_is_cent) {
              break;            /* central version of TIME field ends here */
           }
           if (flags & EB_UT_FL_ATIME) {
              if ((eb_idx+4) <= eb_len) {
                 z_utim->atime = LG((EB_HEADSIZE+eb_idx) + ef_buf);
                 eb_idx += 4;
              } else {
                 flags &= ~EB_UT_FL_ATIME;
              }
           }
           if (flags & EB_UT_FL_CTIME) {
              if ((eb_idx+4) <= eb_len) {
                 z_utim->ctime = LG((EB_HEADSIZE+eb_idx) + ef_buf);
                 eb_idx += 4;
              } else {
                 flags &= ~EB_UT_FL_CTIME;
              }
           }
        }
        break;

      case EF_IZUNIX2:
        if (!have_new_type_eb) {
           flags &= ~0x00ff;    /* ignore any previous IZUNIX field */
           have_new_type_eb = TRUE;
        }
        break;

      case EF_IZUNIX:
        if (eb_len >= EB_UX_MINLEN) {
           if (have_new_type_eb) {
              break;            /* Ignore IZUNIX extra field block ! */
           }
           z_utim->atime = LG((EB_HEADSIZE+EB_UX_ATIME) + ef_buf);
           z_utim->mtime = LG((EB_HEADSIZE+EB_UX_MTIME) + ef_buf);
           flags |= (EB_UT_FL_MTIME | EB_UT_FL_ATIME);  /* signal success */
        }
        break;

      default:
        break;
    }
    /* Skip this extra field block */
    ef_buf += (eb_len + EB_HEADSIZE);
    ef_len -= (eb_len + EB_HEADSIZE);
  }

  return flags;
}

int get_ef_ut_ztime(struct zlist* z, iztimes* z_utim)
{
  int r;

  /* First, scan local extra field. */
  r = ef_scan_ut_time(z->extra, z->ext, FALSE, z_utim);

  /* If this was not successful, try central extra field, but only if
     it is really different. */
  if (!r && z->cext > 0 && z->cextra != z->extra)
    r = ef_scan_ut_time(z->cextra, z->cext, TRUE, z_utim);

  return r;
}


///////////////////////////////////////////////////////////////////////
// Cut the last path component off the name *p in place.
// This should work on both internal and external names.

local void cutpath(char* path)
{
	char *r;              /* pointer to last path delimiter */
	if ((r = strrchr(path, '/')) != NULL)
		*r = 0;
	else
		*path = 0;
}

////////////////////////////////////////////////////////////////////////
// Delete the compressed files and the directories that contained the
// deleted files, if empty.  Return an error code in the ZE_ class.
// Failure of destroy() or deletedir() is ignored.

int trash(ZIPINFO* pz)
{
	extent i;             /* counter on deleted names */
	extent n;             /* number of directories to delete */
	struct zlist** s; /* table of zip entries to handle, sorted */
	struct zlist* z;  /* current zip entry */

	/* Delete marked names and count directories */
	n = 0;
	for (z = pz->zfiles; z != NULL; z = z->nxt)
	{
		if (z->mark == 1 || z->trash)
		{
			z->mark = 1;

			/* don't unlink directory */
			if (z->iname[z->nam - 1] != '/')
			{
				if (pz->verbose)
					fprintf(pz->mesg, "zip diagnostic: deleting file %s\n", z->name);
				if (destroy(z->name))
				{
					zipwarn(pz, "error deleting ", z->name);
				}

				// Try to delete all paths that lead up to marked names. This is
				// necessary only with the -D option.
				if (!pz->dirnames)
				{
					cutpath(z->name);  /* XXX wrong ??? */
					cutpath(z->iname);
					if (z->iname[0] != '\0')
					{
						lstrcat(z->iname, "/");
					}
					z->nam = lstrlen(z->iname);
					if (z->nam > 0)
						n++;
				}
			}
			else
				n++;
		}
	}

	// Construct the list of all marked directories. Some may be duplicated
	// if -D was used.
	if (n)
	{
		if ((s = (struct zlist**)malloc(n*sizeof(struct zlist*))) == NULL)
			return ZE_MEM;
		n = 0;
		for (z = pz->zfiles; z != NULL; z = z->nxt)
		{
			if (z->mark && z->nam > 0 && z->iname[z->nam - 1] == '/'
				&& (n == 0 || strcmp(z->name, s[n-1]->name) != 0))
			{
				s[n++] = z;
			}
		}

		// Sort the files in reverse order to get subdirectories first.
		// To avoid problems with strange naming conventions as in VMS,
		// we sort on the internal names, so x/y/z will always be removed
		// before x/y. On VMS, x/y/z > x/y but [x.y.z] < [x.y]
		qsort((char *)s, n, sizeof(struct zlist*), rqcmp);

		for (i = 0; i < n; i++)
		{
			char *p = s[i]->name;
			if (*p == '\0')
				continue;
			if (p[lstrlen(p) - 1] == '/')
			{ /* keep VMS [x.y]z.dir;1 intact */
				p[lstrlen(p) - 1] = '\0';
			}
			if (i == 0 || strcmp(s[i]->name, s[i-1]->name) != 0)
			{
				if (pz->verbose)
				{
					fprintf(pz->mesg,
							"deleting directory %s (if empty)                \n",
							s[i]->name);
				}
				deletedir(s[i]->name);
			}
		}
		free(s);
	}
	return ZE_OK;
}
