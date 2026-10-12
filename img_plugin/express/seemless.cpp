//////////////////////////////////////////////////////////////////////
// seemless.cpp (gimp original is tiler.c)

#include "stdafx.h"

extern PROGRESSCALLBACK pfnProgress;
void progress_init(int maxprogress);
void progress_continue();
void progress_term();

static int __fastcall scale(int width, int height, int x, int y, int data)
{
	int A = (width >> 1) - 1;
	int B = (height >> 1) - 1;
	int a, b;

	if (x < (width >> 1))
		a = (width >> 1) - x - 1; 
	else if ((x == (width >> 1)) && (width & 1))
		return data;
	else
		a = x - (width >> 1) - (width & 1);

	if (y < (height >> 1))
		b = (height >> 1) - y - 1; 
	else if ((y == (height >> 1)) && (height & 1))
		return data;
	else
		b = y - (height >> 1) - (height & 1);

	if ((B * a < A * b) || ((B * a == A * b) && (a & 1))) {
		a = A - a;
		b = B - b;
		if (a == A)
			return data;
		else
			return data - ((data * (A * B - a * B)) / (A * b + A * B - a * B));
	}
	else {
		if (a == A)
			return 0;
		else
			return (data * (A * B - a * B)) / (A * b + A * B - a * B);
	}
}

static HDIB seemless(HDIB hsrc, LPRECT prc)
{
	CRect rc;
	mif src;
	if (!src.attachdib(hsrc) || !src.validaterect(prc, &rc))
		return NULL;
	if (src.m_bits < 24)
		src.m_bits = 24;

	mif dst;
	dst.create(rc.Width(), rc.Height(), 24);

	int cx = rc.Width();
	int cy = rc.Height();
	int wodd = cx & 1;
	int hodd = cy & 1;

	// allocate row buffers
	BYTE* cur_row = (BYTE*)malloc(cx * 3);
	BYTE* dest_cur = (BYTE*)malloc(cx * 3);
	BYTE* dest_top = (BYTE*)malloc(cx * 3);
	BYTE* dest_bot = (BYTE*)malloc(cx * 3);

progress_init(cy >> 3);
	for (int y = 0; y < cy; y++) {
		src.getrow_rgb(cur_row, rc.left, y + rc.top, cx);
		dst.getrow_rgb(dest_cur, 0, y, cx);
		if (y >= (cy >> 1) + hodd)
			dst.getrow_rgb(dest_top, 0, y - (cy >> 1) - hodd, cx);
		if (y < (cy >> 1))
			dst.getrow_rgb(dest_bot, 0, y + (cy >> 1) + hodd, cx);

		for (int x = 0; x < cx; x++) {
			for (int c = 0; c < 3; c++) {
				int val = scale(cx, cy, x, y, cur_row[x * 3 + c]);

				// Main image
				dest_cur[x * 3 + c] += val;
				// top left
				if ((x >= (cx >> 1) + wodd) && (y >= (cy >> 1) + hodd))
					dest_top[(x - (cx >> 1) - wodd) * 3 + c] += val;
				// top right
				if ((x < (cx >> 1)) && (y >= (cy >> 1) + hodd))
					dest_top[((cx >> 1) + x + wodd) * 3 + c] += val;
				// bottom left
				if ((x >= (cx >> 1) + wodd) && (y < (cy >> 1)))
					dest_bot[(x - (cx >> 1) - wodd) * 3 + c] += val;
				// bottom right
				if ((x < (cx >> 1)) && (y < (cy >> 1)))
					dest_bot[(x + (cx >> 1) + wodd) * 3 + c] += val;
			}
		}

		dst.setrow_rgb(dest_cur, 0, y, cx);
		if (y >= (cy >> 1) + hodd)
			dst.setrow_rgb(dest_top, 0, y - (cy >> 1) - hodd, cx);
		if (y < (cy >> 1))
			dst.setrow_rgb(dest_bot, 0, y + (cy >> 1) + hodd, cx);

if (!(y & 7)) progress_continue();
	}
progress_term();

	free(cur_row);
	free(dest_cur);
	free(dest_top);
	free(dest_bot);

	src.blt(rc.left, rc.top, cx, cy, &dst, 0, 0);
	dst.destroy();
	return src.createdib();
}

HANDLE seemless_ui(HDIB hdib, LPRECT prc, FILTER_EXEC* pfe)
{
pfnProgress = pfe->progress;
	HDIB h = seemless(hdib, prc);
pfnProgress = NULL;
	return h;
}
