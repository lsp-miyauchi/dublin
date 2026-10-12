!include x64.nsh
Name "libjpeg-turbo SDK for Visual C++ 64-bit"
OutFile "E:/github/cairns/gnu_libs/libjpeg/build\${BUILDDIR}libjpeg-turbo-2.1.0-vc64.exe"
InstallDir "c:\libjpeg-turbo64"

SetCompressor bzip2

Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "libjpeg-turbo SDK for Visual C++ 64-bit (required)"
!ifdef WIN64
	${If} ${RunningX64}
	${DisableX64FSRedirection}
	${Endif}
!endif
	SectionIn RO
!ifdef GCC
	IfFileExists $SYSDIR/libturbojpeg.dll exists 0
!else
	IfFileExists $SYSDIR/turbojpeg.dll exists 0
!endif
	goto notexists
	exists:
!ifdef GCC
	MessageBox MB_OK "An existing version of the libjpeg-turbo SDK for Visual C++ 64-bit is already installed.  Please uninstall it first."
!else
	MessageBox MB_OK "An existing version of the libjpeg-turbo SDK for Visual C++ 64-bit or the TurboJPEG SDK is already installed.  Please uninstall it first."
!endif
	quit

	notexists:
	SetOutPath $SYSDIR
!ifdef GCC
	File "E:/github/cairns/gnu_libs/libjpeg/build\libturbojpeg.dll"
!else
	File "E:/github/cairns/gnu_libs/libjpeg/build\${BUILDDIR}turbojpeg.dll"
!endif
	SetOutPath $INSTDIR\bin
!ifdef GCC
	File "E:/github/cairns/gnu_libs/libjpeg/build\libturbojpeg.dll"
!else
	File "E:/github/cairns/gnu_libs/libjpeg/build\${BUILDDIR}turbojpeg.dll"
!endif
!ifdef GCC
	File "E:/github/cairns/gnu_libs/libjpeg/build\libjpeg-62.dll"
!else
	File "E:/github/cairns/gnu_libs/libjpeg/build\${BUILDDIR}jpeg62.dll"
!endif
	File "E:/github/cairns/gnu_libs/libjpeg/build\${BUILDDIR}cjpeg.exe"
	File "E:/github/cairns/gnu_libs/libjpeg/build\${BUILDDIR}djpeg.exe"
	File "E:/github/cairns/gnu_libs/libjpeg/build\${BUILDDIR}jpegtran.exe"
	File "E:/github/cairns/gnu_libs/libjpeg/build\${BUILDDIR}tjbench.exe"
	File "E:/github/cairns/gnu_libs/libjpeg/build\${BUILDDIR}rdjpgcom.exe"
	File "E:/github/cairns/gnu_libs/libjpeg/build\${BUILDDIR}wrjpgcom.exe"
	SetOutPath $INSTDIR\lib
!ifdef GCC
	File "E:/github/cairns/gnu_libs/libjpeg/build\libturbojpeg.dll.a"
	File "E:/github/cairns/gnu_libs/libjpeg/build\libturbojpeg.a"
	File "E:/github/cairns/gnu_libs/libjpeg/build\libjpeg.dll.a"
	File "E:/github/cairns/gnu_libs/libjpeg/build\libjpeg.a"
!else
	File "E:/github/cairns/gnu_libs/libjpeg/build\${BUILDDIR}turbojpeg.lib"
	File "E:/github/cairns/gnu_libs/libjpeg/build\${BUILDDIR}turbojpeg-static.lib"
	File "E:/github/cairns/gnu_libs/libjpeg/build\${BUILDDIR}jpeg.lib"
	File "E:/github/cairns/gnu_libs/libjpeg/build\${BUILDDIR}jpeg-static.lib"
!endif
	SetOutPath $INSTDIR\lib\pkgconfig
	File "E:/github/cairns/gnu_libs/libjpeg/build\pkgscripts\libjpeg.pc"
	File "E:/github/cairns/gnu_libs/libjpeg/build\pkgscripts\libturbojpeg.pc"
	SetOutPath $INSTDIR\lib\cmake\libjpeg-turbo
	File "E:/github/cairns/gnu_libs/libjpeg/build\pkgscripts\libjpeg-turboConfig.cmake"
	File "E:/github/cairns/gnu_libs/libjpeg/build\pkgscripts\libjpeg-turboConfigVersion.cmake"
	File "E:/github/cairns/gnu_libs/libjpeg/build\win\libjpeg-turboTargets.cmake"
	File "E:/github/cairns/gnu_libs/libjpeg/build\win\libjpeg-turboTargets-release.cmake"
!ifdef JAVA
	SetOutPath $INSTDIR\classes
	File "E:/github/cairns/gnu_libs/libjpeg/build\java\turbojpeg.jar"
!endif
	SetOutPath $INSTDIR\include
	File "E:/github/cairns/gnu_libs/libjpeg/build\jconfig.h"
	File "E:/github/cairns/gnu_libs/libjpeg\jerror.h"
	File "E:/github/cairns/gnu_libs/libjpeg\jmorecfg.h"
	File "E:/github/cairns/gnu_libs/libjpeg\jpeglib.h"
	File "E:/github/cairns/gnu_libs/libjpeg\turbojpeg.h"
	SetOutPath $INSTDIR\doc
	File "E:/github/cairns/gnu_libs/libjpeg\README.ijg"
	File "E:/github/cairns/gnu_libs/libjpeg\README.md"
	File "E:/github/cairns/gnu_libs/libjpeg\LICENSE.md"
	File "E:/github/cairns/gnu_libs/libjpeg\example.txt"
	File "E:/github/cairns/gnu_libs/libjpeg\libjpeg.txt"
	File "E:/github/cairns/gnu_libs/libjpeg\structure.txt"
	File "E:/github/cairns/gnu_libs/libjpeg\usage.txt"
	File "E:/github/cairns/gnu_libs/libjpeg\wizard.txt"
	File "E:/github/cairns/gnu_libs/libjpeg\tjexample.c"
	File "E:/github/cairns/gnu_libs/libjpeg\java\TJExample.java"
