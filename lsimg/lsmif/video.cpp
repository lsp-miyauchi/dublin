/*
 *  VideoRGB plug-in v1.0 by Adam D. Moss, adam@foxbox.org.  1997/3/3
 */
////////////////////////////////////////////////////////////////////////
// video.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

#define MAX_PATTERNS 6
#define MAX_PATTERN_SIZE 108

const int pattern_width[MAX_PATTERNS] = {2,4,1,1,3,5};
const int pattern_height[MAX_PATTERNS] = {6,12,3,6,3,15};
/* @@@
const char *pattern_name[MAX_PATTERNS] = {
	"Staggered",
	"Large staggered",
	"Striped",
	"Wide-striped",
	"3x3",
	"Dots",
};
@@@ */
const int pattern[MAX_PATTERNS][MAX_PATTERN_SIZE] =
{
  { 0,1,
    0,2,
    1,2,
    1,0,
    2,0,
    2,1,
  },
  {
    0,0,1,1,
    0,0,1,1,
    0,0,2,2,
    0,0,2,2,
    1,1,2,2,
    1,1,2,2,
    1,1,0,0,
    1,1,0,0,
    2,2,0,0,
    2,2,0,0,
    2,2,1,1,
    2,2,1,1,
  },
  {
    0,
    1,
    2,
  },
  {
    0,
    0,
    1,
    1,
    2,
    2,
  },
  {
    0,1,2,
    2,0,1,
    1,2,0,
  },
  {
    0,1,2,0,0,
    1,1,1,2,0,
    0,1,2,2,2,
    0,0,1,2,0,
    0,1,1,1,2,
    2,0,1,2,2,
    0,0,0,1,2,
    2,0,1,1,1,
    2,2,0,1,2,
    2,0,0,0,1,
    1,2,0,1,1,
    2,2,2,0,1,
    1,2,0,0,0,
    1,1,2,0,1,
    1,2,2,2,0,
  }
};

static void video_render_row(RGBA *src_row, RGBA *dest_row,
		  int row,
		  int rotated,
		  int additive,
		  int pattern_number,
		  int row_width)
{
	int col, n;

	register RGBA* ps = src_row;
	register RGBA* pd = dest_row;
	for (col = 0; col < row_width ; col++) {
		pd->red = 0;
		pd->green = 0;
		pd->blue = 0;
		pd->alpha = ps->alpha;
		if (rotated) {
			n = pattern[pattern_number][pattern_width[pattern_number] *
					(col % pattern_height[pattern_number]) +
					row % pattern_width[pattern_number]];
			if (n == 0)
				pd->red = ps->red;
			else if (n == 1)
				pd->green = ps->green;
			else
				pd->blue = ps->blue;
		} else {
			n = pattern[pattern_number][pattern_width[pattern_number] *
					(row%pattern_height[pattern_number]) +
					col%pattern_width[pattern_number]];
			if (n == 0)
				pd->red = ps->red;
			else if (n == 1)
				pd->green = ps->green;
			else
				pd->blue = ps->blue;
		}
		ps++;
		pd++;
	}

	if (additive) {
		ps = src_row;
		pd = dest_row;
		for (col = 0; col < row_width ; col++) {
			pd->red = ((int)((int)pd->red + (int)ps->red)) < 255 ?
							((int)((int)pd->red + (int)ps->red)) : 255;
			pd->green = ((int)((int)pd->green + (int)ps->green)) < 255 ?
							((int)((int)pd->green + (int)ps->green)) : 255;
			pd->blue = ((int)((int)pd->blue + (int)ps->blue)) < 255 ?
							((int)((int)pd->blue + (int)ps->blue)) : 255;
			ps++;
			pd++;
		}
	}
}

BOOL video(mif& src, LPRECT prc, MIF_VIDEO* pvp)
{
	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;
	src.m_bits = 24;

	int width = rc.Width();

	MIF_VIDEO v;
	v.pattern_number = 2;
	v.additive = TRUE;
	v.rotated = FALSE;
	if (pvp)
		memcpy(&v, pvp, sizeof(v));

	RGBA* src_row = (RGBA*)malloc(width * sizeof(RGBA));
	RGBA* dst_row = (RGBA*)malloc(width * sizeof(RGBA));
	for (int y = rc.top; y < rc.bottom; y++)
    {
		src.getrow(src_row, rc.left, y, width);
		video_render_row(src_row, dst_row, y, v.rotated, v.additive, v.pattern_number, width);
		src.setrow(dst_row, rc.left, y, width);
	}
	free(src_row);
	free(dst_row);
	return TRUE;
}
