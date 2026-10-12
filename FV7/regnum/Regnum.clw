; CLW ファイルは MFC ClassWizard の情報を含んでいます。

[General Info]
Version=1
LastClass=CRegnumDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "Regnum.h"

ClassCount=2
Class1=CRegnumApp
Class2=CRegnumDlg

ResourceCount=3
Resource2=IDR_MAINFRAME
Resource3=IDD_REGNUM_DIALOG

[CLS:CRegnumApp]
Type=0
HeaderFile=Regnum.h
ImplementationFile=Regnum.cpp
Filter=N

[CLS:CRegnumDlg]
Type=0
HeaderFile=RegnumDlg.h
ImplementationFile=RegnumDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=IDC_ISVALID



[DLG:IDD_REGNUM_DIALOG]
Type=1
Class=CRegnumDlg
ControlCount=12
Control1=IDC_STATIC,static,1342308352
Control2=IDC_PREFIX,edit,1350631552
Control3=IDC_STATIC,static,1342308352
Control4=IDC_DATE,SysDateTimePick32,1342242832
Control5=IDC_STATIC,static,1342308352
Control6=IDC_SEQ,edit,1350631552
Control7=IDC_STATIC,button,1342177287
Control8=IDOK,button,1342242817
Control9=IDCANCEL,button,1342242816
Control10=IDC_CLIP,button,1342242816
Control11=IDC_NUMBER,edit,1350631552
Control12=IDC_ISVALID,button,1342242816

