///////////////////////////////////////////////////////////////////////
// regcheck.cpp

#include "stdafx.h"
#include "sys\stat.h"
#include "process.h"
#include "fv7.h"
#include "reguserinfo.h"

///////////////////////////////////////////////////////////////
// ユーザ登録抹消:流出した登録キーのリスト

typedef struct tagINVALIDKEYS {
	DWORD seq;						// シーケンス番号
	int index;						// インデクス番号
	WORD year;						// 発覚した年月日
	WORD month;						// 〃
	WORD day;						// 〃
} INVALIDKEYS;

static INVALIDKEYS sstInvKeys[] = {
//	エントリ（行）番号, 最下位桁(1...6),		// 流出
	2,		3,		2007,  8,  1,	// QW2MN-V4C6M-J1WS6-V3QK3 ベクター
	344,	3,		2011,  3, 31,	// ETLP2-4NYG2-LH816-BM7J3 ベクターキャンセル 高見 健次郎
	734,	3,		2011,  3, 31,	// J6FVR-HGA23-J7X16-2MZ43 ベクターキャンセル 清水 正英
	1317,	4,		2011,  3, 31,	// LS7PN-KP6GQ-MWCY7-38TN4 ベクターキャンセル 吉原 高志
	1317,	4,		2011, 10,  7,	// 284T8-MCFG5-5PTY7-RD5Z4 ↑と同じインデクスだったので、これを無効にした
	65536,	0,		   0,  0,  0,
};



///////////////////////////////////////////////////////////////
// xxDDDDDMMMMYYYYY
// 0011111111111111
// mask d=0x3e00
//      m=0x01e0
//      y=0x001f
//
#define K2E_DAY(d)			(((d) & 0x3e00) >> 9)
#define K2E_MON(d)			(((d) & 0x01e0) >> 5)
#define K2E_YEAR(d)			(((d) & 0x001f))
#define K2C_DAY(d)			(((d) << 9) & 0x3e00)
#define K2C_MON(d)			(((d) << 5) & 0x01e0)
#define K2C_YEAR(d)			(((d) & 0x001f))

// xxMMMMYYYYYDDDDD
// 0011111111111111
// mask d=0x001f
//      m=0x3c00
//      y=0x03e0
//
#define L2E_DAY(d)			(((d) & 0x001f))
#define L2E_MON(d)			(((d) & 0x3c00) >> 10)
#define L2E_YEAR(d)			(((d) & 0x03e0) >> 5)
#define L2C_DAY(d)			(((d) & 0x001f))
#define L2C_MON(d)			(((d) << 10) & 0x3c00)
#define L2C_YEAR(d)			(((d) << 5) & 0x03e0)

#define BF_ROUNDS			16
typedef struct bf_key_st {
	DWORD P[BF_ROUNDS + 2];
	DWORD S[4 * 256];
} BF_KEY;
#define BF_M				0x3fc
#define BF_0				22L
#define BF_1				14L
#define BF_2				6L
#define BF_3				2L			// left shift

#define BF_ENC(LL,R,S,P) \
	LL^=P; \
	LL^= (((*(DWORD*)((BYTE*)&(S[  0])+((R>>BF_0)&BF_M))+ \
		*(DWORD*)((BYTE*)&(S[256])+((R>>BF_1)&BF_M)))^ \
		*(DWORD*)((BYTE*)&(S[512])+((R>>BF_2)&BF_M)))+ \
		*(DWORD*)((BYTE*)&(S[768])+((R<<BF_3)&BF_M)));

static void __fastcall bf_encrypt(DWORD* data, BF_KEY* key);
static void __fastcall bf_setkey(BF_KEY* key, int len, BYTE* data);
static void __fastcall bf_decrypt_pass1(DWORD* data, BF_KEY* key);
static void __fastcall bf_decrypt_pass2(DWORD* data, BF_KEY* key);
void reg_decrypt_pass1(void* pParam);

#ifdef __USE_CREATETHREAD__
	static DWORD WINAPI reg_decrypt_pass2_fn(void* pParam);
#else
	static unsigned __stdcall reg_decrypt_pass2_fn(void* pParam);
#endif

