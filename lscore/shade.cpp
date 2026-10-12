///////////////////////////////////////////////////////
// shade.cpp

#include "stdafx.h"
#include "lscore.h"
#include "common.h"

#include "staticctrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class CShadeRectCtrl : public CDefStaticCtrlImpl<CShadeRectCtrl>,
						public CThemeImpl<CShadeRectCtrl>
{
private:
	COLORREF m_crs[16];
	int m_numcolors;

public:
	CShadeRectCtrl();

BEGIN_MSG_MAP(CStaticCtrl)
	CHAIN_MSG_MAP(CThemeImpl<CShadeRectCtrl>)
	CHAIN_MSG_MAP(CDefStaticCtrlImpl<CShadeRectCtrl>)
	ON_MESSAGE(SDM_SETCOLORS, OnSetColors)
END_MSG_MAP()

	LRESULT OnSetColors(WPARAM wParam, LPARAM lParam);

	void OnDraw(HDC hdc);
	BOOL OnEraseBkgnd(HDC hdc);
};

CShadeRectCtrl::CShadeRectCtrl()
{
	m_crs[0] = GetSysColor(COLOR_ACTIVECAPTION);
	m_numcolors = 1;

	// Theme
	SetThemeClassList(L"Edit");
	SetThemeExtendedStyle(THEME_EX_THEMECLIENTEDGE, 0);
}

_inline BOOL CShadeRectCtrl::OnEraseBkgnd(HDC hdc)
	{ return TRUE; }

LRESULT CShadeRectCtrl::OnSetColors(WPARAM wParam, LPARAM lParam)
{
	if (!lParam || !wParam) return 0;

	m_numcolors = min((int)wParam, 16);
	memcpy(m_crs, (void*)lParam, m_numcolors * sizeof(COLORREF));
	Invalidate(TRUE);
	UpdateWindow();
	return 0;
}

void CShadeRectCtrl::OnDraw(HDC hdc)
{
	CRect rc;
	GetClientRect(&rc);
	FillShadeRectEx(hdc, rc, m_crs, m_numcolors, GetStyle() & 1);
}

//////////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK ShadeWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CShadeRectCtrl* pWnd = new CShadeRectCtrl();
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}
