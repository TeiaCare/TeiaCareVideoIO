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

#include <optional>
#include <string>

struct AVFormatContext;

namespace tc::vio
{
struct video_metadata
{
    int id;
    int width;
    int height;
    int nb_frames;
    double r_frame_rate;
    double avg_frame_rate;
    double duration;
    const char* codec_name;
};

class video_info
{
public:
    explicit video_info() noexcept;
    ~video_info() noexcept;

    std::optional<video_metadata> get_video_metadata(const std::string& video_path);

private:
    void reset(AVFormatContext* fmt_ctx);
};

}
