// calc_hash.cpp : implementation file
//

#include "stdafx.h"
#include "fopstd.h"
#include "calc_hash.h"
#include "mapping_file.h"
#include <ppl.h>

using namespace Concurrency;

static RESLISTHEAD sstCol[] = {
	_MKIR(IDS_HASH_FILENAME),	LVCFMT_LEFT,								80,
	_MKIR(IDS_HASH_FILESIZE),	LVCFMT_RIGHT,								40,
	_T("MD5"),					LVCFMT_LEFT | HDF_CHECKBOX | HDF_CHECKED,	160,
	_T("SHA1"),					LVCFMT_LEFT | HDF_CHECKBOX | HDF_CHECKED,	160,
	_T("SHA256"),				LVCFMT_LEFT | HDF_CHECKBOX | HDF_CHECKED,	160
};

static const TCHAR* sszAborted = _T("*** aborted ***");


/////////////////////////////////////////////////////////////////////////////
// CCalcHashDlg dialog

class CCalcHashDlg : public CDialogImpl<CCalcHashDlg>, public CDialogResize<CCalcHashDlg>
{
private:
	CProgressDialog* m_progress;
	DWORDLONG m_dwlCompletedSize;
	CListViewCtrl m_list;

public:
	CFileArray m_arr;

	CCalcHashDlg() : m_dwlCompletedSize(0), m_progress(NULL) { }
	~CCalcHashDlg() { if (m_progress) delete m_progress; }

	enum { IDD = IDD_CALCHASH };

BEGIN_MSG_MAP(CCalcHashDlg)
	CHAIN_MSG_MAP(CDialogResize<CCalcHashDlg>)
	ON_WM_INITDIALOG()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, OnStart)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_COPY, OnCopy)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CCalcHashDlg)
	DLGRESIZE_CONTROL(IDC_LIST,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDOK,				DLSZ_MOVE_X | DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_X | DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDC_COPY,			DLSZ_MOVE_X | DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

private:
	BOOL OnInitDialog(LPARAM lParam);
	void OnDestroy();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnCopy();
	void OnStart();
	bool ZapMessage();
	int ListupValidColumn(CSimpleArray<BOOL>& flagsArray);

	static BOOL WINAPI __ProgressCallback(DWORDLONG dwlProcessed, DWORDLONG dwlTotal, LPARAM param)
	{
		CCalcHashDlg* pThis = (CCalcHashDlg*)param;
		return pThis->ProgressCallback(dwlProcessed, dwlTotal);
	}

	BOOL ProgressCallback(DWORDLONG dwlProcessed, DWORDLONG dwlTotal);
};

