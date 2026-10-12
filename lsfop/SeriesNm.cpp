// SeriesNm.cpp : implementation file
//

#include "stdafx.h"
#include "fopstd.h"

/////////////////////////////////////////////////////////////////////////////
//

class CSeriesName {
public:
	CSeriesName()
		: m_strFormat(_T(""))
		, m_nStartNumber(1)
		, m_nStep(1)
		, m_entries(0)
	{
	}
	~CSeriesName() {}

	CString m_strFormat;
	int m_nStartNumber;
	int m_nStep;
	int m_entries;
};

//////////////////////////////////////////////////////////////////////
//

#define MAXENTRY		16


/////////////////////////////////////////////////////////////////////////////
// SeriesNameDlgFn

class CSeriesNameDlg : public CDialogImpl<CSeriesNameDlg>
{
public:
	CSeriesName* m_psn;
	CComboBox m_combo;

	enum { IDD = IDD_SERIESNAME };

	CSeriesNameDlg() { m_psn = NULL; }


BEGIN_MSG_MAP(CSeriesNameDlg)
	ON_WM_INITDIALOG()
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_HELPCALL, OnHelpCall)
	ON_BN_CLICKED(IDC_CLEAR, OnClearHistory)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	BOOL OnHelpInfo(HELPINFO* pInfo);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnHelpCall() { ShowHtmlHelp(m_hWnd, IDD); }
	void OnClearHistory();
};

BOOL CSeriesNameDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();

	m_psn = (CSeriesName*)lParam;
	m_combo = GetDlgItem(IDC_FORM);

	SetDlgItemInt(IDC_FIRSTVALUE, m_psn->m_nStartNumber, FALSE);
	SetDlgItemInt(IDC_STEP, m_psn->m_nStep, TRUE);

	CUpDownCtrl spin = GetDlgItem(IDC_SPIN);
	spin.SetRange(0, UD_MAXVAL);

	// ヒストリの設定
	if (RegLoadHistoryEx(m_combo, _MKIR(IDS_SEC_HISTORY), _MKIR(IDS_KEY_SERIESNAME), MAXENTRY) < 1) {
		GetDlgItem(IDC_CLEAR).EnableWindow(FALSE);
		// 以下ドロップダウンボタンクリック時の不具合動作を回避
		m_combo.AddString(m_psn->m_strFormat);
		m_combo.AddString(m_psn->m_strFormat);
		m_combo.AddString(m_psn->m_strFormat);
		m_combo.ResetContent();
	}
	SetDlgItemText(IDC_FORM, m_psn->m_strFormat);
	return TRUE;
}

BOOL CSeriesNameDlg::OnHelpInfo(HELPINFO* pInfo)
{
	if (pInfo->iContextType == HELPINFO_WINDOW) {
		ShowHtmlHelp(m_hWnd, IDD);
		return TRUE;
	}
	return FALSE;
}

void CSeriesNameDlg::OnOK()
{
	TCHAR buf[MAX_PATH];
	if (m_combo.GetWindowText(buf, ARRAYSIZE(buf)) < 1) return;

	m_psn->m_strFormat = buf;
	m_psn->m_nStartNumber = GetDlgItemInt(IDC_FIRSTVALUE, NULL, FALSE);
	m_psn->m_nStep = GetDlgItemInt(IDC_STEP, NULL, TRUE);
	if (!m_psn->m_nStep) {
		MessageBeep(0);
		GetDlgItem(IDC_STEP).SetFocus();
		return;
	}

	LPTSTR p = StrChr(buf, '#');
	if (p) {
		++p;
		TCHAR temp[MAX_PATH];
		LPTSTR r = temp;
		LPTSTR q = StrChr(p, '#');
		if (q) {
			while (p < q) {
				if (!isdigit(*p)) {
					MessageBeep(0);
					return;
				}
				*r++ = *p++;
			}
			// 13桁以上を指定されたらエラー
			int cl = StrToInt(temp);
			if (cl > 12) {
				MessageBeep(0);
				return;
			}
		}
	}
	else {
		if (AppMessage(IDS_TITLE_SERIESNAME, IDS_NOORDERPATTERN, NULL, MB_QYESNO) != IDYES)
			return;
	}

	// レジストリに保存
	RegSaveHistory(MAKEINTRESOURCE(IDS_SEC_HISTORY), MAKEINTRESOURCE(IDS_KEY_SERIESNAME), MAXENTRY, buf);
	EndDialog(IDOK);
}

