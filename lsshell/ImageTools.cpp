// ImageTools.cpp : CImageTools の実装

#include "stdafx.h"
#include "mainmodule.h"
#include "ImageTools.h"

// CImageTools

static bool ImageInClipboard(HWND hwnd)
{
  bool bClipboard = false;
  if (::OpenClipboard(hwnd))
  {
    if (IsClipboardFormatAvailable(CF_DIB) ||
        IsClipboardFormatAvailable(CF_BITMAP))
      bClipboard = true;
    ::CloseClipboard();
  }
  return bClipboard;
}

/////////////////////////////////////////////////////////////////////////////
// Methods of IShellExtInit

#include "..\lsimg\lsimg.h"
#include "..\lsimg\imgspecify.cpp"

void CImageTools::InsertCommandMenu(HMENU hmenu, UINT iInsertAfter, UINT idCommand, UINT idResource, UINT flags)
{
  CString str;
  str.LoadString(idResource);
  InsertMenu(hmenu, iInsertAfter, flags, idCommand, str);
  m_arrCommand.Add(idCommand, idResource);
}

void CImageTools::AppendCommandMenu(HMENU hmenu, UINT idCommand, UINT idResource, UINT flags)
{
  CString str;
  str.LoadString(idResource);
  AppendMenu(hmenu, flags, idCommand, str);
  m_arrCommand.Add(idCommand, idResource);
}

