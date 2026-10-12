//////////////////////////////////////////////////////////////////////
// reg.h

#pragma once

int WINAPIV key_combine(LPTSTR dst, LPCTSTR psz, ... );
ULONG GetRegText(LPCTSTR sec, LPCTSTR vname, LPTSTR psz, ULONG cch);
bool SetRegText(LPCTSTR sec, LPCTSTR vname, LPCTSTR psz);
int GetRegValue(LPCTSTR sec, LPCTSTR vname, int defval);
bool SetRegValue(LPCTSTR sec, LPCTSTR vname, int val);
bool DelRegValue(LPCTSTR sec, LPCTSTR vname);
int GetRegBinary(LPCTSTR sec, LPCTSTR vname, void* data, int size);
//@ void* GetRegBinaryEx(LPCTSTR sec, LPCTSTR vname, int* size);
bool SetRegBinary(LPCTSTR sec, LPCTSTR vname, const void* data, int size);

