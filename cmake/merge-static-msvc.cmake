# MSVC用静的ライブラリ統合スクリプト
#
# NOTE: こうでもしないと統合できないので。

execute_process(
	COMMAND lib /OUT:${BINARY_DIR}/orgestatic.lib ${ORGE_LIB} ${SDL3_LIB} ${YAML_CPP_LIB}
	RESULT_VARIABLE LIB_RESULT
)

if(NOT LIB_RESULT EQUAL 0)
	message(FATAL_ERROR "Failed to create merged static library")
endif()
