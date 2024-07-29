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

#include <teiacare/video_io/video_writer.hpp>

#include <benchmark/cppbenchmark.h>
#include <iostream>
#include <opencv2/videoio.hpp>

const auto video_path = "../../../../tests/data/v.mp4";
// const auto video_path = "../../../../tests/data/testsrc_30sec_30fps.mkv";

class VideoWriterFixture_OpenCV : public CppBenchmark::Benchmark
{
public:
    using Benchmark::Benchmark;

protected:
    cv::VideoWriter v;
    cv::Mat frame;

    void Initialize(CppBenchmark::Context& context) override
    {
        if (!v.open(video_path))
        {
            std::cout << "Unable to open " << video_path << std::endl;
            context.Cancel();
            return;
        }

        if (!v.isOpened())
        {
            std::cout << "cv::VideoWriter is not opened" << std::endl;
            context.Cancel();
            return;
        }
    }

    void Cleanup(CppBenchmark::Context& context) override
    {
        v.release();
    }

    void Run(CppBenchmark::Context& context) override
    {
        while (v.write(frame))
        {
        }
    }
};

class VideoWriterFixture_video_io : public CppBenchmark::Benchmark
{
public:
    using Benchmark::Benchmark;

protected:
    vio::video_writer v;
    uint8_t* frame = {};

    void Initialize(CppBenchmark::Context& context) override
    {
        if (!v.open(video_path))
        {
            std::cout << "Unable to open " << video_path << std::endl;
            context.Cancel();
            return;
        }

        if (!v.is_opened())
        {
            std::cout << "vio::video_writer is not opened" << std::endl;
            context.Cancel();
            return;
        }
    }

    void Cleanup(CppBenchmark::Context& context) override
    {
        v.release();
    }

    void Run(CppBenchmark::Context& context) override
    {
        while (v.write(frame))
        {
        }
    }
};

const auto attempts = 1;
const auto operations = 1;

// BENCHMARK_CLASS(VideoWriterFixture_OpenCV,
// 	"VideoWriterFixture.OpenCV",
// 	Settings().Attempts(attempts).Operations(operations))

BENCHMARK_CLASS(VideoWriterFixture_video_io,
	"VideoWriterFixture.video_io.SW",
	Settings().Attempts(attempts).Operations(operations)

BENCHMARK_MAIN()
