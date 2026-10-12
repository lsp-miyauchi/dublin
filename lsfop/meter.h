//////////////////////////////////////////////////////////////////////
// Meter.h

#pragma once
#ifndef __COPYPROGRESS_H__
#define __COPYPROGRESS_H__

#include "animdlgicon.h"
#include "dialogtempl.h"


////////////////////////////////////////////////////////////////////////
//

class CCopyProgressDlg : public CDialogImpl<CCopyProgressDlg>, public CAnimateDialogIcon<CCopyProgressDlg>
{
private:
	DWORDLONG m_dlTotal;
	DWORDLONG m_dlDone;
	DWORDLONG m_dlStartTick;
	DWORDLONG m_dlLastTick;
	CString m_strPath;
	CString m_strInfo;
	CColoredProgressCtrl m_bar;
	CAnimateCtrl m_anim;
	bool m_bAnimIcon:1;
	bool m_bCanceled:1;

public:
	UINT m_idTitle;
	enum { IDD = IDD_RELAY };

	CCopyProgressDlg()
	{
		m_idTitle = 0;
		m_dlTotal = m_dlDone = 0;
		m_dlStartTick = m_dlLastTick = 0;
		m_bAnimIcon = false;
		m_bCanceled = false;
		m_strInfo.Empty();
	}

	~CCopyProgressDlg()
	{
		Close();
	}

	__inline int Message(UINT idText, LPCTSTR pIns = NULL, UINT nType = MB_STOP) { return AppMessage(m_idTitle, idText, pIns, nType); }
	__inline int Error(LPCTSTR pszTitle, UINT nType = MB_STOP) { return LastError(m_idTitle, pszTitle, nType); }

	HWND CreateIndirect(HWND hWndParent, LPCDLGTEMPLATE lpTemplate, LPARAM dwInitParam = NULL)
	{
		ATLASSERT(m_hWnd == NULL);
		_AtlWinModule.AddCreateWndData(&m_thunk.cd, (ATL::CDialogImplBase*)this);
#ifdef _DEBUG
		m_bModal = false;
#endif //_DEBUG
		HWND hWnd = ::CreateDialogIndirectParam(_AtlBaseModule.GetResourceInstance(), lpTemplate,
					hWndParent, CDialogImplBase::StartDialogProc, dwInitParam);
		ATLASSERT(m_hWnd == hWnd);
		return hWnd;
	}

BEGIN_MSG_MAP(CCopyProgressDlg)
	CHAIN_MSG_MAP(CAnimateDialogIcon<CCopyProgressDlg>)
	ON_WM_INITDIALOG()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		CenterWindow();
		SetDialogIcon(m_hWnd, IDI_FILEOPERATION);
		m_bar = GetDlgItem(IDC_METER);
		m_anim = GetDlgItem(IDC_AVI);

		CString cs;
		cs.LoadString(m_idTitle);
		SetWindowText(cs);

		m_bar.SetRange(0, 100);
		m_bar.SetPos(0);
//@		m_anim.Open(GetModuleHandle(gszShell32), 161);
		m_anim.Open(IDA_SHELL_161);

