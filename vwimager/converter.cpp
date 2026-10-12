//////////////////////////////////////////////////////////////////////
// converter.cpp

#include "stdafx.h"
#include "progress_sup.h"
#include "common.h"
#include "jpegoption.h"
#include "webpoption.h"
#include "fpxoption.h"
#include "avif_option.h"
#include "jp2koption.h"
#include "resource.h"
#include "dropexec.h"
#include "..\lscore\cdll.h"


typedef struct tagCONVERTOPTION {
  int image_type;
  int base_type;
  int base_values[5];
  BOOL do_resize;
  BOOL under_noconvert;
  BOOL delete_source;
  TCHAR path[MAX_PATH * 2];
  EFC_JPEGOPTION jpeg_opt;
  EFC_JPEGOPTION fpx_opt;
  EFC_JPEGOPTION jp2_opt;
  WEBPOPTION webp_opt;
  AVIFOPTION avif_opt;
  int interlace;
} CONVERTOPTION;


//////////////////////////////////////////////////////////////////////
//

typedef struct tagIMAGEINFODATA {
  int type;
  TCHAR* title;
  TCHAR* text;
} IMAGEINFODATA;

IMAGEINFODATA stImageData[] = {
  IMG_JPEG,	_T("JPEG"),		_T("JPEG"),
  IMG_PNG,	_T("PNG"),		_T("Portable Network Graphics"),
  IMG_WEBP,	_T("WEBP"),		_T("WebP"),
  IMG_AVIF,	_T("AVIF"),		_T("AV1 Image File Format"),
  IMG_GIF,	_T("GIF"),		_T("CompuServe GIF"),
  IMG_BMP,	_T("BMP"),		_T("Windows Bitmap"),
  IMG_TIFF,	_T("TIFF"),		_T("Tag Image File Format"),
  IMG_TGA,	_T("TGA"),		_T("Truevision Targa"),
  IMG_PCX,	_T("PCX"),		_T("Zsoft PCX"),
  IMG_PNM,	_T("PNM"),		_T("Portable aNy Map"),
  IMG_FPX,	_T("FPX"),		_T("Flash Pix"),
  IMG_JP2,	_T("JPEG2000"),	_T("JPEG-2000 ISO/IEC 15444-1"),
  -1,			NULL,			NULL
};

#define ON_CTRL_RANGE(code, id, idLast, func) \
	if(uMsg == WM_COMMAND && code == HIWORD(wParam) && id <= LOWORD(wParam) && idLast >= LOWORD(wParam)) { \
		func((UINT)LOWORD(wParam)); \
		return TRUE; \
	}

class CConvertDlg : public CDialogImpl<CConvertDlg>
{
public:
	enum { IDD = IDD_CONVERT };
	CONVERTOPTION* m_pco;
	CListViewCtrl m_list;
	CComboBox m_zoom;
	CComboBox m_folder;
	CString m_basedir;
	CImageList m_imPal;
	CTrackBarCtrl m_zslider;

BEGIN_MSG_MAP(CConvertDlg)
	ON_WM_INITDIALOG()
	ON_WM_HSCROLL()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_IMAGETYPE, OnNotifyItemChanged)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_CTRL_RANGE(BN_CLICKED, IDC_DORESIZE, IDC_OVERSIZE, OnCheckBaseRange)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_PARAM, OnSetParam)
	ON_CBN_SELCHANGE(IDC_ZOOM_VALUE, OnZoomSelChange)
	ON_CBN_EDITCHANGE(IDC_ZOOM_VALUE, OnZoomEditChange)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnNotifyItemChanged(NMHDR* phdr, LRESULT* pResult);
	void OnHScroll(UINT code, UINT pos, HWND hctl);
	void OnOK();
	void OnCancel() { EndDialog(IDCANCEL); }
	void OnCheckBaseRange(UINT id);
	void OnBrowse();
	void OnSetParam();
	void OnZoomSelChange();
	void OnZoomEditChange();

	void EnableParamButton();
	void Error(UINT idCtrl);
	void ApplyZoomValue(LPCTSTR scaleStr);
};

typedef struct {
  UINT base;
  UINT value;
} respair;
respair stResizePair[] = {
  { IDC_BASE_LONG,   IDC_LONG_VALUE },
  { IDC_BASE_SHORT,  IDC_SHORT_VALUE },
  { IDC_BASE_WIDTH,  IDC_WIDTH_VALUE },
  { IDC_BASE_HEIGHT, IDC_HEIGHT_VALUE },
  { IDC_BASE_ZOOM,   IDC_ZOOM_VALUE }
};

