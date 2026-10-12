/*

 Copyright (C) 1990-1997 Mark Adler, Richard B. Wales, Jean-loup Gailly,
 Kai Uwe Rommel, Onno van der Linden and Igor Mandrichenko.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as all of the original files are included,
 that it is not sold for profit, and that this copyright notice is retained.

*/

/*
 *  zipup.c by Mark Adler and Jean-loup Gailly.
 */

#define NOCPYRT         /* this is not a main module */
#include <ctype.h>
#include "zip.h"

#ifndef UTIL            /* This module contains no code for Zip Utilities */

#include "revision.h"
#include "crypt.h"


/* Use the raw functions for MSDOS and Unix to save on buffer space.
   They're not used for VMS since it doesn't work (raw is weird on VMS).
   (This sort of stuff belongs in fileio.c, but oh well.) */

#ifdef WIN32
#  include "zipup.h"
#  include "implode.h"
	typedef struct tagCMPPRM {
		ulg crc_in;
		ulg crc_out;
		ulg totalread;
		ulg totalwrite;
		ZIPINFO* pz;
		FILE* fpIn;
		FILE* fpOut;
	} CMPPRM;
#endif
#ifdef WINDLL
#  include "windll.h"
#endif

/* Deflate "internal" global data (currently not in zip.h) */
extern ulg window_size;         /* size of said window */

/* Local data */

  local ulg crc;       /* crc on uncompressed file data */
  local ftype ifile;   /* file to compress */
  local ulg isize;     /* input file size. */

/* Local functions */
   local int suffixes OF((char *, char *));


local unsigned int impdcl_read(char* pdata, unsigned int* size, void* pv)
{
	CMPPRM* pcp = (CMPPRM*)pv;
	unsigned len = zread(ifile, pdata, *size);
	if (len == (unsigned)EOF || len == 0)
		return 0;

	pcp->crc_in = uzp_crc32(pcp->crc_in, (uch*)pdata, len);
	pcp->totalread += (ulg)len;
	if ((*pcp->pz->pzcl->pfnReport)(pcp->pz->pzcl, ZP_ST_IN_PROGRESS, 0, pcp->pz->zipfile, pcp->pz->curfile, len) == ZP_ST_BREAK)
		return 0;
	return (unsigned int)len;
}

void impdcl_write(char* pdata, unsigned int* size, void* pv)
{
	CMPPRM* pcp = (CMPPRM*)pv;
	pcp->crc_out = uzp_crc32(pcp->crc_out, (uch*)pdata, *size);
	pcp->totalwrite += zfwrite(pcp->pz, pdata, 1, *size, pcp->fpOut);
}

////////////////////////////////////////////////////////////////////////
// Return the percentage compression from n to m using only integer
// operations
int percent(ulg n, ulg m)
/* n is the original size, m is the new size */
{
	if (n > 0xffffffL)            /* If n >= 16M */
	{                             /*  then divide n and m by 256 */
		n += 0x80;  n >>= 8;
		m += 0x80;  m >>= 8;
	}
	return n > m ? (int)(1 + (200 * (n - m)/n)) / 2 : 0;
}

///////////////////////////////////////////////////////////////////////
// Return true if a ends in any of the suffixes in the list s.

local int suffixes(char* a, char* s)
/* a: name to check suffix of */
/* s: list of suffixes separated by : or ; */
{
	int m;                /* true if suffix matches so far */
	char *p;              /* pointer into special */
	char *q;              /* pointer into name a */

	m = 1;
	q = a + lstrlen(a) - 1;
	for (p = s + lstrlen(s) - 1; p >= s; p--)
		if (*p == ':' || *p == ';')
			if (m)
				return 1;
			else
			{
				m = 1;
				q = a + lstrlen(a) - 1;
			}
		else
		{
			m = m && q >= a && case_map(*p) == case_map(*q);
			q--;
		}
	return m;
}

////////////////////////////////////////////////////////////////////////
// Note: a zip "entry" includes a local header (which includes the file
// name), an encryption header if encrypting, the compressed data
// and possibly an extended local header.

////////////////////////////////////////////////////////////////////////
// Compress the file z->name into the zip entry described by *z and write
// it to the file *y. Encrypt if requested.  Return an error code in the
// ZE_ class.  Also, update tempzn by the number of bytes written.

