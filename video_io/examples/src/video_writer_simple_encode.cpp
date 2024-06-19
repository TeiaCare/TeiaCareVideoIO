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
 * example: 	video_writer_simple_encode
 * author:		Stefano Lusardi
 * date:		Sep 2022
 * description:	The simplest example to show video_writer API usage. 
 * 				Single threaded: main thread is responsible for encoding subsequent frames for a given duration.
*/

#include <iostream>
#include <algorithm>
#include <array>
#include <video_io/video_writer.hpp>

// void log_callback(const std::string& str) { std::cout << "[::video_writer::] " << str << std::endl; }

/*
static void get_frame_data(AVFrame *pict, int frame_index, int width, int height)
{
    double increment = (frame_index ) / (STREAM_DURATION * STREAM_FRAME_RATE);

    // Y plane
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            pict->data[0][y * pict->linesize[0] + x] = static_cast<uint8_t>(255 * increment);
        }
    }
 
    // Cb and Cr planes
    for (int y = 0; y < height / 2; y++)
    {
        for (int x = 0; x < width / 2; x++)
        {
            pict->data[1][y * pict->linesize[1] + x] = 127;
            pict->data[2][y * pict->linesize[2] + x] = 0;
        }
    }
}
*/

void record_n_frames(const char* format)
{
    tc::vio::video_writer v;

    const int num_frames_to_write = 300;
	const auto video_path = std::string("out_" + std::to_string(num_frames_to_write) + "_frames" + format);
	const auto fps = 30;
	const auto width = 640;
	const auto height = 480;

	v.open(video_path, width, height, fps);

    const auto frame_size = width * height * 3;
    std::array<uint8_t, frame_size> frame_data = { };

	size_t num_written_frames = 0;
	while(num_written_frames < num_frames_to_write)
	{
        frame_data.fill(static_cast<uint8_t>(num_written_frames));
        if (!v.write(frame_data.data()))
            break;

        num_written_frames++;
	}

    v.save();

    v.check(video_path);

    const float num_written_seconds = static_cast<float>(num_frames_to_write) / fps;
	std::cout << video_path << "\n"
        << " - Frames: " << num_frames_to_write << "\n"
        << " - FPS: " << fps << "\n"
        << " - Seconds = frames / fps = " << num_written_seconds  << "\n"
        << std::endl;
}

void record_n_seconds(const char* format)
{
    tc::vio::video_writer v;

    const int num_seconds_to_write = 10;
	const auto video_path = std::string("out_" + std::to_string(num_seconds_to_write) + "_seconds" + format);
	const auto fps = 30;
	const auto width = 640;
	const auto height = 480;

	v.open(video_path, width, height, fps, num_seconds_to_write);

    const auto frame_size = width * height * 3;
    std::array<uint8_t, frame_size> frame_data = { };

	size_t num_written_frames = 0;
	while(true)
	{
        frame_data.fill(static_cast<uint8_t>(num_written_frames));
        if (!v.write(frame_data.data()))
            break;
        
        num_written_frames++;
	}

    v.save();
	
    std::cout << video_path << "\n"
        << " - Seconds: " << num_seconds_to_write << "\n"
        << " - FPS: " << fps << "\n"
        << " - Frames = seconds * fps = " << num_written_frames << "\n"
        << std::endl;
}

int main(int argc, char** argv)
{
    std::vector<const char *> formats =
    {
        ".mp4",
        // ".mpg",
        // ".avi",
    };

    for (auto format : formats)
    {
        record_n_frames(format);
        record_n_seconds(format);
    }

	return 0;
}

/*
Check number of video frames:
ffprobe -v 0 -select_streams v:0 -of csv=p=0 -show_entries stream=nb_read_frames -count_frames out.mp4

Check FPS:
ffprobe -v 0 -select_streams v:0 -of csv=p=0 -show_entries stream=r_frame_rate out.mp4

Check FPS:
ffprobe -v 0 -select_streams v:0 -of csv=p=0 -show_entries stream=avg_frame_rate out.mp4

Duration:
ffprobe -v 0 -select_streams v:0 -of csv=p=0 -show_entries stream=duration out.mp4


ALL:
ffprobe -v 0 -show_entries stream=nb_read_frames -count_frames -show_entries stream=r_frame_rate -show_entries stream=avg_frame_rate -show_entries stream=duration out.mp4

*/