static void put5bit(BYTE* pcb, int index, BYTE cb)
{
	int i = index * 5;
	while (1) {
		if (i < 8) break;
		pcb++;
		i -= 8;
	}

	switch (i) {
	case 0:
		// 1111111122222222
		// aaaaa
		*pcb &= 0x07;
		*pcb |= (cb << 3);
		break;
	case 1:
		// 1111111122222222
		//  aaaaa
		*pcb &= 0x83;
		*pcb |= (cb << 2);
		break;
	case 2:
		// 1111111122222222
		//   aaaaa
		*pcb &= 0xc1;
		*pcb |= (cb << 1);
		break;
	case 3:
		// 1111111122222222
		//    aaaaa
		*pcb &= 0xe0;
		*pcb |= cb;
		break;
	case 4:
		// 1111111122222222
		//     aaaaa
		*pcb       &= 0xf0;
		*pcb       |= (cb >> 1);
		*(pcb + 1) &= 0x7f;
		*(pcb + 1) |= (cb << 7);
		break;
	case 5:
		// 1111111122222222
		//      aaaaa
		*pcb       &= 0xf8;
		*pcb       |= (cb >> 2);
		*(pcb + 1) &= 0x3f;
		*(pcb + 1) |= (cb << 6);
		break;
	case 6:
		// 1111111122222222
		//       aaaaa
		*pcb       &= 0xfc;
		*pcb       |= (cb >> 3);
		*(pcb + 1) &= 0x1f;
		*(pcb + 1) |= (cb << 5);
		break;
	case 7:
		// 1111111122222222
		//        aaaaa
		*pcb       &= 0xfe;
		*pcb       |= (cb >> 4);
		*(pcb + 1) &= 0x0f;
		*(pcb + 1) |= (cb << 4);
		break;
	}
}

int __fastcall make_bitkey(const char* pcode, BYTE* pbitkey)
{
	int len = lstrlenA(pcode);
	int index = pcode[len - 1] - '1';
	if (0 <= index && index <= 5) {
		int size = (len * 5) / 8;
		if ((len * 5) % 8) size++;
		BYTE* p = pbitkey;
		while (size > 0) {
			*p++ = 0;
			size--;
		}

		BYTE* ptbl = (BYTE*)_get_rcdata(IDR_REGKEYMAP, NULL);
		memset(pbitkey, 0, size);
		const char* q = pcode;
		for (int i = 0; i < len; i++) {
			if (i < len - 1) {
				int j;
				for (j = 0; j < 32; j++) {
					if (ptbl[index * 32 + j] == (*q - 0x20)) {
						put5bit(pbitkey, i, (BYTE)j);
						break;
					}
				}
				if (j == 32)
					return -1;
			}
			else
				put5bit(pbitkey, i, (BYTE)index + 1);
			q++;
		}
	}
	return index;
}

static void __fastcall bf_encrypt(DWORD* data, BF_KEY* key)
{
	register DWORD l,r,*p,*s;

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

	data[1] = l;
	data[0] = r;
}

static void __fastcall bf_setkey(BF_KEY* key, int len, BYTE* data)
{
	int i;
	DWORD *p,ri,in[2];
	BYTE *d,*end;

	// リソースデータからテーブルを読み込む
	void* pInit = _get_rcdata(IDR_BFINIT, NULL);
	if (pInit) memcpy(key, pInit, sizeof(BF_KEY));

	p = key->P;
	if (len > ((BF_ROUNDS + 2) * 4))
		len = (BF_ROUNDS + 2) * 4;

	d = data;
	end = &(data[len]);
	for (i = 0; i < (BF_ROUNDS + 2); i++) {
		ri = *(d++);
		if (d >= end) d = data;

		ri <<= 8;
		ri |= *(d++);
		if (d >= end) d = data;

		ri <<= 8;
		ri |= *(d++);
		if (d >= end) d = data;

		ri <<= 8;
		ri |= *(d++);
		if (d >= end) d = data;

		p[i] ^= ri;
	}

	in[0] = 0L;
	in[1] = 0L;
	for (i = 0; i < (BF_ROUNDS + 2); i += 2) {
		bf_encrypt(in, key);
		p[i    ] = in[0];
		p[i + 1] = in[1];
	}

	p = key->S;
	for (i = 0; i < 4 * 256; i += 2) {
		bf_encrypt(in, key);
		p[i    ] = in[0];
		p[i + 1] = in[1];
	}
}

static void __fastcall bf_decrypt_pass1(DWORD* data, BF_KEY* key)
{
	register DWORD l,r,*p,*s;

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

	data[0] = l;
	data[1] = r;
}

