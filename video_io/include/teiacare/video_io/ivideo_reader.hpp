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
#include <optional>
#include <tuple>

namespace tc::vio
{
class ivideo_reader
{
public:
    virtual ~ivideo_reader() = default;

    virtual bool open(const std::string& video_path) = 0;
    virtual bool is_opened() const = 0;
    virtual bool read(uint8_t** data, double* pts = nullptr) = 0;
    virtual void release() = 0;

    virtual auto get_frame_count() const -> std::optional<int> = 0;
    virtual auto get_duration() const -> std::optional<std::chrono::steady_clock::duration> = 0;
    virtual auto get_frame_size() const -> std::optional<std::tuple<int, int>> = 0;
    virtual auto get_frame_size_in_bytes() const -> std::optional<int> = 0;
    virtual auto get_fps() const -> std::optional<double> = 0;
};

}
