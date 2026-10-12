////////////////////////////////////////////////////////////////////////
// quantize.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"
#include "fsdither.h"
#include "dither_table.h"

#define PRECISION_R 6
#define PRECISION_G 6
#define PRECISION_B 5

#define HIST_R_ELEMS (1<<PRECISION_R)
#define HIST_G_ELEMS (1<<PRECISION_G)
#define HIST_B_ELEMS (1<<PRECISION_B)

#define MR HIST_G_ELEMS*HIST_B_ELEMS
#define MG HIST_B_ELEMS

#define BITS_IN_SAMPLE 8

#define R_SHIFT  (BITS_IN_SAMPLE-PRECISION_R)
#define G_SHIFT  (BITS_IN_SAMPLE-PRECISION_G)
#define B_SHIFT  (BITS_IN_SAMPLE-PRECISION_B)

#define R_SCALE 30               /*  scale R distances by this much  */
#define G_SCALE 59               /*  scale G distances by this much  */
#define B_SCALE 11               /*  and B by this much  */

#define MAXNUMCOLORS		256

typedef void (*PFNPASS1)(void*);
typedef void (*PFNPASS2)(void*, void*, BYTE*);
typedef void (*PFNCLEANUP)(void*);
typedef unsigned long ColorFreq;
typedef ColorFreq* Histogram;
typedef unsigned __int64 uint64;

typedef struct _Color {
	int red;
	int green;
	int blue;
} Color;

typedef struct {
	signed long used_count;
	unsigned char initial_index;
} palentryStruct;

typedef struct _QuantizeObj {
	PFNPASS1 pass_1st;			// first pass over image data creates colormap
	PFNPASS2 pass_2nd;			// second pass maps from image data to colormap
	PFNCLEANUP delete_func; 	// function to clean up data associated with private
	int desired_number_of_colors;	// Number of colors we will allow
	int actual_number_of_colors;	// Number of colors actually needed
	Color cmap[256];  				// colormap created by quantization
	Histogram histogram;			// holds the histogram
	unsigned long index_used_count[256]; /* how many times an index was used */
	int error_freedom;				// 0=much bleed, 1=controlled bleed
} QuantizeObj;

typedef struct {
	// The bounds of the box (inclusive); expressed as histogram indexes
	int   Rmin, Rmax;
	int   Rhalferror;
	int   Gmin, Gmax;
	int   Ghalferror;
	int   Bmin, Bmax;
	int   Bhalferror;

	// The volume (actually 2-norm) of the box
	int   volume;

	// The number of nonzero histogram cells within this box
	long  colorcount;

	// The sum of the weighted error within this box
	uint64 error;

	// The sum of the unweighted error within this box
	uint64 rerror;
	uint64 gerror;
	uint64 berror;
} box, *boxptr;

typedef struct tagINDEXEDIMAGE {
	int width;
	int height;
	int colors;
	Color cmap[256];
	BYTE data[1];
} INDEXEDIMAGE;


void __cdecl qsort_ex(void *base, size_t n, size_t size,
	int (__cdecl *comp)(const void*, const void*, const void*), const void* param);

static void __fastcall compute_color(QuantizeObj *qtobj,
		   Histogram    histogram,
		   boxptr       boxp,
		   int          icolor);

static RGBA found_cols[MAXNUMCOLORS];
static int num_found_cols;
static BOOL needs_quantize;

LOGPALETTE* theCustomPalette = NULL;


/*
 *  Indexed color conversion machinery
 */

static void __fastcall zero_histogram(Histogram histogram)
{
	register int r, g, b;
	for (r = 0; r < HIST_R_ELEMS; r++)
		for (g = 0; g < HIST_G_ELEMS; g++)
			for (b = 0; b < HIST_B_ELEMS; b++)
				histogram[r * MR + g * MG + b] = 0;
}


static void __fastcall generate_histogram(Histogram histogram, mif* image, int col_limit)
{
	ColorFreq *col;
	int nfc_iter;

	register RGBA* data = image->pixels();
	int size = image->width() * image->height();

	if (needs_quantize) {
		while (size--) {
			col = &histogram[(data->red   >> R_SHIFT) * MR +
							 (data->green >> G_SHIFT) * MG +
							 (data->blue  >> B_SHIFT)];
			(*col)++;
			data++;
		}
	}
	else {
		while (size--) {
			col = &histogram[(data->red   >> R_SHIFT) * MR +
							 (data->green >> G_SHIFT) * MG +
							 (data->blue  >> B_SHIFT)];
			(*col)++;

			if (!needs_quantize) {
				for (nfc_iter = 0; nfc_iter < num_found_cols; nfc_iter++) {
					if ((data->red   == found_cols[nfc_iter].red) &&
						(data->green == found_cols[nfc_iter].green) &&
						(data->blue  == found_cols[nfc_iter].blue))
						goto already_found;
				}

				// Colour was not in the table of existing colours
				num_found_cols++;

				if (num_found_cols > col_limit) {
					// There are more colours in the image
					// than were allowed.  We switch to plain
					// histogram calculation with a view to
					// quantizing at a later stage.
					needs_quantize = TRUE;
					goto already_found;
				}
				else {
					// Remember the new colour we just found.
					found_cols[num_found_cols-1].red   = data->red;
					found_cols[num_found_cols-1].green = data->green;
					found_cols[num_found_cols-1].blue  = data->blue;
				}
			}
already_found:
			data++;
		}
	}
}

static boxptr find_split_candidate(boxptr boxlist, int numboxes)
{
	register boxptr boxp;
	register boxptr which = NULL;
	int i;
	uint64 maxc = 0;

	for (i = 0, boxp = boxlist; i < numboxes; i++, boxp++) {
		if (boxp->volume > 0) {
			if (boxp->gerror*G_SCALE > maxc) {
				which = boxp;
				maxc = boxp->gerror*G_SCALE;
			}
			if (boxp->rerror*R_SCALE > maxc) {
				which = boxp;
				maxc = boxp->rerror*R_SCALE;
			}
			if (boxp->berror*B_SCALE > maxc) {
				which = boxp;
				maxc = boxp->berror*B_SCALE;
			}
		}
    }
	return which;
}


/* Find the splittable box with the largest (scaled) volume */
/* Returns NULL if no splittable boxes remain */
static boxptr find_biggest_volume (boxptr boxlist, int numboxes)
{
	register boxptr boxp;
	register boxptr which = NULL;
	int i;
	int maxv = 0;

	for (i = 0, boxp = boxlist; i < numboxes; i++, boxp++) {
		if (boxp->volume > maxv) {
			which = boxp;
			maxv = boxp->volume;
		}
	}
	return which;
}

