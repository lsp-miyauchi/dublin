// regwnd.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include <process.h>
#include "fv7.h"
#include "regwnd.h"

#ifdef _DEBUG
//@ #define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void reg_decrypt_pass1(void* pParam);
void reg_decrypt_pass2(void* pParam);
#ifdef __USE_CREATETHREAD__
	DWORD WINAPI reg_decode_fn(void* pParam);
	DWORD WINAPI reg_valid_fn(void* pParam);
#else
	unsigned __stdcall reg_decode_fn(void* pParam);
	unsigned __stdcall reg_valid_fn(void* pParam);
#endif
void* __fastcall fvreg_initkey();
void __fastcall fvreg_regist(HWND hwnd, USRINFO* pui);
INT_PTR __fastcall fvreg_trial(HWND hwnd, USRINFO* pui);
void __fastcall fvreg_setkeylength(USRINFO* pui);
void __fastcall fvreg_termkey(void* pv);
int __fastcall fvreg_getjum(char* pStr, int cchMax);
BOOL fvreg_setjum(const char* pStr);
BOOL fvreg_setjum_err(const char* pStr);

typedef BOOL (* _REGPROC)(const char*);
static _REGPROC spfnReg[2];

////////////////////////////////////////////////////////////////////////
// IQueryContinue

class CQueryContinue : public IQueryContinue
{
protected:
	ULONG m_ref;					// Object reference count
	ULONGLONG m_delay;
	ULONGLONG m_started;

public:
	CQueryContinue(ULONGLONG d = 0)
			: m_ref(0)
			, m_delay(0)
			, m_started(0)
	{
		SetTimeout(d);
	}

	void SetTimeout(ULONGLONG d)
	{
		m_delay = d;
		m_started = GetTickCount64();
	}

	bool TimeoutReached() const
	{
		bool ok = false;
		if (m_delay)
			ok = (GetTickCount64() - m_started) > m_delay;
		return ok;
	}

	STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppv)
	{
		if (riid == IID_IUnknown || riid == IID_IQueryContinue) {
			*ppv = this;
			AddRef();
			return NOERROR;
		}
		return ResultFromScode(E_NOINTERFACE);
	}

	STDMETHODIMP_(ULONG) AddRef(void)
	{
		return (++m_ref);
	}

	STDMETHODIMP_(ULONG) Release(void)
	{ 
		ULONG refT = --m_ref;
		if (0L == m_ref)
			delete this;
		return refT;
	}

	STDMETHODIMP QueryContinue(VOID)
	{
		HRESULT hr = S_OK; //continue is the default
		if (TimeoutReached()) {
			hr = S_FALSE;
		}
		return hr;
	}
};


////////////////////////////////////////////////////////////////////////
// Show User Notification