BOOL CCalcHashDlg::OnInitDialog(LPARAM lParam)
{
	DlgResize_Init();

	// イメージリストの設定
	m_list = GetDlgItem(IDC_LIST);
	HIMAGELIST himL, himS;
	Shell_GetImageLists(&himL, &himS);
	m_list.SetImageList(himS, LVSIL_SMALL);
	m_list.SetExtendedListViewStyle(m_list.GetExtendedListViewStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// ヘッダにcheckbox
	if (gosp.OsVISTA()) {
		CHeaderCtrl header = m_list.GetHeader();
		header.ModifyStyle(0, HDS_CHECKBOXES);
	}

	// カラム設定
	for (int i = 0; i < ARRAYSIZE(sstCol); i++) {
		CString label(sstCol[i].label);
		m_list.InsertColumn(i, label, sstCol[i].fmt, sstCol[i].cx, i);
	}

	TCHAR buf[MAX_PATH];
	SHFILEINFO sfi;
	LVITEM lvi;
	int cnt = (int)m_arr.GetCount();
	CFileInfo* pf;
	for (int i = 0; i < cnt; i++) {
		pf = m_arr[i];
		SHGetFileInfo(pf->m_strFullPathName, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

		lvi.mask = LVIF_IMAGE | LVIF_TEXT | LVIF_PARAM;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.iImage = sfi.iIcon;
		lvi.pszText = (LPTSTR)pf->m_pszName;
		lvi.lParam = (LPARAM)pf;
		int index = m_list.InsertItem(&lvi);

		pf->FormatSize(buf, ARRAYSIZE(buf));
		m_list.SetItemText(index, 1, buf);
	}
	m_list.SetColumnWidth(0, LVSCW_AUTOSIZE);
	m_list.SetColumnWidth(1, LVSCW_AUTOSIZE);
	m_list.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	// 位置復帰
	WINDOWPLACEMENT wpm;
	if (GetRegBinary(_MKIR(IDS_SEC_WNDPLACE), _MKIR(IDS_KEY_WP_CALCHASH), (LPBYTE)&wpm, sizeof(wpm)) == sizeof(wpm)) {
		wpm.showCmd = SW_SHOWNA;
		SetWindowPlacement(&wpm);
	}
	CenterWindow();

	return TRUE;
}

void CCalcHashDlg::OnDestroy()
{
	WINDOWPLACEMENT wpm;
	wpm.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&wpm);
	SetRegBinary(_MKIR(IDS_SEC_WNDPLACE), _MKIR(IDS_KEY_WP_CALCHASH), &wpm, sizeof(wpm));
}

static void hexdump(CString& str, BYTE* data, int len)
{
	str.Empty();
	for (int i = 0; i < len; i++)
		str.AppendFormat(_T("%02x"), *data++);
}

bool CCalcHashDlg::ZapMessage()
{
	MSG	msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (IsWindow() && !IsDialogMessage(&msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return true;
}

BOOL CCalcHashDlg::ProgressCallback(DWORDLONG dwlProcessed, DWORDLONG dwlTotal)
{
	ZapMessage();
	if (m_progress && m_progress->HasUserCancelled())
		return FALSE;

	if (m_progress) m_progress->SetCompletedSize(m_dwlCompletedSize + dwlProcessed);
	return TRUE;
}

int CCalcHashDlg::ListupValidColumn(CSimpleArray<BOOL>& flagsArray)
{
	flagsArray.RemoveAll();
	CHeaderCtrl header = m_list.GetHeader();
	HDITEM hditem = { HDI_FORMAT };
	int valids = 0;
	for (int i = 2; i < ARRAYSIZE(sstCol); i++) {
		BOOL ok = TRUE;
		if (gosp.OsVISTA()) {
			header.GetItem(i, &hditem);
			ok = ((hditem.fmt & HDF_CHECKED) != 0);
		}
		else {
			ok = ((sstCol[i].fmt & HDF_CHECKED) != 0);
		}
		flagsArray.Add(ok);
		if (ok) valids++;
	}
	return valids;
}

void CCalcHashDlg::OnStart()
{
	// 計算対象をチェック
	CSimpleArray<BOOL> flags;
	if (ListupValidColumn(flags) == 0) {
		MessageBeep(0);
		return;
	}

	GetDlgItem(IDOK).EnableWindow(FALSE);
	GetDlgItem(IDCANCEL).EnableWindow(FALSE);

	CWaitCursor wc;

	DWORDLONG total = 0;
	int i, numFiles = (int)m_arr.GetCount();
	for (i = 0; i < numFiles; i++) {
		for (int col = 2; col < ARRAYSIZE(sstCol); col++) {
			if (flags[col - 2]) {
				m_list.SetItemText(i, col, NULL);
				total += m_arr[i]->m_dlSize;
			}
		}
	}

	m_dwlCompletedSize = 0;
	m_progress = new CProgressDialog();
	m_progress->SetTitle(MAKEINTRESOURCE(IDS_TITLE_CALCHASH));
	m_progress->StartProgressDialog(m_hWnd, PROGDLG_NOMINIMIZE);
	m_progress->SetProgress(0, total);
	m_progress->SetMessage(MAKEINTRESOURCE(IDS_HASH_CALCULATING));

	BYTE hash[64];
	CCryptHash ch;
	ch.SetProgressCallback(__ProgressCallback, (LPARAM)this);
	mapping_file mf;
	CString calc;
	calc.LoadString(IDS_HASH_CALCULATING);
	CString str;
	for (i = 0; i < numFiles; i++) {
		CFileInfo* pf = m_arr[i];
		if (mf.open(pf->m_strFullPathName)) {
			m_progress->SetLine(2, pf->m_pszName, TRUE);
			if (flags[0]) {
				m_list.SetItemText(i, 2, calc);
				if (!ch.CalcMD5(mf.GetDataPtr(), mf.GetSize(), hash)) {
					m_list.SetItemText(i, 2, sszAborted);
					break;
				} else {
					hexdump(str, hash, 16);
					m_list.SetItemText(i, 2, str);
					m_list.SetColumnWidth(2, LVSCW_AUTOSIZE);
					m_dwlCompletedSize += pf->m_dlSize;
				}
			}
			if (flags[1]) {
				m_list.SetItemText(i, 3, calc);
				if (!ch.CalcSHA1(mf.GetDataPtr(), mf.GetSize(), hash)) {
					m_list.SetItemText(i, 3, sszAborted);
					break;
				} else {
					hexdump(str, hash, 20);
					m_list.SetItemText(i, 3, str);
					m_list.SetColumnWidth(3, LVSCW_AUTOSIZE);
					m_dwlCompletedSize += pf->m_dlSize;
				}
			}
			if (flags[2]) {
				m_list.SetItemText(i, 4, calc);
				if (!ch.CalcSHA256(mf.GetDataPtr(), mf.GetSize(), hash)) {
					m_list.SetItemText(i, 4, sszAborted);
					break;
				} else {
					hexdump(str, hash, 32);
					m_list.SetItemText(i, 4, str);
					m_list.SetColumnWidth(4, LVSCW_AUTOSIZE);
					m_dwlCompletedSize += pf->m_dlSize;
				}
			}
			mf.close();
		}
	}

	m_progress->StopProgressDialog();
	delete m_progress;
	m_progress = NULL;
	GetDlgItem(IDOK).EnableWindow(TRUE);
	GetDlgItem(IDCANCEL).EnableWindow(TRUE);
}

void CCalcHashDlg::OnCopy()
{
	CSimpleArray<BOOL> flags;
	if (ListupValidColumn(flags) == 0) {
		MessageBeep(0);
		return;
	}

	// アルゴリズムごとに一覧を作成
	CString clip, name, str;
	int numFiles = m_list.GetItemCount();
	for (int col = 2; col < ARRAYSIZE(sstCol); col++) {
		clip.AppendFormat(_T("--- [%s] --------------------\r\n"), sstCol[col].label);
		if (flags[col - 2]) {
			for (int i = 0; i < numFiles; i++) {
				m_list.GetItemText(i, 0, name);
				m_list.GetItemText(i, col, str);
				clip.AppendFormat(_T("%s (%s) = %s\r\n"), sstCol[col].label, name.GetString(), str.GetString());
			}
		}
	}
	lccSetStringToClipboard(m_hWnd, clip);
}


/////////////////////////////////////////////////////////////////////
//

extern "C"
void WINAPI fopCalcFileHash(HWND hParent, HDROP hDrop)
{
	int cnt;
	if (!hDrop || (cnt = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0)) < 1)
		return;

	CCalcHashDlg dlg;
	TCHAR buf[MAX_PATH * 2];
	for (int i = 0; i < cnt; i++) {
		CFileInfo* pf = new CFileInfo;
		DragQueryFile(hDrop, i, buf, ARRAYSIZE(buf));
		if (GetFileInfo(buf, *pf) && !pf->IsFolder())
			dlg.m_arr.Add(pf);
		else
			delete pf;
	}
	if (dlg.m_arr.GetCount() > 0)
		dlg.DoModal(hParent);
}
