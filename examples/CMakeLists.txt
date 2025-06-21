option(BUILD_EXAMPLE_SIMPLE "examples/simpleをビルドする" OFF)
if(BUILD_EXAMPLE_SIMPLE)
	add_subdirectory(simple)
endif()
