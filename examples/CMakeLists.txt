option(BUILD_EXAMPLES "examplesをビルドする" OFF)

option(BUILD_EXAMPLE_SIMPLE "examples/simpleをビルドする" OFF)
if(BUILD_EXAMPLES OR BUILD_EXAMPLE_SIMPLE)
	add_subdirectory(simple)
endif()

option(BUILD_EXAMPLE_TRIANGLE "examples/triangleをビルドする" OFF)
if(BUILD_EXAMPLES OR BUILD_EXAMPLE_TRIANGLE)
	add_subdirectory(triangle)
endif()

option(BUILD_EXAMPLE_CUBE "examples/cubeをビルドする" OFF)
if(BUILD_EXAMPLES OR BUILD_EXAMPLE_CUBE)
	add_subdirectory(cube)
endif()

option(BUILD_EXAMPLE_INPUT "examples/inputをビルドする" OFF)
if(BUILD_EXAMPLES OR BUILD_EXAMPLE_INPUT)
	add_subdirectory(input)
endif()

option(BUILD_EXAMPLE_SAMPLER "examples/samplerをビルドする" OFF)
if(BUILD_EXAMPLES OR BUILD_EXAMPLE_SAMPLER)
	add_subdirectory(sampler)
endif()

option(BUILD_EXAMPLE_MULTIPASS "examples/multipassをビルドする" OFF)
if(BUILD_EXAMPLES OR BUILD_EXAMPLE_MULTIPASS)
	add_subdirectory(multipass)
endif()
