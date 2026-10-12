// Save.cpp : インプリメンテーション ファイル
//

#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "save.h"
#include "exifinfo.h"
#include "jpegoption.h"
#include "webpoption.h"
#include "avif_option.h"


/////////////////////////////////////////////////////////////////////
//

BOOL SaveFileBrowse(HWND hWnd, LPTSTR pszName, int* ptype)
{
    OPENFILENAME ofn;
    TCHAR szFileName[MAX_PATH * 2], szFileTitle[MAX_PATH];
    TCHAR szFilter[MAX_PATH], szInitialDir[MAX_PATH];
    lstrcpy(szFileName, pszName);
    if (*pszName) {
        lstrcpy(szInitialDir, pszName);
        PathRemoveFileSpec(szInitialDir);
    } else {
        CAppReg reg;
        TCHAR buf[MAX_PATH * 2];
        if ((reg.GetText(IDS_SEC_SAVE, IDS_KEY_LASTSAVEPATH, buf, ARRAYSIZE(buf)) > 0) && PathIsDirectory(buf)) {
            lstrcpy(szInitialDir, buf);
        }
    }
    szFileTitle[0] = '\0';

    int index = 1;
    *ptype = imgSpecifyImageType(szFileName);
    switch (*ptype) {
        case IMG_PNG:     index = 1; break;
        case IMG_GIF:	  index = 2; break;
        case IMG_WEBP:	  index = 3; break;
        case IMG_AVIF:    index = 4; break;
        case IMG_BMP:     index = 5; break;
        case IMG_TIFF:	  index = 6; break;
        case IMG_FPX:	  index = 7; break;
        case IMG_TGA:	  index = 8; break;
        case IMG_RASTER:  index = 9; break;
        case IMG_PCX:     index = 10; break;
        case IMG_PNM:	  index = 11; break;
        case IMG_JP2:	  index = 12; break;
        case IMG_SGI:	  index = 13; break;
    }
    PathRemoveExtension(szFileName);

    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH * 2;
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = MAX_PATH;
    ofn.lpstrInitialDir = szInitialDir;
    ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
    ofn.hInstance = RESINST;
    ofn.nFilterIndex = index;

    // 対応リストのビルド
    CString cs;
//    cs.LoadString(RESINST, IDS_FILTER_SAVE);
    IMAGETYPES* pit = img_types;
    while (pit->type) {
        CString s = pit->avail;
        int si = s.Find(_T('('));
        int ei = s.Find(_T(')'));
        CString e = s.Mid(si, ei - si);
        s.AppendFormat(_T("|%s|"), e.GetString());
        cs.Append(s);
        pit++;
    }
    cs.Append(_T("|"));
    lstrcpy(szFilter, cs);

    TCHAR* pch = szFilter;
    while (*pch) {
    if (*pch == '|') *pch = '\0';
        pch++;
    }
    ofn.lpstrFilter = szFilter;

    if (!GetSaveFileName(&ofn))
        return FALSE;
    lstrcpy(pszName, ofn.lpstrFile);

    index = ofn.nFilterIndex - 1;
    *ptype = img_types[index].type;
    pch = PathFindExtension(pszName);
    if (*pch != '.') {
        lstrcat(pszName, img_types[index].ext);
    } else if (StrCmpI(pch, img_types[index].ext)) {
        PathRenameExtension(pszName, img_types[index].ext);
    }
    return TRUE;
}

