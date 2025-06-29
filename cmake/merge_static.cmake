# Linux/macOS用静的ライブラリ統合スクリプト
#
# NOTE: こうでもしないと再起的に.oを取得して統合できないので。

# ディレクトリ作成
file(MAKE_DIRECTORY ${BINARY_DIR}/o)
file(MAKE_DIRECTORY ${BINARY_DIR}/o/orge)
file(MAKE_DIRECTORY ${BINARY_DIR}/o/sdl3)
file(MAKE_DIRECTORY ${BINARY_DIR}/o/yaml-cpp)

# 依存ライブラリ展開
execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${BINARY_DIR}/o/sdl3     ${CMAKE_AR} -x ${SDL3_LIB})
execute_process(COMMAND ${CMAKE_COMMAND} -E chdir ${BINARY_DIR}/o/yaml-cpp ${CMAKE_AR} -x ${YAML_CPP_LIB})

# 全.oファイルを再帰的に検索
file(GLOB_RECURSE OBJECTS "${BINARY_DIR}/o/*.o")

# 統合アーカイブ作成
execute_process(COMMAND ${CMAKE_AR} -rc ${BINARY_DIR}/liborgestatic.a ${OBJECTS} ${ORGE_OBJECTS})

# ディレクトリ削除
file(REMOVE_RECURSE ${BINARY_DIR}/o)
