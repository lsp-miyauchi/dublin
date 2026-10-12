////////////////////////////////////////////////////////////////////////
// quantize_simple.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"


#define HASH_SIZE		20023

/* values for status of samples */
#define UNUSED			0
#define PRELEAF			1
#define LEAF			2
#define FINALLEAF		3


// hash table entry
class hash_t
{
public:
	hash_t();
	~hash_t() {}

	hash_t* next() { return m_next; }
	void addobj(hash_t* obj) { m_next = obj; }

	BYTE c[3];
	int num;
	hash_t* m_next;
};

hash_t::hash_t()
{
	num = 0;
	m_next = NULL;
}

class hash_t_arr
{
public:
	hash_t_arr();
	~hash_t_arr();
	hash_t* getat(int index) { return m_table[index]; }
	void setat(int index, hash_t* obj) { m_table[index] = obj; }

	hash_t** m_table;
};

hash_t_arr::hash_t_arr()
{
	m_table = (hash_t**)malloc(HASH_SIZE * sizeof(hash_t));
	memset(m_table, 0, HASH_SIZE * sizeof(hash_t));
}

hash_t_arr::~hash_t_arr()
{
	for (int i = 0; i < HASH_SIZE; i++)
	{
		hash_t *entry, *next;
		for (entry = m_table[i]; entry != NULL; entry = next)
		{
			next = entry->next();
			delete entry;
 		}
	}
	free(m_table);
}

// color entry in work space with multiplicity and key
// for sorting wrt principal axis
typedef struct {
	double c[3];
	double key;
	int num;
} color_info;

typedef struct {
	int first;			/* first color */
	int num;			/* number of colors in the sample */ 
	int weight; 		/* number of pixels in the sample */
	double u[3];		/* centroid of the sample in work space */
	BYTE ci[3];			/* centroid in image space */
	double error;		/* quantisation error for the sample */
	double gain;		/* gain of splitting this sample in c1, c2 */
	BYTE status;		/* may be UNUSED, PRELEAF, LEAF, FINALLEAF */
	int c1, c2;			/* children */
} sample;

class quantizer
{
public:
	quantizer();
	~quantizer();

	void init_CIE();
	void goto_workspace(BYTE* rgb, double* luv);
	void goto_imagespace(double* luv, BYTE* rgb);
	void binary_split(int idx);
	int principal_split(int resolution, int additional);
	void spectral(double S[3][3], double U[3][3], double K[3]);
	void invert(double A[3][3], double B[3][3]);
	void build_histogram(mif& image);
	void map_hashed(mif& src, sample *lut, int len);
	double calc_axis(int idx, double* axis);
	void calc_centroid(int idx);
	void calc_error(int idx);
	void sort_along_axis(double axis[], int start, int num);
	void quant(mif& image, int num_colors, BOOL cie_luv, BOOL principal);

	color_info *m_colors_arr;
	int m_max_color;
	sample *m_samples;
	int m_max_sample;

	// basic
	int m_K;					/* number of remaining colors */ 
	int m_rgb, m_luv;			/* workspace */
	// k-means clustering
	int m_kmeans;				/* do it ? this also affects the 2-clustering */
	int m_max_iterations;		/* maximal number of loops over the data points */
	// CIE Luv
	double m_xr, m_yr;			/* the chromaticity coordinates of the red phospor */
	double m_xg, m_yg;			/* the chromaticity coordinates of the green phospor */
	double m_xb, m_yb;			/* the chromaticity coordinates of the blue phospor */
	double m_xw, m_yw, m_Yw;	/* CIE XYZ coordinates for white (R=G=B=1) */ 
};

#define DEG_to_RAD(x)		((x)*M_PI/180.0)
#define RAD_to_DEG(x)		(180.0*(x)/M_PI)


/********************************************************************/
/* color space conversions                                          */
/********************************************************************/

/*
 * This code is based on information found in          
 * Foley, van Dam, Feiner and Hughes, Computer Graphics, 2. Ed, Chapter 13
 * The treatment of small Y in the color space transformations     
 * is not from somewhere else.
 */

static double RGB2XYZ[3][3];
static double XYZ2RGB[3][3];
static double YWhite, uWhite, vWhite;

quantizer::quantizer()
{
	m_colors_arr = NULL;
	m_samples = NULL;

	/* k-means clustering */
	m_kmeans = 1;
	m_max_iterations = 100;

	/* CIE Luv */
	m_xr = 0.67;
	m_yr = 0.33; 				// NTSC Phosphors: red
	m_xg = 0.21;
	m_yg = 0.71;					// green
	m_xb = 0.14;
	m_yb = 0.08;					// blue
	m_xw = 0.31;
	m_yw = 0.316;
	m_Yw = 100.0;				// illuminant C
}

quantizer::~quantizer()
{
	if (m_colors_arr)
		free(m_colors_arr);
	if (m_samples)
		free(m_samples);
}

void quantizer::init_CIE()
{
  double Xw, Zw;
  double U[3][3], Uinv[3][3], C[3]; 
  int i, j;

    Xw = m_xw / m_yw * m_Yw;
    Zw = (1 - m_xw - m_yw) / m_yw * m_Yw;
    U[0][0] = m_xr; 
    U[0][1] = m_xg; 
    U[0][2] = m_xb;
    U[1][0] = m_yr; 
    U[1][1] = m_yg; 
    U[1][2] = m_yb;
    U[2][0] = 1-m_xr-m_yr; 
    U[2][1] = 1-m_xg-m_yg; 
    U[2][2] = 1-m_xb-m_yb;
    
    /* obtain C by solving (Xw Yw Zw) = UC */
	invert(U, Uinv);
    C[0] = Uinv[0][0]*Xw + Uinv[0][1]*m_Yw + Uinv[0][2]*Zw; 
    C[1] = Uinv[1][0]*Xw + Uinv[1][1]*m_Yw + Uinv[1][2]*Zw; 
    C[2] = Uinv[2][0]*Xw + Uinv[2][1]*m_Yw + Uinv[2][2]*Zw; 
    
    /* calculate the matrix for going from RGB to XYZ */
    RGB2XYZ[0][0]=U[0][0]*C[0];RGB2XYZ[0][1]=U[0][1]*C[1];RGB2XYZ[0][2]=U[0][2]*C[2];
    RGB2XYZ[1][0]=U[1][0]*C[0];RGB2XYZ[1][1]=U[1][1]*C[1];RGB2XYZ[1][2]=U[1][2]*C[2];
    RGB2XYZ[2][0]=U[2][0]*C[0];RGB2XYZ[2][1]=U[2][1]*C[1];RGB2XYZ[2][2]=U[2][2]*C[2];
    
    invert(RGB2XYZ,XYZ2RGB);
    
    for (i=0;i<3;i++) {
      for (j=0;j<3;j++) {
	RGB2XYZ[i][j]/=255.0;
	XYZ2RGB[i][j]*=255.0;
      }
    }
    YWhite = m_Yw;
    uWhite = 4.0 * Xw / (Xw + 15.0 * m_Yw + 3.0 * Zw);
    vWhite = 9.0 * m_Yw / (Xw + 15.0 * m_Yw + 3.0 * Zw);
}

