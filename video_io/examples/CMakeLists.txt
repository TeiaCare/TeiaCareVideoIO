function(add_example EXAMPLE_NAME)
    add_executable(${EXAMPLE_NAME} src/${EXAMPLE_NAME}.cpp)
    target_compile_features(${EXAMPLE_NAME} PUBLIC cxx_std_20)
    target_link_libraries(${EXAMPLE_NAME} PRIVATE teiacare::video_io)
    install(TARGETS ${EXAMPLE_NAME} DESTINATION examples)
endfunction()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data/ 
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/data
)

find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

add_example(video_reader_simple_decode)
add_example(video_writer_simple_encode)

# video_reader_opengl_player
add_example(video_reader_opengl_player)
target_link_libraries(video_reader_opengl_player PRIVATE glfw)

# video_reader_imgui_player
set(IMGUI_BINDINGS_DIR ${CMAKE_SOURCE_DIR}/build/imgui_bindings)
add_executable(video_reader_imgui_player
    src/video_reader_imgui_player.cpp
    ${IMGUI_BINDINGS_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_BINDINGS_DIR}/imgui_impl_glfw.h
    ${IMGUI_BINDINGS_DIR}/imgui_impl_opengl3.cpp
    ${IMGUI_BINDINGS_DIR}/imgui_impl_opengl3.h
)
target_include_directories(video_reader_imgui_player PRIVATE ${IMGUI_BINDINGS_DIR})
target_link_libraries(video_reader_imgui_player PRIVATE teiacare::video_io imgui::imgui glfw)
target_compile_features(video_reader_imgui_player PUBLIC cxx_std_20)
install(TARGETS video_reader_imgui_player DESTINATION examples)