static void __fastcall update_box(Histogram histogram, boxptr boxp)
/* Shrink the min/max bounds of a box to enclose only nonzero elements, */
/* and recompute its volume, population and error */
{
	ColorFreq * histp;
	int R,G,B;
	int Rmin,Rmax,Gmin,Gmax,Bmin,Bmax;
	int dist0,dist1,dist2;
	long ccount;
	uint64 tempRerror;
	uint64 tempGerror;
	uint64 tempBerror;
	QuantizeObj dummyqo;
	box dummybox;

	Rmin = boxp->Rmin;  Rmax = boxp->Rmax;
	Gmin = boxp->Gmin;  Gmax = boxp->Gmax;
	Bmin = boxp->Bmin;  Bmax = boxp->Bmax;

	if (Rmax > Rmin)
		for (R = Rmin; R <= Rmax; R++)
			for (G = Gmin; G <= Gmax; G++) {
				histp = histogram + R*MR + G*MG + Bmin;
				for (B = Bmin; B <= Bmax; B++)
					if (*histp++ != 0) {
						boxp->Rmin = Rmin = R;
						goto have_Rmin;
					}
			}

have_Rmin:
	if (Rmax > Rmin)
		for (R = Rmax; R >= Rmin; R--)
			for (G = Gmin; G <= Gmax; G++) {
				histp = histogram + R*MR + G*MG + Bmin;
				for (B = Bmin; B <= Bmax; B++)
					if (*histp++ != 0) {
						boxp->Rmax = Rmax = R;
						goto have_Rmax;
					}
			}

have_Rmax:
	if (Gmax > Gmin)
		for (G = Gmin; G <= Gmax; G++)
			for (R = Rmin; R <= Rmax; R++) {
				histp = histogram + R*MR + G*MG + Bmin;
				for (B = Bmin; B <= Bmax; B++)
					if (*histp++ != 0) {
						boxp->Gmin = Gmin = G;
						goto have_Gmin;
					}
			}

have_Gmin:
	if (Gmax > Gmin)
		for (G = Gmax; G >= Gmin; G--)
			for (R = Rmin; R <= Rmax; R++) {
				histp = histogram + R*MR + G*MG + Bmin;
				for (B = Bmin; B <= Bmax; B++)
					if (*histp++ != 0) {
						boxp->Gmax = Gmax = G;
						goto have_Gmax;
					}
			}

have_Gmax:
	if (Bmax > Bmin)
		for (B = Bmin; B <= Bmax; B++)
			for (R = Rmin; R <= Rmax; R++) {
				histp = histogram + R*MR + Gmin*MG + B;
				for (G = Gmin; G <= Gmax; G++, histp += MG)
					if (*histp != 0) {
						boxp->Bmin = Bmin = B;
						goto have_Bmin;
					}
			}

have_Bmin:
	if (Bmax > Bmin)
		for (B = Bmax; B >= Bmin; B--)
			for (R = Rmin; R <= Rmax; R++) {
				histp = histogram + R*MR + Gmin*MG + B;
				for (G = Gmin; G <= Gmax; G++, histp += MG)
					if (*histp != 0) {
						boxp->Bmax = Bmax = B;
						goto have_Bmax;
					}
			}

have_Bmax:

	/* Update box volume.
	 * We use 2-norm rather than real volume here; this biases the method
	 * against making long narrow boxes, and it has the side benefit that
	 * a box is splittable iff norm > 0.
	 * Since the differences are expressed in histogram-cell units,
	 * we have to shift back to JSAMPLE units to get consistent distances;
	 * after which, we scale according to the selected distance scale factors.
	 */
	dist0 = (( + Rmax - Rmin) << R_SHIFT) * R_SCALE;
	dist1 = (( + Gmax - Gmin) << G_SHIFT) * G_SCALE;
	dist2 = (( + Bmax - Bmin) << B_SHIFT) * B_SCALE;
	boxp->volume = dist0*dist0 + dist1*dist1 + dist2*dist2;

	compute_color(&dummyqo, histogram, boxp, 0);

	/* Now scan remaining volume of box and compute population */
	ccount = 0;
	boxp->error = 0;
	boxp->rerror = 0;
	boxp->gerror = 0;
	boxp->berror = 0;
	for (R = Rmin; R <= Rmax; R++)
		for (G = Gmin; G <= Gmax; G++) {
			histp = histogram + R*MR + G*MG + Bmin;
			for (B = Bmin; B <= Bmax; B++, histp++)
				if (*histp != 0) {
					int ge, be, re;

					dummybox.Rmin = dummybox.Rmax = R;
					dummybox.Gmin = dummybox.Gmax = G;
					dummybox.Bmin = dummybox.Bmax = B;
					compute_color(&dummyqo, histogram, &dummybox, 1);

					re = dummyqo.cmap[0].red   - dummyqo.cmap[1].red;
					ge = dummyqo.cmap[0].green - dummyqo.cmap[1].green;
					be = dummyqo.cmap[0].blue  - dummyqo.cmap[1].blue;

					boxp->rerror += (*histp) * re*re;
					boxp->gerror += (*histp) * ge*ge;
					boxp->berror += (*histp) * be*be;

					boxp->error +=
						(*histp) * (re*re*R_SCALE + ge*ge*G_SCALE + be*be*B_SCALE);
					ccount += *histp;
				}
		}

	/* Scan again, taking note of halfway error point for red axis */
	tempRerror = 0;
	boxp->Rhalferror = Rmin;
	for (R = Rmin; R < Rmax; R++) {
		dummybox.Rmin = dummybox.Rmax = R;
		for (G = Gmin; G <= Gmax; G++) {
			dummybox.Gmin = dummybox.Gmax = G;
			histp = histogram + R*MR + G*MG + Bmin;
			for (B = Bmin; B <= Bmax; B++, histp++)
				if (*histp != 0) {
					int re;
					dummybox.Bmin = dummybox.Bmax = B;
					compute_color(&dummyqo, histogram, &dummybox, 1);
					re = dummyqo.cmap[0].red   - dummyqo.cmap[1].red;
					tempRerror += (*histp)*re*re;

					if (tempRerror*2 > boxp->rerror)
						goto green_axisscan;
					else
						boxp->Rhalferror = R;
				}
		}
	}

green_axisscan:
	// Scan again, taking note of halfway error point for green axis
	tempGerror = 0;
	boxp->Ghalferror = Gmin;
	for (G = Gmin; G < Gmax; G++) {
		dummybox.Gmin = dummybox.Gmax = G;
		for (R = Rmin; R <= Rmax; R++) {
			dummybox.Rmin = dummybox.Rmax = R;
			histp = histogram + R*MR + G*MG + Bmin;
			for (B = Bmin; B <= Bmax; B++, histp++)
				if (*histp != 0) {
					int ge;
					dummybox.Bmin = dummybox.Bmax = B;
					compute_color(&dummyqo, histogram, &dummybox, 1);
					ge = dummyqo.cmap[0].green - dummyqo.cmap[1].green;
					tempGerror += (*histp)*ge*ge;

					if (tempGerror*2 > boxp->gerror)
						goto blue_axisscan;
					else
						boxp->Ghalferror = G;
				}
		}
	}

blue_axisscan:
	// Scan again, taking note of halfway error point for blue axis
	tempBerror = 0;
	boxp->Bhalferror = Bmin;
	for (B = Bmin; B < Bmax; B++) {
		dummybox.Bmin = dummybox.Bmax = B;
		for (R = Rmin; R <= Rmax; R++) {
			dummybox.Rmin = dummybox.Rmax = R;
			for (G = Gmin; G <= Gmax; G++) {
				histp = histogram + R*MR + G*MG + B;
				if (*histp != 0) {
					int be;		  
					dummybox.Gmin = dummybox.Gmax = G;
					compute_color(&dummyqo, histogram, &dummybox, 1);
					be = dummyqo.cmap[0].blue  - dummyqo.cmap[1].blue;
					tempBerror += (*histp)*be*be;

					if (tempBerror*2 > boxp->berror)
						goto finished_axesscan;
					else
						boxp->Bhalferror = B;
				}
			}
		}
	}

finished_axesscan:
	boxp->colorcount = ccount;
}

