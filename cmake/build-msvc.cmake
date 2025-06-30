# MSVC用ビルドスクリプト

add_compile_options(/utf-8)

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
	target_compile_options(orge PRIVATE /MD)
	target_include_directories(orge
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
	)
	target_link_libraries(orge
		PRIVATE
			Vulkan::Headers
			Vulkan::Loader
			SDL3::SDL3
			yaml-cpp::yaml-cpp
	)
	add_library(orge::orge ALIAS orge)

	# NOTE: MSVCではインポートライブラリなしにdllをリンクできないので。
	#       全シンボルを自動エクスポートしてインポートライブラリを生成。
	set_target_properties(orge PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

	set(ORGE_PC_CFLAGS "/I\${includedir} /std:c++20")
	set(ORGE_PC_LIBS "/LIBPATH:\${libdir} orge.lib")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/orge.pc.in ${CMAKE_CURRENT_BINARY_DIR}/orge.pc @ONLY)

	install(TARGETS orge)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/orge.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

if(ORGE_STATIC)
	# 統合前
	add_library(orgestatic_lite STATIC
		src/config.cpp
		src/error.cpp
		src/graphics/graphics.cpp
		src/graphics/rendering.cpp
		src/graphics/swapchain.cpp
		src/graphics/window.cpp
		src/orge.cpp
	)
	target_compile_options(orgestatic_lite PRIVATE /MT)
	target_include_directories(orgestatic_lite
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
	)
	target_link_libraries(orgestatic_lite
		PRIVATE
			Vulkan::Headers
			Vulkan::Loader
			SDL3::SDL3
			yaml-cpp::yaml-cpp
	)
	# NOTE: examplesのビルドにはorgestatic_liteを用いる。
	add_library(orge::orgestatic ALIAS orgestatic_lite)

	# 統合後
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/orgestatic.lib
		COMMAND ${CMAKE_COMMAND}
			-DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
			-DORGE_LIB=$<TARGET_FILE:orgestatic_lite>
			-DSDL3_LIB=$<TARGET_FILE:SDL3::SDL3>
			-DYAML_CPP_LIB=$<TARGET_FILE:yaml-cpp::yaml-cpp>
			-P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/merge-static-msvc.cmake
		DEPENDS orgestatic_lite SDL3::SDL3 yaml-cpp::yaml-cpp
		COMMENT "Creating merged static library"
	)

	pkg_check_modules(SDL3_PC QUIET sdl3)
	if(SDL3_PC_FOUND)
		string(REPLACE ";" " " ORGE_PC_LIBS_PRIVATE "${SDL3_PC_STATIC_LDFLAGS_OTHER}")
	else()
		message(WARNING "SDL3 pkg-config not found. Static linking dependencies may be incomplete.")
		set(ORGE_PC_LIBS_PRIVATE "")
	endif()
	set(ORGE_PC_CFLAGS "/I\${includedir} /std:c++20")
	set(ORGE_PC_LIBS "/LIBPATH:\${libdir} orgestatic.lib")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/orgestatic.pc.in ${CMAKE_CURRENT_BINARY_DIR}/orgestatic.pc @ONLY)

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/orgestatic.lib DESTINATION ${CMAKE_INSTALL_LIBDIR})
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/orgestatic.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
