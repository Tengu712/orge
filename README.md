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
# Vulkan-Headersをビルド
cmake -S thirdparty/Vulkan-Headers -B thirdparty/Vulkan-Headers/build
cmake --install thirdparty/Vulkan-Headers/build --prefix build/deps/Vulkan-Headers

# orgeをビルド
cmake -G Ninja -S . -B build
cmake --build build
```

ただし、examples下の例をビルドする場合、orgeのCMake構成時に次のフラグを指定する:

- simple: `-DBUILD_EXAMPLE_SIMPLE=ON`
