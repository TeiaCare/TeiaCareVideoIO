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

#pragma once

#include <teiacare/video_io/video_reader.hpp>

#include "utils/video_data_path.hpp"
#include <filesystem>
#include <gtest/gtest.h>

namespace tc::vio::tests
{
class video_reader_test : public testing::Test
{
protected:
    explicit video_reader_test()
        : v{std::make_unique<vio::video_reader>()}
        , default_input_directory{std::filesystem::path(tc::vio::tests::utils::video_data_path)}
        , default_video_extension{".mp4"}
        , default_video_name{"video_10sec_4fps_HD"}
        , default_video_path{(default_input_directory / default_video_name).replace_extension(default_video_extension)}
    {
    }

    virtual ~video_reader_test()
    {
        v->release();
    }

    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }

    std::unique_ptr<vio::video_reader> v;
    const std::filesystem::path default_input_directory;
    const std::string default_video_extension;
    const std::string default_video_name;
    const std::filesystem::path default_video_path;

    static const int width = 640;
    static const int height = 480;
    static const int frame_size = width * height * 3;
    std::array<uint8_t, frame_size> frame_data = {};
};

void read_half_video(const std::unique_ptr<vio::video_reader>& v, const std::filesystem::path& video_path, int video_duration_in_seconds, double fps, int width, int height)
{
    ASSERT_TRUE(v->open(video_path.c_str()));
    ASSERT_TRUE(v->is_opened());

    // Initialize the dummy frame
    const size_t frame_size = width * height * 3;
    std::vector<uint8_t> frame_data = {};
    frame_data.resize(frame_size);
    uint8_t* data_buffer = frame_data.data();

    const int total_video_frames_divided_by_two = static_cast<int>(fps * video_duration_in_seconds * 0.5);

    for (int current_frame_index = 0; current_frame_index != total_video_frames_divided_by_two; ++current_frame_index)
    {
        ASSERT_TRUE(v->read(&data_buffer));
    }
}

void read_full_video(const std::unique_ptr<vio::video_reader>& v, const std::filesystem::path& video_path, int video_duration_in_seconds, double fps, int width, int height)
{
    ASSERT_TRUE(v->open(video_path.c_str()));
    ASSERT_TRUE(v->is_opened());

    // Initialize the dummy frame
    const size_t frame_size = width * height * 3;
    std::vector<uint8_t> frame_data = {};
    frame_data.resize(frame_size);
    uint8_t* data_buffer = frame_data.data();

    double pts = 0.0;
    double current_pts = 0.0;

    // The expcted number of video frames is equal to video_duration * FPS
    const int total_frames = static_cast<int>(fps) * video_duration_in_seconds;

    // The expected pts increment (for each consecutive frame) is equal to 1 / FPS
    const double pts_increment = 1.0 / fps;

    // Set the absolute error equal to half of the pts increment due to ossible rounding errors on floating points operations.
    const double absolute_pts_error = pts_increment * 0.5;

    for (int current_frame_index = 0; current_frame_index != total_frames; ++current_frame_index)
    {
        ASSERT_TRUE(v->read(&data_buffer, &pts));
        ASSERT_NE(data_buffer, nullptr);
        ASSERT_EQ(frame_data.size(), frame_size);
        ASSERT_NEAR(pts, current_pts, absolute_pts_error);
        current_pts += pts_increment;
    }

    // Verify that no more frames can be read once the video is finished
    for (int current_frame_index = 0; current_frame_index != total_frames; ++current_frame_index)
    {
        ASSERT_FALSE(v->read(&data_buffer, &pts));
        ASSERT_EQ(data_buffer, nullptr);
        ASSERT_EQ(pts, -1.0);
    }

    v->release();
    ASSERT_FALSE(v->is_opened());
}

struct video_reader_params
{
    std::string name;
    std::string format;
    size_t duration;
    size_t width;
    size_t height;
    double fps;
};

class parametrized_video_reader_test : public video_reader_test, public testing::WithParamInterface<video_reader_params>
{
};

}
