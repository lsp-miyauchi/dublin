//////////////////////////////////////////////////////////////////////
// common.cpp

#include "stdafx.h"
#include "lspmask.h"
#include "common.h"


void loadblock(LPBYTE pcb, LPBYTE pData, int sx, int sy, int dx, int dy, int cxBlock, int cyBlock)
{
	pData += sy * dx + sx;
	for (int y = 0; y < cyBlock; y++) {
		memcpy(pcb, pData, cxBlock);
		pData += dx;
		pcb += cxBlock;
	}
}

void saveblock(LPBYTE pcb, LPBYTE pData, int sx, int sy, int dx, int dy, int cxBlock, int cyBlock)
{
	pData += sy * dx + sx;
	for (int y = 0; y < cyBlock; y++) {
		memcpy(pData, pcb, cxBlock);
		pData += dx;
		pcb += cxBlock;
	}
}

void loadblock8(LPBYTE pcb, LPBYTE pData, int sx, int sy, int dx, int dy)
{
	pData += sy * dx + sx;
	for (int y = 0; y < 8; y++) {
		memcpy(pcb, pData, 8);
		pData += dx;
		pcb += 8;
	}
}

void saveblock8(LPBYTE pcb, LPBYTE pData, int sx, int sy, int dx, int dy)
{
	pData += sy * dx + sx;
	for (int y = 0; y < 8; y++)	{
		memcpy(pData, pcb, 8);
		pData += dx;
		pcb += 8;
	}
}

void loadblock16(LPBYTE pcb, LPBYTE pData, int sx, int sy, int dx, int dy)
{
	pData += sy * dx + sx;
	for (int y = 0; y < 16; y++) {
		memcpy(pcb, pData, 16);
		pData += dx;
		pcb += 16;
	}
}

void saveblock16(LPBYTE pcb, LPBYTE pData, int sx, int sy, int dx, int dy)
{
	pData += sy * dx + sx; 
	for (int y = 0; y < 16; y++) {
		memcpy(pData, pcb, 16);
		pData += dx;
		pcb += 16;
	}
}

void loadpkt8(LPRGBPK pcb, LPRGBPK pData, int sx, int sy, int dx, int dy)
{
	pData += sy * dx + sx;
	for (int y = 0; y < 8; y++) {
		memcpy(pcb, pData, 8 * sizeof(RGBPK));
		pData += dx;
		pcb += 8;
	}
}

void savepkt8(LPRGBPK pcb, LPRGBPK pData, int sx, int sy, int dx, int dy)
{
	pData += sy * dx + sx;
	for (int y = 0; y < 8; y++) {
		memcpy(pData, pcb, 8 * sizeof(RGBPK));
		pData += dx;
		pcb += 8;
	}
}

void form_mirror(LPBYTE pData, int cx, int cy)
{
	register LPBYTE ps, pd;
	LPBYTE pw = new BYTE[cx * cy];
	memcpy(pw, pData, cx * cy);
	for (int y = 0; y < cy; y++) {
		ps = pw + y * cx;
		pd = pData + (y + 1) * cx - 1;
		for (int x = 0; x < cx; x++)
			*pd-- = *ps++;
	}
	delete [] pw;
}

void form_flip(LPBYTE pData, int cx, int cy)
{
	register LPBYTE ps, pd;
	LPBYTE pw = new BYTE[cx * cy];
	memcpy(pw, pData, cx * cy);
	for (int y = 0; y < cy; y++) {
		ps = pw + y * cx;
		pd = pData + (cy - y - 1) * cx;
		memcpy(pd, ps, cx);
	}
	delete [] pw;
}

