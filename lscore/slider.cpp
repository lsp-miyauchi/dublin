///////////////////////////////////////////////////////
// SLIDER control implementation

#include "stdafx.h"
#include "resource.h"
#include "lscore.h"
#include "muldiv32.h"
#include "fnc.h"
#include "common.h"
#include "slider.h"


////////////////////////////////////////////////////////////////////////////
//

extern "C"
LRESULT CALLBACK SliderWndFn(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lr = DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_NCCREATE) {
		CSlider* pWnd = new CSlider;
		pWnd->SubclassWindow(hwnd);
	}
	return lr;
}

