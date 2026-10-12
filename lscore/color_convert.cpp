
/////////////////////////////////////////////////////////////////////////
// INTERNAL: Map from HLS color space to RGB
//
static WORD __convert_hue(int wHue, WORD wMid1, WORD wMid2)
{
	wHue = wHue > 240 ? wHue - 240 : wHue < 0 ? wHue + 240 : wHue;

	if (wHue > 160)
		return wMid1;
	else if (wHue > 120)
		wHue = 160 - wHue;
	else if (wHue > 40)
		return wMid2;

	return ((wHue * (wMid2 - wMid1) + 20) / 40) + wMid1;
}

/////////////////////////////////////////////////////////////////////////
// Convert to RGB and scale into RGB range (0..255)
//
#define GET_RGB(h)			((__convert_hue(h, wMid1, wMid2) * 255 + 120) / 240)


/////////////////////////////////////////////////////////////////////////
//      color_hls2rgb
//
// Convert from hls color space into an rgb COLORREF.
//
// PARAMS
//  wHue        [I] Hue amount
//  wLuminosity [I] Luminosity amount
//  wSaturation [I] Saturation amount
//
// RETURNS
//  A COLORREF representing the converted color.
//
// NOTES
//  Input hls values are constrained to the range (0..240).
//
COLORREF color_hls2rgb(WORD wHue, WORD wLuminosity, WORD wSaturation)
{
	WORD wRed;

	if (wSaturation) {
		WORD wGreen, wBlue, wMid1, wMid2;

		if (wLuminosity > 120)
			wMid2 = wSaturation + wLuminosity - (wSaturation * wLuminosity + 120) / 240;
		else
			wMid2 = ((wSaturation + 240) * wLuminosity + 120) / 240;

		wMid1 = wLuminosity * 2 - wMid2;

		wRed   = GET_RGB(wHue + 80);
		wGreen = GET_RGB(wHue);
		wBlue  = GET_RGB(wHue - 80);

		return RGB(wRed, wGreen, wBlue);
	}
	wRed = wLuminosity * 255 / 240;
	return RGB(wRed, wRed, wRed);
}

/////////////////////////////////////////////////////////////////////////
//      color_rgb2hls
//
// Convert an rgb COLORREF into the hls color space.
//
// PARAMS
//  cRGB         [I] Source rgb value
//  pwHue        [O] Destination for converted hue
//  pwLuminance  [O] Destination for converted luminance
//  pwSaturation [O] Destination for converted saturation
//
// RETURNS
//  Nothing. pwHue, pwLuminance and pwSaturation are set to the converted
//  values.
//
// NOTES
//  Output HLS values are constrained to the range (0..240).
//  For Achromatic conversions, Hue is set to 160.
//
VOID color_rgb2hls(COLORREF cRGB, LPWORD pwHue, LPWORD pwLuminance, LPWORD pwSaturation)
{
	int wR, wG, wB, wMax, wMin, wHue, wLuminosity, wSaturation;

	wR = GetRValue(cRGB);
	wG = GetGValue(cRGB);
	wB = GetBValue(cRGB);

	wMax = max(wR, max(wG, wB));
	wMin = min(wR, min(wG, wB));

	/* Luminosity */
	wLuminosity = ((wMax + wMin) * 240 + 255) / 510;

	if (wMax == wMin) {
		/* Achromatic case */
		wSaturation = 0;
		/* Hue is now unrepresentable, but this is what native returns... */
		wHue = 160;
	}
	else {
		/* Chromatic case */
		int wDelta = wMax - wMin, wRNorm, wGNorm, wBNorm;

		/* Saturation */
		if (wLuminosity <= 120)
			wSaturation = ((wMax + wMin)/2 + wDelta * 240) / (wMax + wMin);
		else
			wSaturation = ((510 - wMax - wMin)/2 + wDelta * 240) / (510 - wMax - wMin);

		/* Hue */
		wRNorm = (wDelta/2 + wMax * 40 - wR * 40) / wDelta;
		wGNorm = (wDelta/2 + wMax * 40 - wG * 40) / wDelta;
		wBNorm = (wDelta/2 + wMax * 40 - wB * 40) / wDelta;

		if (wR == wMax)
			wHue = wBNorm - wGNorm;
		else if (wG == wMax)
			wHue = 80 + wRNorm - wBNorm;
		else
			wHue = 160 + wGNorm - wRNorm;
		if (wHue < 0)
			wHue += 240;
		else if (wHue > 240)
			wHue -= 240;
	}
	if (pwHue)
		*pwHue = wHue;
	if (pwLuminance)
		*pwLuminance = wLuminosity;
	if (pwSaturation)
		*pwSaturation = wSaturation;
}

/////////////////////////////////////////////////////////////////////////
// ColorAdjustLuma      [SHLWAPI.@]
//
// Adjust the luminosity of a color
//
// PARAMS
//  cRGB         [I] RGB value to convert
//  dwLuma       [I] Luma adjustment
//  bUnknown     [I] Unknown
//
// RETURNS
//  The adjusted RGB color.
//
COLORREF color_adjustluma(COLORREF cRGB, int nLuma)
{
	nLuma = min(1000, max(-1000, nLuma));
	if (dwLuma) {
		WORD wH, wL, wS;
		color_rgb2hls(cRGB, &wH, &wL, &wS);

		float delta;
		if (nLuma > 0)
			delta = (float)(240 - wL) / 1000.0F;
		else
			delta = (float)wL / 1000.0F;
		wL = wL + (int)(delta * (float)nLuma);
		wL = min(240, max(0, wL));

		cRGB = color_hls2rgb(wH, wL, wS);
  }
  return cRGB;
}
