// ewxport_gif.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "export_gif.h"
#include "pal_table.h"
#include "common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int qcolor[] = {
	2, 4, 8, 16, 32, 64, 128, 256
};

static int methods[] = {
	ditherNo, ditherFS, ditherFSLowBleed, ditherFixed
};


static void __fastcall format_scale_text(double scale, LPTSTR pszFormat)
{
	int iScale = (int)(scale * 1000);
	int s = iScale / 10;
	int m = iScale % 10;
	if (!m)
		wsprintf(pszFormat, _T("%d%%"), s);
	else
		wsprintf(pszFormat, _T("%d.%d%%"), s, m);
}

/////////////////////////////////////////////////////////////////////////////
// CExportGifDlg ダイアログ


CExportGifDlg::CExportGifDlg()
{
	m_colors = 256;
	m_method = ditherFS;
	m_hpal = NULL;

	m_hdib = NULL;
	m_hdibWorker = NULL;
	m_hdibResult = NULL;

	m_xparent = RGB(192, 192, 192);
	memset(m_xsel, 0, sizeof(m_xsel));
	m_xselcnt = 0;
	m_idCaption = 0;
}

CExportGifDlg::~CExportGifDlg()
{
	if (m_hdibWorker) GlobalFree(m_hdibWorker);
	if (m_hdibResult) GlobalFree(m_hdibResult);
	if (m_hpal) DeleteObject(m_hpal);
}

/////////////////////////////////////////////////////////////////////////////
// CExportGifDlg メッセージ ハンドラ

BOOL CExportGifDlg::OnInitDialog(LPARAM lParam)
{
	if (!m_hdib) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	ExecuteDlgInit(IDD);
	CenterWindow();

	m_palcombo = GetDlgItem(IDC_COMBO);
	m_colcombo = GetDlgItem(IDC_EDIT);
	m_metcombo = GetDlgItem(IDC_METHOD);
	m_bps = GetDlgItem(IDC_BPS);
	m_spin = GetDlgItem(IDC_SPIN);
	m_view = GetDlgItem(IDC_PREVIEW);
	m_palette = GetDlgItem(IDC_PALETTE);

	if (m_idCaption) {
		CString str;
		str.LoadString(m_idCaption);
		SetWindowText(str);
	}

	m_palcombo.SetCurSel(0);

	m_spin.SetRange(2, 256);
	m_spin.SetPos(256);
	m_colcombo.SetCurSel(m_colcombo.GetCount() - 1);
	m_bps.SetCurSel(2);

	int sel;
	if (m_method == ditherNo)				sel = 0;
	else if (m_method == ditherFS)			sel = 1;
	else if (m_method == ditherFSLowBleed)	sel = 2;
	else									sel = 3;
	m_metcombo.SetCurSel(sel);

	ColorPicker_SetBkColor(GetDlgItem(IDC_XPARENT), m_xparent);

	int cx, cy, bits, color;
	dib_getinfo(m_hdib, &cx, &cy, &color, &bits);
	if (bits > 8) {
		CWaitCursor wc;
		UpdatePreview();
	}
	else {
		m_hdibWorker = copyhandle(m_hdib);
		ApplyPalette();
		SetViewImage();
		SetFileSize();

		CString cs;
		cs.Format(_T("%d"), color);
		m_colcombo.SetWindowText(cs);
	}

	return TRUE;
}

void CExportGifDlg::OnOK()
{
	if (m_palcombo.GetCurSel() == 0) {
		m_colors = GetDlgItemInt(IDC_EDIT, NULL, FALSE);
		if (m_colors < 2 || m_colors > 256) {
			MessageBeep(0);
			return;
		}
	}
	else {
		if (!m_hpal) {
			MessageBeep(0);
			return;
		}

		WORD w;
		GetObject(m_hpal, sizeof(WORD), &w);
		m_colors = w;
	}
	m_hdibResult = m_view.SetImage(NULL, FALSE);
	EndDialog(IDOK);
}