BOOL CConvertDlg::OnInitDialog(LPARAM lParam)
{
	CenterWindow();
	m_list = GetDlgItem(IDC_IMAGETYPE);
	m_zoom = GetDlgItem(IDC_ZOOM_VALUE);
	m_folder = GetDlgItem(IDC_FOLDER);
	m_zslider = GetDlgItem(IDC_ZSLIDER);

	m_pco = (CONVERTOPTION*)lParam;

	// リストコントロール
	m_imPal = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_PALETTE), 20, CLR_NONE);
	m_list.SetImageList(m_imPal, LVSIL_SMALL);
	m_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM;
	lvc.pszText = NULL;
	lvc.fmt = LVCFMT_LEFT;
	lvc.iSubItem = 0;
	lvc.cx = 10;
	m_list.InsertColumn(0, &lvc);
	lvc.iSubItem = 1;
	lvc.cx = 10;
	m_list.InsertColumn(1, &lvc);

	LV_ITEM lvi;
	int i, index, sel = 0;
	for (i = 0; ; i++) {
		if (stImageData[i].type < 0)
			break;

		lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.pszText = stImageData[i].title;
		lvi.iImage = 0;
		lvi.lParam = stImageData[i].type;
		index = m_list.InsertItem(&lvi);

		m_list.SetItemText(index, 1, stImageData[i].text);

		if (m_pco->image_type == stImageData[i].type)
			sel = index;
	}
	m_list.SetColumnWidth(0, LVSCW_AUTOSIZE);
	m_list.SetColumnWidth(1, LVSCW_AUTOSIZE);
	m_list.SelectItem(sel);
	m_list.EnsureVisible(sel, FALSE);

	TCHAR buf[128];
	for (i = 20; i <= 200; i += 20) {
		wsprintf(buf, _T("%d"), i);
		index = m_zoom.AddString(buf);
		if (i == 100) m_zoom.SetCurSel(index);
	}

	CheckDlgButton(IDC_DORESIZE, m_pco->do_resize);
	switch(m_pco->under_noconvert) {
		case 1: 
			CheckDlgButton(IDC_UNDERSIZE, TRUE);
			break;
		case 2:
			CheckDlgButton(IDC_OVERSIZE, TRUE);
			break;
	}

	// zoomスライダー
	m_zslider.SetRange(20, 200);
	m_zslider.SetPageSize(10);
	m_zslider.SetTicFreq(10);

	CheckDlgButton(stResizePair[m_pco->base_type].base, TRUE);
	for (i = 0; i < ARRAYSIZE(stResizePair); i++) {
		if (m_pco->base_values[i]) {
			SetDlgItemInt(stResizePair[i].value, m_pco->base_values[i], FALSE);
			if (i == 4) {
				const int scale = m_pco->base_values[i];
				wsprintf(buf, _T("%d"), scale);
				index = m_zoom.FindStringExact(-1, buf);
				if (index != CB_ERR)
					m_zoom.SetCurSel(index);
				else
					m_zoom.SetWindowText(buf);
				m_zslider.SetPos(scale);
			}
		}
	}
	CheckDlgButton(IDC_DELREPLACE, m_pco->delete_source);

	CAppReg reg;
	reg.LoadComboHistory(m_folder, IDS_SEC_CONVERTER, IDS_KEY_FOLDERHISTORY, 10);

	OnCheckBaseRange(IDC_BASE_LONG);
	EnableParamButton();

	if (!m_basedir.IsEmpty()) {
		m_folder.SetWindowText(m_basedir);
	}
	// オートコンプリート
	SHAutoComplete(m_folder.GetDlgItem(1001), SHACF_AUTOSUGGEST_FORCE_ON | SHACF_FILESYS_DIRS);
	return TRUE;
}

void CConvertDlg::OnCheckBaseRange(UINT id)
{
	BOOL bResize = IsDlgButtonChecked(IDC_DORESIZE);
	int i;
	for (i = 0; i < ARRAYSIZE(stResizePair); i++) {
		if (!bResize) {
			GetDlgItem(stResizePair[i].base).EnableWindow(FALSE);
			GetDlgItem(stResizePair[i].value).EnableWindow(FALSE);
		}
		else {
			GetDlgItem(stResizePair[i].base).EnableWindow(TRUE);
			GetDlgItem(stResizePair[i].value).EnableWindow(IsDlgButtonChecked(stResizePair[i].base));
		}
	}

	const BOOL bZoom = id == IDC_BASE_ZOOM;
	GetDlgItem(IDC_UNDERSIZE).EnableWindow(bResize && !bZoom);
	GetDlgItem(IDC_OVERSIZE).EnableWindow(bResize && !bZoom);
	m_zslider.EnableWindow(bZoom);
}

void CConvertDlg::OnNotifyItemChanged(NMHDR* phdr, LRESULT* pResult)
{
	EnableParamButton();
}

void CConvertDlg::OnBrowse()
{
	TCHAR buf[MAX_PATH * 2];
	if (m_folder.GetWindowText(buf, ARRAYSIZE(buf)) < 1)
		buf[0] = '\0';

	if (fopBrowseForFolder(m_hWnd, buf))
		m_folder.SetWindowText(buf);
}

