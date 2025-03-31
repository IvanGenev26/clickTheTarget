cmake_minimum_required(VERSION 3.29)
project(clickTheTargetGame)

set(CMAKE_CXX_STANDARD 20)

find_package(raylib REQUIRED)

add_executable(clickTheTargetGame main.cpp)

target_link_libraries(clickTheTargetGame PRIVATE raylib)
target_link_libraries(clickTheTargetGame PRIVATE "-framework OpenGL")
target_link_libraries(clickTheTargetGame PRIVATE "-framework Cocoa")
target_link_libraries(clickTheTargetGame PRIVATE "-framework IOKit")
