cmake_minimum_required(VERSION 3.17)
project(ihct VERSION 0.11)

set(CMAKE_CC_FLAGS "-Wall -Wextra -pedantic -Wfatal-errors")
set(CMAKE_CC_STANDARD 99)
set(CMAKE_BUILD_TYPE Debug)

find_package(Threads REQUIRED)

add_library(ihct
    SHARED
    src/ihct.c
)
target_link_libraries(ihct PRIVATE Threads::Threads)

include_directories("${CMAKE_SOURCE_DIR}/src")

add_executable(example
    src/ihct.c
    examples/ex.c
)
target_link_libraries(example PRIVATE Threads::Threads ihct)

set(inc_dest "include/")
set(lib_dest "lib/")
install(TARGETS ihct DESTINATION ${lib_dest})
install(FILES "${CMAKE_SOURCE_DIR}/src/ihct.h" DESTINATION ${inc_dest})
