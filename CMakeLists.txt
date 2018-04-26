cmake_minimum_required(VERSION 3.10)
project(Proyecto C)

set(CMAKE_C_STANDARD 99)

find_package (Threads)

add_executable(Launcher launcher.c)
add_executable(Main main.c)

target_link_libraries (Main ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (Receptor ${CMAKE_THREAD_LIBS_INIT})