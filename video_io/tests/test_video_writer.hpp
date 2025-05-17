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

#include <teiacare/video_io/video_info.hpp>
#include <teiacare/video_io/video_writer.hpp>

#include "utils/video_data_path.hpp"
#include "utils/video_params.hpp"
#include <array>
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

namespace tc::vio::tests
{

class video_writer_test : public ::testing::TestWithParam<std::string>
{
protected:
    explicit video_writer_test()
        : v{std::make_unique<vio::video_writer>()}
        , info{std::make_unique<vio::video_info>()}
        , default_output_directory{std::filesystem::path(tc::vio::tests::utils::video_data_path)}
        , default_video_extension{".mp4"}
        , default_video_path{(default_output_directory / "output").replace_extension(default_video_extension)}
    {
        frame_data.fill(static_cast<uint8_t>(0));
    }

    virtual ~video_writer_test()
    {
        v->release();
    }

    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }

    std::unique_ptr<vio::video_writer> v;
    std::unique_ptr<vio::video_info> info;
    const std::filesystem::path default_output_directory;
    const std::string default_video_extension;
    const std::filesystem::path default_video_path;

    static const int fps = 2;
    static const int width = 640;
    static const int height = 480;
    static const int duration = 10;
    static const int frame_size = width * height * 3;
    std::array<uint8_t, frame_size> frame_data = {};
};

}

namespace tc::vio
{
// define operator == for video_metadata
}
