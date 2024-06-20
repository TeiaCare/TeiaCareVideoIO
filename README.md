# TeiaCareVideoIO
Welcome to TeiaCareVideoIO!

*TeiaCareVideoIO* is a C++ video encoder and decoder library.

![TeiaCareVideoIO](https://socialify.git.ci/TeiaCare/TeiaCareVideoIo/image?description=1&font=Raleway&name=1&pattern=Solid&theme=Auto)

---

## Getting Started

**Create a virtual environment**

```bash
python -m pip install --upgrade pip
python -m venv .venv

# Linux
echo "export CONAN_USER_HOME=$PWD" >> .venv/bin/activate
source .venv/bin/activate

# Windows
echo set CONAN_USER_HOME=%CD%>>.venv\Scripts\activate.bat
.venv\Scripts\activate.bat

pip install -r scripts/requirements.txt

# on developer machine only (not in CI)
pip install pre-commit==3.7.1
pre-commit install
```

**Setup Build Environment (Windows Only)**

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

**Dependencies Setup**

This script must be executed in order to setup the conan packages (note that 3rd party libs are only required for unit tests, examples and benchmarks).
```bash
python scripts/conan/setup.py <Debug|Release|DebWithRelInfo> <COMPILER_NAME> <COMPILER_VERSION>
```

**Configure, Build and Install**

This script configures, builds and installs the library.
```bash
python scripts/cmake.py <Debug|Release|DebWithRelInfo> <COMPILER_NAME> <COMPILER_VERSION>
```

## Examples

```bash
python scripts/cmake.py <Debug|Release|RelWithDebInfo> <COMPILER_NAME> <COMPILER_VERSION> --examples --warnings
```
Examples are installed in $PWD/install/examples.


## Unit Tests and Code Coverage

Note that code coverage is not available on Windows.

```bash
python scripts/cmake.py <Debug|Release|RelWithDebInfo> <COMPILER_NAME> <COMPILER_VERSION> --coverage --warnings
python scripts/tools/run_unit_tests.py <Debug|Release|RelWithDebInfo>
python scripts/tools/run_coverage.py <COMPILER_NAME> <COMPILER_VERSION>
```
Unit tests results are available in $PWD/results/unit_tests.
Coverage results are available in $PWD/results/coverage.


## Benchmarks

```bash
python scripts/cmake.py <Debug|Release|RelWithDebInfo> <COMPILER_NAME> <COMPILER_VERSION> --benchmarks --warnings
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

```bash
python scripts/tools/run_cppcheck.py <Debug|Release|RelWithDebInfo>
```

- [cpplint](https://github.com/cpplint/cpplint) [TODO: Review]
```bash
# TODO: add python script.
cpplint --counting=detailed  $(find teiacare_video_io* -type f -name "*.hpp" -or -name "*.cpp")
```


## Generate Documentation

First install and setup *Doxygen* from your OS package manager.
```bash
# Linux
apt-get install doxygen graphviz

# Windows
winget install doxygen

# Update Doxyfile (required only after Doxygen updates)
doxygen -u video_io/docs/Doxyfile
```

```bash
python ./scripts/cmake/configure.py <Debug|Release|RelWithDebInfo>  <COMPILER_NAME> <COMPILER_VERSION> --docs
```
Documentation is now installed in $PWD/install/docs.


## Conan Package - Local Install

```bash
git clone https://teiacare@dev.azure.com/teiacare/Ancelia/_git/TeiaCareVideoIO
cd TeiaCareVideoIO

# Create, test and install local package
# Notes:
# 1) The install directory path must be a valid Conan cache (i.e. ".conan" folder) located in the current directory
#    So, in order to install the package in a desired repository folder, it is required to run this script from the repository folder directly.
# 2) The Conan package tests are automatically run during package creation.
#    The directory test_package contains a test project that is built to validate the proper package creation.

python ./scripts/conan/create.py <Debug|Release|RelWithDebInfo>  <COMPILER_NAME> <COMPILER_VERSION>

# Build, install and run the test package executable
python test_package/build.py <Debug|Release|RelWithDebInfo>  <COMPILER_NAME> <COMPILER_VERSION>
$PWD/install/test_package/teiacare_video_io_test_package
```


## Conan Package - Artifactory Setup

In order to push a Conan package to TeiaCare artifactory server it is required to setup you local Conan client with the following commands:

```bash
# export CONAN_REVISIONS_ENABLED=1
conan remote add teiacare_video_io $(artifactory.url)/teiacare_video_io
conan user $(artifactory.username) -p $(artifactory.password) -r teiacare_video_io
python scripts/conan/create.py <Debug|Release|RelWithDebInfo> <COMPILER_NAME> <COMPILER_VERSION>
python scripts/conan/upload.py teiacare_video_io teiacare_video_io/<PACKAGE_VERSION>@
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
