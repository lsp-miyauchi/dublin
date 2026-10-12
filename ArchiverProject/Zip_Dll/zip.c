/*

 Copyright (C) 1990-1997 Mark Adler, Richard B. Wales, Jean-loup Gailly,
 Kai Uwe Rommel, Onno van der Linden and Igor Mandrichenko.
 Permission is granted to any individual or institution to use, copy, or
 redistribute this software so long as all of the original files are included,
 that it is not sold for profit, and that this copyright notice is retained.

*/

/*
 *  zip.c by Mark Adler.
 */

#include "zip.h"
#ifdef WINDLL
#include <windows.h>
#include "windll.h"
#endif
#include "revision.h"
#include "crypt.h"
#include "ttyio.h"

#if (defined(MSDOS) && !defined(__GO32__)) || defined(__human68k)
#  include <process.h>
#endif

#include <signal.h>

/* Local option flags */
/* Pointer to crc_table, needed in crypt.c */
ZCONST ulg* crc_32_tab;

/* Local functions */

local void freeup  OF((ZIPINFO*));
local int  finish  OF((ZIPINFO*, int));
local int get_filters OF((ZIPINFO* pz, int argc, char **argv));

////////////////////////////////////////////////////////////////////////
// Free all allocations in the found list and the zfiles list

local void freeup(ZIPINFO* pz)
{
	struct flist* f;  /* steps through found list */
	struct zlist* z;  /* pointer to next entry in zfiles list */

	for (f = pz->found; f != NULL; f = fexpel(pz, f))
		;
	while (pz->zfiles != NULL)
	{
		z = pz->zfiles->nxt;
		if (pz->zfiles->zname && pz->zfiles->zname != pz->zfiles->name)
			free(pz->zfiles->zname);
		if (pz->zfiles->name)
			free(pz->zfiles->name);
		if (pz->zfiles->iname)
			free(pz->zfiles->iname);
		if (pz->zfiles->cext &&
			pz->zfiles->cextra &&
			pz->zfiles->cextra != pz->zfiles->extra)
			free(pz->zfiles->cextra);
		if (pz->zfiles->ext && pz->zfiles->extra)
			free(pz->zfiles->extra);
		if (pz->zfiles->com && pz->zfiles->comment)
			free(pz->zfiles->comment);
		free(pz->zfiles);
		pz->zfiles = z;
		pz->zcount--;
	}
}


////////////////////////////////////////////////////////////////////////
// Process -o and -m options (if specified), free up malloc'ed stuff,
// and exit with the code e.

local int finish(ZIPINFO* pz, int e)
{
	int r;                /* return value from trash() */
	ulg t;                /* latest time in zip file */
	struct zlist* z;  /* pointer into zfile list */

	// If latest, set time to zip file to latest file in zip file
	if (pz->latest && pz->zipfile)
	{
		/* find latest time in zip file */
		if (pz->zfiles == NULL)
			zipwarn(pz, "zip file is empty, can't make it as old as latest entry", "");
		else
		{
			t = 0;

			// Ignore directories in time comparisons
			for (z = pz->zfiles; z != NULL; z = z->nxt)
			{
				if (z->zname[z->nam-1] != '/')
				{
					iztimes z_utim;
					ulg z_tim;

					z_tim = ((get_ef_ut_ztime(z, &z_utim) & EB_UT_FL_MTIME) ?
										unix2dostime(&z_utim.mtime) : z->tim);
					if (t < z_tim)
						t = z_tim;
				}
			}

			// set modified time of zip file to that time
			if (t != 0)
				stamp(pz->zipfile, t);
			else
				zipwarn(pz, "zip file has only directories, "
						"can't make it as old as latest entry", "");
		}
	}

	if (pz->tempath != NULL)
	{
		free(pz->tempath);
		pz->tempath = NULL;
	}
	if (pz->zipfile != NULL)
	{
		free(pz->zipfile);
		pz->zipfile = NULL;
	}
	if (pz->zcomment != NULL)
	{
		free(pz->zcomment);
		pz->zcomment = NULL;
	}

	// If dispose, delete all files in the zfiles list that are marked
	if (pz->dispose)
	{
		if ((r = trash(pz)) != ZE_OK)
			ZIPERR(pz, r, "was deleting moved files and directories");
	}

	// Done!
	freeup(pz);
	return e;
}

////////////////////////////////////////////////////////////////////////
// Issue a message for the error, clean up files and memory, and exit.

