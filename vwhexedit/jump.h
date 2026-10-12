////////////////////////////////////////////////////////////////////////
// jump.h

#pragma once
#ifndef __JUMPDIALOG_H__
#define __JUMPDIALOG_H__

#ifndef ISHEX
#define ISHEX(a)		(('0'<=(a)&&(a)<='9')||('a'<=(a)&&(a)<='f')||('A'<=(a)&&(a)<='F'))
#endif

/////////////////////////////////////////////////////////////////////////////
// CJumpDlg ダイアログ

class CJumpDlg : public CDialogImpl<CJumpDlg>
{
public:
	CString m_strOffset;
	long m_offset;
	int m_origin;

public:
	CJumpDlg()
	{
		m_offset = 0;
		m_origin = SEEK_SET;
	}
	~CJumpDlg() { }

	enum { IDD = IDD_JUMP };

BEGIN_MSG_MAP(CJumpDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDC_ENDIAN, OnEndian)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();
		CheckDlgButton(IDC_JUMPSET, TRUE);

		if (!m_strOffset.IsEmpty())
			SetDlgItemText(IDC_EDIT, m_strOffset);
		return TRUE;
	}
	void OnEndian()
	{
		// BigEndian とLittleEndianをトグルする
		TCHAR buf[MAX_PATH * 2];
		if (GetDlgItemText(IDC_EDIT, buf, ARRAYSIZE(buf)) < 1) {
			MessageBeep(0);
			GetDlgItem(IDC_EDIT).SetFocus();
			return;
		}

		// マイナスチェック
		bool minus = false;
		TCHAR* p = buf;
		while (*p) {
			if (ISHEX(*p)) break;

			switch (*p) {
			case _T('-'): minus = true; break;
			case _T('+'): minus = false; break;
			}
			p++;
		}
		if (!StrCmpNI(p, _T("0x"), 2)) p += 2;

		// HEXチェック
		TCHAR* q = p;
		bool valid = true;
		while (*q && valid) {
			valid = ISHEX(*q);
			q++;
		}
		if (!valid) {
			MessageBeep(0);
			GetDlgItem(IDC_EDIT).SetFocus();
			return;
		}

		int len = lstrlen(p);
		if (len < 3 || len > 8) return;

		// 16進文字列 → 10進数値
		CString str;
		str.Format(_T("0x%s"), p);
		int val;
		StrToIntEx(str, STIF_SUPPORT_HEX, &val);

		str.Empty();
		if (minus) str = _T("-");

		if (len <= 4) {
			if      (len == 3) val <<= 4;
			val = ((val << 8) & 0xff00) | ((val >> 8) & 0xff);
			str.AppendFormat(_T("%04X"), val);
		}
		else {
			if      (len == 5) val <<= 12;
			else if (len == 6) val <<= 8;
			else if (len == 7) val <<= 4;

			WORD h = HIWORD(val);
			WORD l = LOWORD(val);
			h = ((h << 8) & 0xff00) | ((h >> 8) & 0xff);
			l = ((l << 8) & 0xff00) | ((l >> 8) & 0xff);
			val = MAKELONG(h, l);
			str.AppendFormat(_T("%08X"), val);
		}
		GetDlgItem(IDC_EDIT).SetWindowText(str);
	}

	void OnCancel() { EndDialog(IDCANCEL); }
	void OnOK()
	{
		TCHAR buf[MAX_PATH * 2];
		if (GetDlgItemText(IDC_EDIT, buf, ARRAYSIZE(buf)) < 1) {
			MessageBeep(0);
			GetDlgItem(IDC_EDIT).SetFocus();
			return;
		}

		// マイナスチェック
		TCHAR* p = buf;
		while (*p && *p == _T(' ')) p++;
		bool minus = (*p == _T('-'));
		if (minus) p++;

		// 16進 → 10進
		CString str;
		if (StrCmpNI(p, _T("0x"), 2))
			str = _T("0x");
		str += p;
		LONGLONG offset;
		StrToInt64Ex(str, STIF_SUPPORT_HEX, &offset);

		// StrToIntExの16進判断はマイナスを無視するので
		// 現在位置からのオフセットであればそれをマイナス補正する
		if (minus)
			offset *= -1;

		m_origin = IsDlgButtonChecked(IDC_JUMPSET) ? SEEK_SET : SEEK_CUR;
		if (m_origin == SEEK_SET && offset < 0) {
			MessageBeep(0);
			GetDlgItem(IDC_EDIT).SetFocus();
			return;
		}

		m_offset = (long)offset;
		EndDialog(IDOK);
	}
};

#endif		// __JUMPDIALOG_H__
