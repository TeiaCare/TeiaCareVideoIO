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

#include <gtest/gtest.h>

namespace tc::vio { class raw_frame; }

namespace tc::vio::test
{

class raw_frame_test : public ::testing::Test
{
protected:
    explicit raw_frame_test() { }
    virtual ~raw_frame_test() { }
    virtual void SetUp() override { }
    virtual void TearDown() override { }

    std::unique_ptr<vio::raw_frame> raw_frame;

private:
    template<typename... Args>
    void log(Args&&... args) const
    {
        ((std::cout << std::forward<Args>(args) << ' ') , ...) << std::endl;
    }
};

}