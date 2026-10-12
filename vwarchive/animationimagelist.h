/////////////////////////////////////////////////////////////////////////
// animationimagelist.h

#pragma once
#ifndef __ANIMATIONIMAGELIST_H__
#define __ANIMATIONIMAGELIST_H__

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
#endif			// __ANIMATIONIMAGELIST_H__

