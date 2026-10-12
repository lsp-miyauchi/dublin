////////////////////////////////////////////////////////////////////////////////
// iconres.cpp  lsfop/lsshell 共用
//

#include "stdafx.h"
#include "shlobj.h"
#include "resource.h"
#include "gdiplus_sup.h"
#include "lsfop.h"
#include "cdll.h"
#include "filehandle.h"
#include "iconres.h"

HICON GetIconFromInstance(HMODULE hmod, LPTSTR resid, int width, int height)
{
	HRSRC hr = FindResource(hmod, resid, RT_GROUP_ICON);
	if (hr) {
		LPVOID pv = LockResource( LoadResource(hmod, hr) );
		if (pv) {
			int id = LookupIconIdFromDirectoryEx((BYTE*)pv, TRUE, width, height, LR_DEFAULTCOLOR);
			hr = FindResource(hmod, MAKEINTRESOURCE(id), RT_ICON);
			if (hr) {
				pv = LockResource( LoadResource(hmod, hr) );
				if (pv) {
					return CreateIconFromResourceEx((BYTE*)pv, SizeofResource(hmod, hr), TRUE, ICOVER, width, height, LR_DEFAULTCOLOR);
				}
			}
		}
	}
	return NULL;
}

void CIconItem::LoadIcon(HMODULE hmod, LPTSTR resid)
{
	static int sizes[] = { 16, 24, 32, 48 };
	for (int i = 0; i < ARRAYSIZE(sizes); i++) {
		HICON hic = GetIconFromInstance(hmod, resid, sizes[i], sizes[i]);
		if (!hic) { hic = (HICON)LoadImage(RESINST, MAKEINTRESOURCE(IDI_DUMMY), IMAGE_ICON, sizes[i], sizes[i], LR_DEFAULTCOLOR); }
		m_mapIcon.Add(sizes[i], hic);
	}
	m_resid = resid;
}

ICONRESOURCE* AllocIconRes(int num)
{
	DWORD size = sizeof(ICONRESOURCE) + (num - 1) * sizeof(ICONIMAGE);
	ICONRESOURCE* pir = (ICONRESOURCE*)malloc(size);
	if (pir) {
		memset(pir, 0, size);
		pir->num = num;
	}
	return pir;
}

void FreeIconRes(ICONRESOURCE* pir)
{
	for (UINT i = 0; i < pir->num; i++) {
		if (pir->images[i].pBitData)
			free(pir->images[i].pBitData);
	}
	free(pir);
}

HBITMAP MakeBitmapFromData(void* data, size_t size)
{
	HBITMAP hbm = NULL;
	if (data && size > 0) {
		HGLOBAL hmem = GlobalAlloc(GHND, size);
		if (hmem) {
			void* pv = GlobalLock(hmem);
			memcpy(pv, data, size);
			GlobalUnlock(hmem);

			IStream* pstm;
			if (CreateStreamOnHGlobal(hmem, FALSE, &pstm) == S_OK) {
				gdiplus_using gu;
				Bitmap* bitmap = new Bitmap(pstm);
				Color color;
				if (bitmap->GetHBITMAP(color, &hbm) != Ok)
					hbm = NULL;
				delete bitmap;
				pstm->Release();
			}
		}
		GlobalFree(hmem);
	}
	return hbm;
}

HICON MakeIconFromResource(ICONIMAGE* pii)
{
	if (!pii || !pii->pBitData) return NULL;

	HICON hIcon = NULL;
	if (pii->IsPng()) {
		// PNG
		ICONINFO ici;
		ici.fIcon = TRUE;
		ici.hbmColor = MakeBitmapFromData(pii->pBitData, pii->numbytes);
		if (ici.hbmColor) {
			BITMAP bm;
			GetObject(ici.hbmColor, sizeof(bm), &bm);
			ici.xHotspot = bm.bmWidth / 2;
			ici.yHotspot = bm.bmHeight / 2;

			int size = WIDTHBYTES(bm.bmWidth) * bm.bmHeight;
			BYTE* pb = new BYTE[size];
			if (pb) {
				memset(pb, 0, size);
				ici.hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, pb);
				if (ici.hbmMask) {
					hIcon = CreateIconIndirect(&ici);
					DeleteObject(ici.hbmMask);
				}
				delete [] pb;
			}
			DeleteObject(ici.hbmColor);
		}
	} else {
		BITMAPINFO* pbi = pii->BitmapInfoPtr();
		hIcon = CreateIconFromResourceEx(pii->pBitData, pii->numbytes,
							TRUE, ICOVER,
							pbi->bmiHeader.biWidth,
							pbi->bmiHeader.biHeight / 2, 0);
		if (!hIcon) {
			// We would break on NT if we try with a 16bpp image
			if (pbi->bmiHeader.biBitCount != 16)
				hIcon = CreateIconFromResource((BYTE*)pbi, pii->numbytes, TRUE, ICOVER);
		}
	}
    return hIcon;
}

