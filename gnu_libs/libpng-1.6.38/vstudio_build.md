# PNG build for Visual Studio 

既存のzlibを参照するため、pngソースを解凍して展開したフォルダ直下の CMakeLists.txt を編集する
テキストエディタで CMakeLists.txt を開き、以下の2行を追加する

```
set(ZLIB_INCLUDE_DIR ../zlib)
set(ZLIB_LIBRARY ../../zlib/winlib/Release/zlibstatic/zlib-static)
```

続いてcmakeにより環境を構築する
```
> mkdir build
> cd build
> cmake ..
```

buildフォルダ配下に作成された libpng.sln を vstudio で開き、pngプロジェクトを選択し,
ターゲットコンフィグを [ Release ] に切り替えてビルドする。

