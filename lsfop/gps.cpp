// finfo.cpp : implementation file
//

#include "stdafx.h"
#include <strsafe.h>
#include <propvarutil.h>
#include <atldlgs.h>
#include <dbghelp.h>
#include "fopstd.h"

/*
#include <propidl.h>
#include <propkey.h>
#include <propvarutil.h>
#include <atlbase.h>
#include <atlalloc.h>
#include <strsafe.h>
#pragma comment(lib, "propsys.lib")
*/

typedef struct tagMAPURL {
	UINT id;
	LPCWSTR title;
	LPCWSTR url;
} MAPURL, *PMAPURL, FAR *LPMAPURL;

static MAPURL internetMapServices[] = {
	{ 1, L"OpenStreetMap", L"https://openstreetmap.org/?mlat=#LATITUDE#&mlon=#LONGITUDE#&zoom=15" },
	{ 2, L"国土地理院", L"https://maps.gsi.go.jp/?vs=c1&z=16#15/#LATITUDE#/#LONGITUDE#" },
	{ 3, L"Google", L"https://google.co.jp/maps?q=#LATITUDE#,#LONGITUDE#&z=15z" },
	{ 4, L"Yahoo", L"https://map.yahoo.co.jp/place?lat=#LATITUDE#&lon=#LONGITUDE#&zoom=15&maptype=basic" },
	{ 5, L"Bing", L"https://www.bing.com/maps?v=2&q=#LATITUDE#,#LONGITUDE#" },
	{ 6, L"Google earth", L"https://earth.google.com/web/@#LATITUDE#,#LONGITUDE#,0a,20000d,1y,0h,0t,0r" },
	{ 7, L"Mapion", L"https://www.mapion.co.jp/m2/#LATITUDE#,#LONGITUDE#,15" },
	{ 8, L"MapFan", L"https://mapfan.com/map/spots/search?c=#LATITUDE#,#LONGITUDE#,15" }
};

/////////////////////////////////////////////////////////////////////////////
// SelMapDlgFn

class CSelMapDlg : public CDialogImpl<CSelMapDlg>
{
public:
	double m_lon;
	double m_lat;
	CSimpleMap<UINT, CString> m_mapServices;

public:
	CSelMapDlg() : m_lon(0), m_lat(0) { }
	~CSelMapDlg() { }
	enum { IDD = IDD_SELMAP };

BEGIN_MSG_MAP(CRenameDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	COMMAND_SIMPLE_RANGE_HANDLER(IDC_BUTTON1, IDC_BUTTON9, OnSelectMap)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();

		wchar_t slat[256], slon[256];
		StringCchPrintf(slat, ARRAYSIZE(slat), L"%f", m_lat);
		StringCchPrintf(slon, ARRAYSIZE(slon), L"%f", m_lon);
		CString str;
		str.Format(_T("緯度 %s, 経度 %s"), slat, slon);
		SetDlgItemText(IDC_EDIT, str.GetString());

		for (int i = 0; i < ARRAYSIZE(internetMapServices); i++) {
			CWindow ctl = GetDlgItem(IDC_BUTTON1 + i);
			ctl.SetWindowText(internetMapServices[i].title);
			ctl.EnableWindow(TRUE);

			str = internetMapServices[i].url;
			str.Replace(_T("#LONGITUDE#"), slon);
			str.Replace(_T("#LATITUDE#"), slat);
			m_mapServices.Add(IDC_BUTTON1 + i, str);
		}
		return TRUE;
	}
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnSelectMap(UINT id, UINT code, HWND hctl)
	{
		Map(id);
	}
	void Map(UINT ctrlId)
	{
		EndDialog(IDOK);
		CString url = m_mapServices.Lookup(ctrlId);
		ShellExecute(nullptr, nullptr, url.GetString(), nullptr, nullptr, SW_NORMAL);
	}
};

////////////////////////////////////////////////////////////////////////////////

class CPropVariant : public PROPVARIANT
{
public:
	CPropVariant() { PropVariantInit(this); }
	~CPropVariant() { PropVariantClear(this); }
};

