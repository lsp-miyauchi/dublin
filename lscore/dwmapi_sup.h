////////////////////////////////////////////////////////
// dwmapi_sup.h
// AERO GLASS をサポートするAPIラッパ
//

#pragma once
#ifndef __DWMAPI_SUP_H__
#define __DWMAPI_SUP_H__

#ifndef _DWMAPI_H_
#include <dwmapi.h>
#endif

#ifndef __CDLL_H__
#include "cdll.h"
#endif

class CDwmApiDll : public CDll
{
private:
	HRESULT (WINAPI* m_DwmEnableComposition)(UINT);
	HRESULT (WINAPI* m_DwmIsCompositionEnabled)(BOOL*);
	HRESULT (WINAPI* m_DwmExtendFrameIntoClientArea)(HWND, const MARGINS*);
	HRESULT (WINAPI* m_DwmSetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD);
	HRESULT (WINAPI* m_DwmGetWindowAttribute)(HWND, DWORD, PVOID, DWORD);
	HRESULT (WINAPI* m_DwmGetColorizationColor)(DWORD*, BOOL*);
	BOOL    (WINAPI* m_DwmDefWindowProc)(HWND, UINT, WPARAM, LPARAM, LRESULT*);
	HRESULT (WINAPI* m_DwmEnableBlurBehindWindow)(HWND, const DWM_BLURBEHIND*);
	HRESULT (WINAPI* m_DwmQueryThumbnailSourceSize)(HTHUMBNAIL, PSIZE);
	HRESULT (WINAPI* m_DwmRegisterThumbnail)(HWND, HWND, PHTHUMBNAIL);
	HRESULT (WINAPI* m_DwmUnregisterThumbnail)(HTHUMBNAIL);
	HRESULT (WINAPI* m_DwmUpdateThumbnailProperties)(HTHUMBNAIL, const DWM_THUMBNAIL_PROPERTIES*);

public:
	CDwmApiDll()
	{
		m_DwmEnableComposition = NULL;
		m_DwmIsCompositionEnabled = NULL;
		m_DwmExtendFrameIntoClientArea = NULL;
		m_DwmSetWindowAttribute = NULL;
		m_DwmGetWindowAttribute = NULL;
		m_DwmGetColorizationColor = NULL;
		m_DwmDefWindowProc= NULL;
		m_DwmEnableBlurBehindWindow = NULL;
		m_DwmQueryThumbnailSourceSize = NULL;
		m_DwmRegisterThumbnail = NULL;
		m_DwmUnregisterThumbnail = NULL;
		m_DwmUpdateThumbnailProperties = NULL;

		if (LoadLibrary(_T("dwmapi.dll")) != NULL) {
			(FARPROC&)m_DwmEnableComposition         = FindFunction("DwmEnableComposition");
			(FARPROC&)m_DwmIsCompositionEnabled      = FindFunction("DwmIsCompositionEnabled");
			(FARPROC&)m_DwmExtendFrameIntoClientArea = FindFunction("DwmExtendFrameIntoClientArea");
			(FARPROC&)m_DwmSetWindowAttribute        = FindFunction("DwmSetWindowAttribute");
			(FARPROC&)m_DwmGetWindowAttribute        = FindFunction("DwmGetWindowAttribute");
			(FARPROC&)m_DwmGetColorizationColor      = FindFunction("DwmGetColorizationColor");
			(FARPROC&)m_DwmDefWindowProc             = FindFunction("DwmDefWindowProc");
			(FARPROC&)m_DwmEnableBlurBehindWindow    = FindFunction("DwmEnableBlurBehindWindow");
			(FARPROC&)m_DwmQueryThumbnailSourceSize  = FindFunction("DwmQueryThumbnailSourceSize");
			(FARPROC&)m_DwmRegisterThumbnail         = FindFunction("DwmRegisterThumbnail");
			(FARPROC&)m_DwmUnregisterThumbnail       = FindFunction("DwmUnregisterThumbnail");
			(FARPROC&)m_DwmUpdateThumbnailProperties = FindFunction("DwmUpdateThumbnailProperties");
		}
	}

	HRESULT EnableComposition(UINT uCompositionAction)
	{
		if (!m_DwmEnableComposition) return E_FAIL;
		return m_DwmEnableComposition(uCompositionAction);
	}

	HRESULT IsCompositionEnabled(BOOL* pfEnabled)
	{
		if (!m_DwmIsCompositionEnabled) return E_FAIL;
		return m_DwmIsCompositionEnabled(pfEnabled);
	}

	HRESULT ExtendFrameIntoClientArea(HWND hwnd, const MARGINS* pMarInset)
	{
		if (!m_DwmExtendFrameIntoClientArea) return E_FAIL;
		return m_DwmExtendFrameIntoClientArea(hwnd, pMarInset);
	}

	HRESULT SetWindowAttribute(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute)
	{
		if (!m_DwmSetWindowAttribute) return E_FAIL;
		return m_DwmSetWindowAttribute(hwnd, dwAttribute, pvAttribute, cbAttribute);
	}

	HRESULT GetWindowAttribute(HWND hwnd, DWORD dwAttribute, PVOID pvAttribute, DWORD cbAttribute)
	{
		if (!m_DwmGetWindowAttribute) return E_FAIL;
		return m_DwmGetWindowAttribute(hwnd, dwAttribute, pvAttribute, cbAttribute);
	}

	HRESULT GetColorizationColor(DWORD* pcrColorization, BOOL* pfOpaqueBlend)
	{
		if (!m_DwmGetColorizationColor) return E_FAIL;
		return m_DwmGetColorizationColor(pcrColorization, pfOpaqueBlend);
	}

	BOOL DefWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* plResult)
	{
		if (!m_DwmDefWindowProc) return FALSE;
		return m_DwmDefWindowProc(hWnd, msg, wParam, lParam, plResult);
	}

	HRESULT EnableBlurBehindWindow(HWND hWnd, const DWM_BLURBEHIND* pBlurBehind)
	{
		if (!m_DwmEnableBlurBehindWindow) return E_FAIL;
		return m_DwmEnableBlurBehindWindow(hWnd, pBlurBehind);
	}

	HRESULT QueryThumbnailSourceSize(HTHUMBNAIL hThumbnail, PSIZE pSize)
	{
		if (!m_DwmQueryThumbnailSourceSize) return E_FAIL;
		return m_DwmQueryThumbnailSourceSize(hThumbnail, pSize);
	}

	HRESULT RegisterThumbnail(HWND hwndDestination, HWND hwndSource, PHTHUMBNAIL phThumbnailId)
	{
		if (!m_DwmRegisterThumbnail) return E_FAIL;
		return m_DwmRegisterThumbnail(hwndDestination, hwndSource, phThumbnailId);
	}

	HRESULT UnregisterThumbnail(HTHUMBNAIL hThumbnailId)
	{
		if (!m_DwmUnregisterThumbnail) return E_FAIL;
		return m_DwmUnregisterThumbnail(hThumbnailId);
	}

	HRESULT UpdateThumbnailProperties(HTHUMBNAIL hThumbnailId, const DWM_THUMBNAIL_PROPERTIES* ptnProperties)
	{
		if (!m_DwmUpdateThumbnailProperties) return E_FAIL;
		return m_DwmUpdateThumbnailProperties(hThumbnailId, ptnProperties);
	}
};



#endif				// __DWMAPI_SUP_H__