void reg_decrypt_pass1(void* pParam)
{
	USRINFO* pui = (USRINFO*)pParam;
	int index = make_bitkey(pui->code, pui->bitkey);
	if (index < 0) {
		PostMessage(pui->hwnd, pui->msg, 1, 0);
	}
	else {
		pui->bfdata[0] = *((DWORD*)pui->bitkey);
		pui->bfdata[1] = *((DWORD*)pui->bitkey + 1);
		bf_setkey((BF_KEY*)pui->bf_key, 2, (BYTE*)&pui->bitkey[8]);
		bf_decrypt_pass1(pui->bfdata, (BF_KEY*)pui->bf_key);
		pui->index = index;
#ifdef __USE_CREATETHREAD__
		DWORD id;
		HANDLE thread = CreateThread(NULL, 0, reg_decrypt_pass2_fn, pui, 0, &id);
#else
		unsigned id;
		HANDLE thread = (HANDLE)_beginthreadex(NULL, 0, reg_decrypt_pass2_fn, pui, 0, &id);
#endif
		if (::IsWindow(pui->hwnd)) {
			::SendMessage(pui->hwnd, nRM_REGADDTHREAD, 0, (LPARAM)thread);
		}
	}
}

///////////////////////////////////////////////////////////
// CRegistDlg

class CRegistDlg : public CDialogImpl<CRegistDlg>
{
public:
	CRegistDlg() { m_pui = NULL; }
	~CRegistDlg() { }

	enum { IDD = IDD_REGIST };

	USRINFO* m_pui;

BEGIN_MSG_MAP(CRegistDlg)
	ON_WM_INITDIALOG()
	ON_WM_DESTROY()
	ON_WM_COMMAND()
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDestroy() { if (m_pui->bClickOK) reg_decrypt_pass1(m_pui); }
	BOOL OnCommand(UINT id, UINT code, HWND hwnd);
};

BOOL CRegistDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	m_pui->bClickOK = 0;
	m_pui->key_raw = 0;
	if (m_pui->retry_counter > 3) {
		EndDialog(IDCANCEL);
		::PostMessage(m_pui->hwnd, m_pui->msg, 0, 0);
	}

	CString str;
	if (GetRegText(IDS_SEC_USER, IDS_KEY_USERNAME, str) > 0) {
		m_pui->m_name = str;
		if (GetRegText(IDS_SEC_USER, IDS_KEY_COMPANY, str) > 0)
			m_pui->m_company = str;
		SetDlgItemText(IDC_FVUR_NAME, m_pui->m_name);
		SetDlgItemText(IDC_FVUR_COMPANY, m_pui->m_company);
	}
	return TRUE;
}

