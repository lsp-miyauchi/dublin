#include "stdafx.h"
#include "fv7.h"

#ifndef __STATUS_H__
	#include "status.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//@ #define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
//

IMPLEMENT_DYNAMIC(CStatBar, CStatusBar)

/////////////////////////////////////////////////////////////////////////////
// CStatBar message handlers

BEGIN_MESSAGE_MAP(CStatBar, CStatusBar)
	//{{AFX_MSG_MAP(CStatBar)
	// control messages
	ON_MESSAGE(WM_SETTEXT, OnSetText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
