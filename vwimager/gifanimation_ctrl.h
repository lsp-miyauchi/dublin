/////////////////////////////////////////////////////////////////////////
// gifanimation_ctrl.h

#pragma once
#ifndef __GIFANIMATION_CTRL_H__
#define __GIFANIMATION_CTRL_H__

#include "gdip_func.h"


/////////////////////////////////////////////////////////////////////////
// class GIFAnimationImage

class GIFAnimationImage : public Image
{
protected:
	HANDLE m_thread;
	HANDLE m_pause;
	HANDLE m_exit;
	HINSTANCE m_hInst;
	HWND m_hWnd;
	UINT m_numFrames;
	UINT m_posFrame;
	PropertyItem* m_pPropItem;
	CPoint m_ptCenter;
	IStream* m_pstrm;
	bool m_bInitCompleted:1;
	bool m_bPause:1;

public:
	GIFAnimationImage(IN LPCTSTR  sResourceType, IN LPCTSTR  sResource)
	{
		ResetParams();
		if (Load(sResourceType, sResource)) {
			nativeImage = NULL;
			lastResult = DllExports::GdipLoadImageFromStreamICM(m_pstrm, &nativeImage);
			TestForAnimatedGIF();
		}
	}

	GIFAnimationImage(const WCHAR* filename, BOOL useEmbeddedColorManagement = FALSE) : Image(filename, useEmbeddedColorManagement)
	{
		ResetParams();
		m_bInitCompleted = true;
		TestForAnimatedGIF();
	}

	~GIFAnimationImage()
	{
		Destroy();
	}

	void Destroy()
	{
		if (m_thread) {
			// If pause un pause
			SetPause(false);
			SetEvent(m_exit);
			WaitForSingleObject(m_thread, INFINITE);
		}

		CloseHandle(m_thread);
		CloseHandle(m_exit);
		CloseHandle(m_pause);

		free(m_pPropItem);

		m_pPropItem = NULL;
		m_thread = NULL;
		m_exit = NULL;
		m_pause = NULL;

		if (m_pstrm)
			m_pstrm->Release();
	}

	CSize GetSize()
	{
		return CSize(GetWidth(), GetHeight());
	}

	bool IsAnimatedGIF()
	{
		return (m_numFrames > 1);
	}

	void SetPause(bool bPause)
	{
		if (!IsAnimatedGIF()) return;

		if (bPause && !m_bPause)
			::ResetEvent(m_pause);
		else if (m_bPause && !bPause)
			::SetEvent(m_pause);
		m_bPause = bPause;
	}

	bool IsPaused()
	{
		return m_bPause;
	}

	bool InitAnimation(HWND hWnd)
	{
		m_hWnd = hWnd;
		if (!m_bInitCompleted) {
			ATLTRACE(_T("GIF not initialized\n"));
			return false;
		};

		if (IsAnimatedGIF()) {
			if (m_thread == NULL) {
				unsigned int id = 0;
				m_thread = (HANDLE)_beginthreadex(NULL, 0, _ThreadAnimationProc, this, CREATE_SUSPENDED, &id);
				if (m_thread) {
					ResumeThread(m_thread);
					return true;
				} 
				else 
					ATLTRACE(_T("Couldn't start a GIF animation thread\n"));
			}
		}
		return false;
	}

	void SetPosition(CPoint ptCenter)
	{
		m_ptCenter = ptCenter;
	}

protected:

	void ResetParams()
	{
		m_pstrm = NULL;
		m_posFrame = 0;
		m_numFrames = 0;
		lastResult = InvalidParameter;
		m_thread = NULL;
		m_bInitCompleted = false;
		m_pPropItem = NULL;
		m_hInst = ATL::_AtlBaseModule.GetResourceInstance();
		m_bPause = false;
		m_exit = CreateEvent(NULL,TRUE,FALSE,NULL);
		m_pause = CreateEvent(NULL,TRUE,TRUE,NULL);
	}


