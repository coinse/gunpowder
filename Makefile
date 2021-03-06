#-------------------------------------------------------------------------------
# Copyright (C) 2017 by Junhwi Kim <junhwi.kim23@gmail.com>
# Copyright (C) 2017 by Byeonghyeon You <byou@kaist.ac.kr>
# Copyright (C) 2017 by Shin Yoo <shin.yoo@kaist.ac.kr>
#
# Licensed under the MIT License:
# See the LICENSE file at the top-level directory of this distribution.
#-------------------------------------------------------------------------------

UNAME := $(shell uname -s)

# The following variables will likely need to be customized, depending on where
# and how you built LLVM & Clang. They can be overridden by setting them on the
# make command line: "make VARNAME=VALUE", etc.

# LLVM_SRC_PATH is the path to the root of the checked out source code. This
# directory should contain the configure script, the include/ and lib/
# directories of LLVM, Clang in tools/clang/, etc.
#
# Alternatively, if you're building vs. a binary distribution of LLVM
# (downloaded from llvm.org), then LLVM_SRC_PATH can point to the main untarred
# directory of the binary download (the directory that has bin/, lib/, include/
# and other directories inside).
# See the build_vs_released_binary.sh script for an example.
CLANG_DIR ?= $$HOME/llvm/llvm3.9-binaries
LLVM_SRC_PATH := $(CLANG_DIR)

# LLVM_BUILD_PATH is the directory in which you built LLVM - where you ran
# configure or cmake.
# For linking vs. a binary build of LLVM, point to the main untarred directory.
# LLVM_BIN_PATH is the directory where binaries are placed by the LLVM build
# process. It should contain the tools like opt, llc and clang. The default
# reflects a release build with CMake and Ninja. binary build of LLVM, point it
# to the bin/ directory.
LLVM_BUILD_PATH := $(CLANG_DIR)/bin
LLVM_BIN_PATH 	:= $(CLANG_DIR)/bin

$(info -----------------------------------------------)
$(info Using CLANG_DIR = $(CLANG_DIR))
$(info Using LLVM_SRC_PATH = $(LLVM_SRC_PATH))
$(info Using LLVM_BUILD_PATH = $(LLVM_BUILD_PATH))
$(info Using LLVM_BIN_PATH = $(LLVM_BIN_PATH))
$(info -----------------------------------------------)

# CXX has to be a fairly modern C++ compiler that supports C++11. gcc 4.8 and
# higher or Clang 3.2 and higher are recommended. Best of all, if you build LLVM
# from sources, use the same compiler you built LLVM with.
# Note: starting with release 3.7, llvm-config will inject flags that gcc may
# not support (for example '-Wcovered-switch-default'). If you run into this
# problem, build with CXX set to a modern clang++ binary instead of g++.
CXX := $(CLANG_DIR)/bin/clang++
CXXFLAGS := -fno-rtti -O0 -g
PLUGIN_CXXFLAGS := -fpic

LLVM_CXXFLAGS := `$(LLVM_BIN_PATH)/llvm-config --cxxflags`
LLVM_LDFLAGS := `$(LLVM_BIN_PATH)/llvm-config --ldflags --libs --system-libs`

# Plugins shouldn't link LLVM and Clang libs statically, because they are
# already linked into the main executable (opt or clang). LLVM doesn't like its
# libs to be linked more than once because it uses globals for configuration
# and plugin registration, and these trample over each other.
LLVM_LDFLAGS_NOLIBS := `$(LLVM_BIN_PATH)/llvm-config --ldflags`
PLUGIN_LDFLAGS := -shared -Wl,-undefined,dynamic_lookup

# These are required when compiling vs. a source distribution of Clang. For
# binary distributions llvm-config --cxxflags gives the right path.
CLANG_INCLUDES := \
	-I$(LLVM_SRC_PATH)/tools/clang/include \
	-I$(LLVM_BUILD_PATH)/tools/clang/include

# List of Clang libraries to link. The proper -L will be provided by the
# call to llvm-config
# Note that I'm using -Wl,--{start|end}-group around the Clang libs; this is
# because there are circular dependencies that make the correct order difficult
# to specify and maintain. The linker group options make the linking somewhat
# slower, but IMHO they're still perfectly fine for tools that link with Clang.

