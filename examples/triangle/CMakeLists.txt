add_executable(triangle main.cpp)

if(TARGET orge::orge)
	target_link_libraries(triangle PRIVATE orge::orge)
elseif(TARGET orge::orgestatic)
	target_link_libraries(triangle PRIVATE orge::orgestatic)
	# NOTE: MSVCでは/MTを指定
	if(MSVC)
		set_target_properties(triangle PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	endif()
endif()

add_custom_command(
	TARGET triangle POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/config.yml
		$<TARGET_FILE_DIR:triangle>
# 	COMMAND ${CMAKE_COMMAND} -E copy_if_different
# 		${CMAKE_CURRENT_SOURCE_DIR}/shader.vert.spv
# 		$<TARGET_FILE_DIR:triangle>
# 	COMMAND ${CMAKE_COMMAND} -E copy_if_different
# 		${CMAKE_CURRENT_SOURCE_DIR}/shader.frag.spv
# 		$<TARGET_FILE_DIR:triangle>
)
