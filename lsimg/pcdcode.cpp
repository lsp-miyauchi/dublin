///////////////////////////////////////////////////////////////////////
// pcdcode.cpp


#include "stdafx.h"
#include "lsimg.h"
#include "common.h"
#include "filemap.h"
#include "libpcd\pcd.h"

extern "C"
HDIB WINAPI imgPcdLoadEx(LPCTSTR pszFile, int subimage)
{
	// subimage	サイズ			ピクセル数		使用メモリ数 
	// 1		1/16BASE		128×192		72KB
	// 2		1/4BASE			256×384		288KB
	// 3		BASE			512×768		1.2MB
	// 4		4BASE			1024×1536		4.5MB
	// 5		16BASE			2048×3072		18MB
	// 6		64BASE			4096×6144		72MB
	// Determine resolution by subimage specification.

	if (subimage < 1 || subimage > 6) return NULL;

	HDIB hdib = NULL;
	PCD_IMAGE* pcd = new PCD_IMAGE;
	if (pcd_open(pcd, pszFile) == 0) {
		int rot = pcd_get_rot(pcd, 0);
		subimage = min(subimage, pcd_get_maxres(pcd));
		bool success = true;
		int width, height;
		if (pcd_select(pcd, subimage, 0, rot, &width, &height) != -1) {
			if (pcd_decode(pcd ) != -1) {
				hdib = dib_create(width, height, 24);
				CDibHandle dib(hdib);
				for (int y = 0; y < height; y++) {
//@					BYTE* p = (BYTE*)dib.BitmapBits() + dib.WidthBytes() * (height - y - 1);
					BYTE* p = (BYTE*)dib.RowPosition(y);
					if (pcd_get_image_line(pcd, y, p) == -1) {
						success = false;
						break;
					}
				}
				hdib = dib.Detach();
				if (!success)
					dib_destroy(hdib);
			}
		}
		pcd_close(pcd);
		delete pcd;
    }
	return hdib;
}

extern "C"
HDIB WINAPI imgPcdLoad(LPCTSTR pszFile)
{
	return imgPcdLoadEx(pszFile, 5);
}

