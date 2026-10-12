////////////////////////////////////////////////////////////////////////
// common.h

#pragma once
#ifndef __LSEXIF_COMMON_H__
#define __LSEXIF_COMMON_H__

extern COSPlatform gosp;

//-----------------------------
// MessageBox attension
//-----------------------------

#define MB_ATTENTION			(MB_OK | MB_ICONEXCLAMATION)
#define MB_QUESTION				(MB_YESNOCANCEL | MB_ICONQUESTION)
#define MB_QYESNO				(MB_YESNO | MB_ICONQUESTION)
#define MB_QOKCANCEL			(MB_OKCANCEL | MB_ICONEXCLAMATION)
#define MB_QABORT				(MB_ICONEXCLAMATION | MB_ABORTRETRYIGNORE)
#define MB_HAND					(MB_OKCANCEL | MB_ICONHAND)
#define MB_STOP					(MB_OK | MB_ICONSTOP)
#define MB_QAST					(MB_OKCANCEL | MB_ICONASTERISK)
#define MB_QRETRY				(MB_RETRYCANCEL | MB_ICONEXCLAMATION)

//-----------------------------
// common.cpp
//-----------------------------

int LastError(UINT idTitle, LPCTSTR pszTitle, UINT nType /*=MB_STOP*/);
void InitDlgCtrl(HWND hctl, UINT idResource);
BOOL PathMakeTree(LPCTSTR pszPath);


//-----------------------------
// browsefolder.cpp
//-----------------------------

int BrowseForFolder(HWND hwnd, LPTSTR pszPath, _U_STRINGorID caption);


//-----------------------------
// reg.cpp
//-----------------------------

HKEY GetSectionKey(LPCTSTR lpszSection);
int GetRegText(_U_STRINGorID sec, _U_STRINGorID key, LPTSTR pszText, int cchMax);
BOOL SetRegText(_U_STRINGorID sec, _U_STRINGorID key, LPCTSTR pszText);
int GetRegValue(_U_STRINGorID sec, _U_STRINGorID key, int nDefValue);
BOOL SetRegValue(_U_STRINGorID sec, _U_STRINGorID key, int nValue);
int GetRegBinary(_U_STRINGorID sec, _U_STRINGorID key, LPVOID lpData, int nLen);
LPBYTE GetRegBinaryEx(_U_STRINGorID sec, _U_STRINGorID key, LPINT lpnLen);
BOOL SetRegBinary(_U_STRINGorID sec, _U_STRINGorID key, LPVOID lpData, int nLen);
LPBYTE GetRegBinarySeqEx(_U_STRINGorID sec, _U_STRINGorID key, int nEntry, LPINT pnLen);
BOOL SetRegBinarySeq(_U_STRINGorID sec, _U_STRINGorID key, int nEntry, void* pd, int nLen);
void RegLoadComboHistory(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key, int nMaxEntry);
BOOL RegSaveComboHistory(HWND hCombo, _U_STRINGorID sec, _U_STRINGorID key, int nMaxEntry, LPCTSTR pszString);


#endif		// __LSEXIF_COMMON_H__
