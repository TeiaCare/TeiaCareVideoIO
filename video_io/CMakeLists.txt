set(TARGET_NAME teiacare_video_io)
add_library(${TARGET_NAME})
add_library(teiacare::video_io ALIAS ${TARGET_NAME})

find_package(ffmpeg CONFIG REQUIRED)

configure_file(
    src/version.cpp.in
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp
)

set(TARGET_HEADERS
    include/teiacare/video_io/version.hpp
    include/teiacare/video_io/video_info.hpp
    include/teiacare/video_io/video_reader.hpp
    include/teiacare/video_io/video_writer.hpp
)

set(TARGET_SOURCES
    src/logger.hpp
    src/version.cpp
    src/video_info.cpp
    src/video_reader_hw.cpp
    src/video_reader_hw.hpp
    src/video_reader.cpp
    src/video_writer.cpp
)

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_20)
target_sources(${TARGET_NAME} PUBLIC ${TARGET_HEADERS} PRIVATE ${TARGET_SOURCES})
target_link_libraries(${TARGET_NAME}
    PRIVATE
        ffmpeg::avformat
        ffmpeg::avcodec
        ffmpeg::swscale
        ffmpeg::avutil
        OpenSSL::SSL
        OpenSSL::Crypto
)
target_include_directories(${TARGET_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:CMAKE_INSTALL_INCLUDEDIR>
)
set_target_properties(${TARGET_NAME} PROPERTIES VERSION ${${PROJECT_NAME}_VERSION} SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR})
set_target_properties(${TARGET_NAME} PROPERTIES PUBLIC_HEADER "${TARGET_HEADERS}")
install(TARGETS ${TARGET_NAME} PUBLIC_HEADER DESTINATION include/teiacare/video_io)


if(TC_ENABLE_WARNINGS_ERROR)
    include(warnings)
    add_warnings(${TARGET_NAME})
    add_warnings_as_errors(${TARGET_NAME})
endif()

if(TC_ENABLE_SANITIZER_ADDRESS)
    include(sanitizer_address)
    add_sanitizer_address(${TARGET_NAME})
endif()

if(TC_ENABLE_SANITIZER_THREAD)
    include(sanitizer_thread)
    add_sanitizer_thread(${TARGET_NAME})
endif()

#################################################################
# Unit Tests
if(TC_ENABLE_UNIT_TESTS)
    set(TEST_TARGET_NAME teiacare_inference_client_unittests)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    set(VIDEO_IO_VIDEO_DATA_ABS_PATH ${CMAKE_SOURCE_DIR}/data/)
    configure_file(src/utils/video_data_path.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/video_data_path.cpp)

    set(UNIT_TESTS_SRC
        tests/main.cpp
        tests/utils/video_data_path.cpp
        tests/utils/video_data_path.hpp
        tests/utils/video_params.hpp
        tests/test_video_reader.hpp
        tests/test_video_reader.cpp
        tests/test_video_writer.hpp
        tests/test_video_writer.cpp
    )

    add_executable(${TEST_TARGET_NAME})
    target_sources(${TEST_TARGET_NAME} PRIVATE ${UNIT_TESTS_SRC})
    target_include_directories(${TEST_TARGET_NAME} PRIVATE  ${CMAKE_CURRENT_BINARY_DIR}/proto)
    target_compile_features(${TEST_TARGET_NAME} PUBLIC cxx_std_20)
    target_link_libraries(${TEST_TARGET_NAME}
        PRIVATE
            GTest::gtest
            GTest::gmock
            teiacare::video_io
    )

    gtest_discover_tests(${TEST_TARGET_NAME})
    install(TARGETS ${TEST_TARGET_NAME} DESTINATION unit_tests)

    if(TC_ENABLE_WARNINGS_ERROR)
        add_warnings(${TEST_TARGET_NAME})
        add_warnings_as_errors(${TEST_TARGET_NAME})
    endif()

    if(TC_ENABLE_UNIT_TESTS_COVERAGE)
        include(coverage)
        add_coverage(${TARGET_NAME})
    endif()
endif()

#################################################################
# Examples
if(TC_ENABLE_EXAMPLES)
endif()
