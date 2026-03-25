cmake_minimum_required(VERSION 3.10)

project(wandoo C)

set(CMAKE_C_STANDARD 99)

add_executable(wandoo main.c)

find_package(Curses REQUIRED)
target_link_libraries(wandoo PRIVATE ${CURSES_LIBRARIES})
target_include_directories(wandoo PRIVATE ${CURSES_INCLUDE_DIR})

install(TARGETS wandoo DESTINATION bin)
