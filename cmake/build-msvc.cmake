# MSVC用ビルドスクリプト

add_compile_options(/utf-8 /EHsc)

file(GLOB_RECURSE ORGE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

if(ORGE_SHARED)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

	add_library(orge SHARED ${ORGE_SOURCES})
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
	if(${ENABLE_VVL})
		target_compile_definitions(orge PRIVATE ENABLE_VVL)
	endif()
	add_library(orge::orge ALIAS orge)

	# NOTE: MSVCではインポートライブラリなしにdllをリンクできないので。
	#       全シンボルを自動エクスポートしてインポートライブラリを生成。
	set_target_properties(orge PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

	set(ORGE_PC_CFLAGS "/I\${includedir} /std:c++20 /EHsc /MD")
	set(ORGE_PC_LIBS "/LIBPATH:\${libdir} orge.lib")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/orge.pc.in ${CMAKE_CURRENT_BINARY_DIR}/orge.pc @ONLY)

	install(TARGETS orge)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/orge.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

if(ORGE_STATIC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	# 統合前
	add_library(orgestatic_lite STATIC ${ORGE_SOURCES})
	target_include_directories(orgestatic_lite
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
	)
	target_link_libraries(orgestatic_lite
		PRIVATE
			Vulkan::Headers
			Vulkan::Loader
			SDL3::SDL3-static
			yaml-cpp::yaml-cpp
	)
	if(${ENABLE_VVL})
		target_compile_definitions(orgestatic_lite PRIVATE ENABLE_VVL)
	endif()
	# NOTE: examplesのビルドにはorgestatic_liteを用いる。
	add_library(orge::orgestatic ALIAS orgestatic_lite)

	# 統合後
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/orgestatic.lib
		COMMAND ${CMAKE_COMMAND}
			-DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
			-DORGE_LIB=$<TARGET_FILE:orgestatic_lite>
			-DSDL3_LIB=$<TARGET_FILE:SDL3::SDL3-static>
			-DYAML_CPP_LIB=$<TARGET_FILE:yaml-cpp::yaml-cpp>
			-P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/merge-static-msvc.cmake
		DEPENDS orgestatic_lite SDL3::SDL3-static yaml-cpp::yaml-cpp
		COMMENT "Creating merged static library"
	)
	add_custom_target(orgestatic ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/orgestatic.lib)

	get_deps_from_pc(SDL3 sdl3 SDL3_DEPS)
	get_deps_from_pc(yaml-cpp yaml-cpp YAML_CPP_DEPS)
	set(ORGE_PC_CFLAGS "/I\${includedir} /std:c++20 /EHsc /MT")
	set(ORGE_PC_LIBS "/LIBPATH:\${libdir} orgestatic.lib vulkan-1.lib")
	set(ORGE_PC_LIBS_PRIVATE "${SDL3_DEPS} ${YAML_CPP_DEPS}")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/orgestatic.pc.in ${CMAKE_CURRENT_BINARY_DIR}/orgestatic.pc @ONLY)

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/orgestatic.lib DESTINATION ${CMAKE_INSTALL_LIBDIR})
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/orgestatic.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
	install(FILES $<TARGET_FILE:Vulkan::Loader> DESTINATION ${CMAKE_INSTALL_BINDIR})
	install(FILES $<TARGET_LINKER_FILE:Vulkan::Loader> DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
