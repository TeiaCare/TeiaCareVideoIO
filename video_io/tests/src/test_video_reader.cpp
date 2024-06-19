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

namespace tc::vio::test
{

TEST_F(video_reader_test, open_valid_path)
{
    ASSERT_TRUE(v->open(default_video_path.c_str()));
    ASSERT_TRUE(v->is_opened());
}

TEST_F(video_reader_test, open_release_without_read)
{
    ASSERT_TRUE(v->open(default_video_path.c_str()));
    ASSERT_TRUE(v->is_opened());

    ASSERT_TRUE(v->release());
    ASSERT_FALSE(v->is_opened());
}

TEST_F(video_reader_test, open_read_release)
{
    ASSERT_TRUE(v->open(default_video_path.c_str()));
    ASSERT_TRUE(v->is_opened());

    uint8_t* data_buffer = frame_data.data();
    ASSERT_TRUE(v->read(&data_buffer));
    ASSERT_TRUE(v->is_opened());

    ASSERT_TRUE(v->release());
    ASSERT_FALSE(v->is_opened());
}

TEST_F(video_reader_test, read_without_open)
{
    ASSERT_FALSE(v->is_opened());

    uint8_t* data_buffer = frame_data.data();
    ASSERT_FALSE(v->read(&data_buffer));

    ASSERT_FALSE(v->is_opened());
    ASSERT_FALSE(v->release());
}

TEST_F(video_reader_test, release_without_open)
{
    ASSERT_FALSE(v->is_opened());
    ASSERT_FALSE(v->release());
    ASSERT_FALSE(v->is_opened());
}

TEST_F(video_reader_test, open_same_path_three_times)
{
    ASSERT_TRUE(v->open(default_video_path.c_str()));
    ASSERT_TRUE(v->open(default_video_path.c_str()));
    ASSERT_TRUE(v->open(default_video_path.c_str()));

    ASSERT_TRUE(v->is_opened());
}

TEST_F(video_reader_test, open_non_existing_path)
{
    const auto invalid_video_path = default_input_directory / "invalid-path.mp4";
    ASSERT_FALSE(v->open(invalid_video_path.c_str()));
    ASSERT_FALSE(v->is_opened());
}

TEST_F(video_reader_test, open_three_different_paths)
{
    const auto video_path1 = (default_input_directory / default_video_name).replace_extension(".mp4");
    ASSERT_TRUE(v->open(video_path1.c_str()));

    v->release();

    const auto video_path2 = (default_input_directory / default_video_name).replace_extension(".mpg");
    ASSERT_TRUE(v->open(video_path2.c_str()));

    const auto video_path3 = (default_input_directory / default_video_name).replace_extension(".mkv");
    ASSERT_TRUE(v->open(video_path3.c_str()));

    ASSERT_TRUE(v->is_opened());
}

TEST_P(video_reader_test, read_n_frames)
{
    const std::string video_extension = GetParam();
    const auto video_path = (default_input_directory / default_video_name).replace_extension(default_video_extension);

    ASSERT_TRUE(v->open(video_path.c_str()));
    ASSERT_TRUE(v->is_opened());

    int num_decoded_frames = 0;
    const int num_frames_to_read = 300;
    while(num_decoded_frames < num_frames_to_read)
    {
        uint8_t* data_buffer = frame_data.data();
        ASSERT_TRUE(v->read(&data_buffer));
        num_decoded_frames++;
    }

    ASSERT_EQ(num_decoded_frames, num_frames_to_read);
    ASSERT_FALSE(v->is_opened());
}

INSTANTIATE_TEST_SUITE_P(multi_format, video_reader_test, ::testing::Values(".mp4", ".mpg", ".mkv", ".avi"));

}
