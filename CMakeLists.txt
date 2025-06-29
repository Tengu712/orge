cmake_minimum_required(VERSION 3.20)
project(orge VERSION 0.1.0)

# =========================================================================== #
#   General                                                                   #
# =========================================================================== #

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
find_package(PkgConfig REQUIRED)
find_package(VulkanHeaders CONFIG REQUIRED)
find_package(VulkanLoader CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

# インストールディレクトリ設定
include(GNUInstallDirs)

# =========================================================================== #
#   Build                                                                     #
# =========================================================================== #

# 共通オブジェクトライブラリ
add_library(orge_objects OBJECT
	src/config.cpp
	src/error.cpp
	src/graphics/graphics.cpp
	src/graphics/rendering.cpp
	src/graphics/swapchain.cpp
	src/graphics/window.cpp
	src/orge.cpp
)
target_include_directories(orge_objects
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(orge_objects
	PRIVATE
		Vulkan::Headers
		Vulkan::Loader
		SDL3::SDL3
		yaml-cpp::yaml-cpp
)
set_target_properties(orge_objects PROPERTIES POSITION_INDEPENDENT_CODE ON)

# 共有ライブラリ
if(ORGE_SHARED)
	add_library(orge SHARED $<TARGET_OBJECTS:orge_objects>)
	add_library(orge::orge ALIAS orge)

	target_include_directories(orge
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
	)
	target_link_libraries(orge
		PUBLIC Vulkan::Loader
		PRIVATE SDL3::SDL3 yaml-cpp::yaml-cpp
	)

	# NOTE: orgeがシステム、orgeのあるディレクトリ、orgeを使うアプリのあるディレクトリの順でVulkanローダを探すように。
    # NOTE: Windowsにはrpathの概念がないらしいので追記なし。
	if(APPLE)
		set_target_properties(orge PROPERTIES
			INSTALL_RPATH "/usr/local/lib;/opt/homebrew/lib;@loader_path;@executable_path"
			BUILD_WITH_INSTALL_RPATH TRUE
		)
	elseif(LINUX)
		set_target_properties(orge PROPERTIES
			INSTALL_RPATH "$ORIGIN"
			BUILD_WITH_INSTALL_RPATH TRUE
		)
	endif()
endif()

# 静的ライブラリ
if(ORGE_STATIC)
	# 依存ライブラリを統合したライブラリを作成
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/liborgestatic.a
		COMMAND ${CMAKE_COMMAND}
			-DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
			-DORGE_OBJECTS="$<TARGET_OBJECTS:orge_objects>"
			-DSDL3_LIB=$<TARGET_FILE:SDL3::SDL3>
			-DYAML_CPP_LIB=$<TARGET_FILE:yaml-cpp::yaml-cpp>
			-DCMAKE_AR=${CMAKE_AR}
			-P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/merge_static.cmake
		DEPENDS orge_objects SDL3::SDL3 yaml-cpp::yaml-cpp
		COMMENT "Creating merged static library"
	)

	# NOTE: 以降、完全にexamplesのビルドのためだけの処理。

	# カスタムターゲット作成
	add_custom_target(orgestatic_target ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/liborgestatic.a)

	# 依存関係を取得
	get_target_property(STATIC_DEPS SDL3::SDL3-static INTERFACE_LINK_LIBRARIES)
	list(APPEND STATIC_DEPS "Vulkan::Loader")

	# カスタムターゲットをorgestaticとしてインポート
	add_library(orgestatic STATIC IMPORTED GLOBAL)
	add_library(orge::orgestatic ALIAS orgestatic)
	set_target_properties(orgestatic PROPERTIES
		IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/liborgestatic.a
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
		INTERFACE_LINK_LIBRARIES "${STATIC_DEPS}"
	)
	add_dependencies(orgestatic orgestatic_target)
endif()

# =========================================================================== #
#   Package Config                                                            #
# =========================================================================== #

# 共通設定
if(MSVC)
	set(ORGE_PC_CFLAGS "/std:c++20")
else()
	set(ORGE_PC_CFLAGS "-std=c++20")
endif()

# 共有ライブラリ
if(ORGE_SHARED)
	if(MSVC)
		set(ORGE_PC_LIBS "orge.dll")
	else()
		set(ORGE_PC_LIBS "-lorge")
	endif()

	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/orge.pc.in ${CMAKE_CURRENT_BINARY_DIR}/orge.pc @ONLY)
endif()

# 静的ライブラリ
if(ORGE_STATIC)
	if(MSVC)
		set(ORGE_PC_LIBS "orgestatic.lib vulkan-1.dll")
	else()
		set(ORGE_PC_LIBS "-lorgestatic -lvulkan.1")
	endif()

	pkg_check_modules(SDL3_PC QUIET sdl3)
	if(SDL3_PC_FOUND)
		string(REPLACE ";" " " ORGE_PC_LIBS_PRIVATE "${SDL3_PC_STATIC_LDFLAGS_OTHER}")
	else()
		message(WARNING "SDL3 pkg-config not found. Static linking dependencies may be incomplete.")
		set(ORGE_PC_LIBS_PRIVATE "")
	endif()

	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/orgestatic.pc.in ${CMAKE_CURRENT_BINARY_DIR}/orgestatic.pc @ONLY)
endif()

# =========================================================================== #
#   Install                                                                   #
# =========================================================================== #

# インストール
if(ORGE_SHARED)
	install(TARGETS orge)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/orge.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()
if(ORGE_STATIC)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/liborgestatic.a DESTINATION ${CMAKE_INSTALL_LIBDIR})
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/orgestatic.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Vulkanローダをインストール先にコピー
# NOTE: macOSにはVulkanローダが標準搭載されていないので問答無用で。
#       Linuxではstaticビルド時にシンボル解決のために。
if(APPLE OR LINUX AND ORGE_STATIC)
	install(FILES $<TARGET_FILE:Vulkan::Loader> TYPE LIB)
	install(SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/cmake/create_vulkan_symlink.cmake)
endif()

# =========================================================================== #
#   Examples                                                                  #
# =========================================================================== #

add_subdirectory(examples)
