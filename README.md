# file description


buildcfg.cpp : C program for code instrumentation for preparing AVM, finding data dependency.  
Makefile : Makefile for compiling buildcfg  

util/ : directory of files to be used for AVMF  
branchdistance.c : program calculating branchdistance (to be compiled with target code)  
log.c : program levaing branch execution trace (to be compiled with target code)  
approachlevel.c: program calculating approoachlevel (to be compiled with target code)  

sample/ : directory of sample test source code files  
calendar.c, line.c, triangle.c : sample file  

# how to use
1. follow step 1,2,3,4 on http://clang.llvm.org/get_started.html
2. modify Makefile so that Clang can be linked.
3. make buildcfg.cpp
4. ./buildcfg $target C file$
