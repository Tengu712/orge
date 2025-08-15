# Build

## Windows

次をインストール:

- MSVC
- Windows SDK
- CMake
- Ninja
- Python3
- Meson (pip)

次を実行:

```bat
git clone --recursive https://github.com/Tengu712/orge.git

.\scripts\setup-vcpkg.bat (static|shared)

meson setup build ^
  -Dbuildtype=(release|debug) ^
  -Db_vscrt=(mt|mtd|md|mdd) ^
  --cmake-prefix-path=.\vcpkg_installed_(static|shared) ^
  --default-library=(static|shared) ^
  --prefix=インストール先パス

meson install -C build
```

## macOS

次をインストール:

- Clang系C++コンパイラ
- CMake
- Ninja
- Python3
- Meson (pip)
- install_name_tool

次を実行:

```sh
git clone --recursive https://github.com/Tengu712/orge.git

./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install --overlay-triplets=./triplets --triplet=custom-arm64-osx

meson setup build \
  -Dbuildtype=(release|debug) \
  --cmake-prefix-path=$(pwd)/vcpkg_installed/custom-arm64-osx \
  --default-library=(static|shared) \
  --prefix=インストール先パス

meson install -C build
```

## Linux

次をインストール:

- Clang系C++コンパイラ
- CMake
- Ninja
- Python3
- Meson (pip)
- x11
- xcb
- xkb
- wayland
- xrandr
- ltdl
- patchelf

次を実行:

```sh
git clone --recursive https://github.com/Tengu712/orge.git

./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install --overlay-triplets=./triplets --triplet=custom-x64-linux

meson setup build \
  -Dbuildtype=(release|debug) \
  --cmake-prefix-path=$(pwd)/vcpkg_installed/custom-x64-linux \
  --default-library=(static|shared) \
  --prefix=インストール先パス

meson install -C build
```

## Examples

examples下のサンプルプログラムをビルドするためにはmeson setup時に`-Dbuild_examples=true`あるいは`-Dbuild_example_<EXAMPLE_NAME>=true`を与える。

ただし、他言語での利用に関するサンプルプログラムに関しては、上記の方法ではビルドされないため、各サンプルのREADME.mdを参照すること。

## Enable Vulkan Validation Layer

Vulkan Validation Layerを有効化するためには、orgeのCMake構成時に`-Denale_vvl=true`を与える。
ただし、orgeはVulkan Validation Layerを提供しないため、自マシンにインストールしておくこと。
