cmake_minimum_required(VERSION 3.20)
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
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/deps/yaml-cpp)
find_package(VulkanHeaders CONFIG REQUIRED)
find_package(VulkanLoader CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

# orge
add_library(orge STATIC
	src/config.cpp
	src/error.cpp
	src/graphics/graphics.cpp
	src/graphics/platform.cpp
	src/graphics/swapchain.cpp
	src/graphics/window.cpp
	src/orge.cpp
)
add_library(orge::orge ALIAS orge)
target_include_directories(orge PUBLIC include)
target_link_libraries(orge PUBLIC Vulkan::Loader SDL3::SDL3 PRIVATE Vulkan::Headers yaml-cpp::yaml-cpp)

# orgeを使用するターゲットを自動的にmacOSバンドルにする関数
function(orge_setup_target target_name)
	if(APPLE)
		add_custom_command(TARGET ${target_name} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory
				$<TARGET_FILE_DIR:${target_name}>/lib
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				${CMAKE_CURRENT_FUNCTION_LIST_DIR}/thirdparty/MoltenVK/Package/Release/MoltenVK/dynamic/dylib/macOS/libMoltenVK.dylib
				$<TARGET_FILE_DIR:${target_name}>/lib/libMoltenVK.dylib
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
				${CMAKE_CURRENT_FUNCTION_LIST_DIR}/thirdparty/MoltenVK/Package/Release/MoltenVK/dynamic/dylib/macOS/MoltenVK_icd.json
				$<TARGET_FILE_DIR:${target_name}>/lib/MoltenVK_icd.json
		)
	endif()
endfunction()

# examples
add_subdirectory(examples)