void CConvertDlg::OnHScroll(UINT code, UINT pos, HWND hctl)
{
	if (hctl != m_zslider)
		return;

	CString str;
	switch (code) {
		case TB_TOP:
		case TB_BOTTOM:
		case TB_ENDTRACK:
		case TB_LINEDOWN:
		case TB_LINEUP:
		case TB_PAGEDOWN:
		case TB_PAGEUP:
			str.Format(_T("%d"), m_zslider.GetPos());
			m_zoom.SetWindowText(str);
			break;
		case TB_THUMBPOSITION:
		case TB_THUMBTRACK:
			str.Format(_T("%d"), pos);
			m_zoom.SetWindowText(str);
			break;
	}
}

void CConvertDlg::ApplyZoomValue(LPCTSTR scaleStr)
{
	int scale = max(20, min(200, _ttoi(scaleStr)));
	m_zslider.SetPos(scale);
}

// コンボボックスで選択された項目値をスライダーに反映させる
void CConvertDlg::OnZoomSelChange()
{
	int sel = m_zoom.GetCurSel();
	CString str;
	m_zoom.GetLBText(sel, str);
	ApplyZoomValue(str);
}

void CConvertDlg::OnZoomEditChange()
{
	CString str;
	m_zoom.GetWindowText(str);
	ApplyZoomValue(str);
}

void CConvertDlg::OnSetParam()
{
	int type = (int)m_list.GetItemData( m_list.GetNextItem(-1, LVNI_SELECTED) );
	if (type == IMG_JPEG) {				// JPEG
		EFC_JPEGOPTION jo;
		memcpy(&jo, &m_pco->jpeg_opt, sizeof(jo));
		CJpegOption opt;
		if (opt.DoModal(m_hWnd, (LPARAM)&jo) == IDOK)
			memcpy(&m_pco->jpeg_opt, &jo, sizeof(jo));
	} else if (type == IMG_FPX) {		// FPX
		EFC_JPEGOPTION jo;
		memcpy(&jo, &m_pco->fpx_opt, sizeof(jo));
		CFpxOption opt;
		if (opt.DoModal(m_hWnd, (LPARAM)&jo) == IDOK)
			memcpy(&m_pco->fpx_opt, &jo, sizeof(jo));
	} else if (type == IMG_JP2) {		// JPEG2000
		EFC_JPEGOPTION jo;
		memcpy(&jo, &m_pco->jp2_opt, sizeof(jo));
		CJp2Option opt;
		if (opt.DoModal(m_hWnd, (LPARAM)&jo) == IDOK)
			memcpy(&m_pco->jp2_opt, &jo, sizeof(jo));
	} else if (type == IMG_WEBP) {		// WEBP
		WEBPOPTION wo;
		memcpy(&wo, &m_pco->webp_opt, sizeof(wo));
		CWebpOption opt;
		if (opt.DoModal(m_hWnd, (LPARAM)&wo) == IDOK)
			memcpy(&m_pco->webp_opt, &wo, sizeof(wo));
	} else if (type == IMG_AVIF) {		// AVIF
		AVIFOPTION ao;
		memcpy(&ao, &m_pco->avif_opt, sizeof(ao));
		CAvifOption opt;
		if (opt.DoModal(m_hWnd, (LPARAM)&ao) == IDOK)
			memcpy(&m_pco->avif_opt, &ao, sizeof(ao));
	}
}

void CConvertDlg::EnableParamButton()
{
	int type = (int)m_list.GetItemData( m_list.GetNextItem(-1, LVNI_SELECTED) );
	GetDlgItem(IDC_PARAM).EnableWindow((type == IMG_JPEG || type == IMG_FPX || type == IMG_JP2 || type == IMG_WEBP || type == IMG_AVIF));
}

void CConvertDlg::Error(UINT idCtrl)
{
	MessageBeep(0);
	GetDlgItem(idCtrl).SetFocus();
}

void CConvertDlg::OnOK()
{
	int type = (int)m_list.GetItemData( m_list.GetNextItem(-1, LVIS_SELECTED) );
	if (type <= 0) {
		Error(IDC_IMAGETYPE);
		return;
	}
	m_pco->image_type = type;
	m_pco->do_resize = IsDlgButtonChecked(IDC_DORESIZE);

	for (int i = 0; i < ARRAYSIZE(stResizePair); i++) {
		if (IsDlgButtonChecked(stResizePair[i].base)) {
			m_pco->base_values[i] = GetDlgItemInt(stResizePair[i].value, NULL, TRUE);
			if (m_pco->base_values[i] <= 0) {
				Error(stResizePair[i].value);
				return;
			}
			m_pco->base_type = i;
			break;
		}
	}
	if (IsDlgButtonChecked(IDC_UNDERSIZE)) {
		m_pco->under_noconvert = 1;
	} else if (IsDlgButtonChecked(IDC_OVERSIZE)) {
		m_pco->under_noconvert = 2;
	} else {
		m_pco->under_noconvert = 0;
	}
  
	if (m_folder.GetWindowText(m_pco->path, ARRAYSIZE(m_pco->path)) < 1) {
		Error(IDC_FOLDER);
		return;
	}
	m_pco->delete_source = IsDlgButtonChecked(IDC_DELREPLACE);

	CAppReg reg;
	reg.SaveComboHistory(m_folder, IDS_SEC_CONVERTER, IDS_KEY_FOLDERHISTORY, 10, m_pco->path);
	EndDialog(IDOK);
}

