cmake_minimum_required(VERSION 3.21)
file(STRINGS "VERSION" VERSION_STR)
project(teiacare_video_io
    VERSION ${VERSION_STR}
    LANGUAGES CXX
    HOMEPAGE_URL "https://github.com/TeiaCare/TeiaCareVideoIO"
    DESCRIPTION "TeiaCareVideoIO is a C++ video encoder and decoder library"
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/modules)

include(GNUInstallDirs)
include(CMakePrintHelpers)

include(compiler_info)
include(options)
validate_project_options()

enable_testing()
add_subdirectory(video_io)
