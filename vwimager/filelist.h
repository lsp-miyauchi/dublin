////////////////////////////////////////////////////////////////////////
// filelist.h

#pragma once
#ifndef __VW_FILELIST_H__
#define __VW_FILELIST_H__

#include "..\lspexif\lspexifapi.h"
#include "mousewheeler.h"
#include "fileitem.h"
#include "threadhandle.h"
#include "dropsource.h"
#include "applydropshadow.h"
#include "gdip_func.h"

#define THUMBRECTSIZE		96
#define THUMBIMAGESIZE		72


HBITMAP LoadExifThumbnail(LPCTSTR pszName, int width, int height);

///////////////////////////////////////////////////////////////////////
//

class CThumbnailItem
{
public:
	CString m_strName;
	FILETIME m_timestamp;
	int m_imagetype;
	CBitmap m_bmp;
	bool m_error:1;
	CSize m_size;

	CThumbnailItem(LPCTSTR pszName = NULL, int imagetype = IMG_NOTSUPPORTED, LPFILETIME timestamp = NULL)
		: m_error(0)
	{
		m_strName = pszName;
		m_imagetype = imagetype;
		(DWORDLONG&)m_timestamp = 0;
		if (timestamp) m_timestamp = *timestamp;
	}

	bool Load()
	{
		if (m_bmp.IsNull()) {
			if (m_error) return false;

			m_bmp.Attach(LoadExifThumbnail(m_strName, THUMBIMAGESIZE, THUMBIMAGESIZE));
			if (m_bmp.IsNull()) {
				m_error = true;
				return false;
			}
			m_bmp.GetSize(m_size);
		}
		return true;
	}

	inline HBITMAP GetThumbnail() { return m_bmp.m_hBitmap; }
	inline LPCTSTR GetName() { return m_strName; }
	inline int CompareName(CThumbnailItem* dst) { return (dst != NULL) ? StrCmpLogicalW(m_strName, dst->m_strName) : 1; }
	inline int CompareTime(CThumbnailItem* dst) { return (dst != NULL) ? CompareFileTime(&m_timestamp, &dst->m_timestamp) : 1; }
	inline long width() { return m_size.cx; }
	inline long height() { return m_size.cy; }
};


///////////////////////////////////////////////////////////////////////
//

class CThumbnaiListView : public CMWListViewCtrl
{
private:
	CSimpleArray<CThumbnailItem*> m_array;
	int m_index;
	CString m_strTarget;
	CString m_strPath;
	CEvent m_event;
	CThreadHandle m_thread;
	UINT m_wm_listupdone;
	UINT m_wm_imageloaded;
	UINT m_wm_getdragimage;
	CImageList m_imDummy;
	CSize m_sizeImage;
	COLORREF m_clrBkgnd;
	CApplyDropShadow m_dropshadow;
	bool m_bDragEnable:1;
	bool m_bDragLoop:1;

	enum {
		m_cxyImage = THUMBRECTSIZE,
	};

public:
	CThumbnaiListView()
		: m_index(0)
		, m_wm_listupdone(::RegisterWindowMessage(_T("vwi_listupdone")))
		, m_wm_imageloaded(::RegisterWindowMessage(_T("vwi_imageloaded")))
		, m_wm_getdragimage(RegisterWindowMessage(DI_GETDRAGIMAGE))
		, m_sizeImage(CSize(0, 0))
		, m_clrBkgnd(RGB(105,105,105))
		, m_bDragEnable(true)
		, m_bDragLoop(false)
	{
	}

	void InitialUpdate()
	{
		// 必要ならイメージリストを作成
		SetTextColor(m_clrBkgnd);
		SetBkColor(m_clrBkgnd);
		SetBkColor(m_clrBkgnd);
		if (m_imDummy.IsNull()) {
			MakeDummyImageList();
			SetImageList(m_imDummy, LVSIL_NORMAL);
			SetIconSpacing(m_sizeImage.cx, m_sizeImage.cy);
		}
	}

