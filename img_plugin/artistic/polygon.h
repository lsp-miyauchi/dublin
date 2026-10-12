////////////////////////////////////////////////////////////
// polygon.h

#pragma once
#ifndef __POLYGON_H__
#define __POLYGON_H__

typedef struct
{
	double x, y;
} VERTEX;

class polygon
{
public:
	polygon()
	{
		npts = 0;
		pts = NULL;
	}

	polygon(int maxcnt)
	{
		npts = 0;
		pts = NULL;
		init(maxcnt);
	}

	~polygon()
	{
		if (pts) free(pts);
	}

	void init(int maxcnt)
	{
		if (pts) free(pts);

		npts = 0;
		maxnpts = maxcnt;
		pts = new VERTEX[maxcnt];
	}

	void add_point(double x, double y)
	{
		if (npts < maxnpts) {
			pts[npts].x = x;
			pts[npts].y = y;
			npts++;
		}
	}

	bool find_center(double* cx, double* cy)
	{
		if (!npts) return false;

		*cx = 0.0;
		*cy = 0.0;
		for (int i = 0; i < npts; i++) {
			*cx += pts[i].x;
			*cy += pts[i].y;
		}
		*cx /= npts;
		*cy /= npts;
		return true;
	}

	void translate(double tx, double ty)
	{
		for (int i = 0; i < npts; i++) {
			pts[i].x += tx;
			pts[i].y += ty;
		}
	}

	void scale(double poly_scale)
	{
		for (int i = 0; i < npts; i++) {
			pts[i].x *= poly_scale;
			pts[i].y *= poly_scale;
		}
	}

	bool extents(double* x1, double* y1, double* x2, double* y2)
	{
		if (!npts) return false;

		*x1 = *x2 = pts[0].x;
		*y1 = *y2 = pts[0].y;
		for (int i = 1; i < npts; i++) {
			if (pts[i].x < *x1) *x1 = pts[i].x;
			if (pts[i].x > *x2) *x2 = pts[i].x;
			if (pts[i].y < *y1) *y1 = pts[i].y;
			if (pts[i].y > *y2) *y2 = pts[i].y;
		}
		return true;
	}

	void rotate(double theta)
	{
		double ct = cos(theta);
		double st = sin (theta);
		double ox, oy;
		for (int i = 0; i < npts; i++) {
			ox = pts[i].x;
			oy = pts[i].y;
			pts[i].x = ct * ox - st * oy;
			pts[i].y = st * ox + ct * oy;
		}
	}

	void reset()
	{
		npts = 0;
	}

	int maxnpts;
	int npts;
	VERTEX* pts;
};


#endif			// __POLYGON_H__
