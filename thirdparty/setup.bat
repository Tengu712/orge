@echo off

cmake ^
	-S Vulkan-Headers ^
	-B Vulkan-Headers/build
cmake ^
	--install Vulkan-Headers/build ^
	--prefix ../build/deps/Vulkan-Headers

cmake ^
	-S Vulkan-Loader ^
	-B Vulkan-Loader/build ^
	-G Ninja ^
	-D CMAKE_BUILD_TYPE=Release ^
	-D VULKAN_HEADERS_INSTALL_DIR=%~dp0/../build/deps/Vulkan-Headers
cmake ^
	--build Vulkan-Loader/build
cmake ^
	--install Vulkan-Loader/build ^
	--prefix ../build/deps/Vulkan-Loader

cmake ^
	-S SDL ^
	-B SDL/build ^
	-G Ninja ^
	-D CMAKE_BUILD_TYPE=Release ^
	-D SDL_SHARED=OFF ^
	-D SDL_STATIC=ON
cmake ^
	--build SDL/build
cmake ^
	--install SDL/build ^
	--prefix ../build/deps/SDL
