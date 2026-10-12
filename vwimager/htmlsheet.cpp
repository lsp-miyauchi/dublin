//////////////////////////////////////////////////////////////////////
// htmlsheet.cpp

#include "stdafx.h"
#include <stdio.h>
#include "progress_sup.h"
#include "common.h"
#include "exifinfo.h"
#include "dropexec.h"
#include "resource.h"

enum {
	srcNoLink = 0,			// リンクせず
	srcRelLink,				// 相対パスでリンク
	srcAbsLink,				// 絶対パスでリンク
};

typedef struct tagSHEETOPTION {
	int item_width;
	int item_height;
	int column;
	int row;
	TCHAR title[MAX_PATH];
	TCHAR outpath[MAX_PATH * 2];
	COLORREF rgb;
	bool copy_source:1;
	bool run_browser:1;
	bool keisen:1;
	int use_link:3;
	int image_type:3;
} SHEETOPTION;

//////////////////////////////////////////////////////////////////////
//

class CHtmlSheetDlg : public CDialogImpl<CHtmlSheetDlg>
{
public:
	enum { IDD = IDD_HTMLSHEET };
	SHEETOPTION* m_pso;
	CComboBox m_combo;
	CComboBox m_link;
	CComboBox m_path;

	CHtmlSheetDlg() : m_pso(NULL) { }

BEGIN_MSG_MAP(CHtmlSheetDlg)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDOK, OnOK);
	ON_BN_CLICKED(IDCANCEL, OnCancel);
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse);
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnBrowse();
};

BOOL CHtmlSheetDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	m_combo = GetDlgItem(IDC_IMAGETYPE);
	m_link = GetDlgItem(IDC_LINK);
	m_path = GetDlgItem(IDC_FOLDER);

	InitDlgCtrl(m_combo, IDS_HTML_IMAGETYPE);
	InitDlgCtrl(m_link, IDS_HTML_LINK);

	CAppReg reg;
	reg.LoadComboHistory(m_path, IDS_SEC_WEBALBUM, IDS_KEY_HISTORY, MAXHISTORY);

	m_pso = (SHEETOPTION*)lParam;
	m_combo.SetCurSel(m_pso->image_type);
	m_link.SetCurSel(m_pso->use_link);

	SetDlgItemInt(IDC_WIDTH, m_pso->item_width, FALSE);
	SetDlgItemInt(IDC_HEIGHT, m_pso->item_height, FALSE);

	SetDlgItemText(IDC_TITLE, m_pso->title);
	SetDlgItemInt(IDC_COLUMN, m_pso->column, FALSE);
	SetDlgItemInt(IDC_ROW, m_pso->row, FALSE);

	SetDlgItemText(IDC_FOLDER, m_pso->outpath);
	if (m_pso->run_browser)
		CheckDlgButton(IDC_RUNBROWSER, TRUE);
	if (m_pso->copy_source)
		CheckDlgButton(IDC_COPY, TRUE);
	if (m_pso->keisen)
		CheckDlgButton(IDC_TABLEBORDER, TRUE);

	ColorPicker_SetBkColor(GetDlgItem(IDC_CELLCOLOR), m_pso->rgb);

	// オートコンプリート
	SHAutoComplete(m_path.GetDlgItem(1001), SHACF_AUTOSUGGEST_FORCE_ON | SHACF_FILESYS_DIRS);

	return TRUE;
}

void CHtmlSheetDlg::OnBrowse()
{
	TCHAR buf[MAX_PATH * 2];
	if (GetDlgItemText(IDC_FOLDER, buf, _COUNTOF(buf)) < 1)
		buf[0] = 0;

	if (fopBrowseForFolder(m_hWnd, buf))
		SetDlgItemText(IDC_FOLDER, buf);
}

