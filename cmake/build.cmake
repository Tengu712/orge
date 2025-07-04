# Linux/macOS用ビルドスクリプト

file(GLOB_RECURSE ORGE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_library(orge_objects OBJECT ${ORGE_SOURCES})
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
if(${ENABLE_VVL})
	target_compile_definitions(orge_objects PRIVATE ENABLE_VVL)
endif()
set_target_properties(orge_objects PROPERTIES POSITION_INDEPENDENT_CODE ON)

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

	# NOTE: orgeがVulkanローダを探せるように。
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

	set(ORGE_PC_CFLAGS "-I\${includedir} -std=c++20")
	set(ORGE_PC_LIBS "-L\${libdir} -lorge")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/orge.pc.in ${CMAKE_CURRENT_BINARY_DIR}/orge.pc @ONLY)

	install(TARGETS orge)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/orge.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

if(ORGE_STATIC)
	# 統合ライブラリ
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/liborgestatic.a
		COMMAND ${CMAKE_COMMAND}
			-DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
			-DORGE_OBJECTS="$<TARGET_OBJECTS:orge_objects>"
			-DSDL3_LIB=$<TARGET_FILE:SDL3::SDL3-static>
			-DYAML_CPP_LIB=$<TARGET_FILE:yaml-cpp::yaml-cpp>
			-P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/merge-static.cmake
		DEPENDS orge_objects SDL3::SDL3-static yaml-cpp::yaml-cpp
		COMMENT "Creating merged static library"
	)

	# NOTE: examplesのビルドのためだけにターゲットを作成。
	add_custom_target(orgestatic_target ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/liborgestatic.a)
	get_target_property(STATIC_DEPS SDL3::SDL3-static INTERFACE_LINK_LIBRARIES)
	list(APPEND STATIC_DEPS "Vulkan::Loader")
	add_library(orgestatic STATIC IMPORTED GLOBAL)
	add_library(orge::orgestatic ALIAS orgestatic)
	set_target_properties(orgestatic PROPERTIES
		IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/liborgestatic.a"
		INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
		INTERFACE_LINK_LIBRARIES "${STATIC_DEPS}"
	)
	add_dependencies(orgestatic orgestatic_target)

	get_deps_from_pc(SDL3 sdl3 SDL3_DEPS)
	get_deps_from_pc(yaml-cpp yaml-cpp YAML_CPP_DEPS)
	set(ORGE_PC_CFLAGS "-I\${includedir} -std=c++20")
	if(APPLE)
		set(ORGE_PC_LIBS "-L\${libdir} -lorgestatic -lvulkan.1")
	else()
		set(ORGE_PC_LIBS "-L\${libdir} -lorgestatic -lvulkan")
	endif()
	set(ORGE_PC_LIBS_PRIVATE "${SDL3_DEPS} ${YAML_CPP_DEPS}")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/orgestatic.pc.in ${CMAKE_CURRENT_BINARY_DIR}/orgestatic.pc @ONLY)

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/liborgestatic.a DESTINATION ${CMAKE_INSTALL_LIBDIR})
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/orgestatic.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# NOTE: macOSはVulkanローダを標準搭載していないのでライブラリタイプがなんであれ。
if(APPLE OR LINUX AND ORGE_STATIC)
	install(FILES $<TARGET_FILE:Vulkan::Loader> TYPE LIB)
	install(SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/cmake/create-vulkan-symlink.cmake)
endif()
