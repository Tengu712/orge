add_executable(multipass main.cpp)

if(TARGET orge::orge)
	target_link_libraries(multipass PRIVATE orge::orge)
elseif(TARGET orge::orgestatic)
	target_link_libraries(multipass PRIVATE orge::orgestatic)
	# NOTE: MSVCでは/MTを指定
	if(MSVC)
		set_target_properties(multipass PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	endif()
endif()

add_custom_command(
	TARGET multipass POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/config.yml
		$<TARGET_FILE_DIR:multipass>
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/mesh.vert.spv
		$<TARGET_FILE_DIR:multipass>
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/mesh.frag.spv
		$<TARGET_FILE_DIR:multipass>
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/pattern.frag.spv
		$<TARGET_FILE_DIR:multipass>
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/integration.frag.spv
		$<TARGET_FILE_DIR:multipass>
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/screen.vert.spv
		$<TARGET_FILE_DIR:multipass>
)