		m_bCanceled = false;
		return TRUE;
	}

	void OnDestroy()
	{
		m_anim.Stop();
		RemoveDialogIcon(m_hWnd);
	}

	void OnCancel()
	{
		m_bCanceled = true;
		ShowWindow(SW_HIDE);
	}

	BOOL Start(HWND hParent, UINT idTitle, DWORDLONG dlTotal)
	{
		m_dlTotal = dlTotal;
		m_idTitle = idTitle;
		m_dlStartTick = m_dlLastTick = GetTickCount64();	// 開始時の時刻
		HWND hwnd = NULL;
		if (::IsWindow(hParent) && ::IsWindowVisible(hParent))
			hwnd = Create(hParent);
		else {
			CDialogTemplate dt(MAKEINTRESOURCE(IDD));
			if (!dt.IsNull()) {
				dt.ModifyStyle(0, WS_MINIMIZEBOX | WS_SYSMENU);
				dt.ModifyExStyle(0, WS_EX_APPWINDOW);
				hwnd = CreateIndirect(hParent, dt);
			}
		}
		return (hwnd != NULL);
	}

	void Close() { if (IsWindow()) DestroyWindow(); }
	bool isCanceled() { return IsWindow() ? m_bCanceled : false; }

	void SetTitle(LPCTSTR pszTitle)
	{
		if (IsWindow()) {
			CString str(pszTitle);
			SetDlgItemText(IDC_TEXT, str.GetString());
			ZapMessage();
		}
	}

	void SetInfo(LPCTSTR pszInfo)
	{
		if (!IsWindow()) return;

		if (!pszInfo) {
			if (m_strInfo.IsEmpty())
				return;
			m_strInfo.Empty();
		}
		else {
			if (!lstrcmp(pszInfo, m_strInfo))
				return;
			m_strInfo = pszInfo;
		}
		SetDlgItemText(IDC_INFO, m_strInfo);
		ZapMessage();
	}

	void SetCompletedSize(DWORDLONG dlDone)
	{
		if (!IsWindow()) return;

		m_dlDone = dlDone;
		int par = (m_dlTotal > 0) ? (int)((m_dlDone * 100) / m_dlTotal) : 100;
		m_bar.SetPos(par);
		DWORDLONG ctk = GetTickCount64();
		if (ctk > m_dlLastTick + 600) {
			m_dlLastTick = ctk;
			DWORDLONG bps = (m_dlDone > 0) ? (m_dlDone * 1000 / (ctk - m_dlStartTick)) : 0;
			TCHAR buf[256];
			lccFormatByteSize64((LONGLONG)bps, buf, ARRAYSIZE(buf));
			lstrcat(buf, _T("/S"));
			SetDlgItemText(IDC_BPS, buf);
		}
		ZapMessage();
	}

	void SetTotalSize(DWORDLONG dlTotal)
	{
		if (!IsWindow()) return;

		m_dlTotal = dlTotal;
		m_dlDone = 0;
		m_bar.SetPos(0);
		ZapMessage();
	}

	void SetFile(WIN32_FIND_DATA* pwfd, LPCTSTR pszName)
	{
		if (!IsWindow()) return;

		DWORDLONG dlTotal;
		SETDWORDLONG(dlTotal, pwfd->nFileSizeLow, pwfd->nFileSizeHigh);
		SetDlgItemText(IDC_TEXT, pwfd->cFileName);

		TCHAR path[MAX_PATH * 2];
		PathGetParent(pszName, path);
		if (m_strPath.IsEmpty() || StrCmpI(m_strPath, path)) {
			m_strPath = path;
			PathSetDlgItemPath(m_hWnd, IDC_PATH, m_strPath);
		}
		SetTotalSize(dlTotal);
	}

	void SetPartCompleted(DWORDLONG dlDone) {
		SetCompletedSize(m_dlDone + dlDone);
	}

	bool ZapMessage()
	{
		MSG	msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (IsWindow() && !IsDialogMessage(&msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (IsWindow()) {
				if (m_bCanceled)
					return false;
			}
		}
		return true;
	}

	static DWORD CALLBACK copy_progress(
		LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred,
		LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred,
		DWORD dwStreamNumber, DWORD dwCallbackReason,
		HANDLE hSourceFile, HANDLE hDestinationFile, LPVOID param
	);
};


////////////////////////////////////////////////////////////////////////
//

class CAnimationProgressDlg : public CDialogImpl<CAnimationProgressDlg>, public CAnimateDialogIcon<CAnimationProgressDlg>
{
private:
	DWORDLONG m_dlTotal;
	DWORDLONG m_dlDone;
	CColoredProgressCtrl m_bar;
	CAnimateCtrl m_anim;
	bool m_bAnimIcon:1;
	bool m_bCanceled:1;
	UINT m_idAnimation;
	CString m_strCaption;

public:
	enum { IDD = IDD_PROGRESS_RELAY };

	CAnimationProgressDlg()
		: m_dlTotal(0)
		, m_dlDone(0)
		, m_bAnimIcon(false)
		, m_bCanceled(false)
		, m_idAnimation(IDA_SHELL_165)
	{ }
	~CAnimationProgressDlg()
	{
		Close();
	}

