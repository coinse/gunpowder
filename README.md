# Coinse AVM (CAVM)

CAVM (read `kaboom` |kəˈbuːm|) is a test data generation tool for C/C++ code, based on the Alternating Variable Method (AVM). 

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
python3 -m cavm.main sample/triangle.c -f get_type --min 0
[0, False] [6, 30, 4]
[0, True] [97, 85, 43]
[1, False] [18, 70, 81]
[1, True] [100, 100, 99]
[2, False] [68, 68, 28]
[2, True] [91, 79, 6]
[3, False] [98, 25, 93]
[3, True] [100, 35, 59]
[4, False] [37, 37, 11]
[4, True] [35, 35, 61]
[5, False] [76, 46, 46]
[5, True] [20, 20, 20]
[6, False] [12, 7, 18]
[6, True] FAIL [88, 83, 13]
[7, False] [93, 99, 88]
[7, True] [82, 37, 82]
```
You can start with examples in `/sample`. CAVM takes path of target code and name of target function as command line arguments. For full usage instructions, please see the output of:
```sh
$ python3 -m cavm.main --help
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
