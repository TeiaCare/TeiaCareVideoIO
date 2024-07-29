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

#include <cstring>
#include <functional>
#include <map>
#include <sstream>
#include <utility>

extern "C"
{
#include <libavutil/error.h>
}

#define VIDEO_CAPTURE_LOG_ENABLED

#if defined(VIDEO_CAPTURE_LOG_ENABLED)
#include <iostream>
#include <mutex>
#define log_info(...) vio::logger::get().log(log_level::info, ##__VA_ARGS__)
#define log_error(...) vio::logger::get().log(log_level::error, ##__VA_ARGS__)
#else
#define log_info(...) (void)0
#define log_error(...) (void)0
#endif

namespace tc::vio
{
enum class log_level
{
    all,
    info,
    error
};

class logger
{
public:
    logger()
    {
#if defined(VIDEO_CAPTURE_LOG_ENABLED)
        set_log_callback([this](const std::string& str) { default_callback_info(str); }, log_level::info);
        set_log_callback([this](const std::string& str) { default_callback_error(str); }, log_level::error);
#endif
    }

    static logger& get()
    {
        static logger instance;
        return instance;
    }

    template <typename... Args>
    void log(const log_level& level, Args&&... args)
    {
        if (auto cb = log_callbacks.find(level); cb != log_callbacks.end())
        {
            std::stringstream s;
            ((s << std::forward<Args>(args) << ' '), ...);
            cb->second(s.str());
        }
    }

    using log_callback_t = std::function<void(const std::string&)>;
    void set_log_callback(const log_callback_t& cb, const log_level& level)
    {
        if (level == log_level::all)
        {
            log_callbacks[log_level::info] = cb;
            log_callbacks[log_level::error] = cb;
            return;
        }

        log_callbacks[level] = cb;
    }

    const char* err2str(int errnum)
    {
        static char str[AV_ERROR_MAX_STRING_SIZE];
        std::memset(str, 0, sizeof(str));
        return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
    }

#if defined(VIDEO_CAPTURE_LOG_ENABLED)
protected:
    void default_callback_info(const std::string& str)
    {
        std::scoped_lock lock(_default_mutex);
        std::cout << "[::  INFO ::] " << str << std::endl;
    }

    void default_callback_error(const std::string& str)
    {
        std::scoped_lock lock(_default_mutex);
        std::cout << "[:: ERROR ::] " << str << std::endl;
    }

    std::mutex _default_mutex;
#endif

private:
    std::map<log_level, log_callback_t> log_callbacks;
};

}
