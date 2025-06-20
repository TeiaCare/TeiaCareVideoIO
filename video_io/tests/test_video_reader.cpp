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

#include <fstream>
#include <thread>

namespace tc::vio::tests
{
TEST_F(video_reader_test, open_with_hw_acceleration_preference)
{
    ASSERT_TRUE(v->open(default_video_path.string().c_str(), decode_support::HW));
    ASSERT_TRUE(v->is_opened());
}

TEST_F(video_reader_test, open_with_sw_acceleration_preference)
{
    ASSERT_TRUE(v->open(default_video_path.string().c_str(), decode_support::SW));
    ASSERT_TRUE(v->is_opened());
}

// Screen capture tests
// TEST_F(video_reader_test, open_screen_capture)
// {
//     screen_options opts{};
//     // This might fail on CI environments, so we just check it doesn't crash
//     v->open("screen", opts);
//     // No assertion on success since screen capture availability varies
// }

// Getter method tests
TEST_F(video_reader_test, get_frame_count_without_open)
{
    ASSERT_FALSE(v->is_opened());
    auto frame_count = v->get_frame_count();
    ASSERT_FALSE(frame_count.has_value());
}

TEST_F(video_reader_test, get_frame_count_with_open)
{
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));
    auto frame_count = v->get_frame_count();
    ASSERT_TRUE(frame_count.has_value());
    ASSERT_GT(frame_count.value(), 0);
}

TEST_F(video_reader_test, get_duration_without_open)
{
    ASSERT_FALSE(v->is_opened());
    auto duration = v->get_duration();
    ASSERT_FALSE(duration.has_value());
}

TEST_F(video_reader_test, get_duration_with_open)
{
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));
    auto duration = v->get_duration();
    ASSERT_TRUE(duration.has_value());
    ASSERT_GT(duration.value().count(), 0);
}

TEST_F(video_reader_test, get_frame_size_without_open)
{
    ASSERT_FALSE(v->is_opened());
    auto frame_size = v->get_frame_size();
    ASSERT_FALSE(frame_size.has_value());
}

TEST_F(video_reader_test, get_frame_size_with_open)
{
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));
    auto frame_size = v->get_frame_size();
    ASSERT_TRUE(frame_size.has_value());
    auto [width, height] = frame_size.value();
    ASSERT_GT(width, 0);
    ASSERT_GT(height, 0);
}

TEST_F(video_reader_test, get_frame_size_in_bytes_without_open)
{
    ASSERT_FALSE(v->is_opened());
    auto frame_size_bytes = v->get_frame_size_in_bytes();
    ASSERT_FALSE(frame_size_bytes.has_value());
}

TEST_F(video_reader_test, get_frame_size_in_bytes_with_open)
{
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));
    auto frame_size_bytes = v->get_frame_size_in_bytes();
    ASSERT_TRUE(frame_size_bytes.has_value());
    ASSERT_GT(frame_size_bytes.value(), 0);
}

TEST_F(video_reader_test, get_fps_without_open)
{
    ASSERT_FALSE(v->is_opened());
    auto fps = v->get_fps();
    ASSERT_FALSE(fps.has_value());
}

TEST_F(video_reader_test, get_fps_with_open)
{
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));
    auto fps = v->get_fps();
    ASSERT_TRUE(fps.has_value());
    ASSERT_GT(fps.value(), 0.0);
}

// Read with PTS tests
TEST_F(video_reader_test, read_with_pts)
{
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));
    uint8_t* data_buffer = frame_data.data();
    double pts = 0.0;
    ASSERT_TRUE(v->read(&data_buffer, &pts));
    ASSERT_NE(data_buffer, nullptr);
    ASSERT_GE(pts, 0.0);
}

TEST_F(video_reader_test, read_without_pts)
{
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));
    uint8_t* data_buffer = frame_data.data();
    ASSERT_TRUE(v->read(&data_buffer, nullptr));
    ASSERT_NE(data_buffer, nullptr);
}

TEST_F(video_reader_test, read_multiple_frames_check_pts_progression)
{
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));
    uint8_t* data_buffer = frame_data.data();
    double pts1 = 0.0, pts2 = 0.0;

    ASSERT_TRUE(v->read(&data_buffer, &pts1));
    ASSERT_TRUE(v->read(&data_buffer, &pts2));
    ASSERT_GT(pts2, pts1);
}

