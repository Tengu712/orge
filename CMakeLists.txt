cmake_minimum_required(VERSION 3.5)
project(orge)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_library(orge STATIC src/lib.cpp)
add_library(orge::orge ALIAS orge)

add_subdirectory(examples)
