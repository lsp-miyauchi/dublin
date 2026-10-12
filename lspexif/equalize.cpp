////////////////////////////////////////////////////////////////////////
// equalize.cpp

#include "stdafx.h"

#define MAXRGB		255

void equalize(int width, int height, BYTE* rgbptr)
{
	register int i, j;
	UINT histogram[MAXRGB + 1], map[MAXRGB + 1];
	BYTE equalize_map[MAXRGB + 1];

	for (i = 0; i <= MAXRGB; i++)
		histogram[i] = 0;

	int total_pixel = width * height;
	BYTE* p = rgbptr;
	for (i = 0; i < total_pixel; i++) {
		j = *p++ * 77;			// red
		j += *p++ * 150;		// green
		j += *p++ * 29;			// brue
		j >>= 8;
		histogram[j]++;
	}

	// Integrate the histogram to get the equalization map.
	j = 0;
	for (i = 0; i <= MAXRGB; i++) {
		j += histogram[i];
		map[i] = j;
	}

	if (map[MAXRGB] == 0) return;

	// Equalize.
	UINT low = map[0];
	UINT high = map[MAXRGB];
	for (i = 0; i <= MAXRGB; i++)
		equalize_map[i] = (BYTE)(((map[i] - low) * MAXRGB) / max(high - low, 1));

	// Stretch the histogram.
	p = rgbptr;
	for (i = 0; i < total_pixel; i++) {
		*p = equalize_map[*p]; p++;
		*p = equalize_map[*p]; p++;
		*p = equalize_map[*p]; p++;
	}
}
