# Build

## Requirements

- Common
  - CMake
  - Ninja
  - pkg-config
- Windows
  - MSVC
  - Windows SDK
- Linux
  - C++コンパイラ
  - ar
  - x11
  - xcb
  - xkb
  - wayland
  - xrandr
  - ltdl
- macOS
  - C++コンパイラ
  - ar

## Build & Install

本リポジトリをクローン:

```sh
git clone --recursive https://github.com/Tengu712/orge.git
```

vcpkgをセットアップ:

```sh
# Windows
.\vcpkg\bootstrap-vcpkg.bat
# Linux/macOS
./vcpkg/bootstrap-vcpkg.sh
```

次を実行してビルド及びインストール:

```sh
cmake \
	-G Ninja \
	-B build \
	-D ORGE_SHARED=(ON|OFF) \
	-D ORGE_STATIC=(ON|OFF) \
	-D CMAKE_BUILD_TYPE=(Release|Debug) \
	-D CMAKE_INSTALL_PREFIX=インストール先ディレクトリパス
cmake --build build
cmake --install build
```

ただし、

- どのOSでも`ORGE_SHARED`と`ORGE_STATIC`の両方を`OFF`にできない
- Windowsでは`ORGE_SHARED`と`ORGE_STATIC`の両方を`ON`にできない

## Examples

examples下のサンプルプログラムをビルドするには、orgeのCMake構成時にオプションを与える必要がある。
オプションは[examples/CMakeLists.txt](../examples/CMakeLists.txt)を参照。

ビルドされたサンプルプログラムはbuild/examplesディレクトリ下に生成される。
また、インストールされない。
