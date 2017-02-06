from setuptools import setup, Extension, find_packages
import subprocess
import platform
import os
from os import path

def parse_config (libs, lib_dirs, extra, include_dirs, flags):
  for param in flags:
    opt = param[:2]
    if opt == '-l':
      libs.append(param)
    elif opt == '-L':
      lib_dirs.append(param[2:])
    elif opt == '-I':
      include_dirs.append(param[2:])
    else:
      extra.append(param)
  
libs = []
lib_dirs = []
extra = []
include_dirs = []

if not os.environ.get('BINARY_DIR_PATH'):
  raise SystemExit('BINARY_DIR_PATH is unset.')
BINARY_DIR_PATH = os.environ['BINARY_DIR_PATH']

LLVM_SRC_PATH = BINARY_DIR_PATH
LLVM_BUILD_PATH = BINARY_DIR_PATH + "/bin"
LLVM_BIN_PATH = BINARY_DIR_PATH + "/bin"
CXXFLAGS = "-fno-rtti" # -O0 -g"
extra.append(CXXFLAGS)
PLUGIN_CXXFLAGS = "-fpic"

proc = subprocess.Popen([LLVM_BIN_PATH+'/llvm-config', '--cxxflags'], stdout=subprocess.PIPE)
LLVM_CXXFLAGS = proc.communicate()[0].decode().split()
parse_config(libs, lib_dirs, extra, include_dirs, LLVM_CXXFLAGS)

proc = subprocess.Popen([LLVM_BIN_PATH+'/llvm-config', '--ldflags', '--libs', '--system-libs'], stdout=subprocess.PIPE)
LLVM_LDFLAGS = proc.communicate()[0].decode().split()
parse_config(libs, lib_dirs, extra, include_dirs, LLVM_LDFLAGS)

LLVM_LDFLAGS_NOLIBS = LLVM_BIN_PATH+'/llvm-config --ldflags'
PLUGIN_LDFLAGS = '-shared -Wl,-undefined,dynamic_lookup'

CLANG_INCLUDES = [LLVM_SRC_PATH+'/tools/clang/include', LLVM_BUILD_PATH+'/tools/clang/include']

CLANG_LIBS = [
	'-lclangAST',
	'-lclangASTMatchers',
	'-lclangAnalysis', 
	'-lclangBasic', 
	'-lclangDriver', 
	'-lclangEdit', 
	'-lclangFrontend', 
	'-lclangFrontendTool', 
	'-lclangLex', 
	'-lclangParse', 
	'-lclangSema', 
	'-lclangEdit', 
	'-lclangRewrite', 
	'-lclangRewriteFrontend', 
	'-lclangStaticAnalyzerFrontend', 
	'-lclangStaticAnalyzerCheckers',
	'-lclangStaticAnalyzerCore', 
	'-lclangSerialization', 
	'-lclangToolingCore', 
	'-lclangTooling', 
	'-lclangFormat'
]
if platform.system() != 'Darwin':
  CLANG_LIBS = ['-Wl,--start-group']+CLANG_LIBS+['-Wl,--end-group']

module = Extension(
  'cavm.clang',
  sources=['lib/python_binding.cpp'],
  depends=['lib/buildcfg.cpp', 'lib/consumer.cpp'],
  include_dirs=CLANG_INCLUDES+include_dirs,
  library_dirs=lib_dirs,
  extra_compile_args=extra,
  extra_link_args=CLANG_LIBS+libs
)

here = path.abspath(path.dirname(__file__))
with open(path.join(here, 'README.rst'), encoding='utf-8') as f:
  long_description = f.read()

setup(
  name = 'cavm',
  version = '0.1.0',
  url = 'http://coinse.kaist.ac.kr',
  author = 'COINSE Lab.',
  description = 'A test data generation tool for C/C++ code, based on the Alternating Variable Method (AVM)',
  long_description = long_description,
  license = 'MIT',
  packages = find_packages(),
  entry_points = {
    'console_scripts': [
      'cavm = cavm.main:main',
    ],
  },
  install_requires = [
    'cffi',
  ],
  ext_modules = [module],
  classifiers = [
    'Development Status :: 3 - Alpha',
    'Environment :: Console',
    'License :: OSI Approved :: MIT License',
    'Programming Language :: Python :: 3',
    'Programming Language :: C++',
    'Topic :: Software Development :: Testing',
  ]
)
