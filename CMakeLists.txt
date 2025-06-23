cmake_minimum_required(VERSION 3.5)
project(orge)

# C++設定
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# コンパイラ設定
if(MSVC)
    add_compile_options(/utf-8)
endif()

# 依存ライブラリ
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/deps/Vulkan-Headers)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/deps/Vulkan-Loader)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/deps/SDL)
find_package(VulkanHeaders CONFIG REQUIRED)
find_package(VulkanLoader CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)

# orge
add_library(orge STATIC
	src/error.cpp
	src/graphics/graphics.cpp
	src/graphics/swapchain.cpp
	src/graphics/window.cpp
	src/orge.cpp
)
add_library(orge::orge ALIAS orge)
target_include_directories(orge PUBLIC include)
target_link_libraries(orge PUBLIC Vulkan::Loader SDL3::SDL3 PRIVATE Vulkan::Headers)

# examples
add_subdirectory(examples)
