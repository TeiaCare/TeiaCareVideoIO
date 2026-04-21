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

import os
from conan import ConanFile
from conan.tools.files import copy


class VideoIOExamples(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    requires = "imgui/1.91.4-docking", "glfw/3.4"
    generators = "CMakeDeps", "CMakeToolchain"

    def generate(self):
        imgui_src = os.path.join(self.dependencies["imgui"].package_folder, "res", "bindings")
        repo_root = os.path.abspath(os.path.join(self.recipe_folder, "..", ".."))
        dst = os.path.join(repo_root, "build", "imgui_bindings")
        for f in (
            "imgui_impl_glfw.h",
            "imgui_impl_glfw.cpp",
            "imgui_impl_opengl3.h",
            "imgui_impl_opengl3.cpp",
            "imgui_impl_opengl3_loader.h",
        ):
            copy(self, f, src=imgui_src, dst=dst)
