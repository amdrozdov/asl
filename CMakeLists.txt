cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

project(asl)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external_includes)
execute_process(
    COMMAND git clone "https://github.com/p-ranav/argparse.git" argparse
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/external_includes
)
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/external_includes/argparse/include/)

add_executable(asl src/main.cpp src/slice.cpp)
