////////////////////////////////////////////////////////////////////////
// brightcontrast.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

static double calc_params(double brightness, double contrast, double value)
{
	double nvalue;
	double power;

	// apply brightness
	if (brightness < 0.0)
		value = value * (1.0 + brightness);
	else
		value = value + ((1.0 - value) * brightness);

	// apply contrast
	if (contrast < 0.0)
	{
		if (value > 0.5)
			nvalue = 1.0 - value;
 		else
			nvalue = value;
		if (nvalue < 0.0)
			nvalue = 0.0;
		nvalue = 0.5 * pow (nvalue * 2.0 , (double)(1.0 + contrast));
		if (value > 0.5)
			value = 1.0 - nvalue;
		else
			value = nvalue;
	}
	else
	{
		if (value > 0.5)
			nvalue = 1.0 - value;
		else
			nvalue = value;
		if (nvalue < 0.0)
			nvalue = 0.0;
		power = (contrast == 1.0) ? 127 : 1.0 / (1.0 - contrast);
		nvalue = 0.5 * pow (2.0 * nvalue, power);
		if (value > 0.5)
			value = 1.0 - nvalue;
		else
			value = nvalue;
	}
	return value;
}

static void lut_process(BYTE* lut, RGBA* pdata, int size)
{
	register RGBA* p = pdata;
	while (size--)
	{
		p->red = lut[p->red];
		p->green = lut[p->green];
		p->blue = lut[p->blue];
		p++;
	}
}

static void lut_setup(BYTE* lut, double brightness, double contrast)
{
	for (int v = 0; v < 256; v++)
	{
		// to add gamma correction use func(v ^ g) ^ 1/g instead.
		double val = 255.0 * calc_params(brightness, contrast, v / 255.0) + 0.5;
		if (val < 0.0)
			lut[v] = 0;
		else if (val >= 255.0)
			lut[v] = 255;
		else
			lut[v] = (BYTE)val;
	}
}

static void brct(RGBA* pdata, int size, int brightness, int contrast)
{
	brightness = min(127, max(brightness, -127));
	contrast = min(127, max(contrast, -127));

	BYTE lut[256];
	lut_setup(lut, brightness / 255.0, contrast / 127.0);
	lut_process(lut, pdata, size);
}

BOOL bright_contrast(mif& src, LPRECT prc, int brightness, int contrast)
{
	if (src.m_bits < 24) {
		brct(src.m_map, src.m_colors, brightness, contrast);
		src.validate_rgb();
	}
	else {
		CRect rc;
		if (!src.validaterect(prc, &rc))
			return FALSE;

		int packets = rc.Width() * rc.Height();
		RGBA* data = (RGBA*)malloc(packets * sizeof(RGBA));
		src.getpacket(data, rc.left, rc.top, rc.Width(), rc.Height());
		brct(data, packets, brightness, contrast);
		src.setpacket(data, rc.left, rc.top, rc.Width(), rc.Height());
		free(data);
	}
	return TRUE;
}