ifneq ($(UNAME),Darwin)
  CLANG_LIBS += \
    -Wl,--start-group
endif
CLANG_LIBS += \
	-lclangAST \
	-lclangAnalysis \
	-lclangBasic \
	-lclangDriver \
	-lclangEdit \
	-lclangFrontend \
	-lclangFrontendTool \
	-lclangLex \
	-lclangParse \
	-lclangSema \
	-lclangRewrite \
	-lclangTooling \
	-lclangSerialization

ifneq ($(UNAME),Darwin)
  CLANG_LIBS += \
    -Wl,--end-group
endif

# Internal paths in this project: where to find sources, and where to put
# build artifacts.

MKDIR = mkdir
SRCDIR = lib
BUILDDIR = bin
SRCS = $(SRCDIR)/main.cpp
DEPS = $(SRCDIR)/consumer.cpp $(SRCDIR)/buildcfg.cpp
OBJS = $(BUILDDIR)/Cavm.o $(BUILDDIR)/FrontendActions.o $(BUILDDIR)/Consumers.o $(BUILDDIR)/ControlDependency.o $(BUILDDIR)/firstRound.o
TARGET= $(BUILDDIR)/buildcfg

.PHONY: all
all: python

python:
	python3 setup.py build_ext -b .

clang: $(TARGET)
	@`[ -d $(BUILDDIR) ] || $(MKDIR) $(BUILDDIR)`

$(TARGET): $(BUILDDIR)/main.o $(OBJS) 
	$(CXX) $(CLANG_LIBS) $(LLVM_LDFLAGS) $^ -o $@

$(BUILDDIR)/main.o: $(SRCDIR)/main.cpp 
	$(CXX) -c $(CXXFLAGS) $(LLVM_CXXFLAGS) $(CLANG_INCLUDES) $< -o $@

$(BUILDDIR)/Cavm.o: $(SRCDIR)/Cavm.cpp 
	$(CXX) -c $(CXXFLAGS) $(LLVM_CXXFLAGS) $(CLANG_INCLUDES) $< -o $@

$(BUILDDIR)/FrontendActions.o: $(SRCDIR)/FrontendActions.cpp 
	$(CXX) -c $(CXXFLAGS) $(LLVM_CXXFLAGS) $(CLANG_INCLUDES) $< -o $@

$(BUILDDIR)/Consumers.o: $(SRCDIR)/Consumers.cpp
	$(CXX) -c $(CXXFLAGS) $(LLVM_CXXFLAGS) $(CLANG_INCLUDES) $< -o $@

$(BUILDDIR)/ControlDependency.o: $(SRCDIR)/ControlDependency.cpp
	$(CXX) -c $(CXXFLAGS) $(LLVM_CXXFLAGS) $(CLANG_INCLUDES) $< -o $@

$(BUILDDIR)/firstRound.o: $(SRCDIR)/firstRound.cpp
	$(CXX) -c $(CXXFLAGS) $(LLVM_CXXFLAGS) $(CLANG_INCLUDES) $< -o $@

# https://packaging.python.org/distributing/#wheels
.PHONY: wheel
wheel:
	python3 setup.py bdist_wheel

.PHONY: clean
clean:
	rm -rf $(OBJS) $(TARGET)
	rm cavm/clang.*.so

.PHONY: test
test:
	python3 test/test.py -v

TESTDIR = test

GTESTFLAGS = -isystem googletest/googletest/include
CAVM_INCLUDES = -I./lib

$(TESTDIR)/gtest_main.a:
	cd $(TESTDIR); make gtest_main.a

$(TESTDIR)/CavmTest.o: $(TESTDIR)/CavmTest.cpp
	$(CXX) -c $(GTESTFLAGS) $(CAVM_INCLUDES) $(CXXFLAGS) $(LLVM_CXXFLAGS) $(CLANG_INCLUDES) $< -o $@

clangtest: $(TESTDIR)/CavmTest.o $(TESTDIR)/gtest_main.a $(OBJS)
	$(CXX) $(GTESTFLAGS) $(CLANG_LIBS) $(LLVM_LDFLAGS) $^ -o $@