void ziperr(ZIPINFO* pz, int c, char* h)
/* c: error code from the ZE_ class */
/* h: message about how it happened */
{
	if (h != NULL)
	{
		if (PERR(c))
			perror("zip error");
		fflush(pz->mesg);
		fprintf(stderr, "\nzip error: %s (%s)\n", errors[c-1], h);
	}
	if (pz->tempzip != NULL)
	{
		if (pz->tempzip != pz->zipfile)
		{
			if (pz->tempzf != NULL)
				fclose(pz->tempzf);
			destroy(pz->tempzip);
			free(pz->tempzip);
			pz->tempzip = NULL;
			pz->tempzf = NULL;
		}
		else
		{
			// -g option, attempt to restore the old file
			int k = 0;                        /* keep count for end header */
			ulg cb = pz->cenbeg;                  /* get start of central */
			struct zlist* z;  /* steps through zfiles linked list */

			fprintf(stderr, "attempting to restore %s to its previous state\n", pz->zipfile);
			fseek(pz->tempzf, pz->cenbeg, SEEK_SET);
			pz->tempzn = pz->cenbeg;
			for (z = pz->zfiles; z != NULL; z = z->nxt)
			{
				putcentral(z, pz->tempzf);
				pz->tempzn += 4 + CENHEAD + z->nam + z->cext + z->com;
				k++;
			}
			putend(k, pz->tempzn - cb, cb, pz->zcomlen, pz->zcomment, pz->tempzf);
			fclose(pz->tempzf);
			pz->tempzf = NULL;
		}
	}

	if (pz->key != NULL)
	{
		free(pz->key);
		pz->key = NULL;
	}

	if (pz->tempath != NULL)
	{
		free(pz->tempath);
		pz->tempath = NULL;
	}

	if (pz->zipfile != NULL)
	{
		free(pz->zipfile);
		pz->zipfile = NULL;
	}

	if (pz->zcomment != NULL)
	{
		free(pz->zcomment);
		pz->zcomment = NULL;
	}
	freeup(pz);
	return;
}


///////////////////////////////////////////////////////////////////////
// Print a warning message to stderr and return.

void zipwarn(ZIPINFO* pz, char* a, char* b)
{
	if (pz->noisy)
		fprintf(stderr, "\tzip warning: %s%s\n", a, b);
}

///////////////////////////////////////////////////////////////////////
// Print license information to stdout.

/* @@@@
static ZCONST char *text[] = {
	"Zip %s (%s). Usage:",
	"zip [-options] [-b path] [-t mmddyyyy] [-n suffixes] [zipfile list] [-xi list]",
	"  The default action is to add or replace zipfile entries from list, which",
	"  can include the special name - to compress standard input.",
	"  If zipfile and list are omitted, zip compresses stdin to stdout.",
	"  -f   freshen: only changed files  -u   update: only changed or new files",
	"  -d   delete entries in zipfile    -m   move into zipfile (delete files)",
	"  -R   recurse into directories     -j   junk (don't record) directory names",
	"  -0   store only"
	"  -1   compress faster              -9   compress better",
	"  -q   quiet operation              -v   verbose operation/print version info",
	"                                    -o   make zipfile as old as latest entry",
	"  -F   fix zipfile (-FF try harder) -D   do not add directory entries",
	"  -A   adjust self-extracting exe   -J   junk zipfile prefix (unzipsfx)",
	"                                    -X   eXclude eXtra file attributes",
	"  -!   use privileges (if granted) to obtain all aspects of WinNT security",
	"  -$   include volume label         -S   include system and hidden files",
	"  -e   encrypt",
	"  -h   show this help"
};
@@@@ */

#ifndef PROCNAME
#  define PROCNAME(n) procname(n)
#endif /* PROCNAME */

////////////////////////////////////////////////////////////////////////
// Counts number of -i or -x patterns, sets patterns and pcount

local int get_filters(ZIPINFO* pz, int argc, char** argv)
{
	int i;
	int flag = 0, archive_seen = 0;
	char *iname;

	pz->pcount = 0;
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (strrchr(argv[i], 'R') != NULL)
				flag = 'R';
			else if (flag != 'R')
				flag = 0;
		}

		if (flag && archive_seen)
		{
			if (pz->patterns != NULL)
			{
				if (argv[i][0] != '-')
				{
					iname = ex2in(pz, argv[i], 0, (int *)NULL);
					pz->patterns[pz->pcount].zname = (iname != NULL ? in2ex(pz, iname) : NULL);
					if (iname != NULL)
						free(iname);
					pz->patterns[pz->pcount].select = flag;
					pz->icount++;
					pz->pcount++;
				}
			}
			else
				pz->pcount++;
		}
		else
			archive_seen = 1;     /* first non-flag arg is archive name */
	}
	if (pz->pcount == 0 || pz->patterns != NULL)
		return ZE_OK;
	pz->patterns = (struct plist*) malloc(pz->pcount * sizeof(struct plist));
	if (pz->patterns == NULL)
	{
		ZIPERR(pz, ZE_MEM, "was creating pattern list");
	}
	return get_filters(pz, argc, argv);
}

