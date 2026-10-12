//////////////////////////////////////////////////////////////////////
// tbb_customdraw.h

#pragma once
#ifndef __TBB_CUSTOMDRAW_H__
#define __TBB_CUSTOMDRAW_H__

#ifndef __XWIN_H__
#include "xwin.h"
#endif

#include "buttonframe.h"


////////////////////////////////////////////////////////////////////////
//

typedef struct _IMAGELIST_ATTRIB {
  HIMAGELIST him;
  HIMAGELIST himDisabled;
  HIMAGELIST himHot;
  int cxImage;
  int cyImage;
  bool bHasAlpha;
} IMAGELIST_ATTRIB;

////////////////////////////////////////////////////////////////////////
//

template <class T>
class CTBarCustomDrawImpl
{
private:
    bool m_bTBarDropDownProcessing:1;
    DWORD m_TBarCommVer;
    DWORD m_TBarOsVer;
    CSimpleMap<HWND, IMAGELIST_ATTRIB> m_mapTBarImageAttrib;

public:
    CLspDrawButton m_buttonDrawer;
    
    CTBarCustomDrawImpl()
    : m_bTBarDropDownProcessing(false)
    , m_TBarCommVer(0)
    , m_TBarOsVer(0)
  {
  }

BEGIN_MSG_MAP(CTBarCustomDrawImpl)
  MESSAGE_HANDLER(WM_NOTIFY, OnHandlerTBarDrawNotify)
END_MSG_MAP()

