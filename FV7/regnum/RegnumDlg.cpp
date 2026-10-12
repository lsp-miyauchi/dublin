// RegnumDlg.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "Regnum.h"
#include "RegnumDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "..\fvur5\regchk_core.cxx"


/////////////////////////////////////////////////////////////////////////////
// CRegnumDlg ダイアログ

CRegnumDlg::CRegnumDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRegnumDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRegnumDlg)
	m_prefix = REGPREFIX;
	m_seq = 0;
	//}}AFX_DATA_INIT
	// メモ: LoadIcon は Win32 の DestroyIcon のサブシーケンスを要求しません。
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRegnumDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRegnumDlg)
	DDX_Text(pDX, IDC_PREFIX, m_prefix);
	DDX_Text(pDX, IDC_SEQ, m_seq);
	//}}AFX_DATA_MAP

	DDX_Control(pDX, IDC_DATE, m_dtp);
}

BEGIN_MESSAGE_MAP(CRegnumDlg, CDialog)
	//{{AFX_MSG_MAP(CRegnumDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CLIP, OnClip)
	ON_BN_CLICKED(IDC_ISVALID, OnIsvalid)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRegnumDlg メッセージ ハンドラ

BOOL CRegnumDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// 大きいアイコンを設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンを設定

	m_dtp.SetFormat("yyy'/'MM'/'dd");

	UpdateData(FALSE);
	return TRUE;
}

// もしダイアログボックスに最小化ボタンを追加するならば、アイコンを描画する
// コードを以下に記述する必要があります。MFC アプリケーションは document/view
// モデルを使っているので、この処理はフレームワークにより自動的に処理されます。

void CRegnumDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画用のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// クライアントの矩形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンを描画します。
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// システムは、ユーザーが最小化ウィンドウをドラッグしている間、
// カーソルを表示するためにここを呼び出します。
HCURSOR CRegnumDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CRegnumDlg::OnOK() 
{
	if (UpdateData(TRUE))
	{
		SYSTEMTIME st;
		m_dtp.GetSystemTime(&st);

		HMODULE hmod = LoadLibrary("kmreg.dll");
		if (hmod)
		{
			void (WINAPI* pfn)(int, int, int, int, int, LPSTR);
			(FARPROC&)pfn = GetProcAddress(hmod, "KrFV5");
			if (pfn)
			{
				char buf[MAX_PATH];
				(*pfn)(m_prefix, st.wYear, st.wMonth, st.wDay, m_seq, buf);
				SetDlgItemText(IDC_NUMBER, buf);
			}
			else
				AfxMessageBox("APIが見つかりません。");
			FreeLibrary(hmod);
		}
		else
			AfxMessageBox("DLLが見つかりません。");
	}
}

void CRegnumDlg::OnClip() 
{
	char buf[MAX_PATH];
	GetDlgItemText(IDC_NUMBER, buf, MAX_PATH);
	HGLOBAL hmem = GlobalAlloc(GHND, lstrlen(buf) + 1);
	if (hmem)
	{
		LPSTR lp = (LPSTR)GlobalLock(hmem);
		lstrcpy(lp, buf);
		GlobalUnlock(hmem);

		if (OpenClipboard())
		{
			EmptyClipboard();
			SetClipboardData(CF_TEXT, hmem);
			CloseClipboard();
		}
		else
			GlobalFree(hmem);
	}
}

void CRegnumDlg::OnIsvalid() 
{
	if (UpdateData(TRUE))
	{
		char buf[MAX_PATH];
		GetDlgItemText(IDC_NUMBER, buf, MAX_PATH);

		CString cs;
		int dates;

		int nSeq = _check_fnc(m_prefix, buf, &dates);
		if (nSeq < 0)
		{
			cs.Format("エラーです(%d)", nSeq);
			MessageBeep(0);
			AfxMessageBox(cs, MB_OK | MB_ICONSTOP);
		}
		else
		{
			SYSTEMTIME st;
			st.wYear = K2E_YEAR(dates) + 2000;
			st.wMonth = K2E_MON(dates);
			st.wDay = K2E_DAY(dates);
			cs.Format("%05d(%04X)  %d/%02d/%02d",
						nSeq, nSeq, st.wYear, st.wMonth, st.wDay);
			AfxMessageBox(cs, MB_OK | MB_ICONINFORMATION);
		}
	}
}
