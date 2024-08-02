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

/**
 * example: 	video_player_imgui
 * author:		Stefano Lusardi
 * date:		Jun 2022
 * description:	Example to show how to integrate vio::video_reader in a simple video player based on OpenGL (using GLFW).
 * 				Single threaded: Main thread decodes and draws subsequent frames.
 * 				Note that this serves only as an example, as in real world application
 * 				you might want to handle decoding and rendering on separate threads (see any video_player_xxx_multi_thread).
 */

#include <teiacare/video_io/video_reader.hpp>

#include "utils/simple_frame.hpp"
#include "utils/video_data_path.hpp"
#include <GLFW/glfw3.h>
#include <chrono>
#include <filesystem>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;

int main(int argc, char** argv)
{
    std::cout << "GLFW version: " << glfwGetVersionString() << std::endl;
    tc::vio::video_reader v;

    std::filesystem::path default_video_path = std::filesystem::path(tc::vio::examples::utils::video_data_path) / "video_10sec_2fps_HD.mp4";
    const char* video_path = default_video_path.c_str();
    if (argc > 1)
        video_path = argv[1];

    if (!v.open(video_path)) // , vio::decode_support::SW
    {
        std::cout << "Unable to open video: " << video_path << std::endl;
        return 1;
    }

    const auto fps = v.get_fps();
    const auto size = v.get_frame_size();
    const auto [frame_width, frame_height] = size.value();

    if (!glfwInit())
    {
        std::cout << "Couldn't init GLFW" << std::endl;
        return 1;
    }

    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    const auto window_width = 1280;
    const auto window_height = 720;
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Video Player imgui", NULL, NULL);
    if (!window)
    {
        std::cout << "Couldn't open window" << std::endl;
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    int screen_width, screen_height;
    glfwGetFramebufferSize(window, &screen_width, &screen_height);
    glViewport(0, 0, screen_width, screen_height);
    glfwSetWindowAspectRatio(window, window_width, window_height);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigDockingTransparentPayload = true;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    GLuint texture_handle;
    glGenTextures(1, &texture_handle);
    glBindTexture(GL_TEXTURE_2D, texture_handle);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glClearColor(0.f, 0.f, 0.f, 0.f);

    uint8_t* frame_data = {};

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_AutoHideTabBar);

        if (!v.read(&frame_data))
        {
            if (v.is_opened())
            {
                std::cout << "Video finished" << std::endl;
                v.release();
            }

            break;
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGB, GL_UNSIGNED_BYTE, frame_data);
        }

        {
            ImGui::Begin("Video");
            ImGui::Image((void*)static_cast<uintptr_t>(texture_handle), ImVec2(frame_width, frame_height));
            ImGui::End();
        }

        {
            ImGui::Begin("Info");
            ImGui::Text("Window Size: [%d, %d]", static_cast<int>(ImGui::GetMainViewport()->Size.x), static_cast<int>(ImGui::GetMainViewport()->Size.y));
            ImGui::Text("Rendering: %.1f (FPS)", ImGui::GetIO().Framerate);
            ImGui::End();
        }

        ImGui::ShowDemoWindow();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        // Limit frame rate by sleeping on the current thread...
        std::this_thread::sleep_for(100ms);
    }

    v.release();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
