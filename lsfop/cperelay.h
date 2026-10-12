////////////////////////////////////////////////////////////
// cperelay.h

////////////////////////////////////////////////////////////
// CCpeRelay dialog


class CCpeRelay : public CDialogImpl<CCpeRelay>
{
public:
	CCpeRelay();
	~CCpeRelay() {}

	enum { IDD = IDD_CPERELAY };

BEGIN_MSG_MAP(CCpeRelay)
	ON_WM_INITDIALOG()
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MSG_MAP()

	BOOL OnInitDialog(LPARAM lParam);
	void OnCancel();

	BOOL m_bCanceled;
	long m_lTotal;
	long m_lDone;

	void Destroy();
	void SetCaption(LPCTSTR pszTitle);
	void SetTitle(LPCTSTR pszTitle);
	void SetFile(LPCTSTR pszFile);
	void SetMessage(LPCTSTR pszMessage);
	void SetRange(long lTotal);
	void SetComplete(long lDone);
	BOOL isCanceled();
	void ZapMessage();
};

