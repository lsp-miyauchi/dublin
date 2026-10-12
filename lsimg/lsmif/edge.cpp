////////////////////////////////////////////////////////////////////////
// edge.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"

static long long_sqrt(long n)
{
#define lsqrt_max4pow (1UL << 30)
  /* lsqrt_max4pow is the (machine-specific) largest power of 4 that can
   * be represented in an unsigned long.
   *
   * Compute the integer square root of the integer argument n
   * Method is to divide n by x computing the quotient x and remainder r
   * Notice that the divisor x is changing as the quotient x changes
   * 
   * Instead of shifting the dividend/remainder left, we shift the
   * quotient/divisor right. The binary point starts at the extreme
   * left, and shifts two bits at a time to the extreme right.
   * 
   * The residue contains n-x^2. (Within these comments, the ^ operator
   * signifies exponentiation rather than exclusive or. Also, the /
   * operator returns fractions, rather than truncating, so 1/4 means
   * one fourth, not zero.)
   * 
   * Since (x + 1/2)^2 == x^2 + x + 1/4,
   * n - (x + 1/2)^2 == (n - x^2) - (x + 1/4)
   * Thus, we can increase x by 1/2 if we decrease (n-x^2) by (x+1/4)
   */

  unsigned long residue;        /* n - x^2  */
  unsigned long root;           /* x + 1/4  */
  unsigned long half;           /* 1/2      */

  residue = n;                  /* n - (x = 0)^2, with suitable alignment */

  /*
   * if the correct answer fits in two bits, pull it out of a magic hat
   */
  if (residue <= 12)
    return (0x03FFEA94 >> (residue *= 2)) & 3;

  root = lsqrt_max4pow;         /* x + 1/4, shifted all the way left */
  /* half = root + root; 1/2, shifted likewise */

  /* 
   * Unwind iterations corresponding to leading zero bits 
   */
  while (root > residue)
    root >>= 2;

  /*
   * Unwind the iteration corresponding to the first one bit
   * Operations have been rearranged and combined for efficiency
   * Initialization of half is folded into this iteration
   */
  residue -= root;              /* Decrease (n-x^2) by (0+1/4)             */
  half = root >> 2;             /* 1/4, with binary point shifted right 2  */
  root += half;                 /* x=1. (root is now (x=1)+1/4.)           */
  half += half;                 /* 1/2, properly aligned                   */

  /*
   * Normal loop (there is at least one iteration remaining)
   */
  do
    {
      if (root <= residue)      /* Whenever we can,                          */
        {
          residue -= root;      /* decrease (n-x^2) by (x+1/4)               */
          root += half;         /* increase x by 1/2                         */
        }
      half >>= 2;               /* Shift binary point 2 places right          */
      root -= half;             /* x{ +1/2 } +1/4 - 1/8 == x { +1/2 } 1/8     */
      root >>= 1;               /* 2x{ +1 } +1/4, shifted right 2 places      */
    }
  while (half);                 /* When 1/2 == 0, bin. point is at far right  */

  /* 
   * round up if (x+1/2)^2 < n
   */
  if (root < residue)
    ++root;

  /* 
   * Guaranteed to be correctly rounded (or truncated)
   */
  return root;
}

static _inline RGBA* get_pixel(mif& image, int x, int y)
{
	return image.pos(max(0, min(x, image.m_width - 1)), max(0, min(y, image.m_height - 1)));
}

BOOL edge(mif& src, LPRECT prc, double amount)
{
	// amount is 1.0 ... 20.0 range

	CRect rc;
	if (!src.validaterect(prc, &rc))
		return FALSE;

	src.m_bits = 24;
	mif dst;
	dst.create(rc.Width(), rc.Height(), src.m_bits);

	if (amount < 1.0)
		amount = 1.0;
	long scale = (long)((10 << 16) / amount);

	parallel_for (rc.top, rc.bottom, [&](int y) {
		RGBA* pd = dst.pos(0, y - rc.top);
		for (int x = rc.left; x < rc.right; x++) {
			RGBA* pix00 = get_pixel(src, x-1, y-1);
			RGBA* pix10 = get_pixel(src, x  , y-1);
			RGBA* pix20 = get_pixel(src, x+1, y-1);
			RGBA* pix01 = get_pixel(src, x-1, y  );
			RGBA* pix21 = get_pixel(src, x+1, y  );
			RGBA* pix02 = get_pixel(src, x-1, y+1);
			RGBA* pix12 = get_pixel(src, x  , y+1);
			RGBA* pix22 = get_pixel(src, x+1, y+1);

			long sum1 = (pix20->red - pix00->red) +
								2 * (pix21->red - pix01->red) +
								(pix22->red - pix20->red);
			long sum2 = (pix02->red - pix00->red) +
								2 * (pix12->red - pix10->red) +
								(pix22->red - pix20->red);
			long sum_r = long_sqrt(sum1 * sum1 + sum2 * sum2);
			sum_r = (sum_r * scale) >> 16;		// arbitrary scaling factor
			pd->red = ADJRGB(sum_r);

			sum1 = (pix20->green - pix00->green) +
								2 * (pix21->green - pix01->green) +
								(pix22->green - pix20->green);
			sum2 = (pix02->green - pix00->green) +
								2 * (pix12->green - pix10->green) +
								(pix22->green - pix20->green);
			long sum_g = long_sqrt(sum1 * sum1 + sum2 * sum2);
			sum_g = (sum_g * scale) >> 16;		// arbitrary scaling factor
			pd->green = ADJRGB(sum_g);

			sum1 = (pix20->blue - pix00->blue) +
								2 * (pix21->blue - pix01->blue) +
								(pix22->blue - pix20->blue);
			sum2 = (pix02->blue - pix00->blue) +
								2 * (pix12->blue - pix10->blue) +
								(pix22->blue - pix20->blue);
			long sum_b = long_sqrt(sum1 * sum1 + sum2 * sum2);
			sum_b = (sum_b * scale) >> 16;		// arbitrary scaling factor
			pd->blue = ADJRGB(sum_b);

			pd++;
		}
	});
	src.blt(rc.left, rc.top, rc.Width(), rc.Height(), &dst, 0, 0);
	return TRUE;
}
