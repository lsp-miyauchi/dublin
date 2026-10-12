////////////////////////////////////////////////////////////////////////
// dpiaware.h

#pragma once
#ifndef __DPIAWARE_H__
#define __DPIAWARE_H__

class CDPIAware
{
public:
	CDPIAware() : _fInitialized(false), _dpiX(96), _dpiY(96) { }
	// 画面の DPI を取得します。
	int GetDPIX() { _Init(); return _dpiX; }
	int GetDPIY() { _Init(); return _dpiY; }
	// 絶対ピクセルと相対ピクセル間の変換を行います。
	int ScaleX(int x) { _Init(); return MulDiv(x, _dpiX, 96); }
	int ScaleY(int y) { _Init(); return MulDiv(y, _dpiY, 96); }
	int UnscaleX(int x) { _Init(); return MulDiv(x, 96, _dpiX); }
	int UnscaleY(int y) { _Init(); return MulDiv(y, 96, _dpiY); }

	// 画面サイズ (相対ピクセル単位) を求めます。
	int ScaledScreenWidth() { return _ScaledSystemMetricX(SM_CXSCREEN); }
	int ScaledScreenHeight() { return _ScaledSystemMetricY(SM_CYSCREEN); }
	// 四角形のサイズを絶対ピクセルから相対ピクセルに変更します。
	void ScaleRect(__inout RECT *pRect)
	{
		pRect->left = ScaleX(pRect->left);
		pRect->right = ScaleX(pRect->right);
		pRect->top = ScaleY(pRect->top);
		pRect->bottom = ScaleY(pRect->bottom);
	}
	void UnscaleRect(__inout RECT *pRect)
	{
		pRect->left = UnscaleX(pRect->left);
		pRect->right = UnscaleX(pRect->right);
		pRect->top = UnscaleY(pRect->top);
		pRect->bottom = UnscaleY(pRect->bottom);
	}
	void ScaleSize(__inout SIZE *pSize)
	{
		pSize->cx = ScaleX(pSize->cx);
		pSize->cy = ScaleX(pSize->cy);
	}
	void UnscaleSize(__inout SIZE *pSize)
	{
		pSize->cx = UnscaleX(pSize->cx);
		pSize->cy = UnscaleX(pSize->cy);
	}
	// 画面解像度が最低値 (相対ピクセル単位) を満たしているかどうかを確認します。
	bool IsResolutionAtLeast(int cxMin, int cyMin)
	{
		return (ScaledScreenWidth() >= cxMin) && (ScaledScreenHeight() >= cyMin); 
	}
	// ポイント サイズ (1/72 インチ) を絶対ピクセルに変換します。
	int PointsToPixels(int pt) { return MulDiv(pt, _dpiY, 72); }
	// キャッシュされたメトリックをすべて無効にします。
	void Invalidate() { _fInitialized = false; }

private:
	void _Init()
	{
		if (!_fInitialized) {
			HDC hdc = GetDC(NULL);
			if (hdc) {
				_dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
				_dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
				ReleaseDC(NULL, hdc);
			}
			_fInitialized = true;
		}
	}

	int _ScaledSystemMetricX(int nIndex) { _Init(); return MulDiv(GetSystemMetrics(nIndex), 96, _dpiX); }
	int _ScaledSystemMetricY(int nIndex) { _Init(); return MulDiv(GetSystemMetrics(nIndex), 96, _dpiY); }

private:
	bool _fInitialized;
	int _dpiX;
	int _dpiY;
};


#endif		// __DPIAWARE_H__
