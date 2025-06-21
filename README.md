# orge

## What's this?

オレオレゲームエンジン。

## Build

次をインストール:

- C/C++コンパイラ
- CMake
- Ninja (option)

次を実行:

```sh
mkdir build
cmake -G Ninja -S . -B build
cmake --build build
```

ただし、examples下の例をビルドする場合は、次のフラグを指定する:

- simple: `-DBUILD_EXAMPLE_SIMPLE=ON`
