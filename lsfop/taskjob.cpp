////////////////////////////////////////////////////////////////////////
// taskjob.cpp

#include "stdafx.h"
#include <initguid.h>
#include <mstask.h>
#include <msterr.h>
#include "fopstd.h"
#include "syncdata.h"
#include "taskjob.h"

#define SECURITY_WIN32
#include <security.h>
#pragma comment(lib, "secur32.lib")

#define SCHED_SERVICE_APP_NAME  TEXT("mstask.exe")
#define SCHED_SERVICE_NAME      TEXT("Schedule")

///////////////////////////////////////////////////////////////////////
//

enum {
	TSA_ERROR = -1,
	TSA_ACTIVE,
	TSA_NOTACTIVE,
};

enum {
	TST_UNDEFINE = 0,
	TST_DAILY,
	TST_WEEKLY,
};

///////////////////////////////////////////////////////////////////////
//

int IsTaskSchedulerActive(bool bStartIfNotRunning)
{
	SC_HANDLE hSC = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (hSC == NULL) {
		return TSA_ERROR;
    }

	DWORD dwDesiredAccess = SERVICE_QUERY_STATUS;
	if (bStartIfNotRunning)
		dwDesiredAccess |= SERVICE_START;

	SC_HANDLE hSchSvc = OpenService(hSC, SCHED_SERVICE_NAME, dwDesiredAccess);
	CloseServiceHandle(hSC);

	if (hSchSvc == NULL) {
		return TSA_ERROR;
	}

    SERVICE_STATUS SvcStatus;
	if (QueryServiceStatus(hSchSvc, &SvcStatus) == FALSE) {
		CloseServiceHandle(hSchSvc);
		return TSA_ERROR;
	}

	if (SvcStatus.dwCurrentState == SERVICE_RUNNING) {
		// The service is already running.
		CloseServiceHandle(hSchSvc);
		return TSA_ACTIVE;
	}

	if (!bStartIfNotRunning) {
		CloseServiceHandle(hSchSvc);
		return TSA_NOTACTIVE;
	}

	if (StartService(hSchSvc, 0, NULL) == FALSE) {
		CloseServiceHandle(hSchSvc);
		return TSA_ERROR;
	}

	CloseServiceHandle(hSchSvc);
	return TSA_ACTIVE;
}

extern "C"
BOOL WINAPI fopGetTaskFolder(LPTSTR pszPath, int cchMax)
{
	BOOL bOK = FALSE;
	ITaskScheduler* pts;
	if (CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void**)&pts) == S_OK) {
		// ダミーJOBを作成
		ITask* pt = NULL;
		if (pts->NewWorkItem(_T("~~dummy"), CLSID_CTask, IID_ITask, (IUnknown**)&pt) == S_OK) {
			IPersistFile* ppf;
			if (pt->QueryInterface(IID_IPersistFile, (void **)&ppf) == S_OK) {
				LPOLESTR p;
				if (ppf->GetCurFile(&p) == S_OK) {
					PathRemoveFileSpec(p);
					lstrcpyn(pszPath, p, cchMax);
					CoTaskMemFree(p);
					bOK = TRUE;
				}
				ppf->Release();
			}
			pt->Release();
		}
		pts->Release();
	}
	return bOK;
}

bool IsTaskJobExists(LPCTSTR pszTaskName)
{
	bool bExists = false;
	ITaskScheduler* pts;
	HRESULT hr = CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void**)&pts);
	if (SUCCEEDED(hr)) {
		// 新しいJOBを作成
		ITask* pt = NULL;
		hr = pts->NewWorkItem(pszTaskName, CLSID_CTask, IID_ITask, (IUnknown**)&pt);
		if (hr == S_OK)
			pt->Release();
		else if (hr == HRESULT_FROM_WIN32 (ERROR_FILE_EXISTS))
			bExists = true;
	}
	pts->Release();
	return bExists;
}

void DeleteTaskJob(LPCTSTR pszTaskName)
{
	ITaskScheduler* pts;
	HRESULT hr = CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void**)&pts);
	if (SUCCEEDED(hr)) {
		pts->Delete(pszTaskName);
		pts->Release();
	}
}

