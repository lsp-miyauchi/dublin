// UnpackDlg.h : CUnpackDlg の宣言

#pragma once

#include "resource.h"       // メイン シンボル
#include <atlhost.h>


#define UNPACK_INCLUDEFOLDER		0x00000001
#define UNPACK_NEWORNOTEXIST		0x00000002
#define UNPACK_MULTIVOL				0x00000004
#define UNPACK_NAMEDIR				0x00000008

#define ARC2BIT(at)					(1 << (at))
#define ISARCBIT(at, flg)			((flg) & (1 << (at)))

#pragma pack(1)

typedef struct tagUNPACKDATA {
	TCHAR szPath[MAX_PATH];			// 書庫ファイルのパス
	TCHAR szPassword[MAX_PATH];
	DWORD arcbits;
	DWORD nStat;
	bool bNamedir:1;
	bool bMultivol:1;
} UNPACKDATA;

#pragma pack()


BOOL __fastcall GetUnpackInfo(HWND hwnd, UNPACKDATA* pud);
BOOL __fastcall MakeUnpackCommand(int arctype, UNPACKDATA* pud, LPTSTR pszCmd, int cchMax);


