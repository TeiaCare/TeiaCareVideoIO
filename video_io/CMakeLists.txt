set(TARGET_NAME teiacare_video_io)
add_library(${TARGET_NAME})
add_library(teiacare::video_io ALIAS ${TARGET_NAME})

find_package(ffmpeg CONFIG REQUIRED)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp.in
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp
)

set(TARGET_HEADERS
    include/teiacare/video_io/ivideo_reader.hpp
    include/teiacare/video_io/ivideo_writer.hpp
    include/teiacare/video_io/version.hpp
    include/teiacare/video_io/video_info.hpp
    include/teiacare/video_io/video_reader.hpp
    include/teiacare/video_io/video_writer.hpp
)

set(TARGET_SOURCES
    src/logger.hpp
    src/version.cpp
    src/video_info.cpp
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
    set(TEST_TARGET_NAME teiacare_video_io_unittests)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    set(VIDEO_IO_VIDEO_DATA_ABS_PATH ${CMAKE_SOURCE_DIR}/data/)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/utils/video_data_path.hpp.in
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/utils/video_data_path.hpp)

    set(UNIT_TESTS_SRC
        tests/main.cpp
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
    set(VIDEO_IO_VIDEO_DATA_ABS_PATH ${CMAKE_SOURCE_DIR}/data/)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/utils/video_data_path.hpp.in
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/utils/video_data_path.hpp)

    # Example decoding - Video Reader
    add_executable(example_simple_decode examples/example_simple_decode.cpp)
    target_compile_features(example_simple_decode PUBLIC cxx_std_20)
    target_link_libraries(example_simple_decode PRIVATE teiacare::video_io)
    install(TARGETS example_simple_decode DESTINATION examples)

    # Example encoding - Video Writer
    add_executable(example_simple_encode examples/example_simple_encode.cpp)
    target_compile_features(example_simple_encode PUBLIC cxx_std_20)
    target_link_libraries(example_simple_encode PRIVATE teiacare::video_io)
    install(TARGETS example_simple_encode DESTINATION examples)

    # Video Player (OpenGL)
    find_package(glfw3 CONFIG REQUIRED)
    add_executable(video_reader_opengl_player examples/video_reader_opengl_player.cpp)
    target_compile_features(video_reader_opengl_player PUBLIC cxx_std_20)
    target_link_libraries(video_reader_opengl_player PRIVATE teiacare::video_io glfw)
    install(TARGETS video_reader_opengl_player DESTINATION examples)

    # Video Player with multi threading (OpenGL)
    find_package(glfw3 CONFIG REQUIRED)
    add_executable(video_reader_opengl_player_mt examples/video_reader_opengl_player_multi_thread.cpp)
    target_compile_features(video_reader_opengl_player_mt PUBLIC cxx_std_20)
    target_link_libraries(video_reader_opengl_player_mt PRIVATE teiacare::video_io glfw)
    install(TARGETS video_reader_opengl_player_mt DESTINATION examples)

    # Video Player (Dear ImGui)
    find_package(imgui CONFIG REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)
    set(IMGUI_BINDINGS_DIR ${CMAKE_SOURCE_DIR}/build/imgui_bindings)
    add_executable(video_reader_imgui_player
        examples/video_reader_imgui_player.cpp
        ${IMGUI_BINDINGS_DIR}/imgui_impl_glfw.cpp
        ${IMGUI_BINDINGS_DIR}/imgui_impl_glfw.h
        ${IMGUI_BINDINGS_DIR}/imgui_impl_opengl3.cpp
        ${IMGUI_BINDINGS_DIR}/imgui_impl_opengl3.h
    )
    target_include_directories(video_reader_imgui_player PRIVATE ${IMGUI_BINDINGS_DIR})
    target_compile_features(video_reader_imgui_player PUBLIC cxx_std_20)
    target_link_libraries(video_reader_imgui_player
        PRIVATE
            teiacare::video_io
            imgui::imgui
            glfw
    )
    install(TARGETS video_reader_imgui_player DESTINATION examples)
endif()