	bool TestForAnimatedGIF()
	{
		m_numFrames = 0;
		UINT count = GetFrameDimensionsCount();
		if (count > 0) {
			GUID* pDimensionIDs = new GUID[count];
			// Get the list of frame dimensions from the Image object.
			GetFrameDimensionsList(pDimensionIDs, count);
			// Get the number of frames in the first dimension.
			m_numFrames = GetFrameCount(&pDimensionIDs[0]);
			// Assume that the image has a property item of type PropertyItemEquipMake.
			// Get the size of that property item.
			int nSize = GetPropertyItemSize(PropertyTagFrameDelay);
			// Allocate a buffer to receive the property item.
			m_pPropItem = (PropertyItem*) malloc(nSize);

			GetPropertyItem(PropertyTagFrameDelay, nSize, m_pPropItem);
			delete  pDimensionIDs;
		}
		return m_numFrames > 1;
	}

	bool LoadFromBuffer(BYTE* pBuff, int nSize)
	{
		bool bResult = false;
		HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, nSize);
		if (hmem) {
			void* pData = GlobalLock(hmem);
			if (pData)
				memcpy(pData, pBuff, nSize);
			GlobalUnlock(hmem);
			if (CreateStreamOnHGlobal(hmem, TRUE, &m_pstrm) == S_OK)
				bResult = true;
		}
		return bResult;
	}

	bool GetResource(LPCTSTR lpName, LPCTSTR lpType, void* pResource, int& nBufSize)
	{ 
		// Find the resource
		bool bResult = false;
		HRSRC hrsrc = FindResource(m_hInst , lpName, lpType);
		if (hrsrc) {
			// Load the resource
			HGLOBAL hglb = LoadResource(m_hInst , hrsrc);
			if (hglb) {
				void* pr = LockResource(hglb);
				if (pr) { 
					if (!pResource) {
						nBufSize = SizeofResource(m_hInst , hrsrc);
						bResult = true;
					}
					else {
						if (nBufSize >= (int)SizeofResource(m_hInst , hrsrc)) {
							memcpy(pResource, pr, nBufSize);
							bResult = true;
						}
					} 
					UnlockResource(hglb);
				}
				else dbg.printf(L"cannot lock resource\n");
				FreeResource(hglb);
			}
		}
		return bResult;
	}

	bool Load(LPCTSTR sResourceType, LPCTSTR sResource)
	{
		bool bResult = false;
		BYTE* pBuff = NULL;
		int nSize = 0;
		if (GetResource(sResource, sResourceType, pBuff, nSize)) {
			if (nSize > 0) {
				pBuff = new BYTE[nSize];
				if (GetResource(sResource, sResourceType, pBuff, nSize)) {
					if (LoadFromBuffer(pBuff, nSize)) {
						bResult = true;
					}
				}
				delete [] pBuff;
			}
		}
		m_bInitCompleted = bResult;
		return bResult;
	}

	bool DrawFrameGIF()
	{
		::WaitForSingleObject(m_pause, INFINITE);

		GUID   pageGuid = FrameDimensionTime;
		long hmWidth = GetWidth();
		long hmHeight = GetHeight();
		{
			CClientDC dc(m_hWnd);
			if (!dc.IsNull()) {
				Graphics g(dc);
				g.DrawImage(this, m_ptCenter.x - hmWidth / 2, m_ptCenter.y - hmHeight / 2, hmWidth, hmHeight);
			}
		}
		SelectActiveFrame(&pageGuid, m_posFrame++);

		if (m_posFrame == m_numFrames)
			m_posFrame = 0;

		long lPause = ((long*)m_pPropItem->value)[m_posFrame] * 5;
		DWORD ret = WaitForSingleObject(m_exit, lPause);
		return ret == WAIT_OBJECT_0;
	}

	void ThreadAnimation()
	{
		m_posFrame = 0;
		bool bExit = false;
		while (!bExit) {
			bExit = DrawFrameGIF();
		}
	}

	static UINT WINAPI _ThreadAnimationProc(LPVOID pParam)
	{
		ATLASSERT(pParam);
		GIFAnimationImage* pImage = reinterpret_cast<GIFAnimationImage*>(pParam);
		pImage->ThreadAnimation();
		return 0;
	}
};

/////////////////////////////////////////////////////////////////////////
//

class CGIFAnimationCtrl : public CWindowImpl<CGIFAnimationCtrl>
{
private:
	GIFAnimationImage* m_anim;
	COLORREF m_clrBkgnd;

public:
	DECLARE_WND_CLASS_EX(NULL, CS_PARENTDC | CS_HREDRAW | CS_VREDRAW, COLOR_WINDOW)

