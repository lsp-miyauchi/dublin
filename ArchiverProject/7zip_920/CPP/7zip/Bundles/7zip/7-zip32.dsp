# Microsoft Developer Studio Project File - Name="7-zip32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=7-zip32 - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "7-zip32.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "7-zip32.mak" CFG="7-zip32 - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "7-zip32 - Win32 Release" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE "7-zip32 - Win32 Debug" ("Win32 (x86) Dynamic-Link Library" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Gz /MT /W3 /GX /O1 /I ".\\" /I "..\..\..\\" /D "NDEBUG" /D "_MBCS" /D "WIN32" /D "_7ZIP_LARGE_PAGES" /D "SUPPORT_DEVICE_FILE" /Yu"StdAfx.h" /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /opt:NOWIN98
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /Gz /MTd /W3 /Gm /GX /ZI /Od /I ".\\" /I "..\..\..\\" /D "_DEBUG" /D "_MBCS" /D "WIN32" /D "_7ZIP_LARGE_PAGES" /D "SUPPORT_DEVICE_FILE" /Yu"StdAfx.h" /FD /GZ /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "7-zip32 - Win32 Release"
# Name "7-zip32 - Win32 Debug"
# Begin Group "Console"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\UI\Console\ConsoleClose.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Console\ExtractCallbackConsole.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Console\List.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Console\List.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Console\OpenCallbackConsole.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Console\PercentPrinter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Console\PercentPrinter.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Console\UpdateCallbackConsole.h
# End Source File
# End Group
# Begin Group "Spec"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\7-zip32.cpp"
# End Source File
# Begin Source File

SOURCE=".\7-zip32.def"
# End Source File
# Begin Source File

SOURCE=".\7-zip32.h"
# End Source File
# Begin Source File

SOURCE=.\ArchiveExtractCallback.cpp
# End Source File
# Begin Source File

SOURCE=.\ConsoleClose.cpp
# End Source File
# Begin Source File

SOURCE=.\Dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\Dialog.h
# End Source File
# Begin Source File

SOURCE=.\ExtractCallbackConsole.cpp
# End Source File
# Begin Source File

SOURCE=.\Main.cpp
# End Source File
# Begin Source File

SOURCE=.\MainAr.cpp
# End Source File
# Begin Source File

SOURCE=.\MainAr.h
# End Source File
# Begin Source File

SOURCE=.\MyOpenArchive.cpp
# End Source File
# Begin Source File

SOURCE=.\MyOpenArchive.h
# End Source File
# Begin Source File

SOURCE=.\OpenCallbackConsole.cpp
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\SplitCmdLine.cpp
# End Source File
# Begin Source File

SOURCE=.\SplitCmdLine.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"StdAfx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\Common\StdOutStream.cpp
# End Source File
# Begin Source File

SOURCE=.\Common\StdOutStream.h
# End Source File
# Begin Source File

SOURCE=.\UpdateCallback.cpp
# End Source File
# Begin Source File

SOURCE=.\UpdateCallbackConsole.cpp
# End Source File
# End Group
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\Common\AutoPtr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Buffer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\CommandLineParser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\CommandLineParser.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\ComTry.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\CRC.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Defs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\DynamicBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\IntToString.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\IntToString.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\ListFileUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\ListFileUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\MyCom.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\MyException.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\MyGuidDef.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\MyInitGuid.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\MyString.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\MyString.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\MyUnknown.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\MyVector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\MyVector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\NewHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\NewHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\StringConvert.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\StringConvert.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\StringToInt.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\StringToInt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Types.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\UTFConvert.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\UTFConvert.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Wildcard.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Common\Wildcard.h
# End Source File
# End Group
# Begin Group "Windows"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\Windows\Defs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\Device.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\DLL.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\DLL.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\Error.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\Error.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\FileDir.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\FileDir.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\FileFind.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\FileFind.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\FileIO.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\FileIO.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\FileName.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\FileName.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\Handle.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\MemoryLock.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\MemoryLock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\PropVariant.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\PropVariant.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\PropVariantConversions.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\PropVariantConversions.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\Synchronization.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\Synchronization.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\System.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\System.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\Thread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\Time.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\Windows\Time.h
# End Source File
# End Group
# Begin Group "7zip Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Common\CreateCoder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\CreateCoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\CrossThreadProgress.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\CrossThreadProgress.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\CWrappers.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\CWrappers.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\FilePathAutoRename.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\FilePathAutoRename.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\FileStreams.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\FileStreams.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\FilterCoder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\FilterCoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\InBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\InBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\InOutTempBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\InOutTempBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\LimitedStreams.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\LimitedStreams.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\LockedStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\LockedStream.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\MemBlocks.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\MethodId.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\MethodId.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\MethodProps.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\MethodProps.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\OffsetStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\OffsetStream.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\OutBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\OutBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\OutMemStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\OutMemStream.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\ProgressMt.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\ProgressMt.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\ProgressUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\ProgressUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\StreamBinder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\StreamBinder.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\StreamObjects.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\StreamObjects.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\StreamUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\StreamUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\Common\VirtThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Common\VirtThread.h
# End Source File
# End Group
# Begin Group "Compress"

