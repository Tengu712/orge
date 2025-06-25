# Build

## Requirements

- Common
  - C++コンパイラ
  - CMake
  - Ninja
- Windows
  - Windows SDK
- Linux
  - x11
  - xcb
  - xkb
  - wayland
  - xrandr
  - pkg-config

## Build

次を実行して依存ライブラリをビルド:

```sh
cd thirdparty

# Windows
setup.bat

# Linux
./setup.sh
```

次を実行してorgeをビルド:

```sh
cmake -G Ninja -S . -B build
cmake --build build
```

## Examples

examples下のサンプルプログラムをビルドするには、orgeのCMake構成時にオプションを与える必要がある。
オプションは[examples/CMakeLists.txt](../examples/CMakeLists.txt)を参照。