void CSeriesNameDlg::OnClearHistory()
{
	RegClearHistoryEx(m_combo, _MKIR(IDS_SEC_HISTORY), _MKIR(IDS_KEY_SERIESNAME));

	SetDlgItemText(IDC_FORM, m_psn->m_strFormat);
	GetDlgItem(IDC_CLEAR).EnableWindow(FALSE);
}

//////////////////////////////////////////////////////////////////////
//

static const TCHAR repName[]       = _T("%name%");
static const TCHAR repBody[]       = _T("%body%");
static const TCHAR repExt[]        = _T("%ext%");
static const TCHAR repFull[]       = _T("%full%");
static const TCHAR repTime[]       = _T("%time%");
static const TCHAR repDate[]       = _T("%date%");
static const TCHAR repFTime[]      = _T("%ftime%");
static const TCHAR repFDate[]      = _T("%fdate%");
static const TCHAR rep422[]        = _T("%04d%02d%02d");
static const TCHAR rep222[]        = _T("%02d%02d%02d");
static const TCHAR repPropTitle[]  = _T("%title%");
static const TCHAR repMusicTrack[] = _T("%mtrack%");
static const TCHAR repDateTaken[]  = _T("%pdate%");
static const TCHAR repTimeTaken[]  = _T("%ptime%");

