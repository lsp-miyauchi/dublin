////////////////////////////////////////////////////////////////
// printinfo.h


#pragma once
#ifndef __PRINTFINO_H__
#define __PRINTFINO_H__

////////////////////////////////////////////////////////////////
//

struct CPrintInfo // Printing information structure
{
	CPrintInfo()
	{
		m_pPD = new CPrintDialog(FALSE, PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOSELECTION);

		SetMinPage(1);              // one based page numbers
		SetMaxPage(0xffff);         // unknown how many pages

		m_nCurPage = 1;
		m_nJobNumber = SP_ERROR;

		m_lpUserData = NULL;        // Initialize to no user data
		m_bPreview = FALSE;         // initialize to not preview
		m_bDirect = FALSE;          // initialize to not direct
		m_bDocObject = FALSE;       // initialize to not IPrint
		m_bContinuePrinting = TRUE; // Assume it is OK to print

		m_dwFlags = 0;
		m_nOffsetPage = 0;
	}
	~CPrintInfo()
	{
		if (m_pPD != NULL && m_pPD->m_pd.hDC != NULL) {
			::DeleteDC(m_pPD->m_pd.hDC);
			m_pPD->m_pd.hDC = NULL;
		}
		delete m_pPD;
	}

	CPrintDialog* m_pPD;     // pointer to print dialog

	BOOL m_bDocObject;       // TRUE if printing by IPrint interface
	BOOL m_bPreview;         // TRUE if in preview mode
	BOOL m_bDirect;          // TRUE if bypassing Print Dialog
	BOOL m_bContinuePrinting;// set to FALSE to prematurely end printing
	UINT m_nCurPage;         // Current page
	UINT m_nNumPreviewPages; // Desired number of preview pages
	CString m_strPageDesc;   // Format string for page number display
	LPVOID m_lpUserData;     // pointer to user created struct
	CRect m_rectDraw;        // rectangle defining current usable page area
	int m_nJobNumber;			 // job number (after StartDoc)

	// these only valid if m_bDocObject
	UINT m_nOffsetPage;      // offset of first page in combined IPrint job
	DWORD m_dwFlags;         // flags passed to IPrint::Print

	void SetMinPage(UINT nMinPage)
	{
		m_pPD->m_pd.nMinPage = (WORD)nMinPage;
	}
	void SetMaxPage(UINT nMaxPage)
	{
		m_pPD->m_pd.nMaxPage = (WORD)nMaxPage;
	}
	UINT GetMinPage() const
	{
		return m_pPD->m_pd.nMinPage;
	}
	UINT GetMaxPage() const
	{
		return m_pPD->m_pd.nMaxPage;
	}
	UINT GetFromPage() const
	{
		return m_pPD->m_pd.nFromPage;
	}
	UINT GetToPage() const
	{
		return m_pPD->m_pd.nToPage;
	}
	UINT GetOffsetPage() const;
};


////////////////////////////////////////////////////////////////
//

typedef struct tagPRINTOPTION {
	BOOL appraster;
	int zoom;
	CPrintInfo info;
} PRINTOPTION;

#endif			// __PRINTFINO_H__