/////////////////////////////////////////////////////////////////////
//

static HDIB convert_size(HDIB hdib, CONVERTOPTION* pco)
{
  if (!hdib)
	return NULL;

  LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hdib);
  if (!pbh) {
	  GlobalFree(hdib);
	  return NULL;
  }
  int width = pbh->biWidth;
  int height = pbh->biHeight;
  int bits = pbh->biBitCount;
  GlobalUnlock(hdib);

  int x, y, value = pco->base_values[pco->base_type];
  if (pco->base_type == 0) {			// 長辺
	if (width > height) {
	  if ((pco->under_noconvert == 1 && width < value) ||
		  (pco->under_noconvert == 2 && width > value)) {
		return hdib;
	  }

	  x = value;
	  y = MulDiv(x, height, width);
	} else {
	  if ((pco->under_noconvert == 1 && height < value) ||
		  (pco->under_noconvert == 2 && height > value)) {
		return hdib;
	  }

	  y = value;
	  x = MulDiv(y, width, height);
	}
  } else if (pco->base_type == 1) {		// 短辺
	if (width < height) {
	  if ((pco->under_noconvert == 1 && width < value) ||
		  (pco->under_noconvert == 2 && width > value)) {
		return hdib;
	  }

	  x = value;
	  y = MulDiv(x, height, width);
	} else {
	  if ((pco->under_noconvert == 1 && height < value) ||
		  (pco->under_noconvert == 2 && height > value)) {
		return hdib;
	  }

	  y = value;
	  x = MulDiv(y, width, height);
	}
  } else if (pco->base_type == 2) {		// 幅
	if (pco->under_noconvert && width < value)
	  return hdib;
	if ((pco->under_noconvert == 1 && width < value) ||
	  (pco->under_noconvert == 2 && width > value)) {
	  return hdib;
	}

	x = value;
	y = MulDiv(x, height, width);
  } else if (pco->base_type == 3) {		// 高さ
	if ((pco->under_noconvert == 1 && height < value) ||
		(pco->under_noconvert == 2 && height > value)) {
	  return hdib;
	}

	y = value;
	x = MulDiv(y, width, height);
  } else {								// 率
	if (value == 100)
	  return hdib;

	x = (width * value) / 100;
	y = (height * value) / 100;
  }

  if (!x || !y) {
	GlobalFree(hdib);
	return NULL;
  }

  mif image(hdib);
  GlobalFree(hdib);
  image.zoom(x, y, zoomMitchell, FALSE);
  return image.createdib();
}

