//////////////////////////////////////////////////////////////////////
// pagecurl.cpp

#include "stdafx.h"
#include "pagecurl.h"

void progress_init(int maxprogress);
void progress_continue();
void progress_term();

/* Page Curl 0.9 --- image filter plug-in for The Gimp 1.3.0
 * Ported to the 0.99.x architecture by Simon Budig
 */

#define NGRADSAMPLES			1000
#ifndef M_PI
	#define M_PI				(3.14159265358979323846)
#endif

typedef struct tagVector2 {
	double x;
	double y;
} Vector2;

typedef struct tagCurlParams {
	double opaque;				// 不透明度 0.00 - 1.00
	BOOL upper_left;
	BOOL upper_right;
	BOOL lower_left;
	BOOL lower_right;
	BOOL vertical;				// めくる向き 縦=TRUE
	COLORREF shade_color;		// めくった場所の色
} CurlParams;


static CurlParams cval;
static int   true_width, true_height;
static int   sel_width, sel_height;
static Vector2 center;
static double radius;
static Vector2 left_tangent;
static Vector2 right_tangent;
static double diagl_slope;
static double diagr_slope;
static double diagb_slope;
static double diagm_slope;
static RGBA fore_color;
static RGBA back_color;


//////////////////////////////////////////////////////////////////////
// vector2 functions

void vector2_set(Vector2 *vector, double x, double y)
{
	vector->x = x;
	vector->y = y;
}

void vector2_add(Vector2 *result, Vector2 *vector1, Vector2 *vector2)
{
	result->x = vector1->x + vector2->x;
	result->y = vector1->y + vector2->y;
}

void vector2_sub(Vector2 *result, Vector2 *vector1, Vector2 *vector2)
{
	result->x = vector1->x - vector2->x;
	result->y = vector1->y - vector2->y;
}

double vector2_inner_product(Vector2 *vector1, Vector2 *vector2)
{
	return (vector1->x * vector2->x + vector1->y * vector2->y);
}

double vector2_length(Vector2 *vector)
{
	return (sqrt(vector->x * vector->x + vector->y * vector->y));
}

//////////////////////////////////////////////////////////////////////
//

static int left_of_diagl(double x, double y)
{
	return (x < (sel_width + (y - sel_height) * diagl_slope));
}

static int right_of_diagr(double x, double y)
{
	return (x > (sel_width + (y - sel_height) * diagr_slope));
}

static int below_diagb(double x, double y)
{
	return (y < (right_tangent.y + (x - right_tangent.x) * diagb_slope));
}

static int right_of_diagm(double x, double y)
{
	return (x > (sel_width + (y - sel_height) * diagm_slope));
}

static int inside_circle(double x, double y)
{
	double dx = x - center.x;
	double dy = y - center.y;
	return (sqrt(dx * dx + dy * dy) <= radius);
}

static void init_calculation(mif& image)
{
	true_width = image.m_width;
	true_height = image.m_height;
	if (cval.vertical)
	{
		sel_width = true_width;
		sel_height = true_height;
	}
	else
	{
		sel_width = true_height;
		sel_height = true_width;
	}

	// Circle parameters
	double alpha = atan((double)sel_height / sel_width);
	double beta = alpha / 2.0;
	double k = sel_width / ((M_PI + alpha) * sin(beta) + cos(beta));
	vector2_set(&center, k * cos(beta), k * sin(beta));
	radius = center.y;

	// left_tangent
	vector2_set(&left_tangent, radius * -sin(alpha), radius * cos(alpha));
	vector2_add(&left_tangent, &left_tangent, &center);

	// right_tangent
	Vector2 v1, v2;
	vector2_sub(&v1, &left_tangent, &center);
	vector2_set(&v2, sel_width - center.x, sel_height - center.y);
	double angle = -2.0 * acos(vector2_inner_product(&v1, &v2) /
						(vector2_length(&v1) * vector2_length(&v2)));
	vector2_set(&right_tangent,
						v1.x * cos(angle) + v1.y * -sin(angle),
						v1.x * sin(angle) + v1.y * cos(angle));
	vector2_add(&right_tangent, &right_tangent, &center);

	// Slopes
	diagl_slope = (double)sel_width / sel_height;
	diagr_slope = (sel_width - right_tangent.x) / (sel_height - right_tangent.y);
	diagb_slope = (right_tangent.y - left_tangent.y) / (right_tangent.x - left_tangent.x);
	diagm_slope = (sel_width - center.x) / sel_height;
}

