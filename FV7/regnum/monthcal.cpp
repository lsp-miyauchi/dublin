#include "stdafx.h"


/////////////////////////////////////////////////////////////////////////////
// CMonthCalCtrl

IMPLEMENT_DYNAMIC(CMonthCalCtrl, CWnd)

BOOL CMonthCalCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	CWnd* pWnd = this;
	return pWnd->Create(MONTHCAL_CLASS, NULL, dwStyle, rect, pParentWnd, nID);
}

CMonthCalCtrl::~CMonthCalCtrl()
{
	DestroyWindow();
}


/////////////////////////////////////////////////////////////////////////////
// CDatePickCtrl

IMPLEMENT_DYNAMIC(CDatePickCtrl, CWnd)

BOOL CDatePickCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	CWnd* pWnd = this;
	return pWnd->Create(DATETIMEPICK_CLASS, NULL, dwStyle, rect, pParentWnd, nID);
}

CDatePickCtrl::~CDatePickCtrl()
{
	DestroyWindow();
}