	CGIFAnimationCtrl()
		: m_anim(NULL)
		, m_clrBkgnd(GetSysColor(COLOR_3DFACE))
	{
	}

	~CGIFAnimationCtrl()
	{
		if (m_anim) {
			delete m_anim;
			m_anim = NULL;
		}
	}

BEGIN_MSG_MAP(CGIFAnimationCtrl)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MSG_MAP()


protected:
	int OnCreate(LPCREATESTRUCT pcs)
	{
		return 0;
	}

	void OnDestroy()
	{
		if (m_anim) {
			delete m_anim;
			m_anim = NULL;
		}
	}

	void OnPaint()
	{
		CPaintDC dc(m_hWnd);
	}

	BOOL OnEraseBkgnd(HDC hdc)
	{
		CDCHandle dc = hdc;
		CRect rc;
		GetClientRect(&rc);
		dc.FillSolidRect(&rc, RGB(64, 64, 64));
		rc.InflateRect(-2, -2);
		dc.FillSolidRect(&rc, m_clrBkgnd);
		return TRUE;
	}

public:
	bool SetAnimation(LPCTSTR pszResourceType, LPCTSTR  pszResourceName)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		m_anim = new GIFAnimationImage(pszResourceType, pszResourceName);
		// ポーズ状態
		m_anim->SetPause(true);
		return m_anim->InitAnimation(m_hWnd);
	}

	bool SetAnimation(const WCHAR* filename, BOOL useEmbeddedColorManagement = FALSE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		m_anim = new GIFAnimationImage(filename, useEmbeddedColorManagement);
		// ポーズ状態
		m_anim->SetPause(true);
		return m_anim->InitAnimation(m_hWnd);
	}

	COLORREF SetBkColor(COLORREF clr, bool bRedraw = true)
	{
		COLORREF clrPrev = m_clrBkgnd;
		if (m_clrBkgnd != clr) {
			m_clrBkgnd = clr;
			if (bRedraw && ::IsWindow(m_hWnd))
				Invalidate();
		}
		return clrPrev;
	}

	void CenterPosition(RECT* prc = NULL)
	{
		CRect rc;
		if (!prc)
			GetParent().GetClientRect(&rc);
		else
			rc = *prc;

		if (m_anim) {
			CRect rci(rc.left, rc.top, rc.left + m_anim->GetWidth() + 12, rc.top + m_anim->GetHeight() + 12);
			m_anim->SetPosition(CPoint(rci.Width() / 2, rci.Height() / 2));
			rci.OffsetRect((rc.Width() - rci.Width()) / 2, (rc.Height() - rci.Height()) / 2);
			MoveWindow(rci.left, rci.top, rci.Width(), rci.Height(), TRUE);
		}
	}

	void Show()
	{
		if (IsWindow() && !IsWindowVisible()) {
			ShowWindow(SW_SHOWNOACTIVATE);
			UpdateWindow();
			if (m_anim) m_anim->SetPause(false);
		}
	}

	void Hide()
	{
		if (IsWindow() && IsWindowVisible()) {
			if (m_anim) m_anim->SetPause(true);
			ShowWindow(SW_HIDE);
		}
	}
};


/////////////////////////////////////////////////////////////////////////
// class AnimationImageList

class AnimationImageList : public CImageList
{
protected:
	HANDLE m_thread;
	HANDLE m_pause;
	HANDLE m_exit;
	HINSTANCE m_hInst;
	HWND m_hWnd;
	UINT m_numFrames;
	UINT m_posFrame;
	CSize m_sizeIcon;
	CPoint m_ptCenter;
	CBitmap m_bmBkgnd;
	CBitmap m_bmWorker;
	bool m_bInitCompleted:1;
	bool m_bPause:1;

public:
	AnimationImageList(LPCTSTR sResourceName, int iconWidth)
	{
		m_hImageList = NULL;
		ResetParams();
		Load(sResourceName, iconWidth);
	}

	AnimationImageList(HIMAGELIST him, int iconWidth)
	{
		Attach(him);
		ResetParams();

		if (him) {
			int cx, cy;
			GetIconSize(cx, cy);
			m_sizeIcon = CSize(cx, cy);
			m_numFrames = GetImageCount();

			// 作業用ビットマップは最初から作成しておく
			CClientDC dc(NULL);
			m_bmWorker.CreateCompatibleBitmap(dc, cx, cy);
			m_bInitCompleted = true;
		}
	}

