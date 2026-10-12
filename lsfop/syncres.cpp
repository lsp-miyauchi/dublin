// syncres.cpp : implementation file
//

#include "stdafx.h"
#include "fopstd.h"
#include "syncdata.h"

typedef struct tagCOLHEAD {
	LPTSTR pszText;
	int cx;
} COLHEAD, *LPCOLHEAD;

static COLHEAD cole[3] = {
	{ _T("ファイル"), 240 },
	{ _T("処理"), 240 },
	{ _T("結果"), 240 }
};

typedef struct tagSRWNDPLACE {
	int posHeader[3];
	WINDOWPLACEMENT wpm;
} SRWNDPLACE, *LPSRWNDPLACE;

static TCHAR* szResText[] = {
	_T(""),
	_T("フォルダ作成"),
	_T("フォルダ削除"),
	_T("上書きコピー"),
	_T("新規コピー"),
	_T("ファイル削除"),
	_T("名前変更"),
	_T("スキップ")
};

/////////////////////////////////////////////////////////////////////////////
// SyncResDlgFn

class CSyncResDlg : public CDialogImpl<CSyncResDlg>, public CDialogResize<CSyncResDlg>
{
private:
	gdiplus_using m_gu;
	CListViewCtrl m_list;
	TiXmlDocument* m_pXml;
	COLORREF m_clrSep;
	int m_numErrors;

public:
	CSyncResDlg()
		: m_pXml(NULL)
		, m_numErrors(-1)
		, m_clrSep(calc_mixed_color(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_HIGHLIGHT), 94))
	{
	}
	~CSyncResDlg()
	{
	}

	enum { IDD = IDD_SYNCRESULT };


BEGIN_MSG_MAP(CSyncResDlg)
	ON_WM_INITDIALOG()
	CHAIN_MSG_MAP(CDialogResize<CSyncResDlg>)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST, OnNotifyCustomDraw)
	ON_BN_CLICKED(IDC_CHECK, OnCheckError)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

BEGIN_DLGRESIZE_MAP(CSyncResDlg)
	DLGRESIZE_CONTROL(IDC_LIST,			DLSZ_SIZE_X | DLSZ_SIZE_Y)
	DLGRESIZE_CONTROL(IDC_CHECK,		DLSZ_MOVE_Y)
	DLGRESIZE_CONTROL(IDCANCEL,			DLSZ_MOVE_X | DLSZ_MOVE_Y)
END_DLGRESIZE_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnDestroy();
	void OnNotifyCustomDraw(LPNMHDR phdr, LRESULT* pResult);
	void OnCheckError() { ShowSyncResultReport(); }
	void OnCancel() { EndDialog(IDCANCEL); }
	BOOL ShowSyncResultReport();
};

BOOL CSyncResDlg::OnInitDialog(LPARAM lParam)
{
	DlgResize_Init();
	SetDialogIcon(m_hWnd, IDI_DSYNC);

//	SetMinTrackSize(320,200);
//	SetMaxTrackSize(800,600);

	// イメージリストの設定
	m_list = GetDlgItem(IDC_LIST);
	HIMAGELIST himl = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_SYNCRESULT), 20, RGB(255, 0, 255));
	m_list.SetImageList(himl, LVSIL_SMALL);
	m_list.EnableGroupView(TRUE);

	// カラム設定
	int i;
	for (i = 0; i < 3; i++)
		m_list.InsertColumn(i, cole[i].pszText, LVCFMT_LEFT, cole[i].cx, i);

	// 結果情報の表示
	m_pXml = (TiXmlDocument*)lParam;
	if (!ShowSyncResultReport() || m_list.GetItemCount() < 1)
		EndDialog(IDCANCEL);
	else {
		SRWNDPLACE sw;
		if (GetRegBinary(_MKIR(IDS_SEC_WNDPLACE), _MKIR(IDS_KEY_WP_SYNCREPORT), (LPBYTE)&sw, sizeof(SRWNDPLACE)) == sizeof(SRWNDPLACE)) {
			for (i = 0; i < 3; i++) {
				if (sw.posHeader[i] > 0)
					m_list.SetColumnWidth(i, sw.posHeader[i]);
			}
			sw.wpm.showCmd = SW_SHOWNORMAL;
			lccMapRectToProgmanPane(&sw.wpm.rcNormalPosition);
			SetWindowPlacement(&sw.wpm);
		}
		else
			CenterWindow();
		m_list.SetColumnWidth(1, LVSCW_AUTOSIZE);		// 同期処理種別は自動調整
	}

	if (m_numErrors > 0) {
		CString str;
		GetWindowText(str);
		str.AppendFormat(IDS_ERROROCCURRED, m_numErrors);
		SetWindowText(str);
	}

	return TRUE;
}

void CSyncResDlg::OnDestroy()
{
	SRWNDPLACE sw;
	for (int i = 0; i < 3; i++)
		sw.posHeader[i] = m_list.GetColumnWidth(i);
	sw.wpm.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(&sw.wpm);
	lccUnmapRectFromProgmanPane(&sw.wpm.rcNormalPosition);
	SetRegBinary(_MKIR(IDS_SEC_WNDPLACE), _MKIR(IDS_KEY_WP_SYNCREPORT), &sw, sizeof(SRWNDPLACE));
	RemoveDialogIcon(m_hWnd);
}

void CSyncResDlg::OnNotifyCustomDraw(LPNMHDR phdr, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pcd = (NMLVCUSTOMDRAW*)phdr;
	*pResult = CDRF_DODEFAULT;
	switch (pcd->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
		if (pcd->nmcd.dwItemSpec & 1)
			*pResult = CDRF_DODEFAULT;
		else {
			pcd->clrTextBk = m_clrSep;
			*pResult = CDRF_NEWFONT;
		}
		break;
	default:
		*pResult = CDRF_DODEFAULT;
		break;
	}
}

