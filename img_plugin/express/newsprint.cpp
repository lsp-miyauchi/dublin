///////////////////////////////////////////////////////////////////////
// newsprint.cpp

#include "stdafx.h"

#define MIN(a,b)				((a)<(b)?(a):(b))
#define MAX(a,b)				((a)>(b)?(a):(b))
#define M_PI					(3.14159265358979323846)
#define M_PI_2					(1.57079632679489661923)
#define M_PI_4					(0.78539816339744830962)
#define INTENSITY(r,g,b)		((DWORD)((DWORD)(r)*77+(DWORD)(g)*150+(DWORD)(b)*29)>>8)
#define	DEG2RAD(x)				((x) * M_PI / 180.0)

void progress_init(int maxprogress);
void progress_continue();
void progress_term();


/*
 * 0.60: 18 Jun 2001  <austin@gimp.org>
 *    fixed long-standing bug where newsprint() function in GREYA images
 *    treated them as RGB (bpp rather than colour_bpp) to select
 *    colorspace to use.  Thanks to warner-gnome.bugzilla@lothar.com for
 *    spotting this and providing the patch.  Bug #52981.
 */

class memitem
{
public:
	memitem(size_t size) { next = NULL; data = malloc(size); }
	~memitem() { if (data) free(data); }

	memitem* next;
	void* data;
};

class memman
{
public:
	memman() { list = NULL; }
	~memman();
	void* alloc(size_t size);

	memitem* list;
};

memman::~memman()
{
	memitem* pi = list;
	while (pi)
	{
		memitem* pw = pi->next;
		delete pi;
		pi = pw;
	}
}

void* memman::alloc(size_t size)
{
	memitem* pi = new memitem(size);
	if (!list)
		list = pi;
	else
	{
		pi->next = list;
		list = pi;
	}
	return pi->data;
}


#define DEF_OVERSAMPLE		4		// default for how much to oversample by
#define ISNEG(x)			(((x) < 0) ? 1 : 0)
#define CLAMPED_ADD(a, b)	(((a)+(b) > 0xff)? 0xff : (a) + (b))
#define ABS(a)				(((a) < 0) ? ((a) * -1) : (a))
#define CLAMP(v,vmin,vmax)	(MAX((vmin), MIN((v),(vmax))))
#define RINT(x)				(floor((x) + 0.5))

/* Bartlett window supersampling weight function.  See table 4.1, page
 * 123 of Alan Watt and Mark Watt, Advanced Animation and Rendering
 * Techniques, theory and practice. Addison-Wesley, 1992. ISBN
 * 0-201-54412-1 */
#define BARTLETT(x,y,o)		(((o/2)+1-ABS(x)) * ((o/2)+1-ABS(y)))

/* colorspaces we can separate to: */
#define CS_GREY				0
#define CS_RGB				1
#define CS_CMYK				2
#define CS_INTENSITY		3
#define NUM_CS				4
#define VALID_CS(x)			((x) >= 0 && (x) <= NUM_CS-1)

/* Spot function related types and definitions */

typedef double spotfn_t (double x, double y);

/* forward declaration of the functions themselves */
static spotfn_t spot_round;
static spotfn_t spot_line;
static spotfn_t spot_diamond;
static spotfn_t spot_PSsquare;
static spotfn_t spot_PSdiamond;

typedef struct {
	spotfn_t *fn;			/* function itself */
	BYTE* thresh;			/* cached threshold matrix */
	double prev_lvl[3];		/* intensities to preview */
	BYTE* prev_thresh;		/* preview-sized threshold matrix */
	int balanced;			/* TRUE if spot fn is already balanced */
} spot_info_t;


/* This is all the info needed per spot function.  Functions are refered to
 * by their index into this array. */
static spot_info_t spotfn_list[] =
{
#define SPOTFN_DOT 0
	{	// Round - 丸
		spot_round, NULL, { .22, .50, .90 }, NULL, FALSE },

	{	// Line - 線形
		spot_line, NULL, { .15, .50, .80 }, NULL, FALSE },

	{	// Diamond - ダイヤモンド
		spot_diamond, NULL, { .15, .50, .80 }, NULL, TRUE },

	{	// PS Square (Euclidean Dot) - PS四角(ユークリッド点)
		spot_PSsquare, NULL, { .15, .50, .90 }, NULL, FALSE },

	{	// PS Diamond - PSダイヤモンド
		spot_PSdiamond, NULL, { .15, .50, .90 }, NULL, FALSE },
};