extern "C"
BOOL WINAPI fopSeriesName(HWND hwnd, HDROP hDrop)
{
	int cnt;
	if (!hDrop || (cnt = DragQueryFile(hDrop, 0xffffffff, NULL, 0)) < 1) {
		return FALSE;
	}

	// 拡張子を切り出して、連番フォーマットに付加
	CSeriesName sn;
	sn.m_entries = cnt;
	cnt = cnt < 100 ? 2 : ((cnt < 1000) ? 3 : 4);
	sn.m_strFormat.Format(_T("#%d#"), cnt);
	TCHAR buf[MAX_PATH * 2];
	DragQueryFile(hDrop, 0, buf, ARRAYSIZE(buf));
	LPTSTR p = PathFindExtension(buf);
	if (*p == '.') {
		sn.m_strFormat += p;
	}
	CSeriesNameDlg dlg;
	if (dlg.DoModal(hwnd, (LPARAM)&sn) != IDOK) {
		return FALSE;
	}
	TCHAR fmt[MAX_PATH * 2], ww[MAX_PATH * 2];
	lstrcpy(fmt, sn.m_strFormat);

	bool bHasOrder = false;
	p = StrChr(fmt, '#');
	if (p) {
		bHasOrder = true;
		lstrcpy(buf, p++);
		if (*p == '#')
			ReplaceString(fmt, _T("##"), _T("%d"));
		else if (isdigit(*p)) {
			p = buf + 1;
			int col = 0;
			while (isdigit(*p)) {
				col = col * 10 + *p - '0';
				p++;
			}
			if (*p != '#')
				return FALSE;
			*++p = '\0';

			wsprintf(ww, _T("%%0%dd"), col);
			ReplaceString(fmt, buf, ww);
		}
		else
			return FALSE;
	}

	// 元ファイル名保持
	CWaitCursor wc;

	BOOL bOK = TRUE;
	CSimpleArray<CPairName *> array;
	CWin32FindData wfd;
	CPairName* ppn;

	TCHAR src[MAX_PATH * 2], dst[MAX_PATH * 2];
	SYSTEMTIME stc, stf;
	GetLocalTime(&stc);

	int nStart = sn.m_nStartNumber;
	int i;
	for (i = 0; i < sn.m_entries; i++) {
		DragQueryFile(hDrop, i, src, ARRAYSIZE(src));
		if (GetFileData(src, wfd)) {
			FileTimeToSystemTime(&wfd.ftLastWriteTime, &stf);
			PathGetParent(src, buf);
			PathAppend(buf, fmt);

			if (StrStr(fmt, repFull))
				ReplaceString(buf, repFull, wfd.cFileName);
			if (StrStr(fmt, repName) || StrStr(fmt, repBody)) {
				lstrcpy(ww, wfd.cFileName);
				p = PathFindExtension(ww);
				if (*p == '.')
					*p = 0;
				ReplaceString(buf, repName, ww);
				ReplaceString(buf, repBody, ww);
			}
			if (StrStr(fmt, repExt)) {
				lstrcpy(ww, wfd.cFileName);
				p = PathFindExtension(ww);
				if (*p == '.')
					ReplaceString(buf, repExt, p);
			}
			if (StrStr(fmt, repDate)) {
				wsprintf(ww, rep422, stc.wYear, stc.wMonth, stc.wDay);
				ReplaceString(buf, repDate, ww);
			}
			if (StrStr(fmt, repTime)) {
				wsprintf(ww, rep222, stc.wHour, stc.wMinute, stc.wSecond);
				ReplaceString(buf, repTime, ww);
			}
			if (StrStr(fmt, repFDate)) {
				wsprintf(ww, rep422, stf.wYear, stf.wMonth, stf.wDay);
				ReplaceString(buf, repFDate, ww);
			}
			if (StrStr(fmt, repFTime)) {
				wsprintf(ww, rep222, stf.wHour, stf.wMinute, stf.wSecond);
				ReplaceString(buf, repFTime, ww);
			}

			VARIANT vr;
			if (StrStr(fmt, repMusicTrack)) {
				if (GetItemProperty(src, &PKEY_Music_TrackNumber, &vr)) {
					wsprintf(ww, _T("%02d"), vr.ulVal);
					VariantClear(&vr);
				} else {
					wsprintf(ww, _T("(%d)"), i);
				}
				ReplaceString(buf, repMusicTrack, ww);
			}
			if (StrStr(fmt, repPropTitle)) {
				if (GetItemProperty(src, &PKEY_Title, &vr)) {
					CString tmp = vr.bstrVal;
					VariantClear(&vr);
					ConvertInvalidToken(tmp);
					ReplaceString(buf, repPropTitle, tmp.GetString());
				} else {
					ReplaceString(buf, repPropTitle, _T(""));
				}
			}
			if (StrStr(fmt, repDateTaken) || StrStr(fmt, repTimeTaken)) {
				if (GetItemProperty(src, &PKEY_Photo_DateTaken, &vr)) {
					FILETIME lt;
					VariantTimeToFileTime(&vr, &lt, true);
					SYSTEMTIME st;
					FileTimeToSystemTime(&lt, &st);
					wsprintf(ww, rep422, st.wYear, st.wMonth, st.wDay);
					ReplaceString(buf, repDateTaken, ww);
					wsprintf(ww, rep222, st.wHour, st.wMinute, st.wSecond);
					ReplaceString(buf, repTimeTaken, ww);
				} else {
					ReplaceString(buf, repDateTaken, _T(""));
					ReplaceString(buf, repTimeTaken, _T(""));
				}
			}
			if (bHasOrder) {
				wsprintf(dst, buf, nStart);
			} else {
				lstrcpy(dst, buf);
			}
			nStart += sn.m_nStep;

			if (GetFileData(dst, wfd)) {
				CString mes;
				FormatFromToString(PathFindFileName(src), PathFindFileName(dst), mes);
				SetLastError(ERROR_FILE_EXISTS);
				LastError(IDS_TITLE_RENAME, mes);
				bOK = FALSE;
				break;
			}

			ppn = new CPairName;
			ppn->m_src = src;
			ppn->m_dst = dst;
			array.Add(ppn);
		}
	}

	if (bOK) {
		for (i = 0; i < array.GetSize(); i++) {
			ppn = array[i];
			if (!_movefile(ppn->m_src, ppn->m_dst)) {
				LastError(IDS_TITLE_RENAME, ppn->m_dst);
				bOK = FALSE;
				break;
			}
		}
	}

	for (i = 0; i < array.GetSize(); i++) {
		ppn = array[i];
		delete ppn;
	}
	array.RemoveAll();

	return bOK;
}
