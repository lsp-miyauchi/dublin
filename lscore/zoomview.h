// zzscrView.h : interface of the CZoomView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef __ZOOMVIEW_H__
#define __ZOOMVIEW_H__

#define USE_GDIPLUS_OPERATION

#ifdef USE_GDIPLUS_OPERATION
	#include "gdiplus_sup.h"
#else
	#include <vfw.h>
#endif

#include "scrollwindow.h"
#include "dib.h"

typedef enum {
	ZoomOff,
	ZoomIn,
	ZoomGrab,
	ZoomSel,
} ZoomMode_;

typedef enum {
	trackAny,
	trackVert,
	trackHorz,
	trackMove
} TrackMode_;

enum {
	htError,
	htNothing,
	htTopLeft,
	htTopRight,
	htBottomRight,
	htBottomLeft,
	htTop,
	htRight,
	htBottom,
	htLeft,
	htMiddle
};


class CZoomView : public CScrollWindowImpl<CZoomView>
{
public:
	virtual void OnFinalMessage(HWND /*hWnd*/)
	{
		delete this;
	}

	typedef CScrollWindowImpl<CZoomView>   _baseClass;

	CDib m_dib;
	Bitmap* m_gdipBitmap;
	HBITMAP m_hbm;
	double m_zoomScale;
	COLORREF m_crBkgnd;
	COLORREF m_crGrid;
	COLORREF m_crGuid;
	CSize m_sizeImage;
	CSize m_grid;
	int m_gridType;
	ZoomMode_  m_zoomMode;
	TrackMode_ m_trackMode;
	CRect m_rcSel;
	CRect m_rcTrack;
	CPoint m_ptLast;
	CPoint m_ptTrack;
	bool m_bCentered:1;
	bool m_bCaptured:1;
	bool m_bAutoSizeClient:1;
	int m_nDrawingMode;

	HCURSOR m_hcZoomIn;
	HCURSOR m_hcZoomOut;
	HCURSOR m_hcHandOpen;
	HCURSOR m_hcHandClose;
	HCURSOR m_hcTLtoBR;
	HCURSOR m_hcBLtoTR;
	HCURSOR m_hcVert;
	HCURSOR m_hcHorz;
	HCURSOR m_hcMove;
	HCURSOR m_hcSel;

#ifdef USE_GDIPLUS_OPERATION
	// gdi+
	gdiplus_using m_gdiplus_using;
#endif USE_GDIPLUS_OPERATION

	CZoomView();
	~CZoomView();

BEGIN_MSG_MAP(CZoomView)
	MESSAGE_HANDLER(WM_MOUSEWHEEL, OnHandlerMouseWheel)
	MESSAGE_HANDLER(WM_SIZE, OnHandlerSize)
	CHAIN_MSG_MAP(CScrollWindowImpl<CZoomView>)
	MESSAGE_HANDLER(WM_CREATE, OnHandlerCreate)
	MESSAGE_RANGE_HANDLER(WM_KEYFIRST, WM_KEYLAST, OnKeyMessages)
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_GETDLGCODE()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_KEYDOWN()
	ON_MESSAGE_VOID(WM_PALETTECHANGED, OnPaletteChanged)
	ON_MESSAGE(WM_QUERYNEWPALETTE, OnQueryNewPalette)
	MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	ON_WM_MOUSEACTIVATE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_MESSAGE(ZMVM_UPDATEIMAGE, OnHandlerUpdateImage)
	ON_MESSAGE(ZMVM_SETIMAGE, OnHandlerSetImage)
	ON_MESSAGE(ZMVM_GETIMAGE, OnHandlerGetImage)
	ON_MESSAGE(ZMVM_GETIMAGESIZE, OnHandlerGetImageSize)
	ON_MESSAGE(ZMVM_GETSCALE, OnHandlerGetScale)
	ON_MESSAGE(ZMVM_SETSCALE, OnHandlerSetScale)
	ON_MESSAGE(ZMVM_ZOOMIN, OnHandlerZoomIn)
	ON_MESSAGE(ZMVM_SETMODE, OnHandlerSetMode)
	ON_MESSAGE(ZMVM_GETMODE, OnHandlerGetMode)
	ON_MESSAGE(ZMVM_GETBKCOLOR, OnHandlerGetBkColor)
	ON_MESSAGE(ZMVM_SETBKCOLOR, OnHandlerSetBkColor)
	ON_MESSAGE_VOID(ZMVM_SETCLIPRECT, OnHandlerSetClipRect)
	ON_MESSAGE(ZMVM_GETCLIPRECT, OnHandlerGetClipRect)
	ON_MESSAGE_VOID(ZMVM_SCALEMENU, OnHandlerScaleMenu)
	ON_MESSAGE_VOID(ZMVM_SETGRID, OnHandlerSetGrid)
	ON_MESSAGE(ZMVM_GETGRID, OnHandlerGetGrid)
	ON_MESSAGE(ZMVM_SETINFO, OnHandlerSetInfo)
	ON_MESSAGE(ZMVM_GETINFO, OnHandlerGetInfo)
	ON_MESSAGE(ZMVM_CLIENTTODEVICE, OnHandlerClientToDevice)
	ON_MESSAGE(ZMVM_DEVICETOCLIENT, OnHandlerDeviceToClient)
	ON_MESSAGE(ZMVM_TRACKIMAGE, OnHandlerTrackImage)
	ON_MESSAGE(ZMVM_GETSCALEFACTORLIST, OnHandlerGetScaleFactorList)
	ON_MESSAGE_VOID(ZMVM_CENTERPOSITION, OnHandlerCenterPosition)
	ON_MESSAGE(ZMVM_SETDRAWMODE, OnHandlerSetDrawMode)
	ON_MESSAGE(ZMVM_GETDRAWMODE, OnHandlerGetDrawMode)
	ON_MESSAGE(ZMVM_CANZOOM, OnHandlerCanZoom)
	ON_MESSAGE(ZMVM_GETPIXELCOLOR, OnHandlerGetPixelColor)
END_MSG_MAP()

