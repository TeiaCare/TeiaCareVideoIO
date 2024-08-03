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

#include <teiacare/video_io/video_info.hpp>

#include "logger.hpp"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

namespace tc::vio
{
video_info::video_info() noexcept
{
}

video_info::~video_info() noexcept
{
}

std::optional<video_metadata> video_info::get_video_metadata(const std::string& video_path)
{
    AVFormatContext* fmt_ctx;

    if (fmt_ctx = avformat_alloc_context(); !fmt_ctx)
    {
        log_error("avformat_alloc_context");
        reset(fmt_ctx);
        return std::nullopt;
    }

    if (auto r = avformat_open_input(&fmt_ctx, video_path.c_str(), nullptr, nullptr); r < 0)
    {
        log_error("avformat_open_input", vio::logger::get().err2str(r));
        reset(fmt_ctx);
        return std::nullopt;
    }

    if (auto r = avformat_find_stream_info(fmt_ctx, nullptr); r < 0)
    {
        log_error("avformat_find_stream_info", vio::logger::get().err2str(r));
        reset(fmt_ctx);
        return std::nullopt;
    }

    const AVCodec* codec = nullptr;
    int stream_index = av_find_best_stream(fmt_ctx, AVMediaType::AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    if (stream_index < 0)
    {
        log_error("av_find_best_stream", vio::logger::get().err2str(stream_index));
        reset(fmt_ctx);
        return std::nullopt;
    }

    AVStream* stream = fmt_ctx->streams[stream_index];
    const auto m = video_metadata{
        stream->id,
        stream->codecpar->width,
        stream->codecpar->height,
        static_cast<int>(stream->nb_frames),
        av_q2d(stream->r_frame_rate),
        av_q2d(stream->avg_frame_rate),
        av_q2d(stream->time_base) * stream->duration,
        codec->name,
    };

    reset(fmt_ctx);
    return m;
}

void video_info::reset(AVFormatContext* fmt_ctx)
{
    if (fmt_ctx)
    {
        avformat_close_input(&fmt_ctx);
        avformat_free_context(fmt_ctx);
    }
}

}
