# How to Use

## Windows

[BUILD.md](./BUILD.md)を参照してビルドおよびインストールすること。
これにより、次が生成される:

- orge.dll (ORGE_SHARED=ON)
- orge.lib (ORGE_SHARED=ON AND MSVC)
- liborge.a (ORGE_SHARED=ON AND NOT MSVC)
- orgestatic.lib (ORGE_STATIC=ON AND MSVC)
- liborgestatic.a (ORGE_STATIC=ON AND NOT MSVC)
- orge.h
- pkgconfig

## macOS

[BUILD.md](./BUILD.md)を参照してビルドおよびインストールすること。
これにより、次が生成される:

- liborge.dylib (ORGE_SHARED=ON)
- liborgestatic.a (ORGE_STATIC=ON)
- libvulkan.1.dylib
- libvulkan.1.4.313.dylib
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
- libvulkan.so.1.4.313 (ORGE_STATIC=ON)
- orge.h
- pkgconfig

liborgestatic.aを利用する場合、libvulkan.soもリンクすること。
