//////////////////////////////////////////////////////////////////////
// testcab.cpp

#include "stdafx.h"
#include "cab.h"
#include "filemap.h"

static DWORD __fastcall CSUMCompute(void *pv, UINT cb, DWORD seed)
{
	int cUlong = cb / 4;				// Number of ULONGs in block
	DWORD csum = seed;					// Init checksum
	BYTE* pb = (BYTE*)pv; 				// Start at front of data block

	//** Checksum integral multiple of ULONGs
	ULONG ul;
	while (cUlong-- > 0) {
		//** NOTE: Build ULONG in big/little-endian independent manner
		ul = *pb++;                     // Get low-order byte
		ul |= (((ULONG)(*pb++)) <<  8); // Add 2nd byte
		ul |= (((ULONG)(*pb++)) << 16); // Add 3nd byte
		ul |= (((ULONG)(*pb++)) << 24); // Add 4th byte
		csum ^= ul;                     // Update checksum
	}

	//** Checksum remainder bytes
	ul = 0;
	switch (cb % 4) {
	case 3:
		ul |= (((ULONG)(*pb++)) << 16); // Add 3nd byte
	case 2:
		ul |= (((ULONG)(*pb++)) <<  8); // Add 2nd byte
	case 1:
		ul |= *pb++;                    // Get low-order byte
	default:
		break;
	}
	csum ^= ul;                         // Update checksum

	//** Return computed checksum
	return csum;
}


BOOL CabCheckInside(CCabEnv* pev, LPBYTE pBase)
{
	LPBYTE pb = pBase;

	// CFHEADER
	CFHEADER* ph = (CFHEADER*)pb;

	// シグネチャ
	if (ph->signature != 0x4643534d) {
		pev->msgout(L"  not a cabinet\n");
		return FALSE;
	}

	// キャビネットサイズ分の読み書きができるかチェック
	if (IsBadReadPtr(pBase, ph->cbCabinet)) {
		pev->msgout(L"  invalid cabinet size\n");
		return FALSE;
	}

	pb += sizeof(CFHEADER);
	int rsvHeader = 0, rsvFolder = 0, rsvData = 0;
	if (ph->flags & cfhdrRESERVE_PRESENT) {
		rsvHeader = *(WORD*)pb;
		rsvFolder = *(pb + 2);
		rsvData = *(pb + 3);
		pb += 4 + rsvHeader;
	}

	if (ph->flags & cfhdrPREV_CABINET) {
		pb += strlen((char*)pb) + 1;
		pb += strlen((char*)pb) + 1;
	}

	if (ph->flags & cfhdrNEXT_CABINET) {
		pb += strlen((char*)pb) + 1;
		pb += strlen((char*)pb) + 1;
	}

	// CFFOLDER
	CFFOLDER* pf;
	for (int iFolder = 0; iFolder < ph->cFolders; iFolder++) {
		pf = (CFFOLDER*)pb;

		// フォルダサイズ分の読み書きができるかチェック
		if (IsBadReadPtr(pf, sizeof(CFFOLDER))) {
			pev->msgout(L"  invalid folder area\n");
			return FALSE;
		}
		pb += sizeof(CFFOLDER) + rsvFolder;

		// CFDATA
		CFDATA* pd;
		LPBYTE pbw = pBase + pf->coffCabStart;
		for (int i = 0; i < pf->cCFData; i++) {
			pd = (CFDATA*)pbw;
			pbw += sizeof(CFDATA) + rsvData;

			// データサイズ分の読み書きができるかチェック
			if (IsBadReadPtr(pbw, pd->cbData)) {
				pev->msgout(L"  invalid data area\n");
				return FALSE;
			}

			DWORD csumPartial = CSUMCompute(pbw, pd->cbData, 0);
			DWORD csum = CSUMCompute(&pd->cbData, sizeof(CFDATA) + rsvData - sizeof(DWORD), csumPartial);
			if (pd->csum != csum) {
				pev->msgout(L" checksum error in folder #%d data #%d\n", iFolder, i);
				return FALSE;
			}
			pbw += pd->cbData;
		}
	}
	return TRUE;
}
