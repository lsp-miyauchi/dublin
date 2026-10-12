///////////////////////////////////////////////////////////////////////
// auto_contrast.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

static void do_autocontrast(RGBA* data, int packets)
{
	RGBA vmin, vmax;
	vmin.red = vmin.green = vmin.blue = 255;
	vmax.red = vmax.green = vmax.blue = 0;

	register RGBA* p = data;
	int i;
	for (i = 0; i < packets; i++) {
		vmin.red = min(p->red, vmin.red);
		vmax.red = max(p->red, vmax.red);
		vmin.green = min(p->green, vmin.green);
		vmax.green = max(p->green, vmax.green);
		vmin.blue = min(p->blue, vmin.blue);
		vmax.blue = max(p->blue, vmax.blue);
		p++;
	}

	// Calculate LUTs with stretched contrast
	RGBA lut[256];
	BYTE range;
	range = vmax.red - vmin.red;
	if (range != 0)
		for (i = vmin.red; i <= vmax.red; i++)
			lut[i].red = 255 * (i - vmin.red) / range;
	else
		lut[vmin.red].red = vmin.red;

	range = vmax.green - vmin.green;
	if (range != 0)
		for (i = vmin.green; i <= vmax.green; i++)
			lut[i].green = 255 * (i - vmin.green) / range;
	else
		lut[vmin.green].green = vmin.green;

	range = vmax.blue - vmin.blue;
	if (range != 0)
		for (i = vmin.blue; i <= vmax.blue; i++)
			lut[i].blue = 255 * (i - vmin.blue) / range;
	else
		lut[vmin.blue].blue = vmin.blue;

	p = data;
	for (i = 0; i < packets; i++) {
		p->red = lut[p->red].red;
		p->green = lut[p->green].green;
		p->blue = lut[p->blue].blue;
		p++;
	}
}

BOOL autocontrast(mif& image, LPRECT prc)
{
	CRect rc;
	if (!image.validaterect(prc, &rc))
		return FALSE;

	if (image.m_bits <= 8)
	{
		do_autocontrast(image.m_map, image.m_colors);
		image.validate_rgb();
	}
	else
	{
		int packets = rc.Width() * rc.Height();
		RGBA* data = (RGBA*)malloc(packets * sizeof(RGBA));
		image.getpacket(data, rc.left, rc.top, rc.Width(), rc.Height());
		do_autocontrast(data, packets);
		image.setpacket(data, rc.left, rc.top, rc.Width(), rc.Height());
		free(data);
	}
	return TRUE;
}
