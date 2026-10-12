///////////////////////////////////////////////////////////////////////
// pixelize.cpp ���U�C�N


#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

#define UPINT(a)		((int)(a))
#define F_SHI			UPINT(224)
#define F_ACC			UPINT(135)
#define F_HIL			UPINT(190)
#define F_SHD			UPINT(190)
#define F_THR			UPINT(135)

BOOL pixelize(mif& src, LPRECT prc, int xblock, int yblock, BOOL renga, BOOL cross)
{
	CRect rc;
	if (xblock < 2 || yblock < 2 || !src.validaterect(prc, &rc))
		return NULL;
	src.m_bits = 24;

	RGBA* arr = new RGBA[xblock * yblock];
	int sum_r, sum_g, sum_b;
	int x_step, y_step, count, i, line = 0;
	for (int y = rc.top; y < rc.bottom; y += yblock) {
		int xxb = (cross && (line & 1) != 0) ? xblock >> 1 : xblock;
		for (int x = rc.left; x < rc.right; ) {
			x_step = min(xxb, rc.right - x);
			y_step = min(yblock, rc.bottom - y);

			src.getpacket(arr, x, y, x_step, y_step);
			sum_r = sum_g = sum_b = 0;

			RGBA *p = arr;
			count = x_step * y_step;
			for (i = 0; i < count; i++) {
				sum_r += p->red;
				sum_g += p->green;
				sum_b += p->blue;
				p++;
			}

			sum_r = ADJRGB(sum_r / count);
			sum_g = ADJRGB(sum_g / count);
			sum_b = ADJRGB(sum_b / count);
			p = arr;
			for (i = 0; i < count; i++) {
				p->red = (BYTE)sum_r;
				p->green = (BYTE)sum_g;
				p->blue = (BYTE)sum_b;
				p++;
			}

			if (renga && x_step > 1 && y_step > 1) {
				RGBA hi, lo;
				hi.red   = ADJRGB((sum_r * F_HIL + 255 * (255 - F_HIL)) / 255);
				hi.green = ADJRGB((sum_g * F_HIL + 255 * (255 - F_HIL)) / 255);
				hi.blue  = ADJRGB((sum_b * F_HIL + 255 * (255 - F_HIL)) / 255);
				lo.red   = ADJRGB((sum_r * F_SHD) / 255);
				lo.green = ADJRGB((sum_g * F_SHD) / 255);
				lo.blue  = ADJRGB((sum_b * F_SHD) / 255);

				for (i = 0; i < x_step; i++) {
					arr[i] = hi;
					if (x_step > 2)
						arr[(y_step - 1) * x_step + i] = lo;
				}
				for (i = 1; i < y_step; i++) {
					arr[x_step * i] = hi;
					if (y_step > 2)
						arr[x_step * (i + 1) - 1] = lo;
				}
			}
			src.setpacket(arr, x, y, x_step, y_step);
			x += xxb;
			xxb = xblock;
		}
		line++;
	}
	delete [] arr;
	return TRUE;
}
