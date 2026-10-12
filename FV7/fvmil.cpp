////////////////////////////////////////////////////////////////////////
// FVMIL.CPP

#include "stdafx.h"
#include "unlha32.h"
#include "fv7.h"
#include "mainfrm.h"
#include "..\lsarcsvr\lsarcsvr_i.h"

////////////////////////////////////////////////////////////////////////
//

static int snLastCheck = ARC_ERROR;

int ArcIsSFX(HWND hWnd, LPCTSTR pszFile)
{
	LONG type = ARC_ERROR;
	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(__uuidof(ArcManager));
	if (SUCCEEDED(hr)) {
		BOOL bUnicodeSupported, bUtf8Supported;
		pam->GetArchiveType(pszFile, &type, &bUnicodeSupported, &bUtf8Supported);
	}
	return (int)type;
}


BOOL ArcOpen(HWND hWnd, LPCTSTR pszFileName)
{
	CString str;
	str.Format(_T("\"%s\""), pszFileName);
//	return lccRunDll(hWnd, DLL_ARC, ARC_VIEW, str);
	TCHAR exec[MAX_PATH];
	if (lccRegGetAppPath(_T("arcman.exe"), exec, ARRAYSIZE(exec))) {
		return InvokeFile(hWnd, exec, str);
	}
	return FALSE;
}

BOOL ArcCompress(HWND hWnd, LPCTSTR pszPath, HDROP hdrop)
{
	// 書庫サーバ
	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(__uuidof(ArcManager));
	if (SUCCEEDED(hr)) {
		hr = pam->CompressFilesExec(hWnd, pszPath, (LONG_PTR)hdrop);
	}
	return (SUCCEEDED(hr) ? TRUE : FALSE);
}

BOOL ArcPackAdd(HWND hWnd, LPCTSTR pArc, HDROP hDrop)
{
	CString dir = GetCommonParentDir(hDrop);
	// 書庫サーバ
	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(__uuidof(ArcManager));
	if (SUCCEEDED(hr)) {
		hr = pam->AddFiles(hWnd, pArc, dir.GetString(), (LONG_PTR)hDrop);
	}
	return (SUCCEEDED(hr) ? TRUE : FALSE);
}

BOOL ArcExtract(HWND hWnd, LPCTSTR pszPath, HDROP hdrop)
{
	// 書庫サーバ
	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(__uuidof(ArcManager));
	if (SUCCEEDED(hr)) {
		hr = pam->ExtractFilesExec(hWnd, pszPath, (LONG_PTR)hdrop);
	}
	return (SUCCEEDED(hr) ? TRUE : FALSE);
}

BOOL ArcCheck(HWND hWnd, HDROP hdrop)
{
	// 書庫サーバ
	CComPtr<IArcManager> pam;
	HRESULT hr = pam.CoCreateInstance(__uuidof(ArcManager));
	if (SUCCEEDED(hr)) {
		hr = pam->CheckFiles(hWnd, (LONG_PTR)hdrop);
	}
	return (SUCCEEDED(hr) ? TRUE : FALSE);
}
