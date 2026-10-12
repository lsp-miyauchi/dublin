////////////////////////////////////////////////////////////////
// for FileVisor5

BYTE mapbyte_fv5[] = {
	0x2c,0x24,0x28,0x38,0x2e,0x39,0x32,0x2d,		// 0
	0x3a,0x25,0x26,0x36,0x23,0x34,0x2a,0x15,
	0x2b,0x12,0x21,0x13,0x14,0x16,0x31,0x17,
	0x19,0x11,0x37,0x18,0x27,0x30,0x33,0x22,
	0x2d,0x19,0x2b,0x23,0x27,0x3a,0x31,0x13,		// 1
	0x2a,0x22,0x14,0x12,0x34,0x15,0x2e,0x16,
	0x25,0x36,0x37,0x18,0x21,0x32,0x11,0x26,
	0x38,0x2c,0x30,0x17,0x24,0x28,0x33,0x39,
	0x2b,0x2d,0x27,0x3a,0x30,0x13,0x36,0x25,		// 2
	0x17,0x23,0x2e,0x31,0x12,0x14,0x11,0x39,
	0x24,0x16,0x32,0x21,0x22,0x34,0x37,0x19,
	0x15,0x26,0x18,0x28,0x2a,0x2c,0x33,0x38,
	0x30,0x36,0x17,0x2b,0x39,0x27,0x22,0x34,		// 3
	0x25,0x2c,0x23,0x19,0x12,0x14,0x3a,0x11,
	0x38,0x2a,0x15,0x21,0x16,0x18,0x26,0x2d,
	0x31,0x24,0x32,0x28,0x13,0x37,0x2e,0x33,
	0x17,0x31,0x36,0x2e,0x21,0x30,0x32,0x13,		// 4
	0x22,0x3a,0x2d,0x39,0x37,0x33,0x16,0x2a,
	0x28,0x2b,0x26,0x15,0x25,0x34,0x38,0x11,
	0x2c,0x24,0x18,0x23,0x19,0x12,0x27,0x14,
	0x25,0x24,0x27,0x39,0x12,0x22,0x23,0x19,		// 5
	0x36,0x28,0x11,0x38,0x15,0x26,0x16,0x33,
	0x34,0x2d,0x13,0x2e,0x30,0x37,0x31,0x21,
	0x17,0x18,0x3a,0x2c,0x2a,0x32,0x2b,0x14,
};

extern "C"
void WINAPI KrFV5(int prx, int year, int month, int day, int seq, LPSTR pszSeq)
{
	BYTE bitkey[12];

	int index = seq % 6;
	year -= 1980;
	WORD date_1 = K2C_YEAR(year) + K2C_MON(month) + K2C_DAY(day);
	WORD date_2 = L2C_YEAR(year) + L2C_MON(month) + L2C_DAY(day);

	DWORD seq_1 = seq + date_2;
	DWORD seq_2 = dword_rangerot_right(seq_1, 17, index + 1);

	BYTE prx_1 = byte_rangerot_left((BYTE)prx, 7, index + 1);
	BYTE prx_2 = byte_rangerot_right((BYTE)prx, 7, index + 1);

	bitkey[ 2] = (BYTE)((seq_1 >> 9) & 0xff);
	bitkey[ 3] = (BYTE)((seq_1 >> 1) & 0xff);
	bitkey[ 4] = (BYTE)((seq_1 << 7) & 0x80) | (BYTE)(date_1 >> 7);
	bitkey[ 5] = (BYTE)((date_1 << 1) & 0xfe) | (prx_1 >> 6);
	bitkey[ 6] = (prx_1 << 2) | (BYTE)(((date_2) >> 12) & 0x03);
	bitkey[ 7] = (BYTE)((date_2 >> 4) & 0xff);
	bitkey[ 8] = (BYTE)((date_2 << 4) & 0xf0) | (BYTE)(prx_2 >> 3);
	bitkey[ 9] = (BYTE)((prx_2 << 5) & 0xf0) | (BYTE)((seq_2 >> 12) & 0xff);
	bitkey[10] = (BYTE)((seq_2 >> 4) & 0xff);
	bitkey[11] = (BYTE)((seq_2 << 4) & 0xf0);
	bitkey[12] = (BYTE)((index + 1) << 4);

	WORD crc = crc16(10, &bitkey[2]);
	bitkey[0] = (BYTE)(crc >> 8);
	bitkey[1] = (BYTE)(crc & 0xff);

	DWORD bfdata[2];
	bfdata[0] = *((DWORD*)bitkey);
	bfdata[1] = *((DWORD*)bitkey + 1);

	BF_KEY key;
	bf_setkey(&key, 2, (BYTE*)&bitkey[8]);
	bf_encrypt(bfdata, &key);

	*((DWORD*)bitkey) = bfdata[0];
	*((DWORD*)bitkey + 1) = bfdata[1];

	int pos = 0;
	char* p = pszSeq;
	for (int i = 0; i < 20; i++) {
		int n = get5bit(bitkey, i);
		if (i < 19)		*p++ = mapbyte_fv5[index * 32 + n] + 0x20;
		else			*p++ = '0' + n;
		pos++;
		if (!(pos % 5))	*p++ = '-';
	}
	pszSeq[23] = '\0';
}