!ifdef GCC
	SetOutPath $INSTDIR\man\man1
	File "E:/github/cairns/gnu_libs/libjpeg\cjpeg.1"
	File "E:/github/cairns/gnu_libs/libjpeg\djpeg.1"
	File "E:/github/cairns/gnu_libs/libjpeg\jpegtran.1"
	File "E:/github/cairns/gnu_libs/libjpeg\rdjpgcom.1"
	File "E:/github/cairns/gnu_libs/libjpeg\wrjpgcom.1"
!endif

	WriteRegStr HKLM "SOFTWARE\libjpeg-turbo64 2.1.0" "Install_Dir" "$INSTDIR"

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo64 2.1.0" "DisplayName" "libjpeg-turbo SDK v2.1.0 for Visual C++ 64-bit"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo64 2.1.0" "UninstallString" '"$INSTDIR\uninstall_2.1.0.exe"'
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo64 2.1.0" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo64 2.1.0" "NoRepair" 1
	WriteUninstaller "uninstall_2.1.0.exe"
SectionEnd

Section "Uninstall"
!ifdef WIN64
	${If} ${RunningX64}
	${DisableX64FSRedirection}
	${Endif}
!endif

	SetShellVarContext all

	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo64 2.1.0"
	DeleteRegKey HKLM "SOFTWARE\libjpeg-turbo64 2.1.0"

!ifdef GCC
	Delete $INSTDIR\bin\libjpeg-62.dll
	Delete $INSTDIR\bin\libturbojpeg.dll
	Delete $SYSDIR\libturbojpeg.dll
	Delete $INSTDIR\lib\libturbojpeg.dll.a
	Delete $INSTDIR\lib\libturbojpeg.a
	Delete $INSTDIR\lib\libjpeg.dll.a
	Delete $INSTDIR\lib\libjpeg.a
!else
	Delete $INSTDIR\bin\jpeg62.dll
	Delete $INSTDIR\bin\turbojpeg.dll
	Delete $SYSDIR\turbojpeg.dll
	Delete $INSTDIR\lib\jpeg.lib
	Delete $INSTDIR\lib\jpeg-static.lib
	Delete $INSTDIR\lib\turbojpeg.lib
	Delete $INSTDIR\lib\turbojpeg-static.lib
!endif
	Delete $INSTDIR\lib\pkgconfig\libjpeg.pc
	Delete $INSTDIR\lib\pkgconfig\libturbojpeg.pc
	Delete $INSTDIR\lib\cmake\libjpeg-turbo\libjpeg-turboConfig.cmake
	Delete $INSTDIR\lib\cmake\libjpeg-turbo\libjpeg-turboConfigVersion.cmake
	Delete $INSTDIR\lib\cmake\libjpeg-turbo\libjpeg-turboTargets.cmake
	Delete $INSTDIR\lib\cmake\libjpeg-turbo\libjpeg-turboTargets-release.cmake
!ifdef JAVA
	Delete $INSTDIR\classes\turbojpeg.jar
!endif
	Delete $INSTDIR\bin\cjpeg.exe
	Delete $INSTDIR\bin\djpeg.exe
	Delete $INSTDIR\bin\jpegtran.exe
	Delete $INSTDIR\bin\tjbench.exe
	Delete $INSTDIR\bin\rdjpgcom.exe
	Delete $INSTDIR\bin\wrjpgcom.exe
	Delete $INSTDIR\include\jconfig.h
	Delete $INSTDIR\include\jerror.h
	Delete $INSTDIR\include\jmorecfg.h
	Delete $INSTDIR\include\jpeglib.h
	Delete $INSTDIR\include\turbojpeg.h
	Delete $INSTDIR\uninstall_2.1.0.exe
	Delete $INSTDIR\doc\README.ijg
	Delete $INSTDIR\doc\README.md
	Delete $INSTDIR\doc\LICENSE.md
	Delete $INSTDIR\doc\example.txt
	Delete $INSTDIR\doc\libjpeg.txt
	Delete $INSTDIR\doc\structure.txt
	Delete $INSTDIR\doc\usage.txt
	Delete $INSTDIR\doc\wizard.txt
	Delete $INSTDIR\doc\tjexample.c
	Delete $INSTDIR\doc\TJExample.java
!ifdef GCC
	Delete $INSTDIR\man\man1\cjpeg.1
	Delete $INSTDIR\man\man1\djpeg.1
	Delete $INSTDIR\man\man1\jpegtran.1
	Delete $INSTDIR\man\man1\rdjpgcom.1
	Delete $INSTDIR\man\man1\wrjpgcom.1
!endif

	RMDir "$INSTDIR\include"
	RMDir "$INSTDIR\lib\pkgconfig"
	RMDir "$INSTDIR\lib\cmake\libjpeg-turbo"
	RMDir "$INSTDIR\lib\cmake"
	RMDir "$INSTDIR\lib"
	RMDir "$INSTDIR\doc"
!ifdef GCC
	RMDir "$INSTDIR\man\man1"
	RMDir "$INSTDIR\man"
!endif
!ifdef JAVA
	RMDir "$INSTDIR\classes"
!endif
	RMDir "$INSTDIR\bin"
	RMDir "$INSTDIR"

SectionEnd
