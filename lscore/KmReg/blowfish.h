//////////////////////////////////////////////////////////////////////
// blowfish.h

#define BF_ROUNDS	16

typedef struct bf_key_st {
	DWORD P[BF_ROUNDS + 2];
	DWORD S[4 * 256];
} BF_KEY;


void bf_encrypt(DWORD* data, BF_KEY* key);
void bf_decrypt(DWORD* data, BF_KEY* key);
void bf_setkey(BF_KEY* key, int len, BYTE* data);
