set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)

if(PORT MATCHES "sdl3|yaml-cpp")
    set(VCPKG_LIBRARY_LINKAGE static)
endif()
