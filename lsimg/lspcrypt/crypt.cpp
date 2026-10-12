////////////////////////////////////////////////////////////
// crypt.cpp

#include "stdafx.h"
#include "lspcrypt.h"

static WORD cbCrc[UCHAR_MAX];
static BYTE cbCPtbl[] = {
	0x10, 0x17, 0x13, 0x15, 0x09,
	0x08, 0x0a, 0x14, 0x06, 0x05,
	0x16, 0x02, 0x0d, 0x03, 0x01,
	0x04, 0x19, 0x0c, 0x0f, 0x0e,
	0x12, 0x07, 0x0b, 0x18, 0x11, 0x1a
};

typedef struct tagCPS {
	int index;
	BOOL flg;
} CPS;

#define CRCPOLY16		0x1021

void __fastcall initcrc16tbl()
{
	register WORD i, j , r;
	for (i = 0; i <= UCHAR_MAX; i++)
	{
		r = i << (16 - CHAR_BIT);
		for (j = 0; j < CHAR_BIT; j++)
		{
			if (r & 0x8000)
				r = (r << 1) ^ CRCPOLY16;
			else
				r <<= 1;
		}
		cbCrc[i] = r & 0xFFFF;
	}
}

WORD __fastcall crc16(int n, BYTE* pcb, WORD crc)
{
	register WORD r = crc;
	while (--n >= 0)
		r = (r << CHAR_BIT) ^ cbCrc[(BYTE)(r >> (16 - CHAR_BIT)) ^ *pcb++];
	return ~r & 0xFFFF;
}

static void __fastcall cp_gent(CPS* pcps, int* ptbl, int count, char *pszCode)
{
	register int i, k, v1, v2, len;
	for (i = 0; i < count; i++)
	{
		ptbl[i] = -1;
		pcps[i].index = i;
		pcps[i].flg = FALSE;
	}

	len = lstrlen(pszCode);
	v1 = count - 1;
	v2 = len + count % len;
	for (k = 0; k < count; k++)
	{
		v1 = cbCPtbl[pszCode[k % len] - 'A'] + v1 + v2;
		if (v1 >= count)
			v1 %= count;

		while (ptbl[v1] != -1)
		{
			if (k & 01)
			{
				if (v1 == 0)
					v1 = count;
				v1--;
			}
			else
			{
				if (++v1 >= count)
					v1 = 0;
			}
		}
		ptbl[v1] = k;
		v2++;
	}

	for (i = 0, k = count - 1; i < k; i++, k--)
	{
		pcps[ptbl[i]].index = ptbl[k];
		pcps[ptbl[k]].index = ptbl[i];
		if ((ptbl[i] ^ ptbl[k]) & 0x01)
			pcps[ptbl[i]].flg = pcps[ptbl[k]].flg = TRUE;
	}
}

static void __fastcall cp_domask(LPBYTE pcb, CPS* cps, int nSize)
{
	register BYTE cb;
	BYTE cbw[1024];

	memcpy(cbw, pcb, nSize);
	for (int i = 0; i < nSize; i++)
	{
		cb = cbw[cps[i].index];
		if (cps[i].flg)
			cb = ((cb & 0x0F) << 4) | ((cb & 0xF0) >> 4);
		pcb[i] = cb;
	}
}

extern "C"
BOOL WINAPI lspCrypt(CRYPTINFO* pcc)
{
	if (!pcc || pcc->cbSize != sizeof(CRYPTINFO) ||
		!pcc->pszCode || lstrlen(pcc->pszCode) < 1 ||
		!pcc->pData || pcc->dwSize < 1)
		return FALSE;

	// パスワード初期化
	char code[MAX_PATH];
	lstrcpy(code, pcc->pszCode);
	int len = lstrlen(code);
	for (int i = 0; i < len; i++)
	{
		if (isalpha(code[i]))
			code[i] = toupper(code[i]);
		else
			code[i] = (code[i] % 26) + 'A';
	}
	pcc->crcCode = crc16(lstrlen(pcc->pszCode), (BYTE*)pcc->pszCode, 0);

	register LPBYTE pd = pcc->pData;
	if (pcc->crypttype == CRYPT_CP1024 || pcc->crypttype == CRYPT_CP256)
	{
		DWORD sector = (pcc->crypttype == CRYPT_CP1024) ? 1024 : 256;
		int* ptbl = new int[sector];
		CPS* pcps = new CPS[sector];
		cp_gent(pcps, ptbl, sector, code);

		DWORD block, done = 0;
		while (done < pcc->dwSize)
		{
			block = min(sector, pcc->dwSize - done);
			if (block != sector)
				cp_gent(pcps, ptbl, block, code);

			pcc->crcSrc = crc16(block, pd, pcc->crcSrc);
			cp_domask(pd, pcps, block);
			pcc->crcDst = crc16(block, pd, pcc->crcDst);

			pd += block;
			done += block;
		}
		delete [] ptbl;
		delete [] pcps;
	}
	else if (pcc->crypttype == CRYPT_XOR)
	{
		register DWORD i;
		register DWORD crc = MAKELONG(pcc->crcCode, pcc->crcCode);
		crc = (crc & 0xFFFFFF00) | (len & 0x000000FF);

		pcc->crcSrc = crc16(pcc->dwSize, pcc->pData, pcc->crcSrc);
		for (i = 0; i < pcc->dwSize; i++)
		{
			*pd = (*pd ^ (BYTE)(crc & 0xff));
			pd++;
			crc = (DWORD)_rotr((int)crc, 1);
		}
		pcc->crcDst = crc16(pcc->dwSize, pcc->pData, pcc->crcDst);
	}
	return TRUE;
}

extern "C"
DWORD WINAPI lspCodeCRC(LPCSTR pszCode)
{
	if (!pszCode || lstrlen(pszCode) < 1)
		return (DWORD)-1;
	return crc16(lstrlen(pszCode), (BYTE*)pszCode, 0);
}
