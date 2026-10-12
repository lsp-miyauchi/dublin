////////////////////////////////////////////////////////////////////////
// slideshow.h

#pragma once

#include "dib.h"
#include "qsort.h"
#include "loaderthread.h"
#include "imagebuttonctrl.h"
#include "gifanimation_ctrl.h"
//#include "faderctrl.h"
#include "gdip_func.h"
#include "resource.h"


////////////////////////////////////////////////////////////////////////
//

class CPictureItem : public CBitmap
{
public:
	CString m_strFileName;
	CString m_strExif;
	CString m_strInformation;
	CSize m_sizeImage;

public:
	CPictureItem()
	{
	}

	CPictureItem(const CPictureItem& src)
	{
		CopyFrom(src);
	}

	void Clear()
	{
		if (!IsNull()) DeleteObject();
		m_sizeImage = CSize(0, 0);
		m_strFileName.Empty();
		m_strExif.Empty();
		m_strInformation.Empty();
	}

	void SetPicture(HDIB hdib, LPCTSTR pszFileName, int index, int entries)
	{
		Clear();

		m_strFileName = pszFileName;
		CDib dib(hdib);
		if (!dib.IsNull()) {
			m_sizeImage.cx = dib.Width();
			m_sizeImage.cy = dib.Height();

			HBITMAP hbm = dib.GetHBitmap(Color(255, 0, 0, 0));
			ATLASSERT(hbm != NULL);
			Attach(hbm);

			size_t size = sizeof(EXIFINFOSTRUCT) + L_MAX_URL_LENGTH * sizeof(TCHAR);
			EXIFINFOSTRUCT* info = (EXIFINFOSTRUCT*)malloc(size);
			memset(info, 0, size);
			info->cchTipText = L_MAX_URL_LENGTH;
			info->pszTipText = (LPTSTR)((BYTE*)info + sizeof(EXIFINFOSTRUCT));

			CComPtr<IExifParser> pep;
			HRESULT hr = pep.CoCreateInstance(__uuidof(ExifParser));
			if (SUCCEEDED(hr)) {
				if (pep->GetExifInfo(m_strFileName, NULL, NULL, false, (BYTE*)info) == S_OK && *info->pszTipText)
					m_strExif = info->pszTipText;
			}
			free(info);

			CString fmt;
			if      (entries >= 1000) fmt = _T("[%04d");
			else if (entries >= 100)  fmt = _T("[%03d");
			else if (entries >= 100)  fmt = _T("[%02d");
			else                      fmt = _T("[%d");
			fmt += _T(" / %d] %s");
			m_strInformation.Format(fmt, index + 1, entries, PathFindFileName(pszFileName));

			if (!m_strExif.IsEmpty()) {
				m_strInformation += _T("\n\n");
				m_strInformation += m_strExif;
			}
		}
	}

	inline long Width() { return m_sizeImage.cx; }
	inline long Height() { return m_sizeImage.cy; }
	inline LPCTSTR GetName() { return m_strFileName; }
	inline LPCTSTR GetExif() { return m_strExif; }
	inline LPCTSTR GetInformation() { return m_strInformation; }

	// Assignment operators
	inline CPictureItem& operator=(const CPictureItem& src) { return CopyFrom(src); }

private:
	CPictureItem& CopyFrom(const CPictureItem& src)
	{
		if (*this != src) {
			Clear();

			m_strFileName = src.m_strFileName;
			m_strExif = src.m_strExif;
			m_strInformation = src.m_strInformation;
			m_sizeImage = src.m_sizeImage;

			// ビットマップを複製する
			if (!src.IsNull()) {
				CClientDC dc(NULL);
				BITMAP bm;
				src.GetBitmap(&bm);
				CreateCompatibleBitmap(dc, bm.bmWidth, bm.bmHeight);

				CDC dc1; dc1.CreateCompatibleDC(dc);
				CDC dc2; dc2.CreateCompatibleDC(dc);
				HBITMAP hbm1 = dc1.SelectBitmap(m_hBitmap);
				HBITMAP hbm2 = dc2.SelectBitmap(src.m_hBitmap);
				dc1.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, dc2, 0, 0, SRCCOPY);

				dc2.SelectBitmap(hbm2);
				dc1.SelectBitmap(hbm1);
			}
		}
		return( *this );
	}

};