static int __fastcall mediancut(Histogram histogram,
		boxptr    boxlist,
		int       numboxes,
		int       desired_colors)
/* Repeatedly select and split the largest box until we have enough boxes */
{
  int n,lb;
  uint64 R,G,B,cmax;
  boxptr b1,b2;

  while (numboxes < desired_colors) {
    b1 = find_split_candidate (boxlist, numboxes);

    if (b1 == NULL)		/* no splittable boxes left! */
      break;
    b2 = boxlist + numboxes;	/* where new box will go */
    /* Copy the color bounds to the new box. */
    b2->Rmax = b1->Rmax; b2->Gmax = b1->Gmax; b2->Bmax = b1->Bmax;
    b2->Rmin = b1->Rmin; b2->Gmin = b1->Gmin; b2->Bmin = b1->Bmin;
    /* Choose which axis to split the box on.
     * See notes in update_box about scaling distances.
     */
    /*
    //        R = ((b1->Rmax - b1->Rmin) << R_SHIFT) * R_SCALE;
	//        G = ((b1->Gmax - b1->Gmin) << G_SHIFT) * G_SCALE;
        //B = ((b1->Bmax - b1->Bmin) << B_SHIFT) * B_SCALE;
    */
    R = R_SCALE*b1->rerror;/* * (((b1->Rmax - b1->Rmin) << R_SHIFT)) * R_SCALE; */
    G = G_SCALE*b1->gerror;/* * (((b1->Gmax - b1->Gmin) << G_SHIFT)) * G_SCALE; */
    B = B_SCALE*b1->berror;/* * (((b1->Bmax - b1->Bmin) << B_SHIFT)) * B_SCALE; */
    /* We want to break any ties in favor of green, then red, blue last.
     */
    cmax = G; n = 1;
    if (R > cmax) { cmax = R; n = 0; }
    if (B > cmax) { n = 2; }

    /* Choose split point along selected axis, and update box bounds.
     * Note that lb value is max for lower box, so must be < old max.
     */
    switch (n)
      {
      case 0:
	lb = b1->Rhalferror;/* *0 + (b1->Rmax + b1->Rmin) / 2; */
	b1->Rmax = lb;
	b2->Rmin = lb+1;
	break;
      case 1:
	lb = b1->Ghalferror;/* *0 + (b1->Gmax + b1->Gmin) / 2; */
	b1->Gmax = lb;
	b2->Gmin = lb+1;
	break;
      case 2:
	lb = b1->Bhalferror;/* *0 + (b1->Bmax + b1->Bmin) / 2; */
	b1->Bmax = lb;
	b2->Bmin = lb+1;
	break;
      }
    /* Update stats for boxes */
    update_box(histogram, b1);
    update_box(histogram, b2);
    numboxes++;
  }
  return numboxes;
}


static void __fastcall compute_color(QuantizeObj *qtobj,
		   Histogram    histogram,
		   boxptr       boxp,
		   int          icolor)
/* Compute representative color for a box, put it in colormap[icolor] */
{
	/* Current algorithm: mean weighted by pixels (not colors) */
	/* Note it is important to get the rounding correct! */
	ColorFreq * histp;
	int R, G, B;
	int Rmin, Rmax;
	int Gmin, Gmax;
	int Bmin, Bmax;
	long count;
	long total = 0;
	long Rtotal = 0;
	long Gtotal = 0;
	long Btotal = 0;

	Rmin = boxp->Rmin;  Rmax = boxp->Rmax;
	Gmin = boxp->Gmin;  Gmax = boxp->Gmax;
	Bmin = boxp->Bmin;  Bmax = boxp->Bmax;

	for (R = Rmin; R <= Rmax; R++)
		for (G = Gmin; G <= Gmax; G++) {
			histp = histogram + R*MR + G*MG + Bmin;
			for (B = Bmin; B <= Bmax; B++) {
				if ((count = *histp++) != 0) {
					total += count;
					Rtotal += ((R << R_SHIFT) + ((1<<R_SHIFT)>>1)) * count;
					Gtotal += ((G << G_SHIFT) + ((1<<G_SHIFT)>>1)) * count;
					Btotal += ((B << B_SHIFT) + ((1<<B_SHIFT)>>1)) * count;
				}
			}
		}

	if (total != 0) {
		qtobj->cmap[icolor].red = (Rtotal + (total>>1)) / total;
		qtobj->cmap[icolor].green = (Gtotal + (total>>1)) / total;
		qtobj->cmap[icolor].blue = (Btotal + (total>>1)) / total;
	}
	else {
		/* The only situation where total==0 is if the image was null or
		 *  all-transparent.  In that case we just put a dummy value in
		 *  the colourmap.
		 */
		qtobj->cmap[icolor].red = qtobj->cmap[icolor].green = qtobj->cmap[icolor].blue = 0;
	}
}

static int __cdecl mapsort(const void* p1, const void* p2, const void* /*param*/)
{
	register Color* ps = (Color*)p1;
	register Color* pt = (Color*)p2;

	// 輝度でソート???
	int s = rgb_to_l(ps->red, ps->green, ps->blue);
	int t = rgb_to_l(pt->red, pt->green, pt->blue);
	if (s == t) {
		// インテンシティでソート???
		s = (int)ps->red * 77 + (int)ps->green * 150 + (int)ps->blue * 29;
		t = (int)pt->red * 77 + (int)pt->green * 150 + (int)pt->blue * 29;
	}
	return (s - t);
}

static void __fastcall select_colors(QuantizeObj *qtobj, Histogram histogram)
{
	// Master routine for color selection
	register boxptr boxlist;
	int numboxes;
	int desired = qtobj->desired_number_of_colors;
	int i;

	// Allocate workspace for box list
	boxlist = (boxptr)malloc(desired * sizeof(box) );

	// Initialize one box containing whole space
	numboxes = 1;
	boxlist[0].Rmin = 0;
	boxlist[0].Rmax = (1 << PRECISION_R) - 1;
	boxlist[0].Gmin = 0;
	boxlist[0].Gmax = (1 << PRECISION_G) - 1;
	boxlist[0].Bmin = 0;
	boxlist[0].Bmax = (1 << PRECISION_B) - 1;

	// Shrink it to actually-used volume and set its statistics
	update_box(histogram, boxlist);

	// Perform median-cut to produce final box list
	numboxes = mediancut(histogram, boxlist, numboxes, desired);

	qtobj->actual_number_of_colors = numboxes;

	// Compute the representative color for each box, fill colormap
	for (i = 0; i < numboxes; i++)
		compute_color(qtobj, histogram, boxlist + i, i);

// カラーマップのソート
qsort_ex(&qtobj->cmap[0], qtobj->actual_number_of_colors, sizeof(Color), mapsort, NULL);


}


