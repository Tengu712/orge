option(BUILD_EXAMPLES "examplesをビルドする" OFF)

option(BUILD_EXAMPLE_SIMPLE "examples/simpleをビルドする" OFF)
if(BUILD_EXAMPLES OR BUILD_EXAMPLE_SIMPLE)
	add_subdirectory(simple)
endif()