	~AnimationImageList()
	{
		Destroy();
	}

	void Destroy()
	{
		CImageList::Destroy();

		if (m_thread) {
			// If pause un pause
			SetPause(false);
			SetEvent(m_exit);
			WaitForSingleObject(m_thread, INFINITE);
		}

		CloseHandle(m_thread);
		CloseHandle(m_exit);
		CloseHandle(m_pause);

		m_thread = NULL;
		m_exit = NULL;
		m_pause = NULL;
	}

	long GetWidth()
	{
		return m_sizeIcon.cx;
	}

	long GetHeight()
	{
		return m_sizeIcon.cy;
	}

	bool GetRect(RECT* prc)
	{
		if (prc) {
			::SetRect(prc, 0, 0, m_sizeIcon.cx, m_sizeIcon.cy);
			::OffsetRect(prc, m_ptCenter.x - m_sizeIcon.cx / 2, m_ptCenter.y - m_sizeIcon.cy / 2);
		}
		return (prc != NULL);
	}

	void SetPause(bool bPause)
	{
		if (bPause && !m_bPause)
			::ResetEvent(m_pause);
		else if (m_bPause && !bPause) {
			// 親画面が更新されている可能性があるので、古い情報は削除する
			m_bmBkgnd.DeleteObject();
			// フレーム位置を先頭に戻す
			m_posFrame = 0;
			::SetEvent(m_pause);
		}
		m_bPause = bPause;
	}

	inline void Start() { SetPause(false); }
	inline void Stop() { SetPause(true); }

	bool IsPaused()
	{
		return m_bPause;
	}

	bool InitAnimation(HWND hWnd)
	{
		m_hWnd = hWnd;
		if (!m_bInitCompleted) {
			ATLTRACE(_T("AnimationImageList not initialized\n"));
			return false;
		};

		if (m_thread == NULL) {
			unsigned int id = 0;
			m_thread = (HANDLE)_beginthreadex(NULL, 0, _ThreadAnimationProc, this, CREATE_SUSPENDED, &id);
			if (m_thread) {
				// スレッドがそのまま実行しないようにポーズする
				::ResetEvent(m_pause);
				m_bPause = true;
				ResumeThread(m_thread);
				return true;
			} 
			else 
				ATLTRACE(_T("Couldn't start a GIF animation thread\n"));
		}
		return false;
	}

	void SetPosition(CPoint ptCenter)
	{
		m_ptCenter = ptCenter;
	}

protected:

	void ResetParams()
	{
		m_posFrame = 0;
		m_numFrames = 0;
		m_sizeIcon = CSize(0, 0);
		m_bmBkgnd.DeleteObject();
		m_bmWorker.DeleteObject();
		m_thread = NULL;
		m_bInitCompleted = false;
		m_hInst = ATL::_AtlBaseModule.GetResourceInstance();
		m_bPause = false;
		m_exit = CreateEvent(NULL,TRUE,FALSE,NULL);
		m_pause = CreateEvent(NULL,TRUE,TRUE,NULL);
	}

	bool Load(LPCTSTR sResourceName, int iconWidth)
	{
		bool bResult = false;
		if (CreateFromImage(sResourceName, iconWidth, 0, RGB(255, 0, 255), IMAGE_BITMAP, LR_CREATEDIBSECTION | LR_DEFAULTCOLOR)) {
			bResult = true;
			int cx, cy;
			GetIconSize(cx, cy);
			m_sizeIcon = CSize(cx, cy);
			m_numFrames = GetImageCount();

			// 作業用ビットマップは最初から作成しておく
			CClientDC dc(NULL);
			m_bmWorker.CreateCompatibleBitmap(dc, cx, cy);
		}
		m_bInitCompleted = bResult;
		return bResult;
	}

