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

#include <chrono>
#include <memory>
#include <optional>
#include <string>

struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVPacket;
struct AVFrame;
struct SwsContext;
struct AVStream;

namespace tc::vio
{
class video_writer
{
public:
    explicit video_writer() noexcept;
    ~video_writer() noexcept;

    // using log_callback_t = std::function<void(const std::string&)>;
    // void set_log_callback(const log_callback_t& cb, const log_level& level = log_level::all);

    bool open(const std::string& video_path, int width, int height, const int fps);
    bool open(const std::string& video_path, int width, int height, const int fps, const int duration);
    bool is_opened() const;
    bool write(const uint8_t* data);
    bool release();
    bool save();

    bool check(const std::string& video_path);

protected:
    void init();
    bool convert(const uint8_t* data);
    bool encode(AVFrame* frame);
    AVFrame* alloc_frame(int pix_fmt, int width, int height);

private:
    AVFormatContext* _format_ctx;
    AVCodecContext* _codec_ctx;
    SwsContext* _sws_ctx;
    AVPacket* _packet;

    AVFrame* _frame;
    AVFrame* _tmp_frame;

    AVStream* _stream;
    int64_t _stream_duration;
    int64_t _next_pts;
};

}
