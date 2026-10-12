/////////////////////////////////////////////////////////////////////////////
// exifinfo.cpp

#include "stdafx.h"
#include <math.h>
#include "exifinfo.h"
#include "common.h"
#include "colordefs.h"
#include "filemap.h"
#include "dib.h"
#include "resource.h"


////////////////////////////////////////////////////////////////////////
//

/*
static HBITMAP load_core_gdip(Bitmap& bitmap, int width, int height)
{
	HBITMAP hbm = NULL;
	int cx = bitmap.GetWidth();
	int cy = bitmap.GetHeight();
	if (cx > 0 && cy > 0) {
		CalcInnerImageSize(width, height, &cx, &cy);
		Bitmap* thumb = static_cast<Bitmap*>(bitmap.GetThumbnailImage(cx, cy, NULL, NULL));
		if (thumb) {
			// HBITMAP抽出
			Color color(255, 255, 255, 255);
			if (thumb->GetHBITMAP(color, &hbm) != Ok)
				hbm = NULL;
			delete thumb;
		}
	}
	return hbm;
}
*/
static HBITMAP load_core_gdip(Bitmap& bitmap, int width, int height)
{
	HBITMAP hbm = NULL;
	int cx = bitmap.GetWidth();
	int cy = bitmap.GetHeight();
	PixelFormat fmt = bitmap.GetPixelFormat();
	if (cx > 0 && cy > 0) {
		CalcInnerImageSize(width, height, &cx, &cy);
		Bitmap base(cx, cy, PixelFormat24bppRGB);
		Graphics g(&base);
		// 下地を塗る
		SolidBrush sbr((fmt & PixelFormatAlpha) ? Color(255, 0xee, 0xf2, 0xfb) : Color(255, 255, 255, 255));
		g.FillRectangle(&sbr, 0, 0, cx, cy);
		// イメージ
		g.DrawImage(&bitmap, 0, 0, cx, cy);
		// HBITMAP抽出
		Color color(255, 255, 255, 255);
		if (base.GetHBITMAP(color, &hbm) != Ok)
			hbm = NULL;
	}
	return hbm;
}

HBITMAP LoadExifThumbnail(LPCTSTR pszName, int width, int height)
{
	HBITMAP hbm = NULL;
	int type = imgSpecifyImageType(pszName);
	if (imgTypeIsExif(type)) {
		CComPtr<IExifParser> pep;
		HRESULT hr = pep.CoCreateInstance(__uuidof(ExifParser));
		if (SUCCEEDED(hr)) {
			IStream* pstrm;
			if (CreateStreamOnHGlobal(NULL, TRUE, &pstrm) == S_OK) {
				EXIFINFOSTRUCT info;
				memset(&info, 0, sizeof(info));
				if (pep->GetExifInfo(pszName, pstrm, NULL, false, (BYTE*)&info) == S_OK) {
					Bitmap bitmap(pstrm);
					switch (info.Orientation) {
					case ORIENTATION_FLIPX:					// 左右反転
						bitmap.RotateFlip(RotateNoneFlipX);
						break;
					case ORIENTATION_ROTATE180:				// 180°回転
						bitmap.RotateFlip(Rotate180FlipNone);
						break;
					case ORIENTATION_FLIPY:					// 上下反転
						bitmap.RotateFlip(RotateNoneFlipY);
						break;
					case ORIENTATION_ROTATELEFTFLIPY:		// 左へ90°回転し上下反転
						bitmap.RotateFlip(Rotate270FlipY);
						break;
					case ORIENTATION_ROTATERIGHT:			// 右へ90°回転する
						bitmap.RotateFlip(Rotate90FlipNone);
						break;
					case ORIENTATION_ROTATERIGHTFLIPY:		// 右へ90°回転し上下反転
						bitmap.RotateFlip(Rotate90FlipY);
						break;
					case ORIENTATION_ROTATELEFT:			// 左へ90°回転する
						bitmap.RotateFlip(Rotate270FlipNone);
						break;
					}
					hbm = load_core_gdip(bitmap, width, height);
				}
				pstrm->Release();
			}
		}
	}
	else if (type == IMG_WEBP) {
		HDIB hdib = imgWebpLoadScaled(pszName, width, height);
		if (hdib) {
			imgCreateBitmapFromDIB(hdib, &hbm, NULL);
			GlobalFree(hdib);
			return hbm;
		}
	}
	else if (type == IMG_AVIF) {
		HDIB hdib = imgAvifLoadScaled(pszName, width, height);
		if (hdib) {
			imgCreateBitmapFromDIB(hdib, &hbm, NULL);
			GlobalFree(hdib);
			return hbm;
		}
	}

	if (!hbm) {
		Bitmap bitmap(pszName);
		hbm = load_core_gdip(bitmap, width, height);
	}
	return hbm;
}

