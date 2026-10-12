// ExtractDialog.h: CExtractDialog クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXTRACTDIALOG_H__0A42BD0A_3259_463F_A975_558851ADEC49__INCLUDED_)
#define AFX_EXTRACTDIALOG_H__0A42BD0A_3259_463F_A975_558851ADEC49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

#include "Windows/Control/Dialog.h"

class CExtractDialog : public NWindows::NControl::CModalDialog  
{
public:
	UString GetFolderName() { return m_folderName; }
	CExtractDialog();
	virtual ~CExtractDialog();
	
	INT_PTR Create(const UString &friendlyName, const UString &installPrompt, const UString &folderName, HWND aWndParent = 0)
	{ 
		m_friendlyName = friendlyName;
		m_installPrompt = installPrompt;
		m_folderName = folderName;
		return CModalDialog::Create(IDD_DIALOG_EXTRACT, aWndParent); 
	}

private:
	virtual bool OnInit();
	virtual void OnOK();
	bool OnButtonClicked(int buttonID, HWND buttonHWND);

	UString m_friendlyName;
	UString m_installPrompt;
	UString m_folderName;
};

#endif // !defined(AFX_EXTRACTDIALOG_H__0A42BD0A_3259_463F_A975_558851ADEC49__INCLUDED_)
