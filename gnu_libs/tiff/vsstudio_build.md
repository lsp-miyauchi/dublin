# libtiff をVisual Studioでビルド
<div style="text-align:right;">(2024/3/20) by K.Miyauchi</div>

## libtiffのソースビルド方法 1

1. tiff-4.4.0.zipをダウンロードして解凍
2. スタートメニューからVisual Studioを立ち上げる
3. コードなしで続行を選択
4. ファイル　→　開く　→　CMake
5. 解凍したtiff-4.4.0のトップにあるCMakeLists.txtを選択
6. ビルド　→　tiffのインストール  
これにより、outフォルダが作られ、その中のinstallフォルダにバイナリが生成される  
ただし、これは構成が [ x64-Debug ]
7. そこで、[ x64-Debug（規定値）]とあるプルダウンメニューから [ 構成を管理します ] を選択  
[ CMakeの設定 ]という画面の左にある[ ＋ ]を押して [ x64-Release ] を追加
[ Ctrl + s ]で保存し、先ほどのプルダウンメニューで [ x64-Release ] を選択
[ ビルド　→　tiffのインストール ]で、outフォルダ内のinstallフォルダにバイナリが生成される

#### libtiffを他のプロジェクトで使用する
1. dllファイルは tiff/out/install/x64-Release/bin/tiff.dll
2. libファイルは tiff/out/install/x64-Release/lib/tiff.lib
3. tiff/out/install/x64-Release/include 配下のヘッダファイルを他のプロジェクトにinclude
4. ビルド時は tiff.lib を取り込み、実行ファイルの場所にtiff.dll を置く


## libtiffのソースビルド方法 2

1. tiff-4.4.0.zipをダウンロードして解凍
2. 解凍した先のフォルダでコマンドプロンプトを開き、msvcコマンド実行環境にする
3. 次のコマンドを実行するとフォルダ直下にvstudioのslnファイル、vcxprojファイルが作成される
```msdos
> cmake .
```
4. 作成されたtiff.slnをvsstudioで開く
5. [ ALL_BUILD ]のプロジェクトが選択されている状態でvstudioが起動するので、コンフィグレーションのRelease を選択し、ALL_BUILDのビルドをおこなう
6. ビルドされた成果物一式を特定のフォルダに出力するには、プロジェクトの中の [ INSTALL ] を選択して、ビルドを実行する。ただし、デフォルトではインストール先が [ C:\Program Files(86)\tiff ] であるため、管理者権限でvstudioを実行していないとインストールに失敗する。  
なので、コマンドプロンプトを開いて、直接インストールコマンドを叩く。その際、CMAKE_INSTALL_PREFIXにインストール先のフォルダを指定する
```msdos
> cmake.exe -DBUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="c:\tiff" -P cmake_install.cmake
```
7. これによりtiff関連のモジュールが指定先のフォルダにコピーされる（上記の場合は C:\tiff がインストール先）

#### libtiffを他のプロジェクトで使用する
1. dllファイルは [ install path ]/bintiff.dll
2. libファイルは [ install path ]/lib/tiff.lib
3. [ install path ]/include 配下のヘッダファイルを他のプロジェクトにinclude
4. ビルド時は tiff.lib を取り込み、実行ファイルの場所にtiff.dll を置く


