cmake_minimum_required(VERSION 3.20)
project(orge VERSION 0.1.0)

# C++設定
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ライブラリタイプ設定
option(ORGE_SHARED "Build shared library" ON)
option(ORGE_STATIC "Build static library" OFF)
if(NOT ORGE_SHARED AND NOT ORGE_STATIC)
	message(FATAL_ERROR "Either ORGE_SHARED or ORGE_STATIC must be ON.")
endif()
# NOTE: Windowsでは/MD vs /MT問題のためにトリプレットを分ける必要があるので、同時に.libと.dllをビルドできない。
if(MSVC AND ORGE_SHARED AND ORGE_STATIC)
	message(FATAL_ERROR "On Windows, ORGE_SHARED and ORGE_STATIC cannot both be ON due to runtime library conflicts.")
endif()

# vcpkgのインストールディレクトリ設定
#
# NOTE: これがないとなぜか再ビルドが走る。
# NOTE: Windows では/MT vs /MD 問題でトリプレットを分けているが、
#       異なるトリプレットを使うと以前のビルドキャッシュが削除されてしまう違うため、インストールディレクトリを分離。
if(MSVC)
	if(ORGE_SHARED)
		set(VCPKG_INSTALLED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed_shared)
	else()
		set(VCPKG_INSTALLED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed_static)
	endif()
else()
	set(VCPKG_INSTALLED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg_installed)
endif()

# vcpkgトリプレット設定
#
# NOTE: トリプレット指定忘れが悲惨なのでここで強制。
set(VCPKG_OVERLAY_TRIPLETS ${CMAKE_CURRENT_SOURCE_DIR}/triplets)
if(MSVC)
	if(ORGE_SHARED)
		set(VCPKG_TARGET_TRIPLET "custom-x64-windows-shared")
	else()
		set(VCPKG_TARGET_TRIPLET "custom-x64-windows-static")
	endif()
elseif(APPLE)
	set(VCPKG_TARGET_TRIPLET "custom-arm64-osx")
elseif(UNIX)
	set(VCPKG_TARGET_TRIPLET "custom-x64-linux")
else()
	message(FATAL_ERROR "unimplemented")
endif()

# vcpkgのためのmacOSのアーキテクチャ設定
#
# NOTE: orgeはmacOSにおいてarmのみ期待する。
# NOTE: これを設定しないとvcpkgが動かない。
if(NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
endif()

# vcpkg
include(${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

# CMAKE_PREFIX_PATHを修正
#
# NOTE: 主にMSVCで勝手に異なるトリプレットの方から依存ライブラリを見つけてこないようにするため。
set(CMAKE_PREFIX_PATH ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET})

# 依存ライブラリ設定
include(GNUInstallDirs)
find_package(VulkanHeaders CONFIG REQUIRED)
find_package(VulkanLoader CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

# ユーティリティ関数追加
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/get-deps-from-pc.cmake)

# 実行
if(MSVC)
	include(cmake/build-msvc.cmake)
elseif(APPLE OR LINUX)
	include(cmake/build.cmake)
endif()

# examples
add_subdirectory(examples)