	void MakeDummyImageList()
	{
		if (!m_imDummy.IsNull()) return;

		CClientDC dc(NULL);
		CDC dcw; dcw.CreateCompatibleDC(dc);
		HFONT hfont = dcw.SelectFont(GetFont());
		TEXTMETRIC tm;
		dcw.GetTextMetrics(&tm);
		dcw.SelectFont(hfont);

		// ------------------------------------------------------------
		// アイテム矩形の高さは イメージリストアイコンの高さ + フォントの高さ + 6 で計算すると大体丁度良い
		// ------------------------------------------------------------
		m_sizeImage.cx = m_cxyImage;
		m_sizeImage.cy = m_cxyImage - (tm.tmHeight + tm.tmExternalLeading + 6);
		HBITMAP hbm = CreateCompatibleBitmap(dc, m_sizeImage.cx, m_sizeImage.cy);
		if (hbm) {
			hbm = dcw.SelectBitmap(hbm);
			dcw.FillSolidRect(0, 0, m_sizeImage.cx, m_sizeImage.cy, RGB(255, 0, 255));
			hbm = dcw.SelectBitmap(hbm);

			if (m_imDummy.Create(m_sizeImage.cx, m_sizeImage.cy, ILC_COLORDDB | ILC_MASK, 1, 1))
				m_imDummy.Add(hbm, RGB(255, 0, 255));
			DeleteObject(hbm);
		}
	}

	void Clear()
	{
		// スレッド動作中ならスレッドを停止
		ThreadExit();
		// リストビューのアイテム削除
		DeleteAllItems();

		// 配列クリア
		int cnt = m_array.GetSize();
		for (int i = 0; i < cnt; i++) {
			CThumbnailItem* item = m_array[i];
			if (item) delete item;
		}
		m_array.RemoveAll();
		m_index = 0;
	}

	void ThreadExit()
	{
		if (m_thread.IsRunning()) {
			// シグナルON
			m_event.Set();
			// 中断しているかもしれないのでリジューム
			m_thread.Resume();
			// スレッド終了を待つ
			m_thread.WaitForExit(INFINITE);
		}
		m_event.Close();
		m_thread.Close();
	}

	static int compname(const void *arg1, const void *arg2)
	{
		CThumbnailItem* p1 = *(CThumbnailItem**)arg1;
		CThumbnailItem* p2 = *(CThumbnailItem**)arg2;
		return p1->CompareName(p2);
	}

	void EnumListup()
	{
		CString path = m_strPath;

		TCHAR buf[MAX_PATH * 2];
		PathCombine(buf, path, _T("*.*"));

		CThumbnailItem* item;
		CFileFinder ff;
		if (ff.BeginFind(buf)) {
			do {
				// スレッド終了？
				if (WaitForSingleObject(m_event, 0) != WAIT_TIMEOUT) return;

				if (!ff.IsFolder()) {
					int type = imgSpecifyImageType(ff.cFileName);
					if (type != IMG_NOTSUPPORTED) {
						PathCombine(buf, path, ff.cFileName);
						item = new CThumbnailItem(buf, type, &ff.ftLastWriteTime);
						m_array.Add(item);
					}
				}
			} while (ff.NextFind());
			ff.EndFind();
		}

		int cnt = m_array.GetSize();
		// エントリが無い場合は、ここでスレッドを終了
		if (cnt == 0) return;

		if (cnt > 1) {
			// ソートする
			qsort(m_array.GetData(),  cnt, sizeof(CThumbnailItem*), compname);
		}

		// ファイルのリストアップが完了したので通知
		int start = (int)::SendMessage(m_hWnd, m_wm_listupdone, (WPARAM)cnt, (LPARAM)&m_array);
		if (start < 0)
			start = 0;

		// まずイベントをチェック
		if (WaitForSingleObject(m_event, 0) != WAIT_TIMEOUT) return;

		// サムネイルを順次読む
		int i;
		for (i = start; i < cnt; i++) {
			// スレッド終了？
			if (WaitForSingleObject(m_event, 0) != WAIT_TIMEOUT) return;

			item = m_array[i];
			if (item->Load()) {
				// サムネイル取得できたので通知
				::PostMessage(m_hWnd, m_wm_imageloaded, 0, (LPARAM)i);
			}
		}

		for (i = 0; i < start; i++) {
			// スレッド終了？
			if (WaitForSingleObject(m_event, 0) != WAIT_TIMEOUT) return;

			item = m_array[i];
			if (item->Load()) {
				// サムネイル取得できたので通知
				::PostMessage(m_hWnd, m_wm_imageloaded, 0, (LPARAM)i);
			}
		}
	}