void quantizer::goto_workspace(BYTE* rgb, double* luv)
{
	double X, Y, Z, d;

	if (m_rgb)
	{
		/* go to normalized RGB */
		luv[0] = rgb[0] / 255.0;
		luv[1] = rgb[1] / 255.0;
		luv[2] = rgb[2] / 255.0;
	}
	else
	{
		/* go to CIE XYZ */
		X = RGB2XYZ[0][0] * rgb[0] + RGB2XYZ[0][1] * rgb[1] + RGB2XYZ[0][2] * rgb[2];
		Y = RGB2XYZ[1][0] * rgb[0] + RGB2XYZ[1][1] * rgb[1] + RGB2XYZ[1][2] * rgb[2];
		Z = RGB2XYZ[2][0] * rgb[0] + RGB2XYZ[2][1] * rgb[1] + RGB2XYZ[2][2] * rgb[2];
		/* go to CIE Luv */
		d = X + 15.0 * Y + 3.0 * Z;
		if (d > 0)
		{
			luv[1] = 4.0 * X / d;
			luv[2] = 9.0 * Y / d;
		}
		else
		{
			luv[1] = 0;
			luv[2] = 0;
		}
		if (Y / YWhite > 0.008856)
			luv[0] = 116.0 * pow(Y / YWhite, 0.33333333) - 16.0;
		else
			luv[0] = 903.0 * Y / YWhite;
		luv[1] = 13.0 * (luv[0]) * (luv[1] - uWhite);
		luv[2] = 13.0 * (luv[0]) * (luv[2] - vWhite);
	}
}

void quantizer::goto_imagespace(double* luv, BYTE* rgb)
{
	double X, Y, Z, d, uu, vv;
  
	if (m_rgb)
	{
		/* go to unnormalized RGB */    
		rgb[0] = (BYTE)(luv[0] * 255.0);
		rgb[1] = (BYTE)(luv[1] * 255.0);
		rgb[2] = (BYTE)(luv[2] * 255.0);
	}
	else
	{
		/* go to CIE XYZ */
		if (luv[0] == 0)
		{
			X = Y = Z = 0;
		}
		else
		{
			if (luv[0] <= 7.996968)
			{
				Y = luv[0] / 903.0 * YWhite;
			}
			else
			{
				Y = pow((luv[0] + 16.0) / 116.0, 3.0)  * YWhite;
			}
			uu = luv[1] / (13.0 * luv[0]) + uWhite;
			vv = luv[2] / (13.0 * luv[0]) + vWhite;
			d = 9.0 * Y / vv;
			X = uu * d / 4.0;
			Z = (d - 15.0 * Y - X) / 3.0;
		}
		/* go to unnormalized RGB */
		rgb[0] = (BYTE)(XYZ2RGB[0][0]*X + XYZ2RGB[0][1]*Y + XYZ2RGB[0][2]*Z);
		rgb[1] = (BYTE)(XYZ2RGB[1][0]*X + XYZ2RGB[1][1]*Y + XYZ2RGB[1][2]*Z);
		rgb[2] = (BYTE)(XYZ2RGB[2][0]*X + XYZ2RGB[2][1]*Y + XYZ2RGB[2][2]*Z);
	}
}

/********************************************************************/
/* matrix decomposition and inversion                               */
/********************************************************************/

/* 
 * The spectral decomposition code is from
 * Ken Shoemake, Polar Matrix Decomposition, in Graphics Gems IV.
 * The inversion code is based on the classical adjoint for 3x3-matrices;
 * look in any book on linear algebra for that.
 */

void quantizer::spectral(double S[3][3], double U[3][3], double K[3])
{
  double OffD[3], sm, g, h, fabsh, fabsOffDi, t, theta;
  double c, s, tau, ta, OffDq, a, b;
  int sweep, i, j, p, q;

  for (i = 0; i < 3; i++) {
    U[i][i] = 1;
    K[i] = S[i][i];
    OffD[i] = S[((i + 1) % 3)][(i + 2) % 3];
    for (j = i + 1; j < 3; j++) {
      U[i][j] = U[j][i] = 0;
    }
  }
  for (sweep = 25; sweep > 0; sweep--) {
    sm = fabs(OffD[0]) + fabs(OffD[1]) + fabs(OffD[2]);
    if (sm == 0) break;
    for (i = 2; i >= 0; i--) {
      p = (i + 1) % 3;
      q = (p + 1) % 3;
      
      fabsOffDi = fabs(OffD[i]);
      g = 100.0 * fabsOffDi;
      if (fabsOffDi > 0) {
	h = K[q] - K[p];
	fabsh = fabs(h);
	if (fabsh + g == fabsh) {
	  t = OffD[i] / h;
	} else {
	  theta = 0.5 * h / OffD[i];
	  t = 1.0 / (fabs(theta) + sqrt(theta * theta + 1.0));
	  if (theta < 0) t = -t;
	}
	c = 1.0 / sqrt(t * t + 1);
	s = t * c;
	tau = s / (c + 1);
	ta = t * OffD[i];
	OffD[i] = 0.0;
	K[p] -= ta;
	K[q] += ta;
	OffDq = OffD[q];
	OffD[q] -= s * (OffD[p] + tau * OffD[q]);
	OffD[p] += s * (OffDq - tau * OffD[p]);
	for (j = 2; j >= 0; j--) {
	  a = U[j][p];
	  b = U[j][q];
	  U[j][p] -= s * (b + tau * a);
	  U[j][q] += s * (a - tau * b);
	}
      }
    }
  }
}

void quantizer::invert(double A[3][3], double B[3][3])
{
	double det;
	B[0][0] =   (A[1][1]*A[2][2] - A[1][2]*A[2][1]);
	B[1][0] = - (A[1][0]*A[2][2] - A[1][2]*A[2][0]);
	B[2][0] =   (A[1][0]*A[2][1] - A[1][1]*A[2][0]);
	B[0][1] = - (A[0][1]*A[2][2] - A[0][2]*A[2][1]);
	B[1][1] =   (A[0][0]*A[2][2] - A[0][2]*A[2][0]);
	B[2][1] = - (A[0][0]*A[2][1] - A[0][1]*A[2][0]);
	B[0][2] =   (A[0][1]*A[1][2] - A[0][2]*A[1][1]);
	B[1][2] = - (A[0][0]*A[1][2] - A[0][2]*A[1][0]);
	B[2][2] =   (A[0][0]*A[1][1] - A[0][1]*A[1][0]);
	det = A[0][0]*B[0][0] + A[0][1]*B[1][0] + A[0][2]*B[2][0];
	B[0][0] /= det; B[0][1] /= det; B[0][2] /= det;
	B[1][0] /= det; B[1][1] /= det; B[1][2] /= det;
	B[2][0] /= det; B[2][1] /= det; B[2][2] /= det;
}


