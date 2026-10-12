// Windows Template Library - WTL version 10.0
// Copyright (C) Microsoft Corporation, WTL Team. All rights reserved.
//
// This file is a part of the Windows Template Library.
// The use and distribution terms for this software are covered by the
// Microsoft Public License (http://opensource.org/licenses/MS-PL)
// which can be found in the file MS-PL.txt at the root folder.

#include <atlctrlw.h>

#ifndef __MENUBAR_H__
#define __MENUBAR_H__

#pragma once

#ifndef __ATLAPP_H__
  #error menubar.h requires atlapp.h to be included first
#endif

#ifndef __ATLCTRLS_H__
  #error menubar.h requires atlctrls.h to be included first
#endif


///////////////////////////////////////////////////////////////////////////////
//

namespace WTL
{

class CMenuBarCtrl : public CCommandBarCtrlImpl<CMenuBarCtrl>
{
public:
  DECLARE_WND_SUPERCLASS(_T("lccMenuBar"), GetWndClassName())

  // K.Miyauchi lccLoadImageList を利用して RT_RCDATA にある PNGイメージを イメージリストとして読み込む
  BOOL LoadMultipleImages(ATL::_U_STRINGorID image, bool bUseInvert = false)
  {
    ATLASSERT(::IsWindow(m_hWnd));
    HINSTANCE hInstance = ModuleHelper::GetResourceInstance();

    HRSRC hRsrc = ::FindResource(hInstance, image.m_lpstr, (LPTSTR)RT_TOOLBAR);
    if (hRsrc == NULL) return FALSE;

    HGLOBAL hGlobal = ::LoadResource(hInstance, hRsrc);
    if (hGlobal == NULL) return FALSE;

    _ToolBarData* pData = (_ToolBarData*)::LockResource(hGlobal);
    if(pData == NULL) return FALSE;
    ATLASSERT(pData->wVersion == 1);

    WORD* pItems = pData->items();
    int nItems = pData->wItemCount;

    // Set internal data
    SetImageSize(pData->wWidth, pData->wHeight);

    // Create image list if needed
    HIMAGELIST him = bUseInvert ?
		lccLoadAlphaImageList(hInstance, RT_RCDATA, image.m_lpstr, pData->wWidth, TRUE) :
		lccLoadImageList(hInstance, RT_RCDATA, image.m_lpstr, pData->wWidth, CLR_DEFAULT);
    if (!him) return FALSE;

    bool bImageListAlreadyExists = (m_hImageList != NULL);
#if _WTL_CMDBAR_VISTA_MENUS
    int nOldImageCount = bImageListAlreadyExists ? ::ImageList_GetImageCount(m_hImageList) : 0;
#endif // _WTL_CMDBAR_VISTA_MENUS
    if (!bImageListAlreadyExists) {
      IMAGEINFO info;
      ImageList_GetImageInfo(him, 0, &info);
      BITMAP bm;
      ::GetObject(info.hbmImage, sizeof(bm), &bm);
      m_bAlphaImages = (bm.bmBitsPixel == 32);
      m_hImageList = him;
    }

    // Fill the array with command IDs
    int i;
    for (i = 0; i < nItems; i++) {
      if (pItems[i] != 0)
        m_arrCommand.Add(pItems[i]);
    }

    // Add bitmap to our image list
    if (bImageListAlreadyExists) {
      int dstCount = ImageList_GetImageCount(him);
      for (i = 0; i < dstCount; i++) {
        HICON hic = ImageList_GetIcon(him, i, ILD_TRANSPARENT);
        ImageList_ReplaceIcon(m_hImageList, -1, hic);
        DestroyIcon(hic);
      }
      ImageList_Destroy(him);
    }

    int nImageCount = ::ImageList_GetImageCount(m_hImageList);
    ATLASSERT(nImageCount == m_arrCommand.GetSize());
    if (nImageCount != m_arrCommand.GetSize()) return FALSE;

#if _WTL_CMDBAR_VISTA_MENUS
    if (RunTimeHelper::IsVista()) {
      _AddVistaBitmapsFromImageList(nOldImageCount, nImageCount - nOldImageCount);
      ATLASSERT(nImageCount == m_arrVistaBitmap.GetSize());
    }
#endif // _WTL_CMDBAR_VISTA_MENUS
    return TRUE;
  }

};

}		// namespace WTL

#endif	// __MENUBAR_H__
