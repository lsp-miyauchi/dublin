/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"
#include <glib-object.h>
#include "libgimpmath/gimpmath.h"
#include "base-types.h"
#include "gimphistogram.h"
#include "levels.h"

#define return_if_fail(a)		(if ((a)) return)


/*  public functions  */

void levels_init(Levels *levels)
{
	GimpHistogramChannel channel;
	return_if_fail(levels != NULL);
	for (channel = GIMP_HISTOGRAM_VALUE; channel <= GIMP_HISTOGRAM_ALPHA; channel++) {
		levels_channel_reset (levels, channel);
	}
}

void levels_channel_reset(Levels* levels, GimpHistogramChannel channel)
{
	return_if_fail(levels != NULL);
	levels->gamma[channel]       = 1.0;
	levels->low_input[channel]   = 0;
	levels->high_input[channel]  = 255;
	levels->low_output[channel]  = 0;
	levels->high_output[channel] = 255;
}

void levels_auto(Levels *levels, GimpHistogram *hist, bool is_color)
{
	GimpHistogramChannel channel;
	return_if_fail (levels != NULL);
	return_if_fail (hist != NULL);

	if (is_color) {
		/*  Set the overall value to defaults  */
		levels_channel_reset (levels, GIMP_HISTOGRAM_VALUE);

		for (channel = GIMP_HISTOGRAM_RED; channel <= GIMP_HISTOGRAM_BLUE; channel++)
			levels_channel_auto (levels, hist, channel);
	}
	else {
		levels_channel_auto(levels, hist, GIMP_HISTOGRAM_VALUE);
	}
}

void levels_channel_auto(Levels *levels, GimpHistogram *hist, GimpHistogramChannel channel)
{
	int    i;
	double count, new_count, percentage, next_percentage;

	return_if_fail (levels != NULL);
	return_if_fail (hist != NULL);

	levels->gamma[channel]       = 1.0;
	levels->low_output[channel]  = 0;
	levels->high_output[channel] = 255;

	count = gimp_histogram_get_count (hist, channel, 0, 255);

	if (count == 0.0) {
		levels->low_input[channel]  = 0;
		levels->high_input[channel] = 0;
	}
	else {
		/*  Set the low input  */
		new_count = 0.0;

		for (i = 0; i < 255; i++) {
			new_count += gimp_histogram_get_value (hist, channel, i);
			percentage = new_count / count;
			next_percentage = (new_count + gimp_histogram_get_value (hist, channel, i + 1)) / count;
			if (fabs (percentage - 0.006) < fabs (next_percentage - 0.006)) {
				levels->low_input[channel] = i + 1;
				break;
			}
		}
		/*  Set the high input  */
		new_count = 0.0;
		for (i = 255; i > 0; i--) {
			new_count += gimp_histogram_get_value (hist, channel, i);
			percentage = new_count / count;
			next_percentage = (new_count + gimp_histogram_get_value (hist, channel, i - 1)) / count;
			if (fabs (percentage - 0.006) < fabs (next_percentage - 0.006)) {
				levels->high_input[channel] = i - 1;
				break;
			}
		}
	}
}

static int levels_input_from_color(GimpHistogramChannel  channel, BYTE *color)
{
	switch (channel) {
	case GIMP_HISTOGRAM_VALUE: return MAX (MAX (color[RED_PIX], color[GREEN_PIX]), color[BLUE_PIX]);
	case GIMP_HISTOGRAM_RED: return color[RED_PIX];
	case GIMP_HISTOGRAM_GREEN: return color[GREEN_PIX];
	case GIMP_HISTOGRAM_BLUE: return color[BLUE_PIX];
	case GIMP_HISTOGRAM_ALPHA: return color[ALPHA_PIX];
	}
	return 0;		/* just to please the compiler */
}

void levels_adjust_by_colors(Levels *levels, GimpHistogramChannel channel, BYTE *black, BYTE *gray, BYTE *white)
{
	return_if_fail (levels != NULL);
	if (black) levels->low_input[channel] = levels_input_from_color (channel, black);
	if (white) levels->high_input[channel] = levels_input_from_color (channel, white);

	if (gray) {
		int    input;
		int    range;
		double inten;
		input = levels_input_from_color (channel, gray);
		range = levels->high_input[channel] - levels->low_input[channel];
		if (range <= 0) return;

		input -= levels->low_input[channel];
		if (input < 0) return;

		inten = (double)input / (double)range;
		levels->gamma[channel] = log (inten) / log (0.5);
	}
}

void levels_calculate_transfers(Levels *levels)
{
	double inten;
	int    i, j;

	return_if_fail (levels != NULL);

	/*  Recalculate the levels arrays  */
	for (j = 0; j < 5; j++) {
		for (i = 0; i < 256; i++) {
			/*  determine input intensity  */
			if (levels->high_input[j] != levels->low_input[j]) {
				inten = ((double)(i - levels->low_input[j]) / (double)(levels->high_input[j] - levels->low_input[j]));
			}
			else {
				inten = (double)(i - levels->low_input[j]);
			}

			inten = CLAMP (inten, 0.0, 1.0);
			if (levels->gamma[j] != 0.0)
				inten = pow (inten, (1.0 / levels->gamma[j]));
			levels->input[j][i] = (BYTE)(inten * 255.0 + 0.5);
		}
	}
}

gfloat levels_lut_func (Levels *levels, int n_channels, int channel, gfloat value)
{
	double inten;
	int    j;

	if (n_channels == 1) j = 0;
	else j = channel + 1;

	inten = value;

	/* For color  images this runs through the loop with j = channel +1
	 * the first time and j = 0 the second time
	 *
	 * For bw images this runs through the loop with j = 0 the first and
	 *  only time
	 */
	for (; j >= 0; j -= (channel + 1)) {
		/* don't apply the overall curve to the alpha channel */
		if (j == 0 && (n_channels == 2 || n_channels == 4) && channel == n_channels -1)
			return inten;

		/*  determine input intensity  */
		if (levels->high_input[j] != levels->low_input[j])
			inten = ((double)(255.0 * inten - levels->low_input[j]) / (double)(levels->high_input[j] - levels->low_input[j]));
		else
			inten = (double)(255.0 * inten - levels->low_input[j]);

		if (levels->gamma[j] != 0.0) {
			if (inten >= 0.0)
				inten =  pow(inten, (1.0 / levels->gamma[j]));
			else
				inten = -pow(-inten, (1.0 / levels->gamma[j]));
		}

		/*  determine the output intensity  */
		if (levels->high_output[j] >= levels->low_output[j])
			inten = (double)(inten * (levels->high_output[j] - levels->low_output[j]) + levels->low_output[j]);
		else if (levels->high_output[j] < levels->low_output[j])
			inten = (double)(levels->low_output[j] - inten * (levels->low_output[j] - levels->high_output[j]));

		inten /= 255.0;
	}
	return inten;
}