#define NUM_SPOTFN			(sizeof(spotfn_list) / sizeof(spot_info_t))
#define VALID_SPOTFN(x)		((x) >= 0 && (x) < NUM_SPOTFN)
#define THRESH(x,y)			(thresh[(y)*width + (x)])
#define THRESHn(n,x,y)		((thresh[n])[(y)*width + (x)])

typedef struct tagNewsPrintParams {
	// resolution section:
	int cell_width;				// 3 - 100

	// screening section:
	int colorspace;				// 0: RGB, 1: CMYK, 2: Intensity
	int k_pullout;				// percentage of black to pull out

	// gray screen (only used if grayscale drawable)
	double gray_angle;			// -90.0 - 90.0
	int gray_spotfn;

	// red / cyan screen
	double red_angle;			// -90.0 - 90.0
	int red_spotfn;

	// green / magenta screen
	double green_angle;			// -90.0 - 90.0
	int green_spotfn;

	// blue / yellow screen
	double blue_angle;			// -90.0 - 90.0
	int blue_spotfn;

	// anti-alias section
	int oversample;				// 1 - 15, 1 == no anti-aliasing
} NewsPrintParams;

// fixed copy so we can reset them
static const NewsPrintParams factory_defaults =
{
	// resolution stuff
	8,							// cell width

	// screen setup (default is the classic rosette pattern)
	CS_RGB,						// use RGB, not CMYK or Intensity
	100,						// max pullout

	// grey/black
	45.0,
	SPOTFN_DOT,

	// red/cyan
	15.0,
	SPOTFN_DOT,

	// green/magenta
	75.0,
	SPOTFN_DOT,

	// blue/yellow
	0.0,
	SPOTFN_DOT,

	// anti-alias control
	DEF_OVERSAMPLE
};

// Mutable copy for normal use.  Initialised in run().
static NewsPrintParams pvals;

// channel templates
typedef struct {
	// pointers to the variables this channel updates
	double* angle;
	int* spotfn;

	// factory defaults for the angle and spot function (as pointers so
	// the silly compiler can see they're really constants)
	const double* factory_angle;
	const int* factory_spotfn;
} chan_tmpl;

static const chan_tmpl grey_tmpl[] = {
	{	// Gray
		&pvals.gray_angle,
		&pvals.gray_spotfn,
		&factory_defaults.gray_angle,
		&factory_defaults.gray_spotfn
	},
};

static const chan_tmpl rgb_tmpl[] = {
	{	// Red
		&pvals.red_angle,
		&pvals.red_spotfn,
		&factory_defaults.red_angle,
		&factory_defaults.red_spotfn
	},

	{	// Green
		&pvals.green_angle,
		&pvals.green_spotfn,
		&factory_defaults.green_angle,
		&factory_defaults.green_spotfn
	},

	{	// Blue
		&pvals.blue_angle,
		&pvals.blue_spotfn,
		&factory_defaults.blue_angle,
		&factory_defaults.blue_spotfn
	},
};

static const chan_tmpl cmyk_tmpl[] = {
	{	// Cyan
		&pvals.red_angle,
		&pvals.red_spotfn,
		&factory_defaults.red_angle,
		&factory_defaults.red_spotfn
	},

	{	// Magenta
		&pvals.green_angle,
		&pvals.green_spotfn,
		&factory_defaults.green_angle,
		&factory_defaults.green_spotfn
	},

	{	// Yellow
		&pvals.blue_angle,
		&pvals.blue_spotfn,
		&factory_defaults.blue_angle,
		&factory_defaults.blue_spotfn
	},

	{	// Black
		&pvals.gray_angle,
		&pvals.gray_spotfn,
		&factory_defaults.gray_angle,
		&factory_defaults.gray_spotfn
	},
};

static const chan_tmpl intensity_tmpl[] = {
	{	// Intensity
		&pvals.gray_angle,
		&pvals.gray_spotfn,
		&factory_defaults.gray_angle,
		&factory_defaults.gray_spotfn
	},
};

#define NCHANS(x) (sizeof(x) / sizeof(chan_tmpl))

/* cspace_nchans gives a quick way of finding the number of channels
 * in a colorspace.  Alternatively, if you're walking the channel
 * template, you can use the NULL entry at the end to stop. */
static const int cspace_nchans[] = {
	NCHANS(grey_tmpl),
	NCHANS(rgb_tmpl),
	NCHANS(cmyk_tmpl),
	NCHANS(intensity_tmpl)
};


static BYTE* spot2thresh(memman* pmm, int type, int width);


/*************************************************************/
/* Spot functions */


