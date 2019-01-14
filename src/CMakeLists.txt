cmake_minimum_required(VERSION 3.10)
project(sdl2-sample)

find_package(SDL2 REQUIRED)
include_directories(sdl2-sample ${SDL2_INCLUDE_DIRS})

add_executable(sdl2-sample main.cpp CounterWidget.h)
configure_file(points points COPYONLY)

target_link_libraries(sdl2-sample ${SDL2_LIBRARIES})