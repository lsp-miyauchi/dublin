////////////////////////////////////////////////////////////////////////
// reguserinfo.h : ヘッダー ファイル
//

#pragma once
#ifndef __REGUSERINFO_H__
#define __REGUSERINFO_H__


////////////////////////////////////////////////////////////////////////
//

#define CRCPOLY16			0x1021

class CCrc16
{
private:
	WORD* m_crctable;

public:
	CCrc16()
	{
		m_crctable = new WORD[UCHAR_MAX + 1];
		init();
	}

	~CCrc16()
	{
		delete [] m_crctable;
	}

	void init()
	{
		WORD i, j , r;
		for (i = 0; i <= UCHAR_MAX; i++) {
			r = i << (16 - CHAR_BIT);
			for (j = 0; j < CHAR_BIT; j++) {
				if (r & 0x8000)	r = (r << 1) ^ CRCPOLY16;
				else			r <<= 1;
			}
			m_crctable[i] = r & 0xFFFF;
		}
	}

	WORD crc16(int n, BYTE* pcb)
	{
		WORD r = 0xFFFF;
		while (--n >= 0)
			r = (r << CHAR_BIT) ^ m_crctable[(BYTE)(r >> (16 - CHAR_BIT)) ^ *pcb++];
		return ~r & 0xFFFF;
	}
};


////////////////////////////////////////////////////////////////////////
//


class USRINFO
{
public:
	USRINFO()
	{
		memset(clen, 0, sizeof(clen));
		hwnd = NULL;
		bClickOK = FALSE;
		memset(bitkey, 0, sizeof(bitkey));
		key_raw = 0;
		memset(bfdata, 0, sizeof(bfdata));
		bf_key = NULL;
		msg = 0;
		crc = 0;
		c_crc = 0;
		tick = 0;
		prx = 0;
		dates = 0;
		seq = 0;
		memset(c_prx, 0, sizeof(c_prx));
		memset(c_date, 0, sizeof(c_date));
		memset(c_seq, 0, sizeof(c_seq));
		index = 0;
		retry_counter = 0;
		m_name.Empty();
		m_company.Empty();
		memset(code, 0, sizeof(code));
		numlimits = 0;

		DWORD dw = GetTickCount();
		WORD d = (WORD)(dw % 315) + 1;
		tick = (double)d;
		prx = (double)REGPREFIX / tick;

		pbk_prx0 = &bitkey[5];
		pbk_prx1 = &bitkey[8];
		pbk_date0 = &bitkey[4];
		pbk_date1 = &bitkey[6];
		pbk_seq1 = &bitkey[2];
		pbk_seq2 = &bitkey[9];
	}

	int clen[4];
	HWND hwnd;
	BOOL bClickOK;
	BYTE bitkey[12];
	DWORDLONG key_raw;
	DWORD bfdata[2];
	void* bf_key;
	UINT msg;
	double crc;						// bitkey[0-1]に埋め込まれた値
	double c_crc;					// bitkey[2～最後]までのcrc値
	double tick;					// GetTickCount()の下位ワードのdouble値
	double prx;						// REGPREFIXをtickで割った値
	int dates;
	int seq;
	double c_prx[2];				// 
	WORD c_date[2];
	DWORD c_seq[2];
	int index;
	int retry_counter;
	CString m_name;
	CString m_company;
	char code[MAX_PATH];
	long numlimits;

	// bitkey[]の各種メンバへのポインタ
	BYTE* pbk_prx0;
	BYTE* pbk_prx1;
	BYTE* pbk_date0;
	BYTE* pbk_date1;
	BYTE* pbk_seq1;
	BYTE* pbk_seq2;

	CCrc16 m_crc;

	void calccrc()
	{
		c_crc = (double)m_crc.crc16(10, &bitkey[2]);
	}
};

#endif			// __REGUSERINFO_H__
