////////////////////////////////////////////////////////////////////////
// CreateInterShortcut.cpp
//
// This article shows you how to create an Internet shortcut for Windows 95, 98,
// and NT computers. Creating internet shortcut is little different than
// the general shortcut, so the code for Internet shortcut creation is shown.
//
// pszURL - url location e.g 'http://www.sample.com/index/html'
//          Note: You can use a syntax such as file://Arguments for local files.
// pszURLFileName - shortcut filename, should have extension of .url otherwise does not work.
//          e.g ;"C:\test.url"
// LPCSTR szDescription - description of the shortcut
// LPCTSTR szIconFile - icon file(could any .exe ot .dll)
// int index - index of the icon resource in above .exe or .dll
//


HRESULT CreateInterShortcut (LPCSTR pszURL, LPCSTR pszURLfilename,
	LPCSTR szDescription,LPCTSTR szIconFile = NULL,int nIndex = -1)
{
	HRESULT hres;
	CoInitialize(NULL);

	IUniformResourceLocator *pHook;

	hres = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER,
							IID_IUniformResourceLocator, (void **)&pHook);

	if (SUCCEEDED(hres)) {
		IPersistFile *ppf;
		IShellLink *psl;

		// Query IShellLink for the IPersistFile interface for
		hres = pHook->QueryInterface (IID_IPersistFile, (void **)&ppf);
		hres = pHook->QueryInterface (IID_IShellLink, (void **)&psl);

		if (SUCCEEDED (hres)) {
			WORD wsz [MAX_PATH]; // buffer for Unicode string

			// Set the path to the shortcut target.
			pHook->SetURL(pszURL,0);

			hres = psl->SetIconLocation(szIconFile,nIndex);
			if (SUCCEEDED (hres)) {
				// Set the description of the shortcut.
				hres = psl->SetDescription (szDescription);
				if (SUCCEEDED (hres)) {
					// Ensure that the string consists of ANSI characters.
					MultiByteToWideChar (CP_ACP, 0, pszURLfilename, -1, wsz, MAX_PATH);

					// Save the shortcut via the IPersistFile::Save member function.
					hres = ppf->Save (wsz, TRUE);
				}
			}

			// Release the pointer to IPersistFile.
			ppf->Release ();
			psl->Release ();
		}

		// Release the pointer to IShellLink.
		pHook->Release ();
	}
	return hres;
}
