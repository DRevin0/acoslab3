cmake_minimum_required(VERSION 3.10)
project(lr2_parallel_text_processing)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

set(SOURCES
    main.cpp
)

add_executable(lr2_parallel_text_processing ${SOURCES})

target_link_libraries(lr2_parallel_text_processing Threads::Threads)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(lr2_parallel_text_processing PRIVATE -O2)
endif()

message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
