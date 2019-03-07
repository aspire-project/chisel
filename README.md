# Chisel [![CircleCI](https://circleci.com/gh/aspire-project/chisel.svg?style=svg)](https://circleci.com/gh/aspire-project/chisel)

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

For ensuring that everything is working properly, run the following command:
```sh
make test
```

## Quick Start
After building the project run the below command:
```sh
$ chisel ./test.sh file.c
```
where `file.c` is a C program that you aim to reduce, and `test.sh` is
the property testing script that returns `0` in a successful call.
The reduced program is saved in `file.c.chisel.c` by default.
