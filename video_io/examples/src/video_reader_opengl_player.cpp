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
 * example: 	video_player_opengl
 * author:		Stefano Lusardi
 * date:		Jun 2021
 * description:	Example to show how to integrate vio::video_reader in a simple video player based on OpenGL (using GLFW). 
 * 				Single threaded: Main thread decodes and draws subsequent frames.
 * 				Note that this serves only as an example, as in real world application 
 * 				you might want to handle decoding and rendering on separate threads (see any video_player_xxx_multi_thread).
*/

#include <iostream>
#include <thread>

#include <teiacare/video_io/video_reader.hpp>
#include "utils/simple_frame.hpp"

#include <GLFW/glfw3.h>

using namespace std::chrono_literals;


double get_elapsed_time()
{
	static std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double>> start_time = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_time = std::chrono::steady_clock::now() - start_time;
	return elapsed_time.count();
}

int main(int argc, char **argv)
{
	std::cout << "GLFW version: " << glfwGetVersionString() << std::endl;
	tc::vio::video_reader v;

	const char* video_path = "video.mkv";
	if (argc > 1)
		video_path = argv[1];

	if (!v.open(video_path))
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

	GLFWwindow *window = glfwCreateWindow(frame_width, frame_height, "Video Player OpenGL", NULL, NULL);
	if (!window)
	{
		std::cout << "Couldn't open window" << std::endl;
		return 1;
	}

	glfwMakeContextCurrent(window);

	GLuint texture_handle;
	glGenTextures(1, &texture_handle);
	glBindTexture(GL_TEXTURE_2D, texture_handle);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	int window_width, window_height;
	glfwGetFramebufferSize(window, &window_width, &window_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, window_width, window_height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);

	tc::vio::examples::utils::simple_frame frame;
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (!v.read(&frame.data, &frame.pts))
		{
			std::cout << "Video finished" << std::endl;
			break;
		}

		if (const auto timeout = frame.pts - get_elapsed_time(); timeout > 0.0)
			std::this_thread::sleep_for(std::chrono::duration<double>(timeout));

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGB, GL_UNSIGNED_BYTE, frame.data);

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture_handle);
		glBegin(GL_QUADS);

		glTexCoord2d(0, 0);
		glVertex2i(0, 0);

		glTexCoord2d(1, 0);
		glVertex2i(frame_width, 0);

		glTexCoord2d(1, 1);
		glVertex2i(frame_width, frame_height);

		glTexCoord2d(0, 1);
		glVertex2i(0, frame_height);

		glEnd();
		glDisable(GL_TEXTURE_2D);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	v.release();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}