// RawThmubnail.cpp : CRawThmubnail の実装

#include "stdafx.h"
#include "RawThmubnail.h"

BOOL exfGetExifInfo(LPCTSTR fname, IStream* pstrm, int* pnOrientation, BOOL bLargeImage, EXIFINFOSTRUCT* info);

////////////////////////////////////////////////////////////////////////
// CRawThmubnail

#define SETFormatEtc(fe, cf, med, li)\
		{\
			(fe).cfFormat = cf;\
			(fe).dwAspect = DVASPECT_CONTENT;\
			(fe).ptd = NULL;\
			(fe).tymed = med;\
			(fe).lindex = li;\
		};

HRESULT CRawThmubnail::Initialize(IShellItem* pShellItem, DWORD grfMode)
{
	HRESULT hr = E_UNEXPECTED;
	if (pShellItem != NULL) {
		// store the reference to the stream.
		m_pShellItem = pShellItem;
		m_pShellItem->AddRef();
		hr = S_OK;
	}
	return hr;
}

static HGLOBAL GetDataFromDataObject(IDataObject* pdo, CLIPFORMAT cf)
{
	HGLOBAL hmem = NULL;
	FORMATETC fe;
	SETFormatEtc(fe, cf, TYMED_HGLOBAL, -1);
	if (pdo->QueryGetData(&fe) == S_OK) {
		STGMEDIUM sm;
		if (pdo->GetData(&fe, &sm) == S_OK) {
			hmem = lccDuplicateMemoryHandle(sm.hGlobal, GMEM_SHARE | GMEM_MOVEABLE);
			ReleaseStgMedium(&sm);
		}
	}
	return hmem;
}

static void calc_inner_image_size(int cxMax, int cyMax, int* pcx, int* pcy)
{
	if (*pcx > cxMax || *pcy > cyMax) {
		double xRatio = (double)cxMax / (double)*pcx;
		double yRatio = (double)cyMax / (double)*pcy;
		double ratio = (xRatio > yRatio) ? yRatio : xRatio;
		*pcx = (int)floor(*pcx * ratio);
		*pcy = (int)floor(*pcy * ratio);
	}
	return;
}

static HBITMAP load_thumbnail(Bitmap& bitmap, int width, int height)
{
	HBITMAP hbm = NULL;
	int cx = bitmap.GetWidth();
	int cy = bitmap.GetHeight();
	if (cx > 0 && cy > 0) {
		calc_inner_image_size(width, height, &cx, &cy);
		Image* thumb = bitmap.GetThumbnailImage(cx, cy, NULL, NULL);
		if (thumb) {
			Bitmap base(cx, cy, PixelFormat24bppRGB);
			Graphics g(&base);
			// イメージ
			g.DrawImage(thumb, 0, 0, cx, cy);
			// HBITMAP抽出
			Color color(255, 255, 255, 255);
			if (base.GetHBITMAP(color, &hbm) != Ok)
				hbm = NULL;
			delete thumb;
		}
	}
	return hbm;
}

HRESULT CRawThmubnail::GetThumbnail(UINT cx, HBITMAP* phbm, WTS_ALPHATYPE* pdwAlpha)
{
	HRESULT hr = E_FAIL;
	if (m_pShellItem) {
		IDataObject* pdo;
		if (m_pShellItem->BindToHandler(NULL, BHID_SFUIObject, IID_IDataObject, (void**)&pdo) == S_OK) {
			FORMATETC fe;
			SETFormatEtc(fe, CF_HDROP, TYMED_HGLOBAL, -1);
			if (pdo->QueryGetData(&fe) == S_OK) {
				STGMEDIUM sm;
				if (pdo->GetData(&fe, &sm) == S_OK) {
					TCHAR name[MAX_PATH * 2];
					if (DragQueryFile((HDROP)sm.hGlobal, 0, name, ARRAYSIZE(name)) > 0) {
						IStream* pstrm;
						if (CreateStreamOnHGlobal(NULL, TRUE, &pstrm) == S_OK) {
							int orientation;
							if (exfGetExifInfo(name, pstrm, &orientation, false, NULL)) {
								Bitmap bitmap(pstrm);
								switch (orientation) {
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
								*phbm = load_thumbnail(bitmap, cx, cx);
								*pdwAlpha = WTSAT_RGB;
							}
							pstrm->Release();
						}
					}
					ReleaseStgMedium(&sm);
				}
			}
			pdo->Release();
		}
	}
    return hr;
}
