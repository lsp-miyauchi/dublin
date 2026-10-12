// cperelay.cpp : implementation file
//

#include "stdafx.h"
#include <limits.h>
#include "fopstd.h"
#include "cperelay.h"


/////////////////////////////////////////////////////////////////////////////
//

static int get_percentage(unsigned long a, unsigned long b)
{
	while (a > 10000000) {
		a >>= 3;
		b >>= 3;
	}
	if (b == 0)
		return 0;
	return ((a * 100) / b);
}

/////////////////////////////////////////////////////////////////////////////
//

CCpeRelay::CCpeRelay()
{
	m_bCanceled = FALSE;
	m_lTotal = 100;
	m_lDone = 0;
}

BOOL CCpeRelay::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	CWindow meter(GetDlgItem(IDC_METER));
	meter.SendMessage(PBM_SETRANGE, 0, MAKELPARAM(0, 100));
	meter.SendMessage(PBM_SETPOS, 0, 0);
	return TRUE;
}

void CCpeRelay::OnCancel()
{
	m_bCanceled = TRUE;
	ShowWindow(SW_HIDE);
}

void CCpeRelay::Destroy()
{
	if (IsWindow())
		DestroyWindow();
}

void CCpeRelay::SetCaption(LPCTSTR pszTitle)
{
	if (IsWindow()) {
		SetWindowText(pszTitle);
		ZapMessage();
	}
}

void CCpeRelay::SetTitle(LPCTSTR pszTitle)
{
	if (IsWindow()) {
		SetDlgItemText(IDC_TITLE, pszTitle);
		ZapMessage();
	}
}

void CCpeRelay::SetFile(LPCTSTR pszFile)
{
	if (IsWindow()) {
		SetDlgItemText(IDC_TEXT, pszFile);
		ZapMessage();
	}
}

void CCpeRelay::SetMessage(LPCTSTR pszMessage)
{
	if (IsWindow()) {
		SetDlgItemText(IDC_MESSAGE, pszMessage);
		ZapMessage();
	}
}

void CCpeRelay::SetRange(long lTotal)
{
	if (IsWindow()) {
		m_lTotal = !lTotal ? 1 : lTotal;
		m_lDone = 0;

		CWindow meter(GetDlgItem(IDC_METER));
		meter.SendMessage(PBM_SETPOS, 0, 0);
		meter.RedrawWindow(NULL, NULL, RDW_UPDATENOW);
		ZapMessage();
	}
}

void CCpeRelay::SetComplete(long lDone)
{
	if (IsWindow()) {
		m_lDone = lDone;

		int par = get_percentage(m_lDone, m_lTotal);
		CWindow meter(GetDlgItem(IDC_METER));
		meter.SendMessage(PBM_SETPOS, (WPARAM)par, 0);
		ZapMessage();
	}
}

BOOL CCpeRelay::isCanceled()
{
	if (IsWindow()) {
		ZapMessage();
		return m_bCanceled;
	}
	return FALSE;
}

void CCpeRelay::ZapMessage()
{
	MSG	msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (!IsDialogMessage(&msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}