BOOL SaveGraphic(HDIB hdib, LPTSTR pszName, int nType)
{
	if (!user_ristriction() || nType == IMG_NOTSUPPORTED) return FALSE;

    DWORD dw = GetFileAttributes(pszName);
    if (dw != 0xFFFFFFFF) {
        if (dw & FILE_ATTRIBUTE_DIRECTORY) {
            AppMessage(IDS_FILEISDIRECTORY, pszName);
            return FALSE;
        } else if (dw & FILE_ATTRIBUTE_READONLY) {
            if (AppMessage(IDR_FRAME, IDS_FILEISREADONLY, pszName, MB_QYESNO) != IDYES)
            return FALSE;

            if (!SetFileAttributes(pszName, dw & ~FILE_ATTRIBUTE_READONLY))
            return FALSE;
        }

        switch (nType) {
            case IMG_JPEG:
            case IMG_TIFF: {
                CComPtr<IExifParser> pep;
                HRESULT hr = pep.CoCreateInstance(__uuidof(ExifParser));
                if (SUCCEEDED(hr) &&
                    (pep->IsExifFile(pszName) == S_OK) &&
                    (AppMessage(IDR_FRAME, IDS_FILEISJPEGEXIF, pszName, MB_QYESNO) != IDYES))
                    return FALSE;
                break;
            }
            case IMG_PSD:
                if (AppMessage(IDR_FRAME, IDS_FILEISPSD, pszName, MB_QYESNO) != IDYES)
                    return FALSE;
                break;
        }
    }

    CAppReg reg;
    BOOL bOK = FALSE;
    CWaitCursor wc;
    if (nType == IMG_JPEG) {
	    // Jpegオプション
	    EFC_JPEGOPTION jo;
	    if (reg.GetBinary(IDS_SEC_SAVE, IDS_KEY_JPEGOPTION, (BYTE*)&jo, sizeof(jo)) != sizeof(jo)) {
		    jo.type = 0;
		    jo.quality = 94;
		    jo.smoothing = 0;
	    }
	    CJpegOption opt;
	    if (opt.DoModal(NULL, (LPARAM)&jo) != IDOK) return FALSE;

	    JPEG_CPARAMS jc;
	    memset(&jc, 0, sizeof(jc));
	    jc.quality = jo.quality;
	    jc.dct = JC_ISLOW;
	    jc.smooth = jo.smoothing;
	    jc.cdata = NULL;
	    if (jo.type == 1) {
		    jc.baseline = TRUE;
	    } else if (jo.type == 2) {
		    jc.progressive = TRUE;
	    }
	    bOK = imgJpgSaveParams(pszName, hdib, &jc);
	    reg.SetBinary(IDS_SEC_SAVE, IDS_KEY_JPEGOPTION, (BYTE*)&jo, sizeof(jo));
    } else if (nType == IMG_WEBP) {
	    // WebPオプション
	    WEBPOPTION wo;
	    if (reg.GetBinary(IDS_SEC_SAVE, IDS_KEY_WEBPOPTION, (BYTE*)&wo, sizeof(wo)) != sizeof(wo)) {
		    wo.quality = 80;
            wo.method = 3;
            wo.preset = WEC_PRESET_DEFAULT;
	    }
	    CWebpOption opt;
	    if (opt.DoModal(NULL, (LPARAM)&wo) != IDOK) return FALSE;

	    bOK = imgWebpSaveParams(pszName, hdib, wo.method, wo.quality, wo.preset);
	    reg.SetBinary(IDS_SEC_SAVE, IDS_KEY_WEBPOPTION, (BYTE*)&wo, sizeof(wo));
    } else if (nType == IMG_AVIF) {
        // WebPオプション
        AVIFOPTION ao;
        if (reg.GetBinary(IDS_SEC_SAVE, IDS_KEY_AVIFOPTION, (BYTE*)&ao, sizeof(ao)) != sizeof(ao)) {
            ao.quality = 60;
            ao.speed = 4;
        }
        CAvifOption opt;
        if (opt.DoModal(NULL, (LPARAM)&ao) != IDOK) return FALSE;

        bOK = imgAvifSaveParams(pszName, hdib, ao.quality, ao.speed);
        reg.SetBinary(IDS_SEC_SAVE, IDS_KEY_AVIFOPTION, (BYTE*)&ao, sizeof(ao));
    } else {
        LPBITMAPINFOHEADER pbh = (LPBITMAPINFOHEADER)GlobalLock(hdib);
        int bits = pbh->biBitCount;
        int bitsNew = bits;
        GlobalUnlock(hdib);

        // GIFの場合は 8bpp で保存!!
        if (nType == IMG_GIF)
            bitsNew = 8;

        // 色数変換保存
        HDIB hdibOrg = hdib;
        if (bitsNew < bits) {
            HDIB hdibNew = quantize(hdib, ditherFS, 1 << bitsNew);
            if (hdibNew)
            hdib = hdibNew;
        } else if (bitsNew > bits) {
            HDIB hdibNew = mifIncreaseColor(hdib, bits);
            if (hdibNew)
            hdib = hdibNew;
        }

        if      (nType == IMG_BMP)		bOK = imgDibSave(pszName, hdib);
        else if (nType == IMG_GIF)		bOK = imgGifSave(pszName, hdib);
        else if (nType == IMG_PNG)		bOK = imgPngSave(pszName, hdib);
        else if (nType == IMG_TIFF)		bOK = imgTifSave(pszName, hdib);
        else if (nType == IMG_FPX)		bOK = imgFpxSaveEx(pszName, hdib, FPXCOMPRESS_JPEG_UNSPECIFIED, 94);
        else if (nType == IMG_TGA)		bOK = imgTgaSave(pszName, hdib);
        else if (nType == IMG_RASTER)	bOK = imgRasSave(pszName, hdib);
        else if (nType == IMG_PCX)		bOK = imgPcxSave(pszName, hdib);
        else if (nType == IMG_PNM)		bOK = imgPnmSave(pszName, hdib);
        else if (nType == IMG_PNG)		bOK = imgPngSave(pszName, hdib);
        else if (nType == IMG_JP2)		bOK = imgJp2Save(pszName, hdib);
	    else if (nType == IMG_SGI)		bOK = imgSgiSave(pszName, hdib, TRUE);

        if (hdib != hdibOrg)
            GlobalFree(hdib);
    }

  // 最後に保存した場所を記録
  TCHAR buf[MAX_PATH * 2];
  lstrcpy(buf, pszName);
  PathRemoveFileSpec(buf);
  reg.SetText(IDS_SEC_SAVE, IDS_KEY_LASTSAVEPATH, buf);

  return bOK;
}
