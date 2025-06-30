# 静的ライブラリ作成用pkg-configから依存ライブラリを取得する関数

find_package(PkgConfig REQUIRED)

function(get_deps_from_pc PKG_NAME OUTPUT)
	pkg_check_modules(${PKG_NAME}_PC QUIET ${PKG_NAME})

	if(${PKG_NAME}_PC_FOUND)
		# STATIC_LDFLAGSとSTATIC_LDFLAGS_OTHERを結合
		set(ALL_LDFLAGS ${${PKG_NAME}_PC_STATIC_LDFLAGS} ${${PKG_NAME}_PC_STATIC_LDFLAGS_OTHER})

		# 格納先
		set(PROCESSED_LIBS)

		# リストを走査
		foreach(FLAG ${ALL_LDFLAGS})
			# -Lから始まる要素をスキップ
			if(FLAG MATCHES "^-L")
				continue()
			endif()

			# 先頭の-lを削除
			if(FLAG MATCHES "^-l(.+)")
				set(IS_LIBRARY 1)
				set(LIB_NAME ${CMAKE_MATCH_1})
			else()
				set(IS_LIBRARY 0)
				set(LIB_NAME ${FLAG})
			endif()

			# パッケージ自身の場合はスキップ
			if(LIB_NAME STREQUAL "${PKG_NAME}" OR LIB_NAME STREQUAL "${PKG_NAME}-static")
				continue()
			endif()

			# ライブラリであった場合は是正
			if(IS_LIBRARY)
				if(MSVC AND NOT LIB_NAME MATCHES "\\.lib$")
					set(LIB_NAME "${LIB_NAME}.lib")
				else(NOT MSVC)
					set(LIB_NAME "-l${LIB_NAME}")
				endif()
			endif()

			# リストに追加
			list(APPEND PROCESSED_LIBS ${LIB_NAME})
		endforeach()

		# リストから空白区切りの文字列へ
		string(REPLACE ";" " " FORMATTED_LIBS "${PROCESSED_LIBS}")

		# 返戻
		set(${OUTPUT} "${FORMATTED_LIBS}" PARENT_SCOPE)
		message(STATUS "get dependencies from ${PKG_NAME}: ${FORMATTED_LIBS}")
	else()
		message(WARNING "${PKG_NAME} pkg-config not found. Dependencies may be incomplete.")
		set(${OUTPUT} "" PARENT_SCOPE)
	endif()
endfunction()
