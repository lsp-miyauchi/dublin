////////////////////////////////////////////////////////////////////////////////
// loaderthread.h

#pragma once
#include "threadbase.h"


////////////////////////////////////////////////////////////////////////
//

class CLoaderThread : public CThreadBaseImpl<CLoaderThread>
{
private:
	CString m_fname;
public:

	bool OnStart(LPARAM param)
	{
		m_fname = (LPCTSTR)param;
		return true;
	}

	void OnStop()
	{
		m_fname.Empty();
	}

	unsigned Run()
	{
		Lock();
		HDIB hdib = imgLoadImage(m_fname);
		if (!PostItemNotify((LPARAM)hdib))
			GlobalFree(hdib);
		Unlock();
		return 0;
	}
};