#ifndef WINDLL
int encr_passwd(int modeflag, char* pwbuf, int size, ZCONST char* zfn)
{
	char *prompt;
    prompt = (modeflag == ZP_PW_VERIFY) ? "Verify password: " : "Enter password: ";

    if (getp(prompt, pwbuf, size) == NULL)
	{
		ziperr(ZE_PARMS, "stderr is not a tty");
    }
    return IZ_PW_ENTERED;
}
#endif /* !WINDLL */

////////////////////////////////////////////////////////////////////////
// Add, update, freshen, or delete zip entries in a zip file.  See the
// command help in help() above.

int zipmain(ZIPINFO* pz, int argc, char** argv)
{
	int attr;				// attributes of zip file
	ulg cstart;				// start of central directory
	BOOL bAdd;				// true if just adding to a zip file
	BOOL bOpen;				// true if there were any ZE_OPEN errors
	char *e;				// malloc'd comment buffer
	struct flist *fdl;		// steps through found linked list
	int i;					// arg counter, root directory flag
	int k;					// next argument type, marked counter,
							// comment size, entry count
	ulg n;					// total of entry len's
	char *p;				// steps through option arguments
	int r;					// temporary variable
	ulg t;					// file time, length of central directory
	int first_listarg = 0;	// index of first arg of "process these files" list
	struct zlist* v;		// temporary variable
	struct zlist** w;		// pointer to last link in zfiles list
	FILE *fpx, *fpy;		// input and output zip files
	struct zlist* z;		// steps through zfiles linked list
	char *zipbuf;			// stdio buffer for the zip file


	/* Re-initialize global variables to make the zip dll re-entrant. It is
	 * possible that we could get away with not re-initializing all of these
	 * but better safe than sorry.
	 */
	pz->action = ZP_OP_ADD; /* one of ADD, UPDATE, FRESHEN, or DELETE */
	pz->latest = 0;   /* 1=set zip file time to time of latest file */
	pz->tempdir = 0;  /* 1=use temp directory (-b) */
	pz->junk_sfx = 0; /* 1=junk the sfx prefix */
	pz->zipstate = -1;
	pz->tempzip = NULL;
	pz->fcount = 0;
	pz->recurse = 0;         /* 1=recurse into directories; 2=match filenames */
	pz->dispose = 0;         /* 1=remove files after put in zip file */
	pz->pathput = 1;         /* 1=store path with name */
	pz->method = BEST;       /* one of BEST, DEFLATE (only), or STORE (only) */
	pz->dosify = 0;          /* 1=make new entries look like MSDOS */
	pz->verbose = 0;         /* 1=report oddities in zip file structure */
	pz->fix = 0;             /* 1=fix the zip file */
	pz->adjust = 0;          /* 1=adjust offsets for sfx'd file (keep preamble) */
	pz->level = 6;           /* 0=fastest compression, 9=best compression */
	pz->use_privileges = 0;  /* 1=use security privileges overrides */
	pz->hidden_files = 0;    /* process hidden and system files */
	pz->volume_label = 0;    /* add volume label */
	pz->dirnames = 1;        /* include directory entries by default */
	pz->noisy = 1;           /* 0=quiet operation */
	pz->extra_fields = 1;    /* 0=do not create extra fields */
	pz->key = NULL;          /* Scramble password if scrambling */
	pz->tempath = NULL;      /* Path for temporary files */
	pz->found = NULL;        /* where in found, or new found entry */
	pz->fnxt = &pz->found;
	pz->patterns = NULL;     /* List of patterns to be matched */
	pz->pcount = 0;          /* number of patterns */
	pz->icount = 0;          /* number of include only patterns */
	pz->mesg = (FILE*)stdout; /* cannot be made at link time for VMS */

	init_upper();           /* build case map table */

	/* extract extended argument list from environment */
	expand_args(&argc, &argv);

	pz->zipfile = pz->tempzip = NULL;
	pz->tempzf = NULL;
	bAdd = FALSE;                 /* disallow adding to a zip file */
	k = 0;                        /* Next non-option argument type */

	r = get_filters(pz, argc, argv);		// scan first the -x and -i patterns
	if (r != ZE_OK)
		return r;

	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			if (argv[i][1])
			{
				for (p = argv[i] + 1; *p; p++)
				{
					switch (*p)
					{
						case '0':
							pz->method = STORE; pz->level = 0;
							break;

						case '1':  case '2':  case '3':  case '4':
						case '5':  case '6':  case '7':  case '8':  case '9':
										// Set the compression efficacy
							pz->level = *p - '0';
							break;

						case 'A':		// Adjust unzipsfx'd zipfile:  adjust offsets only
							pz->adjust = 1;
							break;

						case 'b':		// Specify path for temporary file
							pz->tempdir = 1;
							if (k != 0)
							{
								ZIPERR(pz, ZE_PARMS, "use -b before zip file name");
							}
							else
								k = 1;			// Next non-option is path
							break;

						case 'd':		// Delete files from zip file
							if (pz->action != ZP_OP_ADD)
							{
								ZIPERR(pz, ZE_PARMS, "specify just one action");
							}
							pz->action = ZP_OP_DELETE;
							break;

						case 'D':		// Do not add directory entries
							pz->dirnames = 0;
							break;

						case 'e':		// Encrypt
							if (pz->key == NULL)
							{
								if ((pz->key = (char*)malloc(PWLEN+1)) == NULL)
								{
									ZIPERR(pz, ZE_MEM, "was allocating password space");
								}
								r = encr_passwd(ZP_PW_ENTER, pz->key, PWLEN+1, pz->zipfile);
								if (r != IZ_PW_ENTERED)
								{
									if (r < IZ_PW_ENTERED)
										r = ZE_PARMS;
									ZIPERR(pz, r, "was getting encryption password");
								}
								if (*pz->key == '\0')
								{
									ZIPERR(pz, ZE_PARMS, "zero length password not allowed");
								}
								if ((e = (char*)malloc(PWLEN+1)) == NULL)
								{
									ZIPERR(pz, ZE_MEM, "was verifying encryption password");
								}
								r = encr_passwd(ZP_PW_VERIFY, e, PWLEN+1, pz->zipfile);
								if (r != IZ_PW_ENTERED && r != IZ_PW_SKIPVERIFY)
								{
									free(e);
									if (r < ZE_OK)
										r = ZE_PARMS;
									ZIPERR(pz, r, "was verifying encryption password");
								}
								r = ((r == IZ_PW_SKIPVERIFY) ? 0 : strcmp(pz->key, e));
								free(e);
								if (r)
								{
									ZIPERR(pz, ZE_PARMS, "password verification failed");
								}
							}
							break;

						case 'F':		// fix the zip file
							pz->fix++;
							break;

						case 'f':		// Freshen zip file--overwrite only
							if (pz->action != ZP_OP_ADD)
							{
								ZIPERR(pz, ZE_PARMS, "specify just one action");
							}
							pz->action = ZP_OP_FRESHEN;
							break;

						case 'g':		// Allow appending to a zip file
							bAdd = TRUE;
							break;

						case 'j':		// Junk directory names
							pz->pathput = 0;
							break;

						case 'J':		// Junk sfx prefix
							pz->junk_sfx = 1;
							break;

						case 'k':		// Make entries using DOS names (k for Katz)
							pz->dosify = 1;
							break;

						case 'm':		// Delete files added or updated in zip file
							pz->dispose = 1;
							break;

						case 'M':		// DCLIMPLODE
							pz->method = DCLIMPLODE;
							break;

						case 'o':		// Set zip file time to time of latest file in it
							pz->latest = 1;
							break;

						case 'p':		// Store path with name
							// (do nothing as annoyance avoidance)
							break;

						case 'P':		// password for encryption
							if (k != 0)
							{
								ZIPERR(pz, ZE_PARMS, "use -P before zip file name");
							}
							if (pz->key != NULL)
							{
								ZIPERR(pz, ZE_PARMS, "can only have one -P");
							}
							k = 7;
							break;

						case 'q':		// Quiet operation
							pz->noisy = 0;
							if (pz->verbose)
								pz->verbose--;
							break;

						case 'r':		// Recurse into subdirectories, match full path
							if (pz->recurse == 2)
							{
								ZIPERR(pz, ZE_PARMS, "do not specify both -r and -R");
							}
							pz->recurse = 1;
							break;

						case 'R':		// Recurse into subdirectories, match filename
							if (pz->recurse == 1)
							{
								ZIPERR(pz, ZE_PARMS, "do not specify both -r and -R");
							}
							pz->recurse = 2;
							break;

						case 'S':
							pz->hidden_files = 1;
							break;

						case 'u':		// Update zip file--overwrite only if newer
							if (pz->action != ZP_OP_ADD)
							{
								ZIPERR(pz, ZE_PARMS, "specify just one action");
							}
							pz->action = ZP_OP_UPDATE;
							break;

						case 'v':		// Mention oddities in zip file structure
							pz->noisy = 1;
							pz->verbose++;
							break;

						case '$':		// Include volume label
							pz->volume_label = 1;
							break;

						case 'X':
							pz->extra_fields = 0;
							break;

						case '!':		// use security privilege overrides
							pz->use_privileges = 1;
							break;

						default:
						{
							sprintf(errbuf, "no such option: %c", *p);
							ZIPERR(pz, ZE_PARMS, errbuf);
						}
					}
				}
			}
			else              /* just a dash */
			{
				switch (k)
				{
					case 1:
						ZIPERR(pz, ZE_PARMS, "invalid path");
						break;

					case 2:
						ZIPERR(pz, ZE_PARMS, "invalid time");
						break;

					case 3:  case 4:
						if ((r = PROCNAME(pz, argv[i])) != ZE_OK)
							if (r == ZE_MISS)
								zipwarn(pz, "name not matched: ", argv[i]);
							else
							{
								ZIPERR(pz, r, argv[i]);
							}
						if (k == 3)
						{
							first_listarg = i;
							k = 4;
						}
				}
			}
		}
		else                /* not an option */
		{
			if (k == 5)
				break;		// -i and -x arguments already scanned
			else if (k == 6)
			{
				if ((r = PROCNAME(pz, ".")) != ZE_OK)
					if (r == ZE_MISS)
						zipwarn(pz, "name not matched: ", argv[i]);
					else
					{
						ZIPERR(pz, r, argv[i]);
					}
				break;
			}
			else switch (k)
			{
				case 0:
					if ((pz->zipfile = ziptyp(pz, argv[i])) == NULL)
					{
						ZIPERR(pz, ZE_MEM, "was processing arguments");
					}
					if ((r = readzipfile(pz)) != ZE_OK)
					{
						ZIPERR(pz, r, pz->zipfile);
					}
					k = 3;
					if (pz->recurse == 2)
						k = 6;
					break;

				case 1:
					if ((pz->tempath = (char*)malloc(lstrlen(argv[i]) + 1)) == NULL)
					{
						ZIPERR(pz, ZE_MEM, "was processing arguments");
					}
					lstrcpy(pz->tempath, argv[i]);
					k = 0;
					break;

				case 3:  case 4:
					if ((r = PROCNAME(pz, argv[i])) != ZE_OK)
						if (r == ZE_MISS)
							zipwarn(pz, "name not matched: ", argv[i]);
						else
						{
							ZIPERR(pz, r, argv[i]);
						}
					if (k == 3)
					{
						first_listarg = i;
						k = 4;
					}
					break;

				case 7:
					if ((pz->key = (char*)malloc(lstrlen(argv[i]) + 1)) == NULL)
					{
						ZIPERR(pz, ZE_MEM, "was processing arguments");
					}
					lstrcpy(pz->key, argv[i]);
					k = 0;

			}
		}
	}
	if (k == 7 || k == 1)
	{
		ZIPERR(pz, ZE_PARMS, "missing argument for -b or -P");
	}

	// Clean up selections ("3 <= k <= 5" now)
	if (k != 4 && first_listarg == 0 &&
		(pz->action == ZP_OP_UPDATE || pz->action == ZP_OP_FRESHEN))
	{
		// if -u or -f with no args, do all, but, when present, apply filters
		for (z = pz->zfiles; z != NULL; z = z->nxt)
		{
			z->mark = pz->pcount ? filter(pz, z->zname) : 1;
		}
	}

	// remove duplicates in found list
	if ((r = check_dup(pz)) != ZE_OK)
	{
		if (r == ZE_PARMS)
		{
			ZIPERR(pz, r, "cannot repeat names in zip file");
		}
		else
		{
			ZIPERR(pz, r, "was processing list of files");
		}
	}

	if (pz->zcount)
		free(pz->zsort);

	if (pz->action == ZP_OP_DELETE && (pz->method != BEST || pz->dispose || pz->recurse || pz->key != NULL))
	{
		zipwarn(pz, "invalid option(s) used with -d; ignored.","");

		// reset flags - needed?
		pz->method  = BEST;
		pz->dispose = 0;
		pz->recurse = 0;
		if (pz->key != NULL)
			free(pz->key);
		pz->key = NULL;
	}

	if (pz->fix && pz->adjust)
	{
		zipwarn(pz, "can't use -F with -A, -F ignored", "");
	}
	if (pz->zcount == 0 && (pz->action != ZP_OP_ADD || bAdd))
	{
		zipwarn(pz, pz->zipfile, " not found or empty");
		return ZE_NAME;
	}

	/*
	 * XXX make some kind of mktemppath() function for each OS.
	 */

	/* For CMS, leave tempath NULL.  A-disk will be used as default. */
	/* If -b not specified, make temporary path the same as the zip file */
	if (pz->tempath == NULL &&
		((p = strrchr(pz->zipfile, '/')) != NULL ||
        (p = strrchr(pz->zipfile, '\\')) != NULL ||
        (p = strrchr(pz->zipfile, ':')) != NULL))
	{
		if (*p == ':')
			p++;
		if ((pz->tempath = (char*)malloc((int)(p - pz->zipfile) + 1)) == NULL)
		{
			ZIPERR(pz, ZE_MEM, "was processing arguments");
		}
		r = *p;  *p = 0;
		lstrcpy(pz->tempath, pz->zipfile);
		*p = (char)r;
	}

	// For each marked entry, if not deleting, check if it exists, and if
	// updating or freshening, compare date with entry in old zip file.
	// Unmark if it doesn't exist or is too old, else update marked count.
	k = 0;					// Initialize marked count
	for (z = pz->zfiles; z != NULL; z = z->nxt)
	{
		if (z->mark)
		{
			iztimes f_utim, z_utim;

			sprintf(errbuf, "zip diagnostic: marked file=%s\n", z->zname);

			if (pz->action != ZP_OP_DELETE &&
				((t = filetime(z->name, NULL, NULL, &f_utim)) == 0 ||
				((pz->action == ZP_OP_UPDATE || pz->action == ZP_OP_FRESHEN) &&
				((get_ef_ut_ztime(z, &z_utim) & EB_UT_FL_MTIME) ?
					f_utim.mtime <= ROUNDED_TIME(z_utim.mtime) : t <= z->tim))))
			{
				z->mark = 0;
				// delete if -um or -fm
				z->trash = (t != 0);
				if (pz->verbose)
				{
					fprintf(pz->mesg, "zip diagnostic: %s %s\n", z->zname,
							z->trash ? "up to date" : "missing or early");
				}
			}
			else
				k++;
		}
	}

	// Remove entries from found list that do not exist or are too old
	sprintf(errbuf, "zip diagnostic: fcount=%d\n", (int)pz->fcount);
	for (fdl = pz->found; fdl != NULL;)
	{
		sprintf(errbuf, "zip diagnostic: new file=%s\n", fdl->zname);
		if (pz->action == ZP_OP_DELETE || pz->action == ZP_OP_FRESHEN ||
			(t = filetime(fdl->name, NULL, NULL, NULL)) == 0 ||
			namecmp(fdl->zname, pz->zipfile) == 0)
			fdl = fexpel(pz, fdl);
		else
			fdl = fdl->nxt;
	}

	// Make sure there's something left to do
	if (k == 0 && pz->found == NULL &&
		!(pz->zfiles != NULL &&
		(pz->latest || pz->fix || pz->adjust || pz->junk_sfx)))
	{
		if (pz->action == ZP_OP_UPDATE || pz->action == ZP_OP_FRESHEN)
		{
			RETURN(finish(pz, ZE_OK));
		}
		else if (pz->zfiles == NULL &&
				(pz->latest || pz->fix || pz->adjust || pz->junk_sfx))
		{
			ZIPERR(pz, ZE_NAME, pz->zipfile);
		}
		else
		{
			ZIPERR(pz, ZE_NONE, pz->zipfile);
		}
	}
	// bAdd true if appending
	bAdd = (bAdd && k == 0 && (pz->zipbeg || pz->zfiles != NULL));

	// Initialize the crc_32_tab pointer, when encryption was requested.
	if (pz->key != NULL)
		crc_32_tab = (ulg*)get_crc_table();

	// Before we get carried away, make sure zip file is writeable. This
	// has the undesired side effect of leaving one empty junk file on a WORM,
	// so when the zipfile does not exist already and when -b is specified,
	// the writability check is made in replace().
	if (pz->tempdir && pz->zfiles == NULL && pz->zipbeg == 0)
	{
		attr = 0;
	}
	else
	{
		fpx = (pz->zfiles == NULL && pz->zipbeg == 0) ?
					fopen(pz->zipfile, FOPW) : fopen(pz->zipfile, FOPM);
		// Note: FOPW and FOPM expand to several parameters for VMS
		if (fpx == NULL)
		{
			ZIPERR(pz, ZE_CREAT, pz->zipfile);
		}
		fclose(fpx);
		attr = getfileattr(pz->zipfile);
		if (pz->zfiles == NULL && pz->zipbeg == 0)
			destroy(pz->zipfile);
	}

	// Throw away the garbage in front of the zip file for -J
	if (pz->junk_sfx)
		pz->zipbeg = 0;

	// Open zip file and temporary output file
	fpx = NULL;
	pz->tempzn = 0;
	if (bAdd)			// bAdd true if just appending (-g)
	{
		if ((fpy = fopen(pz->zipfile, FOPM)) == NULL)
		{
			ZIPERR(pz, ZE_NAME, pz->zipfile);
		}
		pz->tempzip = pz->zipfile;
		pz->tempzf = fpy;
		if (fseek(fpy, pz->cenbeg, SEEK_SET))
		{
			ZIPERR(pz, ferror(fpy) ? ZE_READ : ZE_EOF, pz->zipfile);
		}
		pz->tempzn = pz->cenbeg;
	}
	else
	{
		if ((pz->zfiles != NULL || pz->zipbeg) &&
			(fpx = fopen(pz->zipfile, FOPR_EX)) == NULL)
		{
			ZIPERR(pz, ZE_NAME, pz->zipfile);
		}
		if ((pz->tempzip = tempname(pz, pz->zipfile)) == NULL)
		{
			ZIPERR(pz, ZE_MEM, "allocating temp filename");
		}
		if ((pz->tempzf = fpy = fopen(pz->tempzip, FOPW)) == NULL)
		{
			ZIPERR(pz, ZE_TEMP, pz->tempzip);
		}
	}

	// Use large buffer to speed up stdio:
	zipbuf = (char *)malloc(ZBSZ);
	if (zipbuf == NULL)
	{
		ZIPERR(pz, ZE_MEM, pz->tempzip);
	}
	setvbuf(fpy, zipbuf, _IOFBF, ZBSZ);

	// this must go *after* set[v]buf
	if (!bAdd)
	{
		if (pz->zipbeg && (r = fcopy(fpx, fpy, pz->zipbeg)) != ZE_OK)
		{
			ZIPERR(pz, r, r == ZE_TEMP ? pz->tempzip : pz->zipfile);
		}
		pz->tempzn = pz->zipbeg;
	}

	bOpen = FALSE;                                /* no ZE_OPEN errors yet */

	// Process zip file, updating marked files
	w = &pz->zfiles;
	while ((z = *w) != NULL)
	{
		if (z->mark == 1)
		{
			// if not deleting, zip it up
			if (pz->action != ZP_OP_DELETE)
			{
#ifndef WINDLL
				if (pz->noisy)
				{
					if (pz->action == ZP_OP_FRESHEN)
						fprintf(mesg, "freshening: %s", z->zname);
					else
						fprintf(mesg, "updating: %s", z->zname);
					fflush(mesg);
				}
#endif
				r = zipup(pz, z, fpy);
				if (r != ZE_OK && r != ZE_OPEN && r != ZE_MISS)
				{
					if (pz->noisy)
					{
						fprintf(stdout,"\n");
					}
					sprintf(errbuf, "was zipping %s", z->name);
					ZIPERR(pz, r, errbuf);
				}
				if (r == ZE_OPEN || r == ZE_MISS)
				{
					bOpen = TRUE;
					if (pz->noisy)
					{
						fprintf(stdout,"\n");
					}

					if (r == ZE_OPEN)
					{
						perror(z->zname);
						zipwarn(pz, "could not open for reading: ", z->zname);
					}
					else
					{
						zipwarn(pz, "file and directory with the same name: ", z->zname);
					}
					zipwarn(pz, "will just copy entry over: ", z->zname);
					if ((r = zipcopy(pz, z, fpx, fpy)) != ZE_OK)
					{
						sprintf(errbuf, "was copying %s", z->zname);
						ZIPERR(pz, r, errbuf);
					}
					z->mark = 0;
				}
				w = &z->nxt;
			}
			else
			{
				if (pz->noisy)
				{
					fprintf(pz->mesg, "deleting: %s\n", z->zname);
					fflush(pz->mesg);
				}
				v = z->nxt;                     /* delete entry from list */
				free(z->iname);
				free(z->zname);
				if (z->ext)
					free(z->extra);
				if (z->cext && z->cextra != z->extra)
					free(z->cextra);
				if (z->com)
					free(z->comment);
				free(z);
				*w = v;
				pz->zcount--;
			}
		}
		else
		{
			// copy the original entry verbatim
			if (!bAdd && (r = zipcopy(pz, z, fpx, fpy)) != ZE_OK)
			{
				sprintf(errbuf, "was copying %s", z->zname);
				ZIPERR(pz, r, errbuf);
			}
			w = &z->nxt;
		}
	}

	// Process the edited found list, adding them to the zip file
	for (fdl = pz->found; fdl != NULL; fdl = fexpel(pz, fdl))
	{
		// add a new zfiles entry and set the name
		if ((z = (struct zlist*)malloc(sizeof(struct zlist))) == NULL)
		{
			ZIPERR(pz, ZE_MEM, "was adding files to zip file");
		}
		z->nxt = NULL;
		z->name = fdl->name;
		fdl->name = NULL;
		z->iname = fdl->iname;
		fdl->iname = NULL;
		z->zname = fdl->zname;
		fdl->zname = NULL;
		z->ext = z->cext = z->com = 0;
		z->extra = z->cextra = NULL;
		z->mark = 1;
		z->dosflag = fdl->dosflag;

		// zip it up
#ifndef WINDLL
		if (pz->noisy)
		{
			fprintf(mesg, "  adding: %s", z->zname);
			fflush(mesg);
		}
#endif
		r = zipup(pz, z, fpy);
		if (r != ZE_OK  && r != ZE_OPEN && r != ZE_MISS)
		{
			if (pz->noisy)
			{
				fprintf(stdout,"\n");
			}
			sprintf(errbuf, "was zipping %s", z->zname);
			ZIPERR(pz, r, errbuf);
		}
		if (r == ZE_OPEN || r == ZE_MISS)
		{
			bOpen = TRUE;
			if (pz->noisy)
			{
				fprintf(stdout,"\n");
			}
			if (r == ZE_OPEN)
			{
				perror("zip warning");
				zipwarn(pz, "could not open for reading: ", z->zname);
			}
			else
			{
				zipwarn(pz, "file and directory with the same name: ", z->zname);
			}
			free(z->name);
			free(z->iname);
			free(z->zname);
			free(z);
		}
		else
		{
			*w = z;
			w = &z->nxt;
			pz->zcount++;
		}
	}
	if (pz->key != NULL)
	{
		free(pz->key);
		pz->key = NULL;
	}

	// １行コメント機能サプレス
	// 複数行コメント機能サプレス

	// Write central directory and end header to temporary zip
	k = 0;                        /* keep count for end header */
	cstart = pz->tempzn;             /* get start of central */
	n = t = 0;
	for (z = pz->zfiles; z != NULL; z = z->nxt)
	{
		if ((r = putcentral(z, fpy)) != ZE_OK)
		{
			ZIPERR(pz, r, pz->tempzip);
		}
		pz->tempzn += 4 + CENHEAD + z->nam + z->cext + z->com;
		n += z->len;
		t += z->siz;
		k++;
	}
	if (k == 0)
		zipwarn(pz, "zip file empty", "");
	if (pz->verbose)
		fprintf(pz->mesg, "total bytes=%lu, compressed=%lu -> %d%% savings\n",
				n, t, percent(n, t));
	t = pz->tempzn - cstart;               /* compute length of central */
	if ((r = putend(k, t, cstart, pz->zcomlen, pz->zcomment, fpy)) != ZE_OK)
	{
		ZIPERR(pz, r, pz->tempzip);
	}
	pz->tempzf = NULL;
	if (fclose(fpy))
	{
		ZIPERR(pz, bAdd ? ZE_WRITE : ZE_TEMP, pz->tempzip);
	}
	if (fpx != NULL)
		fclose(fpx);

	// Free some memory before spawning unzip
	lm_free();

	// Replace old zip file with new zip file, leaving only the new one
	if (!bAdd)
	{
		if ((r = replace(pz->zipfile, pz->tempzip)) != ZE_OK)
		{
			zipwarn(pz, "new zip file left as: ", pz->tempzip);
			free(pz->tempzip);
			pz->tempzip = NULL;
			ZIPERR(pz, r, "was replacing the original zip file");
		}
		free(pz->tempzip);
	}
	pz->tempzip = NULL;
	if (attr)
	{
		setfileattr(pz->zipfile, attr);
	}

	// Finish up (process -o, -m, clean up).  Exit code depends on o.
	free(zipbuf);
	RETURN(finish(pz, (bOpen ? ZE_OPEN : ZE_OK)));
}