/*
 * These routines are concerned with the time-critical task of mapping input
 * colors to the nearest color in the selected colormap.
 *
 * We re-use the histogram space as an "inverse color map", essentially a
 * cache for the results of nearest-color searches.  All colors within a
 * histogram cell will be mapped to the same colormap entry, namely the one
 * closest to the cell's center.  This may not be quite the closest entry to
 * the actual input color, but it's almost as good.  A zero in the cache
 * indicates we haven't found the nearest color for that cell yet; the array
 * is cleared to zeroes before starting the mapping pass.  When we find the
 * nearest color for a cell, its colormap index plus one is recorded in the
 * cache for future use.  The pass2 scanning routines call fill_inverse_cmap
 * when they need to use an unfilled entry in the cache.
 *
 * Our method of efficiently finding nearest colors is based on the "locally
 * sorted search" idea described by Heckbert and on the incremental distance
 * calculation described by Spencer W. Thomas in chapter III.1 of Graphics
 * Gems II (James Arvo, ed.  Academic Press, 1991).  Thomas points out that
 * the distances from a given colormap entry to each cell of the histogram can
 * be computed quickly using an incremental method: the differences between
 * distances to adjacent cells themselves differ by a constant.  This allows a
 * fairly fast implementation of the "brute force" approach of computing the
 * distance from every colormap entry to every histogram cell.  Unfortunately,
 * it needs a work array to hold the best-distance-so-far for each histogram
 * cell (because the inner loop has to be over cells, not colormap entries).
 * The work array elements have to be ints, so the work array would need
 * 256Kb at our recommended precision.  This is not feasible in DOS machines.
 *
 * To get around these problems, we apply Thomas' method to compute the
 * nearest colors for only the cells within a small subbox of the histogram.
 * The work array need be only as big as the subbox, so the memory usage
 * problem is solved.  Furthermore, we need not fill subboxes that are never
 * referenced in pass2; many images use only part of the color gamut, so a
 * fair amount of work is saved.  An additional advantage of this
 * approach is that we can apply Heckbert's locality criterion to quickly
 * eliminate colormap entries that are far away from the subbox; typically
 * three-fourths of the colormap entries are rejected by Heckbert's criterion,
 * and we need not compute their distances to individual cells in the subbox.
 * The speed of this approach is heavily influenced by the subbox size: too
 * small means too much overhead, too big loses because Heckbert's criterion
 * can't eliminate as many colormap entries.  Empirically the best subbox
 * size seems to be about 1/512th of the histogram (1/8th in each direction).
 *
 * Thomas' article also describes a refined method which is asymptotically
 * faster than the brute-force method, but it is also far more complex and
 * cannot efficiently be applied to small subboxes.  It is therefore not
 * useful for programs intended to be portable to DOS machines.  On machines
 * with plenty of memory, filling the whole histogram in one shot with Thomas'
 * refined method might be faster than the present code --- but then again,
 * it might not be any faster, and it's certainly more complicated.
 */


/* log2(histogram cells in update box) for each axis; this can be adjusted */
#define BOX_R_LOG  (PRECISION_R-3)
#define BOX_G_LOG  (PRECISION_G-3)
#define BOX_B_LOG  (PRECISION_B-3)

#define BOX_R_ELEMS  (1<<BOX_R_LOG) /* # of hist cells in update box */
#define BOX_G_ELEMS  (1<<BOX_G_LOG)
#define BOX_B_ELEMS  (1<<BOX_B_LOG)

#define BOX_R_SHIFT  (R_SHIFT + BOX_R_LOG)
#define BOX_G_SHIFT  (G_SHIFT + BOX_G_LOG)
#define BOX_B_SHIFT  (B_SHIFT + BOX_B_LOG)


/*
 * The next three routines implement inverse colormap filling.  They could
 * all be folded into one big routine, but splitting them up this way saves
 * some stack space (the mindist[] and bestdist[] arrays need not coexist)
 * and may allow some compilers to produce better code by registerizing more
 * inner-loop variables.
 */

static int __fastcall
find_nearby_colors(QuantizeObj *qtobj,
		    int          minR,
		    int          minG,
		    int          minB,
		    int          colorlist[])
/* Locate the colormap entries close enough to an update box to be candidates
 * for the nearest entry to some cell(s) in the update box.  The update box
 * is specified by the center coordinates of its first cell.  The number of
 * candidate colormap entries is returned, and their colormap indexes are
 * placed in colorlist[].
 * This routine uses Heckbert's "locally sorted search" criterion to select
 * the colors that need further consideration.
 */
{
  int numcolors = qtobj->actual_number_of_colors;
  int maxR, maxG, maxB;
  int centerR, centerG, centerB;
  int i, x, ncolors;
  int minmaxdist, min_dist, max_dist, tdist;
  int mindist[MAXNUMCOLORS];	/* min distance to colormap entry i */

  /* Compute true coordinates of update box's upper corner and center.
   * Actually we compute the coordinates of the center of the upper-corner
   * histogram cell, which are the upper bounds of the volume we care about.
   * Note that since ">>" rounds down, the "center" values may be closer to
   * min than to max; hence comparisons to them must be "<=", not "<".
   */
  maxR = minR + ((1 << BOX_R_SHIFT) - (1 << R_SHIFT));
  centerR = (minR + maxR) >> 1;
  maxG = minG + ((1 << BOX_G_SHIFT) - (1 << G_SHIFT));
  centerG = (minG + maxG) >> 1;
  maxB = minB + ((1 << BOX_B_SHIFT) - (1 << B_SHIFT));
  centerB = (minB + maxB) >> 1;

  /* For each color in colormap, find:
   *  1. its minimum squared-distance to any point in the update box
   *     (zero if color is within update box);
   *  2. its maximum squared-distance to any point in the update box.
   * Both of these can be found by considering only the corners of the box.
   * We save the minimum distance for each color in mindist[];
   * only the smallest maximum distance is of interest.
   */
  minmaxdist = 0x7FFFFFFFL;

  for (i = 0; i < numcolors; i++) {
    /* We compute the squared-R-distance term, then add in the other two. */
    x = qtobj->cmap[i].red;
    if (x < minR) {
      tdist = (x - minR) * R_SCALE;
      min_dist = tdist*tdist;
      tdist = (x - maxR) * R_SCALE;
      max_dist = tdist*tdist;
    } else if (x > maxR) {
      tdist = (x - maxR) * R_SCALE;
      min_dist = tdist*tdist;
      tdist = (x - minR) * R_SCALE;
      max_dist = tdist*tdist;
    } else {
      /* within cell range so no contribution to min_dist */
      min_dist = 0;
      if (x <= centerR) {
	tdist = (x - maxR) * R_SCALE;
	max_dist = tdist*tdist;
      } else {
	tdist = (x - minR) * R_SCALE;
	max_dist = tdist*tdist;
      }
    }

    x = qtobj->cmap[i].green;
    if (x < minG) {
      tdist = (x - minG) * G_SCALE;
      min_dist += tdist*tdist;
      tdist = (x - maxG) * G_SCALE;
      max_dist += tdist*tdist;
    } else if (x > maxG) {
      tdist = (x - maxG) * G_SCALE;
      min_dist += tdist*tdist;
      tdist = (x - minG) * G_SCALE;
      max_dist += tdist*tdist;
    } else {
      /* within cell range so no contribution to min_dist */
      if (x <= centerG) {
	tdist = (x - maxG) * G_SCALE;
	max_dist += tdist*tdist;
      } else {
	tdist = (x - minG) * G_SCALE;
	max_dist += tdist*tdist;
      }
    }

    x = qtobj->cmap[i].blue;
    if (x < minB) {
      tdist = (x - minB) * B_SCALE;
      min_dist += tdist*tdist;
      tdist = (x - maxB) * B_SCALE;
      max_dist += tdist*tdist;
    } else if (x > maxB) {
      tdist = (x - maxB) * B_SCALE;
      min_dist += tdist*tdist;
      tdist = (x - minB) * B_SCALE;
      max_dist += tdist*tdist;
    } else {
      /* within cell range so no contribution to min_dist */
      if (x <= centerB) {
	tdist = (x - maxB) * B_SCALE;
	max_dist += tdist*tdist;
      } else {
	tdist = (x - minB) * B_SCALE;
	max_dist += tdist*tdist;
      }
    }

    mindist[i] = min_dist;	/* save away the results */
    if (max_dist < minmaxdist)
      minmaxdist = max_dist;
  }

  /* Now we know that no cell in the update box is more than minmaxdist
   * away from some colormap entry.  Therefore, only colors that are
   * within minmaxdist of some part of the box need be considered.
   */
  ncolors = 0;
  for (i = 0; i < numcolors; i++) {
    if (mindist[i] <= minmaxdist)
      colorlist[ncolors++] = i;
  }
  return ncolors;
}


