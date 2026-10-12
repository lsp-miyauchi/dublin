////////////////////////////////////////////////////////////////////////
// util.h

#pragma once
#ifndef __UTIL_H__
#define __UTIL_H__

typedef struct _lkup {
	int key;
	const TCHAR* text;
} lkup;

typedef struct _lkup_val {
	int key;
	int val;
} lkup_val;

typedef struct _lkup_text {
	const char* key;
	const TCHAR* text;
} lkup_text;

USHORT __fastcall msbShort(void* pv);
void __fastcall _str2datetime(const char* datetime, SYSTEMTIME* pst);
LPTSTR __fastcall _append_sec(LPTSTR psz);
LPTSTR __fastcall _append_mm(LPTSTR psz);
LPTSTR __fastcall _append_cm(LPTSTR psz);
LPTSTR __fastcall _append_m(LPTSTR psz);
LPTSTR __fastcall _format_unknown(LPTSTR psz, int n);
LPTSTR __fastcall _format_int(LPTSTR psz, int n, LPCTSTR sfx = NULL);
LPTSTR __fastcall _format_float_real(LPTSTR psz, float f, LPCTSTR sfx = NULL);
LPTSTR __fastcall _format_float(LPTSTR psz, float f, LPCTSTR sfx = NULL);
LPTSTR __fastcall _format_float2(LPTSTR psz, float f, LPCTSTR sfx = NULL);
LPTSTR __fastcall _format_float_int(LPTSTR psz, float f, LPCTSTR sfx = NULL);
LPTSTR __fastcall _format_scale(LPTSTR psz, float f);
LPTSTR __fastcall _format_float_comp(LPTSTR psz, float f, LPCTSTR sfx = NULL);
LPTSTR __fastcall _format_ev_comp(LPTSTR psz, float f);
LPTSTR __fastcall _format_shutter(LPTSTR psz, float f);
LPTSTR __fastcall _format_Fnumber(LPTSTR psz, float f);
LPTSTR __fastcall _format_hex(LPTSTR psz, int n);
LPTSTR __fastcall _format_xy(LPTSTR psz, int x, int y);
LPTSTR __fastcall _format_int_comp(LPTSTR psz, int n, LPCTSTR sfx = NULL);
LPTSTR __fastcall _format_distance(LPTSTR psz, int mm);
LPTSTR __fastcall _format_ascii(LPTSTR psz, const char* str, int cchMax);
bool __fastcall _format_enumtext(LPTSTR psz, int val, LPCTSTR* table, int entries, LPCTSTR prefix = NULL);
bool __fastcall _format_lookup(LPTSTR psz, int val, const lkup* table, int entries = -1, LPCTSTR prefix = NULL);
bool __fastcall _format_string_lookup(LPTSTR psz, const char* key, const lkup_text* table);
void __fastcall _format_bitmask_lookup(LPTSTR psz, int val, const lkup* table, int entries);
int __fastcall _lookup_val(int val, const lkup_val* table);
LPTSTR __fastcall _format_latitude(LPTSTR psz, float d, float m, float s, const char* sufix);

#endif		// __UTIL_H__
