cmake_minimum_required(VERSION 3.20)
project(orge VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# NOTE: orgeはmacOSにおいてarmのみ期待する。
# NOTE: これを設定しないとvcpkgが動かない。
if(NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
endif()

option(ORGE_STATIC "Build static library" OFF)
option(ORGE_SHARED "Build shared library" ON)
if(NOT ORGE_STATIC AND NOT ORGE_SHARED)
	message(FATAL_ERROR "Either ORGE_STATIC or ORGE_SHARED must be ON.")
endif()

include(GNUInstallDirs)
include(${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)
find_package(PkgConfig REQUIRED)
find_package(VulkanHeaders CONFIG REQUIRED)
find_package(VulkanLoader CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

if(MSVC)
	include(cmake/build-msvc.cmake)
elseif(APPLE OR LINUX)
	include(cmake/build.cmake)
else()
	message(FATAL_ERROR "unimplemented")
endif()

add_subdirectory(examples)
