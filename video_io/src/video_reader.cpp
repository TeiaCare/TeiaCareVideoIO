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
#include "video_reader_hw.hpp"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/buffer.h>
#include <libavutil/dict.h>
#include <libavutil/frame.h>
#include <libavutil/hwcontext.h>
#include <libswscale/swscale.h>
    // #include <libavdevice/avdevice.h> // required for screen recording only
}

namespace tc::vio
{
video_reader::video_reader() noexcept
{
    init();
    av_log_set_level(0);
    // avdevice_register_all(); // required for screen recording only
}

video_reader::~video_reader() noexcept
{
    release();
}

void video_reader::init()
{
    log_info("Reset video capture");

    _format_ctx = nullptr;
    _codec_ctx = nullptr;
    _sws_ctx = nullptr;
    _packet = nullptr;

    _src_frame = nullptr;
    _tmp_frame = nullptr;
    _dst_frame = nullptr;

    _decode_support = decode_support::none;
    _options = nullptr;
    _stream_index = -1;
}

// void video_reader::set_log_callback(const log_callback_t& cb, const log_level& level) { vio::logger::get().set_log_callback(cb, level); }

bool video_reader::open(const char* video_path, decode_support decode_preference)
{
    release();

    log_info("Opening video path:", video_path);
    log_info("HW acceleration", (decode_preference == decode_support::HW ? "required" : "not required"));

    if (decode_preference == decode_support::HW)
    {
        _hw = std::make_unique<hw_acceleration>();
        _decode_support = _hw->init();
    }
    else
    {
        _decode_support = decode_support::SW;
    }

    if (_format_ctx = avformat_alloc_context(); !_format_ctx)
    {
        log_error("avformat_alloc_context");
        return false;
    }

    if (auto r = av_dict_set(&_options, "rtsp_transport", "tcp", 0); r < 0)
    {
        log_error("av_dict_set", vio::logger::get().err2str(r));
        return false;
    }

    return open_input(video_path, nullptr);
}

bool video_reader::open(const char* screen_name, screen_options screen_opt)
{
    (void)screen_opt;
    release();

    log_info("Opening video path:", screen_name);
    _decode_support = decode_support::SW;

    if (_format_ctx = avformat_alloc_context(); !_format_ctx)
    {
        log_error("avformat_alloc_context");
        return false;
    }

    const AVInputFormat* input_format = nullptr;
    screen_name = nullptr;

#if defined(__APPLE__)
    input_format = av_find_input_format("avfoundation");
    screen_name = "1";
#elif defined(_WIN32)
    // input_format = av_find_input_format("gdigrab");
    // screen_name = "desktop";
    input_format = av_find_input_format("dshow");
    screen_name = "video=USB2.0 VGA UVC WebCam";
#elif defined(__linux__)
    input_format = av_find_input_format("x11grab");
    // if(screen_name = std::getenv("DISPLAY"); !screen_name)
    //     screen_name = ":0";
#endif

    if (auto r = av_dict_set(&_options, "framerate", "30", 0); r < 0)
    {
        log_error("av_dict_set", vio::logger::get().err2str(r));
        return false;
    }

    if (auto r = av_dict_set(&_options, "preset", "ultrafast", 0); r < 0)
    {
        log_error("av_dict_set", vio::logger::get().err2str(r));
        return false;
    }

    if (auto r = av_dict_set(&_options, "video_size", "640x480", 0); r < 0)
    {
        log_error("av_dict_set", vio::logger::get().err2str(r));
        return false;
    }

    // TODO: check gdigrab on Windows. On Linux these offsets are useless.
    if (auto r = av_dict_set(&_options, "offset_x", "50", 0); r < 0)
    {
        log_error("av_dict_set", vio::logger::get().err2str(r));
        return false;
    }

    if (auto r = av_dict_set(&_options, "offset_y", "50", 0); r < 0)
    {
        log_error("av_dict_set", vio::logger::get().err2str(r));
        return false;
    }

    return open_input(screen_name, input_format);
}

bool video_reader::open_input(const char* input, const AVInputFormat* input_format)
{
    if (auto r = avformat_open_input(&_format_ctx, input, input_format, &_options); r < 0)
    {
        log_error("avformat_open_input", vio::logger::get().err2str(r));
        return false;
    }

    av_dict_free(&_options);

    if (auto r = avformat_find_stream_info(_format_ctx, nullptr); r < 0)
    {
        log_error("avformat_find_stream_info");
        return false;
    }

/* NOTE: this is a breaking change from ffmpeg v4.x to ffmpeg v5.x in function av_find_best_stream */
#if LIBAVCODEC_VERSION_MAJOR <= 58
    const AVCodec* codec = nullptr;
#elif LIBAVCODEC_VERSION_MAJOR >= 59
    const AVCodec* codec = nullptr;
#endif

    if (_stream_index = av_find_best_stream(_format_ctx, AVMediaType::AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0); _stream_index < 0)
    {
        log_error("av_find_best_stream", vio::logger::get().err2str(_stream_index));
        return false;
    }

    if (_codec_ctx = avcodec_alloc_context3(codec); !_codec_ctx)
    {
        log_error("avcodec_alloc_context3");
        return false;
    }
    _codec_ctx->thread_count = 1; // std::thread::hardware_concurrency();

    if (auto r = avcodec_parameters_to_context(_codec_ctx, _format_ctx->streams[_stream_index]->codecpar); r < 0)
    {
        log_error("avcodec_parameters_to_context", vio::logger::get().err2str(r));
        return false;
    }

    if (_decode_support == decode_support::HW)
    {
        // _codec_ctx->sw_pix_fmt = AV_PIX_FMT_NV12;
        _codec_ctx->hw_device_ctx = av_buffer_ref(_hw->hw_device_ctx);
        // _codec_ctx->hw_frames_ctx = _hw->get_frames_ctx(_codec_ctx->width, _codec_ctx->height);
    }

    if (auto r = avcodec_open2(_codec_ctx, codec, nullptr); r < 0)
    {
        log_error("avcodec_open2", vio::logger::get().err2str(r));
        return false;
    }

    if (_packet = av_packet_alloc(); !_packet)
    {
        log_error("av_packet_alloc");
        return false;
    }

    if (_src_frame = av_frame_alloc(); !_src_frame)
    {
        log_error("av_frame_alloc");
        return false;
    }

    if (_dst_frame = av_frame_alloc(); !_dst_frame)
    {
        log_error("av_frame_alloc");
        return false;
    }

    if (_decode_support == decode_support::HW)
    {
        // HW: Allocate one extra frame for HW decoding.
        if (_tmp_frame = av_frame_alloc(); !_tmp_frame)
        {
            log_error("av_frame_alloc");
            return false;
        }
    }
    else
    {
        // SW: No need to allocate any temporary frame, just make it point to _src_frame.
        _tmp_frame = _src_frame;
    }

    _dst_frame->format = AVPixelFormat::AV_PIX_FMT_BGR24;
    _dst_frame->width = _codec_ctx->width;
    _dst_frame->height = _codec_ctx->height;
    if (auto r = av_frame_get_buffer(_dst_frame, 0); r < 0)
    {
        log_error("av_frame_get_buffer", vio::logger::get().err2str(r));
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
        return reset_data(data, pts);
    }

    if (!decode())
    {
        return reset_data(data, pts);
    }

    if (!convert(data, pts))
    {
        return reset_data(data, pts);
    }

    // ++current_frame;
    return true;
}

void video_reader::release()
{
    log_info("Release video reader");

    if (_sws_ctx)
        sws_freeContext(_sws_ctx);

    if (_codec_ctx)
        avcodec_free_context(&_codec_ctx);

    if (_format_ctx)
    {
        avformat_close_input(&_format_ctx);
        avformat_free_context(_format_ctx);
    }

    if (_options)
        av_dict_free(&_options);

    if (_packet)
        av_packet_free(&_packet);

    if (_src_frame)
        av_frame_free(&_src_frame);

    if (_dst_frame)
        av_frame_free(&_dst_frame);

    if (_tmp_frame && _decode_support == decode_support::HW)
        av_frame_free(&_tmp_frame);

    init();

    if (_decode_support == decode_support::HW)
        _hw->release();
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

    _tmp_frame = _src_frame;
    return true;
}

bool video_reader::convert(uint8_t** data, double* pts)
{
    if (_decode_support == decode_support::HW)
    {
        if (!copy_hw_frame())
            return false;
    }

    if (!_sws_ctx)
    {
        _sws_ctx = sws_getCachedContext(_sws_ctx,
                                        _codec_ctx->width, _codec_ctx->height, (AVPixelFormat)_tmp_frame->format,
                                        _codec_ctx->width, _codec_ctx->height, AVPixelFormat::AV_PIX_FMT_RGB24,
                                        SWS_BILINEAR, nullptr, nullptr, nullptr);

        if (!_sws_ctx)
        {
            log_error("Unable to initialize SwsContext");
            return false;
        }
    }

    sws_scale(_sws_ctx, _tmp_frame->data, _tmp_frame->linesize, 0, _codec_ctx->height, _dst_frame->data, _dst_frame->linesize);

    *data = _dst_frame->data[0];

    if (pts)
    {
        const auto time_base = _format_ctx->streams[_stream_index]->time_base;
        *pts = _tmp_frame->best_effort_timestamp * static_cast<double>(time_base.num) / static_cast<double>(time_base.den);
    }

    return true;
}

bool video_reader::reset_data(uint8_t** data, double* pts) const
{
    if (data)
    {
        *data = nullptr;
    }

    if (pts)
    {
        *pts = -1.0;
    }

    return false;
}

bool video_reader::flush()
{
    AVPacket* packet = nullptr;
    if (auto r = avcodec_send_packet(_codec_ctx, packet); r < 0)
    {
        log_error("avcodec_send_packet", vio::logger::get().err2str(r));
        return false;
    }

    while (true)
    {
        if (auto r = avcodec_receive_frame(_codec_ctx, _src_frame); r < 0)
        {
            // the decoder has been fully flushed
            if (AVERROR_EOF == r)
                break;

            log_info("avcodec_receive_frame", vio::logger::get().err2str(r));
            return false;
        }
    }

    return true;
}

bool video_reader::copy_hw_frame()
{
    if (_src_frame->format == _hw->hw_pixel_format)
    {
        if (auto r = av_hwframe_transfer_data(_tmp_frame, _src_frame, 0); r < 0)
        {
            log_error("av_hwframe_transfer_data", vio::logger::get().err2str(r));
            return false;
        }

        if (auto r = av_frame_copy_props(_tmp_frame, _src_frame); r < 0)
        {
            log_error("av_frame_copy_props", vio::logger::get().err2str(r));
            return false;
        }
    }
    else
    {
        _tmp_frame = _src_frame;
    }

    return true;
}

}