/********************************************************************/
/* building histogram of source image                               */
/********************************************************************/

/* 
 * The next two functions implement histogram building and nearest neighbour 
 * mapping using a hash table with separate chaining. This code is strongly 
 * based on the corresponding parts of libppm, see the copyright notice below.
 */ 

/* libppm3.c - ppm utility library part 3
**
** Colormap routines.
**
** Copyright (C) 1989, 1991 by Jef Poskanzer.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies and that both that
** copyright notice and this permission notice appear in supporting
** documentation.  This software is provided "as is" without express or
** implied warranty.
*/

#define HASH_FUNC(c) ((((long)c->red * 33023 + \
			(long)c->green * 30013 + \
			(long)c->blue * 27011 ) & 0x7fffffff ) % HASH_SIZE )

void quantizer::build_histogram(mif& image)
{
	hash_t *entry; 
	int hash;
	int x, y;
	color_info *c;

	hash_t_arr table;
	m_max_color = 0;

	for (y = 0; y < image.m_height; y++)
	{
		RGBA* p = image.pos(0, y);
		for (x = 0; x < image.m_width; x++)
		{
			hash = HASH_FUNC(p);
			for (entry = table.getat(hash); ; entry = entry->next())
			{
				if (!entry)
				{
					entry = new hash_t;
					entry->c[0] = p->red;
					entry->c[1] = p->green;
					entry->c[2] = p->blue;
					entry->num = 1;
					table.setat(hash, entry);
					m_max_color++;
					break;			// we've inserted it
				}
				else if ((entry->c[0] == p->red) &&
						 (entry->c[1] == p->green) &&
						 (entry->c[2] == p->blue))
				{
					entry->num++;
					break;			// we've found it
				}
			}
			p++;
		}
	}

	m_colors_arr = (color_info*)malloc(m_max_color * sizeof(color_info));
	for (hash = 0, c = m_colors_arr; hash < HASH_SIZE; hash++)
	{
		for (entry = table.getat(hash); entry != NULL; entry = entry->next(), c++)
		{
			goto_workspace(entry->c, c->c);
			c->num = entry->num;
		}
	}
}

void quantizer::map_hashed(mif& src, sample *lut, int len)
{
	double dist, newdist, cs[3];
	sample *s, *last_s, t, v;
	int i, j, idx, x, y;

	// メモリ一括確保
	double* dist_buffer = (double*)malloc(len * (len + 2) * sizeof(double));
	double** sdist = (double**)dist_buffer;
	double* mdist = dist_buffer + len;
	for (i = 0; i < len; i++)
		sdist[i] = dist_buffer + (i + 2) * len;

	// sort samples according to their number of colors;
	// here we use insertion sort since the number of samples
	// will usually be small

	if (lut[0].num < lut[1].num)
	{
		t = lut[1];
		lut[1] = lut[0];
		lut[0] = t;
	}
	t = lut[0];
	lut[0].num = 20000000;		// sentinel key

	for (i = 1; i < len; i++)
	{
		v = lut[i];
		if (lut[i-1].num < v.num)
		{
			j = i;
			do {
				lut[j] = lut[j-1];
				j--;		// shift up
			} while (lut[j-1].num < v.num);
			lut[j] = v;						// insert v
		}
	}

	// re-insert t; this works without sentinel key, since t is not the largest
	i = 0;
	while (lut[i+1].num > t.num)
	{
		lut[i] = lut[i+1];
		i++;
	}
	lut[i] = t;

	// calculate the squared halfdistances between all centroids (sdist)
	// and the radii of the spheres belonging to each centroid (mdist)
	// these can be used to save some comparisons in nearest-neighbour search
	for (i = 0; i < len; i++)
		mdist[i] = 1E20;

	for (i = 0; i < len; i++)
	{
		sdist[i][i] = 0;
		for (j = i + 1; j < len; j++)
		{
			sdist[j][i] = sdist[i][j] =
				( (lut[i].u[0]-lut[j].u[0])*(lut[i].u[0]-lut[j].u[0])
				+(lut[i].u[1]-lut[j].u[1])*(lut[i].u[1]-lut[j].u[1])
				+(lut[i].u[2]-lut[j].u[2])*(lut[i].u[2]-lut[j].u[2]))/4;
			if (sdist[j][i] < mdist[i])
				mdist[i] = sdist[j][i];
			if (sdist[j][i] < mdist[j])
				mdist[j] = sdist[j][i];
		}
	}

	hash_t_arr table;

	// avoid many malloc's, since we know the number of entries in advance
	last_s = lut + len;

	mif dst(src.m_width, src.m_height, src.m_bits);
	hash_t* entry;
	for (y = 0; y < src.m_height; y++)
	{
		RGBA* ps = src.pos(0, y);
		RGBA* pd = dst.pos(0, y);
		for (x = 0; x < src.m_width; x++)
		{
			int hash = HASH_FUNC(ps);
			idx = -1;
			for (entry = table.getat(hash); entry != NULL; entry = entry->next())
			{
				if ((entry->c[0] == ps->red) &&
					(entry->c[1] == ps->green) &&
					(entry->c[2] == ps->blue))
				{
					idx = entry->num;
					break;
				}
			}
			if (idx < 0)
			{
				dist = 1E20;
				idx = 0;
				goto_workspace((BYTE*)ps, cs);

				for (s = lut, i = 0; s != last_s; s++, i++)
				{
					if (dist >= sdist[idx][i])
					{
						// triangle inequation
						newdist =  ((cs[0] - s->u[0]) * (cs[0] - s->u[0]) + 
									(cs[1] - s->u[1]) * (cs[1] - s->u[1]) + 
									(cs[2] - s->u[2]) * (cs[2] - s->u[2]));
						if (newdist < dist)
						{
							idx = i;
							dist = newdist;
							if (dist < mdist[i])
									break;				// cs in sphere of s->u
						}
					}
				}
				hash_t* store = new hash_t;
				store->c[0] = ps->red;
				store->c[1] = ps->green;
				store->c[2] = ps->blue;
				store->num = idx;
				store->addobj(table.getat(hash));
				table.setat(hash, store);
			}

			pd->red   = lut[idx].ci[0];
			pd->green = lut[idx].ci[1];
			pd->blue  = lut[idx].ci[2];
			ps++;
			pd++;
		}
	}

	free(dist_buffer);
	src.movefrom(&dst);
}

