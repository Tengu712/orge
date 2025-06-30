cmake_minimum_required(VERSION 3.20)
project(orge VERSION 0.1.0)

# C++設定
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vcpkgのためのmacOSのアーキテクチャ設定
#
# NOTE: orgeはmacOSにおいてarmのみ期待する。
# NOTE: これを設定しないとvcpkgが動かない。
if(NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
endif()

# vcpkgのインストールディレクトリ設定
#
# NOTE: これがないとなぜか再ビルドが走る。
set(VCPKG_INSTALLED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed)

# vcpkgトリプレット設定
#
# NOTE: トリプレット指定忘れが悲惨なのでここで強制。
set(VCPKG_OVERLAY_TRIPLETS ${CMAKE_CURRENT_SOURCE_DIR}/triplets)
if(MSVC)
	set(VCPKG_TARGET_TRIPLET "custom-x64-windows")
elseif(APPLE)
	set(VCPKG_TARGET_TRIPLET "custom-arm64-osx")
elseif(UNIX)
	set(VCPKG_TARGET_TRIPLET "custom-x64-linux")
else()
	# NOTE: WindowsではMSVCのみサポート。
	message(FATAL_ERROR "unimplemented")
endif()

# ライブラリタイプ設定
option(ORGE_STATIC "Build static library" OFF)
option(ORGE_SHARED "Build shared library" ON)
if(NOT ORGE_STATIC AND NOT ORGE_SHARED)
	message(FATAL_ERROR "Either ORGE_STATIC or ORGE_SHARED must be ON.")
endif()

# 依存ライブラリ設定
include(GNUInstallDirs)
include(${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)
find_package(PkgConfig REQUIRED)
find_package(VulkanHeaders CONFIG REQUIRED)
find_package(VulkanLoader CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

# 実行
if(MSVC)
	include(cmake/build-msvc.cmake)
elseif(APPLE OR LINUX)
	include(cmake/build.cmake)
else()
	# NOTE: WindowsではMSVCのみサポート。
	message(FATAL_ERROR "unimplemented")
endif()

# examples
add_subdirectory(examples)
