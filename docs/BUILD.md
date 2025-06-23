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
