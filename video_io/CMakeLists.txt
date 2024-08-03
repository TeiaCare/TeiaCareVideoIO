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

if(TC_ENABLE_CLANG_FORMAT)
    include(clang_format)
    setup_target_clang_format(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR})
endif()

if(TC_ENABLE_CLANG_TIDY)
    include(clang_tidy)
    setup_target_clang_tidy(${TARGET_NAME})
endif()

if(TC_ENABLE_CPPCHECK)
    include(cppcheck)
    setup_target_cppcheck(${TARGET_NAME})
endif()

if(TC_ENABLE_CPPLINT)
    include(cpplint)
    setup_target_cpplint(${TARGET_NAME} ${TARGET_SRC})
endif()

if(TC_ENABLE_UNIT_TESTS)
    add_subdirectory(tests)
endif()

if(TC_ENABLE_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(TC_ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif()
