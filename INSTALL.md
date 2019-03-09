# Installing Dependencies
## Requirements
* CMake >= 3.11
* Clang and LibClang >= 7.0
* mlpack >= 2.0
* spdlog >= 1.3.1

You can install the requirements with the following commands.
Make sure that your package manager installs the correct versions.
## Linux
```sh
$ apt-get install clang libclang-dev llvm-dev cmake libspdlog-dev libmlpack-dev
```
## macOS
If you use [Homebrew](https://brew.sh):
```sh
$ brew install llvm armadillo boost spdlog
$ git clone https://github.com/mlpack/mlpack
$ cd mlpack
$ mkdir build && cd build
$ cmake ..
$ make
$ make install
```
