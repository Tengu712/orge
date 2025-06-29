# MSVC用静的ライブラリ統合スクリプト
#
# NOTE: こうでもしないと統合できないので。

string(REPLACE ";" " " ORGE_OBJECTS_SPACED "${ORGE_OBJECTS}")                                                                                                                                     │ │

execute_process(COMMAND lib /OUT:${BINARY_DIR}/orgestatic.lib ${SDL3_LIB} ${YAML_CPP_LIB} ${ORGE_OBJECTS_SPACED})