HRESULT MakeTaskJob(HWND hParent, LPCTSTR pszTaskName, LPCTSTR pszApp, LPCTSTR pszParams, LPCTSTR pszWorkdir, LPCTSTR pszAccount, LPCTSTR pszPass, int trigger)
{
	HRESULT hr;
	ITaskScheduler* pts;
	hr = CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void**)&pts);
	if (SUCCEEDED(hr)) {
		// 新しいJOBを作成
		ITask* pt = NULL;
		hr = pts->NewWorkItem(pszTaskName, CLSID_CTask, IID_ITask, (IUnknown**)&pt);
		if (hr == S_OK) {
			// アプリケーション（コマンドライン）をセット
			if (pszApp && *pszApp) {
				hr = pt->SetApplicationName(pszApp);
			}
			// 引数パラメタをセット
			if (pszParams && *pszParams) {
				hr = pt->SetParameters(pszParams);
			}
			// 作業フォルダをセット
			if (pszWorkdir && *pszWorkdir) {
				hr = pt->SetWorkingDirectory(pszWorkdir);
			}
			// アカウント情報をセット
			if (pszAccount && *pszAccount) {
				hr = pt->SetAccountInformation(pszAccount, pszPass);
			}
			if (trigger) {
				WORD wTrigger;
				ITaskTrigger* ptt;
				hr = pt->CreateTrigger(&wTrigger, &ptt);
				if (hr == S_OK) {
					SYSTEMTIME st;
					GetSystemTime(&st);

					TASK_TRIGGER info;
					memset(&info, 0, sizeof(info));
					info.cbTriggerSize = sizeof(info);
					info.wBeginYear = st.wYear;
					info.wBeginMonth = st.wMonth;
					info.wBeginDay = st.wDay;
					info.wStartHour = 18;
					info.wStartMinute = 0;
//					info.MinutesDuration;
//					info.MinutesInterval;
//					info.rgFlags;
					if (trigger == TST_DAILY) {
						info.TriggerType = TASK_TIME_TRIGGER_DAILY;
						info.Type.Daily.DaysInterval = 1;
					}
					else {
						info.TriggerType = TASK_TIME_TRIGGER_WEEKLY;
						info.Type.Weekly.WeeksInterval = 1;
						info.Type.Weekly.rgfDaysOfTheWeek = TASK_FRIDAY;
					}
					ptt->SetTrigger(&info);
					ptt->Release();
				}
			}

			// JOBを保存
			IPersistFile* ppf;
			hr = pt->QueryInterface(IID_IPersistFile, (void **)&ppf);
			if (SUCCEEDED(hr)) {
				hr = ppf->Save(NULL, TRUE);
				ppf->Release();
			}

			// 編集ウィンドウを開く
			hr = pt->EditWorkItem(hParent, 0);

			pt->Release();
		}
		pts->Release();
	}
	return hr;
}


////////////////////////////////////////////////////////////////////////
//

class CTaskJobDlg : public CDialogImpl<CTaskJobDlg>
{
public:
	LPCTSTR m_pszFile;

public:
	CTaskJobDlg() : m_pszFile(NULL) { }
	~CTaskJobDlg() { }

	enum { IDD = IDD_TASKJOB };

BEGIN_MSG_MAP(CTaskJobDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
};

BOOL CTaskJobDlg::OnInitDialog(LPARAM lParam)
{
	if (IsTaskSchedulerActive(false) != TSA_ACTIVE) {
		AppMessage(IDS_TITLE_TASKJOB, IDS_TASKJOB_NOSCHEDULER, NULL, MB_STOP);
		EndDialog(IDCANCEL);
		return FALSE;
	}

	if (!_pathfileexists(m_pszFile)) {
		EndDialog(IDCANCEL);
		return FALSE;
	}
	CenterWindow();

	CSyncData sd;
	sd.Load(m_pszFile);

	SetDlgItemText(IDC_PATH, m_pszFile);
	SetDlgItemText(IDC_TASKJOB_TITLE, sd.m_strTitle);

	// 実行タイプ
	CComboBox combo = GetDlgItem(IDC_TASKJOB_TYPES);
	InitDlgCtrl(combo, MAKEINTRESOURCE(IDS_TASKJOB_TYPE));
	combo.SetCurSel(0);

	// プライオリティ
	combo = GetDlgItem(IDC_TASKJOB_PRIOLITY);
	InitDlgCtrl(combo, MAKEINTRESOURCE(IDS_TASKJOB_PRIOLITY));
	combo.SetCurSel(0);

	// トリガ
	combo = GetDlgItem(IDC_TASKJOB_TRIGGER);
	InitDlgCtrl(combo, MAKEINTRESOURCE(IDS_TASKJOB_TRIGGER));
	combo.SetCurSel(0);

	CheckDlgButton(IDC_TASKJOB_REPORT, TRUE);

	TCHAR name[MAX_PATH];
	ULONG len = ARRAYSIZE(name);
	GetUserNameEx(NameSamCompatible, name, &len);
	SetDlgItemText(IDC_TASKJOB_ACCOUNT, name);

	return TRUE;
}

BOOL CTaskJobDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW && pInfo->iCtrlId) {
		ShowCtrlHelp(GetDlgItem(pInfo->iCtrlId), pInfo->iCtrlId);
		return TRUE;
	}
	return FALSE;
}

