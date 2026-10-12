////////////////////////////////////////////////////////////////////////
// util.cpp

#include "stdafx.h"
#include "util.h"

static const TCHAR sszUnknown[] = _T("不明");

USHORT __fastcall msbShort(void* pv)
{
	BYTE* pb = (BYTE*)pv;
	return ((USHORT)pb[0] << 8) | pb[1];
}

void __fastcall _str2datetime(const char* datetime, SYSTEMTIME* pst)
{
	// 日付時刻文字列からSYSTEMTIME構造体をビルド
	if (!datetime || *datetime == 0 || !pst) return;

	memset(pst, 0, sizeof(SYSTEMTIME));
	char tmp[16];
	char* p = tmp;
	int len = 0;
	while (*datetime && len < 14) {
		if ('0' <= *datetime && *datetime <= '9') {
			*p++ = *datetime;
			len++;
		}
		datetime++;
	}
	*p = 0;
	if (len == 14) {
		int Y, M, D, h, m, s;
		sscanf(tmp, "%04d%02d%02d%02d%02d%02d", &Y, &M, &D, &h, &m, &s);
		pst->wYear = Y;
		pst->wMonth = M;
		pst->wDay = D;
		pst->wHour = h;
		pst->wMinute = m;
		pst->wSecond = s;
	}
}

LPTSTR __fastcall _append_sec(LPTSTR psz)
{
	return lstrcat(psz, _T("秒"));
}

LPTSTR __fastcall _append_mm(LPTSTR psz)
{
	return lstrcat(psz, _T("mm"));
}

LPTSTR __fastcall _append_cm(LPTSTR psz)
{
	return lstrcat(psz, _T("cm"));
}

LPTSTR __fastcall _append_m(LPTSTR psz)
{
	return lstrcat(psz, _T("m"));
}

LPTSTR __fastcall _format_unknown(LPTSTR psz, int n)
{
	wsprintf(psz, _T("%s(%d)"), sszUnknown, n); return psz;
}

LPTSTR __fastcall _format_int(LPTSTR psz, int n, LPCTSTR sfx /* =NULL*/)
{
	wsprintf(psz, _T("%d"), n);
	if (sfx && *sfx) lstrcat(psz, sfx);
	return psz;
}

LPTSTR __fastcall _format_float_real(LPTSTR psz, float f, LPCTSTR sfx /* =NULL*/)
{
	wsprintf(psz, _T("%d.%d"), (int)f, (int)floorf(fabsf(f) * 10) % 10);
	if (sfx && *sfx) lstrcat(psz, sfx);
	return psz;
}

LPTSTR __fastcall _format_float(LPTSTR psz, float f, LPCTSTR sfx /* =NULL*/)
{
	if      (f > 0.0F) f += 0.05F;
	else if (f < 0.0F) f -= 0.05F;
	return _format_float_real(psz, f, sfx);
}

LPTSTR __fastcall _format_float2(LPTSTR psz, float f, LPCTSTR sfx /* =NULL*/)
{
	if      (f > 0.0F) f += 0.005F;
	else if (f > 0.0F) f -= 0.005F;
	wsprintf(psz, _T("%d.%02d"), (int)f, (int)floorf(fabsf(f) * 100) % 100);
	if (sfx && *sfx) lstrcat(psz, sfx);
	return psz;
}

LPTSTR __fastcall _format_float_int(LPTSTR psz, float f, LPCTSTR sfx /* =NULL*/)
{
	if (fmod(f, 1.0F) > 0.05F)
		_format_float(psz, f, sfx);
	else
		_format_int(psz, (int)f, sfx);
	return psz;
}

LPTSTR __fastcall _format_scale(LPTSTR psz, float f)
{
	if (f > 0.0F) {
		_format_float(psz, f, NULL);
		lstrcat(psz, _T("倍"));
	}
	else
		lstrcpy(psz, _T("オフ"));
	return psz;
}

LPTSTR __fastcall _format_float_comp(LPTSTR psz, float f, LPCTSTR sfx /* =NULL*/)
{
	if (f == 0.0F) return lstrcpy(psz, _T("±0"));

	LPTSTR p = psz;
	*p++ = (f < 0.0F) ? _T('-') : _T('+');
	_format_float(p, fabsf(f), sfx);
	return psz;
}

LPTSTR __fastcall _format_ev_comp(LPTSTR psz, float f)
{
	return _format_float_comp(psz, f, _T("EV"));
}

LPTSTR __fastcall _format_shutter(LPTSTR psz, float f)
{
	if (f < 0.3F)
		wsprintf(psz, _T("1/%d"), (int)(0.5F + 1.0F / f));
	else {
		if ((int)floorf((f + 0.05F) * 10.0F) % 10 == 0)
			_format_int(psz, (int)(f + 0.05F), NULL);
		else
			_format_float(psz, f, NULL);
	}
	lstrcat(psz, _T("秒"));
	return psz;
}

