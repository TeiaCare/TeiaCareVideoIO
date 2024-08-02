// Copyright 2024 TeiaCare
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "test_video_reader.hpp"

#include <gtest/gtest.h>

namespace tc::vio::tests
{

TEST_F(video_reader_test, open_valid_video_path)
{
    ASSERT_TRUE(v->open(default_video_path.c_str()));
    ASSERT_TRUE(v->is_opened());
}

TEST_F(video_reader_test, open_non_existing_video_path)
{
    const auto invalid_video_path = default_input_directory / "invalid-path.mp4";
    ASSERT_FALSE(v->open(invalid_video_path.c_str()));
    ASSERT_FALSE(v->is_opened());
}

TEST_F(video_reader_test, open_release_without_read)
{
    ASSERT_TRUE(v->open(default_video_path.c_str()));
    ASSERT_TRUE(v->is_opened());

    v->release();
    ASSERT_FALSE(v->is_opened());
}

TEST_F(video_reader_test, open_read_one_frame_release)
{
    ASSERT_TRUE(v->open(default_video_path.c_str()));
    ASSERT_TRUE(v->is_opened());

    uint8_t* data_buffer = frame_data.data();
    ASSERT_TRUE(v->read(&data_buffer));
    ASSERT_TRUE(v->is_opened());

    v->release();
    ASSERT_FALSE(v->is_opened());
}

TEST_F(video_reader_test, read_without_open)
{
    ASSERT_FALSE(v->is_opened());

    uint8_t* data_buffer = frame_data.data();
    ASSERT_FALSE(v->read(&data_buffer));

    ASSERT_FALSE(v->is_opened());
    v->release();
}

TEST_F(video_reader_test, release_without_open)
{
    ASSERT_FALSE(v->is_opened());
    v->release();
    ASSERT_FALSE(v->is_opened());
}

TEST_F(video_reader_test, open_same_path_without_read)
{
    ASSERT_TRUE(v->open(default_video_path.c_str()));
    ASSERT_TRUE(v->open(default_video_path.c_str()));
    ASSERT_TRUE(v->open(default_video_path.c_str()));

    ASSERT_TRUE(v->is_opened());
}

TEST_F(video_reader_test, open_different_paths_without_read)
{
    const auto video_path1 = (default_input_directory / "video_10sec_4fps_HD.mp4");
    ASSERT_TRUE(v->open(video_path1.c_str()));
    ASSERT_TRUE(v->is_opened());

    const auto video_path2 = (default_input_directory / "video_10sec_4fps_HD.mkv");
    ASSERT_TRUE(v->open(video_path2.c_str()));
    ASSERT_TRUE(v->is_opened());

    const auto video_path3 = (default_input_directory / "video_2sec_2fps_HD.mp4");
    ASSERT_TRUE(v->open(video_path3.c_str()));
    ASSERT_TRUE(v->is_opened());
}

TEST_F(video_reader_test, open_another_path_after_read_half_video)
{
    constexpr const int video_duration_in_seconds = 10;
    constexpr const int video_width = 1280;
    constexpr const int video_height = 720;
    constexpr const double fps = 4.0;

    read_half_video(v, default_input_directory / "video_10sec_4fps_HD.mp4", video_duration_in_seconds, fps, video_width, video_height);
    read_half_video(v, default_input_directory / "video_10sec_4fps_HD.mkv", video_duration_in_seconds, fps, video_width, video_height);
}

TEST_F(video_reader_test, open_different_paths_read_all_files_consecutively)
{
    constexpr const int video_duration_in_seconds = 10;
    constexpr const int video_width = 1280;
    constexpr const int video_height = 720;
    constexpr const double fps = 4.0;

    read_full_video(v, default_input_directory / "video_10sec_4fps_SD.mp4", video_duration_in_seconds, fps, video_width, video_height);
    read_full_video(v, default_input_directory / "video_10sec_4fps_HD.mkv", video_duration_in_seconds, fps, video_width, video_height);
    read_full_video(v, default_input_directory / "video_10sec_4fps_FHD.mp4", video_duration_in_seconds, fps, video_width, video_height);
}

TEST_F(video_reader_test, open_different_paths_read_all_files_in_parallel)
{
    constexpr const int video_duration_in_seconds = 10;
    constexpr const int video_width = 1280;
    constexpr const int video_height = 720;
    constexpr const double video_fps = 4.0;

    auto reader = [](const std::filesystem::path& video_path) {
        auto v = std::make_unique<vio::video_reader>();
        read_full_video(v, video_path, video_duration_in_seconds, video_fps, video_width, video_height);
    };

    std::vector<std::filesystem::path> paths = {
        default_input_directory / "video_10sec_4fps_HD.mp4",
        default_input_directory / "video_10sec_4fps_HD.mkv",
        default_input_directory / "video_10sec_4fps_HD.mkv"};

    std::vector<std::thread> reader_threads;
    for (auto&& video_path : paths)
    {
        reader_threads.push_back(std::thread(reader, video_path));
    }

    for (auto&& reader_thread : reader_threads)
    {
        reader_thread.join();
    }
}

INSTANTIATE_TEST_SUITE_P(video_reader_MP4,
                         parametrized_video_reader_test,
                         ::testing::Values(
                             video_reader_params({.name = "video_2sec_2fps_HD", .format = "mp4", .duration = 2, .width = 640, .height = 480, .fps = 2}),
                             video_reader_params({.name = "video_10sec_1fps_HD", .format = "mp4", .duration = 10, .width = 1280, .height = 720, .fps = 1}),
                             video_reader_params({.name = "video_10sec_2fps_HD", .format = "mp4", .duration = 10, .width = 1280, .height = 720, .fps = 2}),
                             video_reader_params({.name = "video_10sec_4fps_4K", .format = "mp4", .duration = 10, .width = 3840, .height = 2160, .fps = 4}),
                             video_reader_params({.name = "video_10sec_4fps_FHD", .format = "mp4", .duration = 10, .width = 1920, .height = 1080, .fps = 4}),
                             video_reader_params({.name = "video_10sec_4fps_HD", .format = "mp4", .duration = 10, .width = 1280, .height = 720, .fps = 4}),
                             video_reader_params({.name = "video_10sec_4fps_SD", .format = "mp4", .duration = 10, .width = 640, .height = 480, .fps = 4}),
                             video_reader_params({.name = "video_10sec_8fps_HD", .format = "mp4", .duration = 10, .width = 1280, .height = 720, .fps = 8}),
                             video_reader_params({.name = "video_10sec_16fps_HD", .format = "mp4", .duration = 10, .width = 1280, .height = 720, .fps = 16}),
                             video_reader_params({.name = "video_10sec_30fps_HD", .format = "mp4", .duration = 10, .width = 1280, .height = 720, .fps = 30})
                             // video_reader_params({.name="video_120sec_30fps_SD", .format="mp4", .duration=120, .width=640, .height=480, .fps=30})
                             ),
                         [](auto info) { return info.param.name; });

INSTANTIATE_TEST_SUITE_P(video_reader_MKV,
                         parametrized_video_reader_test,
                         ::testing::Values(
                             video_reader_params({.name = "video_2sec_2fps_HD", .format = "mkv", .duration = 2, .width = 640, .height = 480, .fps = 2}),
                             video_reader_params({.name = "video_10sec_1fps_HD", .format = "mkv", .duration = 10, .width = 1280, .height = 720, .fps = 1}),
                             video_reader_params({.name = "video_10sec_2fps_HD", .format = "mkv", .duration = 10, .width = 1280, .height = 720, .fps = 2}),
                             video_reader_params({.name = "video_10sec_4fps_4K", .format = "mkv", .duration = 10, .width = 3840, .height = 2160, .fps = 4}),
                             video_reader_params({.name = "video_10sec_4fps_FHD", .format = "mkv", .duration = 10, .width = 1920, .height = 1080, .fps = 4}),
                             video_reader_params({.name = "video_10sec_4fps_HD", .format = "mkv", .duration = 10, .width = 1280, .height = 720, .fps = 4}),
                             video_reader_params({.name = "video_10sec_4fps_SD", .format = "mkv", .duration = 10, .width = 640, .height = 480, .fps = 4}),
                             video_reader_params({.name = "video_10sec_8fps_HD", .format = "mkv", .duration = 10, .width = 1280, .height = 720, .fps = 8}),
                             video_reader_params({.name = "video_10sec_16fps_HD", .format = "mkv", .duration = 10, .width = 1280, .height = 720, .fps = 16}),
                             video_reader_params({.name = "video_10sec_30fps_HD", .format = "mkv", .duration = 10, .width = 1280, .height = 720, .fps = 30})
                             // video_reader_params({.name="video_120sec_30fps_SD", .format="mkv", .duration=120, .width=640, .height=480, .fps=30})
                             ),
                         [](auto info) { return info.param.name; });

TEST_P(parametrized_video_reader_test, read)
{
    const video_reader_params params = GetParam();
    const std::filesystem::path video_path = default_input_directory / (params.name + "." + params.format);
    read_full_video(v, video_path, params.duration, params.fps, params.width, params.height);
}

}