static void __fastcall
find_best_colors (QuantizeObj *qtobj,
		  int          minR,
		  int          minG,
		  int          minB,
		  int          numcolors,
		  int          colorlist[],
		  int          bestcolor[])
/* Find the closest colormap entry for each cell in the update box,
 * given the list of candidate colors prepared by find_nearby_colors.
 * Return the indexes of the closest entries in the bestcolor[] array.
 * This routine uses Thomas' incremental distance calculation method to
 * find the distance from a colormap entry to successive cells in the box.
 */
{
  int iR, iG, iB;
  int i, icolor;
  int * bptr;		/* pointer into bestdist[] array */
  int * cptr;		/* pointer into bestcolor[] array */
  int dist0, dist1;	/* initial distance values */
  int dist2;		/* current distance in inner loop */
  int xx0, xx1;		/* distance increments */
  int xx2;
  int inR, inG, inB;	/* initial values for increments */

  /* This array holds the distance to the nearest-so-far color for each cell */
  int bestdist[BOX_R_ELEMS * BOX_G_ELEMS * BOX_B_ELEMS];

  /* Initialize best-distance for each cell of the update box */
  bptr = bestdist;
  for (i = BOX_R_ELEMS*BOX_G_ELEMS*BOX_B_ELEMS-1; i >= 0; i--)
    *bptr++ = 0x7FFFFFFFL;

  /* For each color selected by find_nearby_colors,
   * compute its distance to the center of each cell in the box.
   * If that's less than best-so-far, update best distance and color number.
   */

  /* Nominal steps between cell centers ("x" in Thomas article) */
#define STEP_R  ((1 << R_SHIFT) * R_SCALE)
#define STEP_G  ((1 << G_SHIFT) * G_SCALE)
#define STEP_B  ((1 << B_SHIFT) * B_SCALE)

  for (i = 0; i < numcolors; i++) {
    icolor = colorlist[i];
    /* Compute (square of) distance from minR/G/B to this color */
    inR = (minR - qtobj->cmap[icolor].red) * R_SCALE;
    dist0 = inR*inR;
    inG = (minG - qtobj->cmap[icolor].green) * G_SCALE;
    dist0 += inG*inG;
    inB = (minB - qtobj->cmap[icolor].blue) * B_SCALE;
    dist0 += inB*inB;
    /* Form the initial difference increments */
    inR = inR * (2 * STEP_R) + STEP_R * STEP_R;
    inG = inG * (2 * STEP_G) + STEP_G * STEP_G;
    inB = inB * (2 * STEP_B) + STEP_B * STEP_B;
    /* Now loop over all cells in box, updating distance per Thomas method */
    bptr = bestdist;
    cptr = bestcolor;
    xx0 = inR;
    for (iR = BOX_R_ELEMS-1; iR >= 0; iR--) {
      dist1 = dist0;
      xx1 = inG;
      for (iG = BOX_G_ELEMS-1; iG >= 0; iG--) {
	dist2 = dist1;
	xx2 = inB;
	for (iB = BOX_B_ELEMS-1; iB >= 0; iB--) {
	  if (dist2 < *bptr) {
	    *bptr = dist2;
	    *cptr = icolor;
	  }
	  dist2 += xx2;
	  xx2 += 2 * STEP_B * STEP_B;
	  bptr++;
	  cptr++;
	}
	dist1 += xx1;
	xx1 += 2 * STEP_G * STEP_G;
      }
      dist0 += xx0;
      xx0 += 2 * STEP_R * STEP_R;
    }
  }
}

static void __fastcall 
fill_inverse_cmap(QuantizeObj *qtobj,
		       Histogram    histogram,
		       int          R,
		       int          G,
		       int          B)
