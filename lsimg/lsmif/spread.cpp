///////////////////////////////////////////////////////////////////////
// spread.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"
#include <stdlib.h>

BOOL spread(mif& src, LPRECT prc, int amount)
{
	CRect rc;
	if (!src.validaterect(prc, &rc) || amount < 1)
		return FALSE;

	double x_amount = amount;
	double y_amount = amount;

	// Initialize random stuff
	srand(0);
	int angle_mod_value = (int)(M_PI * 2);
	int angle_sub_value = angle_mod_value / 2;
	int x_mod_value = (int)(x_amount + 1);
	int x_sub_value = x_mod_value / 2;
	int y_mod_value = (int)(y_amount + 1);
	int y_sub_value = y_mod_value / 2;

	mif dst(src);
	parallel_for (rc.top, rc.bottom, [&](int y) {
		RGBA* p = src.pos(rc.left, y);
		for (int x = rc.left; x < rc.right; x++) {
			// get random angle, x distance, and y distance
			int xdist = (rand () % x_mod_value) - x_sub_value;
			int ydist = (rand () % y_mod_value) - y_sub_value;
			double angle = (rand () % angle_mod_value) - angle_sub_value;

			int xi = (int)(x + floor(sin(angle) * xdist));
			int yi = (int)(y + floor(cos(angle) * ydist));

			// Only displace the pixel if it's within the bounds of the image.
			RGBA* q;
			if (xi < 0)
				xi *= -1;
			if (xi >= src.m_width)
				xi = (src.m_width << 1) - xi;
			if (yi < 0)
				yi *= -1;
			if (yi >= src.m_height)
				yi = (src.m_height << 1) - yi;

			if (xi >= 0 && xi < src.m_width && yi >= 0 && yi < src.m_height) {
				q = dst.pos(xi, yi);
				*p = *q;
			}
			p++;
		}
	});
	return TRUE;
}