////////////////////////////////////////////////////////////////////////
//

#define SLIDESHOW_CLASS			_T("VWSLIDESHOW_FRAME")

#define IDC_LOADING					1001
#define IDC_STARTBTN				1002
#define IDC_PREVBTN					1003
#define IDC_NEXTBTN					1004
#define IDC_STOPBTN					1005

class CSlideFrame : public CWindowImpl<CSlideFrame>
{
private:
	CSimpleArray<CString> m_arrFiles;
	CSimpleMap<CString, CPictureItem> m_mapHistory;
	int m_pos;
	CPictureItem m_pic;
	CBitmap m_cache;
	UINT_PTR m_timerid;
	CSize m_sizeScreen;						// スクリーンサイズ
	HACCEL m_hacc;
//	CFaderCtrl m_fader;
	CStartButtonWnd m_startbtn;
	CImageButtonWnd m_prevbtn;
	CImageButtonWnd m_nextbtn;
	CImageButtonWnd m_stopbtn;
	AnimationImageList* m_anim;
	CLoaderThread m_loader;
	UINT m_loadcompleted;
	DWORD m_magic;
	CString m_strCurrentFileName;
	DWORD m_timerinterval;
	bool m_bInitializeCompeleted:1;
	bool m_bPaused:1;
	bool m_bShowInformation:1;

public:
	DECLARE_WND_CLASS_EX(SLIDESHOW_CLASS, CS_OWNDC, COLOR_WINDOW)

	HWND Create()
	{
		HWND hwnd = CWindowImpl::Create(NULL, NULL, NULL, WS_POPUP | WS_CLIPSIBLINGS/* | WS_CLIPCHILDREN*/ | WS_VISIBLE, WS_EX_TOOLWINDOW | WS_EX_TOPMOST);
		ATLASSERT(hwnd != NULL);
		return hwnd;
	}

	void MessageLoop()
	{
		while (IsWindow()) {
			MSG msg;
			while (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
				if (::GetMessage(&msg, NULL, 0, 0)) {
					if (!m_hacc || !::TranslateAccelerator(m_hWnd, m_hacc, &msg)) {
						::TranslateMessage(&msg);
						::DispatchMessage(&msg);
					}
				}
			}
		}
	}

BEGIN_MSG_MAP(CSlideFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_SIZE()
//	ON_WM_LBUTTONDOWN()
//	ON_WM_LBUTTONUP()
	ON_WM_CONTEXTMENU()
	ON_WM_ERASEBKGND()
	ON_COMMAND(IDM_SLIDESHOW_START, OnCommandStart)
	ON_COMMAND(IDM_SLIDESHOW_PAUSE, OnCommandPause)
	ON_COMMAND(ID_APP_EXIT, OnCommandAppExit)
	ON_COMMAND(IDM_SLIDESHOW_PREV, OnCommandPrev)
	ON_COMMAND(IDM_SLIDESHOW_NEXT, OnCommandNext)
	ON_BN_CLICKED(IDC_STARTBTN, OnBtnClicked)
	ON_BN_CLICKED(IDC_PREVBTN, OnCommandPrev)
	ON_BN_CLICKED(IDC_NEXTBTN, OnCommandNext)
	ON_BN_CLICKED(IDC_STOPBTN, OnCommandAppExit)
	ON_REGISTERD_MESSAGE(m_loadcompleted, OnHandlerLoadCompleted)
END_MSG_MAP()

	CSlideFrame()
		: m_pos(0)
		, m_timerid(0)
		, m_hacc(NULL)
		, m_loadcompleted(::RegisterWindowMessage(_T("wm_imageloaded")))
		, m_magic(0)
		, m_timerinterval(2000)
		, m_bInitializeCompeleted(false)
		, m_bPaused(false)
		, m_bShowInformation(true)
		, m_anim(NULL)
	{
		static ACCEL acc[4] = {
			{ FVIRTKEY | FNOINVERT, VK_ESCAPE, ID_APP_EXIT },
			{ FVIRTKEY | FNOINVERT, VK_PAUSE, IDM_SLIDESHOW_PAUSE },
			{ FVIRTKEY | FNOINVERT, VK_SPACE, IDM_SLIDESHOW_PAUSE },
			{ FVIRTKEY | FNOINVERT, VK_F5, IDM_SLIDESHOW_START },
		};
		m_hacc = CreateAcceleratorTable(acc, ARRAYSIZE(acc));
	}

	~CSlideFrame()
	{
		if (m_anim) delete m_anim;
		m_pic.Clear();
		m_cache.DeleteObject();
		if (m_hacc) DestroyAcceleratorTable(m_hacc);
	}

	bool Initialize(LPCTSTR pszStartFileName)
	{
		m_bInitializeCompeleted = false;

		TCHAR path[MAX_PATH * 2];
		lstrcpy(path, pszStartFileName);
		BOOL bDirectory = PathIsDirectory(pszStartFileName);
		if (!bDirectory)
			PathRemoveFileSpec(path);
		TCHAR name[MAX_PATH * 2];
		PathCombine(name, path, _T("*.*"));

		CSimpleArray<LPTSTR> array;
		WIN32_FIND_DATA wfd;
		HANDLE hFind = FindFirstFile(name, &wfd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
				if (!imgTypeIsExif(imgSpecifyImageType(wfd.cFileName))) continue;

				array.Add(StrDup(wfd.cFileName));
			} while (FindNextFile(hFind, &wfd));
			FindClose(hFind);
		}

		int entries = array.GetSize();
		if (entries > 0) {
			if (entries > 1) {
				CQSort qs;
				qs.sort(array.GetData(), entries, sizeof(LPTSTR), NULL, NULL);
			}

			for (int i = 0; i < entries; i++) {
				if (array[i]) {
					PathCombine(name, path, array[i]);
					m_arrFiles.Add(name);
					if (!bDirectory && !lstrcmpi(name, pszStartFileName))
						m_pos = m_arrFiles.GetSize() - 1;
					LocalFree(array[i]);
				}
			}

			m_bInitializeCompeleted = true;
			if (::IsWindow(m_hWnd))
				PostMessage(WM_COMMAND, IDM_SLIDESHOW_START);
		}
		else {
			AppMessage(IDR_SLIDESHOW, _T("スライド ショウの対象となるファイルが見つかりません。"), NULL, MB_STOP);
		}

		return m_bInitializeCompeleted;
	}

