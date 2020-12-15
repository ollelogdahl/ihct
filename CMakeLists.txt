cmake_minimum_required(VERSION 3.17)
project(ihct)

set(CMAKE_CC_FLAGS "-Wall -Wfatal-errors")
set(CMAKE_CC_STANDARD 99)
set(CMAKE_BUILD_TYPE Debug)

add_compile_definitions(IHCT_TEST_SELF)

add_executable(ihct
    src/ihct.c
    examples/ex.c
)
target_include_directories(ihct PUBLIC
    "${CMAKE_SOURCE_DIR}/src"
)