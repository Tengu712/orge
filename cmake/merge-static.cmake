# 静的ライブラリ統合スクリプト
#
# NOTE: こうでもしないと再起的に.oを取得して統合できないので。

# ディレクトリ作成
file(MAKE_DIRECTORY ${BINARY_DIR}/o)
file(MAKE_DIRECTORY ${BINARY_DIR}/o/orge)
file(MAKE_DIRECTORY ${BINARY_DIR}/o/sdl3)
file(MAKE_DIRECTORY ${BINARY_DIR}/o/yaml-cpp)

# 依存ライブラリ展開
execute_process(
	COMMAND ${CMAKE_COMMAND} -E chdir ${BINARY_DIR}/o/sdl3     ar -x ${SDL3_LIB}
	RESULT_VARIABLE AR_SDL3_RESULT
)
execute_process(
	COMMAND ${CMAKE_COMMAND} -E chdir ${BINARY_DIR}/o/yaml-cpp ar -x ${YAML_CPP_LIB}
	RESULT_VARIABLE AR_YAML_CPP_RESULT
)

# 全.oファイルを再帰的に検索
file(GLOB_RECURSE OBJECTS "${BINARY_DIR}/o/*.o")

# 統合アーカイブ作成
execute_process(
	COMMAND ar -rc ${BINARY_DIR}/liborgestatic.a ${OBJECTS} ${ORGE_OBJECTS}
	RESULT_VARIABLE AR_RC_ORGESTATIC_RESULT
)

# 確認
if(
	NOT AR_SDL3_RESULT EQUAL 0
	OR NOT AR_YAML_CPP_RESULT EQUAL 0
	OR NOT AR_RC_ORGESTATIC_RESULT EQUAL 0
)
	message(FATAL_ERROR "Failed to create merged static library")
endif()

# ディレクトリ削除
file(REMOVE_RECURSE ${BINARY_DIR}/o)
