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

#include "logger.hpp"
#include <video_io/video_reader.hpp>

extern "C"
{
#include <libavutil/hwcontext.h>
}

namespace tc::vio
{
struct video_reader::hw_acceleration
{
    explicit hw_acceleration();
    ~hw_acceleration();

    decode_support init();
    AVBufferRef* get_frames_ctx(int w, int h);
    void release();
    void reset();

    AVBufferRef* hw_device_ctx;
    AVBufferRef* hw_frames_ctx;
    int hw_pixel_format;
};

}
