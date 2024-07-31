function(add_example EXAMPLE_NAME)
    add_executable(${EXAMPLE_NAME} src/${EXAMPLE_NAME}.cpp)
    target_compile_features(${EXAMPLE_NAME} PUBLIC cxx_std_20)
    target_link_libraries(${EXAMPLE_NAME} PRIVATE teiacare::video_io)
    install(TARGETS ${EXAMPLE_NAME} DESTINATION examples)
endfunction()