HRESULT GetGPSCoordinateAsDecimal(IShellItem2* psi2, REFPROPERTYKEY pkey, REFPROPERTYKEY pkeyRef, double* pdbl)
{
	CPropVariant vr;
	HRESULT hr = psi2->GetProperty(pkey, &vr);
	if (FAILED(hr)) return hr;

	double rgdbl[3];
	ULONG celt;
	hr = PropVariantToDoubleVector(vr, rgdbl, 3, &celt);
	if (FAILED(hr)) return hr;
	if (celt != 3) return E_INVALIDARG;

	double coord = rgdbl[0] + rgdbl[1] / 60.0 + rgdbl[2] / 60.0 / 60.0;

	CComHeapPtr<wchar_t> dir;
	hr = psi2->GetString(pkeyRef, &dir);
	if (FAILED(hr)) return hr;

	if (dir[0] == L'W' || dir[0] == L'S') {
		coord = -coord;
	}
	*pdbl = coord;
	return S_OK;
}

extern "C"
HRESULT WINAPI fopGpsAccess(HWND hwnd, LPCTSTR pszFileName)
{
	CComPtr<IShellItem2> spsi2;
	HRESULT hr;
	hr = SHCreateItemFromParsingName(pszFileName, nullptr, IID_PPV_ARGS(&spsi2));
	if (FAILED(hr)) {
		AppMessage(IDR_FOPSERV, IDS_GPS_NODATA);
		return hr;
	}
	CSelMapDlg dlg;
	hr = GetGPSCoordinateAsDecimal(spsi2, PKEY_GPS_Longitude, PKEY_GPS_LongitudeRef, &dlg.m_lon);
	if (FAILED(hr)) {
		AppMessage(IDR_FOPSERV, IDS_GPS_NODATA);
		return hr;
	}
	hr = GetGPSCoordinateAsDecimal(spsi2, PKEY_GPS_Latitude, PKEY_GPS_LatitudeRef, &dlg.m_lat);
	if (FAILED(hr)) {
		AppMessage(IDR_FOPSERV, IDS_GPS_NODATA);
		return hr;
	}
	dlg.DoModal(hwnd);
	return S_OK;
}

extern "C"
BOOL WINAPI fopHasGpsData(LPCTSTR pszFileName)
{
	CComPtr<IShellItem2> spsi2;
	HRESULT hr;
	hr = SHCreateItemFromParsingName(pszFileName, nullptr, IID_PPV_ARGS(&spsi2));
	if (SUCCEEDED(hr)) {
		CPropVariant vr;
		hr = spsi2->GetProperty(PKEY_GPS_Latitude, &vr);
		if (SUCCEEDED(hr)) {
			double dbl[3];
			ULONG c;
			hr = PropVariantToDoubleVector(vr, dbl, 3, &c);
			return (SUCCEEDED(hr) && (c == 3));
		}
	}
	return FALSE;
}

extern "C"
int WINAPI fopEnumMapServices(ENUMMAPSERVICEPROC cbfn, LONG_PTR lParam)
{
	int entries = ARRAYSIZE(internetMapServices);
	for (int i = 0; i < entries; i++) {
		LPMAPURL pmu = &internetMapServices[i];
		if (!(*cbfn)(pmu->id, pmu->title, lParam)) {
			break;
		}
	}
	return entries;
}

extern "C"
HRESULT WINAPI fopGetMapServiceUrl(UINT id, double lat, double lon, LPTSTR pServiceUrl, int maxLength)
{
	LPMAPURL pTarget = NULL;
	for (int i = 0; i < ARRAYSIZE(internetMapServices); i++) {
		if (internetMapServices[i].id == id) {
			pTarget = &internetMapServices[i];
			break;
		}
	}
	if (!pTarget) {
		return E_FAIL;
	}

	wchar_t slat[256], slon[256];
	StringCchPrintf(slat, ARRAYSIZE(slat), L"%f", lat);
	StringCchPrintf(slon, ARRAYSIZE(slon), L"%f", lon);
	CString url(pTarget->url);
	url.Replace(_T("#LONGITUDE#"), slon);
	url.Replace(_T("#LATITUDE#"), slat);
	StrCpyN(pServiceUrl, url.GetBuffer(), maxLength);
	return S_OK;
}