LPTSTR __fastcall _format_Fnumber(LPTSTR psz, float f)
{
	LPTSTR p = psz;
	*p++ = _T('F');
	if (f >= 10.0F)
		_format_int(p, (int)(f + 0.5F), NULL);
	else {
		if ((int)floorf((f + 0.05F) * 10.0F) % 10 == 0)
			_format_int(p, (int)(f + 0.05F), NULL);
		else
			_format_float(p, f, NULL);
	}
	return psz;
}

LPTSTR __fastcall _format_hex(LPTSTR psz, int n)
{
	wsprintf(psz, _T("%X"), n); return psz;
}

LPTSTR __fastcall _format_xy(LPTSTR psz, int x, int y)
{
	wsprintf(psz, _T("%d, %d"), x, y); return psz;
}

LPTSTR __fastcall _format_int_comp(LPTSTR psz, int n, LPCTSTR sfx /* =NULL*/)
{
	LPTSTR p = psz;
	if (n == 0)
		*p++ = _T('±');
	else if (n > 0)
		*p++ = _T('+');
	_format_int(p, n, sfx);
	return psz;
}

LPTSTR __fastcall _format_distance(LPTSTR psz, int mm)
{
	// input_value mm is mili-meter
	if (mm >= 1000)
		_format_float(psz, (float)mm / 1000.0F, _T("m"));
	else
		_format_float(psz, (float)mm / 10.0F, _T("cm"));
	return psz;
}

static bool __is_printable_string(const char* str)
{
	while (str && *str) {
		if (*str < 0x20 || *str > 0x7e)
			return false;
		str++;
	}
	return true;
}

LPTSTR __fastcall _format_ascii(LPTSTR psz, const char* str, int cchMax)
{
	if (str && *str) {
		if (__is_printable_string(str))
			MultiByteToWideChar(CP_ACP, 0, str, -1, psz, cchMax);
		else {
			LPTSTR p = psz;
			while (*str && cchMax > 3) {
				int len = wsprintf(p, _T("%02X,"), *(BYTE*)str);
				p += len;
				cchMax -= len;
				str++;
			}
		}
	}
	else
		*psz = 0;
	return psz;
}

bool __fastcall _format_enumtext(LPTSTR psz, int val, LPCTSTR* table, int entries, LPCTSTR prefix)
{
	if (0 <= val && val < entries && table[val] && *table[val]) {
		lstrcpy(psz, table[val]);
		return true;
	}
	if (!prefix) prefix = sszUnknown;
	wsprintf(psz, _T("%s(%d)"), prefix, val);
	return false;
}

bool __fastcall _format_lookup(LPTSTR psz, int val, const lkup* table, int entries, LPCTSTR prefix)
{
	if (entries < 0) {
		while (table->key != INT_MAX) {
			if (table->key == val) {
				lstrcpy(psz, table->text);
				return true;
			}
			table++;
		}
	}
	else {
		while (entries--) {
			if (table->key == val) {
				lstrcpy(psz, table->text);
				return true;
			}
			table++;
		}
	}
	if (prefix)
		lstrcpy(psz, prefix);
	else
		*psz = 0;
	wsprintf(psz + lstrlen(psz), _T("(%d)"), val);
	return false;
}

bool __fastcall _format_string_lookup(LPTSTR psz, const char* key, const lkup_text* table)
{
	while (table->key != NULL) {
		if (memcmp(table->key, key, lstrlenA(table->key)) == 0) {
			lstrcpy(psz, table->text);
			return true;
		}
		table++;
	}
#ifdef UNICODE
	char tmp[MAX_PATH];
	wsprintfA(tmp, "(%s)", key);
	MultiByteToWideChar(CP_ACP, 0, tmp, -1, psz, MAX_PATH);
#else
	wsprintf(psz, "(%s)", key);
#endif
	return false;
}

void __fastcall _format_bitmask_lookup(LPTSTR psz, int val, const lkup* table, int entries)
{
	LPTSTR p = psz;
	*p = 0;
	int n = 1;
	while (n < 0x1000) {
		if (val & n) {
			bool ok = _format_lookup(p, val & n, table, entries);
			if (!ok) {
				*p = 0;
				break;
			}
			p += lstrlen(p);
			*p++ = _T('+');
		}
		n <<= 1;
	}
	if (p > psz)
		*(p - 1) = 0;
}

int __fastcall _lookup_val(int val, const lkup_val* table)
{
	while (table->key != INT_MAX) {
		if (table->key == val) return table->val;
		table++;
	}
	return INT_MAX;
}

LPTSTR __fastcall _format_latitude(LPTSTR psz, float d, float m, float s, const char* sufix)
{
	if ((int)d != 0) {
		TCHAR sec[64], suf[64];
		MultiByteToWideChar(CP_ACP, 0, sufix, -1, suf, ARRAYSIZE(suf));
		_format_float2(sec, s);
		wsprintf(psz, _T("%s %d度%d分%s秒"), suf, (int)d, (int)m, sec);

		// 度分秒から10進経緯度への変換式 = 度 + ((秒 / 60) + 分) / 60
		float f = d + ((s / 60.0F) + m) / 60.0F;
		_stprintf(sec, _T(" (%.6f)"), f);
		lstrcat(psz, sec);
	}
	else
		lstrcpy(psz, _T(" - "));
	return psz;
}

