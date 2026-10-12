/////////////////////////////////////////////////////////////////////////
// bar_container.h

#pragma once
#ifndef __BAR_CONTAINER_H__
#define __BAR_CONTAINER_H__


template <class TBase>
class CBarContainerT : public TBase
{
public:
// Constructors
	CBarContainerT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CBarContainerT< TBase >& operator =(HWND hWnd)
	{
		m_hWnd = hWnd;
		return *this;
	}

	HWND Create(HWND hWndParent, ATL::_U_RECT rect = NULL, LPCTSTR szWindowName = NULL,
			DWORD dwStyle = 0, DWORD dwExStyle = 0,
			ATL::_U_MENUorID MenuOrID = 0U, LPVOID lpCreateParam = NULL)
	{
		return TBase::Create(GetWndClassName(), hWndParent, rect.m_lpRect, szWindowName, dwStyle, dwExStyle, MenuOrID.m_hMenu, lpCreateParam);
	}
};

typedef CBarContainerT<ATL::CWindow>   CBarContainer;


#endif		// __BAR_CONTAINER_H__