	bool DrawFrame()
	{
		::WaitForSingleObject(m_pause, INFINITE);

		{
			CClientDC dc(m_hWnd);
			if (::IsWindowVisible(m_hWnd) && !dc.IsNull()) {
				CDC dcw; dcw.CreateCompatibleDC(dc);
				HBITMAP hbmw;
				if (m_bmBkgnd.IsNull()) {
					m_bmBkgnd.CreateCompatibleBitmap(dc, m_sizeIcon.cx, m_sizeIcon.cy);
					hbmw = dcw.SelectBitmap(m_bmBkgnd);
					dcw.BitBlt(0, 0, m_sizeIcon.cx, m_sizeIcon.cy, dc, m_ptCenter.x - m_sizeIcon.cx / 2, m_ptCenter.y - m_sizeIcon.cy / 2, SRCCOPY);
				}
				else
					hbmw = dcw.SelectBitmap(m_bmBkgnd);

				CDC dcm; dcm.CreateCompatibleDC(dc);
				HBITMAP hbmm = dcm.SelectBitmap(m_bmWorker);
				dcm.BitBlt(0, 0, m_sizeIcon.cx, m_sizeIcon.cy, dcw, 0, 0, SRCCOPY);
				Draw(dcm, m_posFrame++, 0, 0, ILD_TRANSPARENT);
				dc.BitBlt(m_ptCenter.x - m_sizeIcon.cx / 2, m_ptCenter.y - m_sizeIcon.cy / 2, m_sizeIcon.cx, m_sizeIcon.cy, dcm, 0, 0, SRCCOPY);
				dcm.SelectBitmap(hbmm);
				dcw.SelectBitmap(hbmw);
				GdiFlush();
			}
		}

		if (m_posFrame == m_numFrames)
			m_posFrame = 0;

		long lPause = 10 * 5;
		DWORD ret = WaitForSingleObject(m_exit, lPause);
		return ret == WAIT_OBJECT_0;
	}

	void ThreadAnimation()
	{
		m_posFrame = 0;
		bool bExit = false;
		while (!bExit) {
			bExit = DrawFrame();
		}
	}

	static UINT WINAPI _ThreadAnimationProc(LPVOID pParam)
	{
		ATLASSERT(pParam);
		AnimationImageList* pImage = reinterpret_cast<AnimationImageList*>(pParam);
		pImage->ThreadAnimation();
		return 0;
	}
};

/////////////////////////////////////////////////////////////////////////
//

class CXparentAnimationCtrl : public CWindowImpl<CXparentAnimationCtrl>
{
private:
	AnimationImageList* m_anim;

public:
	DECLARE_WND_CLASS_EX(NULL, CS_PARENTDC | CS_HREDRAW | CS_VREDRAW, COLOR_WINDOW)

	CXparentAnimationCtrl()
		: m_anim(NULL)
	{
	}

	~CXparentAnimationCtrl()
	{
		if (m_anim) {
			delete m_anim;
			m_anim = NULL;
		}
	}

BEGIN_MSG_MAP(CXparentAnimationCtrl)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MSG_MAP()


protected:
	void OnDestroy()
	{
		if (m_anim) {
			delete m_anim;
			m_anim = NULL;
		}
	}

	void OnPaint()
	{
		CPaintDC dc(m_hWnd);
	}

	BOOL OnEraseBkgnd(HDC hdc)
	{
		return TRUE;
	}

public:
	bool SetAnimation(LPCTSTR pszResourceName, int iconWidth)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		m_anim = new AnimationImageList(pszResourceName, iconWidth);
		return m_anim->InitAnimation(m_hWnd);
	}

	void CenterPosition(RECT* prc = NULL)
	{
		CRect rc;
		if (!prc)
			GetParent().GetClientRect(&rc);
		else
			rc = *prc;

		if (m_anim) {
			CRect rci(rc.left, rc.top, rc.left + m_anim->GetWidth() + 12, rc.top + m_anim->GetHeight() + 12);
			m_anim->SetPosition(CPoint(rci.Width() / 2, rci.Height() / 2));
			rci.OffsetRect((rc.Width() - rci.Width()) / 2, (rc.Height() - rci.Height()) / 2);
			MoveWindow(rci.left, rci.top, rci.Width(), rci.Height(), TRUE);
		}
	}

	void Show()
	{
		if (IsWindow() && !IsWindowVisible()) {
			ShowWindow(SW_SHOWNOACTIVATE);
			UpdateWindow();
			if (m_anim) m_anim->SetPause(false);
		}
	}

	void Hide()
	{
		if (IsWindow() && IsWindowVisible()) {
			if (m_anim) m_anim->SetPause(true);
			ShowWindow(SW_HIDE);
		}
	}
};

#endif			// __GIFANIMATION_CTRL_H__