static void save_to(HDIB hdib, LPTSTR pszSrc, LPTSTR pszTo, CONVERTOPTION* pco)
{
  if (!hdib)
	return;

  TCHAR tmp[MAX_PATH * 2];
  if (pco->delete_source) {
	for (int i = 0; i < 99; i++) {
	  wsprintf(tmp, _T("%s.~%02d"), pszSrc, i);
	  if (!PathFileExists(tmp))
		break;
	}
	MoveFile(pszSrc, tmp);
  }

  switch (pco->image_type) {
	  case IMG_JPEG: {
		  JPEG_CPARAMS jc;
		  memset(&jc, 0, sizeof(jc));
		  jc.quality = pco->jpeg_opt.quality;
		  jc.dct = JC_ISLOW;
		  jc.smooth = pco->jpeg_opt.smoothing;
		  jc.cdata = NULL;
		  if (pco->jpeg_opt.type == 1)
			  jc.baseline = TRUE;
		  else if (pco->jpeg_opt.type == 2)
			  jc.progressive = TRUE;
		  imgJpgSaveParams(pszTo, hdib, &jc);
		  break;
	  }
	  case IMG_PNG:
		  imgPngSave(pszTo, hdib);
		  break;
	  case IMG_GIF:
		  imgGifSave(pszTo, hdib);
		  break;
	  case IMG_WEBP:
		  imgWebpSaveParams(pszTo, hdib, pco->webp_opt.method, pco->webp_opt.quality, pco->webp_opt.preset);
		  break;
	  case IMG_AVIF:
		  imgAvifSaveParams(pszTo, hdib, pco->avif_opt.quality, pco->avif_opt.speed);
		  break;
	  case IMG_BMP:
		  imgDibSave(pszTo, hdib);
		  break;
	  case IMG_TIFF:
		  imgTifSave(pszTo, hdib);
		  break;
	  case IMG_TGA:
		  imgTgaSave(pszTo, hdib);
		  break;
	  case IMG_RASTER:
		  imgRasSave(pszTo, hdib);
		  break;
	  case IMG_PCX:
		  imgPcxSave(pszTo, hdib);
		  break;
	  case IMG_FPX:
		  imgFpxSaveEx(pszTo, hdib,
			  pco->fpx_opt.type ? FPXCOMPRESS_JPEG_UNSPECIFIED : FPXCOMPRESS_NONE,
			  pco->fpx_opt.quality);
		  break;
	  case IMG_JP2: {
			JP2_CPARAMS j2;
			memset(&j2, 0, sizeof(j2));
			j2.rate = pco->jp2_opt.quality;
			j2.xcb = JP2_DEF_XCB;
			j2.ycb = JP2_DEF_YCB;
			j2.nomct = JP2_DEF_NOMCT;
			j2.reslevel = JP2_DEF_RESLEVEL;
			imgJp2SaveEx(pszTo, hdib, &j2);
			break;
	  }
	  case IMG_PNM:
		  imgPnmSave(pszTo, hdib);
		  break;
	}

	if (pco->delete_source) {
		if (PathFileExists(pszTo)) {
			SetFileAttributes(tmp, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(tmp);
		}
		else
			MoveFile(tmp, pszSrc);
	}
}

static BOOL convert_it(HWND hwnd, HDROP hdrop, CONVERTOPTION* pco)
{
  int cnt = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
  if (!cnt) return FALSE;

  CProgressDialog progress;
  progress.SetTotalSize(cnt);
  progress.StartProgressDialog(hwnd, PROGDLG_AUTOTIME | PROGDLG_NOMINIMIZE);

  TCHAR src[MAX_PATH * 2], dst[MAX_PATH * 2];
  for (int i = 0; i < cnt; i++) {
	if (progress.HasUserCancelled())
	  break;

	DragQueryFile(hdrop, i, src, ARRAYSIZE(src));
	PathCombine(dst, pco->path, PathFindFileName(src));

	// 必要なら拡張子を変更する
	rename_imgextension(dst, pco->image_type);
	progress.SetMessage(MAKEINTRESOURCE(IDS_PROCESSING_READ));
	progress.SetLine(2, PathFindFileName(src), TRUE);
	HDIB hdib = imgLoadImage(src);

	if (hdib) {
	  if (pco->do_resize) {
		progress.SetMessage(MAKEINTRESOURCE(IDS_PROCESSING_CONVERT));
		hdib = convert_size(hdib, pco);
	  }

	  if (hdib) {
		progress.SetMessage(MAKEINTRESOURCE(IDS_PROCESSING_WRITE));
		progress.SetLine(2, PathFindFileName(dst), TRUE);
		save_to(hdib, src, dst, pco);
		GlobalFree(hdib);
	  }
	  progress.SetCompletedSize(i);
	}
  }
  progress.StopProgressDialog();
  return TRUE;
}

static BOOL TrushFile(HWND hwnd, LPCTSTR pszFile)
{
  TCHAR name[MAX_PATH * 2];
  lstrcpy(name, pszFile);
  name[lstrlen(name) + 1] = 0;				// end with \0\0

  SHFILEOPSTRUCT sfs = { 0 };
  sfs.hwnd = hwnd;
  sfs.wFunc = FO_DELETE;
  sfs.pFrom = name;
  sfs.fFlags= FOF_ALLOWUNDO | FOF_FILESONLY | FOF_NOCONFIRMATION | FOF_SILENT;
  return (SHFileOperation(&sfs) == 0);
}

static BOOL save_jpeg(int quality, HDIB hdib, LPCTSTR name)
{
  BOOL ok = FALSE;
  if (hdib) {
	JPEG_CPARAMS jc;
	memset(&jc, 0, sizeof(jc));
	jc.quality = quality;
	jc.dct = JC_ISLOW;
	jc.smooth = 0;
	jc.cdata = NULL;
	ok = imgJpgSaveParams(name, hdib, &jc);
  }
  return ok;
}

static HDIB mclip(HANDLE hdib, RECT* prc)
{
  mif image;
  if (image.attachdib(hdib) && image.crop(prc))
	return image.createdib();
  return NULL;
}

static BOOL SplitImages(HWND hwnd, HDROP hdrop)
{
  int cnt = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
  if (!cnt) return FALSE;

  // TaskDialogIndirect
  CDll commctrl(::GetModuleHandle(_T("comctl32.dll")));
  HRESULT (WINAPI* _fTaskDialogIndirect)(const TASKDIALOGCONFIG*, int*, int*, BOOL*);
  (FARPROC&)_fTaskDialogIndirect = commctrl.FindFunction("TaskDialogIndirect");

  CProgressDialog progress;
  progress.SetTotalSize(cnt);
  progress.StartProgressDialog(hwnd, PROGDLG_AUTOTIME | PROGDLG_NOMINIMIZE);

  BOOL yes2all = FALSE;
  BOOL splitany = TRUE;

  TCHAR src[MAX_PATH * 2], dst[MAX_PATH * 2];
  CRect rc;
  for (int i = 0; i < cnt; i++) {
	if (progress.HasUserCancelled())
	  break;

	DragQueryFile(hdrop, i, src, ARRAYSIZE(src));
	// 必要なら拡張子を変更する
	progress.SetMessage(MAKEINTRESOURCE(IDS_PROCESSING_READ));
	progress.SetLine(1, PathFindFileName(src), TRUE);

	HDIB hdib = imgLoadImage(src);
	if (hdib) {
	  LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hdib);
	  int width = pbh->biWidth;
	  int height = pbh->biHeight;
	  GlobalUnlock(hdib);

	  if ((height > width) && _fTaskDialogIndirect && !yes2all) {
		TASKDIALOGCONFIG tdc;
		memset(&tdc, 0, sizeof(tdc));
		tdc.cbSize =				sizeof(tdc);
		tdc.hwndParent =			hwnd;
		tdc.hInstance =				RESINST;
		tdc.dwFlags =				TDF_ALLOW_DIALOG_CANCELLATION;
		tdc.dwCommonButtons =		TDCBF_YES_BUTTON | TDCBF_NO_BUTTON | TDCBF_CANCEL_BUTTON;
		tdc.pszWindowTitle =		MAKEINTRESOURCE(IDS_TITLE_SPLITIMAGE);
		tdc.pszMainIcon =			TD_INFORMATION_ICON;
		CString str; str.Format(IDS_VERTICAL_RECTANGLE, PathFindFileName(src), width, height);
		tdc.pszMainInstruction =	str.GetString();
		tdc.pszVerificationText =	MAKEINTRESOURCE(IDS_YESTOALL);
		int button = 0;
		if (_fTaskDialogIndirect(&tdc, &button, NULL, &yes2all) != S_OK) {
		  GlobalFree(hdib);
		  break;
		}
		splitany = (button == IDYES);
	  }

	  if (height < width || splitany) {
		int done = 0;
		lstrcpy(dst, src);
		PathRemoveExtension(dst);
		lstrcat(dst, _T("-1.jpg"));
		rc.SetRect(width / 2, 0, width, height);
		progress.SetMessage(MAKEINTRESOURCE(IDS_PROCESSING_WRITE));
		progress.SetLine(1, PathFindFileName(dst), TRUE);
		HDIB hw = mclip(hdib, &rc);
		if (hw) {
		  if (save_jpeg(70, hw, dst))
			done++;
		  GlobalFree(hw);
		}

		lstrcpy(dst, src);
		PathRemoveExtension(dst);
		lstrcat(dst, _T("-2.jpg"));
		progress.SetLine(1, PathFindFileName(dst), TRUE);
		rc.SetRect(0, 0, width / 2, height);
		hw = mclip(hdib, &rc);
		if (hw) {
		  if (save_jpeg(70, hw, dst))
			done++;
		  GlobalFree(hw);
		}

		if (done == 2) {
		  TrushFile(hwnd, src);
		}
	  }
	  GlobalFree(hdib);
	  progress.SetCompletedSize(i);
	}
  }
  progress.StopProgressDialog();
  return TRUE;
}

