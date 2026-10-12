REM -- resource.h からマップ ファイルを作成します
echo // htmlheader.cmd が生成したヘッダファイル >html_fop.h
echo. >>html_fop.h
echo // コマンド (ID_* and IDM_*) >>html_fop.h
C:\PROGRA~2\MICROS~2.0\Common7\Tools\makehm.exe /h ID_,IDH_,0x10000 IDM_,IDH_M_,0x10000 "..\resource.h" >>html_fop.h
echo. >>html_fop.h
echo // プロンプト (IDP_*) >>html_fop.h
C:\PROGRA~2\MICROS~2.0\Common7\Tools\makehm.exe /h IDP_,IDH_P_,0x30000 "..\resource.h" >>html_fop.h
echo. >>html_fop.h
echo // リソース (IDR_*) >>html_fop.h
C:\PROGRA~2\MICROS~2.0\Common7\Tools\makehm.exe /h IDR_,IDH_R_,0x20000 "..\resource.h" >>html_fop.h
echo. >>html_fop.h
echo // ダイアログ (IDD_*) >>html_fop.h
C:\PROGRA~2\MICROS~2.0\Common7\Tools\makehm.exe /h IDD_,IDH_D_,0x20000 "..\resource.h" >>html_fop.h
echo. >>html_fop.h
echo // フレーム コントロール (IDW_*) >>html_fop.h
C:\PROGRA~2\MICROS~2.0\Common7\Tools\makehm.exe /h IDW_,IDH_W_,0x50000 "..\resource.h" >>html_fop.h

echo // htmlheader.cmd が生成したヘッダファイル >html_ctrl.h
echo. >>html_ctrl.h
echo // フレーム コントロール (IDC_*) >>html_ctrl.h
C:\PROGRA~2\MICROS~2.0\Common7\Tools\makehm.exe /h IDC_,IDH_C_,0x50000 "..\resource.h" >>html_ctrl.h

pause
