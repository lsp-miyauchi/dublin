/* crypto/bf/blowfish.h */

#ifndef HEADER_BLOWFISH_H
#define HEADER_BLOWFISH_H

#ifdef  __cplusplus
extern "C" {
#endif

#define BF_ENCRYPT	1
#define BF_DECRYPT	0

/* If you make this 'unsigned int' the pointer variants will work on
 * the Alpha, otherwise they will not.  Strangly using the '8 byte'
 * BF_LONG and the default 'non-pointer' inner loop is the best configuration
 * for the Alpha */
#define BF_LONG unsigned long

//#define BF_ROUNDS	16
#define BF_ROUNDS	20
#define BF_BLOCK	8

typedef struct bf_key_st {
	BF_LONG P[BF_ROUNDS + 2];
	BF_LONG S[4 * 256];
} BF_KEY;

#ifndef NOPROTO
 
void BF_set_key(BF_KEY *key, int len, unsigned char *data);
void BF_ecb_encrypt(unsigned char *in,unsigned char *out,BF_KEY *key,
	int encrypt);
void BF_encrypt(BF_LONG *data,BF_KEY *key,int encrypt);
void BF_cbc_encrypt(unsigned char *in, unsigned char *out, long length,
	BF_KEY *ks, unsigned char *iv, int encrypt);
void BF_cfb64_encrypt(unsigned char *in, unsigned char *out, long length,
	BF_KEY *schedule, unsigned char *ivec, int *num, int encrypt);
void BF_ofb64_encrypt(unsigned char *in, unsigned char *out, long length,
	BF_KEY *schedule, unsigned char *ivec, int *num);
char *BF_options(void);

void BFx_encrypt(BF_LONG *data,BF_KEY *key);
void BFx_decrypt(BF_LONG *data,BF_KEY *key);

#else

void BF_set_key();
void BF_ecb_encrypt();
void BF_encrypt();
void BF_cbc_encrypt();
void BF_cfb64_encrypt();
void BF_ofb64_encrypt();
char *BF_options();

void BFx_encrypt();
void BFx_decrypt();

#endif

#ifdef  __cplusplus
}
#endif

#endif
