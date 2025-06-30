# Linux/macOS用Vulkanローダのシンボリックリンクを作成するスクリプト

if(APPLE)
	set(VULKAN_LOADER_NAME "libvulkan.1.4.309.dylib")
	set(SYMLINK_NAME "libvulkan.1.dylib")
elseif(LINUX)
	set(VULKAN_LOADER_NAME "libvulkan.so.1.4.309")
	set(SYMLINK_NAME "libvulkan.so")
endif()

# インストール先を絶対パスで取得
get_filename_component(LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" ABSOLUTE)

# 作成するシンボリックリンクを絶対パスに
set(SYMLINK_PATH "${LIB_DIR}/${SYMLINK_NAME}")

# 作成
message(STATUS "Creating symlink: ${SYMLINK_PATH} -> ${VULKAN_LOADER_NAME}")
execute_process(
	COMMAND ${CMAKE_COMMAND} -E create_symlink ${VULKAN_LOADER_NAME} ${SYMLINK_PATH}
	WORKING_DIRECTORY "${LIB_DIR}"
	RESULT_VARIABLE SYMLINK_RESULT
)
if(NOT SYMLINK_RESULT EQUAL 0)
	message(FATAL_ERROR "Failed to create symlink ${SYMLINK_PATH}")
endif()
