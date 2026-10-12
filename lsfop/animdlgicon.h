////////////////////////////////////////////////////////////////////////
// animdlgicon.h ダイアログアイコンのアニメーション

#pragma once

#ifndef __ANIMDLGICON_H__
#define __ANIMDLGICON_H__


////////////////////////////////////////////////////////////////////////
//

#define ADI_TIMERID			1128

template < class T >
class CAnimateDialogIcon
{
private:
	CIconHandle m_icAdiPrev;
	int m_indexAdi;
	CSimpleArray<CIcon> m_arrAdi;
	UINT_PTR m_timerAdi;

public:
	CAnimateDialogIcon()
		: m_indexAdi(-1)
		, m_timerAdi(0)
	{}

	~CAnimateDialogIcon()
	{
		if (m_indexAdi >= 0)
			stopDialogAnimation();
	}

// Message map and handlers
	BEGIN_MSG_MAP(CAnimateDialogIcon)
		MESSAGE_HANDLER(WM_TIMER, OnADIHandleTimer)
	END_MSG_MAP()

	LRESULT OnADIHandleTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (wParam == m_timerAdi) {
			stepDialogAnimation();
			return 1;
		}
		bHandled = FALSE;
		return 1;
	}

	void startDialogAnimation(UINT idIconStart, int numIcons)
	{
		ATLASSERT(idIconStart != 0);
		ATLASSERT(numIcons >= 0);

		int i;
		for (i = 0; i < numIcons; i++) {
			HICON hic = ::LoadIcon(RESINST, MAKEINTRESOURCE(i + idIconStart));
			if (hic)
				m_arrAdi.Add(hic);
		}

		if (m_arrAdi.GetSize() > 0) {
			T* pT= (T*)this;
			if (pT->IsWindow()) {
				m_icAdiPrev.Attach((HICON)pT->SendMessage(WM_GETICON, ICON_SMALL, 0));
				m_timerAdi = pT->SetTimer(ADI_TIMERID, 200);
				stepDialogAnimation();
			}
		}
	}

	void stepDialogAnimation()
	{
		int num = m_arrAdi.GetSize();
		if (num > 0) {
			T* pT= (T*)this;
			if (pT->IsWindow()) {
				m_indexAdi++;
				if (m_indexAdi >= num)
					m_indexAdi = 0;
				pT->SendMessage(WM_SETICON, ICON_SMALL, (LPARAM)m_arrAdi[m_indexAdi].m_hIcon);
			}
		}
	}

	void stopDialogAnimation()
	{
		T* pT= (T*)this;
		if (pT->IsWindow()) {
			if (m_timerAdi)
				pT->KillTimer(m_timerAdi);
			if (!m_icAdiPrev.IsNull())
				pT->SendMessage(WM_SETICON, ICON_SMALL, (LPARAM)m_icAdiPrev.Detach());
			m_timerAdi = 0;
			m_indexAdi = -1;
		}
	}
};


#endif			// __ANIMDLGICON_H__
