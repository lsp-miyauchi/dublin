/////////////////////////////////////////////////////////////////////
// dropfiles.h

#pragma once


/////////////////////////////////////////////////////////////////////
// CDropFiles

class CDropFiles
{
public:
	CDropFiles()
	{
		m_hDrop = NULL;
		m_nEntry = 0;
		m_nFolder = -1;
	}

	~CDropFiles()
	{
		if (m_hDrop) ::DragFinish(m_hDrop);
	}

	HDROP Attach(HDROP hDrop)
	{
		if (hDrop == NULL || hDrop == m_hDrop) return NULL;

		HDROP hDropTemp = m_hDrop;
		m_hDrop = hDrop;
		if (m_hDrop)
			m_nEntry = ::DragQueryFile(m_hDrop, 0xFFFFFFFF, NULL, 0);
		else
			m_nEntry = 0;
		return hDropTemp;
	}

	HDROP Detach()
	{
		HDROP hDropTemp = m_hDrop;
		m_nEntry = 0;
		m_hDrop = NULL;
		return hDropTemp;
	}

	void DragFinish()
	{
		if (m_hDrop)
			::DragFinish(m_hDrop);
		m_hDrop = NULL;
		m_nEntry = 0;
	}

	BOOL DragQueryFile(int index, LPTSTR pszFile, int cchMax)
	{
		*pszFile = 0;
		if (m_hDrop && index >= 0 && index < m_nEntry) {
			::DragQueryFile(m_hDrop, index, pszFile, cchMax);
			return TRUE;
		}
		return FALSE;
	}

	int DragQueryFile(int index, CString& name)
	{
		name.Empty();
		TCHAR buf[MAX_PATH * 2];
		if (DragQueryFile(index, buf, ARRAYSIZE(buf)))
			name = buf;
		return name.GetLength();
	}

	BOOL IsFolderInclude()
	{
		if (m_hDrop) {
			if (m_nFolder < 0) {
				m_nFolder = 0;
				TCHAR buf[MAX_PATH * 2];
				for (int i = 0; i < m_nEntry; i++) {
					::DragQueryFile(m_hDrop, i, buf, ARRAYSIZE(buf));
					if (PathIsDirectory(buf)) {
						m_nFolder = 1;
						break;
					}
				}
			}
		}
		return (m_nFolder > 0);
	}

	__inline operator HDROP() const { return m_hDrop; }
	__inline int GetCount() { return m_nEntry; }

	HDROP m_hDrop;
	int m_nEntry;
	int m_nFolder;
};

