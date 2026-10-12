================================================================================
  7-zip32.dll ソースファイル + テストプログラム
================================================================================

★コンパイル方法
￣￣￣￣￣￣￣￣
  7-Zip の本家のサイト若しくは以下より 7-Zip のソースを入手します。
　http://akky.cjb.net/dev/null/7-zipsrc/
  それで 7za.exe をコンパイルできる状態にして下さい。
  コンパイルには新しい Platform SDK(Feb 2003 以降) が必要です。
  マイクロソフトのサイトよりダウンロードするなりしてインストールして下さい。
  それが出来たら /CPP/7zip/Bundles に 7-zip32 という名前のディレクトリを
  作成しそこに 7-zip32.dll のソースを展開してください。
  コンパイラは恐らく VC++6.0 以上でしかコンパイル出来ません。

★ソースについて
￣￣￣￣￣￣￣￣
  7-zip32.dll のソースは LGPL ですがテストプログラムの方は
  PDS としますのでご自由に使用してください。何の制限もありません。
  但し、何の参考にもならないと思いますがー。(^^;

★構成・仕様等について
￣￣￣￣￣￣￣￣￣￣￣
  7-zip32.dll は基本的に本家 7-Zip の 7za.exe のソースの
  main() を呼び出しているだけに過ぎません。
  理由は 7-Zip は現在進行形で日々進歩しているのでフォーマットを解析して
  独自に作成すると新しい形式にすぐに対応する事が出来ないためです。
  # フォーマットを解析する力が無いだけとも言います。
  その為、内部部分は可能な限り触っていません。
  どうしても変えなければならない所はその部分があるファイルを
  プロジェクトから除外、7-zip32.dll のディレクトリにコピーして修正後
  それをプロジェクトに追加してコンパイル・リンクさせています。
  こうする事により 7-Zip のバージョンが上がっても 7-zip32.dll の
  ソースファイル郡をコピーするだけでバージョンアップする事が可能です。
  但し、どうしても変えなければならなかったファイルのソースが
  バージョンアップにより修正されていた場合は当然対応したファイルを
  修正しなおす必要が有ります。
　内蔵 SFX は SFXSetup(7zS.sfx) を元に SFXWin(7z.sfx) の要素を加えた
　独自の仕様で作成してあります。
  ちなみに現在コピーして修正しているファイルは以下の物です。
  ソースには修正個所を記載しています。（StdOutStream.cpp 及び
  StdOutStream.h に付いては全面的に修正しているので記載はありません）

  修正ファイル               = 元ファイル
  /* 7-zip32 ファイル */
  Main.cpp                   = /CPP/7zip/UI/Console/Main.cpp
  MainAr.cpp                 = /CPP/7zip/UI/Console/MainAr.cpp
  UpdateCallbackConsole.cpp  = /CPP/7zip/UI/Console/UpdateCallbackConsole.cpp
  ExtractCallbackConsole.cpp = /CPP/7zip/UI/Console/ExtractCallbackConsole.cpp
  OpenCallbackConsole.cpp    = /CPP/7zip/UI/Console/OpenCallbackConsole.cpp
  ConsoleClose.cpp           = /CPP/7zip/UI/Console/ConsoleClose.cpp
  ArchiveExtractCallback.cpp = /CPP/7zip/UI/Common/ArchiveExtractCallback.cpp
  UpdateCallback.cpp         = /CPP/7zip/UI/Common/UpdateCallback.cpp
  Common/StdOutStream.cpp    = /CPP/Common/StdOutStream.cpp
  Common/StdOutStream.h      = /CPP/Common/StdOutStream.h
  /* SFX ファイル */
  SFX/ExtractCallback.cpp    = /CPP/7zip/Bundles/SFXSetup/ExtractCallback.cpp
  SFX/ExtractCallback.h      = /CPP/7zip/Bundles/SFXSetup/ExtractCallback.h
  SFX/ExtractEngine.cpp      = /CPP/7zip/Bundles/SFXSetup/ExtractEngine.cpp
  SFX/Main.cpp               = /CPP/7zip/Bundles/SFXSetup/Main.cpp
  SFX/ProgressDialog.cpp     = /CPP/7zip/UI/FileManager/ProgressDialog.cpp
  SFX/ProgressDialog.h       = /CPP/7zip/UI/FileManager/ProgressDialog.h


  逆に内部部分を触れられないので少し変更をしたい部分が変えられません。
  内部を書き換えれば簡単に修正できますがそれは出来ないので辛いです。
  かといってコピーして修正するとバージョンアップ時に
  チェックしなければならないファイルが増えてしまうので
  変えなくても問題のないものは変えない方針でいきます。

  もし、以上のファイルが変更なしか書換えを行ってもコンパイルする事が
  出来ない場合はプロジェクトファイルが更新されていると思います。
  その場合、前バージョンと新バージョンの Alone.dsp を比較し変更点を
  7-zip32.dsp にも変更を加えてください。
　SFX は SFXWin.dsp が元になります。

  それでもコンパイルできない時は修正内容から変更点を判断し根性で。
