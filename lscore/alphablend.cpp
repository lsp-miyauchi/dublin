//////////////////////////////////////////////////////////////////////
// AlphaBlend関数の自前インプリメント

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "fnc.h"
#include "common.h"

/********************************************************************** 
*                                                                     *
*  Create24BPPDIBSection(HDC, int, int);                              *
*                                                                     *
*  Purpose:                                                           *
*      Creates a 24 bit-per-pixel DIB section with the specified      *
*      width and height.                                              *
*                                                                     *
**********************************************************************/ 
static HBITMAP Create24BPPDIBSection(HDC hDC, int iWidth, int iHeight)
{
	BITMAPINFO bmi;
	LPBYTE pBits;

	// Initialize header to 0s.
	memset(&bmi, 0, sizeof(bmi));

	// Fill out the fields you care about.
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = iWidth;
	bmi.bmiHeader.biHeight = iHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;

	// Create the surface.
	return CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (void**)&pBits, NULL, 0);
}

/**********************************************************************
*                                                                     *
*  BitmapsCompatible(LPBITMAP, LPBITMAP)                              *
*                                                                     *
*  Purpose:                                                           *
*      Compares two bitmap structures to see if the surfaces are      *
*      the same width, height, and pixel format.                      *
*                                                                     *
**********************************************************************/ 
static BOOL BitmapsCompatible(LPBITMAP lpbm1, LPBITMAP lpbm2)
{
	if (lpbm1->bmBitsPixel != lpbm2->bmBitsPixel) return FALSE;
	if (lpbm1->bmPlanes    != lpbm2->bmPlanes)    return FALSE;
	if (lpbm1->bmWidth     != lpbm2->bmWidth)     return FALSE;
	if (lpbm1->bmHeight    != lpbm2->bmHeight)    return FALSE;

	return TRUE;
}

_inline BYTE do_blend(BYTE src, BYTE dst, int alpha)
	{ return (BYTE)(((DWORD)src * alpha + (DWORD)dst * (255 - alpha)) >> 8); }

/**********************************************************************
*                                                                     *
*  BOOL BlendImages(HBITMAP, HBITMAP, HBITMAP, DWORD);                *
*                                                                     *
*  Purpose:                                                           *
*      Blends two source images into a destination image              *
*      using a specified weighting value for the first source         *
*      image.                                                         *
*                                                                     *
*  Notes:                                                             *
*      All HBITMAP parameters must be 24 bit-per-pixel DIB sections.  *
*      All HBITMAP parameters must have the same widths and heights.  *
*      Weighting values must be in the range of 0 .. 255.             *
*                                                                     *
**********************************************************************/ 
static BOOL BlendImages(HBITMAP hbmSrc1, HBITMAP hbmSrc2, HBITMAP hbmDst, DWORD alpha)
{

	// Only values between 0 and 255 are valid.
	if (alpha > 255)
			return FALSE;

	// Get information about the surfaces you were passed.
	BITMAP bmSrc1, bmSrc2, bmDst;
	if (!GetObject(hbmSrc1, sizeof(BITMAP), &bmSrc1)) return FALSE;
	if (!GetObject(hbmSrc2, sizeof(BITMAP), &bmSrc2)) return FALSE;
	if (!GetObject(hbmDst,  sizeof(BITMAP), &bmDst))  return FALSE;

	// Make sure you have data that meets your requirements.
	if (!BitmapsCompatible(&bmSrc1, &bmSrc2)) return FALSE;
	if (!BitmapsCompatible(&bmSrc1, &bmDst))  return FALSE;
	if (bmSrc1.bmBitsPixel != 24) return FALSE;
	if (bmSrc1.bmPlanes != 1) return FALSE;
	if (!bmSrc1.bmBits || !bmSrc2.bmBits || !bmDst.bmBits) return FALSE;

	DWORD widthbytes = bmDst.bmWidthBytes;

	// Initialize the surface pointers.
	RGBTRIPLE* ps1 = (RGBTRIPLE*)bmSrc1.bmBits;
	RGBTRIPLE* ps2 = (RGBTRIPLE*)bmSrc2.bmBits;
	RGBTRIPLE* pd  = (RGBTRIPLE*)bmDst.bmBits;

	for (int y = 0; y < bmDst.bmHeight; y++)
	{
		for (int x = 0; x < bmDst.bmWidth; x++)
		{
			pd[x].rgbtRed   = do_blend(ps1[x].rgbtRed,   ps2[x].rgbtRed,   alpha);
			pd[x].rgbtGreen = do_blend(ps1[x].rgbtGreen, ps2[x].rgbtGreen, alpha);
			pd[x].rgbtBlue  = do_blend(ps1[x].rgbtBlue,  ps2[x].rgbtBlue,  alpha);
        }

		// Move to next scan line.
		ps1 = (RGBTRIPLE*)((LPBYTE)ps1 + widthbytes);
		ps2 = (RGBTRIPLE*)((LPBYTE)ps2 + widthbytes);
		pd  = (RGBTRIPLE*)((LPBYTE)pd  + widthbytes);
	}
	return TRUE;
}