	static unsigned int __stdcall _ThreadProcEntry(void* param)
	{
		CThumbnaiListView* pView = (CThumbnaiListView*)param;
		if (pView) {
			::CoInitialize(NULL);
			pView->EnumListup();
			::CoUninitialize();
		}
		_endthreadex(0);
		return 0;
	}

	void Listup(LPCTSTR pszPathFileName)
	{
		// フォルダ内リストアップ済みの情報をクリア
		Clear();

		if (PathIsDirectory(pszPathFileName)) {
			m_strPath = pszPathFileName;
			m_strTarget.Empty();
		}
		else if (PathFileExists(pszPathFileName)) {
			m_strTarget = pszPathFileName;
			m_strPath = pszPathFileName;
			PathRemoveFileSpec((LPTSTR)(LPCTSTR)m_strPath);
		}
		else
			return;

		// イベント作成、手動リセット、初期状態＝非シグナル
		m_event.Create(NULL, TRUE, FALSE, NULL);
		// リストアップスレッド起動
		m_thread.Begin(_ThreadProcEntry, this, CREATE_SUSPENDED);
		m_thread.SetPriority(THREAD_PRIORITY_BELOW_NORMAL);
		m_thread.Resume();
	}

	bool GetItemFileName(int index, LPTSTR psz, int cch)
	{
		if (!psz || cch < 1 || index < 0 || index >= GetItemCount()) return false;

		CThumbnailItem* item = (CThumbnailItem*)GetItemData(index);
		if (item) {
			lstrcpyn(psz, item->m_strName, cch);
			return true;
		}
		return false;
	}

	int SelectFileItem(LPCTSTR pszFileName)
	{
		if (pszFileName && *pszFileName) {
			int cnt = GetItemCount();
			for (int i = 0; i < cnt; i++) {
				CThumbnailItem* item = (CThumbnailItem*)GetItemData(i);
				if (item && !StrCmpI(pszFileName, item->GetName())) {
					SelectItem(i);
					EnsureVisible(i, FALSE);
					return i;
				}
			}
		}
		return -1;
	}

	BOOL DeleteFileItem(int index)
	{
		if (!DeleteItem(index)) return FALSE;

		CThumbnailItem* item = m_array[index];
		m_array.RemoveAt(index);
		if (item) delete item;
		return TRUE;
	}

BEGIN_MSG_MAP(CThumbnaiListView)
	CHAIN_MSG_MAP(CMWListViewCtrl)
	ON_WM_DESTROY()
	ON_REGISTERD_MESSAGE(m_wm_listupdone, OnHandlerListupDone)
	ON_REGISTERD_MESSAGE(m_wm_imageloaded, OnHandlerImageLoaded)
	ON_REGISTERD_MESSAGE(m_wm_getdragimage, OnHandlerGetDragImage)
END_MSG_MAP()

	void OnDestroy()
	{
		Clear();
	}

