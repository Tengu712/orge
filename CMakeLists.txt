cmake_minimum_required(VERSION 3.20)
project(orge)

# C++設定
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# コンパイラ設定
if(MSVC)
	add_compile_options(/utf-8)
endif()

# ライブラリタイプ設定
option(ORGE_STATIC "Build static library" OFF)
option(ORGE_SHARED "Build shared library" ON)
if(NOT ORGE_STATIC AND NOT ORGE_SHARED)
	message(FATAL_ERROR "Either ORGE_STATIC or ORGE_SHARED must be ON.")
endif()

# 依存ライブラリ
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/deps/Vulkan-Headers)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/deps/Vulkan-Loader)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/deps/SDL)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/deps/yaml-cpp)
find_package(VulkanHeaders CONFIG REQUIRED)
find_package(VulkanLoader CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

# 動的リンクライブラリ
if(ORGE_SHARED)
	add_library(orge SHARED
		src/config.cpp
		src/error.cpp
		src/graphics/graphics.cpp
		src/graphics/rendering.cpp
		src/graphics/swapchain.cpp
		src/graphics/window.cpp
		src/orge.cpp
	)
	add_library(orge::orge ALIAS orge)
	target_include_directories(orge PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
	)
	target_link_libraries(orge
		PUBLIC Vulkan::Loader
		PRIVATE Vulkan::Headers SDL3::SDL3 yaml-cpp::yaml-cpp
	)
	if(APPLE)
		set_target_properties(orge PROPERTIES
			INSTALL_RPATH "/usr/local/lib;/opt/homebrew/lib;@loader_path;@executable_path"
			BUILD_WITH_INSTALL_RPATH TRUE
		)
	else()
		set_target_properties(orge PROPERTIES
			INSTALL_RPATH "$ORIGIN"
			BUILD_WITH_INSTALL_RPATH TRUE
		)
	endif()
endif()

# 静的リンクライブラリ
if(ORGE_STATIC)
	add_library(orgestatic STATIC
		src/config.cpp
		src/error.cpp
		src/graphics/graphics.cpp
		src/graphics/rendering.cpp
		src/graphics/swapchain.cpp
		src/graphics/window.cpp
		src/orge.cpp
	)
	add_library(orge::orgestatic ALIAS orgestatic)
	target_include_directories(orgestatic PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
	)
	target_link_libraries(orgestatic
		PRIVATE Vulkan::Headers Vulkan::Loader SDL3::SDL3 yaml-cpp::yaml-cpp
	)
endif()

# インストール
include(GNUInstallDirs)
if(ORGE_SHARED)
	install(TARGETS orge)
endif()
if(ORGE_STATIC)
	install(TARGETS orgestatic)
endif()
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# 動的リンクライブラリをインストール先にコピー
# NOTE: macOSにはVulkanローダがインストールされていない上、
#       動的リンクライブラリはorgeに統合できないので仕方なく頒布時に同梱する必要がある。
if(APPLE)
	install(FILES $<TARGET_FILE:Vulkan::Loader> TYPE LIB)
	install(CODE "
		set(VULKAN_LOADER_NAME \"libvulkan.1.4.313.dylib\")
		get_filename_component(LIB_DIR \"\${CMAKE_INSTALL_PREFIX}/lib\" ABSOLUTE)
		set(SYMLINK_PATH \"\${LIB_DIR}/libvulkan.1.dylib\")
		message(STATUS \"Creating symlink: \${SYMLINK_PATH} -> \${VULKAN_LOADER_NAME}\")
		execute_process(
			COMMAND \${CMAKE_COMMAND} -E create_symlink \${VULKAN_LOADER_NAME} \${SYMLINK_PATH}
			WORKING_DIRECTORY \"\${LIB_DIR}\"
			RESULT_VARIABLE SYMLINK_RESULT
		)
		if(NOT SYMLINK_RESULT EQUAL 0)
			message(WARNING \"Failed to create symlink\")
		endif()
	")
endif()

# examples
add_subdirectory(examples)
