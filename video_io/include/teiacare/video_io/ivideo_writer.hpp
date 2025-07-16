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

#include <string>

namespace tc::vio
{
class ivideo_writer
{
public:
    virtual ~ivideo_writer() = default;

    virtual bool open(const std::string& video_path, int width, int height, const int fps) = 0;
    virtual bool open(const std::string& video_path, int width, int height, const int fps, const int duration) = 0;
    virtual bool is_opened() const = 0;
    virtual bool write(const uint8_t* data) = 0;
    virtual void release() = 0;
    virtual bool save() = 0;

    virtual bool check(const std::string& video_path) = 0;
};

}
