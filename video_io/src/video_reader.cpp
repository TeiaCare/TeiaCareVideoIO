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

#include <teiacare/video_io/video_reader.hpp>

#include "logger.hpp"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/buffer.h>
#include <libavutil/dict.h>
#include <libavutil/frame.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}

namespace tc::vio
{
video_reader::video_reader() noexcept
{
    av_log_set_level(0);
}

video_reader::~video_reader() noexcept
{
    release();
}

void video_reader::release()
{
    log_info("Release video reader");

    if (_sws_ctx)
    {
        sws_freeContext(_sws_ctx);
        _sws_ctx = nullptr;
    }

    if (_codec_ctx)
    {
        avcodec_free_context(&_codec_ctx);
        _codec_ctx = nullptr;
    }

    if (_format_ctx)
    {
        avformat_close_input(&_format_ctx);
        avformat_free_context(_format_ctx);
        _format_ctx = nullptr;
    }

    if (_options)
    {
        av_dict_free(&_options);
        _options = nullptr;
    }

    if (_packet)
    {
        av_packet_free(&_packet);
        _packet = nullptr;
    }

    if (_src_frame)
    {
        av_frame_free(&_src_frame);
        _src_frame = nullptr;
    }

    if (_dst_frame)
    {
        av_frame_free(&_dst_frame);
        _dst_frame = nullptr;
    }

    _stream_index = -1;
}

bool video_reader::open(const std::string& video_path)
{
    release();

    log_info("Opening video path:", video_path);

    if (_format_ctx = avformat_alloc_context(); !_format_ctx)
    {
        log_error("avformat_alloc_context");
        release();
        return false;
    }

    if (auto r = av_dict_set(&_options, "rtsp_transport", "tcp", 0); r < 0)
    {
        log_error("av_dict_set", vio::logger::get().err2str(r));
        release();
        return false;
    }

    av_dict_set(&_options, "rtsp_flags", "prefer_tcp", 0);
    av_dict_set(&_options, "stimeout", "5000000", 0); // 5 second timeout

    if (auto r = avformat_open_input(&_format_ctx, video_path.c_str(), nullptr, &_options); r < 0)
    {
        log_error("avformat_open_input", vio::logger::get().err2str(r));
        release();
        return false;
    }

    av_dict_free(&_options);

    if (auto r = avformat_find_stream_info(_format_ctx, nullptr); r < 0)
    {
        log_error("avformat_find_stream_info");
        release();
        return false;
    }

    const AVCodec* codec = nullptr;
    if (_stream_index = av_find_best_stream(_format_ctx, AVMediaType::AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0); _stream_index < 0)
    {
        log_error("av_find_best_stream", vio::logger::get().err2str(_stream_index));
        release();
        return false;
    }

    if (_codec_ctx = avcodec_alloc_context3(codec); !_codec_ctx)
    {
        log_error("avcodec_alloc_context3");
        release();
        return false;
    }
    _codec_ctx->thread_count = 1; // std::thread::hardware_concurrency();

    if (auto r = avcodec_parameters_to_context(_codec_ctx, _format_ctx->streams[_stream_index]->codecpar); r < 0)
    {
        log_error("avcodec_parameters_to_context", vio::logger::get().err2str(r));
        release();
        return false;
    }

    if (auto r = avcodec_open2(_codec_ctx, codec, nullptr); r < 0)
    {
        log_error("avcodec_open2", vio::logger::get().err2str(r));
        release();
        return false;
    }

    if (_packet = av_packet_alloc(); !_packet)
    {
        log_error("av_packet_alloc");
        release();
        return false;
    }

    if (_src_frame = av_frame_alloc(); !_src_frame)
    {
        log_error("av_frame_alloc");
        release();
        return false;
    }

    if (_dst_frame = av_frame_alloc(); !_dst_frame)
    {
        log_error("av_frame_alloc");
        release();
        return false;
    }

    _dst_frame->format = AVPixelFormat::AV_PIX_FMT_RGB24;
    _dst_frame->width = _codec_ctx->width;
    _dst_frame->height = _codec_ctx->height;
    if (auto r = av_frame_get_buffer(_dst_frame, 0); r < 0)
    {
        log_error("av_frame_get_buffer", vio::logger::get().err2str(r));
        release();
        return false;
    }

    log_info("Video Reader is opened correctly");
    return true;
}

bool video_reader::is_opened() const
{
    return _codec_ctx != nullptr && _format_ctx != nullptr;
}

bool video_reader::read(uint8_t** data, double* pts)
{
    if (!is_opened())
    {
        reset_data(data, pts);
        return false;
    }

    if (!decode())
    {
        reset_data(data, pts);
        return false;
    }

    if (!convert(data, pts))
    {
        reset_data(data, pts);
        return false;
    }

    // ++current_frame;
    return true;
}

auto video_reader::get_frame_count() const -> std::optional<int>
{
    if (!is_opened())
    {
        log_error("Frame count not available. Video path must be opened first.");
        return std::nullopt;
    }

    auto nb_frames = _format_ctx->streams[_stream_index]->nb_frames;
    if (!nb_frames)
    {
        double duration_sec = static_cast<double>(_format_ctx->duration) / static_cast<double>(AV_TIME_BASE);
        auto fps = get_fps();
        nb_frames = static_cast<int64_t>(std::floor(duration_sec * fps.value() + 0.5));
    }
    if (nb_frames)
        return std::make_optional(static_cast<int>(nb_frames));

    return std::nullopt;
}

auto video_reader::get_duration() const -> std::optional<std::chrono::steady_clock::duration>
{
    if (!is_opened())
    {
        log_error("Duration not available. Video path must be opened first.");
        return std::nullopt;
    }

    auto duration = std::chrono::duration<int64_t, std::ratio<1, AV_TIME_BASE>>(_format_ctx->duration);
    return std::make_optional(duration);
}

auto video_reader::get_frame_size() const -> std::optional<std::tuple<int, int>>
{
    if (!is_opened())
    {
        log_error("Frame size not available. Video path must be opened first.");
        return std::nullopt;
    }

    auto size = std::make_tuple(_codec_ctx->width, _codec_ctx->height);
    return std::make_optional(size);
}

auto video_reader::get_frame_size_in_bytes() const -> std::optional<int>
{
    if (!is_opened())
    {
        log_error("Frame size in bytes not available. Video path must be opened first.");
        return std::nullopt;
    }

    auto bytes = _codec_ctx->width * _codec_ctx->height * 3;
    return std::make_optional(bytes);
}

auto video_reader::get_fps() const -> std::optional<double>
{
    if (!is_opened())
    {
        log_error("FPS not available. Video path must be opened first.");
        return std::nullopt;
    }

    auto frame_rate = _format_ctx->streams[_stream_index]->avg_frame_rate;
    if (frame_rate.num <= 0 || frame_rate.den <= 0)
    {
        log_info("Unable to convert FPS.");
        return std::nullopt;
    }

    auto fps = static_cast<double>(frame_rate.num) / static_cast<double>(frame_rate.den);
    return std::make_optional(fps);
}

bool video_reader::decode()
{
    while (true)
    {
        int ret = 0;
        av_packet_unref(_packet);

        ret = av_read_frame(_format_ctx, _packet);
        if (ret == AVERROR(EAGAIN))
            continue;

        if (ret == AVERROR_EOF)
        {
            // Create a dummy packet in order to flush cached frames from the decoder
            _packet->data = nullptr;
            _packet->size = 0;
            _packet->stream_index = _stream_index;
        }

        if (_packet->stream_index != _stream_index)
            continue;

        avcodec_send_packet(_codec_ctx, _packet);

        ret = avcodec_receive_frame(_codec_ctx, _src_frame);
        if (ret == AVERROR(EAGAIN))
            continue;

        if (ret == AVERROR_EOF)
        {
            av_packet_unref(_packet);
            return false;
        }

        break;
    }

    return true;
}

bool video_reader::convert(uint8_t** data, double* pts)
{
    if (!_sws_ctx)
    {
        _sws_ctx = sws_getCachedContext(_sws_ctx,
                                        _codec_ctx->width, _codec_ctx->height, (AVPixelFormat)_src_frame->format,
                                        _codec_ctx->width, _codec_ctx->height, AVPixelFormat::AV_PIX_FMT_RGB24,
                                        SWS_BILINEAR, nullptr, nullptr, nullptr);

        if (!_sws_ctx)
        {
            log_error("Unable to initialize SwsContext");
            release();
            return false;
        }
    }

    // Scale directly into user's buffer if provided, otherwise use internal buffer
    if (data && *data)
    {
        uint8_t* dst_data[8] = {*data, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
        int dst_linesize[8] = {_codec_ctx->width * 3, 0, 0, 0, 0, 0, 0, 0}; // RGB24 = 3 bytes per pixel
        sws_scale(_sws_ctx, _src_frame->data, _src_frame->linesize, 0, _codec_ctx->height, dst_data, dst_linesize);
    }
    else
    {
        sws_scale(_sws_ctx, _src_frame->data, _src_frame->linesize, 0, _codec_ctx->height, _dst_frame->data, _dst_frame->linesize);
    }

    if (pts)
    {
        const auto time_base = _format_ctx->streams[_stream_index]->time_base;
        *pts = _src_frame->best_effort_timestamp * static_cast<double>(time_base.num) / static_cast<double>(time_base.den);
    }

    return true;
}

void video_reader::reset_data(uint8_t**, double* pts) const
{
    // Don't modify the user's data pointer - just reset pts
    // Setting *data = nullptr could cause issues if the caller expects the original pointer

    if (pts)
    {
        *pts = -1.0;
    }
}

}