// Edge cases and error conditions
// TEST_F(video_reader_test, open_null_path)
// {
//     ASSERT_FALSE(v->open(nullptr));
//     ASSERT_FALSE(v->is_opened());
// }

TEST_F(video_reader_test, open_empty_path)
{
    ASSERT_FALSE(v->open(""));
    ASSERT_FALSE(v->is_opened());
}

// TEST_F(video_reader_test, read_null_data_pointer)
// {
//     ASSERT_TRUE(v->open(default_video_path.string().c_str()));
//     ASSERT_FALSE(v->read(nullptr));
// }

TEST_F(video_reader_test, read_until_eof)
{
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));
    uint8_t* data_buffer = frame_data.data();

    int frame_count = 0;
    while (v->read(&data_buffer))
    {
        frame_count++;
        ASSERT_NE(data_buffer, nullptr);
        if (frame_count > 1000) // Prevent infinite loop
            break;
    }
    ASSERT_GT(frame_count, 0);
}

TEST_F(video_reader_test, multiple_releases)
{
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));
    v->release();
    v->release();
    v->release();
    ASSERT_FALSE(v->is_opened());
}

TEST_F(video_reader_test, open_after_eof)
{
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));
    uint8_t* data_buffer = frame_data.data();

    // Read until EOF
    while (v->read(&data_buffer))
    {
    }

    // Try to open again
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));
    ASSERT_TRUE(v->is_opened());
    ASSERT_TRUE(v->read(&data_buffer));
}

// Corrupted file tests
TEST_F(video_reader_test, open_corrupted_file)
{
    // Create a temporary corrupted file
    const auto corrupted_path = default_input_directory / "corrupted.mp4";
    std::ofstream corrupted_file(corrupted_path, std::ios::binary);
    corrupted_file << "This is not a valid video file";
    corrupted_file.close();

    ASSERT_FALSE(v->open(corrupted_path.string().c_str()));
    ASSERT_FALSE(v->is_opened());

    std::filesystem::remove(corrupted_path);
}

// State consistency tests
TEST_F(video_reader_test, is_opened_consistency)
{
    ASSERT_FALSE(v->is_opened());

    ASSERT_TRUE(v->open(default_video_path.string().c_str()));
    ASSERT_TRUE(v->is_opened());

    uint8_t* data_buffer = frame_data.data();
    ASSERT_TRUE(v->read(&data_buffer));
    ASSERT_TRUE(v->is_opened());

    v->release();
    ASSERT_FALSE(v->is_opened());
}

// Constructor/Destructor tests
TEST_F(video_reader_test, destructor_cleanup)
{
    {
        auto local_reader = std::make_unique<vio::video_reader>();
        ASSERT_TRUE(local_reader->open(default_video_path.string().c_str()));
        ASSERT_TRUE(local_reader->is_opened());
    } // Destructor should clean up properly
    // No assertion needed, just verify no crash
}

// Flush functionality test
TEST_F(video_reader_test, flush_decoder)
{
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));
    uint8_t* data_buffer = frame_data.data();

    // Read a few frames
    ASSERT_TRUE(v->read(&data_buffer));
    ASSERT_TRUE(v->read(&data_buffer));

    // Flush should work (this tests the private flush method indirectly)
    v->release(); // This calls flush internally
    ASSERT_FALSE(v->is_opened());
}

// Metadata consistency tests
TEST_F(video_reader_test, metadata_consistency_across_operations)
{
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));

    auto initial_fps = v->get_fps();
    auto initial_size = v->get_frame_size();
    auto initial_duration = v->get_duration();

    uint8_t* data_buffer = frame_data.data();
    v->read(&data_buffer);

    // Metadata should remain consistent after reading
    ASSERT_EQ(v->get_fps().value(), initial_fps.value());
    ASSERT_EQ(v->get_frame_size().value(), initial_size.value());
    ASSERT_EQ(v->get_duration().value(), initial_duration.value());
}

TEST_F(video_reader_test, open_release_without_read)
{
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));
    ASSERT_TRUE(v->is_opened());

    v->release();
    ASSERT_FALSE(v->is_opened());
}

TEST_F(video_reader_test, open_read_one_frame_release)
{
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));
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
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));
    ASSERT_TRUE(v->open(default_video_path.string().c_str()));

    ASSERT_TRUE(v->is_opened());
}