	LRESULT OnHandlerCreate(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnHandlerSize(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyMessages(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	BOOL OnEraseBkgnd(HDC hdc);
	UINT OnGetDlgCode() { return DLGC_WANTARROWS; }
	void OnDestroy();
	__inline void OnSetFocus(HWND hOldWnd) { ChangeFocus(TRUE); }
	__inline void OnKillFocus(HWND hNewWnd) { ChangeFocus(FALSE); }
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnPaletteChanged(WPARAM wParam, LPARAM lParam);
	LRESULT OnQueryNewPalette(WPARAM wParam, LPARAM lParam);
	LRESULT OnSetCursor(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnHandlerMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	int OnMouseActivate(HWND hDesktopWnd, UINT nHitTest, UINT message);
	void OnMouseMove(UINT nFlags, CPoint point);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnRButtonUp(UINT nFlags, CPoint point);
	LRESULT OnHandlerZoomIn(WPARAM wParam, LPARAM lParam);
	__inline LRESULT OnHandlerUpdateImage(WPARAM wParam, LPARAM lParam) { return (LRESULT)UpdateImage((HANDLE)lParam, (BOOL)wParam); }
	__inline LRESULT OnHandlerSetImage(WPARAM wParam, LPARAM lParam) { return (LRESULT)SetImage((HANDLE)lParam, (BOOL)wParam); }
	__inline LRESULT OnHandlerGetImage(WPARAM wParam, LPARAM lParam) { return (LRESULT)(HANDLE)m_dib; }
	LRESULT OnHandlerGetImageSize(WPARAM wParam, LPARAM lParam);
	__inline LRESULT OnHandlerGetScale(WPARAM wParam, LPARAM lParam) { return (LRESULT)((int)(GetZoomScale() * 1000)); }
	__inline LRESULT OnHandlerSetScale(WPARAM wParam, LPARAM lParam) { return (LRESULT)DoZoomScale((POINT*)lParam, (double)wParam / 1000.0); }
	__inline LRESULT OnHandlerSetMode(WPARAM wParam, LPARAM lParam) { SetZoomMode((ZoomMode_)wParam); return m_zoomMode; }
	__inline LRESULT OnHandlerGetMode(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_zoomMode; }
	__inline LRESULT OnHandlerGetBkColor(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_crBkgnd; }
	__inline LRESULT OnHandlerSetBkColor(WPARAM wParam, LPARAM lParam) { return (LRESULT)SetBkColor((BOOL)wParam, (COLORREF)lParam); }
	__inline LRESULT OnHandlerGetClipRect(WPARAM wParam, LPARAM lParam) { return GetClipRect((LPRECT)lParam); }
	__inline void OnHandlerSetClipRect(WPARAM wParam, LPARAM lParam) { SetClipRect((LPRECT)lParam); }
	void    OnHandlerScaleMenu(WPARAM wParam, LPARAM lParam);
	void    OnHandlerSetGrid(WPARAM wParam, LPARAM lParam);
	__inline LRESULT OnHandlerGetGrid(WPARAM wParam, LPARAM lParam) { return (LRESULT)MAKELONG(m_grid.cx, m_grid.cy); }
	__inline LRESULT OnHandlerSetInfo(WPARAM wParam, LPARAM lParam) { return (LRESULT)SetZoomViewInfo((ZOOMVIEWINFO*)lParam); }
	__inline LRESULT OnHandlerGetInfo(WPARAM wParam, LPARAM lParam) { return (LRESULT)GetZoomViewInfo((ZOOMVIEWINFO*)lParam); }
	__inline LRESULT OnHandlerClientToDevice(WPARAM wParam, LPARAM lParam) { return (LRESULT)ClientToDevice((LPPOINT)lParam, (int)wParam); }
	__inline LRESULT OnHandlerDeviceToClient(WPARAM wParam, LPARAM lParam) { return (LRESULT)DeviceToClient((LPPOINT)lParam, (int)wParam); }
	__inline LRESULT OnHandlerTrackImage(WPARAM wParam, LPARAM lParam) { return (LRESULT)TrackImage((HIMAGELIST)wParam, (LPPOINT)lParam); }
	__inline LRESULT OnHandlerGetScaleFactorList(WPARAM wParam, LPARAM lParam) { return (LRESULT)GetScaleFactorList((int*)lParam, (int)wParam); }
	void    OnHandlerCenterPosition(WPARAM wParam, LPARAM lParam);
	LRESULT OnHandlerSetDrawMode(WPARAM wParam, LPARAM lParam);
	__inline LRESULT OnHandlerGetDrawMode(WPARAM wParam, LPARAM lParam) { return (LRESULT)m_nDrawingMode; }
	LRESULT OnHandlerCanZoom(WPARAM wParam, LPARAM lParam);
	__inline LRESULT OnHandlerGetPixelColor(WPARAM wParam, LPARAM lParam) { return (LRESULT)GetPixelColor(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); }

	void ClearCachedImage();

	void DoScroll(int nType, int nScrollCode, int& cxyOffset, int cxySizeAll, int cxySizePage, int cxySizeLine);
	void DoPaint(CDCHandle dc);

	void GetDeviceScrollOffset(CPoint& pt);
	int HitTest(CPoint point);
	void DrawSelect(HDC hdc, CRect& rcSel);

	void PixelPointToScaledPoint(POINT* ppt, int cnt);
	__inline void PixelPointToScaledPoint(CPoint& rpt) { PixelPointToScaledPoint(&rpt, 1); }
	__inline void PixelPointToScaledPoint(CSize& rsize) { PixelPointToScaledPoint((POINT*)&rsize, 1); }
	__inline void PixelPointToScaledPoint(CRect& rrc) { PixelPointToScaledPoint((POINT*)&rrc, 2); }

	void ScaledPointToPixelPoint(POINT* ppt, int cnt);
	__inline void ScaledPointToPixelPoint(CPoint& rpt) { ScaledPointToPixelPoint(&rpt, 1); }
	__inline void ScaledPointToPixelPoint(CSize& rsize) { ScaledPointToPixelPoint((POINT*)&rsize, 1); }
	__inline void ScaledPointToPixelPoint(CRect& rrc) { ScaledPointToPixelPoint((POINT*)&rrc, 2); }

	void LogDev(POINT* ppt, int cnt);
	__inline void LogDev(CPoint& rpt) { LogDev(&rpt, 1); }
	__inline void LogDev(CSize& rsize) { LogDev((POINT*)&rsize, 1); }
	__inline void LogDev(CRect& rrc) { LogDev((POINT*)&rrc, 2); }

	void DevLog(POINT* ppt, int cnt);
	__inline void DevLog(CPoint& rpt) { DevLog(&rpt, 1); }
	__inline void DevLog(CSize& rsize) { DevLog((POINT*)&rsize, 1); }
	__inline void DevLog(CRect& rrc) { DevLog((POINT*)&rrc, 2); }

	HANDLE UpdateImage(HANDLE hdib, BOOL bRedraw);
	HANDLE SetImage(HANDLE hdib, BOOL bRedraw);
	COLORREF SetBkColor(BOOL bRedraw, COLORREF crNew);
	BOOL SetZoomViewInfo(ZOOMVIEWINFO* pzi);
	BOOL GetZoomViewInfo(ZOOMVIEWINFO* pzi);
	BOOL TrackImage(HIMAGELIST hImage, LPPOINT ppt);

	void NotifyScaleChanged();
	void NotifySelectionChanged();
	void NotifyPosChanged(CPoint& pt);		// point is device
	void NotifyVisibleChanged();

	CPoint GetCenterPoint(void);
	void CenterOnPoint(CPoint ptCenter);
	void ScrollToPosition(CPoint pt);
	void GetScrollLimit(CPoint& ptLimit);
	BOOL DoZoomScale(POINT* point, double scale);
	BOOL DoZoomIn(POINT* point = NULL);
	BOOL DoZoomOut(POINT* point = NULL);
	BOOL DoZoomDefault(POINT* point = NULL);
	BOOL DoZoomFitClient();
	BOOL DoZoomFitHorz();
	BOOL DoZoomFitVert();
	void SetZoomMode(ZoomMode_ zoomMode);
	double GetZoomScale();
	void ScaleMenu(CPoint pt);
	BOOL GetClipRect(RECT* prc);
	void SetClipRect(RECT* prc);
	int GetScaleFactorList(int* ppScale, int numMax);
	BOOL ClientToDevice(LPPOINT ppt, int cnt);
	BOOL DeviceToClient(LPPOINT ppt, int cnt);
	void ChangeFocus(BOOL bFocus);
	COLORREF GetPixelColor(int x, int y);
};


#endif			// __ZOOMVIEW_H__
