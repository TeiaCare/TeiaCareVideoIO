#include <teiacare/video_io/video_reader.hpp>
#include <teiacare/video_io/video_writer.hpp>

#include "utils/video_data_path.hpp"
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
    // Create video_reader object
    tc::vio::video_reader v;

    // Locate video file to be opened
    std::filesystem::path default_video_path = "rtsp://videoproxy.app.teiacare.com:8555/5/main"; // std::filesystem::path(tc::vio::examples::utils::video_data_path) / "video_2sec_2fps_HD.mp4";
    auto video_path = default_video_path.string();
    if (argc > 1)
        video_path = argv[1];

    // Open video (local file or RTSP stream)
    if (!v.open(video_path.c_str()))
    {
        std::cout << "Unable to open input video: " << video_path << std::endl;
        return -1;
    }

    // Retrieve video info
    std::cout << "Opened input video: " << video_path << std::endl;
    const auto fps = v.get_fps();
    const auto size = v.get_frame_size();
    const auto [width, height] = size.value();
    std::cout << "FPS: " << fps.value() << "\n"
              << "Frame Size: [" << width << ", " << height << "]"
              << std::endl;

    // Initialize video writer for output
    tc::vio::video_writer writer;
    const std::string output_video_path = "output.mp4";

    if (!writer.open(output_video_path.c_str(), width, height, fps.value()))
    {
        std::cerr << "Failed to open video writer: " << output_video_path << std::endl;
        return -1;
    }
    std::cout << "Created output video writer: " << output_video_path << std::endl;

    // Read video frame by frame
    std::cout << "Start decoding frames" << std::endl;
    size_t num_decoded_frames = 0;
    std::vector<uint8_t> data_buffer(width * height * 3);
    uint8_t* frame_data = data_buffer.data();

    while (v.read(&frame_data)) // Pass address of the pointer
    {
        // Remove if you want full video decoding
        if (num_decoded_frames == 100)
            break;

        // Get the actual frame size in bytes
        const auto frame_bytes = v.get_frame_size_in_bytes();
        if (!frame_bytes.has_value())
        {
            std::cerr << "Unable to get frame size in bytes for frame " << num_decoded_frames << std::endl;
            break;
        }

        ++num_decoded_frames;
        std::cout << "Frame: " << num_decoded_frames << std::endl;

        // Write frame to output video (TeiaCore video writer expects BGR format)
        if (!writer.write(frame_data))
        {
            std::cerr << "Failed to write frame " << num_decoded_frames << " to video" << std::endl;
            break; // Stop processing on write failure
        }

        // Progress indicator
        if (num_decoded_frames % 100 == 0)
        {
            std::cout << "Processed " << num_decoded_frames << " frames..." << std::endl;
        }
    }

    // Finalize and save the output video
    writer.save();

    // Optional: Check the output video
    writer.check(output_video_path);

    std::cout << "Total decoded frames: " << num_decoded_frames << std::endl;
    // std::cout << "Sample frames saved to: " << output_frame_folder << std::endl;
    std::cout << "Output video saved to: " << output_video_path << std::endl;
    std::cout << " - Input FPS: " << fps.value() << std::endl;
    std::cout << " - Duration: " << static_cast<float>(num_decoded_frames) / fps.value() << " seconds" << std::endl;

    // Release and cleanup video_reader
    v.release();
    return 0;
}