	LRESULT OnHandlerListupDone(WPARAM wParam, LPARAM lParam)
	{
		// リストに追加
		int cnt = (int)wParam;
		CSimpleArray<CThumbnailItem*>* parr = (CSimpleArray<CThumbnailItem*>*)lParam;

		SetRedraw(FALSE);
		m_index = 0;
		LVITEM lvi;
		for (int i = 0; i < cnt; i++) {
			CThumbnailItem* item = (*parr)[i];
			if (item && !m_strTarget.CompareNoCase(item->GetName()))
				m_index = i;
			lvi.mask = LVIF_IMAGE | LVIF_PARAM;
			lvi.iItem = i;
			lvi.iSubItem = 0;
			lvi.iImage = 0;
			lvi.lParam = (LPARAM)item;
			InsertItem(&lvi);
		}
		SelectItem(m_index);
		SetRedraw(TRUE);
		EnsureVisible(m_index, FALSE);
		// 読み出しを開始するインデクスを返す
		return (LRESULT)(max(0, m_index - 16));
	}

	LRESULT OnHandlerImageLoaded(WPARAM wParam, LPARAM lParam)
	{
		// 読み込みが終わった項目を再描画
		int index = (int)lParam;
		RedrawItems(index, index);
		return 0;
	}

	LRESULT OnHandlerGetDragImage(WPARAM wParam, LPARAM lParam)
	{
		SHDRAGIMAGE* image = (SHDRAGIMAGE*)lParam;
		if (!image) FALSE;

		memset(image, 0, sizeof(SHDRAGIMAGE));
		int index = GetSelectedIndex();
		if (index >= 0) {
			CThumbnailItem* item = (CThumbnailItem*)GetItemData(index);
			if (item) {
				image->crColorKey = OsVISTA() ? CLR_NONE : GetSysColor(COLOR_WINDOW);
				image->sizeDragImage.cx = item->width();
				image->sizeDragImage.cy = item->height();
				if (OsVISTA()) {
					image->sizeDragImage.cx += m_dropshadow.delta();
					image->sizeDragImage.cy += m_dropshadow.delta();
				}
				image->ptOffset.x = image->sizeDragImage.cx - (item->width() / 2);
				image->ptOffset.y = image->sizeDragImage.cy * 3 / 4;

				CClientDC dc(NULL);
				HBITMAP hbm = CreateCompatibleBitmap(dc, image->sizeDragImage.cx, image->sizeDragImage.cy);
				if (hbm) {
					CDC dcw; dcw.CreateCompatibleDC(dc);
					hbm = dcw.SelectBitmap(hbm);
					CRect rc(0, 0, image->sizeDragImage.cx, image->sizeDragImage.cy);

					HBITMAP hbmImage = item->GetThumbnail();
					if (!hbmImage)
						dcw.FillSolidRect(&rc, GetSysColor(COLOR_3DFACE));
					else {
						CDC dcm; dcm.CreateCompatibleDC(dc);
						hbmImage = dcm.SelectBitmap(hbmImage);
						dcw.BitBlt(0, 0, item->width(), item->height(), dcm, 0, 0, SRCCOPY);
						dcm.SelectBitmap(hbmImage);
					}

					if (OsVISTA()) {
						// ドロップシャドウ処理
						m_dropshadow.Apply(dcw, &rc);
						rc.right -= m_dropshadow.delta();
						rc.bottom -= m_dropshadow.delta();
					}

					image->hbmpDragImage = dcw.SelectBitmap(hbm);
				}
			}
		}
		return (image->hbmpDragImage != NULL);
	}