/* Spot functions define the order in which pixels should be whitened
 * as a cell lightened in colour.  They are defined over the entire
 * cell, and are called over each pixel in the cell.  The cell
 * co-ordinate space ranges from -1.0 .. +1.0 inclusive, in both x- and
 * y-axes.
 *
 * This means the spot function f(x, y) must be defined for:
 *     -1 <= x <= +1, where x is a real number,   and
 *     -1 <= y <= +1, where y is a real number.
 *
 * The function f's range is -1.0 .. +1.0 inclusive, but it is
 * permissible for f to return values outside this range: the nearest
 * valid value will be used instead.  NOTE: this is in contrast with
 * PostScript spot functions, where it is a RangeError for the spot
 * function to go outside these limits.
 *
 * An initially black cell is filled from lowest spot function value
 * to highest.  The actual values returned do not matter - it is their
 * relative orderings that count.  This means that spot functions do
 * not need to be tonally balanced.  A tonally balanced spot function
 * is one which for all slices though the function (eg say at z), the
 * area of the slice = 4z.  In particular, almost all PostScript spot
 * functions are _not_ tonally balanced.
 */


/* The classic growing dot spot function.  This one isn't tonally
 * balanced.  It can be made so, but it's _really_ ugly.  Thanks to
 * Richard Mortier for this one:
 *
 * #define a(r) \
 *     ((r<=1)? M_PI * (r*r) : \
 *	4 * sqrt(r*r - 1)  +  G_PI*r*r  -  4*r*r*acos(1/r))
 *
 *   radius = sqrt(x*x + y*y);
 *
 * return a(radius) / 4; */
static double spot_round(double x, double y)
{
	return 1 - (x * x + y * y);
}

/* Another commonly seen spot function is the v-shaped wedge. Tonally
 * balanced. */
static double spot_line(double x, double y)
{
	return ABS(y);
}

/* Square/Diamond dot that never becomes round.  Tonally balanced. */
static double spot_diamond(double x, double y)
{
	double xy = ABS (x) + ABS (y);
	// spot only valid for 0 <= xy <= 2
	return ((xy <= 1) ? 2 * xy * xy : 2 * xy * xy - 4 * (xy - 1) * (xy - 1)) / 4;
}

/* The following functions were derived from a peice of PostScript by
 * Peter Fink and published in his book, "PostScript Screening: Adobe
 * Accurate Screens" (Adobe Press, 1992).  Adobe Systems Incorporated
 * allow its use, provided the following copyright message is present:
 *
 *  % Film Test Pages for Screenset Development
 *  % Copyright (c) 1991 and 1992 Adobe Systems Incorporated
 *  % All rights reserved.
 *  %
 *  % NOTICE: This code is copyrighted by Adobe Systems Incorporated, and
 *  % may not be reproduced for sale except by permission of Adobe Systems
 *  % Incorporated. Adobe Systems Incorporated grants permission to use
 *  % this code for the development of screen sets for use with Adobe
 *  % Accurate Screens software, as long as the copyright notice remains
 *  % intact.
 *  %
 *  % By Peter Fink 1991/1992
 */

/* Square (or Euclidean) dot.  Also very common. */
static double spot_PSsquare(double x, double y)
{
	double ax = ABS (x);
	double ay = ABS (y);

	return ((ax + ay) > 1 ?
				((ay - 1) * (ay - 1) + (ax - 1) * (ax - 1)) - 1 :
				1 - (ay * ay + ax * ax));
}

/* Diamond spot function, again from Peter Fink's PostScript
 * original.  Copyright as for previous function. */
static double spot_PSdiamond(double x, double y)
{
	double ax = ABS(x);
	double ay = ABS(y);

	return ((ax + ay) <= 0.75 ? 1 - (ax * ax + ay * ay) :			// dot
				((ax + ay) <= 1.23 ?  1 - ((ay * 0.76) + ax) :		// to diamond (0.76 distort)
				((ay - 1) * (ay - 1) + (ax - 1) * (ax - 1)) -1));	// back to dot
}

/*************************************************************/
/* Spot function to threshold matrix conversion */


/* Each call of the spot function results in one of these */
typedef struct {
	int index;				// (y * width) + x
	double value;			// return value of the spot function
} order_t;

// qsort(3) compare function
static int order_cmp(const void *va, const void *vb)
{
	order_t* a = (order_t*)va;
	order_t* b = (order_t*)vb;
	return (a->value < b->value) ? -1 : ((a->value > b->value)? + 1 : 0);
}

