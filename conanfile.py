#!/usr/bin/env python
# Copyright 2024 TeiaCare
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from conans import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain
import re

def get_project_version():
    with open('VERSION', encoding='utf8') as version_file:
        version_regex = r'^\d+\.\d+\.\d+$'
        version = version_file.read().strip()
        if re.match(version_regex, version):
            return version
        else:
            raise ValueError(f"Invalid version detected into file VERSION: {version}")

class TeiaCareVideoIO(ConanFile):
    name = "teiacare_video_io"
    version = get_project_version()
    author = "TeiaCare"
    url = "https://github.com/TeiaCare/TeiaCareVideoIO"
    description = "TeiaCareVideoIO is a C++ video encoder and decoder library"
    topics = ("video", "encoding", "decoding")
    exports = "VERSION"
    exports_sources = "CMakeLists.txt", "video_io/*", "cmake/*"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    requires = "ffmpeg/5.1"
    generators = "CMakeDeps"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC
        
        # self.options["ffmpeg"].disable_everything=True
        
        self.options["ffmpeg"].with_asm=True
        self.options["ffmpeg"].with_zlib=True
        self.options["ffmpeg"].with_bzip2=True
        self.options["ffmpeg"].with_lzma=False
        self.options["ffmpeg"].with_libiconv=False
        self.options["ffmpeg"].with_freetype=False
        self.options["ffmpeg"].with_openjpeg=False
        self.options["ffmpeg"].with_openh264=True
        self.options["ffmpeg"].with_opus=False
        self.options["ffmpeg"].with_vorbis=False
        self.options["ffmpeg"].with_zeromq=False
        self.options["ffmpeg"].with_sdl=False
        self.options["ffmpeg"].with_libx264=True
        self.options["ffmpeg"].with_libx265=True
        self.options["ffmpeg"].with_libvpx=False
        self.options["ffmpeg"].with_libmp3lame=False
        self.options["ffmpeg"].with_libfdk_aac=False
        self.options["ffmpeg"].with_libwebp=False
        self.options["ffmpeg"].with_ssl="openssl"
        self.options["ffmpeg"].with_libalsa=False
        self.options["ffmpeg"].with_pulse=False
        self.options["ffmpeg"].with_vaapi=False
        self.options["ffmpeg"].with_vdpau=False
        self.options["ffmpeg"].with_vulkan=False
        self.options["ffmpeg"].with_xcb=False
        self.options["ffmpeg"].with_programs=False

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["BUILD_SHARED_LIBS"] = self.options.shared
        tc.variables["TC_ENABLE_UNIT_TESTS"] = False
        tc.variables["TC_ENABLE_UNIT_TESTS_COVERAGE"] = False
        tc.variables["TC_ENABLE_BENCHMARKS"] = False
        tc.variables["TC_ENABLE_EXAMPLES"] = False
        tc.variables["TC_ENABLE_DOCS"] = False
        tc.variables["TC_ENABLE_WARNINGS_ERROR"] = True
        tc.variables["TC_ENABLE_SANITIZER_ADDRESS"] = False
        tc.variables["TC_ENABLE_SANITIZER_THREAD"] = False
        tc.variables["TC_ENABLE_CLANG_FORMAT"] = False
        tc.variables["TC_ENABLE_CLANG_TIDY"] = False
        tc.variables["TC_ENABLE_CPPCHECK"] = False
        tc.variables["TC_ENABLE_CPPLINT"] = False
        tc.variables["TC_ENABLE_DOCS"] = False
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy(pattern="VERSION")
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["teiacare_video_io"]
        self.cpp_info.set_property("cmake_file_name", "teiacare_video_io")
        self.cpp_info.set_property("cmake_target_name", "teiacare::video_io")
