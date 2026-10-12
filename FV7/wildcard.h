// wildcard.h : header file
//

#pragma once

typedef int (CALLBACK* APPLYFILTERPROC)(const CFilterSetting*);

bool SelectFilterSetting(HWND hwnd, bool bAddClear, CFilterSetting& fs, POINT point);
INT_PTR FileFilterDialog(HWND hWnd, CFilterSetting* pfs, LPCTSTR pszCurrentFilter);
INT_PTR DoFilterSelect(HWND hWnd, CFilterSetting* pfs, APPLYFILTERPROC pfnCallback);
int GetFilterHistory(CSimpleArray<CFilterSetting>& list);
void UpdateFilterHistory(CFilterSetting& fs);
void ResetFilterHistory();

