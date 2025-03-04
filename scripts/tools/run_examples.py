#!/usr/bin/python

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

import argparse
from command import run
import pathlib
import os

def parse():
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("examples_directory", help="Examples Directory")
    return parser.parse_args()

def is_executable(file_path):
    return os.access(file_path, os.X_OK)

def print_example_name(example_name):
    line_long = '#'*80
    line_short = '#'*3
    print(f'\n\n{line_long}\n{line_short}', flush=True)
    print(f'{line_short} {example_name}', flush=True)
    print(f'{line_short}\n{line_long}', flush=True)

def examples(args):
    examples_path = pathlib.Path(args.examples_directory).absolute()
    if not examples_path.exists() or not examples_path.is_dir():
        print("Examples directory must be an existing directory")
        return

    examples_found = False
    for example in examples_path.iterdir():
        if example.is_file() and is_executable(example) and example.name.startswith(("video_reader_simple_decode", "video_writer_simple_encode")):
            examples_found = True
            print_example_name(example.name)
            run(example)

    if not examples_found:
        raise RuntimeError(f'No examples found into {examples_path}')

if __name__ == '__main__':
    examples(parse())
