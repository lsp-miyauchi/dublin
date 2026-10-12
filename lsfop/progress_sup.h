////////////////////////////////////////////////////////////////////////
// progress_sup.h

#pragma once
#ifndef __PROGRESS_SUP_H__
#define __PROGRESS_SUP_H__



class CProgressDialog
{
private:
	ULONGLONG m_ullTotal;
	IProgressDialog* m_ppd;
	bool m_running;
public:

	CProgressDialog() : m_ppd(NULL), m_ullTotal(0), m_running(false)
	{
		if (CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_IProgressDialog, (LPVOID*)&m_ppd) != S_OK)
			m_ppd = NULL;
	}
	~CProgressDialog()
	{
		StopProgressDialog();
		if (m_ppd) m_ppd->Release();
	}
	void SetTitle(LPCTSTR pszTitle)
	{
		if (m_ppd) {
			CString str(pszTitle);
			m_ppd->SetTitle(str.GetString());
		}
	}
	void SetTitle(UINT idResource)
	{
		SetTitle(MAKEINTRESOURCE(idResource));
	}
	void SetOperation(SPACTION action)
	{
		if (m_ppd) {
			HINSTANCE hInst = (HINSTANCE)::GetModuleHandle(_T("shell32"));
			switch(action) {
			case SPACTION_SEARCHING_FILES:
				m_ppd->SetAnimation(hInst, 150);
				break;
			case SPACTION_COPYING:
				m_ppd->SetAnimation(hInst, 161);
				break;
			case SPACTION_DELETING:
				m_ppd->SetAnimation(hInst, 164);
				break;
			case SPACTION_APPLYINGATTRIBS:
				m_ppd->SetAnimation(hInst, 165);
				break;
			}
		}
	}
	HRESULT StartProgressDialog(HWND hwndParent, DWORD dwFlags)
	{
		if (m_ppd && !m_running) {
			HRESULT hr = m_ppd->StartProgressDialog(hwndParent, NULL, dwFlags, NULL);
			m_running = (hr == S_OK);
			return hr;
		}
		return E_FAIL;
	}
	HRESULT StopProgressDialog()
	{
		if (m_ppd && m_running) {
			m_running = false;
			return m_ppd->StopProgressDialog();
		}
		return E_FAIL;
	}
	void SetLine(DWORD numLine, LPCTSTR pszString, BOOL fCompactPath)
	{
		if (m_ppd) m_ppd->SetLine(numLine, pszString, fCompactPath, NULL);
	}
	void SetMessage(LPCTSTR pszString)
	{
		if (m_ppd) {
			CString str(pszString);
			SetLine(1, str.GetString(), FALSE);
		}
	}
	BOOL HasUserCancelled()
	{
		if (m_ppd)
			return m_ppd->HasUserCancelled();
		return FALSE;
	}
	void Timer(DWORD dwTimerAction)
	{
		if (m_ppd) m_ppd->Timer(dwTimerAction, NULL);
	}
	void SetProgress(ULONGLONG ullCompleted, ULONGLONG ullTotal)
	{
		m_ullTotal = ullTotal;
		if (m_ppd) m_ppd->SetProgress64(ullCompleted, ullTotal);
	}
	void SetCompletedSize(ULONGLONG ullCompleted)
	{
		if (m_ppd) m_ppd->SetProgress64(ullCompleted, m_ullTotal);
	}
	void SetTotalSize(ULONGLONG ullTotal)
	{
		SetProgress(0, ullTotal);
	}
	operator IProgressDialog*() const { return m_ppd; }
	bool isNull() const { return (m_ppd == NULL); }
};

#endif		// __PROGRESS_SUP_H__