void CExportGifDlg::OnPalSelchange() 
{
	int sel = m_palcombo.GetCurSel();
	if (sel != CB_ERR) {
		HPALETTE hpal = NULL;
		if (sel != 0) {
			m_colcombo.EnableWindow(FALSE);
			m_spin.EnableWindow(FALSE);
			hpal = make_default_palette(sel - 1);
		}
		else {
			m_colcombo.EnableWindow(TRUE);
			m_spin.EnableWindow(TRUE);
			m_colors = GetDlgItemInt(IDC_EDIT, NULL, FALSE);
		}

		if (m_hpal)
			DeleteObject(m_hpal);
		m_hpal = hpal;

		UpdatePreview();
	}
}

#define DL_DELAY(a)		((a) * 75 / 100)

void CExportGifDlg::OnBpsChanged()
{
	int bps;
	switch (m_bps.GetCurSel()) {
		case 0:		// 14.4K
			bps = DL_DELAY(1800);
			break;

		case 1:		// 28.8K
			bps = DL_DELAY(3600);
			break;

		case 2:		// 56.6K
			bps = DL_DELAY(7200);
			break;

		default:
			bps = DL_DELAY(8000);
			break;
	}
	float t = (float)(m_savedsize + (bps >> 1)) / (float)bps;
	TCHAR buf[MAX_PATH];
	_stprintf(buf, _T("%.1f秒"), t);
	SetDlgItemText(IDC_DOWNTIME, buf);
}

HANDLE CExportGifDlg::MakePreviewImage()
{
	GIF_CPARAMS gc;
	memset(&gc, 0, sizeof(gc));
	gc.gif87a = FALSE;
	gc.interlaced = m_interlace;
	gc.xparent = (m_xselcnt >= 0 ? TRUE : FALSE);
	gc.xparent_index = 0;
	return imgGifSaveMemParamsSt(m_view.GetImage(), &gc, &m_savedsize);
}

void CExportGifDlg::SetFileSize()
{
	TCHAR buf[MAX_PATH];
	buf[0] = '\0';

	HANDLE hmem = MakePreviewImage();
	if (hmem) {
		int cx, cy;
		dib_getinfo(m_hdibWorker, &cx, &cy, NULL, NULL);
		TCHAR w[MAX_PATH];
		StrFormatByteSize(m_savedsize, w, _COUNTOF(w));
		wsprintf(buf, _T("%d x %d  %s"), cx, cy, w);
		GlobalFree(hmem);
	}
	SetDlgItemText(IDC_INFO, buf);
	OnBpsChanged();
}

void CExportGifDlg::SetViewImage()
{
	HDIB h;
	if (m_xselcnt > 0)
		h = imgMapIndexColor(m_hdibWorker, m_xsel, m_xselcnt, 0, m_xparent);
	else
		h = copyhandle(m_hdibWorker);

	h = m_view.UpdateImage(h, TRUE);
	if (h) GlobalFree(h);
	else m_view.CenterPosition();
}

void CExportGifDlg::ApplyPalette()
{
	if (m_hdibWorker) {
		HPALETTE hpal = imgCreatePalette(m_hdibWorker);
		m_palette.SendMessage(PLTM_SETPALETTE, TRUE, (LPARAM)hpal);
		SetDlgItemInt(IDC_NUMCOLORS, (int)m_palette.SendMessage(PLTM_GETCOUNT));
		if (hpal)
			DeleteObject(hpal);
	}
}

void CExportGifDlg::UpdatePreview()
{
	if (!m_hdib) return;

	Signal_SetBusy(GetDlgItem(IDC_SIGNAL), TRUE);

	HDIB h;
	if (m_hpal) {
		WORD w;
		GetObject(m_hpal, sizeof(WORD), &w);
		LOGPALETTE* plg =  (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * w);
		plg->palVersion = 0x300;
		plg->palNumEntries = w;
		GetPaletteEntries(m_hpal, 0, w, plg->palPalEntry);
		h = mifQuantizeWcp(m_hdib, m_method, plg);
		free(plg);
	}
	else
		h = mifQuantize(m_hdib, m_method, m_colors);

	if (m_hdibWorker)
		GlobalFree(m_hdibWorker);
	m_hdibWorker = h;

	m_xselcnt = 0;

	SetViewImage();
	ApplyPalette();
	SetFileSize();

	Signal_SetBusy(GetDlgItem(IDC_SIGNAL), FALSE);
}