/* Convert spot function "type" to a threshold matrix of size "width"
 * times "width".  Returns newly allocated threshold matrix.  The
 * reason for qsort()ing the results rather than just using the spot
 * function's value directly as the threshold value is that we want to
 * ensure that the threshold matrix is tonally balanced - that is, for
 * a threshold value of x%, x% of the values in the matrix are < x%.
 *
 * Actually, it turns out that qsort()ing a function which is already
 * balanced can quite significantly detract from the quality of the
 * final result.  This is particularly noticable with the line or
 * diamond spot functions at 45 degrees.  This is because if the spot
 * function has multiple locations with the same value, qsort may use
 * them in any order.  Often, there is quite clearly an optimal order
 * however.  By marking functions as pre-balanced, this random
 * shuffling is avoided.  WARNING: a non-balanced spot function marked
 * as pre-balanced is bad: you'll end up with dark areas becoming too
 * dark or too light, and vice versa for light areas.  This is most
 * easily checked by halftoning an area, then bluring it back - you
 * should get the same colour back again.  The only way of getting a
 * correctly balanced function is by getting a formula for the spot's
 * area as a function of x and y - this can be fairly tough (ie
 * possiblly an integral in two dimensions that must be solved
 * analytically).
 *
 * The threshold matrix is used to compare against image values.  If
 * the image value is greater than the threshold value, then the
 * output pixel is illuminated.  This means that a threshold matrix
 * entry of 0 never causes output pixels to be illuminated.  */
static BYTE* spot2thresh(memman* pmm, int type, int width)
{
	double   sx, sy;
	double   val;
	int wid2 = width * width;
	int balanced = spotfn_list[type].balanced;

	BYTE* thresh;
	if (pmm)
		thresh = (BYTE*)pmm->alloc(wid2 * sizeof(BYTE));
	else
		thresh = (BYTE*)malloc(wid2 * sizeof(BYTE));
	spotfn_t* spotfn = spotfn_list[type].fn;
	order_t* order = (order_t*)malloc(wid2 * sizeof(order_t));

	int i = 0;
	for (int y = 0; y < width; y++)
	{
		for (int x = 0; x < width; x++)
		{
			// scale x & y to -1 ... +1 inclusive
			sx = (((double)x) / (width-1) - 0.5) * 2;
			sy = (((double)y) / (width-1) - 0.5) * 2;
			val = spotfn(sx, sy);
			val = CLAMP(val, -1, 1);			// interval is inclusive

			order[i].index = i;
			order[i].value = val;
			i++;
		}
	}

	if (!balanced)
	{
		// now sort array of (point, value) pairs in ascending order
		qsort(order, wid2, sizeof (order_t), order_cmp);
    }

	// compile threshold matrix in order from darkest to lightest
	for (i = 0; i < wid2; i++)
	{
		// thresh[] contains values from 0 .. 254.  The reason for not
		// including 255 is so that an image value of 255 remains
		// unmolested.  It would be bad to filter a completely white
		// image and end up with black speckles.
		if (balanced)
			thresh[order[i].index] = (BYTE)(order[i].value * 0xfe);
		else
			thresh[order[i].index] = i * 0xff / wid2;
	}
	free(order);

	// TODO: this is where the code to apply a transfer or dot gain
	//       function to the threshold matrix would go.
	return thresh;
}

BOOL newsprint(mif& src, LPRECT prc, int cell_width, int oversample)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;

	memcpy(&pvals, &factory_defaults, sizeof(pvals));
	pvals.cell_width = cell_width;
	pvals.oversample = oversample;

	BYTE* thresh[4];
	double    r;
	double    theta;
	double    rot[4];
	int       b;

	int width = pvals.cell_width;
	if (width < 0) width = -width;
	if (width < 1) width = 1;

	double k_pullout = ((double)pvals.k_pullout) / 100.0;

	width *= oversample;
	int colorspace = pvals.colorspace;

	// Bartlett window matrix optimisation
	int w002 = BARTLETT(0, 0, oversample) * BARTLETT(0, 0, oversample);