TEST_F(video_reader_test, open_different_paths_without_read)
{
    const auto video_path1 = (default_input_directory / "video_10sec_4fps_HD.mp4");
    ASSERT_TRUE(v->open(video_path1.string().c_str()));
    ASSERT_TRUE(v->is_opened());

    const auto video_path2 = (default_input_directory / "video_10sec_4fps_HD.mkv");
    ASSERT_TRUE(v->open(video_path2.string().c_str()));
    ASSERT_TRUE(v->is_opened());

    const auto video_path3 = (default_input_directory / "video_2sec_2fps_HD.mp4");
    ASSERT_TRUE(v->open(video_path3.string().c_str()));
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
                             utils::video_params({.name = "video_2sec_2fps_HD", .format = "mp4", .duration = 2, .width = 640, .height = 480, .fps = 2}),
                             utils::video_params({.name = "video_10sec_1fps_HD", .format = "mp4", .duration = 10, .width = 1280, .height = 720, .fps = 1}),
                             utils::video_params({.name = "video_10sec_2fps_HD", .format = "mp4", .duration = 10, .width = 1280, .height = 720, .fps = 2}),
                             utils::video_params({.name = "video_10sec_4fps_4K", .format = "mp4", .duration = 10, .width = 3840, .height = 2160, .fps = 4}),
                             utils::video_params({.name = "video_10sec_4fps_FHD", .format = "mp4", .duration = 10, .width = 1920, .height = 1080, .fps = 4}),
                             utils::video_params({.name = "video_10sec_4fps_HD", .format = "mp4", .duration = 10, .width = 1280, .height = 720, .fps = 4}),
                             utils::video_params({.name = "video_10sec_4fps_SD", .format = "mp4", .duration = 10, .width = 640, .height = 480, .fps = 4}),
                             utils::video_params({.name = "video_10sec_8fps_HD", .format = "mp4", .duration = 10, .width = 1280, .height = 720, .fps = 8}),
                             utils::video_params({.name = "video_10sec_16fps_HD", .format = "mp4", .duration = 10, .width = 1280, .height = 720, .fps = 16}),
                             utils::video_params({.name = "video_10sec_30fps_HD", .format = "mp4", .duration = 10, .width = 1280, .height = 720, .fps = 30})
                             // utils::video_params({.name="video_120sec_30fps_SD", .format="mp4", .duration=120, .width=640, .height=480, .fps=30})
                             ),
                         [](auto info) { return info.param.name; });

INSTANTIATE_TEST_SUITE_P(video_reader_MKV,
                         parametrized_video_reader_test,
                         ::testing::Values(
                             utils::video_params({.name = "video_2sec_2fps_HD", .format = "mkv", .duration = 2, .width = 640, .height = 480, .fps = 2}),
                             utils::video_params({.name = "video_10sec_1fps_HD", .format = "mkv", .duration = 10, .width = 1280, .height = 720, .fps = 1}),
                             utils::video_params({.name = "video_10sec_2fps_HD", .format = "mkv", .duration = 10, .width = 1280, .height = 720, .fps = 2}),
                             utils::video_params({.name = "video_10sec_4fps_4K", .format = "mkv", .duration = 10, .width = 3840, .height = 2160, .fps = 4}),
                             utils::video_params({.name = "video_10sec_4fps_FHD", .format = "mkv", .duration = 10, .width = 1920, .height = 1080, .fps = 4}),
                             utils::video_params({.name = "video_10sec_4fps_HD", .format = "mkv", .duration = 10, .width = 1280, .height = 720, .fps = 4}),
                             utils::video_params({.name = "video_10sec_4fps_SD", .format = "mkv", .duration = 10, .width = 640, .height = 480, .fps = 4}),
                             utils::video_params({.name = "video_10sec_8fps_HD", .format = "mkv", .duration = 10, .width = 1280, .height = 720, .fps = 8}),
                             utils::video_params({.name = "video_10sec_16fps_HD", .format = "mkv", .duration = 10, .width = 1280, .height = 720, .fps = 16}),
                             utils::video_params({.name = "video_10sec_30fps_HD", .format = "mkv", .duration = 10, .width = 1280, .height = 720, .fps = 30})
                             // utils::video_params({.name="video_120sec_30fps_SD", .format="mkv", .duration=120, .width=640, .height=480, .fps=30})
                             ),
                         [](auto info) { return info.param.name; });

TEST_P(parametrized_video_reader_test, read)
{
    const utils::video_params params = GetParam();
    const std::filesystem::path video_path = default_input_directory / (params.name + "." + params.format);
    read_full_video(v, video_path, params.duration, params.fps, params.width, params.height);
}

}
