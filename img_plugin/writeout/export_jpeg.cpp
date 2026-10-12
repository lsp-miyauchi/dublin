// export_jp2k.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "common.h"
#include "export_jpeg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExportJpegDlg ダイアログ

CExportJpegDlg::CExportJpegDlg()
{
	memset(&m_jc, 0, sizeof(m_jc));
	m_hdib = NULL;
}

BOOL CExportJpegDlg::OnInitDialog(LPARAM lParam)
{
	if (!m_hdib) {
		EndDialog(IDCANCEL);
		return TRUE;
	}

	ExecuteDlgInit(IDD);
	CenterWindow();
	m_bps = GetDlgItem(IDC_BPS);
	m_qtracker = GetDlgItem(IDC_SLIDER);
	m_stracker = GetDlgItem(IDC_SMOOTH);
	m_view = GetDlgItem(IDC_PREVIEW);

	m_bps.SetCurSel(3);				// ISDN 64K

	m_qtracker.SetRange(0, 100, TRUE);
	m_qtracker.SetSelection(50, 95);
	m_qtracker.SetPageSize(5);
	m_qtracker.SetPos(m_jc.quality);
	m_qtracker.SetTicFreq(5);

	m_stracker.SetRange(0, 100, TRUE);
	m_stracker.SetSelection(10, 50);
	m_stracker.SetPageSize(5);
	m_stracker.SetPos(m_jc.smooth);
	m_stracker.SetTicFreq(5);

	if (m_jc.progressive)	CheckDlgButton(IDC_JPGPROGRESSIVE, TRUE);
	else if (m_jc.baseline)	CheckDlgButton(IDC_JPGBASELINE, TRUE);
	else					CheckDlgButton(IDC_JPGSTANDARD, TRUE);

	UpdatePreview();

	return TRUE;
}

#define DL_DELAY(a)		((a) * 75 / 100)

void CExportJpegDlg::OnBpsChanged()
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

void CExportJpegDlg::UpdatePreview()
{
	Signal_SetBusy(GetDlgItem(IDC_SIGNAL), TRUE);

	TCHAR buf[MAX_PATH];
	buf[0] = '\0';
	HGLOBAL hmem = imgJpgSaveMemParams(m_hdib, &m_jc, &m_savedsize);
	if (hmem) {
		HDIB h = imgJpgLoadMem(hmem, 0, 1);
		GlobalFree(hmem);

		h = m_view.UpdateImage(h, TRUE);
		if (h) GlobalFree(h);
		else   m_view.CenterPosition();
		SIZE size;
		m_view.GetImageSize(&size);
		TCHAR w[MAX_PATH];
		StrFormatByteSize(m_savedsize, w, _COUNTOF(w));
		wsprintf(buf, _T("%d x %d  %s"), size.cx, size.cy, w);
	}
	else {
		hmem = m_view.SetImage(NULL, TRUE);
		if (hmem) GlobalFree(hmem);
	}
	SetDlgItemText(IDC_INFO, buf);
	OnBpsChanged();

	Signal_SetBusy(GetDlgItem(IDC_SIGNAL), FALSE);
}

void CExportJpegDlg::OnHScroll(UINT nSBCode, UINT nPos, HWND hScrollBar)
{
	CTrackBarCtrl tracker = hScrollBar;
	int val;
	switch (nSBCode) {
		case TB_THUMBTRACK:
			val = (int)nPos;
			break;

		case TB_TOP:
		case TB_BOTTOM:
		case TB_LINEUP:
		case TB_LINEDOWN:
		case TB_PAGEUP:
		case TB_PAGEDOWN:
			val = tracker.GetPos();
			break;

		case TB_ENDTRACK:
		case TB_THUMBPOSITION:
			UpdatePreview();
			return;

		default:
			return;
	}

	if (hScrollBar == m_qtracker)
		m_jc.quality = val;
	else if (hScrollBar == m_stracker)
		m_jc.smooth = val;
}

void CExportJpegDlg::OnFormat()
{
	m_jc.progressive = 0;
	m_jc.baseline = 0;
	if (IsDlgButtonChecked(IDC_JPGBASELINE))
		m_jc.baseline = TRUE;
	else if (IsDlgButtonChecked(IDC_JPGPROGRESSIVE))
		m_jc.progressive = TRUE;

	UpdatePreview();
}

void CExportJpegDlg::OnZoomIn()
{
	m_view.ZoomIn();
}

void CExportJpegDlg::OnZoomOut()
{
	m_view.ZoomOut();
}

void CExportJpegDlg::OnNotifyPreview(NMHDR* phdr, LRESULT* pResult)
{
	ZOOM_SCALE* pzs = (ZOOM_SCALE*)phdr;

	TCHAR buf[MAX_PATH];
	int iScale = (int)(pzs->scale * 1000);
	int s = iScale / 10;
	int m = iScale % 10;
	if (!m)
		wsprintf(buf, _T("%d%%"), s);
	else
		wsprintf(buf, _T("%d.%d%%"), s, m);
	SetDlgItemText(IDC_POWER, buf);
}

/////////////////////////////////////////////////////////////////////
//

static TCHAR sec[] = _T("WriteJpeg");
static TCHAR keyParamSet[] = _T("JpgParam");
static TCHAR keySaveToFolder[] = _T("SaveToFolder");
static TCHAR sszFilter[] = _T("Jpeg(*.jpg)\0*.jpg\0\0");
static TCHAR sszExt[] = _T(".jpg");

BOOL export_jpeg_ui(HDIB hdib, LPRECT, FILTER_EXEC* pfe)
{
	CExportJpegDlg dlg;
	dlg.m_hdib = hdib;

	regkey reg(pfe->hkey, sec);
	if (reg.getbinary(keyParamSet, &dlg.m_jc, sizeof(JPEG_CPARAMS)) != sizeof(JPEG_CPARAMS)) {
		dlg.m_jc.quality = 93;
		dlg.m_jc.smooth = 0;
	}
	dlg.m_jc.dct = JC_ISLOW;

	TCHAR name[MAX_PATH * 2];
	lstrcpy(name, pfe->name);
	PathRenameExtension(name, sszExt);

	if (dlg.DoModal(pfe->hparent) == IDOK) {
		TCHAR dir[MAX_PATH * 2];
		if (reg.gettext(keySaveToFolder, dir, _COUNTOF(dir)) > 0 && PathIsDirectory(dir)) {
			// 前回保存したフォルダを取得
			PathAppend(dir, PathFindFileName(name));
			lstrcpy(name, dir);
		}
		lstrcpy(dir, name);
		PathRemoveFileSpec(dir);

		CFileDialog save(FALSE, sszExt, name, OFN_HIDEREADONLY, sszFilter, pfe->hparent);
		save.m_ofn.lpstrInitialDir = dir;
		if (save.DoModal(pfe->hparent) == IDOK) {
			if (PathFileExists(save.m_szFileName)) {
				if (AppMessage(pfe->hparent, IDS_CAPTION_WRITEOUT, IDS_FILEISEXIST, NULL, MB_ICONQUESTION | MB_YESNO) != IDYES)
					return FALSE;
			}

			imgJpgSaveParams(save.m_szFileName, hdib, &dlg.m_jc);
			reg.setbinary(keyParamSet, &dlg.m_jc, sizeof(JPEG_CPARAMS));

			lstrcpy(dir, save.m_szFileName);
			PathRemoveFileSpec(dir);
			reg.settext(keySaveToFolder, dir);
			return TRUE;
		}
	}
	return FALSE;
}
