// RepName.cpp : implementation file
//

#include "stdafx.h"
#include "fopstd.h"
#include "rexreplace.h"
#include "meter.h"


/////////////////////////////////////////////////////////////////////////////
//

typedef struct tagELHD {
	int cx;
	UINT ids;
} ELHD;

static ELHD sstHead[] = {
	180, IDS_HEADER_OLDNAME,
	180, IDS_HEADER_NEWNAME,
};

typedef struct __HIMMAP {
	UINT idCtrl;
	UINT idRes;
	UINT idTips;
} _HIMMAP;

static const _HIMMAP himmap[] = {
	{ IDC_PREVSETTING,	IDB_PREVSETTING,	IDS_PREVSETTING },
};

#define MAXHISTORY			16
#define CMD_CLEAR_HISTORY	9999

/////////////////////////////////////////////////////////////////////////////
//

class CRepName
{
public:
	CRepName() : m_bNoCase(false), m_bReplaceAll(false), m_bUseRegexp(false), m_bSaveExtension(false) { };
	~CRepName() {}

	CString m_src;
	CString m_dst;
	bool m_bNoCase;
	bool m_bReplaceAll;
	bool m_bUseRegexp;
	bool m_bSaveExtension;

	bool operator==(const CRepName& a) const {
		return (((m_src.IsEmpty() && a.m_src.IsEmpty()) || !m_src.Compare(a.m_src)) &&
				((m_dst.IsEmpty() && a.m_dst.IsEmpty()) || !m_dst.Compare(a.m_dst)) &&
				(m_bNoCase == a.m_bNoCase) &&
				(m_bReplaceAll == a.m_bReplaceAll) &&
				(m_bUseRegexp == a.m_bUseRegexp) &&
				(m_bSaveExtension == a.m_bSaveExtension));
	}

	CString label()
	{
		CString na;
		na.LoadString(IDS_UNDEFINED);
		CString str;
		str.Format(_T("%s → %s"), m_src.IsEmpty() ? na.GetString() : m_src.GetString(), m_dst.IsEmpty() ? na.GetString() : m_dst.GetString());
		if (!m_bNoCase) { str += _T(", 大小区別"); }
		if (m_bReplaceAll) { str += _T(", 全置換"); }
		if (m_bUseRegexp) { str += _T(", 正規表現"); }
		if (m_bSaveExtension) { str += _T(", 除拡張子"); }
		return str;
	}
};

//////////////////////////////////////////////////////////////////////
//

static CString repstr(LPCTSTR pText, LPCTSTR src, LPCTSTR dst, bool bNoCase, bool bReplaceAll)
{
	CString str;
	if (src == NULL || *src == 0) {
		if (*dst == _T('>')) {
			str = pText;
			str += dst + 1;
		} else {
			if (*dst == _T('<'))
				dst++;
			str = dst;
			str += pText;
		}
		return str;
	}

	LPTSTR (WINAPI* pfn)(LPCTSTR, LPCTSTR);
	pfn = bNoCase ? StrStrI : StrStr;

	CString tmp = pText;
	while (1) {
		LPCTSTR p = pfn(tmp.GetString(), src);
		if (!p) {
			str += tmp;
			break;
		}
		int len = (int)(INT_PTR)(p - tmp.GetString());
		if (len > 0)
			str += tmp.Left(len);
		str += dst;
		tmp = tmp.Mid(len + lstrlen(src));
		if (!bReplaceAll) {
			str += tmp;
			break;
		}
	}
	return str;
}

