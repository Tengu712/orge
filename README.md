# orge

## What's this?

オレオレゲームエンジン。

## Build

次をインストール:

- C/C++コンパイラ
- CMake
- Ninja (option)

次を実行して依存ライブラリをビルド:

```sh
cd thirdparty
setup.bat
```

次を実行してorgeをビルド:

```sh
cmake -G Ninja -S . -B build
cmake --build build
```

ただし、examples下の例をビルドする場合、orgeのCMake構成時に次のフラグを指定する:

- simple: `-D BUILD_EXAMPLE_SIMPLE=ON`
