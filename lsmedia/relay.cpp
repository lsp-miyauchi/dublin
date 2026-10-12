#include "stdafx.h"
#include "lsmedia.h"
#include "relay.h"

/////////////////////////////////////////////////////////////////////////////
// RelayDlgFn dialog

typedef struct tagRELAY {
	long lTotal;
	long lDone;
	BOOL bCanceled;
	HIMAGELIST hImage;
} RELAY;
typedef RELAY* LPRELAY;
static TCHAR sszRel[] = _T("Relay");

static BOOL ry_OnInitDialog(HWND hdlg, LPARAM lParam);
static void ry_OnCancel(HWND hdlg);
static void ry_ZapMessage(HWND hdlg);
static void ry_OnDestroy(HWND hdlg);

extern "C"
BOOL WINAPI RelayDlgFn(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
			return ry_OnInitDialog(hdlg, lParam);

		case WM_DESTROY:
			ry_OnDestroy(hdlg);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
					ry_OnCancel(hdlg);
					return TRUE;

				default:
					break;
			}
			break;

		default:
			break;
	}
	return FALSE;
}

static BOOL ry_OnInitDialog(HWND hdlg, LPARAM lParam)
{
	lccCenterWindow(hdlg, GetParent(hdlg));
	LPRELAY pr = (LPRELAY)malloc(sizeof(RELAY));
	pr->lTotal = 100;
	pr->lDone = 0;
	pr->bCanceled = FALSE;
	pr->hImage = NULL;
	SetProp(hdlg, sszRel, (HANDLE)pr);
	return TRUE;
}

static void ry_OnCancel(HWND hdlg)
{
//	VAS(VA_CANCEL);

	LPRELAY pr = (LPRELAY)GetProp(hdlg, sszRel);
	pr->bCanceled = TRUE;
	ShowWindow(hdlg, SW_HIDE);
}

static void ry_OnDestroy(HWND hdlg)
{
	LPRELAY pr = (LPRELAY)RemoveProp(hdlg, sszRel);
	if (pr)
	{
		if (pr->hImage)
			ImageList_Destroy(pr->hImage);
		free(pr);
	}
}

static void ry_ZapMessage(HWND hdlg)
{
	LPRELAY pr = (LPRELAY)GetProp(hdlg, sszRel);
	if (pr->bCanceled)
		return;

	MSG	msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (!IsDialogMessage(hdlg, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

void ry_InitBar(HWND hdlg, long lTotal, UINT idAnimate, int cx)
{
	LPRELAY pr = (LPRELAY)GetProp(hdlg, sszRel);
	pr->lTotal = lTotal;
	pr->lDone = 0;
	SendDlgItemMessage(hdlg, IDC_METER, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
	SendDlgItemMessage(hdlg, IDC_METER, PBM_SETPOS, 0, 0);

	if (pr->hImage) {
		SendDlgItemMessage(hdlg, IDC_ANIM, AIM_STOP, 0, 0);
		ImageList_Destroy(pr->hImage);
	}
	pr->hImage = ImageList_LoadImage(ghInst, _MKIR(idAnimate), 36, 0, RGB(255, 0, 255), IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_DEFAULTCOLOR);

	SendDlgItemMessage(hdlg, IDC_ANIM, AIM_SETANIMATION, 100, (LPARAM)pr->hImage);
	SendDlgItemMessage(hdlg, IDC_ANIM, AIM_START, 0, 0);
}

void ry_SetFile(HWND hdlg, LPCTSTR lpFile)
{
	SetDlgItemText(hdlg, IDC_TEXT, lpFile);
	ry_ZapMessage(hdlg);
}

void ry_SetCaption(HWND hdlg, LPCTSTR lpCaption)
{
	SetWindowText(hdlg, lpCaption);
}

void ry_SetComplete(HWND hdlg, long lDone)
{
	LPRELAY pr = (LPRELAY)GetProp(hdlg, sszRel);
	pr->lDone += lDone;
	int par = MulDiv(pr->lDone, 100, pr->lTotal);
	SendDlgItemMessage(hdlg, IDC_METER, PBM_SETPOS, par, 0);
	ry_ZapMessage(hdlg);
}

HWND ry_Create(HWND hwnd)
{
	return CreateDialogParam(ghInst, _MKIR(IDD_RELAY), hwnd, (DLGPROC)RelayDlgFn, 0);
}

void ry_Close(HWND hdlg)
{
	if (IsWindow(hdlg))
		DestroyWindow(hdlg);
}

BOOL ry_IsCanceled(HWND hdlg)
{
	LPRELAY pr = (LPRELAY)GetProp(hdlg, sszRel);
	return pr->bCanceled;
}