#define ASRT(_x)				\
do {							\
	if (!VALID_SPOTFN(_x))		\
		_x = SPOTFN_DOT;		\
} while(0)

	// calculate the RGB / CMYK rotations and threshold matrices
	memman mm;
	if (colorspace == CS_INTENSITY) {
		rot[0]    = DEG2RAD(pvals.gray_angle);	
		thresh[0] = spot2thresh(&mm, pvals.gray_spotfn, width);
	}
	else {
		int rf = pvals.red_spotfn;
		int gf = pvals.green_spotfn;
		int bf = pvals.blue_spotfn;

		rot[0] = DEG2RAD(pvals.red_angle);
		rot[1] = DEG2RAD(pvals.green_angle);
		rot[2] = DEG2RAD(pvals.blue_angle);

		// always need at least one threshold matrix
		ASRT(rf);
		spotfn_list[rf].thresh = spot2thresh(&mm, rf, width);
		thresh[0] = spotfn_list[rf].thresh;

		// optimisation: only use separate threshold matrices if the
		// spot functions are actually different
		ASRT(gf);
		if (!spotfn_list[gf].thresh)
			spotfn_list[gf].thresh = spot2thresh(&mm, gf, width);
 		thresh[1] = spotfn_list[gf].thresh;

		ASRT(bf);
		if (!spotfn_list[bf].thresh)
			spotfn_list[bf].thresh = spot2thresh(&mm, bf, width);
		thresh[2] = spotfn_list[bf].thresh;

		if (colorspace == CS_CMYK) {
			rot[3] = DEG2RAD(pvals.gray_angle);
			gf = pvals.gray_spotfn;
			ASRT(gf);
			if (!spotfn_list[gf].thresh)
				spotfn_list[gf].thresh = spot2thresh(&mm, gf, width);
			thresh[3] = spotfn_list[gf].thresh;
		}
	}

progress_init(rc.Height() >> 1);
	for (int row = 0; row < rc.Height(); row++) {
		RGBA* ps = src.pos(rc.left, rc.top + row);
		for (int col = 0; col < rc.Width(); col++) {
			BYTE data[4];
			int rx = (rc.left + col) * oversample;
			int ry = (rc.top + row) * oversample;

			// convert rx and ry to polar (r, theta)
			r = sqrt(((double)rx) * rx + ((double)ry) * ry);
			theta = atan2(((double)ry), ((double)rx));

			data[0] = ps->red;
			data[1] = ps->green;
			data[2] = ps->blue;

			// do colour space conversion
			switch (colorspace) {
				case CS_CMYK:
					data[3] = 0xff;

					data[0] = 0xff - data[0];
					data[3] = MIN(data[3], data[0]);
					data[1] = 0xff - data[1];
					data[3] = MIN(data[3], data[1]);
					data[2] = 0xff - data[2];
					data[3] = MIN(data[3], data[2]);

					data[3] = (BYTE)((double)data[3] * (double)k_pullout);

					data[0] -= data[3];
					data[1] -= data[3];
					data[2] -= data[3];
					break;

				case CS_INTENSITY:
					data[3] = data[0];		// save orig for later
					data[0] = (BYTE)INTENSITY(data[0], data[1], data[2]);
					break;

				default:
					break;
			}

			for (b = 0; b < cspace_nchans[colorspace]; b++) {
				rx = (int)RINT(r * cos(theta + rot[b]));
				ry = (int)RINT(r * sin(theta + rot[b]));

				// Make sure rx and ry are positive and within
				// the range 0 .. width-1 (incl).  Can't use %
				// operator, since its definition on negative
				// numbers is not helpful.  Can't use ABS(),
				// since that would cause reflection about the
				// x- and y-axes.  Relies on integer division
				// rounding towards zero.
				rx -= ((rx - ISNEG(rx) * (width - 1)) / width) * width;
				ry -= ((ry - ISNEG(ry) * (width - 1)) / width) * width;

				{
					DWORD sum = 0;
					int sx, sy;
					int tx, ty;
					for (sy = -oversample / 2; sy <= oversample / 2; sy++) {
						for (sx = -oversample / 2; sx <= oversample / 2; sx++) {
							tx = rx+sx;
							ty = ry+sy;
							while (tx < 0)  tx += width;
							while (ty < 0)  ty += width;
							while (tx >= width)  tx -= width;
							while (ty >= width)  ty -= width;
							if (data[b] > THRESHn(b, tx, ty))
								sum += 0xff * BARTLETT(sx, sy, oversample);
						}
					}
					sum /= w002;
					data[b] = (BYTE)sum;
				}
			}

			// re-pack the colours into RGB
			switch (colorspace) {
				case CS_CMYK:
					data[0] = CLAMPED_ADD(data[0], data[3]);
					data[1] = CLAMPED_ADD(data[1], data[3]);
					data[2] = CLAMPED_ADD(data[2], data[3]);
					data[0] = 0xff - data[0];
					data[1] = 0xff - data[1];
					data[2] = 0xff - data[2];
					break;

				case CS_INTENSITY:
					data[1] = data[1] * data[0] / 0xff;
					data[2] = data[2] * data[0] / 0xff;
					data[0] = data[3] * data[0] / 0xff;
					break;

				default:
					// no other special cases
					break;
			}

			ps->red   = data[0];
			ps->green = data[1];
			ps->blue  = data[2];
			ps++;
		}
if (row & 1) progress_continue();
	}
progress_term();
	return TRUE;
}
