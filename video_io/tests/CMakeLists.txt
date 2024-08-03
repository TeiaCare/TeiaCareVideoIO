if(TC_ENABLE_UNIT_TESTS_COVERAGE)
    include(coverage)
    add_coverage(${TARGET_NAME})
endif()

set(VIDEO_IO_VIDEO_DATA_ABS_PATH ${CMAKE_SOURCE_DIR}/data/)
configure_file(src/utils/video_data_path.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/video_data_path.cpp)

include(unit_tests)
set(UNIT_TESTS_SRC
    src/main.cpp
    src/utils/video_data_path.cpp
    src/utils/video_data_path.hpp
    src/utils/video_params.hpp
    src/test_video_reader.hpp
    src/test_video_reader.cpp
    #src/test_video_writer.hpp
    #src/test_video_writer.cpp
)
setup_unit_tests(${TARGET_NAME} ${UNIT_TESTS_SRC})

# Disable warnings on GCC (-Wall compiler flag), due to a bug in GTest 1.14.0 in Release with GCC 12 and std=c++20
# https://github.com/google/googletest/issues/4108
if(TC_ENABLE_WARNINGS_ERROR)
    if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" AND NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        message(STATUS "Unit Tests warning disabled for GCC/Release (see GoogleTest issue: https://github.com/google/googletest/issues/4108)")
        return()
    endif()

    include(warnings)
    add_warnings(${TARGET_NAME}_unit_tests)
    add_warnings_as_errors(${TARGET_NAME}_unit_tests)
endif()
