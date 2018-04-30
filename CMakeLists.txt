cmake_minimum_required(VERSION 3.10)
project(Proyecto C)

set(CMAKE_C_STANDARD 99)

find_package (Threads)

add_executable(Launcher launcher.c lib/launcherUtils.c headers/launcherUtils.h lib/coms.c headers/coms.h headers/tiempo.h lib/tiempo.c lib/priorityUtils.c headers/priorityUtils.h)
add_executable(Receptor receptor.c lib/ticketUtils.c headers/ticketUtils.h lib/inits.c headers/inits.h lib/coms.c headers/coms.h lib/launcherUtils.c headers/launcherUtils.h lib/exit.c headers/exit.h headers/tiempo.h lib/tiempo.c lib/priorityUtils.c headers/priorityUtils.h)
add_executable(Process process.c lib/ticketUtils.c headers/ticketUtils.h lib/inits.c headers/inits.h lib/coms.c headers/coms.h lib/launcherUtils.c headers/launcherUtils.h lib/exit.c headers/exit.h headers/tiempo.h lib/tiempo.c lib/logger.c headers/logger.h lib/priorityUtils.c headers/priorityUtils.h)

target_link_libraries (Process ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (Receptor ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (Launcher ${CMAKE_THREAD_LIBS_INIT})