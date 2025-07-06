cmake_minimum_required(VERSION 3.13)

project(uffCS_TALKS)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h src/*.hpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(server ${SOURCE_FILES})
