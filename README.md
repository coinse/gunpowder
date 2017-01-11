# Coinse AVM (CAVM)

CAVM is AVM framework which targets C/C++ code.

## Dependencies

This project is using [clang 3.9](http://llvm.org/releases/download.html#3.9.0). You can download pre-built binaries and build script will refer it.

## Build
> This project is based on (https://github.com/eliben/llvm-clang-samples) and (https://github.com/eliben/llvm-clang-samples/pull/11) especially for MacOS compatibility.

First, you need to set the path of pre-built binaries of clang.  
Following is the command to build and install `cavm` python package which instruments and analyzes target code.

```sh
$ export BINARY_DIR_PATH=./clang+llvm-3.9.0-x86_64-apple-darwin
$ make python
```

If you have built llvm and clang using source code at your machine, you may use it directly. In this case, please look at `Makefile`.

## Run

```sh
$ python3 cavm/main.py sample/triangle.c -f get_type
[0, 0] [1, 9, 8]
[1, 0] [8, -2, 1]
[2, 0] [1, 5, 5]
[3, 0] [3, 8, 6]
[4, 0] [6, 1, 6]
[5, 0] [7, 9, 7]
[6, 0] [1, 2, 2]
[7, 0] [8, 7, 3]
[0, 1] [7, 5, 8]
[1, 1] [3, 3, 0]
[2, 1] [6, 1, 4]
[3, 1] [10, 7, 2]
[4, 1] [4, 7, 4]
[5, 1] [4, 4, 4]
[6, 1] fail [5, 4, 3]
[7, 1] [9, 9, 1]
```
You can start with examples in `/sample`. CAVM takes path of target code and name of target function as command line arguments. For full usage instructions, please see the output of:
```sh
$ python3 cavm/main.py --help
```


## Structure
Main part of CAVM is built with python. We use clang to instrument target C/C++ code and it provides python interface. Following is the short description of structure of project:

- `cavm/`: python codes of core CAVM logic.
- `lib/`: clang related codes.
- `util/` : utilties to be compiled with instrumented code.
- `sample/` : directory of sample test source code files  
- `Makefile` : Makefile to compile clang package and standalone instrumentation tool.


## Instrumenation

`cavm` python package provides interface to use clang from python. You can build and install `cavm` package:

```sh
$ make python
```

Also, you can use standalone version.

```sh
$ make
$ ./bin/buildcfg <filename> <function name>
```