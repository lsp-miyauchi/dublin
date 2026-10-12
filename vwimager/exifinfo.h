////////////////////////////////////////////////////////////////////////
// exifinfo.h

#pragma once
#ifndef __EXIFINFO_H__
#define __EXIFINFO_H__

#include "..\lspexif\lspexifapi.h"
#include "mousewheeler.h"
#include "threadbase.h"


////////////////////////////////////////////////////////////////////////
//

class CExifGroup : public CSimpleMap<CString, CString>
{
public:
	CString m_strTitle;

	CExifGroup()
	{
	}

	CExifGroup(const CExifGroup& src)
	{
		m_strTitle = src.m_strTitle;
		if (src.GetSize() > 0) {
			for (int i = 0; i < src.GetSize(); i++) {
				Add(src.GetKeyAt(i), src.GetValueAt(i));
			}
		}
	}

	CExifGroup& operator=(const CExifGroup& src)
	{
		if (&src != this) {
			RemoveAll();
			m_strTitle = src.m_strTitle;
			if (src.GetSize() > 0) {
				for (int i = 0; i < src.GetSize(); i++) {
					Add(src.GetKeyAt(i), src.GetValueAt(i));
				}
			}
		}
		return *this;
	}

	void SetTitle(LPCTSTR pszTitle)
	{
		m_strTitle = pszTitle;
	}
};

///////////////////////////////////////////////////////////////////////
//


class CExifInfoThread : public CThreadBaseImpl<CExifInfoThread>
{
private:
	CString m_fname;

public:
	CSimpleArray<CExifGroup> m_info;

	bool OnStart(LPARAM param)
	{
		m_fname = (LPCTSTR)param;
		m_info.RemoveAll();
		return true;
	}

	void OnStop()
	{
	}

	unsigned Run()
	{
		CoInitialize(NULL);
		CComPtr<IExifParser> pep;
		HRESULT hr = pep.CoCreateInstance(__uuidof(ExifParser));
		if (SUCCEEDED(hr)) {
			pep->EnumExifInfo(m_fname, (UINT_PTR)&CExifInfoThread::ExifProc, (LPARAM)this);
			PostItemNotify(0);
		}
		CoUninitialize();
		return 0;
	}

protected:
	static int CALLBACK ExifProc(int type, LPCTSTR pszTitle, LPCTSTR pszText, LPARAM param)
	{
		CExifInfoThread* parser = (CExifInfoThread*)param;
		if (parser->IsCanceled())
			return 0;

		switch (type) {
		case EXIFCB_GROUP:
			{
			CExifGroup group;
			group.SetTitle(pszTitle);
			parser->m_info.Add(group);
			break;
			}
		case EXIFCB_ITEM:
			{
			int cnt = parser->m_info.GetSize();
			if (cnt > 0)
				parser->m_info[cnt - 1].Add(pszTitle, pszText);
			break;
			}
		}
		return 1;
	}
};


///////////////////////////////////////////////////////////////////////
//

class CExifView : public CMWListViewCtrl
{
private:
	CFont m_font;
	UINT m_message;
	CExifInfoThread m_exifparser;
	DWORD m_magic;
	CString m_strEmptyMessage;

public:
	CExifView()
		: m_message(RegisterWindowMessage(_T("wm_getexifinfo")))
		, m_magic(0)
	{
	}

BEGIN_MSG_MAP(CExifView)
	CHAIN_MSG_MAP(CMWListViewCtrl)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnHandlerEraseBkgnd)
	ON_REGISTERD_MESSAGE(m_message, OnGetExifInfo)
END_MSG_MAP()

	LRESULT OnHandlerEraseBkgnd(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT lr = 0;
		bHandled = EraseBackground((HDC)wParam, &lr);
		return lr;
	}

	virtual BOOL EraseBackground(HDC hdc, LRESULT* pResult)
	{
		if (GetItemCount() == 0 && !m_strEmptyMessage.IsEmpty()) {
			CRect rc;
			GetClientRect(&rc);
			CDCHandle dc = hdc;
			dc.FillSolidRect(&rc, GetSysColor(COLOR_WINDOW));

			HFONT hfont = dc.SelectFont(AtlGetStockFont(DEFAULT_GUI_FONT));
			TEXTMETRIC tm;
			dc.GetTextMetrics(&tm);
			rc.top += tm.tmHeight * 2;
			rc.bottom = rc.top + max(16, tm.tmHeight);

			HICON hic = (HICON)LoadImage(GetModuleHandle(_T("user32.dll")), MAKEINTRESOURCE(101), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
			CSize size;
			dc.GetTextExtent(m_strEmptyMessage, m_strEmptyMessage.GetLength(), &size);

			rc.left += (rc.Width() - size.cx - 16) / 2;
			dc.DrawIconEx(rc.left, rc.top + (rc.Height() - 16) / 2, hic, 0, 0, 0, NULL, DI_NORMAL);
			rc.left += 18;
			dc.DrawText(m_strEmptyMessage, -1, &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

			DestroyIcon(hic);
			dc.SelectFont(hfont);
			return TRUE;
		}
		return FALSE;
	}

	LRESULT OnGetExifInfo(WPARAM wParam, LPARAM lParam);

	void Initialize();
	void ParseExif(LPCTSTR pszImage);
	void ProcessCustomDraw(NMLVCUSTOMDRAW* pcd, LRESULT* pResult);

};


#endif			// __EXIFINFO_H__
