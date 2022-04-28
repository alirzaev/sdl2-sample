cmake_minimum_required(VERSION 3.10)
project(sdl2-sample)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
include_directories(sdl2-sample ${SDL2_INCLUDE_DIRS})

add_executable(sdl2-sample main.cpp widget.h transformation.h sdl_helpers.h)

target_link_libraries(sdl2-sample ${SDL2_LIBRARIES})