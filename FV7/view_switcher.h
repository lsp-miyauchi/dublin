#pragma once

/////////////////////////////////////////////////////////////////////////////
//

class CViewSwitcher : public CAtlArray<HWND>
{
private:
	int m_curIndex;

public:
	CViewSwitcher() : m_curIndex(-1) { }

	HWND GetCurrentView()
	{
		return (m_curIndex < 0) ? NULL : GetAt(m_curIndex);
	}

	void GiveFocus()
	{
		HWND hwndCurrent = GetCurrentView();
		if (::IsWindow(hwndCurrent))
			::SetFocus(hwndCurrent);
	}

	HWND ActivateNextView(BOOL bNext)
	{
		// リストされている次のウィンドウにフォーカスする
		int cnt = (int)GetCount();
		ATLASSERT(cnt > 0);
		if (cnt > 0) {
			if (m_curIndex < 0) {
				m_curIndex = 0;
			}
			else {
				for (int i = 0; i < cnt; i++) {
					if (bNext) {
						m_curIndex++;
						if (m_curIndex >= cnt) m_curIndex = 0;
					}
					else {
						m_curIndex--;
						if (m_curIndex < 0) m_curIndex = cnt - 1;
					}
					HWND hwnd = GetAt(m_curIndex);
					if (::IsWindowVisible(hwnd)) {
						::SetFocus(hwnd);
						return hwnd;
					}
				}
			}
		}
		return NULL;
	}

	int Find(HWND hwnd)
	{
		int cnt = (int)GetCount();
		for (int i = 0; i < cnt; i++) {
			if (hwnd == GetAt(i))
				return i;
		}
		return -1;
	}

	void SetActiveView(HWND hwndView, bool bSetFocus = true)
	{
		// 指定のウィンドウをアクティブとする
		int cnt = (int)GetCount();
		if (cnt > 0) {
			int pos = Find(hwndView);
			if (pos >= 0 && m_curIndex != pos) {
				m_curIndex = pos;
				if (bSetFocus)
					::SetFocus(hwndView);
			}
		}
	}

	BOOL ReplaceView(HWND hwndOld, HWND hwndNew)
	{
		if (!IsEmpty()) {
			int pos = Find(hwndOld);
			if (pos >= 0) {
				SetAt(pos, hwndNew);
				::ShowWindow(hwndOld, SW_HIDE);
				::ShowWindow(hwndNew, SW_SHOWNOACTIVATE);
				// 置換元のウィンドウがアクティブだったなら、置換したウィンドウもアクティブにする
				if (m_curIndex == pos)
					::SetFocus(hwndNew);
				return TRUE;
			}
		}
		return FALSE;
	}

	BOOL RemoveView(HWND hwnd)
	{
		int pos = Find(hwnd);
		if (pos < 0) return FALSE;

		if (pos == m_curIndex)
			m_curIndex = -1;
		else if (pos < m_curIndex)
			m_curIndex--;
		RemoveAt(pos);

		return TRUE;
	}
};

