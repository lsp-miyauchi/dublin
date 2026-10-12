#pragma once

__inline HRESULT IImageList_DrawIcon(IImageList* pil, HDC hdc, int iImage, const POINT& pos, const SIZE& size, DWORD style, DWORD state = ILS_NORMAL)
{
	IMAGELISTDRAWPARAMS idp;
	memset(&idp, 0, sizeof(idp));
	idp.cbSize = sizeof(idp);
	idp.i = iImage;
	idp.hdcDst = hdc;
	idp.rgbBk = CLR_NONE; 
	idp.rgbFg = CLR_DEFAULT;
	idp.fStyle = style;
	idp.fState = state;
	idp.x = pos.x;
	idp.y = pos.y;
	if (!size.cx || !size.cy)
		pil->GetIconSize(&idp.cx, &idp.cy);
	else {
		idp.cx = size.cx;
		idp.cy = size.cy;
	}
	return pil->Draw(&idp);
}

__inline HRESULT IImageList_DrawIcon(int imageListType, HDC hdc, int iImage, const POINT& pos, const SIZE& size, DWORD style, DWORD state = ILS_NORMAL)
{
	CComPtr<IImageList> pil;
	SHGetImageList(imageListType, IID_PPV_ARGS(&pil));
	return IImageList_DrawIcon(pil, hdc, iImage, pos, size, style, state);
}

__inline void IImageList_GetIconSize(int imageType, SIZE& s)
{
	CComPtr<IImageList> pil;
	if (SHGetImageList(imageType, IID_PPV_ARGS(&pil)) == S_OK) {
		int cx, cy;
		pil->GetIconSize(&cx, &cy);
		s.cx = cx;
		s.cy = cy;
	}
}