# PROP Default_Filter ""
# Begin Group "BZip2"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Compress\BZip2Crc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\BZip2Crc.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\BZip2Decoder.cpp

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Compress\BZip2Decoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\BZip2Encoder.cpp

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Compress\BZip2Encoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\BZip2Register.cpp
# End Source File
# End Group
# Begin Group "Copy"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Compress\CopyCoder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\CopyCoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\CopyRegister.cpp
# End Source File
# End Group
# Begin Group "Deflate"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Compress\Deflate64Register.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\DeflateConst.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\DeflateDecoder.cpp

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Compress\DeflateDecoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\DeflateEncoder.cpp

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Compress\DeflateEncoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\DeflateExtConst.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\DeflateRegister.cpp
# End Source File
# End Group
# Begin Group "Huffman"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Compress\HuffmanDecoder.h
# End Source File
# End Group
# Begin Group "Implode"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Compress\ImplodeDecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\ImplodeDecoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\ImplodeHuffmanDecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\ImplodeHuffmanDecoder.h
# End Source File
# End Group
# Begin Group "LZMA"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Compress\Lzma2Decoder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\Lzma2Decoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\Lzma2Encoder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\Lzma2Encoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\Lzma2Register.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\LzmaDecoder.cpp

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Compress\LzmaDecoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\LzmaEncoder.cpp

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Compress\LzmaEncoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\LzmaRegister.cpp
# End Source File
# End Group
# Begin Group "PPMd"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Compress\PpmdContext.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\PpmdDecode.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\PpmdDecoder.cpp

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Compress\PpmdDecoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\PpmdEncode.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\PpmdEncoder.cpp

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Compress\PpmdEncoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\PpmdRegister.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\PpmdSubAlloc.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\PpmdType.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\PpmdZip.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\PpmdZip.h
# End Source File
# End Group
# Begin Group "RangeCoder"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Compress\RangeCoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\RangeCoderBit.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\RangeCoderBitTree.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\RangeCoderOpt.h
# End Source File
# End Group
# Begin Group "Shrink"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Compress\ShrinkDecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\ShrinkDecoder.h
# End Source File
# End Group
# Begin Group "Z"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Compress\ZDecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\ZDecoder.h
# End Source File
# End Group
# Begin Group "BWT"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Compress\Mtf8.h
# End Source File
# End Group
# Begin Group "LZX"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Compress\Lzx.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\Lzx86Converter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\Lzx86Converter.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\LzxDecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\LzxDecoder.h
# End Source File
# End Group
# Begin Group "Quantum"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Compress\QuantumDecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\QuantumDecoder.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\Compress\Bcj2Coder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\Bcj2Coder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\Bcj2Register.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\BcjCoder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\BcjCoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\BcjRegister.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\BitlDecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\BitlDecoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\BitlEncoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\BitmDecoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\BitmEncoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\BranchCoder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\BranchCoder.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\BranchMisc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\BranchMisc.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\BranchRegister.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\ByteSwap.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\ByteSwap.h
# End Source File
# Begin Source File

SOURCE=..\..\Compress\DeltaFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\LzOutWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Compress\LzOutWindow.h
# End Source File
# End Group
# Begin Group "Archive"

# PROP Default_Filter ""
# Begin Group "7z"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Archive\7z\7zCompressionMode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zCompressionMode.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zDecode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zDecode.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zEncode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zEncode.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zExtract.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zFolderInStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zFolderInStream.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zFolderOutStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zFolderOutStream.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zHandlerOut.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zHeader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zHeader.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zIn.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zIn.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zItem.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zOut.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zOut.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zProperties.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zProperties.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zRegister.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zSpecStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zSpecStream.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zUpdate.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\7z\7zUpdate.h
# End Source File
# End Group
# Begin Group "zip"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Archive\Zip\ZipAddCommon.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Zip\ZipAddCommon.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Zip\ZipCompressionMode.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Zip\ZipHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Zip\ZipHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Zip\ZipHandlerOut.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Zip\ZipHeader.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Zip\ZipHeader.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Zip\ZipIn.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Zip\ZipIn.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Zip\ZipItem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Zip\ZipItem.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Zip\ZipItemEx.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Zip\ZipOut.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Zip\ZipOut.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Zip\ZipRegister.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Zip\ZipUpdate.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Zip\ZipUpdate.h
# End Source File
# End Group
# Begin Group "Archive Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Archive\Common\CoderLoader.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\CoderMixer2.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\CoderMixer2.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\CoderMixer2MT.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\CoderMixer2MT.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\DummyOutStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\DummyOutStream.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\FindSignature.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\FindSignature.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\HandlerOut.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\HandlerOut.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\InStreamWithCRC.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\InStreamWithCRC.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\ItemNameUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\ItemNameUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\MultiStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\MultiStream.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\OutStreamWithCRC.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\OutStreamWithCRC.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\ParseProperties.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\Common\ParseProperties.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\Archive\Bz2Handler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\DeflateProps.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\DeflateProps.h
# End Source File
# Begin Source File