	void ProcessCustomDraw(NMLVCUSTOMDRAW* pcd, LRESULT* pResult)
	{
		switch (pcd->nmcd.dwDrawStage) {
		case CDDS_PREPAINT: *pResult = CDRF_NOTIFYITEMDRAW; break;
		case CDDS_ITEMPREPAINT:
		{
			CDCHandle dc = pcd->nmcd.hdc;
			CRect rc;
			GetItemRect((int)pcd->nmcd.dwItemSpec, &rc, LVIR_BOUNDS);

			// 不可視の場合の処理を割愛すると超高速！
			if (!dc.RectVisible(&rc))
				break;

			bool bSelected = ((GetItemState((int)pcd->nmcd.dwItemSpec, LVIS_SELECTED) & LVIS_SELECTED) != 0);
			CThumbnailItem* item = (CThumbnailItem*)pcd->nmcd.lItemlParam;
			if (item) {
				int cx = THUMBIMAGESIZE;
				int cy = THUMBIMAGESIZE;
				HBITMAP hbm = item->GetThumbnail();
				if (hbm) {
					cx = item->width();
					cy = item->height();
				}
				int shadow = m_dropshadow.delta();

				CRect rcw(0, 0, cx + shadow, cy + shadow);
				rcw.OffsetRect(rc.left + (rc.Width() - (cx + shadow)) / 2, rc.top + (rc.Height() - (cy + shadow)) / 2);

				CRect rcs = rcw;
				rcs.left -= (shadow - 1);
				rcs.top -= (shadow - 1);
				if (bSelected) {
					Graphics g(dc);
					g.SetSmoothingMode(SmoothingModeHighQuality);
	 				SolidBrush brush(Color(255, 255, 255, 255));
					RectF r((Gdiplus::REAL)rcs.left, (Gdiplus::REAL)rcs.top, (Gdiplus::REAL)rcs.Width() - 1, (Gdiplus::REAL)rcs.Height() - 1);
					GdipRoundRect(g, NULL, &brush, r, CSize(6, 6));
				}
				else
					dc.FillSolidRect(&rcs, m_clrBkgnd);

				if (hbm) {
					CDC dcw = CreateCompatibleDC(dc);
					hbm = dcw.SelectBitmap(hbm);
					dc.BitBlt(rcw.left, rcw.top, cx, cy, dcw, 0, 0, SRCCOPY);
					dcw.SelectBitmap(hbm);
				}
				else {
					rcs = rcw;
					rcs.right -= shadow;
					rcs.bottom -= shadow;
					dc.FillSolidRect(&rcs, GetSysColor(COLOR_3DSHADOW));
				}
				// ドロップシャドウ適用
				m_dropshadow.Apply(dc, &rcw);
			}
			*pResult = CDRF_SKIPDEFAULT;
			break;
		}
		case CDDS_ITEMPOSTPAINT: *pResult = CDRF_SKIPDEFAULT; break;
		default: *pResult = CDRF_DODEFAULT; break;
		}
	}

	DWORD DragStart(int index)
	{
		if (!m_bDragEnable) return DROPEFFECT_NONE;

		m_bDragLoop = true;
		DWORD eff = DROPEFFECT_NONE;
		CThumbnailItem* item = (CThumbnailItem*)GetItemData(index);
		if (item) {
			CPoint pt;
			GetCursorPos(&pt);
			if (lccBeginTrack(m_hWnd, pt)) {
				PIDLIST_ABSOLUTE pidl = ILCreateFromPath(item->GetName());
				if (pidl) {
					IShellFolder* psf;
					PCUITEMID_CHILD pidlLast;
					if (SHBindToParent(pidl, IID_IShellFolder, (void**)&psf, &pidlLast) == S_OK) {
						IDataObject* pdo;
						if (NOERROR == psf->GetUIObjectOf(m_hWnd, 1, &pidlLast, IID_IDataObject, NULL, (void**)&pdo)) {
							CDropSource* pds = new CDropSource(m_hWnd, pdo);
							if (pds) {
								DWORD eff = DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK;
								::DoDragDrop(pdo, pds, eff, &eff);
								pds->Release();
							}
						}
						psf->Release();
					}
					CoTaskMemFree(pidl);
				}
			}
		}
		m_bDragLoop = false;
		return eff;
	}

	inline bool IsDragging() { return m_bDragLoop; }
	inline bool EnableDragDrop(bool bEnable) { m_bDragEnable = bEnable; }
};


#endif			// __VW_FILELIST_H__
