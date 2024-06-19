set(TARGET_NAME comparison_opencv_video_io)

find_package(OpenCV)

add_executable(${TARGET_NAME} ${TARGET_NAME}.cpp)

target_link_libraries(${TARGET_NAME} 
    PRIVATE vio::video_io
    PRIVATE opencv::highgui
    PRIVATE opencv::videoio
)