SOURCE=..\..\Archive\GzHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\LzmaHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\SplitHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\XzHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Archive\ZHandler.cpp
# End Source File
# End Group
# Begin Group "UI Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\UI\Common\ArchiveCommandLine.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\ArchiveCommandLine.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\ArchiveExtractCallback.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\ArchiveOpenCallback.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\ArchiveOpenCallback.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\DefaultName.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\DefaultName.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\EnumDirItems.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\EnumDirItems.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\Extract.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\Extract.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\ExtractingFilePath.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\ExtractingFilePath.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\LoadCodecs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\LoadCodecs.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\OpenArchive.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\OpenArchive.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\PropIDUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\PropIDUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\SetProperties.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\SetProperties.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\SortUtils.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\SortUtils.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\TempFiles.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\TempFiles.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\Update.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\Update.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\UpdateAction.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\UpdateAction.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\UpdateCallback.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\UpdatePair.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\UpdatePair.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\UpdateProduce.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\UpdateProduce.h
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\WorkDir.cpp
# End Source File
# Begin Source File

SOURCE=..\..\UI\Common\WorkDir.h
# End Source File
# End Group
# Begin Group "Crypto"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Crypto\7zAes.cpp

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Crypto\7zAes.h
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\7zAesRegister.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\HmacSha1.cpp

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Crypto\HmacSha1.h
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\MyAes.cpp

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Crypto\MyAes.h
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Pbkdf2HmacSha1.cpp

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Pbkdf2HmacSha1.h
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\RandGen.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\RandGen.h
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Sha1.cpp

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Crypto\Sha1.h
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\WzAes.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\WzAes.h
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\ZipCrypto.cpp

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O1

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\Crypto\ZipCrypto.h
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\ZipStrong.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Crypto\ZipStrong.h
# End Source File
# End Group
# Begin Group "7-zip"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\ICoder.h
# End Source File
# Begin Source File

SOURCE=..\..\IMyUnknown.h
# End Source File
# Begin Source File

SOURCE=..\..\IPassword.h
# End Source File
# Begin Source File

SOURCE=..\..\IProgress.h
# End Source File
# Begin Source File

SOURCE=..\..\IStream.h
# End Source File
# Begin Source File

SOURCE=..\..\PropID.h
# End Source File
# End Group
# Begin Group "C"

# PROP Default_Filter ""
# Begin Group "Xz"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\C\Xz.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Xz.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\XzCrc64.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\XzCrc64.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\XzDec.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\XzEnc.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\XzEnc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\XzIn.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\..\C\7zCrc.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\7zCrc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\7zCrcOpt.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\7zStream.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Aes.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Aes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\AesOpt.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Alloc.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Alloc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Bcj2.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Bcj2.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Bra.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Bra.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Bra86.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\BraIA64.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\BwtSort.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\BwtSort.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\CpuArch.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\CpuArch.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Delta.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Delta.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\HuffEnc.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\HuffEnc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\IStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\LzFind.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\LzFind.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\LzFindMt.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\LzFindMt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\LzHash.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Lzma2Dec.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Lzma2Dec.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Lzma2Enc.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Lzma2Enc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\LzmaDec.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\LzmaDec.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\LzmaEnc.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\LzmaEnc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\MtCoder.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\MtCoder.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Ppmd.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Ppmd7.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Ppmd7.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Ppmd7Dec.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Ppmd7Enc.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Ppmd8.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Ppmd8.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Ppmd8Dec.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Ppmd8Enc.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\RotateDefs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Sha256.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Sha256.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Sort.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Sort.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Threads.c

!IF  "$(CFG)" == "7-zip32 - Win32 Release"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "7-zip32 - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Threads.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\C\Types.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\SFX\ReleaseD\7zSDJC.sfx
# End Source File
# End Target
# End Project