/********************************************************************/
/* auxiliary functions                                              */
/********************************************************************/

double quantizer::calc_axis(int idx, double* axis)
{
	int princ, princ2;
	double Cov[3][3], U[3][3], lambda[3];
	color_info *first, *last, *c;

	/* calculate the covariance matrix of the sample */
	Cov[0][0] = Cov[0][1] = Cov[0][2] = 0;
	Cov[1][1] = Cov[1][2] = Cov[2][2] = 0;

	first = m_colors_arr + m_samples[idx].first;
	last = first + m_samples[idx].num;
	for (c = first; c != last; c++)
	{
		Cov[0][0] += c->num * (c->c[0]*c->c[0]);
		Cov[0][1] += c->num * (c->c[0]*c->c[1]);
		Cov[0][2] += c->num * (c->c[0]*c->c[2]);
		Cov[1][1] += c->num * (c->c[1]*c->c[1]);
		Cov[1][2] += c->num * (c->c[1]*c->c[2]);
		Cov[2][2] += c->num * (c->c[2]*c->c[2]);
	}
	Cov[0][0] = (Cov[0][0]/m_samples[idx].weight)-(m_samples[idx].u[0]*m_samples[idx].u[0]);
	Cov[0][1] = (Cov[0][1]/m_samples[idx].weight)-(m_samples[idx].u[0]*m_samples[idx].u[1]);
	Cov[0][2] = (Cov[0][2]/m_samples[idx].weight)-(m_samples[idx].u[0]*m_samples[idx].u[2]);
	Cov[1][1] = (Cov[1][1]/m_samples[idx].weight)-(m_samples[idx].u[1]*m_samples[idx].u[1]);
	Cov[1][2] = (Cov[1][2]/m_samples[idx].weight)-(m_samples[idx].u[1]*m_samples[idx].u[2]);
	Cov[2][2] = (Cov[2][2]/m_samples[idx].weight)-(m_samples[idx].u[2]*m_samples[idx].u[2]);
	Cov[1][0] = Cov[0][1]; Cov[2][0] = Cov[0][2]; Cov[2][1] = Cov[1][2]; 

	/* do a spectral decomposition of Cov */
	spectral(Cov, U, lambda);

	/* find the largest eigenvalue, and the corr. eigenvector */
	if (lambda[0] < 0) { lambda[0] = - lambda[0]; }
	if (lambda[1] < 0) { lambda[1] = - lambda[1]; }
	if (lambda[2] < 0) { lambda[2] = - lambda[2]; }

	princ = 0;
	if (lambda[1] > lambda[0]) { princ = 1; }
	if (lambda[2] > lambda[princ]) { princ = 2; }
	princ2 = (princ+1) % 3; 
	if (lambda[(princ+2) % 3] > lambda[princ2]) { princ2 = (princ+2) % 3; }

	axis[0] = U[0][princ];
	axis[1] = U[1][princ];
	axis[2] = U[2][princ];

	if (lambda[princ2] != 0)
		return lambda[princ]/lambda[princ2];
	return 1E20;		/* infinity */
}

void quantizer::calc_centroid(int idx)
{
	sample *s;
	color_info *first, *last, *c;

	s = m_samples + idx;
	s->u[0] = s->u[1] = s->u[2] = 0;
	s->weight = 0;
	first = m_colors_arr + s->first; 
	last = first + s->num;
	for (c = first; c != last; c++)
	{
		s->u[0] += c->num * c->c[0];
		s->u[1] += c->num * c->c[1];
		s->u[2] += c->num * c->c[2];
		s->weight += c->num;
	}
	s->u[0] /= s->weight;
	s->u[1] /= s->weight;
	s->u[2] /= s->weight;
}

void quantizer::calc_error(int idx)
{
  sample *s;
  color_info *first, *last, *c;

  s = m_samples + idx;
  s->error = 0;
  first = m_colors_arr + s->first;
  last = first + s->num;
  for (c = first; c != last; c++) {
    s->error += c->num * (  (c->c[0] - s->u[0]) * (c->c[0] - s->u[0])
			  + (c->c[1] - s->u[1]) * (c->c[1] - s->u[1])
			  + (c->c[2] - s->u[2]) * (c->c[2] - s->u[2]) );
  }
  s->error *= s->weight;
}

/* 
 * Elaborate quicksort a la Segewick, Algorithms, Chapter 9.
 */

void quantizer::sort_along_axis(double axis[], int start, int num)
{
  int i, j, l, r;
  color_info *first, *last, *c, v, t;
  int top, stack[60]; /* much more than enough */

  if (num>1) {
    /* projecting onto axis */
    first = m_colors_arr + start;
    last = first + num;
    for (c = first; c != last; c++) {
      c->key = c->c[0]*axis[0] + c->c[1]*axis[1] + c->c[2]*axis[2];
    }

    /* quicksort */
    l = start; r = start + num - 1;
    top = 0;
    stack[top] = l; top++;
    stack[top] = r; top++;
    while (top > 0) { /* stack empty <==> top==0 */
      if (r-l>10) { /* M==10 is just a guess, Sedgewick recommends 5 < M < 25 */
	/* sort m_colors_arr[l], m_colors_arr[(r+l)/2] and m_colors_arr[r] */
	i = (r+l)/2;
	if (m_colors_arr[l].key > m_colors_arr[i].key) { 
	  t = m_colors_arr[i]; m_colors_arr[i] = m_colors_arr[l]; m_colors_arr[l] = t; 
	}
	if (m_colors_arr[l].key > m_colors_arr[r].key) { 
	  t = m_colors_arr[r]; m_colors_arr[r] = m_colors_arr[l]; m_colors_arr[l] = t; 
	}
	if (m_colors_arr[i].key > m_colors_arr[r].key) { 
	  t = m_colors_arr[r]; m_colors_arr[r] = m_colors_arr[i]; m_colors_arr[i] = t; 
	}
	
	/* exchange m_colors_arr[(r+l)/2] and m_colors_arr[r-1] and make v == m_colors_arr[r-1] */
	v = m_colors_arr[i]; m_colors_arr[i] = m_colors_arr[r-1]; m_colors_arr[r-1] = v;
	
	/* i = partition(l+1,r-1) */
	i = l; j = r-1;
	do {
	  do { i++; } while (v.key > m_colors_arr[i].key);
	  do { j--; } while (v.key < m_colors_arr[j].key);
	  t = m_colors_arr[i]; m_colors_arr[i] = m_colors_arr[j]; m_colors_arr[j] = t;
	} while (i<j);
	m_colors_arr[j] = m_colors_arr[i]; m_colors_arr[i] = m_colors_arr[r-1]; m_colors_arr[r-1] = t;
	
	/* recursion: put the larger part on the stack */
	if (i-l>r-i) {
	  stack[top] = l; top++;
	  stack[top] = i-1; top++;
	  l = i+1;
	} else {
	  stack[top] = i+1; top++;
	  stack[top] = r; top++;
	  r = i-1;
	}
      } else {
	top--; r = stack[top];
	top--; l = stack[top];
      }
    } /* while */
    
    /* insertion sort */
    /* make sure colors[start] is not the largest element */
    if (m_colors_arr[start].key > m_colors_arr[start+1].key) { 
      t = m_colors_arr[start+1]; m_colors_arr[start+1] = m_colors_arr[start]; m_colors_arr[start] = t; 
    }
    t = m_colors_arr[start]; m_colors_arr[start].key = -1E20; /* sentinel key */

    for (i = start + 1; i < start + num; i++) {
      v = m_colors_arr[i];
      if (m_colors_arr[i-1].key > v.key) {
	j = i;
	do {
	  m_colors_arr[j] = m_colors_arr[j-1]; j--; /* shift up */
	} while (m_colors_arr[j-1].key > v.key);
	m_colors_arr[j] = v; /* insert v */
      }
    }
    /* re-insert t; this works without sentinel key, since t is not the largest */
    i = start;
    while (m_colors_arr[i+1].key < t.key) {
      m_colors_arr[i] = m_colors_arr[i+1]; i++;
    }
    m_colors_arr[i] = t;
  } /* if more than one element */
}