void CHtmlSheetDlg::OnOK()
{
	if (GetDlgItemText(IDC_FOLDER, m_pso->outpath, _COUNTOF(m_pso->outpath)) < 1) {
		MessageBeep(0);
		GetDlgItem(IDC_FOLDER).SetFocus();
		return;
	}
	GetDlgItemText(IDC_TITLE, m_pso->title, _COUNTOF(m_pso->title));

	m_pso->item_width = GetDlgItemInt(IDC_WIDTH, NULL, FALSE);
	if (m_pso->item_width < 1) {
		MessageBeep(0);
		GetDlgItem(IDC_WIDTH).SetFocus();
		return;
	}

	m_pso->item_height = GetDlgItemInt(IDC_HEIGHT, NULL, FALSE);
	if (m_pso->item_height < 1) {
		MessageBeep(0);
		GetDlgItem(IDC_HEIGHT).SetFocus();
		return;
	}

	m_pso->column = GetDlgItemInt(IDC_COLUMN, NULL, FALSE);
	if (m_pso->column < 1) {
		MessageBeep(0);
		GetDlgItem(IDC_COLUMN).SetFocus();
		return;
	}

	m_pso->row = GetDlgItemInt(IDC_ROW, NULL, FALSE);
	if (m_pso->row < 1) {
		MessageBeep(0);
		GetDlgItem(IDC_ROW).SetFocus();
		return;
	}

	m_pso->image_type = m_combo.GetCurSel();
	m_pso->use_link = m_link.GetCurSel();
	m_pso->run_browser = (IsDlgButtonChecked(IDC_RUNBROWSER) == TRUE);
	m_pso->copy_source = (IsDlgButtonChecked(IDC_COPY) == TRUE);
	m_pso->keisen = (IsDlgButtonChecked(IDC_TABLEBORDER) == TRUE);

	if (!PathIsDirectory(m_pso->outpath)) {
		if (!_pathmaketree(m_pso->outpath))
			return;
	}

	m_pso->rgb = ColorPicker_GetBkColor(GetDlgItem(IDC_CELLCOLOR));

	CAppReg reg;
	reg.SaveComboHistory(m_path, IDS_SEC_WEBALBUM, IDS_KEY_HISTORY, MAXHISTORY, m_pso->outpath);

	EndDialog(IDOK);
}

///////////////////////////////////////////////////////////////////////
//

static void make_thumbnail(HDIB& hdib, int cx, int cy)
{
	LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hdib);
	if (!pbh) {
		GlobalUnlock(hdib);
		return;
	}
	int width = pbh->biWidth;
	int height = pbh->biHeight;
	GlobalUnlock(hdib);

	if (cx >= width && cy >= height)
		return;

	int dx, dy;
	float f;
	for (f = 1.25F; ; f += 0.25F) {
		dx = (int)((float)width / f);
		dy = (int)((float)height / f);
		if (dx <= cx && dy <= cy)
			break;
	}

	mif image(hdib);
	image.zoom(dx, dy, zoomMitchell, FALSE);

	GlobalFree(hdib);
	hdib = image.createdib();
}

static bool aputs(HANDLE hf, LPCTSTR pszString)
{
	if (!pszString || *pszString == '\0') return true;

	int len = 3 * (lstrlen(pszString) + 1);
	char* str = new char[len];
	WideCharToMultiByte(CP_ACP, 0, pszString, -1, str, len, NULL, NULL);
	DWORD n;
	bool ok = (WriteFile(hf, str, lstrlenA(str), &n, NULL) == TRUE);
	delete [] str;
	return ok;
}

