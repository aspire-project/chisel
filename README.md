# Chisel

## Requirements
* CMake >= 3.11
* Clang and LibClang >= 4.0
* mlpack >= 2.0

## Installation
### Installing Dependencies
See [INSTALL.md](INSTALL.md).

### Installing Chisel
Once you have all the requirements, run the following commands:
```sh
$ git clone https://github.com/aspire-project/chisel.git
$ cd chisel
$ mkdir build && cd build
$ cmake ..
$ make
```
Make sure to add Chisel to the PATH:
```sh
$ export PATH=[chisel directory]/build/bin:$PATH
```

## Quick Start
After building the project run the below command:
```sh
$ chisel ./test.sh file.c
```
where `file.c` is a C program that you aim to reduce, and `test.sh` is
the property testing script that returns `0` in a successful call.
The reduced program is saved in `file.c.chisel.c` by default.

For running Chisel on each of the examples provided under `examples/`, copy the original c program specified by
`[filename].orig.c` to [filename], and run Chisel using the provided test script. For instance, in `examples/loop` directory,
```sh
$ cp loop.c.orig.c loop.c
$ chisel ./test.sh loop.c
```