static BOOL ConvertImageCore(HWND hwnd, HDROP hdrop)
{
  if (!hdrop || DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0) < 1)
	return FALSE;

  CONVERTOPTION co;
  CAppReg reg;
  if (reg.GetBinary(IDS_SEC_CONVERTER, IDS_KEY_STATUS, (BYTE*)&co, sizeof(co)) != sizeof(co)) {
	memset(&co, 0, sizeof(co));

	co.base_type = 4;
	co.base_values[4] = 100;
	co.do_resize = TRUE;
	co.jpeg_opt.type = 1;
	co.jpeg_opt.quality = 90;
	co.jpeg_opt.smoothing = 0;
	co.fpx_opt.type = 1;
	co.fpx_opt.quality = 90;
	co.fpx_opt.smoothing = 0;
	co.jp2_opt.type = 0;
	co.jp2_opt.quality = 8;
	co.jp2_opt.smoothing = 0;
	co.webp_opt.quality = 80;
	co.webp_opt.method = 3;
	co.webp_opt.preset = WEC_PRESET_DEFAULT;
	co.avif_opt.quality = 60;
	co.avif_opt.speed = 4;

	co.interlace = FALSE;
  }

  BOOL bOK = FALSE;
  CConvertDlg dlg;
  TCHAR dir[MAX_PATH * 2];
  DragQueryFile(hdrop, 0, dir, ARRAYSIZE(dir));
  PathRemoveFileSpec(dir);
  dlg.m_basedir = dir;
  if (dlg.DoModal(hwnd, (LPARAM)&co) == IDOK) {
	reg.SetBinary(IDS_SEC_CONVERTER, IDS_KEY_STATUS, (BYTE*)&co, sizeof(co));
	if (fopValidateDir(hwnd, co.path)) {
	  bOK = convert_it(hwnd, hdrop, &co);
	}
  }
  return bOK;
}

