#include "llvm/Support/raw_ostream.h"
#include "./buildcfg.cpp"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        llvm::errs() << "Usage: main <filename>\n";
        return 1;
    }
    instrument(argv[1]);
    return 0;
}
