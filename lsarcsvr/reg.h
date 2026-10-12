/////////////////////////////////////////////////////////////////////
// reg.h

#pragma once

extern TCHAR gszAppRegRootKey[];

int GetRegText(_U_STRINGorID sec, _U_STRINGorID key, LPTSTR pszText, int cchMax);
BOOL SetRegText(_U_STRINGorID sec, _U_STRINGorID key, LPCTSTR pszText);
int GetRegValue(_U_STRINGorID sec, _U_STRINGorID key, int nDefValue);
BOOL SetRegValue(_U_STRINGorID sec, _U_STRINGorID key, int nValue);
int GetRegBinary(_U_STRINGorID sec, _U_STRINGorID key, LPVOID lpData, int nLen);
LPBYTE GetRegBinaryEx(_U_STRINGorID sec, _U_STRINGorID key, LPINT lpnLen);
LPBYTE GetRegBinarySeqEx(_U_STRINGorID sec, _U_STRINGorID key, int nEntry, LPINT pnLen);
BOOL SetRegBinary(_U_STRINGorID pSec, _U_STRINGorID pKey, LPVOID lpData, int nLen);
BOOL SetRegBinarySeq(_U_STRINGorID sec, _U_STRINGorID key, int nEntry, void* pd, int nLen);

// void RegLoadComboHistory(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key, int nMaxEntry);
// BOOL RegSaveComboHistory(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key, int nMaxEntry, LPCTSTR pszString);
void RegLoadHistoryEx(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key, int nMaxEntry);
BOOL RegSaveHistoryEx(_U_STRINGorID sec, _U_STRINGorID key, int nMaxEntry, LPCTSTR pszString);
void RegClearHistoryEx(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key);
