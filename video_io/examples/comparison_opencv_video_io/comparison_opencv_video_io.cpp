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
 * example: 	comparison_opencv_video_io
 * author:		Stefano Lusardi
 * date:		Jun 2021
 * description:	Comparison between OpenCV::VideoCapture and vio::video_reader.
 * 				Public APIs are very similar, while private ones are simplified quite a lot.
 * 				The frames are using OpenCV::imshow in both examples.
 * 				Note that sleep time between consecutive frames is not accurate here,
 * 				see any video_player_xxx example for a more accurate playback.
 */

#include <teiacare/video_io/video_reader.hpp>

#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

void run_opencv(const char* video_path)
{
    cv::VideoCapture v;
    if (!v.open(video_path))
    {
        std::cout << "Unable to open " << video_path << std::endl;
        return;
    }

    const auto w = v.get(cv::CAP_PROP_FRAME_WIDTH);
    const auto h = v.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::Mat frame(w, h, CV_8UC3);

    const char* name = "Example";
    cv::namedWindow(name);

    while (v.read(frame))
    {
        cv::imshow(name, frame);
        cv::waitKey(1);
    }

    v.release();
    cv::destroyWindow(name);
}

void run_video_io(const char* video_path)
{
    tc::vio::video_reader v;
    if (!v.open(video_path))
    {
        std::cout << "Unable to open " << video_path << std::endl;
        return;
    }

    const auto size = v.get_frame_size();
    const auto [w, h] = size.value();
    cv::Mat frame(h, w, CV_8UC3);

    const char* name = "Example";
    cv::namedWindow(name);

    while (v.read(&frame.data))
    {
        cv::imshow(name, frame);
        cv::waitKey(1);
    }

    v.release();
    cv::destroyWindow(name);
}

int main(int argc, char** argv)
{
    const auto video_path = "rtsp://wowzaec2demo.streamlock.net/vod/mp4:BigBuckBunny_115k.mp4";
    // const auto video_path = "../../../../tests/data/testsrc_120sec_30fps.mpg";

    run_opencv(video_path);
    run_video_io(video_path);

    return 0;
}
