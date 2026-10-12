////////////////////////////////////////////////////////////
// xform_color.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"


static int __fastcall hls_value(double n1, double n2, double hue)
{
  double value;

  if (hue > 255)
    hue -= 255;
  else if (hue < 0)
    hue += 255;
  if (hue < 42.5)
    value = n1 + (n2 - n1) * (hue / 42.5);
  else if (hue < 127.5)
    value = n2;
  else if (hue < 170)
    value = n1 + (n2 - n1) * ((170 - hue) / 42.5);
  else
    value = n1;

  return (int)(value * 255);
}

void __fastcall rgb_to_hls(int *r, int *g, int *b)
{
	int red, green, blue;
	double h, l, s;
	int min, max;
	int delta;

	red = *r;
	green = *g;
	blue = *b;

	if (red > green)
	{
		if (red > blue)
			max = red;
		else
			max = blue;

		if (green < blue)
			min = green;
		else
			min = blue;
	}
	else
	{
		if (green > blue)
			max = green;
		else
			max = blue;

		if (red < blue)
			min = red;
		else
			min = blue;
	}
	l = (max + min) / 2.0;

	if (max == min)
	{
		s = 0.0;
		h = 0.0;
	}
	else
	{
		delta = (max - min);

		if (l < 128)
			s = 255 * (double)delta / (double)(max + min);
		else
			s = 255 * (double)delta / (double)(511 - max - min);

		if (red == max)
			h = (green - blue) / (double) delta;
		else if (green == max)
			h = 2 + (blue - red) / (double) delta;
		else
			h = 4 + (red - green) / (double) delta;

		h = h * 42.5;
		if (h < 0)
			h += 255;
		else if (h > 255)
			h -= 255;
	}
	*r = (int)h;
	*g = (int)l;
	*b = (int)s;
}

void __fastcall hls_to_rgb (int *h, int *l, int *s)
{
  double hue, lightness, saturation;
  double m1, m2;

  hue = *h;
  lightness = *l;
  saturation = *s;

  if (saturation == 0)
    {
      /*  achromatic case  */
      *h = (int)lightness;
      *l = (int)lightness;
      *s = (int)lightness;
    }
  else
    {
      if (lightness < 128)
	m2 = (lightness * (255 + saturation)) / 65025.0;
      else
	m2 = (lightness + saturation - (lightness * saturation)/255.0) / 255.0;

      m1 = (lightness / 127.5) - m2;

      /*  chromatic case  */
      *h = hls_value (m1, m2, hue + 85);
      *l = hls_value (m1, m2, hue);
      *s = hls_value (m1, m2, hue - 85);
    }
}



void __fastcall rgb_to_hsv(BYTE r, BYTE g, BYTE b, double* hue, double* sat, double* val)
{
	double red   = (double)r / 255.0;
	double green = (double)g / 255.0;
	double blue  = (double)b / 255.0;

	double vmin, vmax;
	double h = 0.0;		/* Shut up -Wall */
	if (red > green)
	{
		vmax = MAX (red,   blue);
		vmin = MIN (green, blue);
	}
	else
	{
		vmax = MAX (green, blue);
		vmin = MIN (red,   blue);
	}

	double s, v = vmax;
	if (vmax != 0.0)
		s = (vmax - vmin) / vmax;
	else
		s = 0.0;

	if (s == 0.0)
		h = 0.0;
	else
	{
		double delta = vmax - vmin;
		if (delta == 0.0)
			delta = 1.0;

		if (red == vmax)
			h = (green - blue) / delta;
		else if (green == vmax)
			h = 2 + (blue - red) / delta;
		else if (blue == vmax)
			h = 4 + (red - green) / delta;

		h /= 6.0;
		if (h < 0.0)
			h += 1.0;
		else if (h > 1.0)
			h -= 1.0;
	}

	*hue = h;
	*sat = s;
	*val = v;
}

void __fastcall hsv_to_rgb(BYTE* pr, BYTE* pb, BYTE* pg, double hue, double sat, double val)
{
	if (sat == 0.0)
	{
		hue = val;
		sat = val;
		val = val;
	}
	else
	{
		double h = hue * 6.0;
		double s = sat;
		double v = val;
		if (h == 6.0)
			h = 0.0;

		double f = h - (int)h;
		double p = v * (1.0 - s);
		double q = v * (1.0 - s * f);
		double t = v * (1.0 - s * (1.0 - f));

		switch ((int)h)
		{
			case 0:
				hue = v;
				sat = t;
				val = p;
				break;

			case 1:
				hue = q;
				sat = v;
				val = p;
				break;

			case 2:
				hue = p;
				sat = v;
				val = t;
				break;

			case 3:
				hue = p;
				sat = q;
				val = v;
				break;

			case 4:
				hue = t;
				sat = p;
				val = v;
				break;

			case 5:
				hue = v;
				sat = p;
				val = q;
				break;
		}
	}

	*pr = ADJRGB((int)(hue * 255));
	*pb = ADJRGB((int)(sat * 255));
	*pg = ADJRGB((int)(val * 255));
}