/**********************************************************************
*                                                                     *
*  BOOL DoAlphaBlend()                                                *
*                                                                     *
*  Purpose:                                                           *
*      Captures a copy of the source and destination areas and        *
*      alpha blends them into a memory surface that it displays       *
*      into the destination area.                                     *
*                                                                     *
*  Notes:                                                             *
*      Takes the same parameters as the AlphaBlend function except    *
*      that the last parameter is a source weighting value rather     *
*      than a BLENDFUNCTION structure.                                *
*                                                                     *
**********************************************************************/ 
extern "C"
BOOL WINAPI lccAlphaBlend(
  HDC hdcDest,                 // Handle to destination DC.
  int nXDest,            // X-coord of upper-left corner.
  int nYDest,            // Y-coord of upper-left corner.
  int nWidth,				   // Destination width.
  int nHeight,		           // Destination height.
  HDC hdcSrc,                  // Handle to source DC.
  int nXSrc,             // X-coord of upper-left corner.
  int nYSrc,             // Y-coord of upper-left corner.
  DWORD dwSourceWeight)        // Source weighting (between 0 and 255).
{
    HDC      hdcSrc1 = NULL;
    HDC      hdcSrc2 = NULL;
    HDC      hdcDst  = NULL;
    HBITMAP  hbmSrc1 = NULL;
    HBITMAP  hbmSrc2 = NULL;
    HBITMAP  hbmDst  = NULL;
    BOOL     bReturn;

    // Create surfaces for sources and destination images.
    hbmSrc1 = Create24BPPDIBSection(hdcDest, nWidth, nHeight);
    if (!hbmSrc1) goto HANDLEERROR;

    hbmSrc2 = Create24BPPDIBSection(hdcDest, nWidth, nHeight);
    if (!hbmSrc2) goto HANDLEERROR;

    hbmDst  = Create24BPPDIBSection(hdcDest, nWidth, nHeight);
    if (!hbmDst) goto HANDLEERROR;

    // Create HDCs to hold our surfaces.
    hdcSrc1 = CreateCompatibleDC(hdcDest);
    if (!hdcSrc1) goto HANDLEERROR;
    
    hdcSrc2 = CreateCompatibleDC(hdcDest);
    if (!hdcSrc2) goto HANDLEERROR;
    
    hdcDst  = CreateCompatibleDC(hdcDest);
    if (!hdcDst) goto HANDLEERROR;

    // Prepare the surfaces for drawing.
    SelectObject(hdcSrc1, hbmSrc1);
    SelectObject(hdcSrc2, hbmSrc2);
    SelectObject(hdcDst,  hbmDst);

    // Capture a copy of the source area.
    if (!BitBlt(hdcSrc1, 0,0,nWidth, nHeight, hdcSrc,  nXSrc, nYSrc, SRCCOPY))
         goto HANDLEERROR;

    // Capture a copy of the destination area.
    if (!BitBlt(hdcSrc2, 0,0,nWidth, nHeight, hdcDest, nXDest, nYDest, SRCCOPY))
         goto HANDLEERROR;

    // Blend the two source areas to create the destination image.
    bReturn = BlendImages(hbmSrc1, hbmSrc2, hbmDst, dwSourceWeight);

    // Clean up objects you do not need any longer.
    // You cannot delete an object that's selected into an
    // HDC so delete the HDC first. 
    DeleteDC(hdcSrc1);
    DeleteDC(hdcSrc2);
    DeleteObject(hbmSrc1);
    DeleteObject(hbmSrc2);

    // Display the blended (destination) image to the target HDC.
    if (bReturn) {
        BitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight, hdcDst, 0, 0, SRCCOPY);
    }

    // Clean up the rest of the objects you created.
    DeleteDC(hdcDst);
    DeleteObject(hbmDst);

    return bReturn;

HANDLEERROR:

    if (hdcSrc1) DeleteDC(hdcSrc1);
    if (hdcSrc2) DeleteDC(hdcSrc2);
    if (hdcDst)  DeleteDC(hdcDst);
    if (hbmSrc1) DeleteObject(hbmSrc1);
    if (hbmSrc2) DeleteObject(hbmSrc2);
    if (hbmDst)  DeleteObject(hbmDst);

    return FALSE;
} 
