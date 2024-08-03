include(examples)

set(VIDEO_IO_VIDEO_DATA_ABS_PATH ${CMAKE_SOURCE_DIR}/data/)
configure_file(src/utils/video_data_path.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/video_data_path.cpp)

# Video Decoding example
add_example(video_reader_simple_decode)

# Video Encoding example
add_example(video_writer_simple_encode)

# Video Player (OpenGL)
add_example(video_reader_opengl_player)
target_link_libraries(video_reader_opengl_player PRIVATE glfw)

# Video Player with decoding on background thread (OpenGL)
add_example(video_reader_opengl_player_multi_thread)
target_link_libraries(video_reader_opengl_player_multi_thread PRIVATE glfw)

# Video Player (Dear ImGui)
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
set(IMGUI_BINDINGS_DIR ${CMAKE_SOURCE_DIR}/build/imgui_bindings)
add_executable(video_reader_imgui_player
    src/video_reader_imgui_player.cpp
    src/utils/video_data_path.cpp
    ${IMGUI_BINDINGS_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_BINDINGS_DIR}/imgui_impl_glfw.h
    ${IMGUI_BINDINGS_DIR}/imgui_impl_opengl3.cpp
    ${IMGUI_BINDINGS_DIR}/imgui_impl_opengl3.h
)
target_include_directories(video_reader_imgui_player PRIVATE ${IMGUI_BINDINGS_DIR})
target_link_libraries(video_reader_imgui_player PRIVATE teiacare::video_io imgui::imgui glfw)
target_compile_features(video_reader_imgui_player PUBLIC cxx_std_20)
install(TARGETS video_reader_imgui_player DESTINATION examples)