  LRESULT OnHandlerTBarDrawNotify(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
  {
    NMHDR* pNmhdr = (NMHDR*)lParam;
    if (!IsWindow(pNmhdr->hwndFrom) || m_mapTBarImageAttrib.GetSize() < 1 || m_mapTBarImageAttrib.FindKey(pNmhdr->hwndFrom) < 0) {
      bHandled = FALSE;
      return 0;
    }

    LRESULT lr = 0;
    T* pT = static_cast<T*>(this);
    switch (pNmhdr->code) {
    case TBN_DROPDOWN:
    {
      SetDropDownProcessing(true);
      CRect rc;
      ::SendMessage(pNmhdr->hwndFrom, TB_GETRECT, ((TBNOTIFY*)pNmhdr)->iItem, (LPARAM)&rc);
      ::RedrawWindow(pNmhdr->hwndFrom, &rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

      lr = pT->DoToolBarDropDown((TBNOTIFY*)pNmhdr);
      SetDropDownProcessing(false);
      ::RedrawWindow(pNmhdr->hwndFrom, &rc, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
      break;
    }
    case NM_CUSTOMDRAW:
      ProcessToolbarCustomDraw(pNmhdr, &lr);
      break;
    default:
      bHandled = FALSE;
      break;
    }
    return lr;
  }

  inline void SetDropDownProcessing(bool bDropDownProcessing)
  {
    m_bTBarDropDownProcessing = bDropDownProcessing;
  }

  void AddCustomDrawBar(HWND hbar)
  {
    int index = m_mapTBarImageAttrib.FindKey(hbar);
    if (index < 0) {
      IMAGELIST_ATTRIB ia;
      memset(&ia, 0, sizeof(ia));
      ia.him = (HIMAGELIST)::SendMessage(hbar, TB_GETIMAGELIST, 0, 0);
      ia.himDisabled = (HIMAGELIST)::SendMessage(hbar, TB_GETDISABLEDIMAGELIST, 0, 0);
      if (ia.him) {
        BITMAP bm;
#ifdef _WIN64
        IMAGEINFO info;
        ImageList_GetImageInfo(ia.him, 0, &info);
        GetObject(info.hbmImage, sizeof(bm), &bm);
        ia.bHasAlpha = (info.hbmMask == NULL ) && (bm.bmBitsPixel == 32);
#else
        DWORD* pdw = (DWORD*)GlobalLock((HGLOBAL)ia.him);
        if (pdw) {
          if (*pdw == IMAGELIST_MAGIC_V1) {
            GetObject(((IMAGELISTSTRUCT_V1*)pdw)->hbmImage, sizeof(bm), &bm);
            ia.bHasAlpha = (bm.bmBitsPixel == 32);
          } else if (*pdw == IMAGELIST_MAGIC_V2) {
            GetObject(((IMAGELISTSTRUCT_V2*)pdw)->hbmImage, sizeof(bm), &bm);
            ia.bHasAlpha = (bm.bmBitsPixel == 32);
          } else {
            GetObject(((IMAGELISTSTRUCT_V3*)pdw)->hbmImage, sizeof(bm), &bm);
            ia.bHasAlpha = (bm.bmBitsPixel == 32);
          }
          GlobalUnlock((HGLOBAL)ia.him);
        }
#endif	// _WIN64
        ImageList_GetIconSize(ia.him, &ia.cxImage, &ia.cyImage);
      }
      m_mapTBarImageAttrib.Add(hbar, ia);
    }
  }

  void RemoveCustomDrawBar(HWND hbar)
  {
    int index = m_mapTBarImageAttrib.FindKey(hbar);
    if (index >= 0)
      m_mapTBarImageAttrib.RemoveAt(index);
  }

  void ProcessToolbarCustomDraw(NMHDR* pNmhdr, LRESULT* pResult)
  {
    LPNMTBCUSTOMDRAW pcd = (LPNMTBCUSTOMDRAW)pNmhdr;
    switch (pcd->nmcd.dwDrawStage) {
      case CDDS_PREPAINT:
        OnPrepaint(pcd, pResult);
        break;
      case CDDS_ITEMPREPAINT:
        OnItemPrepaint(pcd, pResult);
        break;
      case CDDS_ITEMPOSTPAINT:
        OnItemPostpaint(pcd, pResult);
        break;
    }
  }

  void OnPrepaint(LPNMTBCUSTOMDRAW pcd, LRESULT* pResult)
  {
    T* pT = static_cast<T*>(this);
    *pResult = pT->OnCustomPrepaint(pcd);
  }

  void OnItemPrepaint(LPNMTBCUSTOMDRAW pcd, LRESULT* pResult)
  {
    DrawItem(pcd, pResult);

    T* pT = static_cast<T*>(this);
    *pResult = pT->OnCustomItemPrepaint(pcd);
  }

  /**
   * 
   */
  void DrawItem(LPNMTBCUSTOMDRAW pcd, LRESULT* pResult)
  {
    if ((int)pcd->nmcd.dwItemSpec < 0) {
      // セパレータ?
      *pResult = CDRF_DODEFAULT;
      return;
    }

    CWindow bar = pcd->nmcd.hdr.hwndFrom;

    UINT state = pcd->nmcd.uItemState;
    CDCHandle dc = pcd->nmcd.hdc;

    TBBUTTONINFO tbbi = { 0 };
    tbbi.cbSize = sizeof(TBBUTTONINFO);
    tbbi.dwMask = TBIF_STATE | TBIF_STYLE | TBIF_IMAGE;
    int button_index = (int)bar.SendMessage(TB_GETBUTTONINFO, (WPARAM)pcd->nmcd.dwItemSpec, (LPARAM)&tbbi);
    if (button_index < 0) {
      *pResult = CDRF_DODEFAULT;
      return;
    }
    // ボタンラベル取得
    CString strLabel;
    if (bar.GetStyle() & TBSTYLE_LIST) {
      TBBUTTON tbb = { 0 };
      if (bar.SendMessage(TB_GETBUTTON, button_index, (LPARAM)&tbb)) {
        if (IS_INTRESOURCE(tbb.iString)) {
          TCHAR buf[200];
          if (bar.SendMessage(TB_GETSTRING, MAKEWPARAM(ARRAYSIZE(buf), tbb.iString), (LPARAM)buf) > 0)
            strLabel = buf;
        }
        else
          strLabel = (LPCTSTR)tbb.iString;
      }
    }

    // 押下中フラグ
    if (tbbi.fsState & TBSTATE_PRESSED) {
      state |= CDIS_PRESSED;
    }

    CRect rc = pcd->nmcd.rc;
    CSize sizeButton((DWORD)bar.SendMessage(TB_GETBUTTONSIZE, 0, 0));
    // ドロップダウン
    int cxDropdown = 0;
    if (tbbi.fsStyle & BTNS_DROPDOWN) {
      CRect rcdd;
      bar.SendMessage(TB_GETITEMDROPDOWNRECT, (WPARAM)button_index, (LPARAM)&rcdd);
      cxDropdown = rcdd.Width();
      if (cxDropdown == 0)
        cxDropdown = rc.Width() - sizeButton.cx;
    }

    if (!(state & CDIS_DISABLED)) {
      if (m_bTBarDropDownProcessing && (tbbi.fsStyle & (BTNS_DROPDOWN | BTNS_WHOLEDROPDOWN))) {
        state |= CDIS_PRESSED;
        m_buttonDrawer.Draw(dc, rc, state, ROUND_TOP);
      } else if ((tbbi.fsStyle & BTNS_DROPDOWN) && (!(state & CDIS_CHECKED) || (state & CDIS_HOT))) {
        // Dropdown右側
        rc.left = rc.right - cxDropdown;
        m_buttonDrawer.Draw(dc, rc, state, ROUND_RIGHT);

        // 現在のクリップリージョンを取得しておく
        CRgn rgn;
        dc.GetClipRgn(rgn);

        // 右側は描いたので対象から除く
        CRect rcx = rc;
        dc.ExcludeClipRect(&rcx);

        // Dropdown左側
        rc.right = rc.left + 1;
        rc.left = pcd->nmcd.rc.left;
        m_buttonDrawer.Draw(dc, rc, state, ROUND_LEFT);

        // Exludeする前のリージョンに戻す
        dc.SelectClipRgn(rgn);
      } else {
        m_buttonDrawer.Draw(dc, rc, state);
      }
    }

    const int cxyButtonMargin = 7;

    rc = pcd->nmcd.rc;
    if (tbbi.fsStyle & BTNS_DROPDOWN) {
      rc.right -= (cxDropdown - 1);
    } else if (tbbi.fsStyle & BTNS_WHOLEDROPDOWN) {
      rc.right -= cxyButtonMargin;
    }
    CRect rct = rc;				// ラベル表示用
    int mapindex = m_mapTBarImageAttrib.FindKey(bar.m_hWnd);
    if (mapindex >= 0) {
      IMAGELIST_ATTRIB ia = m_mapTBarImageAttrib.GetValueAt(mapindex);
      if (ia.him) {
        CPoint pt;
        if (!strLabel.IsEmpty()) {
          pt.x = rc.left + cxyButtonMargin / 2;
          rct.left += cxyButtonMargin + ia.cxImage;
        } else {
          pt.x = (rc.left + rc.right - ia.cxImage) / 2;
        }
        pt.y = rc.top + (sizeButton.cy - ia.cyImage) / 2;

        if (state & (CDIS_CHECKED | CDIS_PRESSED)) {
          pt.x++;
        }

        T* pT = static_cast<T*>(this);
        int iOverlayImage = pT->QeuryOverlayImageIndex((UINT)pcd->nmcd.dwItemSpec);
        tbd_draw_image(dc, &ia, tbbi.iImage, iOverlayImage, pt, state);
      }
    }

    int mode = dc.SetBkMode(TRANSPARENT);

    HFONT hFont;
    if (!strLabel.IsEmpty()) {
      if (state & (CDIS_CHECKED | CDIS_PRESSED)) {
        rct.OffsetRect(1, 0);
      }
      hFont = dc.SelectFont(bar.GetFont());
      dc.DrawText(strLabel, -1, &rct, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
      dc.SelectFont(hFont);
    }

    if (tbbi.fsStyle & (BTNS_DROPDOWN | BTNS_WHOLEDROPDOWN)) {
      COLORREF clr = dc.SetTextColor(GetSysColor((state & CDIS_DISABLED) ? COLOR_GRAYTEXT : COLOR_BTNTEXT));

      int check_height = 11;
      hFont = CreateFont(check_height, 0, 0, 0, FW_NORMAL, 0, 0, 0, SYMBOL_CHARSET, 0, 0, 0, 0, _T("Marlett"));
      hFont = dc.SelectFont(hFont);
      rc = pcd->nmcd.rc;
      rc.left = cxDropdown ? (rc.right - cxDropdown) : (rc.right - cxyButtonMargin);
      if (tbbi.fsStyle & BTNS_WHOLEDROPDOWN) {
        rc.OffsetRect((state & (CDIS_CHECKED | CDIS_PRESSED)) ? -2 : -3, 0);
      }
      TCHAR ch = _T('6');
      dc.DrawText(&ch, 1, &rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

      DeleteObject(dc.SelectFont(hFont));
      dc.SetTextColor(clr);
    }

    dc.SetBkMode(mode);
  }

  void OnItemPostpaint(LPNMTBCUSTOMDRAW pcd, LRESULT* pResult)
  {
    T* pT = static_cast<T*>(this);
    *pResult = pT->OnCustomItemPostpaint(pcd);
  }

  // overridable
  virtual LRESULT OnCustomPrepaint(LPNMTBCUSTOMDRAW pcd)
  {
    return CDRF_NOTIFYITEMDRAW;
  }

  // overridable
  virtual LRESULT OnCustomItemPrepaint(LPNMTBCUSTOMDRAW pcd)
  {
    return CDRF_SKIPDEFAULT;
  }

  // overridable
  virtual LRESULT OnCustomItemPostpaint(LPNMTBCUSTOMDRAW pcd)
  {
    return CDRF_DODEFAULT;
  }

  // overridable
  virtual int DoToolBarDropDown(TBNOTIFY* pTBNotify)
  {
    return TBDDRET_NODEFAULT;
  }

  // query
  virtual int QeuryOverlayImageIndex(UINT id)
  {
    return 0;
  }

private:
  void tbd_init_commver()
  {
    if (m_TBarCommVer == 0) {
      DWORD major, minor;
      if (SUCCEEDED(AtlGetCommCtrlVersion(&major, &minor)))
        m_TBarCommVer = MAKELONG(minor, major);
      else
        m_TBarCommVer = MAKELONG(0, 4);
    }
  }

  bool tbd_IsCommVer6()
  {
    tbd_init_commver();
    return (m_TBarCommVer >= MAKELONG(0, 6));
  }

  void tbd_init_osver()
  {
    if (m_TBarOsVer == 0) {
      typedef struct _OSVERSION_DETECT { DWORD major; DWORD minor; } OSVERSION_DETECT;
      OSVERSION_DETECT oda[] = { {5,1}, {6,0}, {6.1}, {6,2}, {6,3}, {10,0} };

      OSVERSIONINFOEX osv = {0};
      osv.dwOSVersionInfoSize = sizeof(osv);
      DWORDLONG cdm = 0;
      VER_SET_CONDITION(cdm, VER_MAJORVERSION, VER_GREATER_EQUAL);
      VER_SET_CONDITION(cdm, VER_MINORVERSION, VER_GREATER_EQUAL);
      for (int i = 0; i < ARRAYSIZE(oda); i++) {
        osv.dwMajorVersion = oda[i].major;
        osv.dwMinorVersion = oda[i].minor;
        if (!VerifyVersionInfo(&osv, VER_MAJORVERSION | VER_MINORVERSION, cdm)) break;
        m_TBarOsVer = MAKELONG(oda[i].minor, oda[i].major);
      }
    }
  }

  bool tbd_IsOsXP()
  {
    tbd_init_osver();
    return (m_TBarOsVer >= MAKELONG(1, 5));
  }

  bool tbd_IsOsVISTA()
  {
    tbd_init_osver();
    return (m_TBarOsVer >= MAKELONG(0, 6));
  }

  void tbd_draw_image(HDC hdc, IMAGELIST_ATTRIB* pia, int iImage, int iOverlayImage, POINT pt, UINT state)
  {
    const DWORD MAGICROP_CODE = 0xb8074a;

    UINT style = ILD_TRANSPARENT;
    if (iOverlayImage > 0)
      style |= INDEXTOOVERLAYMASK(iOverlayImage);
    if (state & CDIS_HOT && pia->himHot) {
      ImageList_Draw(pia->himHot, iImage, hdc, pt.x, pt.y, style);
    } else if (state & CDIS_DISABLED) {
      if (pia->himDisabled) {
        ImageList_Draw(pia->himDisabled, iImage, hdc, pt.x, pt.y, style);
      } else if (tbd_IsCommVer6() && pia->bHasAlpha) {
        IMAGELISTDRAWPARAMS ildp;
        memset(&ildp, 0, sizeof(ildp));
        ildp.cbSize = sizeof(IMAGELISTDRAWPARAMS);
        ildp.himl = pia->him;
        ildp.i = iImage;
        ildp.hdcDst = hdc;
        ildp.x = pt.x;
        ildp.y = pt.y;
        ildp.cx = 0;
        ildp.cy = 0;
        ildp.xBitmap = 0;
        ildp.yBitmap = 0;
        ildp.fStyle = style;
//				ildp.fState = ILS_SATURATE;
//				ildp.Frame = 0;
        ildp.fState = ILS_SATURATE | ILS_ALPHA;
        ildp.Frame = 80;
        ::ImageList_DrawIndirect(&ildp);
      } else {
        // カラービットマップの作成
        HBITMAP hbm = CreateBitmap(pia->cxImage, pia->cyImage, 1, 1, NULL);

        // メモリDCへ描画 -- 背景は先に白で埋める
        HDC hmemdc = CreateCompatibleDC(hdc);
        HGDIOBJ hbmOld = SelectObject(hmemdc, hbm);
        PatBlt(hmemdc, 0, 0, pia->cxImage, pia->cyImage, WHITENESS);
        ImageList_Draw(pia->him, iImage, hmemdc, 0, 0, ILD_TRANSPARENT);

        // これを先にやんなきゃダメ。 でもなぜ？
        COLORREF rgb = SetBkColor(hdc, RGB(255, 255, 255));

        // Draw using hilite offset by (1,1), then shadow
        HBRUSH hbrShadow = CreateSolidBrush(GetSysColor(COLOR_3DSHADOW));
        HBRUSH hbrHilite = CreateSolidBrush(GetSysColor(COLOR_3DHIGHLIGHT));
        HGDIOBJ hbrOld = SelectObject(hdc, hbrHilite);
        BitBlt(hdc, pt.x + 1, pt.y + 1, pia->cxImage, pia->cyImage, hmemdc, 0, 0, MAGICROP_CODE);
        SelectObject(hdc, hbrShadow);
        BitBlt(hdc, pt.x, pt.y, pia->cxImage, pia->cyImage, hmemdc, 0, 0, MAGICROP_CODE);
        SelectObject(hdc, hbrOld);

        DeleteObject(hbrShadow);
        DeleteObject(hbrHilite);

        SetBkColor(hdc, rgb);					// restore
        SelectObject(hmemdc, hbmOld);			// ...
        DeleteObject(hbm);
        DeleteDC(hmemdc);
      }
    } else {
      ImageList_Draw(pia->him, iImage, hdc, pt.x, pt.y, style);
    }
  }
};


#endif			// __TBB_CUSTOMDRAW_H__
