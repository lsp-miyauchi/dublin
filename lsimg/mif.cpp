//////////////////////////////////////////////////////////////////////
// mif.cpp

#include "stdafx.h"
#include "lsimg.h"
#include "common.h"

extern "C"
HDIB WINAPI mifIncreaseColor(HDIB hsrc, int bits)
{
	mif image;
	if (image.attachdib(hsrc))
	{
		if (bits <= 4)
			bits = 4;
		else if (bits <= 8)
			bits = 8;
		else
			bits = 24;

		// 指定されたビットカウントが現在のDIBのビットカウント以下ならば何もしない
		if (image.m_bits < bits)
		{
			image.m_bits = bits;
			return image.createdib();
		}
	}
	return NULL;
}

extern "C"
HDIB WINAPI mifDecreaseColorDirect(HDIB hsrc, int bits)
{
	// 例えば8bpp 16色を 4bpp 16色に、
	//       8bpp  2色を 1bpp  2色に、
	//       4bpp  2色を 1bpp  2色に変換する

	mif image;
	if (image.attachdib(hsrc))
	{
		if (image.m_bits <= 8 && image.m_bits > bits)
		{
			if (image.m_colors <= (1 << bits))
			{
				image.m_bits = bits;
				return image.createdib();
			}
		}
	}
	return NULL;
}