////////////////////////////////////////////////////////////////////////
//

static COLORREF calc_mixed_color(COLORREF clrA, COLORREF clrB, int perA)
{
	// (9a + 1b)/10
	int r = (perA * GetRValue(clrA) + (100 - perA) * GetRValue(clrB)) / 100;
	int g = (perA * GetGValue(clrA) + (100 - perA) * GetGValue(clrB)) / 100;
	int b = (perA * GetBValue(clrA) + (100 - perA) * GetBValue(clrB)) / 100;
	return RGB(r, g, b);
}

void CExifView::Initialize()
{
	EnableGroupView(TRUE);
	SetExtendedListViewStyle(GetExtendedListViewStyle() | LVS_EX_DOUBLEBUFFER);

	// フォント
	LOGFONT lf;
	HFONT hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	if (hf)
		GetObject(hf, sizeof(lf), &lf);
	else
		SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lf, FALSE);
	m_font.CreateFontIndirect(&lf);
	SetFont(m_font);

	// カラム設定
	InsertColumn(0, _T(""), LVCFMT_LEFT, 80, 0);
	InsertColumn(0, _T(""), LVCFMT_LEFT, 80, 1);

	// Exif情報読み込みスレッド初期化
	m_exifparser.Initialize(m_hWnd, m_message);
}

LRESULT CExifView::OnGetExifInfo(WPARAM wParam, LPARAM lParam)
{
	m_strEmptyMessage.Empty();
	SetRedraw(FALSE);
	DeleteAllItems();
	if ((DWORD)wParam == m_magic) {
		int numGroups = m_exifparser.m_info.GetSize();
		for (int i = 0; i < numGroups; i++) {
			LV_ITEM item;
			CExifGroup& eg = m_exifparser.m_info[i];
			int numItems = eg.GetSize();
			if (numItems > 0) {
				LVGROUP group;
				memset(&group, 0, sizeof(group));
#ifdef _WIN64
				group.cbSize = sizeof(group);
#else
				group.cbSize = OsVISTA() ? sizeof(group) : LVGROUP_V5_SIZE;
#endif
				group.mask = LVGF_GROUPID | LVGF_HEADER | LVGF_STATE | LVGF_ALIGN;
				group.stateMask = LVGS_NORMAL;
				group.state = LVGS_NORMAL;
				if (OsVISTA()) {
					group.stateMask |= VISTA_LVGS_COLLAPSIBLE;
					group.state |= VISTA_LVGS_COLLAPSIBLE;
				}
				group.uAlign = LVGA_HEADER_LEFT | LVGA_FOOTER_LEFT;

				group.iGroupId = i;
				group.pszHeader = (LPTSTR)(LPCTSTR)eg.m_strTitle;
				group.cchHeader = lstrlen(group.pszHeader);
				InsertGroup(-1, (LVGROUP*)&group);

				for (int j = 0; j < numItems; j++) {
					CString strTitle = eg.GetKeyAt(j);
					CString strText = eg.GetValueAt(j);
					memset(&item, 0, sizeof(item));
					item.mask = LVIF_TEXT | LVIF_PARAM;
					item.mask |= LVIF_INDENT | LVIF_GROUPID;
					item.iIndent = 1;
					item.iItem = GetItemCount();
					item.iGroupId = i;
					item.pszText = (LPTSTR)(LPCTSTR)strTitle;
					int index = InsertItem(&item);
					SetItemText(index, 1, strText);
				}
			}
		}
		SetColumnWidth(0, LVSCW_AUTOSIZE);
		SetColumnWidth(1, LVSCW_AUTOSIZE);
	}

	if (GetItemCount() < 1) {
		m_strEmptyMessage = _T("表示する情報はありません。");
	}

	SetRedraw(TRUE);
	Invalidate();
	return 0;
}

void CExifView::ParseExif(LPCTSTR pszImageFile)
{
	if (IsWindow()) {
		if (pszImageFile && PathFileExists(pszImageFile)) {
			m_magic = GetTickCount();
			m_exifparser.Start(m_magic, (LPARAM)pszImageFile);
		}
	}
}

void CExifView::ProcessCustomDraw(NMLVCUSTOMDRAW* pcd, LRESULT* pResult)
{
	switch (pcd->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
		if (pcd->nmcd.dwItemSpec & 1)
			*pResult = CDRF_DODEFAULT;
		else {
			pcd->clrTextBk = calc_mixed_color(GetSysColor(COLOR_WINDOW), LCOLOR_FACE, 80);
			*pResult = CDRF_NEWFONT;
		}
		break;
	default:
		*pResult = CDRF_DODEFAULT;
		break;
	}
}