int zipup(ZIPINFO* pz, struct zlist* z, FILE* y)
/* z: zip entry to compress */
/* y: output file */
{
	iztimes f_utim;       /* UNIX GMT timestamps, filled by filetime() */
	ulg tim;              /* time returned by filetime() */
	ulg a = 0L;           /* attributes returned by filetime() */
	char *pbuf;              /* malloc'ed file buffer */
	extent k = 0;         /* result of zread */
	int m;                /* method for this entry */
	ulg off, poff;        /* offsets in zip file */
	long q = -3L;         /* size returned by filetime */
	int r;                /* temporary variable */
	ulg s = 0L;           /* size of compressed data */
	BOOL bDir;            /* set for a directory name */
	int set_type = 0;     /* set if file type (ascii/binary) unknown */

	pz->curfile = NULL;

	z->nam = lstrlen(z->iname);
	bDir = z->iname[z->nam-1] == '/';

	if ((tim = filetime(z->name, &a, &q, &f_utim)) == 0 || q < -2L)
		return ZE_OPEN;

	// q is set to -1 if the input file is a device, -2 for a volume label
	if (q == -2L)
	{
		bDir = 1;
		q = 0;
	}
	else if (bDir != ((a & MSDOS_DIR_ATTR) != 0))
	{
		/* don't overwrite a directory with a file and vice-versa */
		return ZE_MISS;
	}
	z->att = (ush)UNKNOWN; /* will be changed later */
	z->atx = 0; /* may be changed by set_extra_field() */

	/* Free the old extra fields which are probably obsolete */
	if (z->ext)
		free(z->extra);
	if (z->cext && z->extra != z->cextra)
		free(z->cextra);
	z->extra = z->cextra = NULL;
	z->ext = z->cext = 0;

	window_size = 0L;

	// Select method based on the suffix and the global method
	m = pz->method;

	// Open file to zip up unless it is stdin
	if (pz->extra_fields)
	{
		// create extra field and change z->att and z->atx if desired
		set_extra_field(pz, z, &f_utim);
	}

	pz->curfile = z->name;
	if (bDir) 		/* directory */
	{
		ifile = fbad;
		m = STORE;
		q = 0;
	}
	else
	{
		if ((ifile = zopen(z->name, fhow)) == fbad)
			return ZE_OPEN;
#ifdef WINDLL
		if ((*pz->pzcl->pfnReport)(pz->pzcl, ZP_ST_START_COMPRESS, pz->action, pz->zipfile, z->zname, q) == ZP_ST_BREAK)
			return ZE_ABORT;
#endif
	}

	z->tim = tim;

	if (q == 0)
		m = STORE;
	if (m == BEST)
		m = DEFLATE;

	/* Do not create STORED files with extended local headers if the
	 * input size is not known, because such files could not be extracted.
	 * So if the zip file is not seekable and the input file is not
	 * on disk, obey the -0 option by forcing deflation with stored block.
	 * Note however that using "zip -0" as filter is not very useful...
	 * ??? to be done.
	 */

	/* Fill in header information and write local header to zip file.
	 * This header will later be re-written since compressed length and
	 * crc are not yet known.
	 */

	/* (Assume ext, cext, com, and zname already filled in.) */
	z->vem = z->dosflag ? (pz->dosify ? 20 :  /* Made under MSDOS by PKZIP 2.0 */
                        (0 + Z_MAJORVER * 10 + Z_MINORVER))
         : OS_CODE + Z_MAJORVER * 10 + Z_MINORVER;
	/* For a FAT file system, we cheat and pretend that the file
	 * was not made on OS2 but under DOS. unzip is confused otherwise.
	 */

	z->ver = m == STORE ? 10 : 20;     /* Need PKUNZIP 2.0 except for store */
	z->crc = 0;  /* to be updated later */

	/* Assume first that we will need an extended local header: */
	z->flg = 8;  /* to be updated later */
	if (pz->key != NULL)
	{
		z->flg |= 1;
		/* Since we do not yet know the crc here, we pretend that the crc
		 * is the modification time:
		 */
		z->crc = z->tim << 16;
	}
	z->lflg = z->flg;
	z->how = m;									// may be changed later
	z->siz = (m == STORE && q >= 0) ? q : 0;	// will be changed later
	z->len = (q >= 0) ? q : 0;					// may be changed later
	z->dsk = 0;
	if (z->att == (ush)UNKNOWN)
	{
		z->att = BINARY;						// set sensible value in header
		set_type = 1;
	}

	// Attributes from filetime(), flag bits from set_extra_field():
	z->atx = z->dosflag ? a & 0xff : a | (z->atx & 0x0000ff00);
	z->off = pz->tempzn;
	if ((r = putlocal(z, y)) != ZE_OK)
		return r;
	pz->tempzn += 4 + LOCHEAD + z->nam + z->ext;

	if (pz->key != NULL)
	{
		crypthead(pz->key, z->crc, y);
		z->siz += RAND_HEAD_LEN;  /* to be updated later */
		pz->tempzn += RAND_HEAD_LEN;
	}
	if (ferror(y))
		ZIPERR(pz, ZE_WRITE, "unexpected error on zip file");

	off = ftell(y);			// for debugging only, ftell can fail on pipes
	if (ferror(y))
		clearerr(y);

	// Write stored or deflated file to zip file
	isize = 0L;
	crc = CRCVAL_INITIAL;

	if (m == DEFLATE)
	{
		bi_init(y);
		if (set_type)
			z->att = (ush)UNKNOWN; /* will be changed in deflate() */
		ct_init(pz, &z->att, &m);
		lm_init(pz, pz->level, &z->flg);
		s = deflate(pz);
	}
#if 0
	else if (m == DCLIMPLODE)
	{
		// ‚±‚±‚ÉDCL IMPLODE ƒƒ\ƒbƒh‚ð’Ç‰Á
		CMPPRM cp;
		UINT type  = CMP_BINARY;
		UINT dsize = 4096;
		char* pb = (char*)malloc(CMP_BUFFER_SIZE);
		if (!pb)
			return ZE_MEM;

		cp.crc_in = crc;
		cp.crc_out = crc;
		cp.pz = pz;
		cp.totalread = 0;
		cp.totalwrite = 0;
		cp.fpOut = y;
		implode(impdcl_read, impdcl_write, pb, &cp, &type, &dsize);
		free(pb);

		isize = cp.totalread;
		s = cp.totalwrite;
		crc = cp.crc_out;
	}
#endif
	else if (!bDir)
	{
		if ((pbuf = (char*)malloc(CBSZ)) == NULL)
			return ZE_MEM;

		while ((k = file_read(pz, pbuf, CBSZ)) > 0 && k != (extent) EOF)
		{
			if (zfwrite(pz, pbuf, 1, k, y) != k)
			{
				free(pbuf);
				return ZE_TEMP;
			}
			if (pz->verbose)
				fprintf(stdout,"%c",'.');
		}
		free(pbuf);
		s = isize;
	}
	if (ifile != fbad && zerr(ifile))
	{
		perror("\nzip warning");
		zipwarn(pz, "could not read input file: ", z->zname);
	}
	if (ifile != fbad)
		zclose(ifile);

	pz->tempzn += s;
	poff = pz->tempzn; /* save for future fseek() */

	/* Try to rewrite the local header with correct information */
	z->crc = crc;
	z->siz = s;
	if (pz->key != NULL)
		z->siz += RAND_HEAD_LEN;
	z->len = isize;

	if (!fseekable(y) || fseek(y, z->off, SEEK_SET))
	{
		if (z->how != (ush) m)
			ziperr(pz, ZE_LOGIC, "can't rewrite method");
		if (m == STORE && q < 0)
			ZIPERR(pz, ZE_PARMS, "zip -0 not supported for I/O on pipes or devices");
		if ((r = putextended(z, y)) != ZE_OK)
			return r;
		pz->tempzn += 16L;
		z->flg = z->lflg; /* if flg modified by inflate */
	}
	else
	{
		/* seek ok, ftell() should work, check compressed size */
		if (poff - off != s)
		{
			fprintf(pz->mesg, " s=%ld, actual=%ld ", s, poff - off);
			ziperr(pz, ZE_LOGIC, "incorrect compressed size");
		}
		z->how = m;
		z->ver = m == STORE ? 10 : 20;     /* Need PKUNZIP 2.0 except for store */
		if ((z->flg & 1) == 0)
			z->flg &= ~8; /* clear the extended local header flag */
		z->lflg = z->flg;

		/* rewrite the local header: */
		if ((r = putlocal(z, y)) != ZE_OK)
			return r;
		if (fseek(y, poff, SEEK_SET))
			return ZE_READ;
		if ((z->flg & 1) != 0)
		{
			/* encrypted file, extended header still required */
			if ((r = putextended(z, y)) != ZE_OK)
				return r;
			pz->tempzn += 16L;
		}
	}

	/* Free the local extra field which is no longer needed */
	if (z->ext)
	{
		if (z->extra != z->cextra)
		{
			free(z->extra);
			z->extra = NULL;
		}
		z->ext = 0;
	}

	/* Display statistics */
#ifndef WINDLL
	if (noisy)
	{
		if (verbose)
			fprintf(mesg, "\t(in=%lu) (out=%lu)", isize, s);
		if (m == DEFLATE)
			fprintf(mesg, " (deflated %d%%)\n", percent(isize, s));
		else
			fprintf(mesg, " (stored 0%%)\n");
		fflush(mesg);
	}
#else
	if (!bDir)
	{
		if ((*pz->pzcl->pfnReport)(pz->pzcl, ZP_ST_FINISH_MEMBER, 0, pz->zipfile, z->name, s) == ZP_ST_BREAK)
			return ZE_ABORT;
	}
#endif

	return ZE_OK;
}

////////////////////////////////////////////////////////////////////////
// Read a new buffer from the current input file, perform end-of-line
// translation, and update the crc and input file size.
// IN assertion: size >= 2 (for end-of-line translation)

int file_read(ZIPINFO* pz, char* buf, unsigned size)
{
	unsigned len;

	len = zread(ifile, buf, size);
	if (len == (unsigned)EOF || len == 0)
		return (int)len;

	crc = uzp_crc32(crc, (uch *) buf, len);
	isize += (ulg)len;
#ifdef WINDLL
	if ((*pz->pzcl->pfnReport)(pz->pzcl, ZP_ST_IN_PROGRESS, 0, pz->zipfile, pz->curfile, len) == ZP_ST_BREAK)
		return 0;
#endif
	return (int)len;
}


#endif /* !UTIL */