	int Message(LPCTSTR pszText, LPCTSTR pIns = NULL, UINT typeAction = MB_STOP)
	{
		return AppMessage(m_strCaption, pszText, pIns, typeAction);
	}
	int Error(LPCTSTR pszAddtional, UINT typeAction = MB_STOP)
	{
		DWORD code = GetLastError();
		HWND hwnd = IsWindow() ? m_hWnd : ::GetActiveWindow();
		return (code != NO_ERROR) ? ErrorBox(hwnd, code, m_strCaption, pszAddtional, typeAction) : 0;
	}
	HWND CreateIndirect(HWND hWndParent, LPCDLGTEMPLATE lpTemplate, LPARAM dwInitParam = NULL)
	{
		ATLASSERT(m_hWnd == NULL);
		_AtlWinModule.AddCreateWndData(&m_thunk.cd, (ATL::CDialogImplBase*)this);
#ifdef _DEBUG
		m_bModal = false;
#endif //_DEBUG
		HWND hWnd = ::CreateDialogIndirectParam(_AtlBaseModule.GetResourceInstance(), lpTemplate,
					hWndParent, CDialogImplBase::StartDialogProc, dwInitParam);
		ATLASSERT(m_hWnd == hWnd);
		return hWnd;
	}

BEGIN_MSG_MAP(CAnimationProgressDlg)
	CHAIN_MSG_MAP(CAnimateDialogIcon<CAnimationProgressDlg>)
	ON_WM_INITDIALOG()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		m_bCanceled = false;
		CenterWindow();
		SetDialogIcon(m_hWnd, IDI_FILEOPERATION);
		m_bar = GetDlgItem(IDC_METER);
		m_anim = GetDlgItem(IDC_AVI);

		SetWindowText(m_strCaption);
		m_bar.SetRange(0, 100);
		m_bar.SetPos(0);
		m_anim.Open(m_idAnimation);

		UpdateProgress();
		return TRUE;
	}

	void OnDestroy()
	{
		m_anim.Stop();
		RemoveDialogIcon(m_hWnd);
	}

	void OnCancel()
	{
		m_bCanceled = true;
		ShowWindow(SW_HIDE);
	}

	BOOL Start(HWND hParent)
	{
		HWND hwnd = NULL;
		if (::IsWindow(hParent) && ::IsWindowVisible(hParent))
			hwnd = Create(hParent);
		else {
			CDialogTemplate dt(MAKEINTRESOURCE(IDD));
			if (!dt.IsNull()) {
				dt.ModifyStyle(0, WS_MINIMIZEBOX | WS_SYSMENU);
				dt.ModifyExStyle(0, WS_EX_APPWINDOW);
				hwnd = CreateIndirect(hParent, dt);
			}
		}
		return (hwnd != NULL);
	}

	void Close() { if (IsWindow()) DestroyWindow(); }
	bool isCanceled()
	{
		if (IsWindow()) {
			ZapMessage();
			return m_bCanceled;
		}
		return false;
	}

	void SetAnimation(UINT idResource)
	{
		if (m_idAnimation != idResource) {
			m_idAnimation = idResource;
			if (IsWindow()) {
				m_anim.Stop();
				m_anim.Open(m_idAnimation);
			}
		}
	}
	void SetCaption(LPCTSTR pszTitle)
	{
		CString str(pszTitle);
		m_strCaption = str.GetString();
		if (IsWindow())
			SetWindowText(m_strCaption.GetString());
	}
	void SetTitle(LPCTSTR pszTitle)
	{
		if (IsWindow()) {
			CString str(pszTitle);
			SetDlgItemText(IDC_TEXT, str.GetString());
		}
	}
	void SetLine(int nLine, LPCTSTR pszText)
	{
		if (IsWindow()) {
			SetDlgItemText(((nLine == 2) ? IDC_LINE2 : IDC_LINE1), pszText);
			ZapMessage();
		}
	}
	void SetCompletedSize(DWORDLONG dlDone)
	{
		m_dlDone = dlDone;
		UpdateProgress();
	}
	void UpdateProgress()
	{
		if (IsWindow()) {
			int par = (m_dlTotal > 0) ? (int)((m_dlDone * 100) / m_dlTotal) : 100;
			m_bar.SetPos(par);
		}
	}
	void SetTotalSize(DWORDLONG dlTotal)
	{
		m_dlTotal = dlTotal;
		m_dlDone = 0;
		if (IsWindow()) {
			m_bar.SetPos(0);
		}
	}

private:
	bool ZapMessage()
	{
		MSG	msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (IsWindow() && !IsDialogMessage(&msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			if (IsWindow()) {
				if (m_bCanceled)
					return false;
			}
		}
		return true;
	}
};



#endif				// __COPYPROGRESS_H__