/********************************************************************/
/* binary splitting                                                 */
/********************************************************************/

void quantizer::binary_split(int idx)
{
	int xi, convergence, it;
	int acc_num, opt_num;
	double max[3], acc[3], opt[3], normal[3], ofs, cofs, value, val_i, axis[3];
	color_info *c, ctmp, *first, *last;
	sample *s, *c1, *c2;

	s = m_samples + idx;

	if (s->num == 1)
	{
		s->status = FINALLEAF; /* a leaf that can not be split any more */
		return; 
	}

	// find free samples for the children of s
	for (s->c1 = 0; s->c1 < m_max_sample; s->c1++)
	{
		if (m_samples[s->c1].status == UNUSED)
			break;
	}
	c1 = m_samples + s->c1; 
	c1->status = PRELEAF;

	for (s->c2 = 0; s->c2 < m_max_sample; s->c2++)
	{
		if (m_samples[s->c2].status == UNUSED)
			break;
	}
	c2 = m_samples + s->c2; 
	c2->status = PRELEAF;

	// sort along the principal axis of s
	calc_axis(idx, axis);
	sort_along_axis(axis, s->first, s->num);

	max[0] = s->u[0] * s->weight;
	max[1] = s->u[1] * s->weight;
	max[2] = s->u[2] * s->weight;

	first = m_colors_arr + s->first;
	last = first + s->num;

	xi = 0;
	opt_num = 0;
	opt[0] = opt[1] = opt[2] = 0;
	value = 0;
	acc_num = 0;
	acc[0] = acc[1] = acc[2] = 0;

	c = first;
	while (1)
	{
		acc[0] += c->num * c->c[0];
		acc[1] += c->num * c->c[1];
		acc[2] += c->num * c->c[2];
		acc_num += c->num;
		c++;
		if (c == last)
			break;
		val_i = ((acc[0]*acc[0] + acc[1]*acc[1] + acc[2]*acc[2]) / acc_num
					+ (  (max[0] - acc[0])*(max[0] - acc[0])
					+ (max[1] - acc[1])*(max[1] - acc[1])
					+ (max[2] - acc[2])*(max[2] - acc[2]) ) 
					/ (s->weight - acc_num));
		if (value < val_i)
		{
			value = val_i;
			xi = c - first;
			opt_num = acc_num;
			opt[0] = acc[0]; opt[1] = acc[1]; opt[2] = acc[2];
		}
	}

	c1->first = s->first;
	c1->num = xi; 
	c1->weight = opt_num;

	c1->u[0] = opt[0] / c1->weight;
	c1->u[1] = opt[1] / c1->weight;
	c1->u[2] = opt[2] / c1->weight;

	c2->first = c1->first + c1->num;
	c2->num = s->num - c1->num;
	c2->weight = s->weight - c1->weight;

	c2->u[0] = (max[0] - opt[0]) / c2->weight;
	c2->u[1] = (max[1] - opt[1]) / c2->weight;
	c2->u[2] = (max[2] - opt[2]) / c2->weight;

	if (m_kmeans)
	{
		// adaptive 2-clustering
		it = 0;
		ofs = 0;		// avoid compiler warning
		convergence = 0;
		do {
			it++; 
			if (it == m_max_iterations)
				break;
			c = first;
			while (c != last)
			{
				if (convergence == 0)
				{
					// find the plane normal of the separating plane
					normal[0] = c2->u[0] - c1->u[0];
					normal[1] = c2->u[1] - c1->u[1];
					normal[2] = c2->u[2] - c1->u[2];

					// find the offset of the halfway vector between the centroids
					ofs = (  (c1->u[0] + c2->u[0])*normal[0]
								+ (c1->u[1] + c2->u[1])*normal[1]
								+ (c1->u[2] + c2->u[2])*normal[2] ) / 2;
				}
				cofs = c->c[0]*normal[0] + c->c[1]*normal[1] + c->c[2]*normal[2];
				if ((c < m_colors_arr + c2->first) && (cofs > ofs) && (c1->num > 1))
				{
					convergence = 0;

					// update the samples
					c1->u[0] = (c1->weight*c1->u[0] - c->num*c->c[0])/(c1->weight - c->num);
					c1->u[1] = (c1->weight*c1->u[1] - c->num*c->c[1])/(c1->weight - c->num);
					c1->u[2] = (c1->weight*c1->u[2] - c->num*c->c[2])/(c1->weight - c->num);
					c2->u[0] = (c2->weight*c2->u[0] + c->num*c->c[0])/(c2->weight + c->num);
					c2->u[1] = (c2->weight*c2->u[1] + c->num*c->c[1])/(c2->weight + c->num);
					c2->u[2] = (c2->weight*c2->u[2] + c->num*c->c[2])/(c2->weight + c->num);
					c1->weight -= c->num;
					c2->weight += c->num;

					// move c from c1 to c2
					c1->num--;
					c2->first--;
					c2->num++;
					ctmp = *c;
					*c = m_colors_arr[c2->first];
					m_colors_arr[c2->first] = ctmp;
					// do NOT increase c, we still have to check that color
				}
				else if ((c >= m_colors_arr + c2->first) && (cofs < ofs) && (c2->num > 1))
				{
					convergence = 0;

					// update the samples
					c1->u[0] = (c1->weight*c1->u[0] + c->num*c->c[0])/(c1->weight + c->num);
					c1->u[1] = (c1->weight*c1->u[1] + c->num*c->c[1])/(c1->weight + c->num);
					c1->u[2] = (c1->weight*c1->u[2] + c->num*c->c[2])/(c1->weight + c->num);
					c2->u[0] = (c2->weight*c2->u[0] - c->num*c->c[0])/(c2->weight - c->num);
					c2->u[1] = (c2->weight*c2->u[1] - c->num*c->c[1])/(c2->weight - c->num);
					c2->u[2] = (c2->weight*c2->u[2] - c->num*c->c[2])/(c2->weight - c->num);
					c1->weight += c->num;
					c2->weight -= c->num;

					// move c from c2 to c1
					ctmp = *c;
					*c = m_colors_arr[c2->first];
					m_colors_arr[c2->first] = ctmp;
					c1->num++;
					c2->first++;
					c2->num--;
					// do NOT increase c, we still have to check that color
				}
				else
				{
					c++;
					convergence++;
				}							// this point is ok or can't be moved
			}								// loop over all colors
		} while (convergence < s->num);		// until cycle w/o move completed
	}										// if kmeans allowed

	calc_error(s->c1);
	calc_error(s->c2);
	s->gain = s->error - c1->error - c2->error;
}

