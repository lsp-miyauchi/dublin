/////////////////////////////////////////////////////////////////////////////
// CZoomViewCtrl

#pragma once
#ifndef __lcc_include_zoomviewctrl_h__
#define __lcc_include_zoomviewctrl_h__


template <class TBase>
class CZoomViewCtrlT : public TBase
{
public:
// Constructors
	CZoomViewCtrlT(HWND hWnd = NULL) : TBase(hWnd) { }

	CZoomViewCtrlT< TBase >& operator=(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, _U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return CWindow::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return WC_LSC_ZOOMVIEW;
	}

	void SetClipRect(LPRECT prc)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, ZMVM_SETCLIPRECT, 0, (LPARAM)prc);
	}
	BOOL GetClipRect(LPRECT prc)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, ZMVM_GETCLIPRECT, 0, (LPARAM)prc);
	}
	BOOL SetZoomViewInfo(ZOOMVIEWINFO* pzi)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, ZMVM_SETINFO, 0, (LPARAM)pzi);
	}
	BOOL GetZoomViewInfo(ZOOMVIEWINFO* pzi)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, ZMVM_GETINFO, 0, (LPARAM)pzi);
	}
	int SetMode(int mode)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, ZMVM_SETMODE, (WPARAM)mode, 0);
	}
	int GetMode()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, ZMVM_GETMODE, 0, 0);
	}
	int GetScale()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, ZMVM_GETSCALE, 0, 0);
	}
	BOOL SetScale(int scale, POINT* ppt = NULL)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, ZMVM_SETSCALE, (WPARAM)scale, (LPARAM)ppt);
	}
	void ScaleMenu(LPPOINT ppt)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		SendMessage(m_hWnd, ZMVM_SCALEMENU, 0, (LPARAM)ppt);
	}
	void ZoomIn()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, ZMVM_ZOOMIN, DOZOOM_IN, 0);
	}
	void ZoomOut()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, ZMVM_ZOOMIN, DOZOOM_OUT, 0);
	}
	void ZoomDefault()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, ZMVM_ZOOMIN, DOZOOM_DEFAULT, 0);
	}
	void ZoomFull()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, ZMVM_ZOOMIN, DOZOOM_FULL, 0);
	}
	void ZoomFitVert()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, ZMVM_ZOOMIN, DOZOOM_FITVERT, 0);
	}
	void ZoomFitHorz()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, ZMVM_ZOOMIN, DOZOOM_FITHORZ, 0);
	}
	void ShowGrid(BOOL bShow)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, ZMVM_SETGRID, (WPARAM)bShow, 0);
	}
	void ToggleGrid()
	{
		ShowGrid((GetStyle() & ZMVS_GRID) ? FALSE : TRUE);
	}
	HANDLE UpdateImage(HANDLE hdib, BOOL bRedraw)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HANDLE)::SendMessage(m_hWnd, ZMVM_UPDATEIMAGE, (WPARAM)bRedraw, (LPARAM)hdib);
	}
	HANDLE SetImage(HANDLE hdib, BOOL bRedraw)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HANDLE)::SendMessage(m_hWnd, ZMVM_SETIMAGE, (WPARAM)bRedraw, (LPARAM)hdib);
	}
	HANDLE GetImage()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HANDLE)::SendMessage(m_hWnd, ZMVM_GETIMAGE, 0, 0);
	}
	BOOL GetImageSize(SIZE* psize)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, ZMVM_GETIMAGESIZE, 0, (LPARAM)psize);
	}
	COLORREF GetBkColor()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, ZMVM_GETBKCOLOR, 0, 0);
	}
	COLORREF SetBkColor(COLORREF clr, BOOL bRedraw)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, ZMVM_SETBKCOLOR, (WPARAM)bRedraw, (LPARAM)clr);
	}
	int GetZoomMode()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, ZMVM_GETMODE, 0, 0);
	}
	int SetZoomMode(int mode)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, ZMVM_SETMODE, (WPARAM)mode, 0);
	}
	BOOL ClientToDevice(LPPOINT ppt, int cnt)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, ZMVM_CLIENTTODEVICE, (WPARAM)cnt, (LPARAM)ppt);
	}
	BOOL DeviceToClient(LPPOINT ppt, int cnt)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, ZMVM_DEVICETOCLIENT, (WPARAM)cnt, (LPARAM)ppt);
	}
	BOOL TrackImage(HIMAGELIST hImage, LPPOINT ppt)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, ZMVM_TRACKIMAGE, (WPARAM)hImage, (LPARAM)ppt);
	}

	int GetScaleFactorList(int* pFactorList, int numMax)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, ZMVM_GETSCALEFACTORLIST, (WPARAM)numMax, (LPARAM)pFactorList);
	}

	void CenterPosition()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, ZMVM_CENTERPOSITION, 0, 0);
	}

	int GetDrawMode()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, ZMVM_GETDRAWMODE, 0, 0);
	}

	int SetDrawMode(int mode)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, ZMVM_SETDRAWMODE, (WPARAM)mode, 0);
	}

	void UseGdiPlus(bool bUse)
	{
		SetDrawMode(bUse ? ZVDM_GDIPLUS : ZVDM_BITMAP);
	}

	void UseDirect2D(bool bUse)
	{
		SetDrawMode(bUse ? ZVDM_DIRECT2D : ZVDM_BITMAP);
	}

	DWORD CanZoom()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, ZMVM_CANZOOM, 0, 0);
	}

	COLORREF GetPixelColor(int x, int y)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (COLORREF)::SendMessage(m_hWnd, ZMVM_GETPIXELCOLOR, 0, MAKELPARAM(x, y));
	}

	__inline COLORREF GetPixelColor(POINT pt)
	{
		return GetPixelColor(pt.x, pt.y)
	}
};

typedef CZoomViewCtrlT<CWindow>	CZoomViewCtrl;


#endif		// __lcc_include_zoomviewctrl_h__