static BOOL ContrastResize(HWND hwnd, HDROP hdrop)
{
  int cnt = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
  if (!cnt) return FALSE;

  CProgressDialog progress;
  progress.SetTotalSize(cnt);
  progress.StartProgressDialog(hwnd, PROGDLG_AUTOTIME | PROGDLG_NOMINIMIZE);

  TCHAR src[MAX_PATH * 2], dst[MAX_PATH * 2];
  for (int i = 0; i < cnt; i++) {
	if (progress.HasUserCancelled())
	  break;

	DragQueryFile(hdrop, i, src, ARRAYSIZE(src));
	LPCTSTR fname = PathFindFileName(src);

	// 必要なら拡張子を変更する
	progress.SetMessage(MAKEINTRESOURCE(IDS_PROCESSING_READ));
	progress.SetLine(2, fname, TRUE);
	HDIB hdib = imgLoadImage(src);
	if (hdib) {
	  mif image(hdib);
	  GlobalFree(hdib);
//			image.bright_contrast(NULL, 0, 100);
	  image.grayscale();

	  int width = image.width();
	  int height = image.height();
	  if (height > 1600) {
		image.zoom(width * 1600 / height, 1600, zoomMitchell, FALSE);
	  }
	  image.normalize(NULL);

	  hdib = image.createdib();
	  progress.SetMessage(MAKEINTRESOURCE(IDS_PROCESSING_WRITE));
	  progress.SetLine(2, fname, TRUE);

	  lstrcpy(dst, src);
	  if (!TrushFile(hwnd, src)) {
		lstrcat(dst, _T(".cr.jpg"));
	  }
	  save_jpeg(70, hdib, dst);
	  GlobalFree(hdib);
	  image.destroy();

	  progress.SetCompletedSize(i);
	}
  }
  progress.StopProgressDialog();
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////

typedef struct {
  BOOL overwrite;
  int quality;
} CROPOPTION;

class CCropImageDlg : public CDialogImpl<CCropImageDlg>
{
public:
  enum { IDD = IDD_CROPIMAGE };
  CRect m_rc;
  CROPOPTION m_op;
  CTrackBarCtrl m_slider;
  int m_quality;

BEGIN_MSG_MAP(CCropImageDlg)
  ON_WM_INITDIALOG()
  ON_BN_CLICKED(IDOK, OnOK)
  ON_BN_CLICKED(IDCANCEL, OnCancel)
  ON_WM_HSCROLL()

END_MSG_MAP()

  BOOL OnInitDialog(LPARAM lParam)
  {
	CAppReg reg;
	if (reg.GetBinary(IDS_SEC_CROPIMAGE, IDS_KEY_STATUS, (BYTE*)&m_op, sizeof(m_op)) != sizeof(m_op)) {
	  m_op.quality = 75;
	  m_op.overwrite = FALSE;
	}

	CenterWindow();
	m_rc.SetRectEmpty();
	m_slider = GetDlgItem(IDC_SLIDER);
	m_slider.SetRange(1, 100);
	m_slider.SetTicFreq(10);
	m_slider.SetPageSize(10);
	m_slider.SetLineSize(1);
	m_slider.SetPos(m_op.quality);
	SetDlgItemInt(IDC_QUALITY, m_op.quality);
	CheckDlgButton(IDC_OVERWRITEIMAGE, m_op.overwrite);

	return TRUE;
  }

  void OnHScroll(UINT nSBCode, UINT nPos, HWND hctl)
  {
	if (hctl != m_slider) {
	  DefWindowProc();
	  return;
	}

	int val;
	switch (nSBCode) {
	  case TB_THUMBTRACK: val = (int)nPos; break;

	  case TB_TOP:
	  case TB_BOTTOM:
	  case TB_LINEUP:
	  case TB_LINEDOWN:
	  case TB_PAGEUP:
	  case TB_PAGEDOWN: val = m_slider.GetPos(); break;

	  default: return;
	}

	SetDlgItemInt(IDC_QUALITY, val, TRUE);
  }

  void OnOK()
  {
	m_rc.left = GetDlgItemInt(IDC_LEFTCROP);
	m_rc.right = GetDlgItemInt(IDC_RIGHTCROP);
	m_rc.top = GetDlgItemInt(IDC_TOPCROP);
	m_rc.bottom = GetDlgItemInt(IDC_BOTTOMCROP);
	m_op.overwrite = IsDlgButtonChecked(IDC_OVERWRITEIMAGE);
	if (!m_rc.IsRectNull()) {
	  CAppReg reg;
	  reg.SetBinary(IDS_SEC_CROPIMAGE, IDS_KEY_STATUS, (BYTE*)&m_op, sizeof(m_op));
	  EndDialog(IDOK);
	}
  }

  void OnCancel() { EndDialog(IDCANCEL); }
};

static void CropImage(HWND hwnd, HDROP hdrop)
{
  int cnt = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
  if (!cnt) return;

  CCropImageDlg dlg;
  if (dlg.DoModal(hwnd) != IDOK) return;

  CProgressDialog progress;
  progress.SetTotalSize(cnt);
  progress.StartProgressDialog(hwnd, PROGDLG_AUTOTIME | PROGDLG_NOMINIMIZE);

  TCHAR src[MAX_PATH * 2], dst[MAX_PATH * 2];
  for (int i = 0; i < cnt; i++) {
	if (progress.HasUserCancelled())
	  break;

	DragQueryFile(hdrop, i, src, ARRAYSIZE(src));
	LPCTSTR fname = PathFindFileName(src);

	// 必要なら拡張子を変更する
	progress.SetMessage(MAKEINTRESOURCE(IDS_PROCESSING_READ));
	progress.SetLine(2, fname, TRUE);
	HDIB hdib = imgLoadImage(src);
	if (hdib) {
	  mif image(hdib);
	  GlobalFree(hdib);
	  int width = image.width();
	  int height = image.height();

	  CRect rc;
	  rc.left = dlg.m_rc.left;
	  rc.right = width - dlg.m_rc.right;
	  rc.top = dlg.m_rc.top;
	  rc.bottom = height - dlg.m_rc.bottom;
	  if (rc.right >= rc.left && rc.bottom >= rc.top) {
		image.crop(&rc);
		hdib = image.createdib();

		progress.SetMessage(MAKEINTRESOURCE(IDS_PROCESSING_WRITE));
		progress.SetLine(2, fname, TRUE);

		lstrcpy(dst, src);
		// 元形式に関係なくjpegで出力するので拡張子を変更する
		PathRenameExtension(dst, _T(".jpg"));
		if (dlg.m_op.overwrite) {
		  TrushFile(hwnd, src);
		} else if (!StrCmpI(src, dst)) {
		  // "(cropped)<original_filebopdy"
		  TCHAR fs[MAX_PATH], fp[MAX_PATH];
		  lstrcpy(fp, dst);
		  PathRemoveFileSpec(fp);
		  lstrcpy(fs, dst);
		  PathStripPath(fs);
		  CString str = _T("(cropped)");
		  str += fs;
		  PathCombine(dst, fp, str);
		}
		save_jpeg(dlg.m_op.quality, hdib, dst);
		GlobalFree(hdib);
	  }
	  image.destroy();
	}
	progress.SetCompletedSize(i);
  }
  progress.StopProgressDialog();
}

// defunc'd
extern "C" BOOL WINAPI vwiConvertImage(HWND hwnd, HDROP hdrop) { return FALSE; }

extern "C"
void WINAPI RunConvertImageW(HWND hwnd, HINSTANCE hInst, LPWSTR lpCmdLine, int nCmdShow)
{
  CRegExec re;
  if (re.Initialize(lpCmdLine) && re.m_hmem)
	ConvertImageCore(hwnd, (HDROP)re.m_hmem);
}

extern "C"
void WINAPI RunConvertImage(HWND hwnd, HINSTANCE hInst, LPSTR lpCmdLine, int nCmdShow)
{
  WCHAR cmd[MAX_PATH * 4];
  MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, cmd, ARRAYSIZE(cmd));
  RunConvertImageW(hwnd, hInst, cmd, nCmdShow);
}

