////////////////////////////////////////////////////////
// finfogrep.h

#ifndef __FINFOGREP_H__
#define __FINFOGREP_H__

#include <thumbcache.h>
#include "thread.h"
#include "shellimagestore_sup.h"
#include "..\lscore\dib.h"


#define X_IMAGESIZE_MIN		96
#define Y_IMAGESIZE_MIN		96
#define X_IMAGESIZE_MAX		160
#define Y_IMAGESIZE_MAX		160


////////////////////////////////////////////////////////
// CFGrepThread

class CFGrepThread : public CGrepThread<CFGrepThread>
{
public:
	CFGrepThread()
	{
		m_msgcode = nRM_GREP_FILE;
		m_priority = THREAD_PRIORITY_BELOW_NORMAL;
	}

	void GrepItem(INT_PTR index, CFileItem* pf)
	{
		CString str;
		WIN32_FIND_DATA wfd;
		if (pf->GetPathName(m_psf, str, &wfd)) {
			bool needNotify = false;
			if (CompareFileTime(&pf->m_ftLastWrite, &wfd.ftLastWriteTime)) {
				pf->m_ftLastWrite = wfd.ftLastWriteTime;
				FormatFileTime(&wfd.ftLastWriteTime, pf->m_column[2], gnListStat & LTS_FMTDATE, gnListStat & LTS_RELATIVEDATE);
				needNotify = true;
			}

			if (wfd.cAlternateFileName[0]) {
				pf->m_column[5] = wfd.cAlternateFileName;
				needNotify = true;
			} else if (!pf->m_column[5].IsEmpty()) {
				pf->m_column[5].Empty();
				needNotify = true;
			}

			if (needNotify) PostItemNotify(index);
		}
	}
};

////////////////////////////////////////////////////////
// CIconGrepThread

class CIconGrepThread : public CGrepThread<CIconGrepThread>
{
public:
	CIconGrepThread()
	{
		m_msgcode = nRM_GREP_ICON;
	}

	void GrepItem(INT_PTR index, CFileItem* pf)
	{
		if (!pf->isDirtyIcon()) return;

		SHFILEINFO sfi;
		if (SHGetFileInfo((LPCTSTR)pf->m_pidlFq, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX)) {
			pf->m_iImage = sfi.iIcon;
			pf->m_fDirtyIcon = false;
			PostItemNotify(index);
		}
/*
		int iImage = SHMapPIDLToSystemImageListIndex(m_psf, pf->m_pidl, NULL);
		if ((iImage >= 0) && (iImage != pf->m_iImage)) {
			pf->m_iImage = iImage;
			pf->m_fDirtyIcon = false;
			PostItemNotify(index);
		}
*/
	}
};

////////////////////////////////////////////////////////
// CColGrepThread

class CColGrepThread : public CGrepThread<CColGrepThread>
{
private:
	CComQIPtr<IShellFolder2> m_psf2;
	CSimpleArray<SHELLCOLUMNINFO> m_ciarr;
public:
	CColGrepThread()
	{
		m_msgcode = nRM_GREP_COLUMN;
		m_priority = THREAD_PRIORITY_BELOW_NORMAL;
	}

	void GrepInitialize()
	{
		// get IShellFolder2 interface
		if (m_psf)
			m_psf2 = m_psf;
		// copy SHELLCOLUMNINFO array
		for (UINT i = 0; i < m_pArray->m_numcol; i++) {
			m_ciarr.Add(m_pArray->m_colinfo[i]);
		}
	}
	void GrepUninitialize()
	{
		m_psf2.Release();
		m_ciarr.RemoveAll();
	}

	void GrepItem(INT_PTR index, CFileItem* pf)
	{
		if (!m_psf2) return;

		CString str;
		bool needNotify = false;
		int cnt = m_ciarr.GetSize();
		for (int i = 0; i < cnt; i++) {
			// 終了チェック
			if (isCanceled()) return;

			if (pf->m_column[i].IsEmpty() && (m_ciarr[i].state & SHCOLSTATE_SLOW)) {
				str.Empty();
				if (m_ciarr[i].scid == PKEY_Null ||
					(!idl_GetShellPropertyText(m_psf2, pf->m_pidl, m_ciarr[i].scid, str, &pf->m_columnvalue[i]) &&
					 !idl_GetPropertyStoreText(pf->m_pidlFq, m_ciarr[i].scid, str, &pf->m_columnvalue[i])))
					idl_GetShellColumnText(m_psf2, pf->m_pidl, m_ciarr[i].iCol, str);

				if (!str.IsEmpty()) {
					pf->m_column[i] = str;
					needNotify = true;
				}
			}
		}
		if (needNotify) PostItemNotify(index);
	}
};

