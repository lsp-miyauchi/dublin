////////////////////////////////////////////////////////////
// xform_color.cpp

#include "stdafx.h"
#include "lsimg.h"
#include "common.h"

static int __fastcall hls_value(float n1, float n2, float hue)
{
  float value;

  if (hue > 255)
    hue -= 255;
  else if (hue < 0)
    hue += 255;
  if (hue < 42.5f)
    value = n1 + (n2 - n1) * (hue / 42.5f);
  else if (hue < 127.5)
    value = n2;
  else if (hue < 170)
    value = n1 + (n2 - n1) * ((170 - hue) / 42.5f);
  else
    value = n1;

  return (int)(value * 255);
}

void __fastcall rgb_to_hls(int *r, int *g, int *b)
{
	int red, green, blue;
	float h, l, s;
	int min, max;
	int delta;

	red = *r;
	green = *g;
	blue = *b;

	if (red > green) {
		if (red > blue)
			max = red;
		else
			max = blue;

		if (green < blue)
			min = green;
		else
			min = blue;
	}
	else {
		if (green > blue)
			max = green;
		else
			max = blue;

		if (red < blue)
			min = red;
		else
			min = blue;
	}
	l = (max + min) / 2.0f;

	if (max == min) {
		s = 0.0;
		h = 0.0;
	}
	else {
		delta = (max - min);

		if (l < 128)
			s = 255 * (float)delta / (float)(max + min);
		else
			s = 255 * (float)delta / (float)(511 - max - min);

		if (red == max)
			h = (green - blue) / (float) delta;
		else if (green == max)
			h = 2 + (blue - red) / (float) delta;
		else
			h = 4 + (red - green) / (float) delta;

		h = h * 42.5f;
		if (h < 0)
			h += 255;
		else if (h > 255)
			h -= 255;
	}
	*r = (int)floorf(h);
	*g = (int)floorf(l);
	*b = (int)floorf(s);
}

void __fastcall hls_to_rgb (int *h, int *l, int *s)
{
	float hue, lightness, saturation;
	float m1, m2;

	hue = (float)*h;
	lightness = (float)*l;
	saturation = (float)*s;

	if (saturation == 0) {
		/*  achromatic case  */
		*h = (int)floorf(lightness);
		*l = (int)floorf(lightness);
		*s = (int)floorf(lightness);
	}
	else {
		if (lightness < 128)
			m2 = (lightness * (255 + saturation)) / 65025.0f;
		else
			m2 = (lightness + saturation - (lightness * saturation)/255.0f) / 255.0f;

		m1 = (lightness / 127.5f) - m2;

		/*  chromatic case  */
		*h = hls_value (m1, m2, hue + 85);
		*l = hls_value (m1, m2, hue);
		*s = hls_value (m1, m2, hue - 85);
	}
}

void __fastcall rgb_to_hsv(int *r, int *g, int *b)
{
	int red, green, blue;
	double h, s, v, delta;
	int dmin, dmax;

	h = 0.0;

	red = *r;
	green = *g;
	blue = *b;

	if (red > green)
	{
		dmax = max(red, blue);
		dmin = min(green, blue);
	}
	else
    {
		dmax = max(green, blue);
		dmin = min(red, blue);
	}
	v = (double)dmax;

	if (dmax != 0)
		s = (double)((dmax - dmin) * 255) / (double)dmax;
	else
		s = 0;

	if (s == 0)
		h = 0;
	else
	{
		delta = (double)(dmax - dmin);
		if (red == dmax)
			h = (green - blue) / delta;
		else if (green == dmax)
			h = 2 + (blue - red) / delta;
		else if (blue == dmax)
			h = 4 + (red - green) / delta;
		h *= 42.5;

		if (h < 0)
			h += 255;
		if (h > 255)
			h -= 255;
	}

	*r = (int)h;
	*g = (int)s;
	*b = (int)v;
}

void __fastcall hsv_to_rgb(int *h, int *s, int *v)
{
	double hue, sat, val;
	double f, p, q, t;

	if (*s == 0)
	{
		*h = *v;
		*s = *v;
		*v = *v;
	}
	else
	{
		hue = *h * 6.0 / 255.0;
		sat = *s / 255.0;
		val = *v / 255.0;

 		f = hue - (int)hue;
		p = val * (1.0 - sat);
		q = val * (1.0 - (sat * f));
		t = val * (1.0 - (sat * (1.0 - f)));

		switch ((int)hue)
		{
			case 0:
				*h = (int)(val * 255);
				*s = (int)(t * 255);
				*v = (int)(p * 255);
				break;

			case 1:
				*h = (int)(q * 255);
				*s = (int)(val * 255);
				*v = (int)(p * 255);
				break;

			case 2:
				*h = (int)(p * 255);
				*s = (int)(val * 255);
				*v = (int)(t * 255);
				break;

			case 3:
				*h = (int)(p * 255);
				*s = (int)(q * 255);
				*v = (int)(val * 255);
				break;

			case 4:
				*h = (int)(t * 255);
				*s = (int)(p * 255);
				*v = (int)(val * 255);
				break;

			case 5:
				*h = (int)(val * 255);
				*s = (int)(p * 255);
				*v = (int)(q * 255);
				break;
		}
	}
}
