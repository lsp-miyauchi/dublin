/////////////////////////////////////////////////////////////////////////////
// CUndo

#pragma once

#ifndef __UNDO_H__
#define __UNDO_H__

#define MAXUNDO					16


/////////////////////////////////////////////////////////////////////////////
//

class CUndo
{
public:
	CUndo()
	{
		m_csOperation = _T("");
		m_hdib = NULL;
	}
	CUndo(LPCTSTR pszOperation, HDIB hdib)
	{
		m_csOperation = pszOperation;
		m_hdib = hdib;
	}
	CUndo(UINT nIDCommand, HDIB hdib)
	{
		CString str;
		str.LoadString(nIDCommand);
		LPCTSTR p = StrChr(str, '\n');
		if (!p) p = str;
		else p++;
		m_csOperation = p;
		m_hdib = hdib;
	}
	~CUndo()
	{
		if (m_hdib) GlobalFree(m_hdib);
	}

	HDIB DetachDIB()
	{
		HDIB hdib = m_hdib;
		m_hdib = NULL;
		return hdib;
	}

	CString m_csOperation;
	HDIB m_hdib;
	CRect m_rc;
};


#endif			// __UNDO_H__