// Initialize
HRESULT CImageTools::Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdo, HKEY hProgID)
{
  if (pdo == NULL)
    return E_INVALIDARG;

  m_userlevel = _check_user();
  if (m_userlevel == USRL_NOUSER)
    return E_INVALIDARG;

  DWORD arc, etc;
  GetShellSetting(arc, etc, m_flags);
  if (!m_flags)
    return E_INVALIDARG;

  // Get the data as CF_HDROP
  STGMEDIUM sm;
  FORMATETC fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  if (FAILED(pdo->GetData(&fe, &sm)))
    return E_INVALIDARG;

  m_df.Attach((HDROP)DuplicateMemoryHandle(sm.hGlobal, GHND | GMEM_SHARE));
  ReleaseStgMedium(&sm);

  m_exif = false;
  m_imageoperation = false;
  m_clipboard = ImageInClipboard(GetActiveWindow());

  bool otherfile = false;
  WIN32_FILE_ATTRIBUTE_DATA ad;
  CString name;
  m_numJpeg = m_numPng = m_numTiff = m_numWebp = m_numAvif = 0;
  for (int i = 0; i < m_df.GetCount(); i++) {
    m_df.DragQueryFile(i, name);
    if (!GetFileAttributesEx(name, GetFileExInfoStandard, &ad) ||
        (ad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      otherfile = true;
      break;
    }

    int type = imgSpecifyImageType(name);
    if (type == IMG_NOTSUPPORTED)
      otherfile = true;
    else {
      m_imageoperation = true;
      if (imgTypeIsExif(type))
        m_exif = true;
      switch (type) {
		case IMG_JPEG:	m_numJpeg++; break;
		case IMG_PNG:	m_numPng++; break;
		case IMG_TIFF:	m_numTiff++; break;
		case IMG_WEBP:	m_numWebp++; break;
		default: break;
      }
    }
  }
  return (otherfile && !m_exif && !m_imageoperation && !m_clipboard) ? E_INVALIDARG : NOERROR;
}

HRESULT CImageTools::QueryContextMenu(HMENU hmenu, UINT index, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
  if (!IsValidExecuteModule())
    return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);

  // スタートメニューのプログラムメニュー内のフォルダメニューで
  // コンテキストメニューの動作が変だ
  if ((uFlags & ~CMF_RESERVED) == 0)
    return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
  if (uFlags & CMF_DEFAULTONLY)
    return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
  if (uFlags & CMF_VERBSONLY)
    return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);

  UINT mflg = MF_STRING | MF_BYPOSITION;
  if (m_userlevel == USRL_EXPIRE)
    mflg |= MF_GRAYED;

  InsertMenu(hmenu, index++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);

  UINT idCmd = idCmdFirst;
  m_idCmdFirst = idCmdFirst;

  CMenuItemInfo mi;
  mi.fMask = MIIM_SUBMENU | MIIM_TYPE | MIIM_ID;
  mi.fType = MFT_STRING;

  CString str;
  CMenu pop;
  pop.CreatePopupMenu();
  if (m_flags & SHLI_COPY)
  {
    if (m_imageoperation && m_df.GetCount() == 1)
      AppendCommandMenu(pop, idCmd++, IDM_IMG_COPYIMAGE, mflg);
    if (m_clipboard)
      AppendCommandMenu(pop, idCmd++, IDM_IMG_OPENCLIPBOARD, mflg);
  }
  if (m_imageoperation && m_flags & SHLI_CONVERT)
    AppendCommandMenu(pop, idCmd++, IDM_IMG_IMAGECONVERT, mflg);
  if (m_imageoperation && m_flags & SHLI_ALBUM)
    AppendCommandMenu(pop, idCmd++, IDM_IMG_WEBALBUM, mflg);
  if (m_numJpeg && m_flags & SHLI_JPEG2PDF)
    AppendCommandMenu(pop, idCmd++, IDM_IMG_JPEG2PDF, mflg);
  if (m_exif && (m_flags & SHLI_SLIDESHOW))
    AppendCommandMenu(pop, idCmd++, IDM_IMG_SLIDESHOW, mflg);
  if (m_exif && (m_flags & SHLI_THUMBOUT))
    AppendCommandMenu(pop, idCmd++, IDM_IMG_PICKOUTTHUMB, mflg);
  if (m_exif && (m_flags & SHLI_RENAME))
    AppendCommandMenu(pop, idCmd++, IDM_IMG_EXIFDATE, mflg);
  if (m_imageoperation && m_flags & SHLI_CROPIMAGE)
	AppendCommandMenu(pop, idCmd++, IDM_IMG_CROPIMAGE, mflg);
  if ((m_numJpeg || m_numPng || m_numTiff || m_numWebp || m_numAvif) && m_flags & SHLI_MYTOOLS) {
    AppendCommandMenu(pop, idCmd++, IDM_IMG_SPLITIMAGE, mflg);
    AppendCommandMenu(pop, idCmd++, IDM_IMG_CONTRASTRESIZE, mflg);
  }
  if (m_exif && m_df.GetCount() == 1)
    AppendCommandMenu(pop, idCmd++, IDM_IMG_GPS, mflg);

  if (m_exif && (m_flags & SHLI_ORIENTATION))
  {
    CMenu sub;
    sub.CreatePopupMenu();
    AppendCommandMenu(sub, idCmd++, IDM_IMG_ORI_NONE, mflg);
    AppendCommandMenu(sub, idCmd++, IDM_IMG_ORI_FLIPX, mflg);
    AppendCommandMenu(sub, idCmd++, IDM_IMG_ORI_ROTATE180, mflg);
    AppendCommandMenu(sub, idCmd++, IDM_IMG_ORI_FLIPY, mflg);
    AppendCommandMenu(sub, idCmd++, IDM_IMG_ORI_ROTATELEFTFLIPY, mflg);
    AppendCommandMenu(sub, idCmd++, IDM_IMG_ORI_ROTATERIGHT, mflg);
    AppendCommandMenu(sub, idCmd++, IDM_IMG_ORI_ROTATERIGHTFLIPY, mflg);
    AppendCommandMenu(sub, idCmd++, IDM_IMG_ORI_ROTATELEFT, mflg);
    str.LoadString(IDS_GROUP_EXIFORIENTATION);
    pop.AppendMenu(MF_SEPARATOR | MF_BYPOSITION, (UINT_PTR)0, (LPCTSTR)NULL);
    pop.AppendMenu(MF_POPUP, (UINT_PTR)sub.Detach(), str);
  }
  str.LoadString(IDS_GROUP_IMAGETOOLS);
  mi.dwTypeData = (LPTSTR)(LPCTSTR)str;
  mi.wID = idCmd++;
  mi.hSubMenu = pop.Detach();
  InsertMenuItem(hmenu, index++, TRUE, &mi);

  InsertMenu(hmenu, index++, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
  return MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_NULL, (idCmd - idCmdFirst) + 1);
}