static void put_navi(HANDLE hf, int curpage, int numpages)
{
	TCHAR buf[MAX_PATH * 2];
	wsprintf(buf, _T("<p><br></p><table><tr><td>%d/%dページ&nbsp;[&nbsp;</td>"), curpage + 1, numpages);
	aputs(hf, buf);
	for (int i = 0; i < numpages; i++) {
		aputs(hf, _T("<td>"));
		if (i != curpage) {
			wsprintf(buf, _T("<a href=\"sheet_%d.html\">"), i + 1);
			aputs(hf, buf);
		}
		else
			aputs(hf, _T("<b>"));
		wsprintf(buf, _T("&nbsp;%d&nbsp;"), i + 1);
		aputs(hf, buf);
		if (i != curpage)
			aputs(hf, _T("</a>"));
		else
			aputs(hf, _T("</b>"));
		aputs(hf, _T("</td>"));
	}
	aputs(hf, _T("<td>&nbsp;]&nbsp;</td>"));

	if (numpages > 1) {
		if (curpage > 0) {
			wsprintf(buf, _T("<td><a href=\"sheet_%d.html\">≪前へ</a></td>"), curpage);
			aputs(hf, buf);
		}
		if (curpage < numpages - 1) {
			wsprintf(buf, _T("<td><a href=\"sheet_%d.html\">次へ≫></a></td>"), curpage + 2);
			aputs(hf, buf);
		}
	}
	aputs(hf, _T("</tr></table>\n"));
}

class CWebArray : public CSimpleArray<LPTSTR>
{
public:
	~CWebArray()
	{
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++) {
			LPTSTR p = (*this)[i];
			if (p) LocalFree(p);
		}
		RemoveAll();
	}
};

class CPageArray : public CSimpleArray<CWebArray*>
{
public:
	~CPageArray()
	{
		int cnt = GetSize();
		for (int i = 0; i < cnt; i++) {
			CWebArray* p = (*this)[i];
			if (p) delete p;
		}
		RemoveAll();
	}
};

// imgspecify.cpp で定義。 exec.cppでインクルードされる
extern "C" int WINAPI imgSpecifyImageType(LPCTSTR lpName);
extern "C" int WINAPI imgTypeIsExif(int type);

static HDIB loadthumbimage(LPCTSTR pszName, LPTSTR tips, int cchMax)
{
	HDIB hdib = NULL;
	int type = imgSpecifyImageType(pszName);
	if (type != IMG_NOTSUPPORTED && imgTypeIsExif(type)) {
		CComPtr<IExifParser> pep;
		HRESULT hr = pep.CoCreateInstance(__uuidof(ExifParser));
		if (SUCCEEDED(hr)) {
			IStream* pstrm;
			if (CreateStreamOnHGlobal(NULL, TRUE, &pstrm) == S_OK) {
				if (pep->GetThumbsnail(pszName, pstrm, NULL, FALSE, tips, cchMax) == S_OK) {
					HGLOBAL hmem;
					if (GetHGlobalFromStream(pstrm, &hmem) == S_OK) {
						WORD* pw = (WORD*)GlobalLock(hmem);
						if (pw) {
							WORD w = *pw;
							GlobalUnlock(hmem);
							if (w == 0x4d42)			// BITMAP
								hdib = imgDibLoadMem(hmem);
							else if (w == 0xd8ff)		// JPEG
								hdib = imgJpgLoadMemParams(hmem, 0, 1, JC_ISLOW, JD_FS, FALSE);
						}
						else {
							// 失敗した場合は、通常の画像読み込みを行う
							hdib = imgLoadImage(pszName);
						}
					}
				}
				pstrm->Release();
			}
		}
	}
	if (!hdib)
		hdib = imgLoadImage(pszName);

	return hdib;
}

