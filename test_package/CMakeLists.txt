cmake_minimum_required(VERSION 3.21 FATAL_ERROR)
project(teiacare_video_io_test_package CXX)

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/modules)
find_package(teiacare_video_io CONFIG REQUIRED)

add_executable(teiacare_video_io_test_package src/test.cpp)
target_link_libraries(teiacare_video_io_test_package PRIVATE teiacare::video_io)
install(TARGETS teiacare_video_io_test_package DESTINATION .)
