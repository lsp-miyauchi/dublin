///////////////////////////////////////////////////////////////////////
// noise.cpp

#include "stdafx.h"
#include "lsmif.h"
#include "common.h"
#include <stdlib.h>

#define NoiseEpsilon				1.0e-5
#define NoiseMask					0x7fff
#define SigmaUniform				4.0
#define SigmaGaussian				4.0
#define SigmaImpulse				0.10
#define SigmaLaplacian				10.0
#define SigmaMultiplicativeGaussian	0.5
#define SigmaPoisson				0.05
#define TauGaussian					20.0

static BYTE __fastcall gen_noise(BYTE pixel, UINT noise_type)
{
	double alpha, beta, sigma, value;
	alpha = (double)(rand() & NoiseMask) / NoiseMask;
	if (alpha == 0.0)
		alpha = 1.0;

	switch (noise_type) {
		case noiseGaussian:
		{
			double tau;
			beta = (double)(rand() & NoiseMask) / NoiseMask;
			sigma = sqrt(-2.0 * log(alpha)) * cos(2.0 * M_PI * beta);
			tau = sqrt(-2.0 * log(alpha)) * sin(2.0 * M_PI * beta);
			value = (double)pixel + (sqrt((double)pixel) * SigmaGaussian * sigma) + (TauGaussian * tau);
			break;
		}

		case noiseMultiplicativeGaussian:
			if (alpha <= NoiseEpsilon)
				sigma = 255;
			else
				sigma = sqrt(-2.0 * log(alpha));
			beta = (rand() & NoiseMask) / NoiseMask;
			value = (double)pixel +
					pixel * SigmaMultiplicativeGaussian * sigma * cos(2.0 * M_PI * beta);
			break;

		case noiseImpluse:
			if (alpha < (SigmaImpulse / 2.0))
				value = 0;
			else if (alpha >= (1.0 - (SigmaImpulse / 2.0)))
				value = 255;
			else
				value = pixel;
			break;

		case noiseLaplacian:
			if (alpha <= 0.5)
			{
				if (alpha <= NoiseEpsilon)
					value = (double)pixel - 255;
				else
					value = (double)pixel + SigmaLaplacian * log(2.0 * alpha);
				break;
			}
			beta = 1.0 - alpha;
			if (beta <= (0.5 * NoiseEpsilon))
				value = (double)pixel + 255;
			else
				value = (double)pixel - SigmaLaplacian * log(2.0 * beta);
			break;

		case noisePoisson:
		{
			register int i;
			for (i = 0; alpha > exp(-SigmaPoisson * pixel); i++)
			{
				beta = (double)(rand() & NoiseMask) / NoiseMask;
				alpha = alpha * beta;
			}
			value = i / SigmaPoisson;
			break;
		}

		case noiseUniform:
		default:
			value = (double)pixel + SigmaUniform * (alpha - 0.5);
			break;
	}

	if (value < 0.0)
		return 0;
	if (value > 255)
		return 255;
	return (BYTE)(value + 0.5);
}

BOOL addnoise(mif& src, LPRECT prc, int noise_type)
{
	CRect rc;
	if(!src.validaterect(prc, &rc))
		return FALSE;
	src.m_bits = 24;

	srand(GetTickCount());
	parallel_for (0, rc.Height(), [&](int y) {
		RGBA* p = src.pos(rc.left, rc.top + y);
		int x = rc.Width();
		while (x-- > 0) {
			p->red = gen_noise(p->red, noise_type);
			p->green = gen_noise(p->green, noise_type);
			p->blue = gen_noise(p->blue, noise_type);
			p++;
		}
	});
	return TRUE;
}
