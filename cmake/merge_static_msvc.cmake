# MSVC用静的ライブラリ統合スクリプト
#
# NOTE: こうでもしないと再帰的に.objを取得して統合できないので。

# ディレクトリ作成
file(MAKE_DIRECTORY ${BINARY_DIR}/obj)
file(MAKE_DIRECTORY ${BINARY_DIR}/obj/orge)
file(MAKE_DIRECTORY ${BINARY_DIR}/obj/sdl3)
file(MAKE_DIRECTORY ${BINARY_DIR}/obj/yaml-cpp)

# 依存ライブラリ展開
execute_process(COMMAND lib /EXTRACT:* /OUT:${BINARY_DIR}/obj/sdl3     ${SDL3_LIB})
execute_process(COMMAND lib /EXTRACT:* /OUT:${BINARY_DIR}/obj/yaml-cpp ${YAML_CPP_LIB})

# 全.objファイルを再帰的に検索
file(GLOB_RECURSE OBJECTS "${BINARY_DIR}/obj/*.obj")

# 統合ライブラリ作成
execute_process(COMMAND lib /OUT:${BINARY_DIR}/orgestatic.lib ${OBJECTS} ${ORGE_OBJECTS})

# ディレクトリ削除
file(REMOVE_RECURSE ${BINARY_DIR}/obj)