static void applyname(CSimpleArray<CPairName>& arr, const CRepName& condition, int index)
{
	if (index < 0) return;

	CWin32FindData wfd;
	TCHAR path[MAX_PATH * 2], src[MAX_PATH * 2], dst[MAX_PATH * 2], ext[MAX_PATH], body[MAX_PATH];
	CString res;

	CPairName& pair = arr[index];
	pair.m_dst.Empty();
	pair.m_err = 0;
	PathGetParent(pair.m_src, path);
	lstrcpy(src, PathFindFileName(pair.m_src));
	lstrcpy(body, src);
	PathRemoveExtension(body);
	ext[0] = 0;
	if (condition.m_bSaveExtension) {
		// 拡張子を保存
		LPTSTR p = PathFindExtension(src);
		if (*p == _T('.')) {
			lstrcpy(ext, p);
			*p = 0;
		}
	}
	if (condition.m_bUseRegexp) {
		// 正規表現
		CRexReplace<> rp;
		res = rp.Replace(src, condition.m_src, condition.m_dst, condition.m_bReplaceAll, condition.m_bNoCase);
	} else {
		// 単純置換
		CString cs = condition.m_src;
		if (StrStr(cs.GetString(), _T("%full%"))) {
			cs.Replace(_T("%full%"), src);
		} else if (StrStr(cs.GetString(), _T("%body%"))) {
			cs.Replace(_T("%body%"), body);
		} else if (StrStr(cs.GetString(), _T("%name%"))) {
			cs.Replace(_T("%name%"), body);
		}

		VARIANT vr;
		CString cd = condition.m_dst;
		LPCTSTR cdp = cd.GetString();
		CString tmp;
		if (StrStr(cdp, _T("%full%"))) {
			cd.Replace(_T("%full%"), src);
		}
		if (StrStr(cdp, _T("%body%"))) {
			cd.Replace(_T("%body%"), body);
		}
		if (StrStr(cdp, _T("%name%"))) {
			cd.Replace(_T("%name%"), body);
		}
		if (StrStr(cdp, _T("%title%"))) {
			// タイトル
			if (GetItemProperty(pair.m_src, &PKEY_Title, &vr)) {
				tmp = vr.bstrVal;
				VariantClear(&vr);
				ConvertInvalidToken(tmp);
			} else {
				tmp = _T("(title)");
			}
			cd.Replace(_T("%title%"), tmp.GetString());
		}
		if (StrStr(cdp, _T("%mtrack%"))) {
			// トラック#
			if (GetItemProperty(pair.m_src, &PKEY_Music_TrackNumber, &vr)) {
				tmp.Format(_T("%02d"), vr.ulVal);
				VariantClear(&vr);
			} else {
				tmp = _T("(track)");
			}
			cd.Replace(_T("%mtrack%"), tmp.GetString());
		}
		if (StrStr(cdp, _T("%album%"))) {
			// アルバムタイトル
			if (GetItemProperty(pair.m_src, &PKEY_Music_AlbumTitle, &vr)) {
				tmp = vr.bstrVal;
				VariantClear(&vr);
				ConvertInvalidToken(tmp);
			} else {
				tmp = _T("(album)");
			}
			cd.Replace(_T("%album%"), tmp.GetString());
		}
		if (StrStr(cdp, _T("%martist%"))) {
			// アルバムアーティスト
			if (GetItemProperty(pair.m_src, &PKEY_Music_AlbumArtist, &vr)) {
				tmp = vr.bstrVal;
				VariantClear(&vr);
				ConvertInvalidToken(tmp);
			} else {
				tmp = _T("(martist)");
			}
			cd.Replace(_T("%martist%"), tmp.GetString());
		}
		if (StrStr(cdp, _T("%pdate%")) || StrStr(cdp, _T("%ptime%")) || StrStr(cdp, _T("%ptime!%"))) {
			// 撮影日時
			if (GetItemProperty(pair.m_src, &PKEY_Photo_DateTaken, &vr)) {
				FILETIME lt;
				VariantTimeToFileTime(&vr, &lt, true);
				SYSTEMTIME st;
				FileTimeToSystemTime(&lt, &st);
				tmp.Format(_T("%04d%02d%02d"), st.wYear, st.wMonth, st.wDay);
				cd.Replace(_T("%pdate%"), tmp.GetString());
				tmp.Format(_T("%02d%02d%02d"), st.wHour, st.wMinute, st.wSecond);
				cd.Replace(_T("%ptime%"), tmp.GetString());
				tmp.Format(_T("%02d%02d"), st.wHour, st.wMinute);
				cd.Replace(_T("%ptime!%"), tmp.GetString());
			} else {
				cd.Replace(_T("%pdate%"), _T("(date)"));
				cd.Replace(_T("%ptime%"), _T("(time)"));
			}
		}
		if (StrStr(cdp, _T("%cdate%")) || StrStr(cdp, _T("ctime%")) || StrStr(cdp, _T("ctime!%"))) {
			// ファイル作成日時
			GetFileData(pair.m_src, wfd);
			SYSTEMTIME st;
			FileTimeToSystemTime(&wfd.ftCreationTime, &st);
			tmp.Format(_T("%04d%02d%02d"), st.wYear, st.wMonth, st.wDay);
			cd.Replace(_T("%cdate%"), tmp.GetString());
			tmp.Format(_T("%02d%02d%02d"), st.wHour, st.wMinute, st.wSecond);
			cd.Replace(_T("%ctime%"), tmp.GetString());
			tmp.Format(_T("%02d%02d"), st.wHour, st.wMinute);
			cd.Replace(_T("%ctime!%"), tmp.GetString());
		}
		if (StrStr(cdp, _T("%fdate%")) || StrStr(cdp, _T("%ftime%")) || StrStr(cdp, _T("%ftime!%"))) {
			// ファイル更新日時
			GetFileData(pair.m_src, wfd);
			SYSTEMTIME st;
			FileTimeToSystemTime(&wfd.ftLastWriteTime, &st);
			tmp.Format(_T("%04d%02d%02d"), st.wYear, st.wMonth, st.wDay);
			cd.Replace(_T("%fdate%"), tmp.GetString());
			tmp.Format(_T("%02d%02d%02d"), st.wHour, st.wMinute, st.wSecond);
			cd.Replace(_T("%ftime%"), tmp.GetString());
			tmp.Format(_T("%02d%02d"), st.wHour, st.wMinute);
			cd.Replace(_T("%ftime!%"), tmp.GetString());
		}
		if (StrStr(cdp, _T("%xframe%"))) {
			//  video フレーム幅
			if (GetItemProperty(pair.m_src, &PKEY_Video_FrameWidth, &vr)) {
				tmp.Format(_T("%d"), vr.ulVal);
				VariantClear(&vr);
			} else {
				tmp = _T("(##)");
			}
			cd.Replace(_T("%xframe%"), tmp.GetString());
		}
		if (StrStr(cdp, _T("%yframe%"))) {
			//  video フレーム高さ
			if (GetItemProperty(pair.m_src, &PKEY_Video_FrameHeight, &vr)) {
				tmp.Format(_T("%d"), vr.ulVal);
				VariantClear(&vr);
			} else {
				tmp = _T("(##)");
			}
			cd.Replace(_T("%yframe%"), tmp.GetString());
		}
		res = repstr(src, cs, cd, condition.m_bNoCase, condition.m_bReplaceAll);
	}
	if (condition.m_bSaveExtension)
		res += ext;
	// 空白を取り除く
	res.Trim();
	// フルパス
	PathCombine(dst, path, res.GetString());

	// エラーチェック
	if (res.FindOneOf(gszInvalidToken) >= 0) {
		// 使用不可文字
		pair.m_err = 2;
	} else if (!pair.m_src.CompareNoCase(dst)) {
		// 変化なし
		pair.m_err = 1;
	} else if (GetFileData(dst, wfd)) {
		// 同一ファイル名が存在
		pair.m_err = 3;
	} else {
		// 同一ファイル名がリスト中に存在するかチェック
		for (int j = 0; j < index; j++) {
			if (!arr[j].m_dst.CompareNoCase(dst)) {
				pair.m_err = 4;
				break;
			}
		}
	}
	pair.m_dst = dst;
}

