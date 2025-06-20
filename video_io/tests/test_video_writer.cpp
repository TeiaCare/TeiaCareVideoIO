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

#include <thread>

namespace tc::vio::tests
{

TEST_F(video_writer_test, open_without_duration)
{
    EXPECT_TRUE(v->open(default_video_path, width, height, fps));
}

TEST_F(video_writer_test, open_with_duration)
{
    EXPECT_TRUE(v->open(default_video_path, width, height, fps, duration));
}

TEST_F(video_writer_test, open_invalid_width)
{
    EXPECT_FALSE(v->open(default_video_path, -123, height, fps));
}

TEST_F(video_writer_test, open_invalid_height)
{
    EXPECT_FALSE(v->open(default_video_path, width, -123, fps));
}

TEST_F(video_writer_test, open_invalid_fps)
{
    EXPECT_FALSE(v->open(default_video_path, width, height, -123));
}

TEST_F(video_writer_test, open_invalid_duration)
{
    EXPECT_FALSE(v->open(default_video_path, width, height, fps, -123));
}

TEST_F(video_writer_test, open_invalid_extension_default_to_mp4)
{
    const auto invalid_video_path = (default_output_directory / "video").replace_extension("invalid-extension");
    EXPECT_TRUE(v->open(invalid_video_path, width, height, fps));

    constexpr const int num_frames_to_write = 20;
    for (int written_frames = 0; written_frames < num_frames_to_write; ++written_frames)
    {
        EXPECT_TRUE(v->write(frame_data.data()));
    }

    EXPECT_TRUE(v->save());

    const tc::vio::video_metadata expected_metadata =
        {
            .id = 1,
            .width = width,
            .height = height,
            .nb_frames = num_frames_to_write,
            .r_frame_rate = fps,
            .avg_frame_rate = fps,
            .duration = num_frames_to_write / fps,
            .codec_name = "h264"};

    const auto video_metadata = info->get_video_metadata(invalid_video_path);
    EXPECT_EQ(video_metadata.value(), expected_metadata);
}

TEST_F(video_writer_test, open_release_without_write)
{
    EXPECT_TRUE(v->open(default_video_path, width, height, fps));
    EXPECT_TRUE(v->is_opened());

    EXPECT_NO_THROW(v->release());
    EXPECT_FALSE(v->is_opened());
}

TEST_F(video_writer_test, open_save_without_write)
{
    EXPECT_TRUE(v->open(default_video_path, width, height, fps));
    EXPECT_TRUE(v->is_opened());

    EXPECT_TRUE(v->save());
    EXPECT_FALSE(v->is_opened());
}

TEST_F(video_writer_test, open_write_release_without_save)
{
    EXPECT_TRUE(v->open(default_video_path, width, height, fps));
    EXPECT_TRUE(v->is_opened());

    EXPECT_TRUE(v->write(frame_data.data()));

    EXPECT_NO_THROW(v->release());
    EXPECT_FALSE(v->is_opened());

    // assert file does not exists
}

TEST_F(video_writer_test, open_write_save_without_release)
{
    EXPECT_TRUE(v->open(default_video_path, width, height, fps));
    EXPECT_TRUE(v->is_opened());

    EXPECT_TRUE(v->write(frame_data.data()));

    EXPECT_TRUE(v->save());
    EXPECT_FALSE(v->is_opened());

    // assert file exists
}

TEST_F(video_writer_test, open_write_save_release)
{
    EXPECT_TRUE(v->open(default_video_path, width, height, fps));
    EXPECT_TRUE(v->is_opened());

    EXPECT_TRUE(v->write(frame_data.data()));
    EXPECT_TRUE(v->is_opened());

    EXPECT_TRUE(v->save());
    EXPECT_FALSE(v->is_opened());

    v->release();
    EXPECT_FALSE(v->is_opened());

    // assert file exists
}

TEST_F(video_writer_test, write_without_open)
{
    EXPECT_FALSE(v->is_opened());
    EXPECT_FALSE(v->write(frame_data.data()));

    EXPECT_FALSE(v->is_opened());
    v->release();
}

TEST_F(video_writer_test, save_without_open)
{
    EXPECT_FALSE(v->is_opened());
    EXPECT_FALSE(v->save());

    EXPECT_FALSE(v->is_opened());
    v->release();
}

TEST_F(video_writer_test, release_without_open)
{
    EXPECT_FALSE(v->is_opened());
    v->release();
    EXPECT_FALSE(v->is_opened());
}

TEST_F(video_writer_test, open_same_path_three_times)
{
    EXPECT_TRUE(v->open(default_video_path, width, height, fps));
    EXPECT_TRUE(v->open(default_video_path, width, height, fps));
    EXPECT_TRUE(v->open(default_video_path, width, height, fps));

    EXPECT_TRUE(v->is_opened());
}

TEST_F(video_writer_test, open_non_existing_video_path)
{
    const auto video_path = (default_output_directory / "not_existing_file").replace_extension(default_video_extension);
    EXPECT_TRUE(v->open(video_path.c_str(), width, height, fps));
    EXPECT_TRUE(v->is_opened());
}

TEST_F(video_writer_test, open_non_existing_folder_path)
{
    const auto video_path = (default_output_directory / "not_existing_folder" / "not_existing_file").replace_extension(default_video_extension);
    EXPECT_FALSE(v->open(video_path.c_str(), width, height, fps));
    EXPECT_FALSE(v->is_opened());
}

TEST_F(video_writer_test, open_three_different_paths)
{
    const auto video_path1 = (default_output_directory / "1").replace_extension(default_video_extension);
    EXPECT_TRUE(v->open(video_path1, width, height, fps));

    const auto video_path2 = (default_output_directory / "2").replace_extension(default_video_extension);
    EXPECT_TRUE(v->open(video_path2, width, height, fps));

    const auto video_path3 = (default_output_directory / "3").replace_extension(default_video_extension);
    EXPECT_TRUE(v->open(video_path3, width, height, fps));

    EXPECT_TRUE(v->is_opened());
}

TEST_P(video_writer_test, write_n_frames)
{
    const std::string video_extension = GetParam();
    const auto video_path = (default_output_directory / ("video_n_frames" + video_extension)).replace_extension(video_extension);

    if (!std::filesystem::exists(video_path.parent_path()))
    {
        std::filesystem::create_directories(video_path.parent_path());
    }

    EXPECT_TRUE(v->open(video_path, width, height, fps));
    EXPECT_TRUE(v->is_opened());

    int num_written_frames = 0;
    const int num_frames_to_write = 300;
    while (num_written_frames < num_frames_to_write)
    {
        EXPECT_TRUE(v->write(frame_data.data()));
        num_written_frames++;
    }

    EXPECT_EQ(num_written_frames, num_frames_to_write);

    EXPECT_TRUE(v->save());
    EXPECT_FALSE(v->is_opened());

    // Check video info
}

TEST_P(video_writer_test, write_n_seconds)
{
    const std::string video_extension = GetParam();
    const auto video_path = (default_output_directory / ("video_n_seconds" + video_extension)).replace_extension(video_extension);

    if (!std::filesystem::exists(video_path.parent_path()))
    {
        std::filesystem::create_directories(video_path.parent_path());
    }

    const int duration_to_write_in_seconds = 10;
    EXPECT_TRUE(v->open(video_path, width, height, fps, duration_to_write_in_seconds));
    EXPECT_TRUE(v->is_opened());

    int num_written_frames = 0;
    while (true)
    {
        if (!v->write(frame_data.data()))
            break;

        num_written_frames++;
    }

    EXPECT_EQ(num_written_frames, duration_to_write_in_seconds * fps);

    EXPECT_TRUE(v->save());
    EXPECT_FALSE(v->is_opened());

    // Check video info
}

TEST_P(video_writer_test, write_parallel)
{
    const std::string video_extension = "." + GetParam();

    constexpr int parallel_count = 4;
    std::array<std::thread, parallel_count> threads;
    std::array<std::shared_ptr<video_writer>, parallel_count> writers;

    auto writer_callback = [this](const std::shared_ptr<video_writer>& writer, const std::filesystem::path& video_path) {
        if (!std::filesystem::exists(video_path.parent_path()))
        {
            std::filesystem::create_directories(video_path.parent_path());
        }

        EXPECT_TRUE(writer->open(video_path, width, height, fps));
        EXPECT_TRUE(writer->is_opened());

        int num_written_frames = 0;
        const int num_frames_to_write = 300;
        while (num_written_frames < num_frames_to_write)
        {
            EXPECT_TRUE(writer->write(frame_data.data()));
            num_written_frames++;
        }

        EXPECT_EQ(num_written_frames, num_frames_to_write);
        EXPECT_TRUE(writer->save());
        EXPECT_FALSE(writer->is_opened());
    };

    for (auto i = 0; i < parallel_count; ++i)
    {
        writers[i] = std::make_shared<video_writer>();
    }

    for (auto i = 0; i < parallel_count; ++i)
    {
        const auto video_path = (default_output_directory / (video_extension + std::to_string(i))).replace_extension(video_extension);
        threads[i] = std::thread(writer_callback, writers[i], video_path);
    }

    for (auto i = 0; i < parallel_count; ++i)
    {
        threads[i].join();
        EXPECT_FALSE(writers[i]->is_opened());
        EXPECT_NO_THROW(writers[i]->release());
    }
}

INSTANTIATE_TEST_SUITE_P(multi_format,
                         video_writer_test,
                         ::testing::Values("mp4", "mkv"),
                         [](auto info) { return info.param; });

/*
TODO:
- N parallel writers
- always cleanup generated files (use TearDown?)

*/

}
