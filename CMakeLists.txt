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
# TODO: 静的リンクライブラリも生成できるようにする
set(ORGE_LIBRARY_TYPE SHARED)

# 依存ライブラリ
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/deps/Vulkan-Headers)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/deps/Vulkan-Loader)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/deps/SDL)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/deps/yaml-cpp)
find_package(VulkanHeaders CONFIG REQUIRED)
find_package(VulkanLoader CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

# orge-lite
add_library(orge-lite ${ORGE_LIBRARY_TYPE}
	src/config.cpp
	src/error.cpp
	src/graphics/graphics.cpp
	src/graphics/rendering.cpp
	src/graphics/swapchain.cpp
	src/graphics/window.cpp
	src/orge.cpp
)
add_library(orge::orge-lite ALIAS orge-lite)
target_include_directories(orge-lite PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)
target_link_libraries(orge-lite PUBLIC
	Vulkan::Headers
	Vulkan::Loader
	SDL3::SDL3
	yaml-cpp::yaml-cpp
)

# orge
add_library(orge ${ORGE_LIBRARY_TYPE}
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
    PUBLIC
        $<$<NOT:$<BOOL:${ORGE_STATIC}>>:Vulkan::Loader>
        $<$<NOT:$<BOOL:${ORGE_STATIC}>>:SDL3::SDL3>
    PRIVATE
        Vulkan::Headers
        $<$<BOOL:${ORGE_STATIC}>:Vulkan::Loader>
        $<$<BOOL:${ORGE_STATIC}>:SDL3::SDL3>
        yaml-cpp::yaml-cpp
)

# Set rpath for dynamic libraries
set_target_properties(orge PROPERTIES
    INSTALL_RPATH "@loader_path"
    BUILD_WITH_INSTALL_RPATH TRUE
)
set_target_properties(orge-lite PROPERTIES
    INSTALL_RPATH "@loader_path"
    BUILD_WITH_INSTALL_RPATH TRUE
)

# Install
include(GNUInstallDirs)
install(
	TARGETS orge-lite orge
	EXPORT orgeTargets
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(
	DIRECTORY include/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(
	EXPORT orgeTargets
	FILE orgeTargets.cmake
	NAMESPACE orge::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/orge
)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	orgeConfigVersion.cmake
	VERSION 1.0.0
	COMPATIBILITY AnyNewerVersion
)
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/orgeConfig.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/orgeConfig.cmake
	@ONLY
)
install(
	FILES
		${CMAKE_CURRENT_BINARY_DIR}/orgeConfig.cmake
		${CMAKE_CURRENT_BINARY_DIR}/orgeConfigVersion.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/orge
)

# 動的リンクライブラリコピー
# NOTE: 動的リンクライブラリはorgeに統合できないので仕方なく。
if(WIN32)
	install(FILES $<TARGET_FILE:Vulkan::Loader> TYPE BIN)
else()
	install(FILES $<TARGET_FILE:Vulkan::Loader> TYPE LIB)
endif()

# examples
add_subdirectory(examples)