	int OnCreate(LPCREATESTRUCT pcs)
	{
		// メニュー
		HMENU hmenu = GetMenu();
		SetMenu(NULL);
		if (hmenu)
			DestroyMenu(hmenu);

		// フェーダー
//		m_fader.Create(m_hWnd, NULL, NULL, WS_CHILD | WS_VISIBLE);

		// スクリーンサイズ
		m_sizeScreen.cx = GetSystemMetrics(SM_CXSCREEN);
		m_sizeScreen.cy = GetSystemMetrics(SM_CYSCREEN);
		MoveWindow(0, 0, m_sizeScreen.cx, m_sizeScreen.cy, TRUE);

		// Loading animation
		HIMAGELIST him = lccLoadImageList(RESINST, RT_RCDATA, MAKEINTRESOURCE(IDB_LOADER_WHITE_48X48), 48, CLR_NONE);
		m_anim = new AnimationImageList(him, 48);
		if (m_anim) {
			m_anim->InitAnimation(m_hWnd);
			CRect rc(0, 0, m_sizeScreen.cx, m_sizeScreen.cy);
			m_anim->SetPosition(rc.CenterPoint());
			m_anim->Start();
		}

		// ボタン
		m_startbtn.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE, 0, IDC_STARTBTN);

		m_prevbtn.SetImageResource(IDB_SLIDESHOW_PREV, CSize(32, 32));
		m_prevbtn.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE, 0, IDC_PREVBTN);
		m_nextbtn.SetImageResource(IDB_SLIDESHOW_NEXT, CSize(32, 32));
		m_nextbtn.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE, 0, IDC_NEXTBTN);
		m_stopbtn.SetImageResource(IDB_SLIDESHOW_STOP, CSize(32, 32));
		m_stopbtn.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE, 0, IDC_STOPBTN);

		// ローダー
		m_loader.Initialize(m_hWnd, m_loadcompleted);

		if (m_bInitializeCompeleted)
			PostMessage(WM_COMMAND, IDM_SLIDESHOW_START);
		return 0;
	}

	void OnDestroy()
	{
		StopTimer();
	}

	void StartTimer()
	{
		if (!m_timerid)
			m_timerid = SetTimer(1001, m_timerinterval, NULL);
	}

	void StopTimer()
	{
		if (m_timerid) {
			KillTimer(m_timerid);
			m_timerid = 0;
		}
	}

	void OnTimer(UINT_PTR idEvent)
	{
		if (idEvent == m_timerid) {
			StopTimer();
			PostMessage(WM_COMMAND, IDM_SLIDESHOW_START);
		}
	}

	void OnSize(UINT nType, int cx, int cy)
	{
		if (nType == SIZE_MINIMIZED) return;

		// ビットマップキャッシュをクリア
		m_cache.DeleteObject();

		// フェーダー
		CRect rc;
		rc.SetRect(0, 0, cx, cy);
//		m_fader.MoveWindow(&rc, TRUE);

		// ボタン
		CSimpleArray<HWND> arr;
		arr.Add(m_prevbtn);
		arr.Add(m_startbtn);
		arr.Add(m_nextbtn);
		arr.Add(m_stopbtn);

		int num = arr.GetSize();
		rc.SetRect(0, 0, cx, cy);
		rc.InflateRect(-8, -8);
		rc.left = rc.left + (rc.Width() - (num * 32)) / 2;
		rc.right = rc.left + 32;
		rc.top = rc.bottom - 32;
		for (int i = 0; i < num; i++) {
			::MoveWindow(arr[i], rc.left, rc.top, rc.Width(), rc.Height(), TRUE);
			rc.OffsetRect(32, 0);
		}
	}

	LRESULT OnHandlerLoadCompleted(WPARAM wParam, LPARAM lParam)
	{
		HDIB hdib = (HDIB)lParam;
		DWORD magic = m_magic;
		m_magic = 0;

		if ((DWORD)wParam != magic) {
			if (hdib)
				GlobalFree(hdib);

			if (m_anim) m_anim->Stop();
			return 0;
		}

		if (m_startbtn.IsPaused()) {
			// せっかく読んだので捨てずにヒストリに格納しておく
			CString str = m_arrFiles[m_pos];
			CPictureItem pic; pic.SetPicture(hdib, str, m_pos, m_arrFiles.GetSize());
			if (hdib)
				GlobalFree(hdib);

			if (m_mapHistory.GetSize() >= 10)
				m_mapHistory.RemoveAt(0);
			m_mapHistory.Add(str, pic);

			if (m_anim) m_anim->Stop();
			return 0;
		}

		ShowPicture(m_pos, hdib);
		if (++m_pos >= m_arrFiles.GetSize())
			m_pos = 0;

		StartTimer();
		return 0;
	}

	void ShowPicture(int index, HDIB hdib)
	{
		m_pic.Clear();
		m_cache.DeleteObject();

		CString strFileName = m_arrFiles[index];
		m_pic.SetPicture(hdib, strFileName, index, m_arrFiles.GetSize());
		if (hdib) {
			GlobalFree(hdib);

			if (m_mapHistory.GetSize() >= 10)
				m_mapHistory.RemoveAt(0);
			m_mapHistory.Add(strFileName, m_pic);
			CreateCachedBitmap();
		}

		if (m_anim) m_anim->Stop();

		Invalidate(FALSE);
		UpdateWindow();
	}

	void OnCommandStart()
	{
		m_startbtn.SetPause(false);
		int pos = m_mapHistory.FindKey(m_arrFiles[m_pos]);
		if (pos < 0) {
			if (m_anim) m_anim->Start();
			m_magic = GetTickCount();
			m_loader.Start(m_magic, (LPARAM)(LPCTSTR)m_arrFiles[m_pos]);
		}
		else {
			m_cache.DeleteObject();
			m_pic = m_mapHistory.GetValueAt(pos);
			if (++m_pos >= m_arrFiles.GetSize())
				m_pos = 0;
			StartTimer();
			Invalidate(FALSE);
		}
	}

	void OnCommandPause()
	{
		if (m_startbtn.IsPaused()) {
			PostMessage(WM_COMMAND, IDM_SLIDESHOW_START);
		}
		else {
			m_startbtn.SetPause(true);
			StopTimer();
		}
	}

	void OnCommandPrev()
	{
		Pause();

		if (--m_pos < 0)
			m_pos = m_arrFiles.GetSize() - 1;
		if (--m_pos < 0)
			m_pos = m_arrFiles.GetSize() - 1;

		int pos = m_mapHistory.FindKey(m_arrFiles[m_pos]);
		if (pos < 0) {
			if (m_anim) m_anim->Start();
			HDIB hdib = imgLoadImage(m_arrFiles[m_pos]);
			ShowPicture(m_pos, hdib);
		}
		else {
			m_cache.DeleteObject();
			m_pic = m_mapHistory.GetValueAt(pos);
			CreateCachedBitmap();

			Invalidate(FALSE);
		}
		if (++m_pos >= m_arrFiles.GetSize())
			m_pos = 0;
	}

	void OnCommandNext()
	{
		Pause();

		int pos = m_mapHistory.FindKey(m_arrFiles[m_pos]);
		if (pos < 0) {
			if (m_anim) m_anim->Start();
			HDIB hdib = imgLoadImage(m_arrFiles[m_pos]);
			ShowPicture(m_pos, hdib);
		}
		else {
			m_cache.DeleteObject();
			m_pic = m_mapHistory.GetValueAt(pos);
			CreateCachedBitmap();

			Invalidate(FALSE);
		}
		if (++m_pos >= m_arrFiles.GetSize())
			m_pos = 0;
	}

	void OnCommandAppExit()
	{
		PostMessage(WM_CLOSE);
	}

	void OnBtnClicked()
	{
		OnCommandPause();
	}

	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		switch (nChar) {
		case VK_DOWN:
			break;
		case VK_UP:
			break;
		case VK_RIGHT:
			break;
		case VK_HOME:
			break;
		case VK_END:
			break;
		case VK_LEFT:
			break;
		case VK_ESCAPE:
			break;
		case VK_RETURN:
		case VK_SPACE:
			break;
	    }
	}

