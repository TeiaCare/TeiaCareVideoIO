include(benchmarks)
find_package(OpenCV)

setup_benchmarks(benchmark_video_reader_opencv teiacare::video_io)
setup_benchmarks(benchmark_video_writer_opencv teiacare::video_io)

# TODO: use CMake script to setup benchmarks

# include(benchmarks)
# set(BENCHMARKS_SRC
#     src/benchmark_event_dispatcher.cpp
#     src/benchmark_event_dispatcher.hpp
#     src/main.cpp
# )
# setup_benchmarks(${TARGET_NAME} ${BENCHMARKS_SRC})
