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

#include <teiacare/video_io/ivideo_reader.hpp>

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
struct AVInputFormat;
struct AVDictionary;

namespace tc::vio
{
class video_reader : public tc::vio::ivideo_reader
{
public:
    explicit video_reader() noexcept;
    ~video_reader() noexcept override;

    bool open(const std::string& video_path) override;
    bool is_opened() const override;
    bool read(uint8_t** data, double* pts = nullptr) override;
    void release() override;

    auto get_frame_count() const -> std::optional<int> override;
    auto get_duration() const -> std::optional<std::chrono::steady_clock::duration> override;
    auto get_frame_size() const -> std::optional<std::tuple<int, int>> override;
    auto get_frame_size_in_bytes() const -> std::optional<int> override;
    auto get_fps() const -> std::optional<double> override;

protected:
    bool decode();
    bool convert(uint8_t** data, double* pts);
    void reset_data(uint8_t** data, double* pts) const;

private:
    AVFormatContext* _format_ctx = nullptr;
    AVCodecContext* _codec_ctx = nullptr;
    SwsContext* _sws_ctx = nullptr;
    AVPacket* _packet = nullptr;

    AVFrame* _src_frame = nullptr;
    AVFrame* _dst_frame = nullptr;

    AVDictionary* _options = nullptr;
    int _stream_index = -1;
};

}
