cmake_minimum_required(VERSION 3.10)
project(Proyecto C)

set(CMAKE_C_STANDARD 99)

find_package (Threads)

add_executable(Launcher launcher.c)
add_executable(Main main.c lib/ticketUtils.c headers/ticketUtils.h lib/inits.c headers/inits.h lib/coms.c headers/coms.h)

target_link_libraries (Main ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (Launcher ${CMAKE_THREAD_LIBS_INIT})