static void curl_effect(mif& image)
{
	RGBA* ps = image.m_pixels;
	int cnt = image.m_packets;
	while (cnt-- > 0) {
		ps->red = ps->green = ps->blue = ps->alpha = 0;
		ps++;
	}

	// Init shadow
	Vector2 dl;
	vector2_set(&dl, -sel_width, -sel_height);
	double dl_mag = vector2_length(&dl);

	Vector2 dr;
	vector2_set(&dr, -(sel_width - right_tangent.x), -(sel_height - right_tangent.y));
	double dr_mag = vector2_length(&dr);

	double alpha = acos(vector2_inner_product(&dl, &dr) / (dl_mag * dr_mag));
	double beta = alpha / 2;

	int x, y;
progress_init(true_height);
	for (int y1 = 0; y1 < true_height; y1++) {
		ps = image.pos(0, y1);
		for (int x1 = 0; x1 < true_width; x1++) {
			// Map coordinates to get the curl correct...
			if (cval.vertical) {
				x = (cval.upper_right || cval.lower_right) ? x1 : sel_width - 1 - x1;
				y = (cval.upper_left || cval.upper_right) ? y1 : sel_height - 1 - y1;
			}
			else {
				x = (cval.lower_right || cval.lower_left) ? y1 : sel_width - 1 - y1;
				y = (cval.upper_left || cval.lower_left) ? x1 : sel_height - 1 - x1;
			}

			if (left_of_diagl(x, y)) {
				// uncurled region
				ps->red = ps->green = ps->blue = ps->alpha = 0;
			}
			else if (right_of_diagr(x, y) || (right_of_diagm(x, y) && below_diagb(x, y) && !inside_circle(x, y))) {
				// めくれた場所
				ps->red   = GetRValue(cval.shade_color);
				ps->green = GetGValue(cval.shade_color);
				ps->blue  = GetBValue(cval.shade_color);
				ps->alpha = 255;
			}
			else {
				Vector2 v;
				v.x = -(sel_width - x);
				v.y = -(sel_height - y);
				double angle = acos(vector2_inner_product(&v, &dl) / (vector2_length(&v) * dl_mag));

				if (inside_circle(x, y) || below_diagb(x, y)) {
					// Below the cval.
					double factor = angle / alpha;
					ps->red = ps->green = ps->blue = 0;
					ps->alpha = (BYTE)((double)255.0 * factor);
				}
				else {
					// On the curl
					double intensity = pow(sin(M_PI * angle / alpha), 1.5);
					ps->red   = (BYTE)(intensity * back_color.red   + (1 - intensity) * fore_color.red);
					ps->green = (BYTE)(intensity * back_color.green + (1 - intensity) * fore_color.green);
					ps->blue  = (BYTE)(intensity * back_color.blue  + (1 - intensity) * fore_color.blue);
					ps->alpha = (BYTE)((double)255 * (1 - intensity * (1 - cval.opaque)));
				}
			}
			ps++;
		}
progress_continue();
	}
progress_term();
}

BOOL page_curl(mif& image, LPRECT prc, PAGECURLINFO* pci)
{
	CRect rc;
	if (!image.validaterect(prc, &rc))
		return FALSE;

	// パラメタ初期設定
	cval.upper_left  = (pci->corner == curlUpperLeft);
	cval.upper_right = (pci->corner == curlUpperRight);
	cval.lower_left  = (pci->corner == curlLowerLeft);
	cval.lower_right = (pci->corner == curlLowerRight);
	cval.vertical    = pci->vert;
	cval.opaque      = (double)pci->opaque / 100.0;
	cval.shade_color = pci->back_color;

	if (cval.upper_left) {
		rc.right = rc.left + rc.Width() * pci->xper / 100;
		rc.bottom = rc.top + rc.Height() * pci->yper / 100;
	}
	else if (cval.upper_right) {
		rc.left = rc.right - rc.Width() * pci->xper / 100;
		rc.bottom = rc.top + rc.Height() * pci->yper / 100;
	}
	else if (cval.lower_left) {
		rc.right = rc.left + rc.Width() * pci->xper / 100;
		rc.top = rc.bottom - rc.Height() * pci->yper / 100;
	}
	else {
		rc.left = rc.right - rc.Width() * pci->xper / 100;
		rc.top = rc.bottom - rc.Height() * pci->yper / 100;
	}

	fore_color.red   = GetRValue(pci->darken_color);
	fore_color.green = GetGValue(pci->darken_color);
	fore_color.blue  = GetBValue(pci->darken_color);
	back_color.red   = GetRValue(pci->lighten_color);
	back_color.green = GetGValue(pci->lighten_color);
	back_color.blue  = GetBValue(pci->lighten_color);

	mif curl(rc.Width(), rc.Height(), 24);
	init_calculation(curl);
	curl_effect(curl);

	image.addlayer(&curl, rc.left, rc.top, blendNormal, 255 * pci->opaque / 100);
	return TRUE;
}