extern "C"
void WINAPI RunSplitImageW(HWND hwnd, HINSTANCE hInst, LPWSTR lpCmdLine, int nCmdShow)
{
  CRegExec re;
  if (re.Initialize(lpCmdLine) && re.m_hmem)
	SplitImages(hwnd, (HDROP)re.m_hmem);
}

extern "C"
void WINAPI RunSplitImage(HWND hwnd, HINSTANCE hInst, LPSTR lpCmdLine, int nCmdShow)
{
  WCHAR cmd[MAX_PATH * 4];
  MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, cmd, ARRAYSIZE(cmd));
  RunSplitImageW(hwnd, hInst, cmd, nCmdShow);
}

extern "C"
void WINAPI RunContrastResizeW(HWND hwnd, HINSTANCE hInst, LPWSTR lpCmdLine, int nCmdShow)
{
  CRegExec re;
  if (re.Initialize(lpCmdLine) && re.m_hmem)
	ContrastResize(hwnd, (HDROP)re.m_hmem);
}

extern "C"
void WINAPI RunContrastResize(HWND hwnd, HINSTANCE hInst, LPSTR lpCmdLine, int nCmdShow)
{
  WCHAR cmd[MAX_PATH * 4];
  MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, cmd, ARRAYSIZE(cmd));
  RunContrastResizeW(hwnd, hInst, cmd, nCmdShow);
}

extern "C"
void WINAPI RunCropImageW(HWND hwnd, HINSTANCE hInst, LPWSTR lpCmdLine, int nCmdShow)
{
  CRegExec re;
  if (re.Initialize(lpCmdLine) && re.m_hmem)
	CropImage(hwnd, (HDROP)re.m_hmem);
}

extern "C"
void WINAPI RunCropImage(HWND hwnd, HINSTANCE hInst, LPSTR lpCmdLine, int nCmdShow)
{
  WCHAR cmd[MAX_PATH * 4];
  MultiByteToWideChar(CP_ACP, 0, lpCmdLine, -1, cmd, ARRAYSIZE(cmd));
  RunCropImageW(hwnd, hInst, cmd, nCmdShow);
}