BOOL CRegistDlg::OnCommand(UINT id, UINT code, HWND hwnd)
{
	TCHAR buf[MAX_PATH];
	switch (id) {
	case IDOK:
		for (int i = 0; i < 4; i++) {
			if (m_pui->clen[i] != 5) {
				GetDlgItem(IDC_FVUR_REG1 + i).SetFocus();
				return TRUE;
			}
		}

		if (GetDlgItemText(IDC_FVUR_NAME, buf, ARRAYSIZE(buf)) < 1) {
			GetDlgItem(IDC_NAME).SetFocus();
			return TRUE;
		}
		m_pui->bClickOK = 1;
		m_pui->code[20] = 0;
		m_pui->m_name = buf;
		if (GetDlgItemText(IDC_FVUR_COMPANY, buf, ARRAYSIZE(buf)) < 1)
			buf[0] = 0;
		m_pui->m_company = buf;

		SetRegText(IDS_SEC_USER, IDS_KEY_USERNAME, m_pui->m_name);
		SetRegText(IDS_SEC_USER, IDS_KEY_COMPANY, m_pui->m_company);

		m_pui->retry_counter++;
		EndDialog(IDOK);
		return TRUE;

	case IDCANCEL:
		EndDialog(IDCANCEL);
		::PostMessage(m_pui->hwnd, m_pui->msg, 0, 0);
		break;

	case IDC_FVUR_REG1:
	case IDC_FVUR_REG2:
	case IDC_FVUR_REG3:
	case IDC_FVUR_REG4:
		if (code == EN_CHANGE) {
			int len = GetDlgItemText(id, buf, ARRAYSIZE(buf));
			len = min(5, len);
			int index = id - IDC_FVUR_REG1;
			WideCharToMultiByte(CP_ACP, 0, buf, len, &m_pui->code[index * 5], len + 1, NULL, NULL);
			m_pui->clen[index] = len;
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////
//

static void __fastcall bf_decrypt_pass2(DWORD* data, BF_KEY* key)
{
	register DWORD l,r,*p,*s;

	p = key->P;
	s = &(key->S[0]);
	l = data[0];
	r = data[1];

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

	data[1] = l;
	data[0] = r;
}

/////////////////////////////////////////////////////////////////////
//

#ifdef __USE_CREATETHREAD__
DWORD WINAPI reg_decode_fn(void* pParam)
#else
unsigned __stdcall reg_decode_fn(void* pParam)
#endif
{
	USRINFO* pui = (USRINFO*)pParam;

	pui->calccrc();

	BYTE cb = ((pui->pbk_prx1[0] & 0x0f) << 3) | (pui->pbk_prx1[1] >> 5);

	cb = byte_rangerot_left(cb, 7, pui->index + 1);
	pui->c_prx[1] = (double)cb / pui->tick;
	pui->c_date[1] = (((WORD)pui->pbk_date1[0] & 3) << 12) |
					 ((WORD)pui->pbk_date1[1] << 4) |
					 ((WORD)pui->pbk_date1[2] >> 4);

	PostMessage(pui->hwnd, pui->msg, 2, 0);
#ifdef __USE_CREATETHREAD__
	ExitThread(0);
#else
	_endthreadex(0);
#endif
//	return 0;
}

#ifdef __USE_CREATETHREAD__
static DWORD WINAPI reg_decrypt_pass2_fn(void* pParam)
#else
static unsigned __stdcall reg_decrypt_pass2_fn(void* pParam);
#endif
{
	USRINFO* pui = (USRINFO*)pParam;
	bf_decrypt_pass2(pui->bfdata, (BF_KEY*)pui->bf_key);
	*((DWORD*)pui->bitkey) = pui->bfdata[0];
	*((DWORD*)pui->bitkey + 1) = pui->bfdata[1];
	SETDWORDLONG(pui->key_raw, pui->bfdata[0], pui->bfdata[1]);

	pui->crc = (double)(((WORD)pui->bitkey[0] << 8) | (WORD)pui->bitkey[1]);

	BYTE cb = ((pui->pbk_prx0[0] & 1) << 6) | (pui->pbk_prx0[1] >> 2);
	cb = byte_rangerot_right(cb, 7, pui->index + 1);
	pui->c_prx[0] = (double)cb / pui->tick;
	pui->c_date[0] = (((WORD)pui->pbk_date0[0] & 0x7f) << 7) | 
					 (((WORD)pui->pbk_date0[1] & 0xfe) >> 1);

	PostMessage(pui->hwnd, pui->msg, 1, 0);
#ifdef __USE_CREATETHREAD__
	ExitThread(0);
#else
	_endthreadex(0);
#endif
//	return 0;
}

#ifdef __USE_CREATETHREAD__
DWORD WINAPI reg_valid_fn(void* pParam)
#else
unsigned __stdcall reg_valid_fn(void* pParam)
#endif
{
	USRINFO* pui = (USRINFO*)pParam;
	DWORD seq_2 = (((DWORD)pui->pbk_seq2[0] & 0x1f) << 12) |
				  ((DWORD)pui->pbk_seq2[1] << 4) |
				  ((DWORD)(pui->pbk_seq2[2] & 0xf0) >> 4);
	seq_2 = dword_rangerot_left(seq_2, 17, pui->index + 1);

/* @@ まだやんないよ
	DWORD seq_1 = (((DWORD)pui->seq1[0] << 16) |
				  ((DWORD)pui->seq1[1] << 8) |
				  (DWORD)pui->seq1[2]) >> 7;

	if (seq_1 >= (DWORD)pui->c_date[1] &&
		seq_1 == seq_2) {
		if (((seq_1 - (DWORD)pui->c_date[1]) % 6) == pui->index) {
			WORD w = L2C_YEAR(K2E_YEAR(pui->c_date[0])) +
					 L2C_MON(K2E_MON(pui->c_date[0])) +
					 L2C_DAY(K2E_DAY(pui->c_date[0]));
			if (w == pui->c_date[1]) {
				pui->dates = (K2E_YEAR(pui->c_date[0] << 16) +
							 (K2E_MON(pui->c_date[0]) << 8) +
							  K2E_DAY(pui->c_date[0]);
			}
@@ */
			WORD d1 = (K2E_MON(pui->c_date[0]) << 8) + K2E_DAY(pui->c_date[0]);
			WORD d2 = (L2E_MON(pui->c_date[1]) << 8) + L2E_DAY(pui->c_date[1]);

			double db1 = (double)d1 / pui->crc;
			double db2 = (double)d2 / pui->c_crc;

			seq_2 -= (L2C_YEAR(K2E_YEAR(pui->c_date[0])) +
					 L2C_MON(K2E_MON(pui->c_date[0])) +
					 L2C_DAY(K2E_DAY(pui->c_date[0])));

			// ユーザ登録抹消チェック
			bool bMatch = false;
			for (int n = 0; n < sizeof(sstInvKeys) / sizeof(INVALIDKEYS); n++) {
				if (sstInvKeys[n].seq == seq_2 &&
					sstInvKeys[n].index == (pui->index + 1) &&
					sstInvKeys[n].day == K2E_DAY(pui->c_date[0])) {
#ifdef _DEBUG
dbg.printf(L"invkeys index=%d\n", n);
#endif
					bMatch = true;
					break;
				}
			}
			if (!bMatch && db1 == db2) {
				pui->dates = (K2E_YEAR(pui->c_date[0]) << 16) + d1;
			}
/* @@ まだやんないよ
		}
	}
@@ */
	PostMessage(pui->hwnd, pui->msg, 3, (LPARAM)pui);
#ifdef __USE_CREATETHREAD__
	ExitThread(0);
#else
	_endthreadex(0);
#endif
//	return 0;
}


/////////////////////////////////////////////////////////////////////
//

// このルーチンは lspculの lspRegGetStrJum/lspRegSetStrJumとまったく
// 同じ挙動にしないとだめですよ

#pragma pack(1)

typedef struct tagJUMDATA {
	WORD crc;
	BYTE data[1];
} JUMDATA;

#pragma pack()

int __fastcall fvreg_getjum(char* pStr, int /*cchMax*/)
{
	*pStr = '\0';
	int len;
	BYTE* pd = GetRegBinaryEx(IDS_SEC_USER, IDS_KEY_REGIST, &len);
	if (pd) {
		if (len > 2) {
			len -= 2;
			JUMDATA* jum = (JUMDATA*)pd;
			BYTE* p = jum->data;
			BYTE* q = (BYTE*)pStr;
			for (int i = 0; i < len; i++) {
				*q = *p + 0x1E;
				p++;
				q++;
			}
			*q = '\0';
			if (jum->crc != lccCirRed_us(0, (BYTE*)pStr, len))
				*pStr = '\0';
		}
		free(pd);
	}
	return lstrlenA(pStr);
}

BOOL fvreg_setjum(const char* pStr)
{
	if (pStr) {
		BYTE buf[MAX_PATH];
		JUMDATA* jum = (JUMDATA*)buf;
		int len = lstrlenA(pStr);
		jum->crc = lccCirRed_us(0, (BYTE*)pStr, len);
		BYTE* p = jum->data;
		while (*pStr) {
			*p = (BYTE)*pStr - 0x1E;
			pStr++;
			p++;
		}
		len += 2;
		return SetRegBinary(IDS_SEC_USER, IDS_KEY_REGIST, buf, len);
	}
	return SetRegBinary(IDS_SEC_USER, IDS_KEY_REGIST, NULL, 0);
}

BOOL fvreg_setjum_err(const char* pStr)
{
	if (pStr) {
		BYTE buf[MAX_PATH];
		BYTE* p = buf;
		int len = lstrlenA(pStr);
		WORD crc = lccCirRed_us(1, (BYTE*)pStr, len);		// ここ! 0 --> 1
		p += 2;
		while (*pStr) {
			*p = (BYTE)*pStr - 0x1E;
			pStr++;
			p++;
		}
		*((WORD*)buf) = crc;
		len += 2;
		return SetRegBinary(IDS_SEC_USER, IDS_KEY_REGIST, buf, len);
	}
	return SetRegBinary(IDS_SEC_USER, IDS_KEY_REGIST, NULL, 0);
}

/////////////////////////////////////////////////////////////////////
//

void __fastcall fvreg_regist(HWND hwnd, USRINFO* pui)
{
	CRegistDlg dlg;
	dlg.m_pui = pui;
	dlg.DoModal();
}

void* __fastcall fvreg_initkey()
{
	BF_KEY* pkey = new BF_KEY;
	bf_setkey(pkey, 10, (BYTE*)APP_PRODUCT);
	return (void*)pkey;
}

void __fastcall fvreg_termkey(void* pv)
{
	BF_KEY* pkey = (BF_KEY*)pv;
	delete pkey;
}
