#pragma once

//////////////////////////////////////////////////////////////////////
//

template <class T>
class CArchiverPageImpl
{
public:
	CPackInfo* m_pd;

	CArchiverPageImpl() : m_pd(NULL)
	{
	}

	BEGIN_MSG_MAP(CArchiverPageImpl)
		MESSAGE_HANDLER(WM_INITDIALOG, OnAPHandlerInitDialog)
		MESSAGE_HANDLER(WM_ON_OK, OnAPHandlerOK)
	END_MSG_MAP()

	LRESULT OnAPHandlerInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_pd = (CPackInfo*)lParam;

		T* pT = static_cast<T*>(this);
		return (LRESULT)pT->OnInitDialog();
	}

	LRESULT OnAPHandlerOK(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		T* pT = static_cast<T*>(this);
		return (LRESULT)pT->OnOK();
	}

	// overridable
	virtual BOOL OnInitDialog()
	{
		return TRUE;
	}

	// overridable
	virtual BOOL OnOK()
	{
		return TRUE;
	}
};

