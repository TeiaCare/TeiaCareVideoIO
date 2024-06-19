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

#include <gtest/gtest.h>
#include <teiacare/video_io/video_reader.hpp>

#include <filesystem>

namespace tc::vio::test
{

class video_reader_test : public ::testing::TestWithParam<std::string>
{
protected:
    explicit video_reader_test()
    : v{ std::make_unique<vio::video_reader>() }
    , test_name { testing::UnitTest::GetInstance()->current_test_info()->name() }
    , default_input_directory{ std::filesystem::current_path() / "../../../tests/data/new" }
    , default_video_extension { ".mp4" }
    , default_video_name { "testsrc2_3sec_30fps_640x480" }
    , default_video_path { (default_input_directory / default_video_name).replace_extension(default_video_extension) }
    { }

    virtual ~video_reader_test() { v->release(); }

    virtual void SetUp() override { }
    virtual void TearDown() override { }

    std::unique_ptr<vio::video_reader> v;
    const std::string test_name;
    const std::filesystem::path default_input_directory;
    const std::string default_video_extension;
    const std::string default_video_name;
    const std::filesystem::path default_video_path;

    static const int fps = 30;
    static const int width = 640;
	static const int height = 480;

    static const int frame_size = width * height * 3;
    std::array<uint8_t, frame_size> frame_data = { };

private:
    template<typename... Args>
    void log(Args&&... args) const
    {
        ((std::cout << std::forward<Args>(args) << ' ') , ...) << std::endl;
    }
};

}
