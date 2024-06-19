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

/**
 * banchmark: 	video_reader_opencv
 * author:		Stefano Lusardi
 * date:		Aug 2022
 * description:	Comparison between OpenCV::VideoCapture and video_io::video_reader. 
*/

#include <iostream>
#include <teiacare/video_io/video_reader.hpp>
#include <opencv2/videoio.hpp>
#include <benchmark/cppbenchmark.h>


const auto video_path = "../../../../tests/data/v.mp4";
// const auto video_path = "../../../../tests/data/testsrc_30sec_30fps.mkv";

class VideoReaderFixture_OpenCV : public CppBenchmark::Benchmark
{
public:
    using Benchmark::Benchmark;

protected:
    cv::VideoCapture v;
	cv::Mat frame;

    void Initialize(CppBenchmark::Context& context) override
	{
		if(!v.open(video_path))
		{
			std::cout << "Unable to open " << video_path << std::endl;
			context.Cancel();
			return;
		}

		if(!v.isOpened())
		{
			std::cout << "cv::VideoCapture is not opened" << std::endl;
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
		while(v.read(frame))
		{
		}
	}
};

class VideoReaderFixture_video_io : public CppBenchmark::Benchmark
{
public:
    using Benchmark::Benchmark;

protected:
	vio::video_reader v;
	uint8_t* frame = {};

    void Initialize(CppBenchmark::Context& context) override
	{
		int param = context.x();
		const vio::decode_support decode_support = static_cast<vio::decode_support>(param);
		
		if(!v.open(video_path, decode_support))
		{
			std::cout << "Unable to open " << video_path << std::endl;
			context.Cancel();
			return;
		}

		if(!v.is_opened())
		{
			std::cout << "vio::video_reader is not opened" << std::endl;
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
		while(v.read(&frame))
		{
		}
	}
};

const auto attempts = 1;
const auto operations = 1;

// BENCHMARK_CLASS(VideoReaderFixture_OpenCV,
// 	"VideoCaptureFixture.OpenCV",
// 	Settings().Attempts(attempts).Operations(operations))

BENCHMARK_CLASS(VideoReaderFixture_video_io,
	"VideoCaptureFixture.video_io.SW",
	Settings().Attempts(attempts).Operations(operations).Param(static_cast<int>(vio::decode_support::SW)))

BENCHMARK_CLASS(VideoReaderFixture_video_io,
	"VideoCaptureFixture.video_io.HW",
	Settings().Attempts(attempts).Operations(operations).Param(static_cast<int>(vio::decode_support::HW)))
	
BENCHMARK_MAIN()
