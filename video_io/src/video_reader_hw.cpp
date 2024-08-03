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

#include "video_reader_hw.hpp"

#include "logger.hpp"

extern "C"
{
#include <libavutil/hwcontext.h>
}

/*
https://ffmpeg.org/doxygen/trunk/hw_decode_8c-example.html
https://ffmpeg.org/doxygen/trunk/ffmpeg__hw_8c_source.html#l00317
*/

namespace tc::vio
{
video_reader::hw_acceleration::hw_acceleration()
{
    reset();
}

video_reader::hw_acceleration::~hw_acceleration()
{
    release();
}

decode_support video_reader::hw_acceleration::init()
{
    auto get_hw_video_device_type = []() -> const char* {
        AVHWDeviceType hw_type = AV_HWDEVICE_TYPE_NONE;

        std::vector<AVHWDeviceType> supported_hw_types;
        log_info("Available devices for HW Acceleration: ");
        while ((hw_type = av_hwdevice_iterate_types(hw_type)) != AV_HWDEVICE_TYPE_NONE)
        {
            log_info(av_hwdevice_get_type_name(hw_type));
            supported_hw_types.push_back(hw_type);
        }

#if defined(_WIN32)
        return "dxva2"; // "d3d11va";
#elif defined(__linux__)
        return "vaapi"; // "vdpau";
#else
        return "";
#endif
    };

    const char* device_type_str = get_hw_video_device_type();
    const auto hw_type = av_hwdevice_find_type_by_name(device_type_str);
    if (hw_type == AV_HWDEVICE_TYPE_NONE)
    {
        log_info("HW decoder not available. Fall back to SW decoding");
        return decode_support::SW;
    }

    auto get_hw_pixel_format = [](const AVHWDeviceType type) -> int {
        switch (type)
        {
        case AV_HWDEVICE_TYPE_VAAPI:
            return AV_PIX_FMT_VAAPI;
        case AV_HWDEVICE_TYPE_DXVA2:
            return AV_PIX_FMT_DXVA2_VLD;
        case AV_HWDEVICE_TYPE_D3D11VA:
            return AV_PIX_FMT_D3D11;
        case AV_HWDEVICE_TYPE_VDPAU:
            return AV_PIX_FMT_VDPAU;
        case AV_HWDEVICE_TYPE_VIDEOTOOLBOX:
            return AV_PIX_FMT_VIDEOTOOLBOX;
        default:
            return AV_PIX_FMT_NONE;
        }
    };

    hw_pixel_format = get_hw_pixel_format(hw_type);
    if (auto r = av_hwdevice_ctx_create(&hw_device_ctx, hw_type, nullptr, nullptr, 0); r < 0)
    {
        log_error("av_hwdevice_ctx_create", vio::logger::get().err2str(r));
        log_info("HW decoder not available. Fall back to SW decoding");
        return decode_support::SW;
    }

    // if (hw_frame = av_frame_alloc(); !hw_frame)
    // {
    //     _logger->log(log_level::error, "av_frame_alloc");
    //     return decode_support::SW;
    // }

    // av_hwdevice_ctx_create_derived

    log_info("HW decoding enabled using", "xxx");
    return decode_support::HW;
}

AVBufferRef* video_reader::hw_acceleration::get_frames_ctx(int w, int h)
{
    hw_frames_ctx = av_hwframe_ctx_alloc(hw_device_ctx);
    AVHWFramesContext* frames_ctx = (AVHWFramesContext*)(hw_frames_ctx->data);
    frames_ctx->width = w;
    frames_ctx->height = h;
    frames_ctx->format = (AVPixelFormat)hw_pixel_format;
    frames_ctx->sw_format = AV_PIX_FMT_NV12;
    frames_ctx->initial_pool_size = 32;
    if (av_hwframe_ctx_init(hw_frames_ctx) < 0)
    {
        log_error("Error initilizing HW frame context");
    }

    return hw_frames_ctx;
}

void video_reader::hw_acceleration::release()
{
    if (hw_frames_ctx)
        av_buffer_unref(&hw_frames_ctx);

    if (hw_device_ctx)
        av_buffer_unref(&hw_device_ctx);

    reset();
}

void video_reader::hw_acceleration::reset()
{
    hw_device_ctx = nullptr;
    hw_frames_ctx = nullptr;
    hw_pixel_format = -1;
}

}
