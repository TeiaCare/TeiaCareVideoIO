# TeiaCareVideoIO

Welcome to TeiaCareVideoIO!

*TeiaCareVideoIO* is a C++ video encoder and decoder library.

[![Docs](https://img.shields.io/badge/Docs-TeiaCareVideoIO-white.svg)](https://teiacare.github.io/TeiaCareVideoIO/md_README.html)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/baef7f2dfab74df1985a25ee5ebcb88e)](https://app.codacy.com/gh/TeiaCare/TeiaCareVideoIO/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![Codacy Badge](https://app.codacy.com/project/badge/Coverage/baef7f2dfab74df1985a25ee5ebcb88e)](https://app.codacy.com/gh/TeiaCare/TeiaCareVideoIO/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_coverage)

![TeiaCareVideoIO](https://socialify.git.ci/TeiaCare/TeiaCareVideoIo/image?description=1&font=Raleway&name=1&pattern=Solid&theme=Auto)

## Getting Started

This project uses git submodules so it is required to clone it using the *--recursive* flag in order to retrive the required submodules.

```bash
git clone https://github.com/TeiaCare/TeiaCareVideoIO.git --recursive
```

### Create Development Environment
The build toolchain uses Conan 2 and [venvpp2](https://github.com/FrancescoOlivaTC/venvpp2) (vendored as the `scripts/` submodule).

```bash
# Linux/MacOS
python3 -m venv .venv
source .venv/bin/activate
pip3 install -r scripts/scripts/requirements.txt

# Windows
python -m venv .venv
.venv\Scripts\activate.bat
pip install -r scripts/scripts/requirements.txt
```

### Setup Build Environment (Windows Only)
When building from command line on Windows it is necessary to activate the Visual Studio Developer Command Prompt.

Depending on the version of Visual Studio compiler and on its install location it is required to run *vcvars64.bat* script the set the development environment properly.

*Note*: using Visual Studio IDE or the CMake extension for VSCode this step is already managed in the background, so no action is required.

Examples:

```bash
# Visual Studio 2022 - Build Tools
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

# Visual Studio 2019 - Enterprise
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
```

### Dependencies Setup
Point Conan 2 at the TeiaCare Artifactory and install dependencies into a local cache under `.conan2/`.

```bash
export CONAN_HOME=$PWD/.conan2
conan remote add teiacare https://artifactory.app.teiacare.com/artifactory/api/conan/teiacare --index 0 --force
conan remote login teiacare <USERNAME> -p <PASSWORD>

# Library deps (ffmpeg)
conan install video_io/conanfile.py --output-folder build/modules -pr:a=scripts/profiles/linux-gcc -s build_type=Debug --build missing

# Test deps (gtest, spdlog)
conan install video_io/tests --output-folder build/modules -pr:a=scripts/profiles/linux-gcc -s build_type=Debug --build missing

# Example deps (imgui, glfw) — copies ImGui bindings into build/imgui_bindings
conan install video_io/examples --output-folder build/modules -pr:a=scripts/profiles/linux-gcc -s build_type=Debug --build missing
```

### Configure, Build and Install
```bash
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE=build/modules/conan_toolchain.cmake \
  -DTC_ENABLE_UNIT_TESTS=True -DTC_ENABLE_EXAMPLES=True \
  -B build/Debug -S . --fresh
cmake --build build/Debug
cmake --install build/Debug --prefix install
```

## Install FFmpeg for Examples and Unit Tests

Examples and Unit Tests execution requires video data generation. In order to generate such data it is required to install FFmpeg using your OS package manager such as:

```bash
# Windows, using chocolatey
choco install ffmpeg -y

# Windows, using winget
winget install ffmpeg

# Linux
sudo apt-get install -y ffmpeg

# MacOS
brew install ffmpeg
```

Once FFmpeg is installed and available on the path run the following python script:

```bash
python scripts/tests/generate_test_data.py
```


## Examples

```bash
# Build with examples enabled (see "Configure, Build and Install" above with -DTC_ENABLE_EXAMPLES=True)

# Run all the examples
python scripts/tools/run_examples.py install/examples
```
Examples are installed in $PWD/install/examples.


## Unit Tests and Code Coverage

```bash
# Run Unit Tests
python scripts/tools/run_unit_tests.py <Debug|Release|RelWithDebInfo>

# Run Code Coverage (enable with -DTC_ENABLE_UNIT_TESTS_COVERAGE=True at configure time)
python scripts/tools/run_coverage.py <COMPILER_NAME> <COMPILER_VERSION> video_io
```
Note that code coverage is not available on Windows.

Unit tests results are available in $PWD/results/unit_tests.
Coverage results are available in $PWD/results/coverage.


## Sanitizers

### Address Sanitizer

Configure with `-DTC_ENABLE_SANITIZER_ADDRESS=True` (Linux only), then:

```bash
python scripts/tools/run_sanitizer.py --address_sanitizer install/unit_tests/teiacare_video_io_unit_tests
```
Note that Address Sanitizer is supported only on Linux.


### Thread Sanitizer

Configure with `-DTC_ENABLE_SANITIZER_THREAD=True` (Linux only), then:

```bash
python scripts/tools/run_sanitizer.py --thread_sanitizer install/unit_tests/teiacare_video_io_unit_tests
```
Note that Thread Sanitizer is supported only on Linux.


## Benchmarks

Configure with `-DTC_ENABLE_BENCHMARKS=True`, then:

```bash
python scripts/tools/run_benchmarks.py <COMPILER_NAME> <COMPILER_VERSION>
```
Benchmarks are installed in $PWD/install/benchmarks.


## Code Formatting

- [clang-format](https://clang.llvm.org/docs/ClangFormat.html)

*clang-format* can be installed via *pip* using the provided *scripts/requirements.txt*

```bash
python scripts/tools/run_clang_format.py -r -i video_io
```


## Code Analysis

- [clang-tidy](https://clang.llvm.org/extra/clang-tidy/)

*clang-tidy* can be installed via *pip* using the provided *scripts/requirements.txt*

```bash
python scripts/tools/run_clang_tidy.py -header-filter=.* video_io
```

- [cppcheck](http://cppcheck.net/)

First install and setup *cppcheck* from your OS package manager.
```bash
# Linux
sudo apt install cppcheck

# Windows
winget install cppcheck
```

Then run CppCheck using the provided python script:
```bash
python scripts/tools/run_cppcheck.py <Debug|Release|RelWithDebInfo>
```


## Generate Documentation

First install and setup *Doxygen* from your OS package manager.
```bash
# Linux
apt-get install doxygen graphviz

# Windows
winget install doxygen
```

Then run Doxygen using the provided python script:
```bash
python scripts/tools/run_doxygen.py
```
Documentation is now installed in $PWD/docs.


## Conan Package

### Local Install

Create, test and install local package.

Notes:
1) The install directory path must be a valid Conan cache (i.e. ".conan" folder) located in the current directory.
   So, in order to install the package in a desired repository folder, it is required to run this script from the repository folder directly.
2) The Conan package tests are automatically run during package creation.
   The directory test_package contains a test project that is built to validate the proper package creation.

```bash
# Create the Conan package locally (runs the test_package automatically)
conan create . -s build_type=Debug -pr:a=.ci/profiles/linux-gcc-12 --build missing
```


### Artifactory Upload

In order to upload a Conan package to TeiaCare Artifactory server it is required to setup you local Conan client once with the following commands:

```bash
# Add TeiaCare Artifactory remote to local Conan client
conan remote add teiacare $(artifactory.url)/teiacare --index 0 --force

# Authenticate with Artifactory credentials
conan remote login teiacare $(artifactory.username) -p $(artifactory.password)
```

Now it is possible to create and upload a Conan package with the following commands:

```bash
# Create the Conan package locally
conan create . -s build_type=Debug -pr:a=.ci/profiles/linux-gcc-12 --build missing

# Upload all cached packages to the teiacare remote
conan upload "*" --remote teiacare --confirm
```


## Contributing

In order to contribute to TeiaCareVideoIO, please follow our [contribution guidelines](./CONTRIBUTING).

[![Contributions](https://img.shields.io/badge/Contributions-Welcome-green.svg)](./CONTRIBUTING)


## License

This project is licensed under the [Apache License, Version 2.0](./LICENSE).
Copyright © 2024 [TeiaCare](https://teiacare.com/)

[![License](https://img.shields.io/badge/License-Apache_v2-blue)](./LICENSE)


## Video Streaming Integration Test (RTSP)

```bash
# Start mediamtx proxy stream
docker run --network=host -e MTX_PROTOCOLS=tcp -p 8554:8554 bluenviron/mediamtx

# Stream a local video to the proxy
ffmpeg -nostdin -stream_loop -1 -re -i "<PATH/TO/LOCAL/VIDEO>" -an -f rtsp -rtsp_transport tcp rtsp://localhost:8554/live

# Run a video reader example application
./video_reader_imgui_player rtsp://localhost:8554/live
./video_reader_opengl_player rtsp://localhost:8554/live
```
