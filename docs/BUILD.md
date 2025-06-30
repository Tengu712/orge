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

## Build

次を実行して依存ライブラリをビルド:

```sh
cd thirdparty

# Windows
setup.bat

# Linux/macOS
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

## Install

orgeを特定のディレクトリへインストールするには、次を行う必要がある:

- orgeのCMake構成時に`-DCMAKE_INCLUDE_PREFIX=`によってインストール先ディレクトリを指定する
- `cmake --install ... --prefix ...`のようにインストール時にインストール先ディレクトリを指定する

ただし、orgeではCMake構成時にpkgconfigの生成を行うため、インストール後にpkg-configを使う場合は必ずCMake構成時にインストール先ディレクトリを指定すること。

orgeをビルドした後、次を実行してインストール:

```sh
# CMake構成時にインストール先ディレクトリを指定した場合
cmake --install ビルドディレクトリ

# ここでインストール先ディレクトリを指定する場合
cmake --install ビルドディレクトリ --prefix インストール先ディレクトリ
```
