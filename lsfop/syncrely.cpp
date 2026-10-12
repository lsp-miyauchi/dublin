#include "stdafx.h"
#include "fopstd.h"
#include "syncdata.h"

void CSyncRelay::TaskbarButtonCreated(ITaskbarList3* pTaskbarList)
{
	if (pTaskbarList != NULL) {
//		HICON hIcon = LoadIcon(RESINST, MAKEINTRESOURCE(IDI_SYNC));
		HICON hIcon = (HICON)LoadImage(RESINST, MAKEINTRESOURCE(IDI_SYNC), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		if (hIcon) {
			pTaskbarList->SetOverlayIcon(m_hWnd, hIcon, NULL);
			DestroyIcon(hIcon);
		}
	}
}

BOOL CSyncRelay::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	SetDialogIcon(m_hWnd, IDI_DSYNC);

	m_anim = GetDlgItem(IDC_AVI);
	m_list = GetDlgItem(IDC_LIST);

	LedClock_Start(GetDlgItem(IDC_CLOCK));
	LedClock_Start(GetDlgItem(IDC_CLOCKTOTAL));

	CRect rc;
	m_list.GetClientRect(&rc);
	rc.right -= GetSystemMetrics(SM_CXVSCROLL);

	m_list.InsertColumn(0, _T(""), LVCFMT_LEFT, rc.Width() - 120, 0);
	m_list.InsertColumn(1, _T(""), LVCFMT_LEFT, 120, 1);

	// イメージリストの設定
	HINSTANCE hInst = RESINST;
	HIMAGELIST him = lccLoadImageList(hInst, RT_RCDATA, MAKEINTRESOURCE(IDB_PRGSTAT), 16, RGB(255, 0, 255));
	m_list.SetImageList(him, LVSIL_SMALL);

	// ダイアログアイコンのアニメーション開始
	startDialogAnimation(IDI_BSPINNER01, 12);

	return TRUE;
}

void CSyncRelay::OnDestroy()
{
	m_anim.Stop();

	// ダイアログアイコンのアニメーション終了
	stopDialogAnimation();

	RemoveDialogIcon(m_hWnd);
}