static BOOL make_it(HWND hwnd, HDROP hdrop, SHEETOPTION* pso)
{
	CPageArray pages;
	CWebArray* pweb = NULL;
	TCHAR buf[MAX_PATH * 2];
	int num_items = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
	int i;
	for (i = 0; i < num_items; i++) {
		if (!(i % (pso->column * pso->row))) {
			pweb = new CWebArray;
			if (!pweb)
				break;
			pages.Add(pweb);
		}
		DragQueryFile(hdrop, i, buf, ARRAYSIZE(buf));
		pweb->Add(StrDup(buf));
	}

	CProgressDialog progress;
	progress.SetTotalSize(num_items);
	progress.StartProgressDialog(hwnd, PROGDLG_AUTOTIME | PROGDLG_NOMINIMIZE);

	TCHAR html[MAX_PATH * 2], nail[MAX_PATH * 2], toppage[MAX_PATH * 2], tips[MAX_PATH * 2];

	toppage[0] = '\0';
	int page_counter = 0;
	for (i = 0; i < pages.GetSize(); i++) {
		if (progress.HasUserCancelled())
			break;

		// htmlファイル作成
		wsprintf(buf, _T("sheet_%d.html"), page_counter + 1);
		PathCombine(html, pso->outpath, buf);
		if (!page_counter)
			lstrcpy(toppage, html);

		HANDLE hf = CreateFile(html, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
		if (hf == INVALID_HANDLE_VALUE)
			break;

		// ヘッダ
		aputs(hf, _T("<head>\n"));
		aputs(hf, _T("<meta http-equiv=\"Content-Type\" ")
						 _T("content=\"text/html; charset=shift_jis\">\n")
						 _T("<meta http-equiv=\"Content-Language\" content=\"ja\">\n")
						 _T("<meta name=\"GENERATOR\" content=\"Lightship Web Album\">")
						 _T("<title>"));
		if (pso->title[0])
			aputs(hf, pso->title);
		else
			aputs(hf, _T("sheet"));
		wsprintf(buf, _T("[%d]</title>\n"), page_counter);
		aputs(hf, buf);
		aputs(hf, _T("<style type=\"text/css\">\n<!--\n"));

		aputs(hf, _T("a { text-decoration:none; }\n"));
		aputs(hf, _T("a:link { color:#003399; text-decoration:none; }\n"));
		aputs(hf, _T("a:visited {color:#003399; text-decoration:none; }\n"));
		aputs(hf, _T("a:hover { color:#ffffff; background-color:#003399; text-decoration:none; }\n"));
		aputs(hf, _T("body { font: , , ; font-size: 10pt; }\n"));
		aputs(hf, _T("td { font: , , ; font-size: 10pt; vertical-align: middle;\n}\n"));
		aputs(hf, _T("//-->\n</style>\n"));
		aputs(hf, _T("</head>\n"));

		// BODY
		aputs(hf, _T("<body><center>\n"));
		if (pso->title[0]) {
			wsprintf(buf, _T("<p>%s</p>\n"), pso->title);
			aputs(hf, buf);
		}
		// TABLE
		wsprintf(buf, _T("<table border=%d cellpadding=2>\n"), pso->keisen ? 1 : 0);
		aputs(hf, buf);

		bool bTrEntering = false;
		pweb = pages[i];
		int j;
		for (j = 0; j < pweb->GetSize(); j++) {
			if (progress.HasUserCancelled()) break;

			// テーブル行開始
			if (!(j % pso->column)) {
				aputs(hf, _T("<tr>\n"));
				bTrEntering = true;
			}

			TCHAR* pName = (*pweb)[j];
			progress.SetMessage(MAKEINTRESOURCE(IDS_PROCESSING_READ));
			progress.SetLine(1, pName, TRUE);
			if (pso->copy_source) {
				PathCombine(buf, pso->outpath, PathFindFileName(pName));
				CopyFile(pName, buf, FALSE);
			}

			tips[0] = 0;
			HDIB hdib = loadthumbimage(pName, tips, ARRAYSIZE(tips));

			progress.SetMessage(MAKEINTRESOURCE(IDS_PROCESSING_WRITE));
			make_thumbnail(hdib, pso->item_width, pso->item_height);
			progress.SetMessage(MAKEINTRESOURCE(IDS_PROCESSING_CONVERT));
			wsprintf(buf, _T("sh_%s"), PathFindFileName(pName));
			CharLower(buf);						// 小文字ファイル名
			PathCombine(nail, pso->outpath, buf);

			if (pso->image_type == 0) {
				// JPEG
				rename_imgextension(nail, IMG_JPEG);
				JPEG_CPARAMS jc;
				memset(&jc, 0, sizeof(jc));
				jc.quality = 60;
				jc.dct = JC_ISLOW;
				jc.smooth = 0;
				jc.cdata = NULL;
				imgJpgSaveParams(nail, hdib, &jc);
			}
			else if (pso->image_type == 1) {
				// PNG
				rename_imgextension(nail, IMG_PNG);
				imgPngSave(nail, hdib);
			}
			else {
				// GIF
				rename_imgextension(nail, IMG_GIF);
				imgGifSave(nail, hdib);
			}
			GlobalFree(hdib);

			wsprintf(buf, _T("<td align=center width=%d height=%d "), pso->item_width, pso->item_height);
			aputs(hf, buf);

			wsprintf(buf, _T("bgcolor=#%02X%02X%02X>"), GetRValue(pso->rgb), GetGValue(pso->rgb), GetBValue(pso->rgb));
			aputs(hf, buf);

			if (pso->use_link != srcNoLink) {
				// 元画像にリンクを張る
				TCHAR* q;
				TCHAR rel[MAX_PATH * 2];
				if (pso->copy_source)
					q = PathFindFileName(pName);
				else {
					if (pso->use_link == srcRelLink &&
						PathRelativePathTo(rel, pso->outpath, FILE_ATTRIBUTE_DIRECTORY, pName, FILE_ATTRIBUTE_NORMAL))
						q = rel;
					else
						q = pName;
				}
				wsprintf(buf, _T("<a href=\"%s\">"), q);
				lccReplaceChar(buf, '\\', '/');
				aputs(hf, buf);
			}

			LPTSTR ptips = tips;
			if (tips[0]) ptips += lstrlen(tips) + 1;
			wsprintf(buf, _T("<img border=0 src=\"%s\" alt=\"%s\"><br>%s"), PathFindFileName(nail), ptips, PathFindFileName(pName));
			aputs(hf, buf);
			if (pso->use_link)
				aputs(hf, _T("</a>"));
			aputs(hf, _T("</td>\n"));

			// テーブル行終了
			if ((j % pso->column) == pso->column - 1) {
				aputs(hf, _T("</tr>\n"));
				bTrEntering = false;
			}
			progress.SetCompletedSize(i * pweb->GetSize() + j + 1);
		}

		if (bTrEntering) {
			for (j %= pso->column; j < pso->column; j++)
				aputs(hf, _T("<td></td>\n"));
			aputs(hf, _T("</tr>\n"));
			bTrEntering = false;
		}

		// ファイルを閉じる
		aputs(hf, _T("</table>\n"));
		put_navi(hf, page_counter, pages.GetSize());
		aputs(hf, _T("</center></body></html>\n"));
		CloseHandle(hf);

		page_counter++;
	}

	progress.StopProgressDialog();

	if (pso->run_browser && toppage[0])
		ShellExecute(hwnd, NULL, toppage, NULL, pso->outpath, SW_SHOWNORMAL);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////
//

extern "C"
void WINAPI RunWebAlbumW(HWND hwnd, HINSTANCE hInst, LPWSTR lpCmdLine, int nCmdShow)
{
	CRegExec re;
	if (re.Initialize(lpCmdLine) && re.m_hmem) {
		CAppReg reg;
		SHEETOPTION so;
		if (reg.GetBinary(IDS_SEC_WEBALBUM, IDS_KEY_STATUS, (BYTE*)&so, sizeof(so)) != sizeof(so)) {
			memset(&so, 0, sizeof(so));
			so.column = 4;
			so.row = 3;
			so.item_width = 128;
			so.item_height = 96;
			so.rgb = RGB(192, 192, 192);
			so.keisen = TRUE;
		}

		CHtmlSheetDlg dlg;
		if (dlg.DoModal(hwnd, (LPARAM)&so) == IDOK) {
			reg.SetBinary(IDS_SEC_WEBALBUM, IDS_KEY_STATUS, (BYTE*)&so, sizeof(so));
			make_it(hwnd, (HDROP)re.m_hmem, &so);
		}
	}
}

extern "C"
void WINAPI RunWebAlbum(HWND hwnd, HINSTANCE hInst, LPSTR lpCmdLine, int nCmdShow)
{
	WCHAR cmd[MAX_PATH * 4];
	MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, cmd, _COUNTOF(cmd));
	RunWebAlbumW(hwnd, hInst, cmd, nCmdShow);
}