/* Fill the inverse-colormap entries in the update box that contains */
/* histogram cell R/G/B.  (Only that one cell MUST be filled, but */
/* we can fill as many others as we wish.) */
{
  int minR, minG, minB;	/* lower left corner of update box */
  int iR, iG, iB;
  int * cptr;   	/* pointer into bestcolor[] array */
  ColorFreq * cachep;	/* pointer into main cache array */
  /* This array lists the candidate colormap indexes. */
  int colorlist[MAXNUMCOLORS];
  int numcolors;		/* number of candidate colors */
  /* This array holds the actually closest colormap index for each cell. */
  int bestcolor[BOX_R_ELEMS * BOX_G_ELEMS * BOX_B_ELEMS];

  /* Convert cell coordinates to update box id */
  R >>= BOX_R_LOG;
  G >>= BOX_G_LOG;
  B >>= BOX_B_LOG;

  /* Compute true coordinates of update box's origin corner.
   * Actually we compute the coordinates of the center of the corner
   * histogram cell, which are the lower bounds of the volume we care about.
   */
  minR = (R << BOX_R_SHIFT) + ((1 << R_SHIFT) >> 1);
  minG = (G << BOX_G_SHIFT) + ((1 << G_SHIFT) >> 1);
  minB = (B << BOX_B_SHIFT) + ((1 << B_SHIFT) >> 1);

  /* Determine which colormap entries are close enough to be candidates
   * for the nearest entry to some cell in the update box.
   */
  numcolors = find_nearby_colors (qtobj, minR, minG, minB, colorlist);

  /* Determine the actually nearest colors. */
  find_best_colors (qtobj, minR, minG, minB, numcolors, colorlist,
		    bestcolor);

  /* Save the best color numbers (plus 1) in the main cache array */
  R <<= BOX_R_LOG;		/* convert id back to base cell indexes */
  G <<= BOX_G_LOG;
  B <<= BOX_B_LOG;
  cptr = bestcolor;
  for (iR = 0; iR < BOX_R_ELEMS; iR++) {
    for (iG = 0; iG < BOX_G_ELEMS; iG++) {
      cachep = & histogram[(R+iR)*MR+(G+iG)*MG+B];
      for (iB = 0; iB < BOX_B_ELEMS; iB++) {
	*cachep++ = (*cptr++) + 1;
      }
    }
  }
}


/*  This is pass 1  */

static void mediancut_pass1(QuantizeObj *qtobj)
{
	select_colors(qtobj, qtobj->histogram);
}

static void monopal_pass1(QuantizeObj *qtobj)
{
	qtobj->actual_number_of_colors = 2;
	qtobj->cmap[0].red = 0;
	qtobj->cmap[0].green = 0;
	qtobj->cmap[0].blue = 0;
	qtobj->cmap[1].red = 255;
	qtobj->cmap[1].green = 255;
	qtobj->cmap[1].blue = 255;
}

static void custompal_pass1(QuantizeObj *qtobj)
{
	int entries = (int)theCustomPalette->palNumEntries;
	for (int i = 0; i < entries; i++) {
		qtobj->cmap[i].red = theCustomPalette->palPalEntry[i].peRed;
		qtobj->cmap[i].green = theCustomPalette->palPalEntry[i].peGreen;
		qtobj->cmap[i].blue = theCustomPalette->palPalEntry[i].peBlue;
	}
	qtobj->actual_number_of_colors = entries;
}

/*
 * Map some rows of pixels to the output colormapped representation.
 */

static void
mediancut_pass2_no_dither(QuantizeObj* qtobj, mif* image, BYTE* idxdata)
{
	Histogram histogram = qtobj->histogram;
	ColorFreq * cachep;
	int R, G, B;
	unsigned long* index_used_count = qtobj->index_used_count;

	zero_histogram(histogram);

	RGBA* src = image->pixels();
	BYTE* dst = idxdata;

	int size = image->width() * image->height();
	for (int i = 0; i < size; i++)
	{
		// get pixel value and index into the cache
		R = (src->red) >> R_SHIFT;
		G = (src->green) >> G_SHIFT;
		B = (src->blue) >> B_SHIFT;
		cachep = &histogram[R * MR + G * MG + B];

		// If we have not seen this color before, find nearest colormap entry
		// and update the cache
		if (*cachep == 0)
			fill_inverse_cmap(qtobj, histogram, R, G, B);

		// Now emit the colormap index for this cell
		*dst= (BYTE)(*cachep - 1);
		index_used_count[*dst]++;
		src++;
		dst++;
	}
}

static void
mediancut_pass2_fixed_dither(QuantizeObj *qtobj, mif* image, BYTE* idxdata)
{
	Histogram histogram = qtobj->histogram;
	ColorFreq * cachep;
	Color* color;
	int R, G, B;
	int re, ge, be;
	unsigned long* index_used_count = qtobj->index_used_count;

	zero_histogram(histogram);

	RGBA* src = image->pixels();
	BYTE* dst = idxdata;

	int size = image->width() * image->height();
	for (int i = 0; i < size; i++)
	{
		int dmval =
		DM[(i % image->width()) & DM_WIDTHMASK][(i / image->width()) & DM_HEIGHTMASK];

		// get pixel value and index into the cache
		R = (src->red) >> R_SHIFT;
		G = (src->green) >> G_SHIFT;
		B = (src->blue) >> B_SHIFT;
		cachep = &histogram[R * MR + G * MG + B];

		// If we have not seen this color before, find nearest
		// colormap entry and update the cache
		if (*cachep == 0)
			fill_inverse_cmap(qtobj, histogram, R, G, B);

		// Get the error and modulate it between 0x and 2x according
		// to the fixed dither matrix, then add it back to the 0x colour
		// and look up the new histogram entry.  To do better fixed
		// dithering, I believe that we need to be able to find the
		// closest colour match on the 'other side' of the desired colour,
		// which is not information which we have cheap access to.

		color = &qtobj->cmap[*cachep - 1];
		re = src->red - color->red;
		ge = src->green - color->green;
		be = src->blue - color->blue;

		re = (re * dmval * 2) / 63;
		ge = (ge * dmval * 2) / 63;
		be = (be * dmval * 2) / 63;

		R = (ADJRGB(color->red + re)) >> R_SHIFT;
		G = (ADJRGB(color->green + ge)) >> G_SHIFT;
		B = (ADJRGB(color->blue + be)) >> B_SHIFT;

		cachep = &histogram[R * MR + G * MG + B];

		// If we have not seen this color before, find nearest
		// colormap entry and update the cache
		if (*cachep == 0)
			fill_inverse_cmap(qtobj, histogram, R, G, B);

		// Now emit the colormap index for this cell, barfbarf
		*dst= (BYTE)(*cachep - 1);
		index_used_count[*dst]++;
		src++;
		dst++;
	}
}

static void
mediancut_pass2_nodestruct_dither(QuantizeObj *qtobj, mif* image, BYTE* idxdata)
{
	int lastindex = 0;
	int lastred = -1;
	int lastgreen = -1;
	int lastblue = -1;

	RGBA* src = image->pixels();
	BYTE* dst = idxdata;

	int size = image->width() * image->height();
	for (int i = 0; i < size; i++)
	{
		if (lastred != src->red || lastgreen != src->green || lastblue != src->blue)
		{
			for (int n = 0 ; n < qtobj->actual_number_of_colors; n++)
			{
				if ((qtobj->cmap[n].red == src->red) &&
					(qtobj->cmap[n].green == src->green) &&
					(qtobj->cmap[n].blue == src->blue))
				{
					lastred = src->red;
					lastgreen = src->green;
					lastblue = src->blue;
					lastindex = n;
					break;
				}
			}
		}
		*dst++ = lastindex;
		src++;
	}
}


