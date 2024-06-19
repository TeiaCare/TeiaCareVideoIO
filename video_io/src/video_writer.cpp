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

#include <video_io/video_writer.hpp>
#include "logger.hpp"

extern "C"
{
#include <libavutil/log.h>
#include <libavutil/avassert.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavutil/dict.h>
#include <libavutil/channel_layout.h>
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
 
namespace tc::vio
{
video_writer::video_writer() noexcept
: _is_opened { false }
{
    init(); 
    av_log_set_level(0);
}

video_writer::~video_writer() noexcept
{
    release();
}

void video_writer::init()
{
    _is_opened = false;
    
    _stream = nullptr;
    _stream_duration = -1;
    _next_pts = 0;

    _codec_ctx = nullptr;
    _frame = nullptr;
    _tmp_frame = nullptr;
    _packet = nullptr;
    _sws_ctx = nullptr;
    _format_ctx = nullptr;
}

bool video_writer::open(const std::string& video_path, int width, int height, const int fps)
{
    if(width <= 0 || height <= 0 || fps <= 0)
    {
        log_error("open: invalid parameters:", "width:", width, "height:", height, "fps:", fps);
        return false;
    }

    std::lock_guard lock(_open_mutex);
    release();

    log_info("Opening video path:", video_path, "width:", width, "height:", height, "fps:", fps);

    if (auto r = avformat_alloc_output_context2(&_format_ctx, nullptr, nullptr, video_path.c_str()); r < 0) 
    {
        log_error("Could not deduce output format from file extension: using MPEG", vio::logger::get().err2str(r));
        
        if (auto r = avformat_alloc_output_context2(&_format_ctx, nullptr, "mpeg", video_path.c_str()); r < 0)
        {
            log_error("avformat_alloc_output_context2", vio::logger::get().err2str(r));
            return false;
        }
    }

    const AVCodec* codec = avcodec_find_encoder(_format_ctx->oformat->video_codec);
    if (!codec)
    {
        log_error("Could not find encoder for:", avcodec_get_name(_format_ctx->oformat->video_codec));
        return false;
    }

    if (_stream = avformat_new_stream(_format_ctx, nullptr); !_stream)
    {
        log_error("avformat_new_stream");
        return false;
    }
    _stream->id = _format_ctx->nb_streams-1;
    _stream->time_base = AVRational{ 1, fps }; // For fixed-fps content timebase should be 1/framerate
    _stream->r_frame_rate = AVRational{ fps, 1 };
    _stream->avg_frame_rate = AVRational{ fps, 1 };

    if (_codec_ctx = avcodec_alloc_context3(codec); !_codec_ctx)
    {
        log_error("avcodec_alloc_context3");
        return false;
    }

    _codec_ctx->codec_id = codec->id;
    _codec_ctx->bit_rate = 400000;
    _codec_ctx->width = width - (width % 2); // Keep sizes a multiple of 2
    _codec_ctx->height = height - (height % 2);
    _codec_ctx->time_base = _stream->time_base;
    _codec_ctx->gop_size = 12; // emit one intra frame every 12 frames at most
    _codec_ctx->pix_fmt = AVPixelFormat::AV_PIX_FMT_YUV420P;

    if (_codec_ctx->codec_id == AV_CODEC_ID_MPEG2VIDEO)
    {
        _codec_ctx->max_b_frames = 2;
    }

    if (_codec_ctx->codec_id == AV_CODEC_ID_MPEG1VIDEO)
    {
        _codec_ctx->mb_decision = 2;
    }

    if (_format_ctx->oformat->flags & AVFMT_GLOBALHEADER)
        _codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

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

    if (_frame = alloc_frame(static_cast<int>(_codec_ctx->pix_fmt), _codec_ctx->width, _codec_ctx->height); !_frame)
    {
        log_error("alloc_frame");
        return false;
    }

    if (_codec_ctx->pix_fmt != AV_PIX_FMT_YUV420P)
    {
        if (_tmp_frame = alloc_frame(static_cast<int>(AVPixelFormat::AV_PIX_FMT_YUV420P), _codec_ctx->width, _codec_ctx->height); !_frame)
        {
            log_error("alloc_frame");
            return false;
        }
    }

    if (auto r = avcodec_parameters_from_context(_stream->codecpar, _codec_ctx); r < 0)
    {
        log_error("avcodec_parameters_from_context", vio::logger::get().err2str(r));
        return false;
    }

    if (!(_format_ctx->oformat->flags & AVFMT_NOFILE)) 
    {
        if (auto r = avio_open(&_format_ctx->pb, video_path.c_str(), AVIO_FLAG_WRITE); r < 0) 
        {
            log_error("avio_open", vio::logger::get().err2str(r));
            return false;
        }
    }
 
    if (auto r = avformat_write_header(_format_ctx, nullptr); r < 0) 
    {
        log_error("avformat_write_header", vio::logger::get().err2str(r));
        return false;
    }

    _is_opened = true;
    log_info("Video Writer is opened correctly");
    return true;
}

bool video_writer::open(const std::string& video_path, int width, int height, const int fps, const int duration)
{
    if(duration <= 0)
    {
        log_error("open: invalid duration:", "duration:", duration);
        return false;
    }
    
    _stream_duration = duration;
    return open(video_path, width, height, fps);
}

bool video_writer::is_opened() const
{
    return _is_opened;
}

AVFrame* video_writer::alloc_frame(int pix_fmt, int width, int height)
{
    AVFrame* frame;

    if (frame = av_frame_alloc(); !frame)
    {
        log_error("av_frame_alloc");
        return nullptr;
    }

    frame->format = pix_fmt;
    frame->width  = width;
    frame->height = height;
    if (auto r = av_frame_get_buffer(frame, 0); r < 0)
    {
        log_error("av_frame_get_buffer", vio::logger::get().err2str(r));
        return nullptr;
    }
 
    return frame;
}

bool video_writer::encode(AVFrame* frame)
{
    if (auto r = avcodec_send_frame(_codec_ctx, frame); r < 0) 
    {
        log_error("avcodec_send_frame", vio::logger::get().err2str(r));
        return false;
    }

    while (true)
    {        
        if (auto r = avcodec_receive_packet(_codec_ctx, _packet); r < 0)
        {
            if(r == AVERROR(EAGAIN))
                return true;

            return false;
        }
 
        av_packet_rescale_ts(_packet, _codec_ctx->time_base, _stream->time_base);
        _packet->stream_index = _stream->index;
 
        // After the next line _packet is blank since av_interleaved_write_frame() takes ownership of its contents and resets it.
        // Unreferencing is not necessary, i.e. no need to call av_packet_unref(_packet).
        if (auto r = av_interleaved_write_frame(_format_ctx, _packet); r < 0)
        {
            log_info("av_interleaved_write_frame", vio::logger::get().err2str(r));
            return false;
        }
    }

    return true;
}

bool video_writer::convert(const uint8_t* data)
{
    if (_stream_duration > 0 && av_compare_ts(_next_pts, _codec_ctx->time_base, _stream_duration, AVRational{ 1, 1 }) >= 0)
    {
        log_info("End of stream. Flush remaining packets.");
        encode(nullptr);
        return false;
    }

    // when we pass a frame to the encoder, it may keep a reference to it internally; make sure we do not overwrite it here
    if (auto r = av_frame_make_writable(_frame); r < 0)
    {
        log_error("av_frame_make_writable", vio::logger::get().err2str(r));
        return false;
    }

    // /* Y */
    // for (int y = 0; y < _codec_ctx->height; y++)
    // {
    //     for (int x = 0; x < _codec_ctx->width; x++)
    //     {
    //         _frame->data[0][y * _frame->linesize[0] + x] = static_cast<uint8_t>(x);
    //     }
    // }

    // /* Cb and Cr */
    // for (int y = 0; y < _codec_ctx->height / 2; y++)
    // {
    //     for (int x = 0; x < _codec_ctx->width / 2; x++)
    //     {
    //         _frame->data[1][y * _frame->linesize[1] + x] = _next_pts;
    //         _frame->data[2][y * _frame->linesize[2] + x] = 0;
    //     }
    // }

    if (_codec_ctx->pix_fmt != AV_PIX_FMT_YUV420P)
    {
        // as we only generate a YUV420P picture, we must convert it to the codec pixel format if needed 
        if (!_sws_ctx) 
        {
            _sws_ctx = sws_getContext(
                _codec_ctx->width, _codec_ctx->height, AVPixelFormat::AV_PIX_FMT_YUV420P,
                _codec_ctx->width, _codec_ctx->height, _codec_ctx->pix_fmt, 
                SWS_BICUBIC, nullptr, nullptr, nullptr);
            
            if (!_sws_ctx)
            {
                log_error("Unable to initialize SwsContext");
                return false;
            }
        }

        if (auto r = av_image_fill_arrays(_tmp_frame->data, _tmp_frame->linesize, data, _codec_ctx->pix_fmt, _codec_ctx->width, _codec_ctx->height, 1); r < 0)
        {
            log_error("av_image_fill_arrays", vio::logger::get().err2str(r));
            return false;
        }

        sws_scale(_sws_ctx, (const uint8_t * const *)_tmp_frame->data, _tmp_frame->linesize, 0, _codec_ctx->height, _frame->data, _frame->linesize);
    }
    else 
    {
        if (auto r = av_image_fill_arrays(_frame->data, _frame->linesize, data, _codec_ctx->pix_fmt, _codec_ctx->width, _codec_ctx->height, 1); r < 0)
        {
            log_error("av_image_fill_arrays", vio::logger::get().err2str(r));
            return false;
        }
    }
    
    _frame->pts = _next_pts++; // Timestamp increment must be 1 for fixed-fps content
    
    return true;
}

bool video_writer::write(const uint8_t* data)
{
    if(!_is_opened)
        return false;
        
    if(!convert(data))
        return false;

    if(!encode(_frame))
        return false;

    return true;
}

bool video_writer::save()
{
    if(!_is_opened)
        return false;

    encode(nullptr);

    if(auto r = av_write_trailer(_format_ctx); r < 0) 
    {
        log_error("avformat_write_header", vio::logger::get().err2str(r));
        return false;
    }

    if (!(_format_ctx->oformat->flags & AVFMT_NOFILE))
    {
        if (auto r = avio_closep(&_format_ctx->pb); r < 0) 
        {
            log_error("avformat_write_header", vio::logger::get().err2str(r));
            return false;
        }
    }

    return release();
}

bool video_writer::release()
{
    if(!_is_opened)
        return false;
    
    log_info("Release video writer");

    if(_codec_ctx)
        avcodec_free_context(&_codec_ctx);

    if(_frame)
        av_frame_free(&_frame);
    
    if(_tmp_frame)
        av_frame_free(&_tmp_frame);
    
    if(_packet)
        av_packet_free(&_packet);

    if(_sws_ctx)
        sws_freeContext(_sws_ctx);

    if(_format_ctx)
        avformat_free_context(_format_ctx);

    init();
    return true;
}

bool video_writer::check(const std::string& video_path)
{
    AVFormatContext* fmt_ctx;

    if (fmt_ctx = avformat_alloc_context(); !fmt_ctx)
    {
        log_error("avformat_alloc_context");
        return false;
    }

    if (auto r = avformat_open_input(&fmt_ctx, video_path.c_str(), nullptr, nullptr); r < 0)
    {
        log_error("avformat_open_input", vio::logger::get().err2str(r));
        return false;
    }

    if (auto r = avformat_find_stream_info(fmt_ctx, nullptr); r < 0)
    {
        log_error("avformat_find_stream_info", vio::logger::get().err2str(r));
        return false;
    }

#if LIBAVCODEC_VERSION_MAJOR <= 58
    AVCodec* codec = nullptr;
#elif LIBAVCODEC_VERSION_MAJOR >= 59
    const AVCodec* codec = nullptr;
#endif

    int stream_index = av_find_best_stream(fmt_ctx, AVMediaType::AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0); 
    if (stream_index < 0)
    {
        log_error("av_find_best_stream", vio::logger::get().err2str(stream_index));
        return false;
    }
/*
    AVStream* stream = fmt_ctx->streams[stream_index];
    stream->nb_frames;
    stream->r_frame_rate;
    stream->avg_frame_rate;
    stream->duration;
    stream->id;

    stream->codecpar->width;
    stream->codecpar->height;
    stream->codecpar->codec_id;

    codec->name;
    codec->long_name;
    (AVPixelFormat)stream->codecpar->format;
*/

    if(fmt_ctx)
    {
        avformat_close_input(&_format_ctx);
        avformat_free_context(_format_ctx);
    }

    return true;
}

}
