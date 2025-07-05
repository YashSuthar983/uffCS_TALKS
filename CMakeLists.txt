cmake_minimum_required(VERSION 3.13)

project(RedisStarter)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.h src/*.hpp)  # Fixed typo: GLOB_RECURSE and variable name

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(server ${SOURCE_FILES})