/********************************************************************/
/* splitting along the principal axis                               */
/********************************************************************/

#define Wrs(a,r,s) (W_rs[((r)<<1)+(s)])[a]
#define W2(a) (Wrs(a,0,0) + Wrs(a,1,1) + Wrs(a,2,2))
#define EEE(a,b) (   W2(b) - W2(a) \
    - (  (W1[0][b] - W1[0][a])*(W1[0][b] - W1[0][a]) \
       + (W1[1][b] - W1[1][a])*(W1[1][b] - W1[1][a]) \
       + (W1[2][b] - W1[2][a])*(W1[2][b] - W1[2][a]) ) / (W0[b] - W0[a]) )
#define COV(a,b,r,s) ( (Wrs(b,r,s) - Wrs(a,r,s)) / (W0[b] - W0[a]) \
  - ((W1[r][b] - W1[r][a])*(W1[s][b] - W1[s][a])) / \
    ((W0[b] - W0[a])*(W0[b] - W0[a])) )

int quantizer::principal_split(int resolution, int additional)
{
  int i, j, k, go_on, princ, princ2, biased, add;
  int t, tt, cut, n, **L, *q, *W0;
  double *E, *W1[3], *W_rs[7], et, e;
  double deviation, Cov[3][3], U[3][3], lambda[3];
  double axis[3], bias, step, key;
  color_info *last, *c, ctmp;
  double *normal[3], *ofs, cofs0, cofs1;
  int convergence, it;

	double max_deviation = cos(DEG_to_RAD(15));	// tolerate 15 degree
	double min_bias = 1.1;						// minimal ratio beween largest eigenvalues

  k = 1;
  calc_centroid (0);
  bias = calc_axis (0, axis);
  biased = (bias > min_bias);
  if (biased) {
    sort_along_axis (axis, 0, m_max_color);

    /* room for optimal principal quantizer */
    q = (int *) malloc ((m_K + 1) * sizeof (int));  
    q[0] = 0;
    
    /* calculate the cumulative moments */
    W0 = (int *) malloc ((resolution + 1) * sizeof (int));
    W1[0]  = (double *) malloc ((resolution + 1) * sizeof (double));
    W1[1]  = (double *) malloc ((resolution + 1) * sizeof (double));
    W1[2]  = (double *) malloc ((resolution + 1) * sizeof (double));
    W_rs[0] = (double *) malloc ((resolution + 1) * sizeof (double));
    W_rs[1] = (double *) malloc ((resolution + 1) * sizeof (double));
    W_rs[2] = (double *) malloc ((resolution + 1) * sizeof (double));
    W_rs[3] = (double *) malloc ((resolution + 1) * sizeof (double));
    W_rs[4] = (double *) malloc ((resolution + 1) * sizeof (double));
    W_rs[6] = (double *) malloc ((resolution + 1) * sizeof (double));
    W0[0] = 0;
    W1[0][0] = W1[1][0] = W1[2][0] = 0;
    Wrs(0,0,0) = Wrs(0,0,1) = Wrs(0,0,2) = 0;
    Wrs(0,1,1) = Wrs(0,1,2) = Wrs(0,2,2) = 0;
    c = m_colors_arr;
    last = m_colors_arr + m_max_color;
    step = (m_colors_arr[m_max_color-1].key - c->key) / (resolution - 0.5);
    key = c->key - 0.25*step;
    for (i = 1; i <= resolution; i++) {
      W0[i] = W0[i-1]; 
      W1[0][i] = W1[0][i-1];
      W1[1][i] = W1[1][i-1];
      W1[2][i] = W1[2][i-1];
      Wrs(i,0,0) = Wrs(i-1,0,0); 
      Wrs(i,0,1) = Wrs(i-1,0,1); 
      Wrs(i,0,2) = Wrs(i-1,0,2);
      Wrs(i,1,1) = Wrs(i-1,1,1);
      Wrs(i,1,2) = Wrs(i-1,1,2);
      Wrs(i,2,2) = Wrs(i-1,2,2);
      key += step;
      while (c->key <= key) {
	W0[i] += c->num; 
	W1[0][i] += c->num * (c->c[0]);
	W1[1][i] += c->num * (c->c[1]);
	W1[2][i] += c->num * (c->c[2]);
	Wrs(i,0,0) += c->num * (c->c[0] * c->c[0]); 
	Wrs(i,0,1) += c->num * (c->c[0] * c->c[1]); 
	Wrs(i,0,2) += c->num * (c->c[0] * c->c[2]);
	Wrs(i,1,1) += c->num * (c->c[1] * c->c[1]); 
	Wrs(i,1,2) += c->num * (c->c[1] * c->c[2]); 
	Wrs(i,2,2) += c->num * (c->c[2] * c->c[2]);     
	c++;
	if (c == last) break;
      }
    }

    /* initialize the deviations */
    E = (double *) malloc ((resolution + 1) * sizeof (double));
    E[0] = 0;
    for (i = 1; i <= resolution; i++) { E[i] = EEE(0,i); }
    
    /* Initialize the L's. 
       We allocate L[][] incrementally, since we don't know the 
       length of the optimal principal quantizer in advance. */
    L = (int **) malloc ((m_K + 1) * sizeof (int *));
    
    /* dynamic programming */
    add = 0; /* counter for additional smoothness splits */
    do {
      k++;
      L[k] = (int *) malloc ((resolution + 1) * sizeof (int));
      L[k][k] = k - 1;
      for (n = resolution; n > k; n--) {
	cut = n; 
	do { cut--; } while (W0[n] == W0[cut]); /* we want to cut something off! */
	e = E[cut] + EEE(cut,n);
 	t = cut;
	do {
	  tt = t;
	  do { t--; } while (W0[tt] == W0[t]); /* we want a different cut */
	  if (t < k - 1) { break; } 
	  et = EEE(t,n);
	  if (e < et) { break; } 
	  et += E[t];
	  if (et < e) { cut = t; e = et; } 
	} while (1); 
	L[k][n] = cut; E[n] = e;
      }
      
      /* determine the k-1 optimal splitting points */      
      q[k] = resolution;
      for (j = k - 1; j >= 1; j--) {
	q[j] = L[j+1][q[j+1]];
      }
      
      go_on = 0;
      if (k < m_K) {
	if (biased) {
	  biased = 0;
	  /* calc the axes, check the bias */
	  for (j = 0; j < k; j++) {
	    Cov[0][0] = COV(q[j],q[j+1],0,0);
	    Cov[1][1] = COV(q[j],q[j+1],1,1);
	    Cov[2][2] = COV(q[j],q[j+1],2,2);     
	    Cov[0][1] = Cov[1][0] = COV(q[j],q[j+1],0,1);
	    Cov[0][2] = Cov[2][0] = COV(q[j],q[j+1],0,2);
	    Cov[1][2] = Cov[2][1] = COV(q[j],q[j+1],1,2);      
	  
	    /* do a spectral decomposition of Cov */
	    spectral (Cov, U, lambda);
	    
	    /* find the largest eigenvalue, and the corr. eigenvector */
	    if (lambda[0] < 0) lambda[0] = - lambda[0];
	    if (lambda[1] < 0) lambda[1] = - lambda[1];
	    if (lambda[2] < 0) lambda[2] = - lambda[2];
	    
	    princ = 0;
	    if (lambda[1] > lambda[0]) princ = 1;
	    if (lambda[2] > lambda[princ]) princ = 2;
	    princ2 = (princ+1) % 3; 
	    if (lambda[(princ+2) % 3] > lambda[princ2]) princ2 = (princ+2) % 3; 
	    
	    deviation = axis[0]*U[0][princ]+axis[1]*U[1][princ]+axis[2]*U[2][princ];
	    if (deviation < 0) deviation = -deviation; 
	    
	    if ((lambda[princ] > min_bias*lambda[princ2]) && 
		(deviation > max_deviation)) {
	      biased = 1;  /* look for more splits */
	      break;  /* no need to look at more axes */
	    }
	  } /* check bias loop */
	} /* if was biased */
	if (biased) {
	  go_on = 1;
	} else {
	  add++;
	  go_on = (add < additional);
	}
      } /* if more leafs needed */
    } while (go_on);   /* still biased ? */
      

    /* free storage for dynamic programming */
    for (j = 2; j <= k; j++) { free (L[j]); }
    free (L); 
    free (E); 
    free (W0);
    free (W1[0]); free (W1[1]); free (W1[2]);
    free (W_rs[0]); free (W_rs[1]); free (W_rs[2]);
    free (W_rs[3]); free (W_rs[4]); free (W_rs[6]);
    
    /* splitting at the positions stored in q */
    c = m_colors_arr; 
    key = c->key - 0.25*step;
    for (j = 0; j < k; j++) {
      m_samples[j].status = LEAF;   
      m_samples[j].first = c - m_colors_arr;
      m_samples[j].num = 0;
      m_samples[j].weight = 0;
      m_samples[j].u[0] = 0;
      m_samples[j].u[1] = 0;
      m_samples[j].u[2] = 0;
      key += (q[j+1] - q[j]) * step; 
      while ((c!= last) && (c->key <= key)) {
	m_samples[j].num++;
	m_samples[j].weight += c->num;
	m_samples[j].u[0] += c->num * c->c[0];
	m_samples[j].u[1] += c->num * c->c[1];
	m_samples[j].u[2] += c->num * c->c[2];
	c++;
      }
      
      m_samples[j].u[0] /= m_samples[j].weight;
      m_samples[j].u[1] /= m_samples[j].weight;
      m_samples[j].u[2] /= m_samples[j].weight;
    }
    free (q);
    
    if (m_kmeans) {
      /* adaptive k-clustering */
      normal[0] = (double *) malloc ((k + 1) * sizeof (double));
      normal[1] = (double *) malloc ((k + 1) * sizeof (double));
      normal[2] = (double *) malloc ((k + 1) * sizeof (double));
      ofs = (double *) malloc ((k + 1) * sizeof (double));
      
      for (i = 1; i < k; i++) {
	normal[0][i] = m_samples[i].u[0] - m_samples[i-1].u[0];
	normal[1][i] = m_samples[i].u[1] - m_samples[i-1].u[1];
	normal[2][i] = m_samples[i].u[2] - m_samples[i-1].u[2];
	ofs[i] = (  (m_samples[i-1].u[0] + m_samples[i].u[0])*normal[0][i]	
		  + (m_samples[i-1].u[1] + m_samples[i].u[1])*normal[1][i]
		  + (m_samples[i-1].u[2] + m_samples[i].u[2])*normal[2][i] )/2;
      }
      ofs[0] = -1; ofs[k] = 1;
      normal[0][0] = normal[1][0] = normal[2][0] = 0;
      normal[0][k] = normal[1][k] = normal[2][k] = 0;
      
      it = 0;
      convergence = 0;
      do {
	it++; 
	if (it == m_max_iterations) { break; }
	i = 0; c = m_colors_arr;
	while (c != last) {
	  if (m_samples[i].num > 1) {
	    /* this sample can get smaller */
	    cofs0=c->c[0]*normal[0][i] + c->c[1]*normal[1][i] + c->c[2]*normal[2][i];
	    cofs1=c->c[0]*normal[0][i+1]+c->c[1]*normal[1][i+1]+c->c[2]*normal[2][i+1];
	    if ((cofs0 < ofs[i]) || (cofs1 > ofs[i+1])) {
	      convergence = 0;
	      if (cofs0 < ofs[i]) {
		/* update samples i and i - 1 */
		m_samples[i].u[0] =((m_samples[i].weight*m_samples[i].u[0]-c->num*c->c[0])
				  /(m_samples[i].weight - c->num));
		m_samples[i].u[1] =((m_samples[i].weight*m_samples[i].u[1]-c->num*c->c[1])
				  /(m_samples[i].weight - c->num));
		m_samples[i].u[2] =((m_samples[i].weight*m_samples[i].u[2]-c->num*c->c[2])
				  /(m_samples[i].weight - c->num));
		m_samples[i-1].u[0]=((m_samples[i-1].weight*m_samples[i-1].u[0]+c->num*c->c[0])
				   /(m_samples[i-1].weight + c->num));
		m_samples[i-1].u[1]=((m_samples[i-1].weight*m_samples[i-1].u[1]+c->num*c->c[1])
				   /(m_samples[i-1].weight + c->num));
		m_samples[i-1].u[2]=((m_samples[i-1].weight*m_samples[i-1].u[2]+c->num*c->c[2])
				   /(m_samples[i-1].weight + c->num));
		m_samples[i].weight -= c->num;
		m_samples[i-1].weight += c->num;
		/* move c to sample i - 1 */
		ctmp=*c;*c=m_colors_arr[m_samples[i].first];m_colors_arr[m_samples[i].first]=ctmp;
		m_samples[i-1].num++; m_samples[i].num--; m_samples[i].first++; 
	      } else {
		/* update samples i and i + 1 */
		m_samples[i].u[0] = (  (m_samples[i].weight*m_samples[i].u[0] - c->num*c->c[0])
				   / (m_samples[i].weight - c->num));
		m_samples[i].u[1] = (  (m_samples[i].weight*m_samples[i].u[1] - c->num*c->c[1])
				   / (m_samples[i].weight - c->num));
		m_samples[i].u[2] = (  (m_samples[i].weight*m_samples[i].u[2] - c->num*c->c[2])
				   / (m_samples[i].weight - c->num));
		m_samples[i+1].u[0]=((m_samples[i+1].weight*m_samples[i+1].u[0]+c->num*c->c[0])
				   /(m_samples[i+1].weight + c->num));
		m_samples[i+1].u[1]=((m_samples[i+1].weight*m_samples[i+1].u[1]+c->num*c->c[1])
				   /(m_samples[i+1].weight + c->num));
		m_samples[i+1].u[2]=((m_samples[i+1].weight*m_samples[i+1].u[2]+c->num*c->c[2])
				   /(m_samples[i+1].weight + c->num));
		m_samples[i].weight -= c->num;
		m_samples[i+1].weight += c->num;
		/* move c to sample i + 1 */
		m_samples[i].num--; m_samples[i+1].num++; m_samples[i+1].first--; 
		ctmp=*c; *c=m_colors_arr[m_samples[i+1].first]; m_colors_arr[m_samples[i+1].first]=ctmp;
	      }
	      if (i > 0) {
		/* update normal and offset i */
		normal[0][i] = m_samples[i].u[0] - m_samples[i-1].u[0];
		normal[1][i] = m_samples[i].u[1] - m_samples[i-1].u[1];
		normal[2][i] = m_samples[i].u[2] - m_samples[i-1].u[2];
		ofs[i] = (  (m_samples[i-1].u[0] + m_samples[i].u[0])*normal[0][i]
			  + (m_samples[i-1].u[1] + m_samples[i].u[1])*normal[1][i]
			  + (m_samples[i-1].u[2] + m_samples[i].u[2])*normal[2][i] )/2;
	      }
	      if (i + 1 < k) {
		/* update normal and offset i + 1 */	  
		normal[0][i+1] = m_samples[i+1].u[0] - m_samples[i].u[0];
		normal[1][i+1] = m_samples[i+1].u[1] - m_samples[i].u[1];
		normal[2][i+1] = m_samples[i+1].u[2] - m_samples[i].u[2];
		ofs[i+1] = (  (m_samples[i].u[0] + m_samples[i+1].u[0])*normal[0][i+1]
			    + (m_samples[i].u[1] + m_samples[i+1].u[1])*normal[1][i+1]
			    + (m_samples[i].u[2] + m_samples[i+1].u[2])*normal[2][i+1] )/2;
	      }
	      /* do NOT increase c, we still have to check that color */
	      /* avoid the following test in the outer loop */
	      if (c < m_colors_arr + m_samples[i].first) { i--; } 
	    } else { convergence++; c++; } /* c is ok */
	  } else { convergence++; c++; } /* single-color sample */
	  /* update i */
	  if (m_colors_arr + m_samples[i].first + m_samples[i].num <= c) { i++; } 
	}
      } while (convergence < m_max_color);
      
      free (normal[0]);
      free (normal[1]);
      free (normal[2]);
      free (ofs);
    } /* if kmeans allowed */
  } /* if initially biased */  
  return k; /* return the number of leafs */
}

