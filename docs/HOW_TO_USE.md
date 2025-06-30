# How to Use

## Windows

[BUILD.md](./BUILD.md)を参照してビルドおよびインストールすること。
これにより、次が生成される:

- orge.dll (ORGE_SHARED=ON)
- orge.lib (ORGE_SHARED=ON)
- orgestatic.lib (ORGE_STATIC=ON)
- vulkan-1.dll (ORGE_STATIC=ON)
- vulkan-1.lib (ORGE_STATIC=ON)
- orge.h
- pkgconfig

orge.dll(orge.lib)を使う場合は/MDを指定すること。

orgestatic.libを使う場合は次に注意すること:

- /MTを指定すること
- SDL3が依存するライブラリもリンクすること
- vulkan-1.libもリンクすること

## macOS

[BUILD.md](./BUILD.md)を参照してビルドおよびインストールすること。
これにより、次が生成される:

- liborge.dylib (ORGE_SHARED=ON)
- liborgestatic.a (ORGE_STATIC=ON)
- libvulkan.1.dylib
- libvulkan.1.4.309.dylib
- orge.h
- pkgconfig

macOSにはVulkanランタイムが標準搭載されていない。
そのため、MoltenVKとVulkanローダが必要となる。
その内、orgeはVulkanローダ(libvulkan1.dylib)を提供する。
MoltenVKに関してはorgeの利用者の責務とする。

liborge.dylibを利用する場合、アプリがliborge.dylibを発見できるようにすること。
また、liborge.dylibがlibvulkan.1.dylibを発見できるようにすること。
ただし、liborge.dylibは次の優先順位でlibvulkan.1.dylibを探す:

1. /usr/local/lib
1. /opt/homebrew/lib
1. liborge.dylibと同ディレクトリ
1. アプリと同ディレクトリ

liborgestatic.aを利用する場合、次に注意すること:

- libvulkan.1.dylibもリンクすること
- SDL3が依存するフレームワークもリンクすること
- アプリがlibvulkan.1.dylibを発見できるようにすること

## Linux

[BUILD.md](./BUILD.md)を参照してビルドおよびインストールすること。
これにより、次が生成される:

- liborge.so (ORGE_SHARED=ON)
- liborgestatic.a (ORGE_STATIC=ON)
- libvulkan.so (ORGE_STATIC=ON)
- libvulkan.so.1.4.309 (ORGE_STATIC=ON)
- orge.h
- pkgconfig

liborgestatic.aを利用する場合、libvulkan.soもリンクすること。

## pkg-configを使うと良いよという話

次のようにして、必要なフラグを自動で指定できる。
ただし、インストール先ディレクトリのlib/pkgconfigディレクトリに`PKG_CONFIG_PATH`を通す。

```sh
# 共有ライブラリ利用
g++ ../examples/simple/main.cpp $(pkg-config orge --cflags --libs)

# 静的ライブラリ利用
g++ ../examples/simple/main.cpp $(pkg-config orgestatic --cflags --libs --static)
```