void CExportGifDlg::OnZoomIn() 
{
	m_view.ZoomIn();
}

void CExportGifDlg::OnZoomOut() 
{
	m_view.ZoomOut();
}

void CExportGifDlg::OnDither() 
{
	m_method = methods[m_metcombo.GetCurSel()];
	UpdatePreview();
}

void CExportGifDlg::OnNumColorEnChange() 
{
	int colors = GetDlgItemInt(IDC_EDIT, NULL, FALSE);
	if (colors > 1 && colors <= 256) {
		m_colors = colors;
		UpdatePreview();
	}
}

void CExportGifDlg::OnNumColorChange() 
{
	int sel = m_colcombo.GetCurSel();
	if (sel != CB_ERR) {
		m_colors = qcolor[sel];
		UpdatePreview();
	}
}

void CExportGifDlg::OnDeltaposSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pud = (NM_UPDOWN*)pNMHDR;
	m_colors = pud->iPos;
	UpdatePreview();

	*pResult = 0;
}

void CExportGifDlg::OnXparentColorChange()
{
	m_xparent = ColorPicker_GetBkColor(GetDlgItem(IDC_XPARENT));

	SetViewImage();
	SetFileSize();
}

void CExportGifDlg::OnInterlace()
{
	m_interlace = IsDlgButtonChecked(IDC_INTERLACE);
	SetFileSize();
}

void CExportGifDlg::OnNotifySelChanged(NMHDR* pNmhdr, LRESULT* pResult)
{
	*pResult = FALSE;

	m_xselcnt = (int)m_palette.SendMessage(PLTM_GETSELCOUNT, 0, 0);
	if (m_xselcnt > 0)
		m_palette.SendMessage(PLTM_GETSELITEMS, m_xselcnt, (LPARAM)&m_xsel);

	SetViewImage();
	SetFileSize();
}

void CExportGifDlg::OnNotifyScaleChanged(NMHDR* phdr, LRESULT* pResult)
{
	*pResult = FALSE;

	ZOOM_SCALE* pzs = (ZOOM_SCALE*)phdr;
	TCHAR buf[MAX_PATH];
	format_scale_text(pzs->scale, buf);
	SetDlgItemText(IDC_POWER, buf);
}

/////////////////////////////////////////////////////////////////////
//

static TCHAR sszGifFilter[] = _T("GIF(*.gif)\0*.gif\0\0");
static TCHAR sszGifExt[] = _T(".gif");

BOOL export_gif_ui(HDIB hdib, LPRECT, FILTER_EXEC* pfe)
{
	CExportGifDlg dlg;
	dlg.m_hdib = hdib;
	if (dlg.DoModal(pfe->hparent) != IDOK) return FALSE;

	TCHAR name[MAX_PATH * 2];
	lstrcpy(name, pfe->name);
	PathRenameExtension(name, sszGifExt);

	BOOL bOK = FALSE;
	CFileDialog save(FALSE, sszGifExt, name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, sszGifFilter, pfe->hparent);
	TCHAR dir[MAX_PATH * 2];
	lstrcpy(dir, name);
	PathRemoveFileSpec(dir);
	save.m_ofn.lpstrInitialDir = dir;
	if (save.DoModal(pfe->hparent) == IDOK) {
		if (PathFileExists(save.m_szFileName)) {
			if (AppMessage(pfe->hparent, IDS_CAPTION_WRITEOUT, IDS_FILEISEXIST, NULL, MB_ICONQUESTION | MB_YESNO) != IDYES)
				return FALSE;
		}

		GIF_CPARAMS gc;
		memset(&gc, 0, sizeof(gc));
		gc.gif87a = FALSE;
		gc.interlaced = dlg.m_interlace;
		gc.xparent = (dlg.m_xselcnt > 0 ? TRUE : FALSE);
		gc.xparent_index = 0;
		bOK = imgGifSaveParamsSt(save.m_szFileName, dlg.m_hdibResult, &gc);
	}
	return bOK;
}