#pragma pack(1)
typedef struct tagEZPNGHEADER {
	DWORD magic;
	DWORD dummy[3];
	DWORD msbl_width;
	DWORD msbl_height;
} EZPNGHEADER;
#pragma pack()

static BOOL adjust_iconimage(ICONIMAGE* pii)
{
	if (!pii) return FALSE;

	// VistaではPNG形式でアイコンが保存されていることがある
	if (pii->IsPng()) {
		EZPNGHEADER* pnghdr = (EZPNGHEADER*)pii->pBitData;

		pii->width = MAKELONG(
							MAKEWORD(HIBYTE(HIWORD(pnghdr->msbl_width)), LOBYTE(HIWORD(pnghdr->msbl_width))),
							MAKEWORD(HIBYTE(LOWORD(pnghdr->msbl_width)), LOBYTE(LOWORD(pnghdr->msbl_width)))
						);
		pii->height = MAKELONG(
							MAKEWORD(HIBYTE(HIWORD(pnghdr->msbl_height)), LOBYTE(HIWORD(pnghdr->msbl_height))),
							MAKEWORD(HIBYTE(LOWORD(pnghdr->msbl_height)), LOBYTE(LOWORD(pnghdr->msbl_height)))
						);
		pii->colors = 24;
	}
	else {
		BITMAPINFOHEADER* hdr = (BITMAPINFOHEADER*)pii->pBitData;
		if (hdr->biPlanes != 1) return FALSE;

		switch (hdr->biBitCount) {
		case 1:
		case 4:
		case 8:
		case 16:
		case 24:
		case 32: break;
		default: return FALSE;
		}
		switch (hdr->biCompression) {
		case BI_RGB:
		case BI_RLE8:
		case BI_RLE4:
		case BI_BITFIELDS: break;
		default: return FALSE;
		}

	    // BITMAPINFO is at beginning of bits
		pii->width = hdr->biWidth;

		// Icons are stored in funky format where height is doubled
		pii->height = (hdr->biHeight) / 2;
		pii->colors = hdr->biPlanes * hdr->biBitCount;
	}
	return TRUE;
}

ICONRESOURCE* ReadIconFromResource(HMODULE hmod, LPCTSTR resid)
{
	// Find the group icon resource
	HGLOBAL hmem = LoadResource(hmod, FindResource(hmod, resid, RT_GROUP_ICON));
	if (!hmem) return NULL;

	LPMEMICONDIR pmid = (MEMICONDIR*)LockResource(hmem);
	if (!pmid) return NULL;

	// Allocate enough memory for the images
	ICONRESOURCE* pir = AllocIconRes(pmid->num);
	if (!pir) return NULL;

	// Loop through the images
	int index = 0;
	for (UINT i = 0; i < pir->num; i++) {
		// Get the individual image
		HRSRC hr = FindResource(hmod, MAKEINTRESOURCE(pmid->memdir[i].id), RT_ICON);
		hmem = LoadResource(hmod, hr);
		if (hmem) {
			// Store a copy of the resource locally
			pir->images[index].numbytes = SizeofResource(hmod, hr);
			pir->images[index].pBitData = (BYTE*)malloc(pir->images[index].numbytes);
			memcpy(pir->images[index].pBitData, LockResource(hmem), pir->images[index].numbytes);

			// Adjust internal pointers
			if (adjust_iconimage(&pir->images[index]))
				index++;
			else {
				pir->images[index].numbytes = 0;
				if (pir->images[index].pBitData) free(pir->images[index].pBitData);
				pir->images[index].pBitData = NULL;
			}
		}
	}
	pir->num = index;

	if (index < 1) {
		FreeIconRes(pir);
		pir = NULL;
	}
	return pir;
}

static BOOL CALLBACK enum_groupname(HMODULE hmod, LPCTSTR pType, LPTSTR pName, LONG_PTR data)
{
	CSimpleArray<CIconItem*>* parr = (CSimpleArray<CIconItem*>*)data;
	CIconItem* pii = new CIconItem;
	pii->LoadIcon(hmod, pName);
	parr->Add(pii);
	return TRUE;
}