/*
	void OnPrintClient(HDC hdc, LPARAM flags)
	{
		if (flags & PRF_ERASEBKGND)
			DefWindowProc(WM_ERASEBKGND, (WPARAM)hdc, 0);
		if (flags & (PRF_CHECKVISIBLE | PRF_CLIENT | PRF_OWNED))
			OnDraw(hdc);
	}
*/
	void OnPaint()
	{
		CPaintDC dc(m_hWnd);
		OnDraw(dc);
	}

	void CreateCachedBitmap()
	{
		if (!m_pic.IsNull() && m_cache.IsNull()) {
			CRect rc;
			GetClientRect(&rc);

			Bitmap bitmap((HBITMAP)m_pic, (HPALETTE)NULL);
			Bitmap cache(rc.Width(), rc.Height(), PixelFormat24bppRGB);
			Graphics g(&cache);
			SolidBrush brush(Color(255, 0, 0, 0));
			g.FillRectangle(&brush, 0, 0, rc.Width(), rc.Height());

			int cx = m_pic.Width();
			int cy = m_pic.Height();
			CalcInnerImageSize(rc.Width(), rc.Height(), &cx, &cy);
			int x = (rc.Width() - cx) / 2;
			int y = (rc.Height() - cy) / 2;
			g.DrawImage(&bitmap, x, y, cx, cy);

			// GDI+で文字列出力
			CString str = m_pic.GetInformation();
			if (m_bShowInformation && !str.IsEmpty()) {
				g.SetSmoothingMode(SmoothingModeHighQuality);
				const int nPointSize = 10;
				Font myFont(L"MS UI Gothic", (Gdiplus::REAL)nPointSize, FontStyleRegular, UnitPoint);
				RectF r(0, 0, (Gdiplus::REAL)rc.Width(), (Gdiplus::REAL)rc.Height());
				r.Inflate(-8, -8);
				RectF br;
				g.MeasureString(str, str.GetLength(), &myFont, r, &br);
				br.Width += 12;
				br.Height += 12;

				Pen pen(Color(255, 64, 64, 64), 1);
 				SolidBrush brush(Color(160, 64, 64, 64));
				GdipRoundRect(g, &pen, &brush, br, CSize(14, 14));

				br.Inflate(-6, -6);
				StringFormat fmt;
				fmt.SetAlignment(StringAlignmentNear);						// DT_CENTER
				fmt.SetFormatFlags(StringFormatFlagsNoWrap);				// DT_SINGLELINE
				fmt.SetLineAlignment(StringAlignmentCenter);				// DT_VCENTER
				fmt.SetTrimming(StringTrimmingEllipsisCharacter);			// followed by "..."
				SolidBrush sbr(Color(255, 255, 255, 255));
				g.SetTextRenderingHint(TextRenderingHintAntiAlias);
				g.DrawString(str, str.GetLength(), &myFont, br, &fmt, &sbr);
			}

			HBITMAP hbm;
			cache.GetHBITMAP(Color(255, 0, 0, 0), &hbm);
			m_cache.Attach(hbm);
		}
	}

	void OnDraw(HDC hdc)
	{
		if (!m_pic.IsNull()) {
			CRect rc;
			GetClientRect(&rc);

			CDC dcm; dcm.CreateCompatibleDC(hdc);
			HBITMAP hbm = dcm.SelectBitmap(m_cache);
			::BitBlt(hdc, 0, 0, rc.Width(), rc.Height(), dcm, 0, 0, SRCCOPY);
			dcm.SelectBitmap(hbm);
			dcm.DeleteDC();
		}
	}

	void OnLButtonDown(UINT nFlags, CPoint pt)
	{
	}

	void OnLButtonUp(UINT nFlags, CPoint pt)
	{
	}

	void Pause()
	{
		if (!m_startbtn.IsPaused()) {
			m_startbtn.SetPause(true);
			StopTimer();
		}
	}

	void OnContextMenu(HWND hWnd, CPoint pt)
	{
		if (hWnd == m_hWnd) {
			bool paused = m_startbtn.IsPaused();
			if (!paused) {
				m_startbtn.SetPause(true);
				StopTimer();
			}

			CMenu menu;
			menu.LoadMenu(IDR_SLIDESHOW);
			CMenuHandle pop = menu.GetSubMenu(0);
			if (!pop.IsNull()) {
				if (m_bShowInformation)
					pop.CheckMenuItem(IDM_SLIDESHOW_INFORMATION, MF_BYCOMMAND | MF_CHECKED);

				int idCmd = ::TrackPopupMenu(pop, TPM_RETURNCMD, pt.x,  pt.y, 0, m_hWnd, NULL);
				switch (idCmd) {
				case IDM_EDIT_COPY:
					if (OpenClipboard()) {
						EmptyClipboard();
						SetClipboardData(CF_BITMAP, (HBITMAP)m_pic);
						CloseClipboard();
					}
					break;
				case IDM_SLIDESHOW_INFORMATION:
					if (!m_cache.IsNull())
						m_cache.DeleteObject();
					m_bShowInformation ^= true;
					Invalidate(FALSE);
					UpdateWindow();
					break;
				case IDM_EXEC_EDITOR:
					if (!m_strCurrentFileName.IsEmpty()) {
						lccDllExecute(m_hWnd, DLL_GMIL, _T("pxfRun"), m_strCurrentFileName, _T("/image"), FALSE);
						PostMessage(WM_CLOSE);
					}
					break;
				case IDM_SLIDESHOW_PREV:
				case IDM_SLIDESHOW_NEXT:
					paused = true;			// 以後ポーズ状態を続ける
					// 下へ続く
				default:
					if (idCmd != 0)
						PostMessage(WM_COMMAND, idCmd);
					break;
				}
			}

			if (!paused) {
				PostMessage(WM_COMMAND, IDM_SLIDESHOW_START);
			}
		}
	}

	BOOL OnEraseBkgnd(HDC hdc)
	{
		CRect rc;
		GetClientRect(&rc);
		CDCHandle dc = hdc;

//@		dc.FillSolidRect(&rc, RGB(0, 0, 0));
		dc.PatBlt(0, 0, rc.Width(), rc.Height(), BLACKNESS);
		return TRUE;
	}

	void CalcInnerImageSize(int cxMax, int cyMax, int* pcx, int* pcy)
	{
		if (*pcx > cxMax || *pcy > cyMax) {
			double xRatio = (double)cxMax / (double)*pcx;
			double yRatio = (double)cyMax / (double)*pcy;
			double ratio = (xRatio > yRatio) ? yRatio : xRatio;
			*pcx = (int)floor(*pcx * ratio);
			*pcy = (int)floor(*pcy * ratio);
		}
	}
};
