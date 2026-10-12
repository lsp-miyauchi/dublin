//////////////////////////////////////////////////////////////////////
//

#pragma once
#ifndef __FILTERDIALOG_H__
#define __FILTERDIALOG_H__

#ifndef __ATL_SPECIAL_HANDLER_H__
	#error filterdialog.h requires atl_special_handlers.h to be included first
#endif

#include "resource.h"

template <class T>
class CFilterDialogImpl
{
public:
	CWindow m_view;

BEGIN_MSG_MAP(CFilterDialogImpl< T >)
	ON_WM_INITDIALOG()
	ON_WM_HSCROLL()
	ON_NOTIFY(MVN_GETEFFECT, IDC_PREVIEW, OnNotifyPreview)
	ON_BN_CLICKED(IDOK, OnHandlerOK)
	ON_BN_CLICKED(IDCANCEL, OnHandlerCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam)
	{
		T* pT = static_cast<T*>(this);
		pT->ExecuteDlgInit(pT->IDD);
		pT->CenterWindow();
		m_view = pT->GetDlgItem(IDC_PREVIEW);
		return pT->InitDialog(lParam);
	}

	void OnHScroll(UINT nSBCode, UINT nPos, HWND hScrollBar)
	{
		T* pT = static_cast<T*>(this);
		CTrackCtrl track = hScrollBar;
		int val;
		switch (nSBCode) {
			case TB_THUMBTRACK: val = (int)nPos; break;

			case TB_TOP:
			case TB_BOTTOM:
			case TB_LINEUP:
			case TB_LINEDOWN:
			case TB_PAGEUP:
			case TB_PAGEDOWN: val = track.GetPos(); break;

			default: pT->DefWindowProc(); return;
		}
		pT->HScroll(val, hScrollBar);
	}

	void OnNotifyPreview(NMHDR* pNMHDR, LRESULT* pr)
	{
		T* pT = static_cast<T*>(this);
		MV_GETEFFECT* pmg = (MV_GETEFFECT*)pNMHDR;
		if (pmg->hdibSrc) {
			pmg->hdibDst = pT->GetPreview(pmg->hdibSrc);
		}
		*pr = TRUE;
	}

	void OnHandlerOK()
	{
		T* pT = static_cast<T*>(this);
		pT->OnOK();
	}

	void OnHandlerCancel()
	{
		T* pT = static_cast<T*>(this);
		pT->OnCancel();
	}

	virtual BOOL InitDialog(LPARAM lParam)
	{
		return TRUE;
	}

	virtual void HScroll(int val, HWND hScrollBar)
	{
	}

	virtual void OnOK()
	{
		T* pT = static_cast<T*>(this);
		pT->EndDialog(IDOK);
	}

	virtual void OnCancel()
	{
		T* pT = static_cast<T*>(this);
		pT->EndDialog(IDCANCEL);
	}

	virtual HDIB GetPreview(HDIB hdib)
	{
		return NULL;
	}

	void SetWholePreview(BOOL bWhole)
	{
		m_view.SendMessage(IVM_SETWHOLEVIEW, bWhole, 0);
	}

	void SetPreviewImage(HDIB hdib)
	{
		m_view.SendMessage(IVM_SETDIB, TRUE, (LPARAM)hdib);
	}

	void UpdatePreview()
	{
		m_view.SendMessage(IVM_UPDATEEFFECT, TRUE, 0);
	}
};


#endif			// __FILTERDIALOG_H__