int EnumIconFromFile(LPCTSTR pszName, CSimpleArray<CIconItem*>& arr)
{
	int oldcnt = arr.GetSize();
	HMODULE hmod = LoadLibraryEx(pszName, NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (hmod) {
		EnumResourceNames(hmod, RT_GROUP_ICON, enum_groupname, (LONG_PTR)&arr);
		FreeLibrary(hmod);
	} else {
		CIconItem* pii = new CIconItem;
		pii->m_bIconFile = TRUE;
		pii->SetIcon(16, (HICON)LoadImage(NULL, pszName, IMAGE_ICON, 16, 16, LR_LOADFROMFILE));
		pii->SetIcon(24, (HICON)LoadImage(NULL, pszName, IMAGE_ICON, 24, 24, LR_LOADFROMFILE));
		pii->SetIcon(32, (HICON)LoadImage(NULL, pszName, IMAGE_ICON, 32, 32, LR_LOADFROMFILE));
		pii->SetIcon(48, (HICON)LoadImage(NULL, pszName, IMAGE_ICON, 48, 48, LR_LOADFROMFILE));
		if (pii->IsEmpty())
			delete pii;
		else
			arr.Add(pii);
	}
	return ((int)arr.GetSize() - oldcnt);
}

static BOOL CALLBACK pick_groupname(HMODULE hmod, LPCTSTR pType, LPTSTR pName, LONG_PTR data)
{
	CSimpleArray<_STRINGorINT>* arr = (CSimpleArray<_STRINGorINT>*)data;
	arr->Add(_STRINGorINT(pName));
	return TRUE;
}

ICONRESOURCE* GetIconResourceFromFile(LPCTSTR pszName, int index)
{
	ICONRESOURCE* pir = NULL;
	HINSTANCE hmod = LoadLibraryEx(pszName, NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (hmod) {
		CSimpleArray<_STRINGorINT> arr;
		if (EnumResourceNames(hmod, RT_GROUP_ICON, pick_groupname, (LONG_PTR)&arr)) {
			int numIcons = arr.GetSize();
			if (0 <= index && index < numIcons)
				pir = ReadIconFromResource(hmod, arr[index].m_pstr);
		}
		FreeLibrary(hmod);
	}
	return pir;
}

int ExportMultipleIcons(LPCTSTR pszName, LPCTSTR pszTo, int* pnIndex, int entries)
{
	HINSTANCE hmod = LoadLibraryEx(pszName, NULL, LOAD_LIBRARY_AS_DATAFILE);
	int num = 0;
	if (hmod) {
		CWaitCursor wc;

		TCHAR pref[MAX_PATH];
		TCHAR name[MAX_PATH];

		TCHAR* p = PathFindFileName(pszName);
		PathCombine(pref, pszTo, p);
		p = PathFindExtension(pref);
		*p = 0;

		CSimpleArray<_STRINGorINT> arr;
		if (EnumResourceNames(hmod, RT_GROUP_ICON, pick_groupname, (LONG_PTR)&arr)) {
			int numIcons = arr.GetSize();
			for (int i = 0; i < numIcons; i++) {
				LPCTSTR p = arr[i].m_pstr;
				BOOL bMatched = FALSE;
				if (pnIndex) {
					int n;
					for (n = 0; n < entries; n++) {
						if (pnIndex[n] == i) {
							bMatched = TRUE;
							break;
						}
					}
					if (bMatched)
						wsprintf(name, _T("%s_%d"), pref, pnIndex[n]);
				} else {
					bMatched = TRUE;
					if (IS_INTRESOURCE(p))
						wsprintf(name, _T("%s_%d"), pref, p);
					else
						wsprintf(name, _T("%s_%s"), pref, p);
				}

				if (!bMatched) continue;

				ICONRESOURCE* pir = ReadIconFromResource(hmod, p);
				if (pir) {
					lstrcat(name, _T(".ico"));
					if (WriteIconToICOFile(pir, name)) {
						num++;
					}
					FreeIconRes(pir);
				}
			}
		}
		FreeLibrary(hmod);
	}
	return num;
}

BOOL ExportSingleIcon(LPCTSTR pszName, LPCTSTR pszExportIconName, int index)
{
	BOOL bOK = FALSE;
	HINSTANCE hmod = LoadLibraryEx(pszName, NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (hmod) {
		CWaitCursor wc;
		CSimpleArray<_STRINGorINT> arr;
		if (EnumResourceNames(hmod, RT_GROUP_ICON, pick_groupname, (LONG_PTR)&arr)) {
			int numIcons = arr.GetSize();
			if (0 <= index && index < numIcons) {
				ICONRESOURCE* pir = ReadIconFromResource(hmod, arr[index].m_pstr);
				if (pir) {
					bOK = WriteIconToICOFile(pir, pszExportIconName);
					FreeIconRes(pir);
				}
			}
		}
		FreeLibrary(hmod);
	}
	return bOK;
}

int MakeIconDataObject(LPCTSTR pszName, CSimpleArray<CIconItem*>& arrIcon, ICONDATAOBJECT* pido)
{
	memset(pido, 0, sizeof(ICONDATAOBJECT));

	HINSTANCE hmod = LoadLibraryEx(pszName, NULL, LOAD_LIBRARY_AS_DATAFILE);
	int num = 0;
	if (hmod) {
		TCHAR name[MAX_PATH], pref[MAX_PATH];
		lstrcpy(pref, PathFindFileName(pszName));
		LPTSTR p = PathFindExtension(pref);
		*p = 0;

		CSimpleArray<_STRINGorINT> arr;
		if (EnumResourceNames(hmod, RT_GROUP_ICON, pick_groupname, (LONG_PTR)&arr)) {
			int entries = arrIcon.GetSize();
			DWORD size = sizeof(FILEGROUPDESCRIPTOR) + sizeof(FILEDESCRIPTOR) * (entries - 1);
			pido->hFileGroupDescriptor = GlobalAlloc(GHND | GMEM_SHARE, size);
			FILEGROUPDESCRIPTOR* pFileGroupDescriptor = (FILEGROUPDESCRIPTOR*)GlobalLock(pido->hFileGroupDescriptor);
			pido->hFileContents = NULL;

			int numIcons = arr.GetSize();
			for (int i = 0; i < numIcons; i++) {
				LPCTSTR pn = arr[i].m_pstr;
				BOOL bMatched = FALSE;
				if (arrIcon.GetSize() > 0) {
					int n;
					for (n = 0; n < entries; n++) {
						if (arrIcon[n]->m_resid == arr[i]) {
							bMatched = TRUE;
							break;
						}
					}
				} else {
					bMatched = TRUE;
				}

				if (!bMatched) continue;

				if (IS_INTRESOURCE(pn))
					wsprintf(name, _T("%s_%d"), pref, pn);
				else
					wsprintf(name, _T("%s_%s"), pref, pn);
				ICONRESOURCE* pir = ReadIconFromResource(hmod, pn);
				if (pir) {
					lstrcat(name, _T(".ico"));
					DWORD written;
					pido->hFileContents = WriteIconToMemory(pido->hFileContents, pir, &written);

					FILEDESCRIPTOR* pFileDesc = &pFileGroupDescriptor->fgd[num];
					memset(pFileDesc, 0, sizeof(FILEDESCRIPTOR));
					pFileDesc->dwFlags = FD_PROGRESSUI | FD_FILESIZE | FD_UNICODE;
					lstrcpy(pFileDesc->cFileName, name);
					pFileDesc->nFileSizeLow = written;
					FreeIconRes(pir);

					num++;
				}
			}
			pFileGroupDescriptor->cItems = num;
			GlobalUnlock(pido->hFileGroupDescriptor);
		}
		FreeLibrary(hmod);
	}
	return num;
}

UINT read_icoheader(HANDLE hf)
{
	WORD w[3];
	DWORD dw;
	if (ReadFile(hf, w, sizeof(w), &dw, NULL) &&
		dw == sizeof(w) && w[0] == 0 && w[1] == 1)
		return w[2];
	return (UINT)-1;
}

ICONRESOURCE* ReadIconFromICOFile(LPCTSTR pszName)
{
	CFileHandle file;
	file.CreateFile(pszName, GENERIC_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL);
	if (file.IsInvalid())
		return NULL;

    // Read in the header
	UINT num = read_icoheader(file);
	if (num == (UINT)-1)
		return NULL;

	// Adjust the size of the struct to account for the images
	ICONRESOURCE* pir = AllocIconRes(num);
	if (!pir)
		return NULL;

	// Read in the icon directory entries
	ICONDIRENTRY* pde = new ICONDIRENTRY[num];
	DWORD dw;
	if (!file.ReadFile(pde, num * sizeof(ICONDIRENTRY), &dw) ||
		dw != num * sizeof(ICONDIRENTRY))
	{
		FreeIconRes(pir);
		return NULL;
	}

	// Loop through and read in each image
	for (UINT i = 0; i < num; i++) {
		// Allocate memory for the resource
		if ((pir->images[i].pBitData = (BYTE*)malloc(pde[i].numbytes)) == NULL) {
			FreeIconRes(pir);
			delete [] pde;
			return NULL;
		}
		pir->images[i].numbytes = pde[i].numbytes;

		// Read it in
		if (file.SetFilePointer(pde[i].offset, NULL, FILE_BEGIN) == 0xFFFFFFFF ||
			!file.ReadFile(pir->images[i].pBitData, pde[i].numbytes, &dw) ||
			dw != pde[i].numbytes ||
			!adjust_iconimage(&pir->images[i]))
        {
			FreeIconRes(pir);
			delete [] pde;
			return NULL;
		}
	}
	delete [] pde;
	return pir;
}

static BOOL write_icoheader(HANDLE hf, UINT entries)
{
	WORD w[3];
    DWORD dw;

	w[0] = 0;
	w[1] = 1;
	w[2] = (WORD)entries;
	return (WriteFile(hf, w, sizeof(w), &dw, NULL) && dw == sizeof(w));
}

static DWORD calc_imgoffset(ICONRESOURCE* pir, int index)
{
    // Calculate the ICO header size
	DWORD size = 3 * sizeof(WORD);

	// Add the ICONDIRENTRY's
	size += pir->num * sizeof(ICONDIRENTRY);

	// Add the sizes of the previous images
	for (int i = 0; i < index; i++)
		size += pir->images[i].numbytes;
	return size;
}

BOOL WriteIconToStream(IStream* pstrm, LPICONRESOURCE pir, DWORD* pnumWritten)
{
	LARGE_INTEGER li;
	li.QuadPart = 0;
	ULARGE_INTEGER origin;
	pstrm->Seek(li, STREAM_SEEK_CUR, &origin);

	*pnumWritten = 0;

    // Write the header
	WORD w[3];
	w[0] = 0;
	w[1] = 1;
	w[2] = (WORD)pir->num;
	ULONG written;
	if (pstrm->Write(w, sizeof(w), &written) != S_OK) {
		return FALSE;
	}

    // Write the ICONDIRENTRY's
	LPBITMAPINFO pbi;
	UINT i;
	for (i = 0; i < pir->num; i++) {
        // Convert internal format to ICONDIRENTRY
		ICONDIRENTRY ide;
		ide.width = pir->images[i].width;
		ide.height = pir->images[i].height;
		ide.reserved = 0;

		if (pir->images[i].IsPng()) {
			ide.planes = 1;
			ide.bits = 24;
			ide.colors = 0;
		} else {
			pbi = pir->images[i].BitmapInfoPtr();
			ide.planes = pbi->bmiHeader.biPlanes;
			ide.bits = pbi->bmiHeader.biBitCount;
			if ((ide.planes * ide.bits) >= 8)
				ide.colors = 0;
        	else
				ide.colors = 1 << (ide.planes * ide.bits);
		}

		ide.numbytes = pir->images[i].numbytes;
		ide.offset = calc_imgoffset(pir, i);
		// Write the ICONDIRENTRY out to disk
		if (pstrm->Write(&ide, sizeof(ICONDIRENTRY), &written) != S_OK || written != sizeof(ICONDIRENTRY)) {
			return FALSE;
		}
	}

    // Write the image bits for each image
	for (i = 0; i < pir->num; i++ ) {
		bool bPng = pir->images[i].IsPng();

		pbi = pir->images[i].BitmapInfoPtr();
		DWORD dwTemp = pbi->bmiHeader.biSizeImage;

		// Set the sizeimage member to zero
		if (!bPng) pbi->bmiHeader.biSizeImage = 0;

		// Write the image bits to file
		if (pstrm->Write(pbi, pir->images[i].numbytes, &written) != S_OK ||
			written != pir->images[i].numbytes) {
			return FALSE;
		}
        // set it back
		if (!bPng) pbi->bmiHeader.biSizeImage = dwTemp;
	}

	li.QuadPart = 0;
	ULARGE_INTEGER curr;
	pstrm->Seek(li, STREAM_SEEK_CUR, &curr);
	*pnumWritten = (DWORD)(curr.QuadPart - origin.QuadPart);

	return TRUE;
}

BOOL WriteIconToICOFile(LPICONRESOURCE pir, LPCTSTR szFileName)
{
	BOOL ok = FALSE;
	IStream* pstrm;
	if (CreateStreamOnHGlobal(NULL, TRUE, &pstrm) == S_OK) {
		DWORD bytes;
		if (WriteIconToStream(pstrm, pir, &bytes)) {
			HGLOBAL hmem;
			if (GetHGlobalFromStream(pstrm, &hmem) == S_OK) {
				void* pv = GlobalLock(hmem);
			    // open the file
				CFileHandle file;
				file.CreateFile(szFileName, GENERIC_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL);
				if (!file.IsInvalid()) {
					DWORD written;
					ok = (file.WriteFile(pv, bytes, &written) && written == bytes);
				}
				GlobalUnlock(hmem);
			}
		}
		pstrm->Release();
	}
	return ok;
}

BOOL xxxWriteIconToICOFile(LPICONRESOURCE pir, LPCTSTR szFileName)
{
	UINT        i;

    // open the file
	CFileHandle file;
	file.CreateFile(szFileName, GENERIC_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL);
	if (file.IsInvalid())
		return FALSE;

    // Write the header
	if (!write_icoheader(file, pir->num))
		return FALSE;

    // Write the ICONDIRENTRY's
    DWORD numWritten;
	LPBITMAPINFO pbi;
	for (i = 0; i < pir->num; i++) {
        // Convert internal format to ICONDIRENTRY
		ICONDIRENTRY ide;
		ide.width = pir->images[i].width;
		ide.height = pir->images[i].height;
		ide.reserved = 0;

		if (pir->images[i].IsPng()) {
			ide.planes = 1;
			ide.bits = 24;
			ide.colors = 0;
		}
		else {
			pbi = pir->images[i].BitmapInfoPtr();
			ide.planes = pbi->bmiHeader.biPlanes;
			ide.bits = pbi->bmiHeader.biBitCount;
			if ((ide.planes * ide.bits) >= 8)
				ide.colors = 0;
        	else
				ide.colors = 1 << (ide.planes * ide.bits);
		}

		ide.numbytes = pir->images[i].numbytes;
		ide.offset = calc_imgoffset(pir, i);

		// Write the ICONDIRENTRY out to disk
		if (!file.WriteFile(&ide, sizeof(ICONDIRENTRY), &numWritten))
			return FALSE;

        // Did we write a full ICONDIRENTRY ?
		if (numWritten != sizeof(ICONDIRENTRY))
            return FALSE;
	}

    // Write the image bits for each image
	for (i = 0; i < pir->num; i++ ) {
		bool bPng = pir->images[i].IsPng();

		pbi = pir->images[i].BitmapInfoPtr();
		DWORD dwTemp = pbi->bmiHeader.biSizeImage;

		// Set the sizeimage member to zero
		if (!bPng) pbi->bmiHeader.biSizeImage = 0;

		// Write the image bits to file
		if (!file.WriteFile(pbi, pir->images[i].numbytes, &numWritten))
			return FALSE;

		if (numWritten != pir->images[i].numbytes)
            return FALSE;

        // set it back
		if (!bPng) pbi->bmiHeader.biSizeImage = dwTemp;
	}
	return TRUE;
}

HGLOBAL WriteIconToMemory(HGLOBAL hmem, LPICONRESOURCE pir, DWORD* written)
{
	IStream* pstrm;
	if (CreateStreamOnHGlobal(hmem, FALSE, &pstrm) == S_OK) {
		if (hmem) {
			LARGE_INTEGER li;
			li.QuadPart = 0;
			ULARGE_INTEGER npos;
			pstrm->Seek(li, STREAM_SEEK_END, &npos);
		}
		DWORD written;
		if (WriteIconToStream(pstrm, pir, &written)) {
			GetHGlobalFromStream(pstrm, &hmem);
		}
		pstrm->Release();
	}
	return hmem;
}

BOOL WriteIconToPNGFile(ICONIMAGE* pii, LPCTSTR szFileName)
{
	// ----------------------------------------------------
	// アイコンイメージデータをPNGファイルに書き出す
	// ----------------------------------------------------
	if (pii->IsPng()) {
		// png形式のアイコンの場合
		CFileHandle file;
		file.CreateFile(szFileName, GENERIC_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL);
		if (file.IsInvalid()) return FALSE;

	    DWORD numWritten;
		return (file.WriteFile(pii->BitmapInfoPtr(), pii->numbytes, &numWritten) && numWritten == pii->numbytes);
	}

	BOOL bOK = FALSE;
	if (pii->colors < 32) {
		// ARGBデータでなければgdi+を使う
		// いったんHICONに逆コンバート
		HICON hic = MakeIconFromResource(pii);
		if (hic) {
			Bitmap* bitmap = new Bitmap(hic);
			if (bitmap && bitmap->GetLastStatus() == Ok) {
				UINT num;			// number of image encoders
				UINT size;			// size, in bytes, of the image encoder array
				GetImageEncodersSize(&num, &size) ;
				void* codecinfoptr = malloc(size);
				ImageCodecInfo* pici = (ImageCodecInfo*)codecinfoptr;
				GetImageEncoders(num, size, pici);
				for (UINT i = 0; i < num; i++) {
					if (!wcscmp(pici->MimeType, L"image/png")) {
						bOK = (bitmap->Save(szFileName, &pici->Clsid) == Ok);
						break;
					}
					pici++;
				}
				free(codecinfoptr);
				delete bitmap;
			}
			::DestroyIcon(hic);
		}
	}

	if (!bOK) {
		// lsimgを使って出力
		HGLOBAL hmem = GlobalAlloc(GHND, pii->numbytes);
		if (hmem) {
			// dibを作成
			BITMAPINFO* pbi = (BITMAPINFO*)GlobalLock(hmem);
			memcpy(pbi, pii->BitmapInfoPtr(), pii->numbytes);
			pbi->bmiHeader.biWidth = pii->width;
			pbi->bmiHeader.biHeight = pii->height;
			pbi->bmiHeader.biSizeImage = 0;
			GlobalUnlock(hmem);
			CDll dll;
			if (dll.LoadLibrary(_T("lsimg.dll"))) {
				BOOL (WINAPI* pfn)(LPCTSTR, HGLOBAL);
				(FARPROC&)pfn = dll.FindFunction("imgPngSave");
				if (pfn)
					bOK = pfn(szFileName, hmem);
			}
			GlobalFree(hmem);
		}
	}
	return bOK;
}

void CopyIconsToClipboard(HICON* phics, int numIcons, COLORREF clrBack)
{
	// DIBメモリの初期化
	int numBits = 32;
	int total_width = 0;
	int height = 0;
	CSimpleArray<CSize> arrSize;
	int i;
	for (i = 0; i < numIcons; i++) {
		BITMAP bm;
		memset(&bm, 0, sizeof(bm));
		ICONINFO ii;
		if (GetIconInfo(phics[i], &ii)) {
			GetObject(ii.hbmColor, sizeof(bm), &bm);
			if (ii.hbmColor) ::DeleteObject(ii.hbmColor);
			if (ii.hbmMask) ::DeleteObject(ii.hbmMask);
		}
		arrSize.Add(CSize(bm.bmWidth, bm.bmHeight));
		numBits = max(numBits, (int)bm.bmBitsPixel);
		total_width += bm.bmWidth;
		height = max(height, bm.bmHeight);
	}

	DWORD size = sizeof(BITMAPINFOHEADER) + WIDTHBYTES(numBits * total_width) * height;
	HGLOBAL hmem = GlobalAlloc(GHND, size);
	BITMAPINFOHEADER* pbh = (BITMAPINFOHEADER*)GlobalLock(hmem);
	pbh->biSize = sizeof(BITMAPINFOHEADER);
	pbh->biWidth = total_width;
	pbh->biHeight = height;
	pbh->biPlanes = 1;
	pbh->biBitCount = numBits;
	pbh->biCompression = BI_RGB;
	void* pbits = (void*)((BYTE*)pbh + sizeof(BITMAPINFOHEADER));

	CClientDC dc(NULL);
	HBITMAP hbm = ::CreateDIBitmap(dc, pbh, CBM_INIT, pbits, (BITMAPINFO*)pbh, DIB_RGB_COLORS);
	CDC dcw;
	dcw.CreateCompatibleDC(dc);

	HBITMAP hbmOld = dcw.SelectBitmap(hbm);
	if (numBits < 32)
		dcw.FillSolidRect(0, 0, total_width, height, clrBack);

	int x = 0;
	for (i = 0; i < numIcons; i++) {
		CSize size = arrSize[i];
		if (size.cx && size.cy) {
			dcw.DrawIconEx(x, 0, phics[i], size.cx, size.cy, 0, NULL, DI_NORMAL);
			x += size.cx;
		}
	}

	// DIBメモリの初期化
	::GetDIBits(dcw, hbm, 0, height, pbits, (BITMAPINFO*)pbh, DIB_RGB_COLORS);

	dcw.SelectBitmap(hbmOld);
	::DeleteObject(hbm);

	GlobalUnlock(hmem);

	if (OpenClipboard(GetActiveWindow())) {
		EmptyClipboard();
		SetClipboardData(CF_DIB, hmem);
		CloseClipboard();
	} else {
		GlobalFree(hmem);
	}
}

typedef struct _MATCHICONINFO {
	bool bMatched;
	int idResource;
	int index;
} MATCHICONINFO;

static BOOL CALLBACK enum_matchicon(HMODULE hmod, LPCTSTR pType, LPTSTR pName, LONG_PTR data)
{
	MATCHICONINFO* pmi = (MATCHICONINFO*)data;
	if (MAKEINTRESOURCE(pmi->idResource) == pName) {
		pmi->bMatched = true;
		return FALSE;
	}
	pmi->index++;
	return TRUE;
}

static int EnumMatchIcon(LPCTSTR pszName, int index)
{
	// リソースIDからアイコンインデクスを取得する
	if (index >= 0) return index;

	MATCHICONINFO mi;
	mi.bMatched = false;
	mi.idResource = abs(index);
	mi.index = 0;
	HMODULE hmod = LoadLibraryEx(pszName, NULL, LOAD_LIBRARY_AS_DATAFILE);
	if (hmod) {
		EnumResourceNames(hmod, RT_GROUP_ICON, enum_matchicon, (LONG_PTR)&mi);
		FreeLibrary(hmod);
	}
	return (mi.bMatched ? mi.index : -1);
}

BOOL GetDefaultIconPath(LPCTSTR pszName, LPTSTR pszIconPath, int* pIndex)
{
	BOOL bOK = FALSE;
	LPITEMIDLIST pidlFq = ILCreateFromPath(pszName);
	if (pidlFq) {
		IShellFolder* psf;
		LPCITEMIDLIST pidl;
		if (SHBindToParent(pidlFq, IID_IShellFolder, (void**)&psf, &pidl) == S_OK) {
			IQueryAssociations* pqa;
			if (NOERROR == psf->GetUIObjectOf(NULL, 1, &pidl, IID_IQueryAssociations, 0, (void**)&pqa)) {
				TCHAR buf[MAX_PATH * 2];
				DWORD len = ARRAYSIZE(buf);
				if (pqa->GetString(0, ASSOCSTR_DEFAULTICON, NULL, buf, &len) == S_OK) {
					int index = PathParseIconLocation(buf);
					if (index == 0 && !lstrcmpi(_T(".ico"), PathFindExtension(buf))) {
						// アイコンファイルそのもの
						*pIndex = INT_MAX;
						lstrcpy(pszIconPath, buf);
						bOK = TRUE;
					} else {
						// 取得した名前の正確なフルパス名を取得
						HMODULE hmod = LoadLibraryEx(buf, NULL, LOAD_LIBRARY_AS_DATAFILE);
						if (hmod) {
							GetModuleFileName(hmod, buf, ARRAYSIZE(buf));
							index = EnumMatchIcon(buf, index);
							if (index >= 0) {
								*pIndex = index;
								lstrcpy(pszIconPath, buf);
								bOK = TRUE;
							}
							FreeLibrary(hmod);
						}
					}
				}
				pqa->Release();
			}
			psf->Release();
		}
		CoTaskMemFree(pidlFq);
	}
	return bOK;
}

///////////////////////////////////////////////////////////////////////////////
// lsfop api entry

#ifdef _LSFOP
#include "common.h"
#include "func.h"

extern "C" int WINAPI fopBrowseForFolder(HWND hParent, LPTSTR lpPath);



extern "C"
HICON WINAPI fopGetIconFromInstance(HMODULE hmod, LPTSTR index, int width, int height)
{
	return GetIconFromInstance(hmod, index, width, height);
}

extern "C"
BOOL WINAPI fopExportIconFromFile(HWND hwnd, LPCTSTR pszName, LPCTSTR pszTo)
{
	TCHAR path[MAX_PATH * 2];
	if (pszTo == NULL || !PathIsDirectory(pszTo)) {
		PathGetParent(pszName, path);
		if (fopBrowseForFolder(hwnd, path) != IDOK) { return FALSE; }
	} else {
		lstrcpy(path, pszTo);
		if (AppMessage(IDS_TITLE_ICONPICKER, IDS_QUERY_ICON_PICKOUT, PathFindFileName(pszName), MB_QYESNO) != IDYES) { return FALSE; }
	}
	int cnt = ExportMultipleIcons(pszName, path, NULL, 0);
	CString s; s.Format(_T("%d"), cnt);
	AppMessage(IDS_TITLE_ICONPICKER, IDS_NUM_OF_ICON_PICKOUT, s.GetString(), MB_INFORMATION);
	return TRUE;
}

extern "C"
BOOL WINAPI fopExportSingleIcon(LPCTSTR pszName, LPCTSTR pszExportIconName, int index)
{
	return ExportSingleIcon(pszName, pszExportIconName, index);
}

extern "C"
BOOL WINAPI fopExportMultipleIcons(LPCTSTR pszName, LPCTSTR pszTo, int* pnIndex, int entries)
{
	return ExportMultipleIcons(pszName, pszTo, pnIndex, entries);
}

extern "C"
HANDLE WINAPI fopIcmOpen(LPCTSTR pszFileName)
{
	CIconListManager* pilm = new CIconListManager;
	pilm->Initialize(pszFileName);
	return (HANDLE)pilm;
}

extern "C"
void WINAPI fopIcmClose(HANDLE h)
{
	CIconListManager* pilm = (CIconListManager*)h;
	if (pilm) { delete pilm; }
}

extern "C"
HIMAGELIST WINAPI fopIcmGetImageList(HANDLE h, int cx)
{
	CIconListManager* pilm = (CIconListManager*)h;
	return pilm ? pilm->GetImageList(cx) : NULL;
}

extern "C"
int WINAPI fopIcmGetIconCount(HANDLE h)
{
	CIconListManager* pilm = (CIconListManager*)h;
	return pilm ? pilm->GetIconCount() : 0;
}

extern "C"
LPCTSTR WINAPI fopIcmGetIconPath(HANDLE h)
{
	CIconListManager* pilm = (CIconListManager*)h;
	return pilm ? pilm->GetIconPath() : NULL;
}


#endif		// _LSFOP
