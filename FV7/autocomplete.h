////////////////////////////////////////////////////////////////////////
// autocomplete.h

#pragma once

class CAutoComplete
{
private:
	IAutoComplete* m_pac;
	IUnknown* m_punk;

public:
	CAutoComplete()
	{
		m_pac = NULL;
		m_punk = NULL;
	}

	~CAutoComplete()
	{
		Term();
	}

	bool Init(HWND hwndEdit)
	{
		Term();
		bool ok = false;
		if (CoCreateInstance(CLSID_AutoComplete, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pac)) == S_OK) {
			if (CoCreateInstance(CLSID_ACListISF, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_punk)) == S_OK) {
				if (m_pac->Init(hwndEdit, m_punk, NULL, NULL) == S_OK) {
					ok = true;
				}
			}
		}
		if (!ok)
			Term();
		return ok;
	}

	void Term()
	{
		if (m_punk) { m_punk->Release(); m_punk = NULL; }
		if (m_pac) { m_pac->Release(); m_pac = NULL; }
	}
};
