from distutils.core import setup, Extension
import subprocess

def parse_config (libs, lib_dirs, extra, include_dirs, flags):
  for param in flags:
    opt = param[:2]
    if opt == '-l':
      libs.append(param[2:])
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

BINARY_DIR_PATH = "./clang+llvm-3.9.0-x86_64-apple-darwin"
LLVM_SRC_PATH = BINARY_DIR_PATH
LLVM_BUILD_PATH = BINARY_DIR_PATH + "/bin"
LLVM_BIN_PATH = BINARY_DIR_PATH + "/bin"
CXXFLAGS = "-fno-rtti" # -O0 -g"
extra.append(CXXFLAGS)
PLUGIN_CXXFLAGS = "-fpic"

proc = subprocess.Popen([f'{LLVM_BIN_PATH}/llvm-config', '--cxxflags'], stdout=subprocess.PIPE)
LLVM_CXXFLAGS = proc.communicate()[0].decode().split()
parse_config(libs, lib_dirs, extra, include_dirs, LLVM_CXXFLAGS)

proc = subprocess.Popen([f'{LLVM_BIN_PATH}/llvm-config', '--ldflags', '--libs', '--system-libs'], stdout=subprocess.PIPE)
LLVM_LDFLAGS = proc.communicate()[0].decode().split()
parse_config(libs, lib_dirs, extra, include_dirs, LLVM_LDFLAGS)

print(libs)
print(lib_dirs)
print(extra)
print(include_dirs)

LLVM_LDFLAGS_NOLIBS = f'{LLVM_BIN_PATH}/llvm-config --ldflags'
PLUGIN_LDFLAGS = '-shared -Wl,-undefined,dynamic_lookup'

CLANG_INCLUDES = [f'{LLVM_SRC_PATH}/tools/clang/include', f'{LLVM_BUILD_PATH}/tools/clang/include']

CLANG_LIBS = [
	'clangAST',
	'clangASTMatchers',
	'clangAnalysis', 
	'clangBasic', 
	'clangDriver', 
	'clangEdit', 
	'clangFrontend', 
	'clangFrontendTool', 
	'clangLex', 
	'clangParse', 
	'clangSema', 
	'clangEdit', 
	'clangRewrite', 
	'clangRewriteFrontend', 
	'clangStaticAnalyzerFrontend', 
	'clangStaticAnalyzerCheckers',
	'clangStaticAnalyzerCore', 
	'clangSerialization', 
	'clangToolingCore', 
	'clangTooling', 
	'clangFormat'
]

module = Extension(
  'clang',
  sources=['lib/python_binding.cpp'],
  include_dirs=CLANG_INCLUDES+include_dirs,
  libraries=CLANG_LIBS+libs,
  library_dirs=lib_dirs,
  extra_compile_args=extra
)

setup(
  name = 'clang',
  version = '1.0',
  ext_modules = [module]
)
