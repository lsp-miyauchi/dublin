///////////////////////////////////////////////////////////////////////
// equalize.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"


///////////////////////////////////////////////////////////////////////
//

#define MAXRGB		255

BOOL equalize(mif& m, LPRECT prc)
{
	register int i, j;
	register RGBA* p;
	UINT histogram[MAXRGB + 1], map[MAXRGB + 1], high, low;
	BYTE equalize_map[MAXRGB + 1];

	CRect rc;
	if (!m.validaterect(prc, &rc))
		return FALSE;

	for (i = 0; i <= MAXRGB; i++)
		histogram[i] = 0;

	p = m.pos(0, 0);
	for (i = 0; i < m.m_packets; i++)
	{
		j = ((int)p->red * 77 + (int)p->green * 150 + (int)p->blue * 29) >> 8;
		histogram[j]++;
		p++;
	}

	// Integrate the histogram to get the equalization map.
	j = 0;
	for (i = 0; i <= MAXRGB; i++)
	{
		j += histogram[i];
		map[i] = j;
	}

	if (map[MAXRGB] == 0)
		return NULL;

	// Equalize.
	low = map[0];
	high = map[MAXRGB];
	for (i = 0; i <= MAXRGB; i++)
		equalize_map[i] = (BYTE)(((map[i] - low) * MAXRGB) / max(high - low, 1));

	// Stretch the histogram.
	if (m.m_bits < 24)
	{
		for (i = 0; i < m.m_colors; i++)
		{
			m.m_map[i].red = equalize_map[m.m_map[i].red];
			m.m_map[i].green = equalize_map[m.m_map[i].green];
			m.m_map[i].blue = equalize_map[m.m_map[i].blue];
		}
		m.validate_rgb();
	}
	else
	{
		for (i = rc.top; i < rc.bottom; i++)
		{
			p = m.pos(rc.left, i);
			for (j = rc.left; j < rc.right; j++)
			{
				p->red = equalize_map[p->red];
				p->green = equalize_map[p->green];
				p->blue = equalize_map[p->blue];
				p++;
			}
		}
	}
	return TRUE;
}