/*
 * Initialize the error-limiting transfer function (lookup table).
 * The raw F-S error computation can potentially compute error values of up to
 * +- MAXJSAMPLE.  But we want the maximum correction applied to a pixel to be
 * much less, otherwise obviously wrong pixels will be created.  (Typical
 * effects include weird fringes at color-area boundaries, isolated bright
 * pixels in a dark area, etc.)  The standard advice for avoiding this problem
 * is to ensure that the "corners" of the color cube are allocated as output
 * colors; then repeated errors in the same direction cannot cause cascading
 * error buildup.  However, that only prevents the error from getting
 * completely out of hand; Aaron Giles reports that error limiting improves
 * the results even with corner colors allocated.
 * A simple clamping of the error values to about +- MAXJSAMPLE/8 works pretty
 * well, but the smoother transfer function used below is even better.  Thanks
 * to Aaron Giles for this idea.
 */

static int* init_error_limit(int error_freedom)
{
	int* table = (int*)malloc(sizeof(int) * (255 * 2 + 1));
	table += 255;				// so we can index -255 ... +255

	int in, out;
	if (error_freedom == 0)
	{
		// Coarse function, much bleeding.
		const int STEPSIZE = 190;
		for (in = 0; in < STEPSIZE; in++)
		{
			table[in] = in;
			table[-in] = -in;
		}
		for (; in <= 255; in++)
		{
			table[in] = STEPSIZE;
			table[-in] = -STEPSIZE;
		}
	}
	else
	{
		// Smooth function, bleeding more constrained
		const int STEPSIZE = 24;

		// Map errors 1:1 up to +- STEPSIZE
		out = 0;
		for (in = 0; in < STEPSIZE; in++, out++)
		{
			table[in] = out;
			table[-in] = -out;
		}

		// Map errors 1:2 up to +- 3*STEPSIZE
		for (; in < STEPSIZE*3; in++, out += (in&1) ? 0 : 1)
		{
			table[in] = out;
			table[-in] = -out;
		}

		// Clamp the rest to final out value (which is STEPSIZE*2)
		for (; in <= 255; in++)
		{
			table[in] = out;
			table[-in] = -out;
		}
	}
	return table;
}


/*
 * Map some rows of pixels to the output colormapped representation.
 * Perform floyd-steinberg dithering.
 */

static void mediancut_pass2_fs_dither(QuantizeObj *qtobj, mif* image, BYTE* idxdata)
{
	Histogram histogram = qtobj->histogram;
	ColorFreq *cachep;
	Color *color;
	int *error_limiter;
	short *fs_err1, *fs_err2;
	short *fs_err3, *fs_err4;
	short *range_limiter;
	int *red_n_row, *red_p_row;
	int *grn_n_row, *grn_p_row;
	int *blu_n_row, *blu_p_row;
	int *rnr, *rpr;
	int *gnr, *gpr;
	int *bnr, *bpr;
	int *tmp;
	int r, g, b;
	int re, ge, be;
	int row, col;
	int index;
	int step;
	int odd_row;
	unsigned long *index_used_count = qtobj->index_used_count;

	zero_histogram(histogram);

	int width = image->width();
	int height = image->height();

	error_limiter = init_error_limit(qtobj->error_freedom);
	range_limiter = range_array + 256;

	RGBA* src_buf = (RGBA*)malloc(width * sizeof(RGBA));
	BYTE* dest_buf = (BYTE*)malloc(width);
	red_n_row = (int*)malloc(sizeof(int) * (width + 2));
	red_p_row = (int*)malloc(sizeof(int) * (width + 2));
	grn_n_row = (int*)malloc(sizeof(int) * (width + 2));
	grn_p_row = (int*)malloc(sizeof(int) * (width + 2));
	blu_n_row = (int*)malloc(sizeof(int) * (width + 2));
	blu_p_row = (int*)malloc(sizeof(int) * (width + 2));

	memset(red_p_row, 0, (width + 2) * sizeof(int));
	memset(grn_p_row, 0, (width + 2) * sizeof(int));
	memset(blu_p_row, 0, (width + 2) * sizeof(int));

	fs_err1 = floyd_steinberg_error1 + 511;
	fs_err2 = floyd_steinberg_error2 + 511;
	fs_err3 = floyd_steinberg_error3 + 511;
	fs_err4 = floyd_steinberg_error4 + 511;

	odd_row = 0;

	RGBA* src;
	BYTE* dest;
	for (row = 0; row < height; row++)
    {
		image->getrow(src_buf, 0, row, width);

		src = src_buf;
		dest = dest_buf;

		rnr = red_n_row;
 		gnr = grn_n_row;
		bnr = blu_n_row;
		rpr = red_p_row + 1;
		gpr = grn_p_row + 1;
		bpr = blu_p_row + 1;

		if (odd_row) {
			step = -1;

			src += width - 1;
			dest += width - 1;

			rnr += width + 1;
			gnr += width + 1;
			bnr += width + 1;
			rpr += width;
			gpr += width;
			bpr += width;

			*(rnr - 1) = *(gnr - 1) = *(bnr - 1) = 0;
		}
		else {
			step = 1;

			*(rnr + 1) = *(gnr + 1) = *(bnr + 1) = 0;
		}

		*rnr = *gnr = *bnr = 0;

		for (col = 0; col < width; col++) {
			r = range_limiter[src->red + error_limiter[*rpr]];
			g = range_limiter[src->green + error_limiter[*gpr]];
			b = range_limiter[src->blue + error_limiter[*bpr]];

			re = r >> R_SHIFT;
			ge = g >> G_SHIFT;
			be = b >> B_SHIFT;

			cachep = &histogram[re*MR + ge*MG + be];

			// If we have not seen this color before, find nearest colormap entry
			// and update the cache
			if (*cachep == 0)
				fill_inverse_cmap(qtobj, histogram, re, ge, be);
			index = *cachep - 1;
			index_used_count[index]++;
			*dest = index;

			color = &qtobj->cmap[index];
			re = r - color->red;
			ge = g - color->green;
			be = b - color->blue;

			if (odd_row) {
				*(--rpr) += fs_err1[re];
				*(--gpr) += fs_err1[ge];
				*(--bpr) += fs_err1[be];

				*rnr-- += fs_err2[re];
				*gnr-- += fs_err2[ge];
				*bnr-- += fs_err2[be];

				*rnr += fs_err3[re];
				*gnr += fs_err3[ge];
				*bnr += fs_err3[be];

				*(rnr-1) = fs_err4[re];
				*(gnr-1) = fs_err4[ge];
				*(bnr-1) = fs_err4[be];
			}
			else {
				*(++rpr) += fs_err1[re];
				*(++gpr) += fs_err1[ge];
				*(++bpr) += fs_err1[be];

				*rnr++ += fs_err2[re];
				*gnr++ += fs_err2[ge];
				*bnr++ += fs_err2[be];

				*rnr += fs_err3[re];
				*gnr += fs_err3[ge];
				*bnr += fs_err3[be];

				*(rnr+1) = fs_err4[re];
				*(gnr+1) = fs_err4[ge];
				*(bnr+1) = fs_err4[be];
			}
			dest += step;
			src += step;
		}

		tmp = red_n_row;
		red_n_row = red_p_row;
		red_p_row = tmp;

		tmp = grn_n_row;
		grn_n_row = grn_p_row;
		grn_p_row = tmp;

		tmp = blu_n_row;
		blu_n_row = blu_p_row;
		blu_p_row = tmp;

		odd_row = !odd_row;

		memcpy(idxdata + row * width, dest_buf, width);
	}

	free(error_limiter - 255);
	free(red_n_row);
	free(red_p_row);
	free(grn_n_row);
	free(grn_p_row);
	free(blu_n_row);
	free(blu_p_row);
	free(src_buf);
	free(dest_buf);
}