/////////////////////////////////////////////////////////////////////
//

CExportPngDlg::CExportPngDlg()
{
	m_idCaption = IDS_EXPORT_PNG;
}

HANDLE CExportPngDlg::MakePreviewImage()
{
	PNG_CPARAMS pc;
	memset(&pc, 0, sizeof(pc));
	pc.mask = PNGF_INTERLACE;

	int cnt = m_xselcnt;
	if (cnt > 0) {
		int* array = m_xsel;
		int maxindex = array[cnt - 1];
		pc.mask |= PNGF_XPARENTCOLOR;
		pc.num_trans = maxindex + 1;
		pc.alpha_trans_ptr = new BYTE[pc.num_trans];
		memset(pc.alpha_trans_ptr, 255, pc.num_trans);
		for (int i = 0; i < cnt; i++)
			pc.alpha_trans_ptr[array[i]] = 0;
	}
	pc.interlace = m_interlace;
	HANDLE hmem = imgPngSaveMemParams(m_hdibWorker, &pc, &m_savedsize);
	if (pc.alpha_trans_ptr)
		delete [] pc.alpha_trans_ptr;
	return hmem;
}


/////////////////////////////////////////////////////////////////////
//

static TCHAR sszPngFilter[] = _T("PNF(*.png)\0*.png\0\0");
static TCHAR sszPngExt[] = _T(".png");
static TCHAR sec[] = _T("WriteGIF");
static TCHAR keySaveToFolder[] = _T("SaveToFolder");

BOOL export_png_ui(HDIB hdib, LPRECT, FILTER_EXEC* pfe)
{
	CExportPngDlg dlg;
	dlg.m_hdib = hdib;
	if (dlg.DoModal(pfe->hparent) != IDOK || dlg.m_hdibWorker == NULL) return FALSE;

	BOOL bOK = FALSE;
	TCHAR name[MAX_PATH * 2];
	lstrcpy(name, pfe->name);
	PathRenameExtension(name, sszPngExt);

	regkey reg(pfe->hkey, sec);
	TCHAR dir[MAX_PATH * 2];
	if (reg.gettext(keySaveToFolder, dir, _COUNTOF(dir)) > 0 && PathIsDirectory(dir)) {
		// 前回保存したフォルダを取得
		PathAppend(dir, PathFindFileName(name));
		lstrcpy(name, dir);
	}
	lstrcpy(dir, name);
	PathRemoveFileSpec(dir);

	CFileDialog save(FALSE, sszPngExt, name, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, sszPngFilter, pfe->hparent);
	save.m_ofn.lpstrInitialDir = dir;
	if (save.DoModal(pfe->hparent) == IDOK) {
		if (PathFileExists(save.m_szFileName)) {
			if (AppMessage(pfe->hparent, IDS_CAPTION_WRITEOUT, IDS_FILEISEXIST, NULL, MB_ICONQUESTION | MB_YESNO) != IDYES)
				return FALSE;
		}

		PNG_CPARAMS pc;
		memset(&pc, 0, sizeof(pc));
		pc.mask = PNGF_INTERLACE;

		int cnt = dlg.m_xselcnt;
		if (cnt > 0) {
			int* array = dlg.m_xsel;
			int maxindex = array[cnt - 1];
			pc.mask |= PNGF_XPARENTCOLOR;
			pc.num_trans = maxindex + 1;
			pc.alpha_trans_ptr = new BYTE[pc.num_trans];
			memset(pc.alpha_trans_ptr, 255, pc.num_trans);
			for (int i = 0; i < cnt; i++)
				pc.alpha_trans_ptr[array[i]] = 0;
		}
		pc.interlace = dlg.m_interlace;
		bOK = imgPngSaveParams(save.m_szFileName, dlg.m_hdibWorker, &pc);
		if (pc.alpha_trans_ptr)
			delete [] pc.alpha_trans_ptr;

		lstrcpy(dir, save.m_szFileName);
		PathRemoveFileSpec(dir);
		reg.settext(keySaveToFolder, dir);
	}
	return bOK;
}