class CColumnGrep
{
private:
	CColGrepThread* m_grep;
public:
	CColumnGrep() : m_grep(NULL) {}
	~CColumnGrep() { Stop(THREAD_EXIT_TIMEOUT); }
	void Stop(DWORD dwTimeout = 2000)
	{
		if (m_grep) { m_grep->Stop(dwTimeout); delete m_grep; m_grep = NULL; }
	}
	BOOL Start(HWND hwnd, IShellFolder* psf, CFileStack* pArray, ULONGLONG magic, int numLoops = 1)
	{
		Stop();
		m_grep = new CColGrepThread;
		return m_grep->Start(hwnd, psf, pArray, magic, numLoops);
	}
	bool isRunning() { return (m_grep && m_grep->isRunning()); }
};

////////////////////////////////////////////////////////
// CIGrepThread

HRESULT rSHCreateItemFromIDList(PCIDLIST_ABSOLUTE pidlFullyQual, REFIID riid, void** ppv);
Bitmap* LoadThumbnail(LPCTSTR pszName, int width, int height);
Bitmap* LoadRawThumbnail(LPCTSTR pszName, int width, int height, EXIFINFOSTRUCT* pei);
Bitmap* ResizeBitmap(HBITMAP hbm, int width, int height);

class CImageGrepThread : public CGrepThread<CImageGrepThread>
{
private:
	CSize m_sizeImage;
	DWORD m_lock;
	IThumbnailCache* m_ptc;
	int m_useSystemCache;

public:
	CImageGrepThread()
		: m_ptc(NULL)
		, m_lock(0)
		, m_useSystemCache(0)
	{
		m_msgcode = nRM_GREP_IMAGE;
		m_sizeImage = CSize(X_IMAGESIZE_MAX, Y_IMAGESIZE_MAX);
	}

	void SetImageSize(CSize size)
	{
		if (size != m_sizeImage) {
			m_cs.Enter();
			m_sizeImage = size;
			m_cs.Leave();
		}
	}

	void UseSystemCache(int mode)
	{
		m_cs.Enter();
		m_useSystemCache = mode;			// 0=no-use, 1=readonly, 2=update
		m_cs.Leave();
	}

	void GrepInitialize()
	{
		CoCreateInstance(CLSID_LocalThumbnailCache, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&m_ptc));
	}

	void GrepUninitialize()
	{
		if (m_ptc) {
			m_ptc->Release();
			m_ptc = NULL;
		}
	}

	void GrepItem(INT_PTR index, CFileItem* pf)
	{
		if (pf && !pf->m_bmp) {
			if (pf->m_column[0].IsEmpty()) return;
			if (pf->isFolder() && (gnFileStat & FTS_NOTHUMBFOLDER)) return;

			// .icoファイルはどうもちょっと...
			LPTSTR p = PathFindExtension(pf->m_column[0]);
			if (pf->isFSFile() && !StrCmpI(p, _T(".ico")))
				return;

			// キャッシュを優先してイメージを取得
			if (m_useSystemCache) {
				pf->m_bmp = GetHBitmapFromSystemCache(pf);
				if (pf->m_bmp) {
					// 読めたらexif情報をチェック
					CExifInformation* exifptr = new CExifInformation;
					if (exifptr->GetShellProperties(m_psf, pf->m_pidl)) {
						// exif情報
						if (pf->m_exifptr)
							delete pf->m_exifptr;
						pf->m_exifptr = exifptr;
					}
				}
			}
			//------------------------------------------
			// jpg/raw などの画像ファイル
			//------------------------------------------
			CString str;
			if (!pf->m_bmp && pf->isFSFile()) {
				idl_GetName(m_psf, pf->m_pidl, SHGDN_FORPARSING, str);

				TCHAR* tip = new TCHAR[MAX_URL_LENGTH + MAX_PATH];
				EXIFINFOSTRUCT info;
				info.cchLabelText = MAX_PATH;
				info.pszLabelText = tip;
				info.cchTipText = MAX_URL_LENGTH;
				info.pszTipText = tip + MAX_PATH;
				pf->m_bmp = LoadRawThumbnail(str, m_sizeImage.cx, m_sizeImage.cy, &info);
				if (pf->m_bmp && *info.pszLabelText) {
					pf->m_infotip = info.pszTipText;
					pf->SetExifInfo(&info);
				}
				if (!pf->m_bmp)
					pf->m_bmp = LoadThumbnail(str, m_sizeImage.cx, m_sizeImage.cy);
				delete [] tip;
			}

			// キャッシュを優先しない場合はここで
			if (!pf->m_bmp && !m_useSystemCache) {
				pf->m_bmp = GetHBitmapFromSystemCache(pf);
			}
			if (!pf->m_bmp) {
				HBITMAP hbm = idl_ExtractImage(m_psf, pf->m_pidl, &m_sizeImage);
				if (hbm) {
					BITMAP bm;
					GetObject(hbm, sizeof(bm), &bm);
					if (bm.bmWidth > m_sizeImage.cx || bm.bmHeight > m_sizeImage.cy) {
						pf->m_bmp = ResizeBitmap(hbm, m_sizeImage.cx, m_sizeImage.cy);
						DeleteObject(hbm);
					}
				}
				else {
					if (!pf->isFSFile() && !pf->isFolder() && m_psf) {
						CComPtr<IDataObject> pdo;
						if (NOERROR == m_psf->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&pf->m_pidl, IID_IDataObject, NULL, (void**)&pdo)) {
							HDROP hdrop = (HDROP)GetDataFromDataObject(pdo, CF_HDROP);
							if (hdrop) {
								TCHAR buf[MAX_PATH * 2];
								DragQueryFile(hdrop, 0, buf, ARRAYSIZE(buf));
								DragFinish(hdrop);
								pf->m_bmp = LoadThumbnail(buf, m_sizeImage.cx, m_sizeImage.cy);
							}
						}
					}
				}
			}

			if (pf->m_bmp && IsWindow(m_hWnd))
				PostItemNotify(index);
		}
	}
