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
 * example: 	video_reader_simple_decode
 * author:		Stefano Lusardi
 * date:		Jun 2021
 * description:	The simplest example to show video_reader API usage.
 * 				Single threaded: main thread is responsible for decode subsequent frames until the video ends.
 * 				In case an rtsp stream is provided the main loop will never return.
 */

#include <teiacare/video_io/video_reader.hpp>

#include "utils/video_data_path.hpp"
#include <filesystem>
#include <iostream>

int main(int argc, char** argv)
{
    // Create video_reader object
    tc::vio::video_reader v;

    // Locate video file to be opened
    std::filesystem::path default_video_path = std::filesystem::path(tc::vio::examples::utils::video_data_path) / "video_10sec_2fps_HD.mp4";
    auto video_path = default_video_path.string();
    if (argc > 1)
        video_path = argv[1];

    // Open video (local file or RTSP stream)
    if (!v.open(video_path.c_str())) //, vio::decode_support::HW);
    {
        std::cout << "Unable to open input video: " << video_path << std::endl;
        return EXIT_FAILURE;
    }

    // Retrieve video info
    std::cout << "Opened input video: " << video_path << std::endl;
    const auto fps = v.get_fps();
    const auto size = v.get_frame_size();
    const auto [width, height] = size.value();
    std::cout << "FPS: " << fps.value() << "\n"
              << "Frame Size: [" << width << ", " << height << "]"
              << std::endl;

    // Read video frame by frame
    std::cout << "Start decoding frames" << std::endl;
    size_t num_decoded_frames = 0;
    uint8_t* frame_data = {};
    while (v.read(&frame_data))
    {
        ++num_decoded_frames;
        std::cout << "Frame: " << num_decoded_frames << std::endl;
        // Use frame_data array
        // ...
    }

    std::cout << "Decoded Frames: " << num_decoded_frames << std::endl;

    // Release and cleanup video_reader
    v.release();

    return EXIT_SUCCESS;
}

// -formats            show available formats
// -muxers             show available muxers
// -demuxers           show available demuxers
// -devices            show available devices
// -codecs             show available codecs
// -decoders           show available decoders
// -encoders           show available encoders
// -bsfs               show available bit stream filters
// -protocols          show available protocols
// -filters            show available filters
