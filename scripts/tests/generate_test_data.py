#!/usr/bin/python
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
import shutil
import sys
import concurrent.futures
import subprocess
import pathlib
import argparse

APPLICATION_NAME = "Test data generator"


def parse():
    parser = argparse.ArgumentParser(APPLICATION_NAME, formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("--directory", help="Directory where output files will be generated", required=False, default=pathlib.Path("data"))
    parser.add_argument("--executable", help='Path to the ffmpeg executable', required=False, default='ffmpeg')
    return parser.parse_args()

def ffmpeg_generate_video_commands(ffmpeg_executable, output_dir, video_formats):
    commands = []
    for format in video_formats:
        commands.extend([
            # Resolution
            f'{ffmpeg_executable} -f lavfi -i "color=c=blue:size=640x480:duration=10:rate=4"   -vf "drawtext=fontsize=72:fontcolor=white:x=(w-text_w)/2:y=(h-text_h)/2:text=\'%{{n}}\':start_number=1" {output_dir}/video_10sec_4fps_SD.{format}',
            f'{ffmpeg_executable} -f lavfi -i "color=c=blue:size=1280x720:duration=10:rate=4"  -vf "drawtext=fontsize=72:fontcolor=white:x=(w-text_w)/2:y=(h-text_h)/2:text=\'%{{n}}\':start_number=1" {output_dir}/video_10sec_4fps_HD.{format}',
            f'{ffmpeg_executable} -f lavfi -i "color=c=blue:size=1920x1080:duration=10:rate=4" -vf "drawtext=fontsize=72:fontcolor=white:x=(w-text_w)/2:y=(h-text_h)/2:text=\'%{{n}}\':start_number=1" {output_dir}/video_10sec_4fps_FHD.{format}',
            f'{ffmpeg_executable} -f lavfi -i "color=c=blue:size=3840x2160:duration=10:rate=4" -vf "drawtext=fontsize=72:fontcolor=white:x=(w-text_w)/2:y=(h-text_h)/2:text=\'%{{n}}\':start_number=1" {output_dir}/video_10sec_4fps_4K.{format}',

            # FPS
            f'{ffmpeg_executable} -f lavfi -i "color=c=blue:size=1280x720:duration=10:rate=1"  -vf "drawtext=fontsize=72:fontcolor=white:x=(w-text_w)/2:y=(h-text_h)/2:text=\'%{{n}}\':start_number=1" {output_dir}/video_10sec_1fps_HD.{format}',
            f'{ffmpeg_executable} -f lavfi -i "color=c=blue:size=1280x720:duration=10:rate=2"  -vf "drawtext=fontsize=72:fontcolor=white:x=(w-text_w)/2:y=(h-text_h)/2:text=\'%{{n}}\':start_number=1" {output_dir}/video_10sec_2fps_HD.{format}',
            f'{ffmpeg_executable} -f lavfi -i "color=c=blue:size=1280x720:duration=10:rate=8"  -vf "drawtext=fontsize=72:fontcolor=white:x=(w-text_w)/2:y=(h-text_h)/2:text=\'%{{n}}\':start_number=1" {output_dir}/video_10sec_8fps_HD.{format}',
            f'{ffmpeg_executable} -f lavfi -i "color=c=blue:size=1280x720:duration=10:rate=16" -vf "drawtext=fontsize=72:fontcolor=white:x=(w-text_w)/2:y=(h-text_h)/2:text=\'%{{n}}\':start_number=1" {output_dir}/video_10sec_16fps_HD.{format}',
            f'{ffmpeg_executable} -f lavfi -i "color=c=blue:size=1280x720:duration=10:rate=30" -vf "drawtext=fontsize=72:fontcolor=white:x=(w-text_w)/2:y=(h-text_h)/2:text=\'%{{n}}\':start_number=1" {output_dir}/video_10sec_30fps_HD.{format}',

            # Duration
            f'{ffmpeg_executable} -f lavfi -i "color=c=blue:size=1280x720:duration=2:rate=2"   -vf "drawtext=fontsize=72:fontcolor=white:x=(w-text_w)/2:y=(h-text_h)/2:text=\'%{{n}}\':start_number=1" {output_dir}/video_2sec_2fps_HD.{format}',
            f'{ffmpeg_executable} -f lavfi -i "color=c=blue:size=640x480:duration=120:rate=30" -vf "drawtext=fontsize=72:fontcolor=white:x=(w-text_w)/2:y=(h-text_h)/2:text=\'%{{n}}\':start_number=1" {output_dir}/video_120sec_30fps_SD.{format}',
        ])

    return commands

def create_file(cmd):
    print(cmd)

    try:
        process_result = subprocess.run(cmd, shell=True, check=True)
        process_result.check_returncode()
    except Exception as e:
        print(f"Error creating video file: {e}")
        return 1
    return 0

def check_executable(executable):
    print(f"Using ffmpeg executable: {executable}")
    try:
        process_result = subprocess.run([executable, '-version'], check=True)
        process_result.check_returncode()
    except Exception as e:
        print(f"Error with ffmpeg executable: {executable} {e}")
        sys.exit(1)

def main():
    print(f"{APPLICATION_NAME} - Execution started")
    args = parse()
    output_dir = pathlib.Path(args.directory)
    executable = args.executable
    video_formats = ["mp4", "mkv"]

    if os.path.exists(output_dir) and os.path.isdir(output_dir):
        shutil.rmtree(output_dir)

    os.mkdir(output_dir)

    check_executable(executable)

    with concurrent.futures.ThreadPoolExecutor(max_workers=4) as executor:
        commands = ffmpeg_generate_video_commands(executable, output_dir, video_formats)
        futures = {executor.submit(create_file, cmd): cmd for cmd in commands}
        for future in concurrent.futures.as_completed(futures):
            if future.result() != 0:
                print(f"Error running {APPLICATION_NAME} - Execution finished")
                return future.result()

    print(f"{APPLICATION_NAME} - Execution finished")
    return 0

if __name__ == '__main__':
    sys.exit(main())