void CTaskJobDlg::OnOK()
{
	TCHAR name[MAX_PATH];
	if (GetDlgItemText(IDC_TASKJOB_TITLE, name, ARRAYSIZE(name)) < 1) {
		MessageBeep(0);
		GetDlgItem(IDC_TASKJOB_TITLE).SetFocus();
		return;
	}

	if (IsTaskJobExists(name)) {
		if (AppMessage(IDS_TITLE_TASKJOB, IDS_TASKJOB_EXIST, name, MB_QYESNO) != IDYES)
			return;
		DeleteTaskJob(name);
	}

	// 実行タイプ
	CComboBox combo = GetDlgItem(IDC_TASKJOB_TYPES);
	int synctype = combo.GetCurSel();
	if (synctype < 0) {
		MessageBeep(0);
		combo.SetFocus();
		return;
	}

	// プライオリティ
	combo = GetDlgItem(IDC_TASKJOB_PRIOLITY);
	int prio = combo.GetCurSel();
	if (prio < 0) {
		MessageBeep(0);
		combo.SetFocus();
		return;
	}

	TCHAR acc[MAX_PATH], pass[MAX_PATH];
	if (GetDlgItemText(IDC_TASKJOB_ACCOUNT, acc, ARRAYSIZE(acc)) > 0) {
		if (GetDlgItemText(IDC_TASKJOB_PASSWORD, pass, ARRAYSIZE(pass)) < 1) {
			if (AppMessage(IDS_TITLE_TASKJOB, IDS_TASKJOB_NOPASSWORD, NULL, MB_QYESNO) != IDYES)
				return;
			pass[0] = 0;
		}
	}
	else {
		if (AppMessage(IDS_TITLE_TASKJOB, IDS_TASKJOB_NOACCOUNT, NULL, MB_QYESNO) != IDYES)
			return;
		acc[0] = 0;
	}

	TCHAR app[MAX_PATH * 2], work[MAX_PATH];
	if (!RegGetAppPath(_T("syncman.exe"), app)) {
		AppMessage(IDS_TITLE_TASKJOB, IDS_TASKJOB_NOFRONTEND, NULL, MB_STOP);
		EndDialog(IDCANCEL);
		return;
	}
	GetShortPathName(app, app, ARRAYSIZE(app));
	lstrcpy(work, app);
	PathRemoveFileSpec(work);

	CString str;
	// 実行タイプ（保存・復元・ミラー）
	str.Format(_T("-m%d "), synctype);
	// プライオリティ
	str.AppendFormat(_T("-p%d "), prio);
	// 進捗状況を表示するかどうか
	if (IsDlgButtonChecked(IDC_TASKJOB_MINIMIZE)) str += _T("-i ");
	// 処理結果の表示
	str += IsDlgButtonChecked(IDC_TASKJOB_REPORT) ? _T("-rs ") : _T("-rn ");
	// ログに保存するかどうか
	if (IsDlgButtonChecked(IDC_TASKJOB_SAVELOG)) str += _T("-rf ");
	// 確認にyesを仮定
	str += _T("-y ");

	// 同期ファイルを "%syncbasedir%\fname.fsd" の形式に変換する
	TCHAR cmd[MAX_PATH * 2];
	LPTSTR cmdptr = cmd;
	*cmdptr++ = _T('"');
	lstrcpy(cmdptr, m_pszFile);
	fopSubstSyncPath(cmdptr, ARRAYSIZE(cmd) - 1);
	cmdptr += lstrlen(cmdptr);
	*cmdptr++ = _T('"');
	*cmdptr = 0;
	str += cmd;

	lstrcpy(cmd, str);

	// トリガ
	combo = GetDlgItem(IDC_TASKJOB_TRIGGER);
	int trigger = combo.GetCurSel();

	EnableWindow(FALSE);
	MakeTaskJob(m_hWnd, name, app, cmd, work, acc, pass, trigger);

	EndDialog(IDOK);
}

////////////////////////////////////////////////////////////////////////
//

HRESULT SetupFolderSyncTaskJob(HWND hwndParent, LPCTSTR pszSyncFile)
{
	CTaskJobDlg dlg;
	dlg.m_pszFile = pszSyncFile;
	return (dlg.DoModal(hwndParent) == IDOK) ? S_OK : E_FAIL;
}
