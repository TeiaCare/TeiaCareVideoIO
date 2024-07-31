function(setup_unit_tests TARGET_NAME)
	set(TARGET_NAME_UNIT_TEST ${TARGET_NAME}_unit_tests)
	find_package(GTest REQUIRED)
	include(GoogleTest)
	add_executable(${TARGET_NAME_UNIT_TEST})
	target_sources(${TARGET_NAME_UNIT_TEST} PRIVATE ${ARGN})
	target_compile_features(${TARGET_NAME_UNIT_TEST} PUBLIC cxx_std_20)
	target_link_libraries(${TARGET_NAME_UNIT_TEST} PRIVATE GTest::GTest PRIVATE ${TARGET_NAME})
	gtest_discover_tests(${TARGET_NAME_UNIT_TEST} PROPERTIES TEST_DISCOVERY_TIMEOUT 30)
    install(TARGETS ${TARGET_NAME_UNIT_TEST} DESTINATION unit_tests)
	target_compile_definitions(${TARGET_NAME_UNIT_TEST} PUBLIC VIDEO_IO_UNIT_TESTS_DATA="${CMAKE_SOURCE_DIR}/data")
endfunction()