void quantizer::quant(mif& image, int num_colors, BOOL cie_luv, BOOL principal)
{
	int i, j, splitting, leafs;
	double gain;

	/* Presets */
	/* basic */
	m_K = num_colors;
	m_rgb = cie_luv ? 0 : 1;
	m_luv = cie_luv ? 1 : 0;

	if (m_luv)
		init_CIE();

	build_histogram(image);

	m_max_sample = 4 * m_K;		// more than enough
	m_samples = (sample*)malloc(m_max_sample * sizeof(sample));

	/* create the root sample */	
	m_samples[0].status = LEAF;
	m_samples[0].first = 0;
	m_samples[0].num = m_max_color;
	leafs = 1;

	/* init the other samples */
	for (i = 1; i < m_max_sample; i++)
		m_samples[i].status = UNUSED;

	if (principal)
	{
		// do the split along the principal axis
		int resolution = 1024;
		int additional = 0;
		leafs = principal_split(resolution, additional);
	}
	else
		calc_centroid(0);

	// The gimp_progress_update was done in principal_split().
	for (i = 0; i < leafs; i++)
	{
		calc_error(i);
		binary_split(i);
	}

	if (leafs < m_K)
	{
		// additional split loop
		while (1)
		{
			// find splitting sample (we implement the max-gain splitting strategy)
			splitting = -1;
			gain = 0;
			for (i = 0; i < m_max_sample; i++)
			{
				if ((m_samples[i].status == LEAF) && (m_samples[i].gain > gain))
				{
					splitting = i;
					gain = m_samples[i].gain;
				}
			}
			if (splitting < 0)
				break;						// cannot split any more

			// split the selected sample
			m_samples[splitting].status = UNUSED;
			m_samples[m_samples[splitting].c1].status = LEAF;
			m_samples[m_samples[splitting].c2].status = LEAF;
			leafs++;

			if (leafs >= m_K)
				break;						// enough leafs found

			binary_split(m_samples[splitting].c1);
			binary_split(m_samples[splitting].c2);
		}									// end of splitting loop
	}

	// bring the leafs to the top of the samples array;
	// transform their centroids to image space
	j = m_max_sample;
	for (i = 0; i < leafs; i++)
	{
		if (m_samples[i].status < LEAF)
		{
			do {
				j--;
				if (m_samples[j].status >= LEAF)
				{
					m_samples[i] = m_samples[j];
					m_samples[j].status = UNUSED;
					break;
				}
			} while (j > i);
		}
		goto_imagespace(m_samples[i].u, m_samples[i].ci);
	}
	map_hashed(image, m_samples, leafs);
}

void quantize_simple(mif& image, int num_colors, BOOL cie_luv, BOOL principal)
{
	quantizer qt;
	qt.quant(image, num_colors, cie_luv, principal);
}
