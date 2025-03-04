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
import sys

def parse():
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("--doxyfile_path", help="Path of the Doxyfile to be used", default="video_io/docs/Doxyfile")
    return parser.parse_args()

def main():
    args = parse()
    doxyfile_path = pathlib.Path(args.doxyfile_path).absolute()
    if not doxyfile_path.exists() or not doxyfile_path.is_file():
        raise RuntimeError(f'Doxyfile does not exist: {doxyfile_path}')

    print("Doxygen version:")
    run(['doxygen', '--version'])
    run(['doxygen', doxyfile_path])

if __name__ == '__main__':
    sys.exit(main())