HRESULT NotifyUser(UINT idIcon, UINT idTitle, UINT idCaption, UINT idText)
{
	CComPtr<IUserNotification> pun;
	HRESULT hr = CoCreateInstance(CLSID_UserNotification, 0, CLSCTX_ALL, IID_PPV_ARGS(&pun));
	if (SUCCEEDED(hr)) {
		CString str;
		HICON hIcon = NULL;
		if (idIcon && idTitle) {
			if (idIcon)
				hIcon = (HICON)::LoadImage(RESINST, MAKEINTRESOURCE(idIcon), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
			str.LoadString(idTitle);
			pun->SetIconInfo(hIcon, str);
		}

		CString cap;
		cap.LoadString(idCaption);
		str.LoadString(idText);
		pun->SetBalloonInfo(cap, str, NIIF_INFO);

		// Looks like it controls what happends when the X button is clicked on
		pun->SetBalloonRetry(0, 250, 0);

		CQueryContinue* pqc = new CQueryContinue(16000);
		hr = pun->Show(pqc, 250);
		if (hIcon)
			DestroyIcon(hIcon);
	}
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// レジストリ登録用ダミーエントリ

static BOOL ___reg_dummy(const char* /*pStr*/)
{
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CRegWnd

CRegWnd::CRegWnd()
{
	m_pfnRegist = ___reg_dummy;
	spfnReg[0] = fvreg_setjum_err;
	spfnReg[1] = fvreg_setjum;
	m_bRegistResult = FALSE;

	// 試用チェック構造体初期化
	m_pui = new USRINFO;
}

CRegWnd::~CRegWnd()
{
	if (m_pui) {
		if (m_pui->bf_key)
			fvreg_termkey(m_pui->bf_key);
		delete m_pui;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CRegWnd メッセージ ハンドラ

/////////////////////////////////////////////////////////////////////
//

LRESULT CRegWnd::OnRegCheck1st(WPARAM wParam, LPARAM /*lParam*/)
{
#ifdef __USE_CREATETHREAD__
	DWORD id;
#else
	unsigned id;
#endif
	HANDLE thread;

	m_pui->msg = nRM_REGCHECK1ST;
	switch (wParam) {
	case 0:
		if (m_pui->code[0] == '\0') {		// レジストリにはキーが登録されていないので試用画面を出す
			PostMessage(nRM_REGDIALOG, 0);
		}
		else {								// レジストリのキーが正しいかパス１でチェック
			reg_decrypt_pass1(m_pui);
		}
		break;

	case 1:
		if (m_pui->key_raw) {				// パス１がOKなのでパス２を実行
#ifdef __USE_CREATETHREAD__
			thread = CreateThread(NULL, 0, reg_decode_fn, m_pui, 0, &id);
#else
			thread = (HANDLE)_beginthreadex(NULL, 0, reg_decode_fn, m_pui, 0, &id);
#endif
			SendMessage(nRM_REGADDTHREAD, 0, (LPARAM)thread);
		}
		else {								// パス１がNGなので試用画面を出す
			PostMessage(nRM_REGDIALOG, 0);
		}
		break;

	case 2:
		// 妥当性チェック
// @@ まだチェックしないよ
//		if (m_pui->c_prx[0] == m_pui->prx && m_pui->c_prx[0] == m_pui->c_prx[1]) {
		if (m_pui->c_prx[0] == m_pui->prx) {
#ifdef __USE_CREATETHREAD__
			thread = CreateThread(NULL, 0, reg_valid_fn, m_pui, 0, &id);
#else
			thread = (HANDLE)_beginthreadex(NULL, 0, reg_valid_fn, m_pui, 0, &id);
#endif
			SendMessage(nRM_REGADDTHREAD, 0, (LPARAM)thread);
		}
		else {								// キーがNGなので試用画面を出す
			PostMessage(nRM_REGDIALOG, 0);
		}
		break;

	case 3:
		if (!m_pui->dates) {				// キーがNGなので試用画面を出す
			PostMessage(nRM_REGDIALOG, 0);
		}
		else {							// キーがOKなのでこのウィンドウを削除
			glFVValid = 1;
			GetParent().SendMessage(nRM_REGOK, 0, 0);
			DestroyWindow();
		}
		break;

	default:
		break;
	}
	return 0;
}

LRESULT CRegWnd::OnRegDialog(WPARAM wParam, LPARAM /*lParam*/)
{
#ifdef __USE_CREATETHREAD__
	DWORD id;
#else
	unsigned id;
#endif

	HANDLE thread;
	m_pui->msg = nRM_REGDIALOG;
	switch (wParam) {
	case 0:
		m_pui->key_raw = 0;
		fvreg_setkeylength(m_pui);
		fvreg_trial(GetParent(), m_pui);
		break;

	case 4:								// 試用画面からキャンセル戻り
		fvApp.ResetTSR();
		GetParent().PostMessage(WM_CLOSE, 0, 0);
		DestroyWindow();
		break;

	case 5:								// 試用画面から「いますぐ登録」戻り
		fvreg_regist(GetParent(), m_pui);
		break;

	case 6:								// 試用画面から試用キーＯＫ戻り
		DestroyWindow();
		break;

	case 1:
		if (m_pui->key_raw) {
#ifdef __USE_CREATETHREAD__
			thread = CreateThread(NULL, 0, reg_decode_fn, m_pui, 0, &id);
#else
			thread = (HANDLE)_beginthreadex(NULL, 0, reg_decode_fn, m_pui, 0, &id);
#endif
			SendMessage(nRM_REGADDTHREAD, 0, (LPARAM)thread);
		}
		else
			PostMessage(nRM_REGDIALOG, 0);
		break;

	case 2:
// @@ まだチェックしないよ
//		if (m_pui->c_prx[0] == m_pui->prx && m_pui->c_prx[0] == m_pui->c_prx[1])
		if (m_pui->c_prx[0] == m_pui->prx) {
#ifdef __USE_CREATETHREAD__
			thread = CreateThread(NULL, 0, reg_valid_fn, m_pui, 0, &id);
#else
			thread = (HANDLE)_beginthreadex(NULL, 0, reg_valid_fn, m_pui, 0, &id);
#endif
			SendMessage(nRM_REGADDTHREAD, 0, (LPARAM)thread);
		}
		else							// キーがNGなので試用画面を出す
			PostMessage(nRM_REGDIALOG, 0);
		break;

	case 3:
		{
// @@ まだやんないよ
//		SYSTEMTIME st;
//		memset(st, 0, sizeof(st));
//		st.wYear = (m_pui->dates >> 16) + 1980;
//		st.wMon = (m_pui->dates >> 8) & 0xff;
//		st.wDay = m_pui->dates & 0xff;
//		if (m_pui->dates && SystemTimeToFileTime(&st)) {
		if (m_pui->dates) {				// 入力されたキーがOKなので、トップフレームで登録完了画面を出す
			glFVValid = 1;
			int n = (m_pui->c_prx[0] != m_pui->c_prx[1]) ? 0 : 1;

			m_pfnRegist = spfnReg[n];

			GetParent().SendMessage(nRM_REGOK, 0, 0);
			GetParent().PostMessage(nRM_REGTHANKS, 0, 0);
			DestroyWindow();
		}
		else							// キーがNGなので、試用画面に戻る
			PostMessage(nRM_REGDIALOG, 0);
		break;
		}

	default:
		break;
	}
	return 0;
}

LRESULT CRegWnd::OnRegAddThread(WPARAM wParam, LPARAM lParam)
{
	HANDLE thread = (HANDLE)lParam;
	if (thread) {
		m_threadarray.Add(thread);
	}
	return 0;
}

int CRegWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	m_pui->bf_key = fvreg_initkey();
	m_pui->hwnd = m_hWnd;
	return 0;
}
