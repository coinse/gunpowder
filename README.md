# Dependencies
This project is using [clang 3.9](http://llvm.org/releases/download.html#3.9.0). You can download pre-built binaries and build script will refer it.

# Build
> This project is based on (https://github.com/eliben/llvm-clang-samples) and (https://github.com/eliben/llvm-clang-samples/pull/11) especially for MacOS compatibility.

First, you need to set the path of pre-built binaries of clang.  
For example, following is the command to build `buildcfg`

```sh
$ export BINARY_DIR_PATH=./clang+llvm-3.9.0-x86_64-apple-darwin
$ ./built_vs_released_binary.sh
```

If you have built llvm and clang using source code at your machine, you may use it directly. In this case, please look at `Makefile`.

# Run
```sh
$ ./buildcfg <target C file>
```

# File description
- `buildcfg.cpp` : C program for code instrumentation for preparing AVM, finding data dependency.  
- `Makefile` : Makefile for compiling buildcfg  

- `util/` : directory of files to be used for AVMF  
  - `branchdistance.c` : program calculating branchdistance (to be compiled with target code)  
  - `log.c` : program levaing branch execution trace (to be compiled with target code)  
  - `approachlevel.`c: program calculating approoachlevel (to be compiled with target code)  

- `sample`/ : directory of sample test source code files  
  - `calendar.c`, `line.c`, `triangle.c` : sample file  