BOOL CSyncResDlg::ShowSyncResultReport()
{
	// いったん全項目を削除
	m_list.DeleteAllItems();
	if (!m_pXml) return FALSE;

	// 表示する項目はエラー項目のみ？
	bool bErrorOnly = (IsDlgButtonChecked(IDC_CHECK) != FALSE);

	int index = 0;
	int iGroupId = -1;
	int numErrors = 0;
	LV_ITEM lvi;

	CString tit, src, dst;
	TiXmlNode* pDoc = m_pXml->ToDocument();
	if (pDoc) {
		TiXmlElement* pRoot = pDoc->FirstChildElement("log");
		while (pRoot) {
			utf2str(pRoot->Attribute("title"), tit);
			// グループの追加
			utf2str(pRoot->Attribute("source"), src);
			utf2str(pRoot->Attribute("destination"), dst);
			if (tit.IsEmpty())
				tit.Format(_T("%s → %s"), src.GetString(), dst.GetString());
			else
				tit.AppendFormat(_T(" - (%s → %s)"), src.GetString(), dst.GetString());

			LVGROUP group;
			memset(&group, 0, sizeof(group));
#ifdef _WIN64
			group.cbSize = sizeof(group);
#else
			group.cbSize = OsVISTA() ? sizeof(group) : LVGROUP_V5_SIZE;
#endif
			group.mask = LVGF_GROUPID | LVGF_HEADER | LVGF_STATE | LVGF_ALIGN;
			group.stateMask = LVGS_NORMAL | LVGS_COLLAPSIBLE;
			group.state = LVGS_NORMAL | LVGS_COLLAPSIBLE;
			group.uAlign = LVGA_HEADER_LEFT | LVGA_FOOTER_LEFT;
			group.iGroupId = ++iGroupId;
			group.pszHeader = (LPTSTR)tit.GetString();
			m_list.InsertGroup(iGroupId, &group);

			utf2str(pRoot->Attribute("description"), tit);
			memset(&lvi, 0, sizeof(lvi));
			lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_INDENT | LVIF_GROUPID;
			lvi.iGroupId = iGroupId;
			lvi.pszText = (LPTSTR)tit.GetString();
			lvi.iItem = index++;
			m_list.InsertItem(&lvi);

			TiXmlElement* pItem = pRoot->FirstChildElement("item");
			while (pItem) {
				int code = 0;
				pItem->Attribute("code", &code);
				int type = 0;
				pItem->Attribute("type", &type);
				utf2str(pItem->Attribute("name"), tit);

				memset(&lvi, 0, sizeof(lvi));

				bool bSkip = false;
				if (type & REPTYPE_ERROR) {
					lvi.iImage = 12;
					numErrors++;
				}
				else if (bErrorOnly)
					bSkip = true;

				if (!bSkip) {
					lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_INDENT | LVIF_GROUPID;
					lvi.iGroupId = iGroupId;
					lvi.iIndent = 1;
					lvi.pszText = (LPTSTR)tit.GetString();
					lvi.iItem = index++;
					lvi.iImage = (type & ~REPTYPE_MASK);
					if (type & REPTYPE_ERROR)
						lvi.iImage = 12;

					int n = m_list.InsertItem(&lvi);
					m_list.SetItemText(n, 1, szResText[type & ~REPTYPE_MASK]);
					if (code != NOERROR) {
						// エラーコードに対応するメッセージを取得
						LPTSTR pszError;
						if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
											NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pszError, 0, NULL) > 0) {
							m_list.SetItemText(n, 2, pszError);
							LocalFree(pszError);
						}
						else {
							tit.Format(IDS_UNKNOWNERROR, code);
							m_list.SetItemText(n, 2, tit.GetString());
						}
					}
				}
				pItem = pItem->NextSiblingElement("item");
			}
			pRoot = pRoot->NextSiblingElement("log");
		}
	}

	if (m_numErrors < 0)
		m_numErrors = numErrors;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//

void OpenSyncReport(HWND hParent, TiXmlDocument* pXmlDoc)
{
	if (pXmlDoc) {
		CSyncResDlg dlg;
		dlg.DoModal(hParent, (LPARAM)pXmlDoc);
	}
}

TiXmlDocument* LoadSyncReport(LPCTSTR pszFile)
{
	CStringA xml(pszFile);
	TiXmlDocument* pXmlDoc = new TiXmlDocument();
	if (!pXmlDoc->LoadFile( xml )) {
		delete pXmlDoc;
		pXmlDoc = NULL;
	}
	return pXmlDoc;
}

void SaveSyncReport(TiXmlDocument* pXmlDoc)
{
	if (pXmlDoc) {
		TCHAR buf[MAX_PATH];
		SYSTEMTIME st;
		GetLocalTime(&st);

		CString str;
		str.LoadString(IDS_SYNC_FORMATDATE);
		GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, str, buf, ARRAYSIZE(buf));
		int len = lstrlen(buf);
		str.LoadString(IDS_SYNC_FORMATTIME);
		GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, str, buf + len, ARRAYSIZE(buf) - len);
		lstrcat(buf, _T(".synclog"));

		TCHAR name[MAX_PATH * 2];
		GetSyncLogFolder(name, ARRAYSIZE(name));
		PathAppend(name, buf);

		CStringA utf(name);
		pXmlDoc->SaveFile(utf);
	}
}