// InvokeCommand
HRESULT CImageTools::InvokeCommand(LPCMINVOKECOMMANDINFO pci)
{
  if (HIWORD(pci->lpVerb) == 0)
  {
    UINT idResource = m_arrCommand.Lookup(LOWORD(pci->lpVerb) + m_idCmdFirst);

    HMODULE hmod;
    CString name;

    switch (idResource) {
      case IDM_IMG_WEBALBUM:
        GuidRunDll(pci->hwnd, 0, NULL, m_df, DLL_GMIL, _T("RunWebAlbum"));
        break;
      case IDM_IMG_IMAGECONVERT:
        GuidRunDll(pci->hwnd, 0, NULL, m_df, DLL_GMIL, _T("RunConvertImage"));
        break;
      case IDM_IMG_JPEG2PDF:
        GuidRunDll(pci->hwnd, 0, NULL, m_df, DLL_GMIL, _T("RunJpeg2Pdf"));
        break;
      case IDM_IMG_SPLITIMAGE:
        GuidRunDll(pci->hwnd, 0, NULL, m_df, DLL_GMIL, _T("RunSplitImage"));
        break;
      case IDM_IMG_CONTRASTRESIZE:
        GuidRunDll(pci->hwnd, 0, NULL, m_df, DLL_GMIL, _T("RunContrastResize"));
        break;
      case IDM_IMG_CROPIMAGE:
        GuidRunDll(pci->hwnd, 0, NULL, m_df, DLL_GMIL, _T("RunCropImage"));
        break;
      case IDM_IMG_COPYIMAGE: {
        hmod = LoadMyLibrary(DLL_IMAGE); // lsimg.dll
        if (!hmod)
          return E_FAIL;

        m_df.DragQueryFile(0, name);

        BOOL(WINAPI * pfn)
        (HWND, LPCTSTR);
        (FARPROC &)pfn = GetProcAddress(hmod, "imgCopyImageToClipboard");
        if (pfn)
          (*pfn)(pci->hwnd, name);
        FreeLibrary(hmod);
        break;
      }
      case IDM_IMG_GPS: {
        hmod = LoadMyLibrary(DLL_FOPSRV); // lsfop.dll
        if (!hmod)
          return E_FAIL;

        m_df.DragQueryFile(0, name);

        BOOL(WINAPI * pfn)
        (HWND, LPCTSTR);
        (FARPROC &)pfn = GetProcAddress(hmod, "fopGpsAccess");
        if (pfn)
          (*pfn)(pci->hwnd, name);
        FreeLibrary(hmod);
        break;
      }

      case IDM_IMG_ORI_NONE:
      case IDM_IMG_ORI_FLIPX:
      case IDM_IMG_ORI_ROTATE180:
      case IDM_IMG_ORI_FLIPY:
      case IDM_IMG_ORI_ROTATELEFTFLIPY:
      case IDM_IMG_ORI_ROTATERIGHT:
      case IDM_IMG_ORI_ROTATERIGHTFLIPY:
      case IDM_IMG_ORI_ROTATELEFT: {
        hmod = LoadMyLibrary(DLL_IMAGE); // lsimg.dll
        if (!hmod)
          return E_FAIL;

        name.LoadString(idResource);
        CString str;
        str.LoadString(IDS_QUERY_EXIFORIENTATION);
        str.Replace(_T("%1"), name);
        name.LoadString(IDS_GROUP_EXIFORIENTATION);
        if (MessageBox(pci->hwnd, str, name, MB_YESNO | MB_ICONQUESTION) != IDYES)
          return E_FAIL;

        BOOL(WINAPI * setproc)
        (LPCTSTR, int);
        (FARPROC &)setproc = GetProcAddress(hmod, "imgJpgSetOrientation");
        if (setproc) {
          int or = (idResource - IDM_IMG_ORI_NONE + 1);
          for (int i = 0; i < m_df.GetCount(); i++) {
            m_df.DragQueryFile(i, name);
            (*setproc)(name, or);
          }
        }
        FreeLibrary(hmod);
        break;
      }
      case IDM_IMG_OPENCLIPBOARD:
        RunDll(pci->hwnd, DLL_GMIL, _T("pxfRun"), _T("/clip"));
        break;
      case IDM_IMG_EXIFDATE:
        GuidRunDll(pci->hwnd, 0, NULL, m_df, DLL_GMIL, _T("RunRenameToExifDate"));
        break;
      case IDM_IMG_PICKOUTTHUMB:
        GuidRunDll(pci->hwnd, 0, NULL, m_df, DLL_GMIL, _T("RunPickoutThumbnail"));
        break;
      case IDM_IMG_SLIDESHOW:
        m_df.DragQueryFile(0, name);
        RunDll(pci->hwnd, DLL_GMIL, _T("slideRun"), name);
        break;
    }
  }
  return S_OK;
}

// GetCommandString
HRESULT CImageTools::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT *pwReserved, LPSTR pszText, UINT cchMax)
{
  return S_OK;
}
