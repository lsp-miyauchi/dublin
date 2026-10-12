//////////////////////////////////////////////////////////////////////
// dropper.h

#pragma once
#ifndef __DROPPER_H__
#define __DROPPER_H__


#ifndef DROPEFFECT_OPEN
#define DROPEFFECT_OPEN			5
#endif


template <class T>
class TDropperImpl : public TDropTargetHandler
{
private:
	bool m_bDppDragEnter:1;
	bool m_bDppDragMyself:1;
	bool m_bDppWaitingForDestroyWindow:1;
	DWORD m_uDppScrollTimer;
	DWORD m_uDppScrollTimerLimit;
	DWORD m_uDppDropFormats;
	CComPtr<IDropTarget> m_pDppDropTarget;
	CComPtr<IDropTargetHelper> m_pdtHelper;
	CComPtr<IDataObject> m_pdoCurrent;
	CComPtr<TDropSource> m_pDppDropSource;

public:
	TDropperImpl()
		: m_bDppDragEnter(false)
		, m_bDppDragMyself(false)
		, m_bDppWaitingForDestroyWindow(false)
		, m_uDppDropFormats(0)
		, m_uDppScrollTimer(0)
		, m_uDppScrollTimerLimit(0)
	{
	}

	// --------------------------------------------
	// TDropTargetHandler - OnTargetDragEnter
	// --------------------------------------------
	DWORD OnTargetDragEnter(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
	{
		// コピーして参照カウントをアップ
		m_pdoCurrent = pdo;
		// 利用可能なフォーマットを取得
		m_uDppDropFormats = GetDataObjectFormats(pdo);

		// 自分自身のD&D
		if (m_uDppDropFormats & SDO_INFILEVISORDRAGLOOP) {
			// マーカー取得
			bool bReworked = false;
			INFILEVISORDRAGLOOP marker = { 0 };
			FORMATETC fe = { (CLIPFORMAT)nRCF_INFILEVISORDRAGLOOP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			STGMEDIUM stm;
			if (pdo->GetData(&fe, &stm) == S_OK) {
				INFILEVISORDRAGLOOP* pMarker = (INFILEVISORDRAGLOOP*)GlobalLock(stm.hGlobal);
				if (pMarker && !pMarker->flag && pMarker->itemcount > 1) {
					// ドラッグイメージを改造する
					ReworkedDragImage();
					memcpy(&marker, pMarker, sizeof(marker));
					bReworked = true;
				}
				ReleaseStgMedium(&stm);
			}
		}

		T* pT = static_cast<T*>(this);
		if (S_OK == CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pdtHelper)))
			m_pdtHelper->DragEnter(pT->m_hWnd, pdo, &point, dropEffect);

		ResetDragScrollTimer();
		m_bDppDragEnter = true;

		return pT->OnDragEnter(pdo, keyState, dropEffect, point);
	}

