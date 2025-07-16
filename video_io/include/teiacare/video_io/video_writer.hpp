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

#include <teiacare/video_io/ivideo_writer.hpp>

#include <chrono>
#include <memory>
#include <string>

struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVPacket;
struct AVFrame;
struct AVStream;
struct SwsContext;

namespace tc::vio
{
class video_writer : public tc::vio::ivideo_writer
{
public:
    explicit video_writer() noexcept;
    ~video_writer() noexcept override;

    bool open(const std::string& video_path, int width, int height, const int fps) override;
    bool open(const std::string& video_path, int width, int height, const int fps, const int duration) override;
    bool is_opened() const override;
    bool write(const uint8_t* data) override;
    void release() override;
    bool save() override;

    bool check(const std::string& video_path) override;

protected:
    bool convert(const uint8_t* data);
    bool encode(AVFrame* frame);
    AVFrame* alloc_frame(int pix_fmt, int width, int height);

private:
    AVFormatContext* _format_ctx = nullptr;
    AVCodecContext* _codec_ctx = nullptr;
    SwsContext* _sws_ctx = nullptr;
    AVPacket* _packet = nullptr;

    AVFrame* _src_frame = nullptr;
    AVFrame* _dst_frame = nullptr;

    AVStream* _stream = nullptr;
    int64_t _stream_duration = -1;
    int64_t _next_pts = 0;
};

}
