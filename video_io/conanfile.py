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

class FFMPEG(ConanFile):
    requires = "ffmpeg/6.1.1"
    generators = "CMakeDeps"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            del self.options.fPIC

        self.options["ffmpeg"].disable_all_encoders=True
        self.options["ffmpeg"].enable_encoders='libx264'

        self.options["ffmpeg"].disable_all_decoders=True
        self.options["ffmpeg"].enable_decoders='h264,hevc,mpegvideo,mpeg1video,mpeg2video,mpeg4,mjpeg'

        self.options["ffmpeg"].disable_all_muxers=True
        self.options["ffmpeg"].enable_muxers='rtp,rtsp,mov,h264,hevc,matroska,mp4'

        self.options["ffmpeg"].disable_all_demuxers=True
        self.options["ffmpeg"].enable_demuxers='rtp,rtsp,h264,hevc,mov,matroska,mjpeg,mpeg,mpegvideo'

        self.options["ffmpeg"].disable_all_protocols=True
        self.options["ffmpeg"].enable_protocols='rtp,srtp,udp,tcp,file,async'

        self.options["ffmpeg"].disable_all_parsers=True
        self.options["ffmpeg"].disable_all_hardware_accelerators=True
        self.options["ffmpeg"].disable_all_bitstream_filters=True
        self.options["ffmpeg"].disable_all_devices=True
        self.options["ffmpeg"].disable_all_filters=True

        self.options["ffmpeg"].avformat=True
        self.options["ffmpeg"].avcodec=True
        self.options["ffmpeg"].swscale=True
        self.options["ffmpeg"].avdevice=False
        self.options["ffmpeg"].avfilter=False
        self.options["ffmpeg"].swresample=False

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
        self.options["ffmpeg"].with_ssl='openssl'
        self.options["ffmpeg"].with_programs=True
        self.options["ffmpeg"].with_libsvtav1=False
        self.options["ffmpeg"].with_libaom=False
        self.options["ffmpeg"].with_libdav1d=False

        if self.settings.os == "Linux":
            self.options["ffmpeg"].with_libalsa=False
            self.options["ffmpeg"].with_pulse=False
            self.options["ffmpeg"].with_vaapi=False
            self.options["ffmpeg"].with_vdpau=False
            self.options["ffmpeg"].with_vulkan=False
            self.options["ffmpeg"].with_xcb=False
            self.options["ffmpeg"].with_xlib=False