	// --------------------------------------------
	// TDropTargetHandler - OnTargetDragOver
	// --------------------------------------------
	DWORD OnTargetDragOver(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
	{
		T* pT = static_cast<T*>(this);
		if (m_pdtHelper.p != NULL)
			m_pdtHelper->DragOver(&point, dropEffect);
		return pT->OnDragOver(pdo, keyState, dropEffect, point);
	}

	// --------------------------------------------
	// TDropTargetHandler - OnTargetDragLeave
	// --------------------------------------------
	void OnTargetDragLeave()
	{
		if (m_pdtHelper.p != NULL)
			m_pdtHelper->DragLeave();

		T* pT = static_cast<T*>(this);
		pT->OnDragLeave();
		m_bDppDragEnter = false;
		pT->OnDropExit();
	}

	// --------------------------------------------
	// TDropTargetHandler - OnTargetDrop
	// --------------------------------------------
	DWORD OnTargetDrop(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
	{
		T* pT = static_cast<T*>(this);
		if (m_pdtHelper.p != NULL)
			m_pdtHelper->Drop(pdo, &point, dropEffect);

		DWORD eff = pT->OnDrop(pdo, keyState, dropEffect, point);
		m_bDppDragEnter = false;
		pT->OnDropExit();

		if (m_bDppWaitingForDestroyWindow) {
			m_bDppWaitingForDestroyWindow = false;
			if (pT->IsWindow())
				pT->PostMessage(WM_CLOSE);
		}
		return eff;
	}

BEGIN_MSG_MAP(TDropperImpl)
	MESSAGE_HANDLER(WM_CREATE, OnDppCreate)
	MESSAGE_HANDLER(WM_DESTROY, OnDppDestroy)
	ON_REGISTERD_MESSAGE(nRM_GETDRAGIMAGE, OnDppGetDragImage)
END_MSG_MAP()

	LRESULT OnDppCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		BeginDropTarget();
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnDppDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		EndDropTarget();
		bHandled = FALSE;

		return 0;
	}

	LRESULT OnDppGetDragImage(WPARAM, LPARAM lParam)
	{
		T* pT = static_cast<T*>(this);
		return (LRESULT)pT->OnGetDragImage((SHDRAGIMAGE*)lParam);
	}

	// overridable
	virtual BOOL OnGetDragImage(SHDRAGIMAGE* psdi)
	{
		return FALSE;
	}

	// overridable
	virtual DWORD OnDragEnter(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
	{
		T* pT = static_cast<T*>(this);
		return pT->OnDragOver(pdo, keyState, dropEffect, point);
	}

	// overridable
	virtual DWORD OnDragOver(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
	{
		return DROPEFFECT_NONE;
	}

	// overridable
	virtual void OnDragLeave()
	{
	}

	// overridable
	virtual DWORD OnDrop(IDataObject* pdo, DWORD keyState, DWORD dropEffect, POINT point)
	{
		return DROPEFFECT_NONE;
	}

	// overridable
	void OnDropExit()
	{
		m_pdtHelper.Release();

		if (m_pdoCurrent.p != NULL) {
			ClearDropTargetDescription(m_pdoCurrent);
			// 参照カウント減
			m_pdoCurrent.Release();
		}
	}

	inline bool isDragging() { return m_bDppDragEnter; }
	inline bool isDragMyself() { return m_bDppDragMyself; }
	inline DWORD GetSupportedDropFormats() { return m_uDppDropFormats; }
	inline DWORD IsDropFormatsSupported(DWORD mask) { return (m_uDppDropFormats & mask); }
	inline void SetDropWaitingForDestroyWindow(bool bSet) { m_bDppWaitingForDestroyWindow = bSet; }

	void SetDragScrollTimer(DWORD timer)
	{
		m_uDppScrollTimer = timer;
		ResetDragScrollTimer();
	}

	inline void CancelDragScrollTimer() { SetDragScrollTimer(0); }

	void ResetDragScrollTimer()
	{
		m_uDppScrollTimerLimit = (m_uDppScrollTimer != 0) ? GetTickCount() + m_uDppScrollTimer : 0;
	}

	bool IsTimeToDragScroll()
	{
		bool b = (m_uDppScrollTimer && (GetTickCount() > m_uDppScrollTimerLimit)) ? true : false;
		if (b) ResetDragScrollTimer();
		return b;
	}

	void BeginDropTarget()
	{
		T* pT = static_cast<T*>(this);
		IDropTarget* pdt;
		TDropTarget::CreateInstance(&pdt, this);
		if (CoLockObjectExternal(pdt, TRUE, FALSE) == S_OK) {
			if (RegisterDragDrop(pT->m_hWnd, pdt) != S_OK) {
				CoLockObjectExternal(pdt, FALSE, FALSE);
				m_pDppDropTarget = pdt;
				pdt->Release();
			}
		}
	}

	void EndDropTarget()
	{
		T* pT = static_cast<T*>(this);
		if (m_pDppDropTarget.p != NULL) {
			RevokeDragDrop(pT->m_hWnd);
			CoLockObjectExternal(m_pDppDropTarget, FALSE, TRUE);
			m_pDppDropTarget.Release();
		}
	}

	BOOL ShowDropHelper(BOOL fShow)
	{
		if (m_pdtHelper.p != NULL)
			m_pdtHelper->Show(fShow);
		return fShow;
	}

	HRESULT DragStart(HWND hwndDragImage, IDataObject* pdo, int cnt, LPDWORD peff)
	{
		HRESULT hr = E_INVALIDARG;
		if (pdo) {
			m_bDppDragMyself = true;

			// 自分自身であることのマーカー
			FORMATETC fe = { (CLIPFORMAT)nRCF_INFILEVISORDRAGLOOP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
			STGMEDIUM stm = {0};
			stm.tymed = TYMED_HGLOBAL;
			stm.hGlobal = ::GlobalAlloc(GHND, sizeof(INFILEVISORDRAGLOOP));
			if (stm.hGlobal) {
				INFILEVISORDRAGLOOP* marker = (INFILEVISORDRAGLOOP*)::GlobalLock(stm.hGlobal);
				ATLASSERT(marker != NULL);
				marker->magic = 1;
				marker->itemcount = cnt;
				marker->flag = 0;						// ドラッグイメージの改造は未だ
				::GlobalUnlock(stm.hGlobal);
				pdo->SetData(&fe, &stm, TRUE);
			}

			IDropSource* pds;
			TDropSource::CreateInstance(&pds, hwndDragImage, pdo, cnt);

			// フレームウィンドウへのD&Dを無効にする
			CWindow wnd = ::GetActiveWindow();
			CWindow top = wnd.GetTopLevelParent();

			top.ModifyStyleEx(WS_EX_ACCEPTFILES, 0);

			// SHDoDragDrop（XP sp2以降）を使って IDropSource にNULLを指定すると、
			// Windows側で勝手にインタフェースを作成してくれる → VISTAでDROPDESCRIPTIONが可能
			hr = SHDoDragDrop(hwndDragImage, pdo, NULL, *peff, peff);
			top.ModifyStyleEx(0, WS_EX_ACCEPTFILES);

			m_bDppDragMyself = false;
		}
		return hr;
	}

	void SetDropTargetDescription(IDataObject* pdo, LPCTSTR pszInsert, DWORD dropEffect)
	{
		if (!pdo) return;

		// FileVisor7でなければ無視
		if (!IsDropFormatsSupported(SDO_INFILEVISORDRAGLOOP)) return;

		FORMATETC fe = { nRCF_DROPDESCRIPTION, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM stm = {0};
		stm.tymed = TYMED_HGLOBAL;
		stm.hGlobal = ::GlobalAlloc(GHND, sizeof(DROPDESCRIPTION));
		if (stm.hGlobal) {
			DROPDESCRIPTION* pdd = (DROPDESCRIPTION*)::GlobalLock(stm.hGlobal);
			pdd->type = (DROPIMAGETYPE)dropEffect;
			if (pszInsert && *pszInsert) {
				UINT id = 0;
				switch (dropEffect) {
				case DROPEFFECT_COPY: id = IDS_DROPTYPE_COPY; break;
				case DROPEFFECT_MOVE: id = IDS_DROPTYPE_MOVE; break;
				case DROPEFFECT_LINK: id = IDS_DROPTYPE_LINK; break;
				case DROPEFFECT_OPEN: id = IDS_DROPTYPE_OPEN; pdd->type = DROPIMAGE_COPY; break;
				default:              id = IDS_DROPTYPE_NONE; break;
				}
				LoadString(RESINST, id, pdd->szMessage, ARRAYSIZE(pdd->szMessage));
				lstrcpyn(pdd->szInsert, pszInsert, ARRAYSIZE(pdd->szInsert));
			}
			::GlobalUnlock(stm.hGlobal);
			pdo->SetData(&fe, &stm, TRUE);
		}
	}

	void ClearDropTargetDescription(IDataObject* pdo)
	{
		if (!pdo) return;

		// VISTAでなく且つFileVisor7でなければ無視
		if (!IsDropFormatsSupported(SDO_INFILEVISORDRAGLOOP)) return;

		FORMATETC fe = { nRCF_DROPDESCRIPTION, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		if (pdo->QueryGetData(&fe) == S_OK) {
			STGMEDIUM stm = {0};
			stm.tymed = TYMED_HGLOBAL;
			stm.hGlobal = ::GlobalAlloc(GHND, sizeof(DROPDESCRIPTION));
			if (stm.hGlobal) {
				DROPDESCRIPTION* pdd = (DROPDESCRIPTION*)::GlobalLock(stm.hGlobal);
				pdd->type = DROPIMAGE_INVALID;
				::GlobalUnlock(stm.hGlobal);
				pdo->SetData(&fe, &stm, TRUE);
			}
		}
	}

	void ReworkedDragImage()
	{
		if (!m_pdoCurrent) return;

		// マーカー取得
		bool bNeedReworked = false;
		INFILEVISORDRAGLOOP marker = { 0 };
		FORMATETC fe = { (CLIPFORMAT)nRCF_INFILEVISORDRAGLOOP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM stm;
		if (m_pdoCurrent->GetData(&fe, &stm) == S_OK) {
			INFILEVISORDRAGLOOP* pMarker = (INFILEVISORDRAGLOOP*)GlobalLock(stm.hGlobal);
			if (pMarker && !pMarker->flag && pMarker->itemcount > 1) {
				// ドラッグイメージを改造する
				memcpy(&marker, pMarker, sizeof(marker));
				bNeedReworked = true;
			}
			ReleaseStgMedium(&stm);
		}

		if (!bNeedReworked) return;

		// DragImageBitsを取り出して加工
		HGLOBAL hmem = GetDataFromDataObject(m_pdoCurrent, nRCF_DRAGIMAGEBITS);
		if (hmem) {
			DRAGIMAGEBITSSTRUCT* bits = (DRAGIMAGEBITSSTRUCT*)GlobalLock(hmem);
			Bitmap* bitmap = new Bitmap(bits->width, bits->height, 4 * bits->width, PixelFormat32bppARGB, bits->data);
			Status stat = bitmap->GetLastStatus();
			if (stat == Ok) {
				Graphics g(bitmap);

				// 上下反転して描画
				bitmap->RotateFlip(RotateNoneFlipY);

				// 文字列
				Font font(L"Arial", 12, FontStyleBold, UnitPixel);
				RectF layoutRect(0.0f, 0.0f, (REAL)bits->width, (REAL)bits->height);
				StringFormat format;
				format.SetAlignment(StringAlignmentNear);
				SolidBrush brush(Color(255, 255, 255, 255));		// 白

				// アンチエリアス
				g.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

				// 文字列矩形を計算
				CString str;
				str.Format(_T("%d"), marker.itemcount);

				RectF boundRect;
				g.MeasureString(str, str.GetLength(), &font, layoutRect, &format, &boundRect);
				boundRect.Offset(6, 6);
				boundRect.Inflate(2, 2);
				// フレーム内部背景
				SolidBrush sbr(Color(255, 0x44, 0x93, 0xeb));
				g.FillRectangle(&sbr, boundRect);
				// フレーム枠
				Pen pen(Color(255, 0x39, 0x5f, 0xcb), 1);
				g.DrawRectangle(&pen, boundRect);
				// 文字列描画
				boundRect.Inflate(-2, -2);
				g.DrawString(str, str.GetLength(), &font, boundRect, &format, &brush);

				// 上下反転して元に戻す
				bitmap->RotateFlip(RotateNoneFlipY);
				// ビットデータ抽出
				BitmapData* bitmapData = new BitmapData;
				Rect rect(0, 0, bits->width, bits->height);
				bitmap->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, bitmapData);
				memcpy(bits->data, bitmapData->Scan0, bits->width * 4 * bits->height);
				bitmap->UnlockBits(bitmapData);
				delete bitmapData;
			}
			delete bitmap;
			GlobalUnlock(hmem);

			if (stat == Ok) {
				FORMATETC few = { nRCF_DRAGIMAGEBITS, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
				STGMEDIUM stmw = {0};
				stmw.tymed = TYMED_HGLOBAL;
				stmw.hGlobal = hmem;
				m_pdoCurrent->SetData(&few, &stmw, TRUE);
			}
			else
				GlobalFree(hmem);
		}

		stm.hGlobal = ::GlobalAlloc(GHND, sizeof(INFILEVISORDRAGLOOP));
		if (stm.hGlobal) {
			INFILEVISORDRAGLOOP* pMarker = (INFILEVISORDRAGLOOP*)::GlobalLock(stm.hGlobal);
			memcpy(pMarker, &marker, sizeof(marker));
			pMarker->flag = 1;						// ドラッグイメージの改造完了
			::GlobalUnlock(stm.hGlobal);
			m_pdoCurrent->SetData(&fe, &stm, TRUE);
		}
	}
};

#endif			// __DROPPER_H__