static BOOL get_common_string(CSimpleArray<CPairName>& arr, LPTSTR pszCommon)
{
	int cnt = arr.GetSize();
	if (!pszCommon || cnt < 1) return FALSE;

	TCHAR buf[MAX_PATH * 2], cmn[MAX_PATH * 2];
	lstrcpy(cmn, PathFindFileName(arr[0].m_src));
	*pszCommon = 0;

	LPTSTR pc = cmn;
	while (*pc) {
		int len = lstrlen(pc);
		lstrcpy(buf, pc);
		while (len > 0) {
			BOOL bFound = TRUE;
			for (int i = 1; i < cnt; i++) {
				LPTSTR pf = PathFindFileName(arr[i].m_src);
				if (!StrStrI(pf, buf)) {
					bFound = FALSE;
					break;
				}
			}

			if (bFound && lstrlen(buf) > lstrlen(pszCommon))
				lstrcpy(pszCommon, buf);
			buf[--len] = 0;
		}
		pc = CharNext(pc);
	}
	return TRUE;
}

static TCHAR sszRepNamesPrev[] = _T("prev.repnames.$$$");

static LPCTSTR BuildPrevSettingFileName(CString& str)
{
	LPTSTR p = str.GetBuffer(MAX_PATH * 2);
	GetFopLocalDataFolder(p);
	PathAppend(p, sszRepNamesPrev);
	str.ReleaseBuffer();
	return str;
}

