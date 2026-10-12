///////////////////////////////////////////////////////////////////////
// normalize.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

#define MAXRGB			255

BOOL normalize(mif& m, LPRECT prc)
{
	int histogram[MAXRGB + 1], threshold_intensity;
	BYTE gray_value, normalize_map[MAXRGB + 1];
	UINT high, low;

	CRect rc;
	if (!m.validaterect(prc, &rc))
		return FALSE;

	int i;
	for (i = 0; i <= MAXRGB; i++)
		histogram[i] = 0;

	RGBA* p = m.m_pixels;
	for (i = 0; i < m.m_packets; i++) {
		gray_value = ((int)p->red * 77 + (int)p->green * 150 + (int)p->blue * 29) >> 8;
		histogram[gray_value]++;
		p++;
	}

	// Find the histogram boundaries by locating the 1 percent levels.
	threshold_intensity = m.m_packets / 100;
	int intensity = 0;
	for (low = 0; low < MAXRGB; low++) {
		intensity += histogram[low];
		if (intensity > threshold_intensity)
			break;
	}

	intensity = 0;
	for (high = MAXRGB; high != 0; high--) {
		intensity += histogram[high];
		if (intensity > threshold_intensity)
			break;
	}

	if (low == high) {
		// Unreasonable contrast;  use zero threshold to determine boundaries.
		threshold_intensity = 0;
		intensity = 0;
		for (low = 0; low < MAXRGB; low++) {
			intensity += histogram[low];
			if (intensity > threshold_intensity)
				break;
		}

		intensity = 0;
		for (high = MAXRGB; high != 0; high--) {
			intensity += histogram[high];
			if (intensity > threshold_intensity)
				break;
		}
		if (low == high) {
			m.destroy();
			return NULL;				// zero span bound
		}
	}

	// Stretch the histogram to create the normalized image mapping.
	for (i = 0; i <= MAXRGB; i++) {
		if (i < (int)low)
			normalize_map[i] = 0;
		else if (i > (int)high)
			normalize_map[i] = MAXRGB;
		else
			normalize_map[i] = (MAXRGB - 1) * (i - low) / (high - low);
	}

	// Normalize the image.
	if (m.m_bits == 24) {
		parallel_for (rc.top, rc.bottom, [&](int y) {
			RGBA* p = m.pos(rc.left, y);
			for (int x = rc.left; x < rc.right; x++) {
				p->red = normalize_map[p->red];
				p->green = normalize_map[p->green];
				p->blue = normalize_map[p->blue];
				p++;
			}
		});
	}
	else {
		for (i = 0; i < m.m_colors; i++) {
			m.m_map[i].red = normalize_map[m.m_map[i].red];
			m.m_map[i].green = normalize_map[m.m_map[i].green];
			m.m_map[i].blue = normalize_map[m.m_map[i].blue];
		}
	}
	return TRUE;
}
