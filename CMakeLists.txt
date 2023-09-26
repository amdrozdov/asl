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
add_library(slice STATIC src/slice.cpp)

include(ExternalProject)
ExternalProject_Add(gtest
  URL https://github.com/google/googletest/archive/release-1.8.0.zip
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
  INSTALL_COMMAND ""
)
ExternalProject_Get_Property(gtest source_dir binary_dir)
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/gtest/src/gtest/googletest/include/)

enable_testing()
add_executable(
  wav_test
  tests/wav.cpp
)
target_include_directories(wav_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(wav_test ${CMAKE_CURRENT_BINARY_DIR}/gtest/src/gtest-build/googlemock/gtest/libgtest.a)
target_link_libraries(wav_test ${CMAKE_CURRENT_BINARY_DIR}/gtest/src/gtest-build/googlemock/gtest/libgtest_main.a)
target_link_libraries(wav_test slice)

add_executable(
  ulaw_test
  tests/ulaw.cpp
)
target_include_directories(ulaw_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ulaw_test ${CMAKE_CURRENT_BINARY_DIR}/gtest/src/gtest-build/googlemock/gtest/libgtest.a)
target_link_libraries(ulaw_test ${CMAKE_CURRENT_BINARY_DIR}/gtest/src/gtest-build/googlemock/gtest/libgtest_main.a)
target_link_libraries(ulaw_test slice)

add_executable(
  alaw_test
  tests/alaw.cpp
)
target_include_directories(alaw_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(alaw_test ${CMAKE_CURRENT_BINARY_DIR}/gtest/src/gtest-build/googlemock/gtest/libgtest.a)
target_link_libraries(alaw_test ${CMAKE_CURRENT_BINARY_DIR}/gtest/src/gtest-build/googlemock/gtest/libgtest_main.a)
target_link_libraries(alaw_test slice)

include(GoogleTest)
gtest_discover_tests(wav_test)
gtest_discover_tests(ulaw_test)
gtest_discover_tests(alaw_test)
