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
  -Dbuild_examples=(true|false) ^
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
./vcpkg/vcpkg install --overlay-triplets=./triplets --triplet=custom-x64-linux

meson setup build \
  -Dbuild_examples=(true|false) \
  -Dbuildtype=(release|debug) \
  --cmake-prefix-path=$(pwd)/vcpkg_installed/custom-x64-linux \
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
  -Dbuild_examples=(true|false) \
  -Dbuildtype=(release|debug) \
  --cmake-prefix-path=$(pwd)/vcpkg_installed/custom-x64-linux \
  --default-library=(static|shared) \
  --prefix=インストール先パス

meson install -C build
```

## Enable Vulkan Validation Layer

Vulkan Validation Layerを有効化するためには、orgeのCMake構成時に`-D ENABLE_VVL=ON`を与える。
ただし、orgeはVulkan Validation Layerを提供しないため、自マシンにインストールしておくこと。
