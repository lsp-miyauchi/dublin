/* crypto/bf/bf_enc.c */

#include "blowfish.h"
#include "bf_locl.h"

/* Blowfish as implemented from 'Blowfish: Springer-Verlag paper'
 * (From LECTURE NOTES IN COIMPUTER SCIENCE 809, FAST SOFTWARE ENCRYPTION,
 * CAMBRIDGE SECURITY WORKSHOP, CAMBRIDGE, U.K., DECEMBER 9-11, 1993)
 */

#if (BF_ROUNDS != 16) && (BF_ROUNDS != 20)
If you set BF_ROUNDS to some value other than 16 or 20, you will have
to modify the code.
#endif

void BF_encrypt(data, key, encrypt)
BF_LONG *data;
BF_KEY *key;
int encrypt;
{
	register BF_LONG l,r,*p,*s;

	p = key->P;
	s = &(key->S[0]);
	l = data[0];
	r = data[1];

	if (encrypt)
	{
		l^=p[0];
		BF_ENC(r,l,s,p[ 1]);
		BF_ENC(l,r,s,p[ 2]);
		BF_ENC(r,l,s,p[ 3]);
		BF_ENC(l,r,s,p[ 4]);
		BF_ENC(r,l,s,p[ 5]);
		BF_ENC(l,r,s,p[ 6]);
		BF_ENC(r,l,s,p[ 7]);
		BF_ENC(l,r,s,p[ 8]);
		BF_ENC(r,l,s,p[ 9]);
		BF_ENC(l,r,s,p[10]);
		BF_ENC(r,l,s,p[11]);
		BF_ENC(l,r,s,p[12]);
		BF_ENC(r,l,s,p[13]);
		BF_ENC(l,r,s,p[14]);
		BF_ENC(r,l,s,p[15]);
		BF_ENC(l,r,s,p[16]);
#if BF_ROUNDS == 20
		BF_ENC(r,l,s,p[17]);
		BF_ENC(l,r,s,p[18]);
		BF_ENC(r,l,s,p[19]);
		BF_ENC(l,r,s,p[20]);
#endif
		r^=p[BF_ROUNDS+1];
	}
	else
	{
		l^=p[BF_ROUNDS+1];
#if BF_ROUNDS == 20
		BF_ENC(r,l,s,p[20]);
		BF_ENC(l,r,s,p[19]);
		BF_ENC(r,l,s,p[18]);
		BF_ENC(l,r,s,p[17]);
#endif
		BF_ENC(r,l,s,p[16]);
		BF_ENC(l,r,s,p[15]);
		BF_ENC(r,l,s,p[14]);
		BF_ENC(l,r,s,p[13]);
		BF_ENC(r,l,s,p[12]);
		BF_ENC(l,r,s,p[11]);
		BF_ENC(r,l,s,p[10]);
		BF_ENC(l,r,s,p[ 9]);
		BF_ENC(r,l,s,p[ 8]);
		BF_ENC(l,r,s,p[ 7]);
		BF_ENC(r,l,s,p[ 6]);
		BF_ENC(l,r,s,p[ 5]);
		BF_ENC(r,l,s,p[ 4]);
		BF_ENC(l,r,s,p[ 3]);
		BF_ENC(r,l,s,p[ 2]);
		BF_ENC(l,r,s,p[ 1]);
		r^=p[0];
	}
	data[1] = l & 0xffffffff;
	data[0] = r & 0xffffffff;
}

void BFx_encrypt(data, key)
BF_LONG *data;
BF_KEY *key;
{
	register BF_LONG l,r,*p,*s;

	p = key->P;
	s = &(key->S[0]);
	l = data[0];
	r = data[1];

	l^=p[0];
	BF_ENC(r,l,s,p[ 1]);
	BF_ENC(l,r,s,p[ 2]);
	BF_ENC(r,l,s,p[ 3]);
	BF_ENC(l,r,s,p[ 4]);
	BF_ENC(r,l,s,p[ 5]);
	BF_ENC(l,r,s,p[ 6]);
	BF_ENC(r,l,s,p[ 7]);
	BF_ENC(l,r,s,p[ 8]);
	BF_ENC(r,l,s,p[ 9]);
	BF_ENC(l,r,s,p[10]);
	BF_ENC(r,l,s,p[11]);
	BF_ENC(l,r,s,p[12]);
	BF_ENC(r,l,s,p[13]);
	BF_ENC(l,r,s,p[14]);
	BF_ENC(r,l,s,p[15]);
	BF_ENC(l,r,s,p[16]);
#if BF_ROUNDS == 20
	BF_ENC(r,l,s,p[17]);
	BF_ENC(l,r,s,p[18]);
	BF_ENC(r,l,s,p[19]);
	BF_ENC(l,r,s,p[20]);
#endif
	r^=p[BF_ROUNDS+1];

	data[1] = l & 0xffffffff;
	data[0] = r & 0xffffffff;
}

void BFx_decrypt(data, key)
BF_LONG *data;
BF_KEY *key;
{
	register BF_LONG l,r,*p,*s;

	p = key->P;
	s = &(key->S[0]);
	l = data[0];
	r = data[1];

	l^=p[BF_ROUNDS+1];
#if BF_ROUNDS == 20
	BF_ENC(r,l,s,p[20]);
	BF_ENC(l,r,s,p[19]);
	BF_ENC(r,l,s,p[18]);
	BF_ENC(l,r,s,p[17]);
#endif
	BF_ENC(r,l,s,p[16]);
	BF_ENC(l,r,s,p[15]);
	BF_ENC(r,l,s,p[14]);
	BF_ENC(l,r,s,p[13]);
	BF_ENC(r,l,s,p[12]);
	BF_ENC(l,r,s,p[11]);
	BF_ENC(r,l,s,p[10]);
	BF_ENC(l,r,s,p[ 9]);
	BF_ENC(r,l,s,p[ 8]);
	BF_ENC(l,r,s,p[ 7]);
	BF_ENC(r,l,s,p[ 6]);
	BF_ENC(l,r,s,p[ 5]);
	BF_ENC(r,l,s,p[ 4]);
	BF_ENC(l,r,s,p[ 3]);
	BF_ENC(r,l,s,p[ 2]);
	BF_ENC(l,r,s,p[ 1]);
	r^=p[0];

	data[1] = l & 0xffffffff;
	data[0] = r & 0xffffffff;
}