static void delete_mediancut(QuantizeObj *qtobj)
{
	free(qtobj->histogram);
	free(qtobj);
}



static QuantizeObj* init_mediancut(int numcolors, int dither_type, int pal_type)
{
	// Initialize the data structures
	QuantizeObj* qtobj = (QuantizeObj*)malloc(sizeof(QuantizeObj));
	memset(qtobj, 0, sizeof(QuantizeObj));

	qtobj->histogram = (ColorFreq*)malloc(sizeof(ColorFreq) *
									HIST_R_ELEMS *
									HIST_G_ELEMS *
									HIST_B_ELEMS);

	qtobj->desired_number_of_colors = numcolors;

	switch (pal_type)
	{
		case paletteMake:
			qtobj->pass_1st = (PFNPASS1)mediancut_pass1;
			break;

		case paletteCustom:
			qtobj->pass_1st = (PFNPASS1)custompal_pass1;
			needs_quantize=TRUE;
			break;

		case paletteMono:
		default:
			qtobj->pass_1st = (PFNPASS1)monopal_pass1;
	}

	switch (dither_type)
	{
		case ditherNo:
			qtobj->pass_2nd = (PFNPASS2)mediancut_pass2_no_dither;
			break;

		case ditherFS:
			qtobj->error_freedom = 0;
			qtobj->pass_2nd = (PFNPASS2)mediancut_pass2_fs_dither;
			break;

		case ditherFSLowBleed:
			qtobj->error_freedom = 1;
			qtobj->pass_2nd = (PFNPASS2)mediancut_pass2_fs_dither;
			break;

		case ditherFixed:
			qtobj->pass_2nd = (PFNPASS2)mediancut_pass2_fixed_dither;
			break;

		case ditherNode:
			qtobj->pass_2nd = (PFNPASS2)mediancut_pass2_nodestruct_dither;
			break;
	}

	qtobj->delete_func = (PFNCLEANUP)delete_mediancut;
	return qtobj;
}

void convert_image(mif* image, int num_cols, int dither, int pal_type, INDEXEDIMAGE* idximage)
{
	QuantizeObj *qtobj = NULL;

	// Build histogram if necessary.
	qtobj = init_mediancut(num_cols, dither, pal_type);

	if (pal_type == paletteMake) {
		zero_histogram(qtobj->histogram);

		// To begin, assume that there are fewer colours in
		// the image than the user actually asked for.  In that
		//  case, we don't need to quantize or dither.
		needs_quantize = FALSE;
		num_found_cols = 0;

		// Build the histogram
		generate_histogram(qtobj->histogram, image, num_cols);
	}

	if (!needs_quantize && pal_type == paletteMake) {
		/* If this is an RGB image, and the user wanted a custom-built
		 *  generated palette, and this image has no more colours than
		 *  the user asked for, we don't need the first pass (quantization).
		 *
		 * There's also no point in dithering, since there's no error to
		 *  spread.  So we destroy the old qtobj and make a new one
		 *  with the remapping function set to a special LUT-based
		 *  no-dither remapper.
		 */

		qtobj->delete_func(qtobj);
		qtobj = init_mediancut(num_cols, ditherNode, pal_type);

		// We can skip the first pass (palette creation)
		qtobj->actual_number_of_colors = num_found_cols;
		for (int i = 0; i < num_found_cols; i++) {
			qtobj->cmap[i].red = found_cols[i].red;
			qtobj->cmap[i].green = found_cols[i].green;
			qtobj->cmap[i].blue = found_cols[i].blue;
		}
	}
	else
		(*qtobj->pass_1st)(qtobj);

	idximage->colors = qtobj->actual_number_of_colors;
	memcpy(idximage->cmap, qtobj->cmap, idximage->colors * sizeof(Color));
	(*qtobj->pass_2nd)(qtobj, image, idximage->data);

	// Delete the quantizer object, if there is one
	if (qtobj)
		qtobj->delete_func(qtobj);
}

void quantize(mif& image, int dither_type, int numcolors, LOGPALETTE* plgp)
{
	int idxdatasize = image.width() * image.height();
	INDEXEDIMAGE* idximage = (INDEXEDIMAGE*)malloc(sizeof(INDEXEDIMAGE) + idxdatasize);

	theCustomPalette = plgp;
	convert_image(&image,
					plgp ? plgp->palNumEntries : numcolors,
					dither_type,
					plgp ? paletteCustom : paletteMake,
					idximage);
	theCustomPalette = NULL;

	if (idximage->colors <= 2)
		image.m_bits = 1;
	else if (idximage->colors <= 16)
		image.m_bits = 4;
	else
		image.m_bits = 8;

	// カラーマップの設定
	image.alloccmap(idximage->colors);
	int i;
	for (i = 0; i < idximage->colors; i++) {
		image.m_map[i].red = idximage->cmap[i].red;
		image.m_map[i].green = idximage->cmap[i].green;
		image.m_map[i].blue = idximage->cmap[i].blue;
	}

	register RGBA* ps = image.pixels();
	register BYTE* pd = idximage->data;

	for (i = 0; i < idxdatasize; i++) {
		ps->alpha = *pd++;
		ps->red = image.m_map[ps->alpha].red;
		ps->green = image.m_map[ps->alpha].green;
		ps->blue = image.m_map[ps->alpha].blue;
		ps++;
	}
	free(idximage);
}


////////////////////////////////////////////////////////////////////////
//

HANDLE LSMIF_API mifQuantize(HANDLE hsrc, int dither_type, int numcolors)
{
	mif image;
	if (numcolors > 256 || !image.attachdib(hsrc))
		return NULL;

	image.quantize(dither_type, numcolors, NULL);
	return image.createdib();
}

HANDLE LSMIF_API mifQuantizeWcp(HANDLE hsrc, int dither_type, LOGPALETTE* plgp)
{
	mif image;
	if (!plgp || !image.attachdib(hsrc))
		return NULL;

	image.quantize(dither_type, 0, plgp);
	return image.createdib();
}

HANDLE LSMIF_API mifQuantizeWdp(HANDLE hsrc, int dither_type, int wdpal)
{
	mif image;
	if (!image.attachdib(hsrc))
		return NULL;

	image.quantize_wdp(dither_type, wdpal);
	return image.createdib();
}

HANDLE LSMIF_API mifQuantizeWin256(HANDLE hsrc, int dither_type)
{
	return mifQuantizeWdp(hsrc, dither_type, wdpalWin256);
}

HANDLE LSMIF_API mifQuantizeWin16(HANDLE hsrc, int dither_type)
{
	return mifQuantizeWdp(hsrc, dither_type, wdpalWin16);
}
