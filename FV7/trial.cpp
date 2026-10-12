///////////////////////////////////////////////////////////////////////
// trial.cpp

#include "stdafx.h"
#include "sys\stat.h"
#include "process.h"
#include "fv7.h"
#include "reguserinfo.h"
#include "..\lsftc\lsftc_i.h"

/////////////////////////////////////////////////////////////////////////////
//

static int TRIALKEY = 3;
static const TCHAR sszProp[] = _T("trkprop");

#define LIMIT_OF_TRIAL_DATES			30

/////////////////////////////////////////////////////////////////////////////
//

void __fastcall fvreg_setkeylength(USRINFO* pui)
{
	long limits = 0;
	CComPtr<IFileXferComponent> pt;
	HRESULT hr = pt.CoCreateInstance(__uuidof(FileXferComponent));
	if (SUCCEEDED(hr)) {
		limits = LIMIT_OF_TRIAL_DATES;				// 試用期間を入れて呼び出せば、残りの日数が返る
		if (pt->GetInstance(&limits) != S_OK)
			limits = 0;
	}
	if (limits <= 0 || LIMIT_OF_TRIAL_DATES < limits) {
		TRIALKEY = (limits < -14) ? 12 : 6;
		glFVAttension = 1;
		limits = 0;
	}
	pui->numlimits = limits;
}

/////////////////////////////////////////////////////////////////////////////
//

class CTrialDlg : public CDialogImpl<CTrialDlg>
{
public:
	CTrialDlg();

	enum { IDD = IDD_TRIAL };

	USRINFO* m_pui;

BEGIN_MSG_MAP(CTrialDlg)
	ON_WM_INITDIALOG()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, OnOK);
	ON_BN_CLICKED(IDCANCEL, OnCancel);
	ON_BN_CLICKED(IDC_TRIAL_DOREGIST, OnDoRegist);
	ON_BN_CLICKED(IDC_TRIAL_HELP, OnHelpCall);
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDestroy();
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnHelpCall() { fvApp.HelpTopic(NULL, HB_DLG, IDD); }
	void OnDoRegist();
};

CTrialDlg::CTrialDlg()
{
	m_pui = NULL;
}

BOOL CTrialDlg::OnInitDialog(LPARAM lParam)
{
	if (m_pui->retry_counter > 3)
		GetDlgItem(IDC_TRIAL_DOREGIST).ShowWindow(SW_HIDE);

	CenterWindow();

	TCHAR buf[MAX_PATH];
	// 試用キーをビルド
	srand((unsigned)(LOWORD(GetTickCount())));
	for (int i = 0; i < TRIALKEY; i++)
		buf[i] = (TCHAR)(rand() % 26) + 'A';
	buf[TRIALKEY] = 0;
	ATOM atom = AddAtom(buf);
	SetProp(m_hWnd, sszProp, (HANDLE)atom);
	m_pui->bClickOK = 4;

	CString str;
#ifdef _WIN64
	#ifdef _DEBUG
		str.Format(IDS_TRIAL_LIMIT, m_pui->numlimits);
		::MessageBox(NULL, str, _T("!"), MB_OK);
	#endif
#endif
	if (TRIALKEY > 3) {
		str.Format(IDS_TRIALKEYWORD, buf);
		SetDlgItemText(IDC_TRIAL_TEXT, str);
	} else {
		SetDlgItemText(IDC_TRIAL_KEY, buf);
		GetDlgItem(IDC_TRIAL_KEY).ShowWindow(SW_HIDE);
	}
	if (m_pui->numlimits < 8) {
		// 残り1週間
		str.Format(IDS_TRIAL_LIMIT, m_pui->numlimits);
		SetDlgItemText(IDC_TRIAL_LIMIT, str);
	}
	return TRUE;
}

void CTrialDlg::OnDestroy()
{
	ATOM atom = (ATOM)RemoveProp(m_hWnd, sszProp);
	DeleteAtom(atom);
	::PostMessage(m_pui->hwnd, m_pui->msg, m_pui->bClickOK, 0);
}

void CTrialDlg::OnOK()
{
	TCHAR buf[MAX_PATH], key[MAX_PATH];
	ATOM atom = (ATOM)GetProp(m_hWnd, sszProp);
	GetAtomName(atom, key, ARRAYSIZE(key));
	if (GetDlgItemText(IDC_TRIAL_KEY, buf, ARRAYSIZE(buf)) > 0 && !StrCmp(key, buf)) {
		m_pui->bClickOK = 6;
		EndDialog(IDOK);
	} else {
		MessageBeep(0);
		GetDlgItem(IDC_TRIAL_KEY).SetFocus();
	}
}

void CTrialDlg::OnDoRegist()
{
	m_pui->bClickOK = 5;
	EndDialog(IDOK);
}

/////////////////////////////////////////////////////////////////////////////
//

INT_PTR __fastcall fvreg_trial(HWND hwnd, USRINFO* pui)
{
	if (!FindResource(RESINST, _MKIR(IDD_TRIAL), RT_DIALOG)) {
		fvApp.wndMain.PostMessage(WM_CLOSE);
		return 0;
	}
	CTrialDlg dlg;
	dlg.m_pui = pui;
	return dlg.DoModal();
}