private:
	Bitmap* GetHBitmapFromSystemCache(CFileItem* pf)
	{
		Bitmap* bitmap = NULL;
		CComPtr<IShellItem> psi;
		if (m_ptc && SHCreateItemFromIDList(pf->m_pidlFq, IID_PPV_ARGS(&psi)) == S_OK) {
			WTS_FLAGS wts;
			switch (m_useSystemCache) {
				case 0: 	wts = WTS_EXTRACT | WTS_EXTRACTDONOTCACHE; break;
				case 1: 	wts = WTS_EXTRACT | WTS_INCACHEONLY; break;
				case 2:
				default:	wts = WTS_EXTRACT; break;
			}

			CComPtr<ISharedBitmap> psb;
			if (m_ptc->GetThumbnail(psi, m_sizeImage.cy, wts, &psb, NULL, NULL) == S_OK) {
				HBITMAP hbm;
				if (psb->GetSharedBitmap(&hbm) == S_OK) {
					BITMAP bm;
					GetObject(hbm, sizeof(bm), &bm);
					if (bm.bmBitsPixel > 24) {
						CDib dib(hbm, NULL);
						PixelFormat fmt = PixelFormat24bppRGB;
						switch (dib.NumBits()) {
							case  1: fmt = PixelFormat1bppIndexed; break;
							case  4: fmt = PixelFormat4bppIndexed; break;
							case  8: fmt = PixelFormat8bppIndexed; break;
							case 24: fmt = PixelFormat24bppRGB; break;
							case 32: fmt = PixelFormat32bppARGB; break;
						}
						bitmap = new Bitmap(dib.Width(), dib.Height(), dib.WidthBytes(), fmt, (BYTE*)dib.FindBits());
						bitmap->RotateFlip(RotateNoneFlipY);
					}
					else {
						bitmap = ResizeBitmap(hbm, m_sizeImage.cx, m_sizeImage.cy);
					}
				}
			}
		}
		return bitmap;
	}
};


////////////////////////////////////////////////////////
// CDirGrepThread

class CDirGrepThread : public CGrepThread<CDirGrepThread>
{
public:
	CDirGrepThread()
	{
		m_msgcode = nRM_GREP_DIR;
//@		m_priority = THREAD_PRIORITY_BELOW_NORMAL;
	}

	bool GrepDirectory(LPCTSTR path, DWORDLONG& totalsize)
	{
		TCHAR buf[MAX_PATH * 2];
		PathCombine(buf, path, _T("*.*"));
		bool cont = true;
		CWin32FindData fd;
		HANDLE hFind = FindFirstFile(buf, &fd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				// 終了チェック
				if (isCanceled()) {
					cont = false;
					break;
				}

				if (fd.isDot()) continue;

				if (fd.isFolder()) {
					PathCombine(buf, path, fd.cFileName);
					cont = GrepDirectory(buf, totalsize);
					if (!cont) break;
				}
				else {
					DWORDLONG tmp;
					SETDWORDLONG(tmp, fd.nFileSizeLow, fd.nFileSizeHigh);
					totalsize += tmp;
				}
			} while (FindNextFile(hFind, &fd));
			FindClose(hFind);
			Sleep(0);
		}
		return cont;
	}

	void GrepItem(INT_PTR index, CFileItem* pf)
	{
		CString	str;
		if (pf->isFSDir() &&
			!pf->m_filesize &&
			pf->m_column[1].IsEmpty() &&
			idl_GetName(m_psf, pf->m_pidl, SHGDN_FORPARSING, str))
		{
			if (pf->m_fileattr & FILE_ATTRIBUTE_REPARSE_POINT) {
				// リパースポイントは無視
				pf->m_column[1] = _T("(Junction)");
				PostItemNotify(index);
				return;
			}
			DWORDLONG total = 0;
			if (GrepDirectory(str.GetString(), total)) {
				pf->m_filesize = total;
				CString fs;
				if (gnListStat & LTS_FMTSIZE)
					FormatFileSize(pf->m_filesize, fs, gnListStat & LTS_FMTSIZE);
				else {
					LPTSTR pch = fs.GetBuffer(MAX_PATH);
					StrFormatByteSize64(pf->m_filesize, pch, MAX_PATH);
					fs.ReleaseBuffer();
				}
				pf->m_column[1] = _T("*") + fs;
				PostItemNotify(index);
			}
		}
	}
};

#endif				// __FINFOGREP_H__
