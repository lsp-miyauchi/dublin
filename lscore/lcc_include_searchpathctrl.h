///////////////////////////////////////////////////////////////////////////////
// CLccSearchPathCtrl

#pragma once
#ifndef __lcc_include_searchpathctrl_h__
#define __lcc_include_searchpathctrl_h__


template <class TBase>
class CLccSearchPathCtrlT : public TBase
{
public:
// Constructors
	CLccSearchPathCtrlT(HWND hWnd = NULL) : TBase(hWnd)
	{ }

	CLccSearchPathCtrlT< TBase >& operator =(HWND hWnd)
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

// Attributes
	static LPCTSTR GetWndClassName()
	{
		return WC_LSC_SEARCHPATH;
	}

	// for entire combo box
	int GetCount() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, SEM_GETCOUNT, 0, 0L);
	}

	int GetCurSel() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, SEM_GETCURSEL, 0, 0L);
	}

	int SetCurSel(int nSelect)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, SEM_SETCURSEL, nSelect, 0L);
	}

	void ResetContent()
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, SEM_RESETCONTENT, 0, 0L);
	}

	// for drop-down combo boxes
	void ShowDropDown(BOOL bShowIt = TRUE)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, SEM_SHOWDROPDOWN, bShowIt, 0L);
	}

	int AddString(LPCTSTR lpszString, int iImage = -1)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, SEM_ADDSTRING, (WPARAM)iImage, (LPARAM)lpszString);
	}

	int DeleteString(UINT nIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, SEM_DELETESTRING, nIndex, 0L);
	}

	int InsertString(int nIndex, LPCTSTR lpszString, int iImage = -1)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		nIndex = (int)::SendMessage(m_hWnd, SEM_INSERTSTRING, nIndex, (LPARAM)lpszString);
		if (nIndex >= 0) {
			::SendMessage(m_hWnd, SEM_SETITEMIMAGE, nIndex, (LPARAM)iImage);
		}
		return (LRESULT)nIndex;
	}

	int FindString(int nStartAfter, LPCTSTR lpszString) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, SEM_FINDSTRING, nStartAfter, (LPARAM)lpszString);
	}

	int FindStringExact(int nStartAfter, LPCTSTR lpszString) const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, SEM_FINDSTRINGEXACT, nStartAfter, (LPARAM)lpszString);
	}

	int SelectString(int nStartAfter, LPCTSTR lpszString)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, SEM_SELECTSTRING, nStartAfter, (LPARAM)lpszString);
	}

	int SelectStringExact(int nStartAfter, LPCTSTR lpszString)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (int)::SendMessage(m_hWnd, SEM_SELECTSTRINGEXACT, nStartAfter, (LPARAM)lpszString);
	}

	DWORD GetEditSel() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (DWORD)::SendMessage(m_hWnd, SEM_GETEDITSEL, 0, 0L);
	}

	BOOL SetEditSel(int nStartChar, int nEndChar)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (BOOL)::SendMessage(m_hWnd, SEM_SETEDITSEL, 0, MAKELONG(nStartChar, nEndChar));
	}

	void SetReadOnly(BOOL bReadOnly)
	{
		ATLASSERT(::IsWindow(m_hWnd));
		::SendMessage(m_hWnd, SEM_SETREADONLY, (WPARAM)bReadOnly, 0);
	}

	HIMAGELIST GetImageList() const
	{
		ATLASSERT(::IsWindow(m_hWnd));
		return (HIMAGELIST)::SendMessage(m_hWnd, SEM_GETIMAGELIST, 0, 0);
	}

	HIMAGELIST SetImageList(HIMAGELIST hImageList)
	{
		ATLASSERT(::IsWindow(m_hWnd)); 
		return (HIMAGELIST)::SendMessage(m_hWnd, SEM_SETIMAGELIST, 0, (LPARAM)hImageList);
	}

	void SetItemImage(int nIndex, int iImage)
	{
		ATLASSERT(::IsWindow(m_hWnd)); 
		::SendMessage(m_hWnd, SEM_SETITEMIMAGE, nIndex, (LPARAM)iImage);
	}

	int GetItemImage(int nIndex)
	{
		ATLASSERT(::IsWindow(m_hWnd)); 
		::SendMessage(m_hWnd, SEM_GETITEMIMAGE, nIndex, 0);
	}

};

typedef CLccSearchPathCtrlT<CWindow>   CLccSearchPathCtrl;


#endif		// __lcc_include_searchpathctrl_h__