extern "C"
LPCSTR WINAPI KrFV5Ex(int nPrx, int nYear, int nMonth, int nDay, int nSeqNumber)
{
	KrFV5(nPrx, nYear, nMonth, nDay, nSeqNumber, sszEncBuff);
	return sszEncBuff;
}

extern "C"
BOOL WINAPI KrFV5_Decode(LPCSTR pszSrc, LPSTR pszDst)
{
	char buf[MAX_PATH];
	char* p = (char*)pszSrc;
	char* q = buf;
	while (*p) {
		if (*p != '-') {
			if (*(p + 1))	*q++ = *p - 0x20;
			else			*q++ = *p;
		}
		p++;
	}
	*q = '\0';

	int index = buf[19] - '1';
	if (index < 0 || index > 5) {
		lstrcpy(pszDst, "index error");
		return FALSE;
	}

	BYTE bitkey[12];
	memset(bitkey, 0, sizeof(bitkey));
	q = buf;
	int i;
	for (i = 0; i < 20; i++) {
		if (i < 19) {
			int j;
			for (j = 0; j < 32; j++) {
				if (mapbyte_fv5[index * 32 + j] == *q) {
					put5bit(bitkey, i, j);
					break;
				}
			}
			if (j == 32) {
				lstrcpy(pszDst, "mapchar error");
				return FALSE;
			}
		}
		else
			put5bit(bitkey, i, index + 1);
		q++;
	}

	DWORD bfdata[2];
	bfdata[0] = *((DWORD*)bitkey);
	bfdata[1] = *((DWORD*)bitkey + 1);

	BF_KEY key;
	bf_setkey(&key, 2, (BYTE*)&bitkey[8]);
	bf_decrypt(bfdata, &key);

	*((DWORD*)bitkey) = bfdata[0];
	*((DWORD*)bitkey + 1) = bfdata[1];

	WORD crc = ((WORD)bitkey[0] << 8) | (WORD)bitkey[1];
	if (crc != crc16(10, &bitkey[2])) {
		lstrcpy(pszDst, "crc error");
		return FALSE;
	}

	DWORD seq_2 = (((DWORD)bitkey[9] & 0x1f) << 12) |
				  ((DWORD)bitkey[10] << 4) |
				  ((DWORD)(bitkey[11] & 0xf0) >> 4);
	seq_2 = dword_rangerot_left(seq_2, 17, index + 1);

	BYTE prx_2  = ((bitkey[8] & 0x0f) << 3) |
				  (bitkey[9] >> 5);
	prx_2 = byte_rangerot_left(prx_2, 7, index + 1);

	WORD date_2 = (((WORD)bitkey[6] & 3) << 12) |
				  ((WORD)bitkey[7] << 4) |
				  ((WORD)bitkey[8] >> 4);
	BYTE prx_1  = ((bitkey[5] & 1) << 6) |
				  (bitkey[6] >> 2);
	prx_1 = byte_rangerot_right(prx_1, 7, index + 1);

	WORD date_1 = (((WORD)bitkey[4] & 0x7f) << 7) | 
				  (((WORD)bitkey[5] & 0xfe) >> 1);
	DWORD seq_1 = ((DWORD)bitkey[2] << 9) |
				  ((DWORD)bitkey[3] << 1) |
				  (((DWORD)bitkey[4] & 0x80) >> 7);

	if (prx_2 != prx_1) {
		wsprintf(pszDst, "prefix error 1=%02X, 2=%02X", prx_1, prx_2);
		return FALSE;
	}

	if (seq_1 != seq_2) {
		lstrcpy(pszDst, "seq1 not eq seq2");
		return FALSE;
	}

	if (seq_1 < date_2) {
		lstrcpy(pszDst, "sequence error");
		return FALSE;
	}
	seq_1 -= date_2;

	if ((K2E_YEAR(date_1) != L2E_YEAR(date_2)) ||
		(K2E_MON(date_1) != L2E_MON(date_2)) ||
		(K2E_DAY(date_1) != L2E_DAY(date_2)))
	{
		lstrcpy(pszDst, "date match error");
		return FALSE;
	}

	int year = K2E_YEAR(date_1) + 1980;
	int mon = K2E_MON(date_1);
	int day = K2E_DAY(date_1);

	if ((seq_1 % 6) != index) {
		lstrcpy(pszDst, "seq index mod error");
		return FALSE;
	}

	wsprintf(pszDst, "%04d/%02d/%02d: %d", year, mon, day, seq_1);
	return TRUE;
}

