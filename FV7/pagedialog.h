////////////////////////////////////////////////////////////////////////
// pagedialog.h

#pragma once
#ifndef __PAGEDIALOG_H__
#define __PAGEDIALOG_H__

/////////////////////////////////////////////////////////////////////////////
//

class CPageDialogHandler
{
protected:
	BOOL m_bUpdate;

public:
	CPageDialogHandler() : m_bUpdate(FALSE) { }

	BOOL isChanged() { return m_bUpdate; }

	virtual BOOL OnApply() = 0;
	virtual void OnHelpInfo() { }
};


/////////////////////////////////////////////////////////////////////////////
//

class CPageDialog : public CWindow
{
public:
	UINT m_idd;
	CPageDialogHandler* m_handler;

	CPageDialog(UINT idd, CPageDialogHandler* handler) : m_idd(idd), m_handler(handler)
	{
		ATLASSERT(m_idd != 0);
		ATLASSERT(m_handler != NULL);
	}

	inline CPageDialogHandler* GetHandler() { return m_handler; }
};


#endif		// __PAGEDIALOG_H__
