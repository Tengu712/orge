# Build

## Requirements

- Common
  - C++コンパイラ
  - CMake
  - Ninja
  - pkg-config
- Windows
  - Windows SDK
- Linux
  - x11
  - xcb
  - xkb
  - wayland
  - xrandr

## Build & Install

本リポジトリをクローン:

```sh
git clone --recursive https://github.com/Tengu712/orge.git
```

依存ライブラリをインストール:

```sh
# Windows
.\vcpkg\bootstrap-vcpkg.bat
# Linux/macOS
./vcpkg/bootstrap-vcpkg.sh

# Windows
.\vcpkg\vcpkg install --triplet custom-x64-windows --overlay-triplets=.\triplets
# Linux
./vcpkg/vcpkg install --triplet custom-x64-linux --overlay-triplets=./triplets
# macOS
./vcpkg/vcpkg install --triplet custom-arm64-osx --overlay-triplets=./triplets
```

次を実行してorgeをビルド及びインストール:

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

## Examples

examples下のサンプルプログラムをビルドするには、orgeのCMake構成時にオプションを与える必要がある。
オプションは[examples/CMakeLists.txt](../examples/CMakeLists.txt)を参照。

ビルドされたサンプルプログラムはbuild/examplesディレクトリ下に生成される。
また、インストールされない。
