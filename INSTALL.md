# Installing Dependencies
## Linux
```sh
$ apt-get install clang libclang-dev llvm-dev cmake
$ apt-get install libspdlog-dev nlohmann-json-dev
$ apt-get install libmlpack-dev
```
## macOS
If you use [Homebrew](https://brew.sh):
```sh
$ brew install llvm armadillo boost
$ git clone https://github.com/mlpack/mlpack
$ cd mlpack
$ mkdir build && cd build
$ cmake ..
$ make
$ make install

$ brew install spdlog
```

## From source
Install Clang with the extra Clang tools following the instructions [here](https://clang.llvm.org/get_started.html).
