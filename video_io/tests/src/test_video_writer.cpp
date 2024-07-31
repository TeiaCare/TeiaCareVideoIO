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

#include "test_video_writer.hpp"

#include <teiacare/video_io/video_writer.hpp>

#include <filesystem>
#include <thread>

namespace tc::vio::test
{

TEST_F(video_writer_test, open_without_duration)
{
    ASSERT_TRUE(v->open(default_video_path, width, height, fps));
}

TEST_F(video_writer_test, open_with_duration)
{
    ASSERT_TRUE(v->open(default_video_path, width, height, fps, duration));
}

TEST_F(video_writer_test, open_invalid_width)
{
    ASSERT_FALSE(v->open(default_video_path, -123, height, fps));
}

TEST_F(video_writer_test, open_invalid_height)
{
    ASSERT_FALSE(v->open(default_video_path, width, -123, fps));
}

TEST_F(video_writer_test, open_invalid_fps)
{
    ASSERT_FALSE(v->open(default_video_path, width, height, -123));
}

TEST_F(video_writer_test, open_invalid_duration)
{
    ASSERT_FALSE(v->open(default_video_path, width, height, fps, -123));
}

TEST_F(video_writer_test, open_invalid_extension_default_to_mpeg)
{
    const auto invalid_video_path = (default_output_directory / test_name).replace_extension("invalid-extension");
    ASSERT_TRUE(v->open(invalid_video_path, width, height, fps));

    ASSERT_TRUE(v->write(frame_data.data()));
    ASSERT_TRUE(v->save());

    // verify written file is mpeg format
}

TEST_F(video_writer_test, open_release_without_write)
{
    ASSERT_TRUE(v->open(default_video_path, width, height, fps));
    ASSERT_TRUE(v->is_opened());

    ASSERT_TRUE(v->release());
    ASSERT_FALSE(v->is_opened());
}

TEST_F(video_writer_test, open_save_without_write)
{
    ASSERT_TRUE(v->open(default_video_path, width, height, fps));
    ASSERT_TRUE(v->is_opened());

    ASSERT_TRUE(v->save());
    ASSERT_FALSE(v->is_opened());
}

TEST_F(video_writer_test, open_write_release_without_save)
{
    ASSERT_TRUE(v->open(default_video_path, width, height, fps));
    ASSERT_TRUE(v->is_opened());

    ASSERT_TRUE(v->write(frame_data.data()));

    const auto is_released = v->release();
    ASSERT_TRUE(is_released);
    ASSERT_FALSE(v->is_opened());

    // assert file does not exists
}

TEST_F(video_writer_test, open_write_save_without_release)
{
    ASSERT_TRUE(v->open(default_video_path, width, height, fps));
    ASSERT_TRUE(v->is_opened());

    ASSERT_TRUE(v->write(frame_data.data()));

    ASSERT_TRUE(v->save());
    ASSERT_FALSE(v->is_opened());

    // assert file exists
}

TEST_F(video_writer_test, open_write_save_release)
{
    ASSERT_TRUE(v->open(default_video_path, width, height, fps));
    ASSERT_TRUE(v->is_opened());

    ASSERT_TRUE(v->write(frame_data.data()));
    ASSERT_TRUE(v->is_opened());

    ASSERT_TRUE(v->save());
    ASSERT_FALSE(v->is_opened());

    v->release();
    ASSERT_FALSE(v->is_opened());

    // assert file exists
}

TEST_F(video_writer_test, write_without_open)
{
    ASSERT_FALSE(v->is_opened());
    ASSERT_FALSE(v->write(frame_data.data()));

    ASSERT_FALSE(v->is_opened());
    v->release();
}

TEST_F(video_writer_test, save_without_open)
{
    ASSERT_FALSE(v->is_opened());
    ASSERT_FALSE(v->save());

    ASSERT_FALSE(v->is_opened());
    v->release();
}

TEST_F(video_writer_test, release_without_open)
{
    ASSERT_FALSE(v->is_opened());
    v->release();
    ASSERT_FALSE(v->is_opened());
}

TEST_F(video_writer_test, open_same_path_three_times)
{
    ASSERT_TRUE(v->open(default_video_path, width, height, fps));
    ASSERT_TRUE(v->open(default_video_path, width, height, fps));
    ASSERT_TRUE(v->open(default_video_path, width, height, fps));

    ASSERT_TRUE(v->is_opened());
}

TEST_F(video_writer_test, open_non_existing_path)
{
    const auto video_path = (default_output_directory / "not_existing" / test_name).replace_extension(default_video_extension);
    ASSERT_FALSE(v->open(video_path, width, height, fps));
    ASSERT_FALSE(v->is_opened());
}

TEST_F(video_writer_test, open_three_different_paths)
{
    const auto video_path1 = (default_output_directory / (test_name + "1")).replace_extension(default_video_extension);
    ASSERT_TRUE(v->open(video_path1, width, height, fps));

    const auto video_path2 = (default_output_directory / (test_name + "2")).replace_extension(default_video_extension);
    ASSERT_TRUE(v->open(video_path2, width, height, fps));

    const auto video_path3 = (default_output_directory / (test_name + "3")).replace_extension(default_video_extension);
    ASSERT_TRUE(v->open(video_path3, width, height, fps));

    ASSERT_TRUE(v->is_opened());
}

TEST_P(video_writer_test, write_n_frames)
{
    const std::string video_extension = GetParam();
    const auto video_path = (default_output_directory / test_name).replace_extension(video_extension);

    if (!std::filesystem::exists(video_path.parent_path()))
    {
        std::filesystem::create_directories(video_path.parent_path());
    }

    ASSERT_TRUE(v->open(video_path, width, height, fps));
    ASSERT_TRUE(v->is_opened());

    int num_written_frames = 0;
    const int num_frames_to_write = 300;
    while (num_written_frames < num_frames_to_write)
    {
        ASSERT_TRUE(v->write(frame_data.data()));
        num_written_frames++;
    }

    ASSERT_EQ(num_written_frames, num_frames_to_write);

    ASSERT_TRUE(v->save());
    ASSERT_FALSE(v->is_opened());

    // Check video info
}

TEST_P(video_writer_test, write_n_seconds)
{
    const std::string video_extension = GetParam();
    const auto video_path = (default_output_directory / test_name).replace_extension(video_extension);

    if (!std::filesystem::exists(video_path.parent_path()))
    {
        std::filesystem::create_directories(video_path.parent_path());
    }

    const int duration_to_write_in_seconds = 10;
    ASSERT_TRUE(v->open(video_path, width, height, fps, duration_to_write_in_seconds));
    ASSERT_TRUE(v->is_opened());

    int num_written_frames = 0;
    while (true)
    {
        if (!v->write(frame_data.data()))
            break;

        num_written_frames++;
    }

    ASSERT_EQ(num_written_frames, duration_to_write_in_seconds * fps);

    ASSERT_TRUE(v->save());
    ASSERT_FALSE(v->is_opened());

    // Check video info
}

TEST_P(video_writer_test, write_parallel)
{
    const std::string video_extension = GetParam();

    constexpr int parallel_count = 4;
    std::array<std::thread, parallel_count> threads;
    std::array<std::shared_ptr<video_writer>, parallel_count> writers;

    auto writer_callback = [this](const std::shared_ptr<video_writer>& writer, const std::filesystem::path& video_path) {
        if (!std::filesystem::exists(video_path.parent_path()))
        {
            std::filesystem::create_directories(video_path.parent_path());
        }

        ASSERT_TRUE(writer->open(video_path, width, height, fps));
        ASSERT_TRUE(writer->is_opened());

        int num_written_frames = 0;
        const int num_frames_to_write = 300;
        while (num_written_frames < num_frames_to_write)
        {
            ASSERT_TRUE(writer->write(frame_data.data()));
            num_written_frames++;
        }

        ASSERT_EQ(num_written_frames, num_frames_to_write);
        ASSERT_TRUE(writer->save());
        ASSERT_FALSE(writer->is_opened());
    };

    for (auto i = 0; i < parallel_count; ++i)
    {
        writers[i] = std::make_shared<video_writer>();
    }

    for (auto i = 0; i < parallel_count; ++i)
    {
        const auto video_path = (default_output_directory / test_name / std::to_string(i)).replace_extension(video_extension);
        threads[i] = std::thread(writer_callback, writers[i], video_path);
    }

    for (auto i = 0; i < parallel_count; ++i)
    {
        threads[i].join();
        ASSERT_FALSE(writers[i]->is_opened());
        ASSERT_FALSE(writers[i]->release());
    }
}

INSTANTIATE_TEST_SUITE_P(multi_format, video_writer_test, ::testing::Values(".mp4", ".mpeg", ".avi"));

/*
TODO:
- N parallel writers
- always cleanup generated files (use TearDown?)

*/

}
