cmake_minimum_required (VERSION 3.14)
project (wandoo C)

find_package(Curses REQUIRED)

add_executable(wandoo main.c)

target_link_libraries(wandoo PRIVATE ${CURSES_LIBRARIES})