/////////////////////////////////////////////////////////////////////////////
// RepNamesDlg

class CRepNamesDlg : public CDialogImpl<CRepNamesDlg>
{
private:
	UINT m_wm_applyname;
	CRepName m_rn;
	CSimpleArray<CRepName> m_rna;
	int m_curindex;
	int m_errors;
	int m_hisindex;
	CString m_strPrevSetting;
public:
	CRepNamesDlg()
		: m_wm_applyname(::RegisterWindowMessage(_T("wm_applyname")))
		, m_curindex(-1)
		, m_hisindex(-1)
		, m_errors(0)
	{ }
	~CRepNamesDlg() { }

	enum { IDD = IDD_REPNAME };
	CSimpleArray<CPairName>* m_parr;
	CListViewCtrl m_list;

BEGIN_MSG_MAP(CRepNamesDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_EN_KILLFOCUS(IDC_SRC, OnCheck)
	ON_EN_KILLFOCUS(IDC_DST, OnCheck)
	ON_BN_CLICKED(IDC_CASESENSE, OnCheck)
	ON_BN_CLICKED(IDC_REPLACEALL, OnCheck)
	ON_BN_CLICKED(IDC_USEREGEXP, OnCheck)
	ON_BN_CLICKED(IDC_SAVEEXTENSION, OnCheck)
	ON_BN_CLICKED(IDC_PREVSETTING, OnPrevSetting)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
	ON_REGISTERD_MESSAGE(m_wm_applyname, OnApplyName)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM)
	{
		CenterWindow();

		m_list = GetDlgItem(IDC_LIST);
		HIMAGELIST himl = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_PRGSTAT), 16, RGB(255, 0, 255));
		m_list.SetImageList(himl, LVSIL_SMALL);

		int i;
		CString str;
		for (i = 0; i < ARRAYSIZE(himmap); i++) {
			himl = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(himmap[i].idRes), 16, RGB(255, 0, 255));
			FlatButton_SetImageList(GetDlgItem(himmap[i].idCtrl), himl);
			if (himmap[i].idTips) {
				str.LoadString(himmap[i].idTips);
				FlatButton_SetTipText(GetDlgItem(himmap[i].idCtrl), str);
			}
		}

		// 列の挿入
		for (i = 0; i < 2; i++) {
			str.LoadString(sstHead[i].ids);
			m_list.InsertColumn(i, str.GetString(), LVCFMT_LEFT, sstHead[i].cx, i);
		}

		int cnt = m_parr->GetSize();
		for (i = 0; i < cnt; i++) {
			m_list.InsertItem(i, PathFindFileName((*m_parr)[i].m_src));
		}

		// 共通文字列抽出
		TCHAR cmn[MAX_PATH];
		if (get_common_string(*m_parr, cmn))
			SetDlgItemText(IDC_SRC, cmn);

		BuildPrevSettingFileName(m_strPrevSetting);
		if (!_pathisfile(m_strPrevSetting)) {
			GetDlgItem(IDC_PREVSETTING).EnableWindow(FALSE);
		}

		// 過去履歴
		LoadHistory();
		return TRUE;
	}
	BOOL OnHelpInfo(HELPINFO* pInfo)
	{
		if (pInfo->iContextType == HELPINFO_WINDOW) {
			ShowHtmlHelp(m_hWnd, IDD);
			return TRUE;
		}
		return FALSE;
	}
	BOOL CheckParam()
	{
		m_curindex = -1;
		m_errors = 0;
		GetDlgItemText(IDC_SRC, m_rn.m_src);
		GetDlgItemText(IDC_DST, m_rn.m_dst);
		if (m_rn.m_src.IsEmpty() && m_rn.m_dst.IsEmpty()) {
			return FALSE;
		}
		m_rn.m_bNoCase = (IsDlgButtonChecked(IDC_CASESENSE) != FALSE);
		m_rn.m_bReplaceAll = (IsDlgButtonChecked(IDC_REPLACEALL) != FALSE);
		m_rn.m_bUseRegexp = (IsDlgButtonChecked(IDC_USEREGEXP) != FALSE);
		m_rn.m_bSaveExtension = (IsDlgButtonChecked(IDC_SAVEEXTENSION) != FALSE);
		return TRUE;
	}
	void OnCheck()
	{
		if (CheckParam())
			::PostMessage(m_hWnd, m_wm_applyname, 0, 0);
	}
	void OnPrevSetting()
	{
		int cnt = m_rna.GetSize();
		if (cnt > 0) {
/*
			m_hisindex = (m_hisindex + 1) % cnt;
			m_rn = m_rna[cnt - m_hisindex - 1];
			SetDlgItemText(IDC_SRC, m_rn.m_src);
			SetDlgItemText(IDC_DST, m_rn.m_dst);
			CheckDlgButton(IDC_CASESENSE, m_rn.m_bNoCase);
			CheckDlgButton(IDC_REPLACEALL, m_rn.m_bReplaceAll);
			CheckDlgButton(IDC_USEREGEXP, m_rn.m_bUseRegexp);
			CheckDlgButton(IDC_SAVEEXTENSION, m_rn.m_bSaveExtension);
			::PostMessage(m_hWnd, m_wm_applyname, 0, 0);
*/
			CMenu pop; pop.CreatePopupMenu();

			CString str;
			for (int i = cnt - 1; i >= 0; i--) {
				str = m_rna[i].label();
				pop.AppendMenu(MF_STRING, i + 1, str);
			}
			// 履歴クリアメニュー追加
			pop.AppendMenu(MF_SEPARATOR, -1, (LPCTSTR)NULL);
			str.LoadString(IDS_CLEAR_HISTORY);
			pop.AppendMenu(MF_STRING, CMD_CLEAR_HISTORY, str);

			CRect rc;
			GetDlgItem(IDC_PREVSETTING).GetWindowRect(&rc);
			CPoint pt(rc.left, rc.bottom);
			int cmd = pop.TrackPopupMenu(TPM_RIGHTALIGN | TPM_RETURNCMD, rc.right, rc.bottom, m_hWnd);
			if (cmd > 0) {
				if (cmd == CMD_CLEAR_HISTORY) {
					// 履歴をクリア
					m_rna.RemoveAll();
					fopRemoveFileNoConfirm(m_strPrevSetting);
					GetDlgItem(IDC_PREVSETTING).EnableWindow(FALSE);
				} else {
					m_rn = m_rna[cmd - 1];
					SetDlgItemText(IDC_SRC, m_rn.m_src);
					SetDlgItemText(IDC_DST, m_rn.m_dst);
					CheckDlgButton(IDC_CASESENSE, m_rn.m_bNoCase);
					CheckDlgButton(IDC_REPLACEALL, m_rn.m_bReplaceAll);
					CheckDlgButton(IDC_USEREGEXP, m_rn.m_bUseRegexp);
					CheckDlgButton(IDC_SAVEEXTENSION, m_rn.m_bSaveExtension);
					::PostMessage(m_hWnd, m_wm_applyname, 0, 0);
				}
			}
		}
	}

	LRESULT OnApplyName(WPARAM, LPARAM)
	{
		if (m_curindex < 0) {
			m_curindex = 0;
		} else if (++m_curindex >= m_parr->GetSize()) {
			m_curindex = -1;
			return 0;
		}

		applyname(*m_parr, m_rn, m_curindex);
		CPairName& pair = (*m_parr)[m_curindex];
		if (pair.m_err > 1)
			m_errors++;

		CString str = PathFindFileName(pair.m_dst);
		int iIcon = 0;
		if (pair.m_err == 1) {
			iIcon = 4;
			str.LoadString(IDS_NOCHANGE);
		} else if (pair.m_err) {
			iIcon = 3;
		}
		m_list.SetItemText(m_curindex, 1, str.GetString());
		m_list.SetItem(m_curindex, 0, LVIF_IMAGE, NULL, iIcon, 0, 0, 0);

		::PostMessage(m_hWnd, m_wm_applyname, 0, 0);
		return 0;
	}

	void OnOK()
	{
		CString src;
		CString dst;
		GetDlgItemText(IDC_SRC, src);
		GetDlgItemText(IDC_DST, m_rn.m_dst);
		if (src != m_rn.m_src || dst != m_rn.m_dst) {
			// チェックを反映できてない場合にはここでチェック
			if (!CheckParam())
				return;

			int cnt = m_parr->GetSize();
			for (int i = 0; i < cnt; i++) {
				applyname(*m_parr, m_rn, i);
				CPairName& pair = (*m_parr)[i];
				if (pair.m_err > 1)
					m_errors++;
			}
		}

		if (!m_errors) {
			SaveHistory();
			EndDialog(IDOK);
		}
	}
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnHelpCall() { ShowHtmlHelp(m_hWnd, IDD); }

	bool SaveHistory()
	{
		// 実行パターンが過去履歴にあるかどうかをチェックし、あれば過去履歴配列から削除
		int i = m_rna.Find(m_rn);
		if (i >= 0) {
			m_rna.RemoveAt(i);
		}
		m_rna.Add(m_rn);

		char ssz[MAX_PATH * 2];
		// Save XML
		TiXmlDocument xmlDoc;
		xmlDoc.InsertEndChild( TiXmlDeclaration( "1.0", "UTF-8", "yes" ) );

		TiXmlNode* pDoc;
		TiXmlElement* pNode;
		int cnt = m_rna.GetSize();
		i = (cnt > MAXHISTORY) ? cnt - MAXHISTORY : 0;
		for (; i < cnt; i++) {
			pDoc = xmlDoc.InsertEndChild( TiXmlElement( "settings" ) );
			pNode = pDoc->ToElement();
			CRepName& rn = m_rna[i];
			// 検索文字列
			WideCharToMultiByte(CP_UTF8, 0, rn.m_src, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
			pNode->SetAttribute( "search", ssz );
			// 置換文字列
			WideCharToMultiByte(CP_UTF8, 0, rn.m_dst, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
			pNode->SetAttribute( "replacewith", ssz );
			// 大文字小文字
			pNode->SetAttribute( "nocase", (rn.m_bNoCase != false) );
			// 全部置換
			pNode->SetAttribute( "replaceall", (rn.m_bReplaceAll != false) );
			// 正規表現
			pNode->SetAttribute( "regexp", (rn.m_bUseRegexp != false) );
			// 拡張子を残す
			pNode->SetAttribute( "saveext", (rn.m_bSaveExtension != false) );
		}
		// Save XML
		WideCharToMultiByte(CP_ACP, 0, m_strPrevSetting, -1, ssz, ARRAYSIZE(ssz), NULL, NULL);
		return xmlDoc.SaveFile(ssz);
	}

	bool LoadHistory()
	{
		char xml[MAX_PATH * 2];
		WideCharToMultiByte(CP_ACP, 0, m_strPrevSetting, -1, xml, ARRAYSIZE(xml), NULL, NULL);

		TiXmlDocument xmlDoc;
		if (!xmlDoc.LoadFile( xml )) return false;

		const char* psz;
		WCHAR wsz[MAX_PATH * 2];
		TiXmlNode* pDoc = xmlDoc.ToDocument();
		if (pDoc) {
			TiXmlElement* pNode = pDoc->FirstChildElement("settings");
			while (pNode) {
				// 検索文字列
				wsz[0] = 0;
				psz = pNode->Attribute("search");
				if (psz) {
					MultiByteToWideChar(CP_UTF8, 0, psz, -1, wsz, ARRAYSIZE(wsz));
					m_rn.m_src = wsz;
				}
				// 置換文字列
				wsz[0] = 0;
				psz = pNode->Attribute("replacewith");
				if (psz) {
					MultiByteToWideChar(CP_UTF8, 0, psz, -1, wsz, ARRAYSIZE(wsz));
					m_rn.m_dst = wsz;
				}
				// 大文字小文字
				BOOL b;
				if (pNode->QueryIntAttribute("nocase", &b) != TIXML_SUCCESS) b = FALSE;
				m_rn.m_bNoCase = (b != FALSE);
				// 全部置換
				if (pNode->QueryIntAttribute("replaceall", &b) != TIXML_SUCCESS) b = FALSE;
				m_rn.m_bReplaceAll = (b != FALSE);
				// 正規表現
				if (pNode->QueryIntAttribute("regexp", &b) != TIXML_SUCCESS) b = FALSE;
				m_rn.m_bUseRegexp = (b != FALSE);
				// 拡張子を残す
				if (pNode->QueryIntAttribute("saveext", &b) != TIXML_SUCCESS) b = FALSE;
				m_rn.m_bSaveExtension = (b != FALSE);
				// 履歴配列に追加
				m_rna.Add(m_rn);
				// 次のノード
				pNode = pNode->NextSiblingElement("settings");
			}
		}
		return (pDoc != NULL);
	}
};

extern "C"
BOOL WINAPI fopRepStrNames(HWND hwnd, HDROP hDrop)
{
	int cnt;
	if (!hDrop || (cnt = DragQueryFile(hDrop, 0xffffffff, NULL, 0)) < 1)
		return FALSE;

	// ファイルをリストアップ
	CSimpleArray<CPairName> array;
	CWin32FindData wfd;
	TCHAR src[MAX_PATH * 2];
	CPairName pair;
	int i;
	for (i = 0; i < cnt; i++) {
		DragQueryFile(hDrop, i, src, ARRAYSIZE(src));
		pair.set(src, NULL);
		array.Add(pair);
	}
	if (array.GetSize() < 1)
		return FALSE;

	CRepNamesDlg dlg;
	dlg.m_parr = &array;
	if (dlg.DoModal(hwnd) != IDOK) return FALSE;

	cnt = array.GetSize();

	CAnimationProgressDlg progress;
	progress.SetTotalSize(cnt);
	progress.Start(hwnd);
	progress.SetCaption(MAKEINTRESOURCE(IDS_TITLE_RENAME));
	progress.SetTitle(MAKEINTRESOURCE(IDS_RENAMING));
	BOOL ok = TRUE;
	for (i = 0; i < cnt && ok; i++) {
		if (progress.isCanceled()) {
			ok = FALSE;
			break;
		}
		if (array[i].m_err == 0) {
			progress.SetLine(1, PathFindFileName(array[i].m_src));
			while (!_movefile(array[i].m_src, array[i].m_dst)) {
				int res = progress.Error(array[i].m_dst, MB_QABORT);
				if (res == IDABORT) {
					ok = FALSE;
					break;
				}
				else if (res == IDIGNORE)
					break;
			}
		}
		progress.SetCompletedSize(i);
	}
	progress.Close();
	return ok;
}
